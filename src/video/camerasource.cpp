/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include <QMutexLocker>
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <memory>
#include <functional>
#include "camerasource.h"
#include "cameradevice.h"
#include "videoframe.h"

CameraSource::CameraSource()
    : CameraSource{CameraDevice::getDefaultDeviceName()}
{
}

CameraSource::CameraSource(const QString deviceName)
    : CameraSource{deviceName, VideoMode{0,0,0}}
{
}

CameraSource::CameraSource(const QString deviceName, VideoMode mode)
    : deviceName{deviceName}, device{nullptr}, mode(mode),
      cctx{nullptr}, videoStreamIndex{-1},
      biglock{false}, freelistLock{false}, subscriptions{0}
{
    av_register_all();
    avdevice_register_all();
}

CameraSource::~CameraSource()
{
    // Fast lock, in case our stream thread is running
    {
        bool expected = false;
        while (!biglock.compare_exchange_weak(expected, true))
            expected = false;
    }

    // Free all remaining VideoFrame
    // Locking must be done precisely this way to avoid races
    for (int i=0; i<freelist.size(); i++)
    {
        std::shared_ptr<VideoFrame> vframe = freelist[i].lock();
        if (!vframe)
            continue;
        vframe->releaseFrame();
    }

    if (cctx)
        avcodec_free_context(&cctx);
    if (cctxOrig)
        avcodec_close(cctxOrig);

    for (int i=subscriptions; i; --i)
        device->close();
    device = nullptr;
    biglock=false;

    // Synchronize with our stream thread
    while (streamFuture.isRunning())
        QThread::yieldCurrentThread();
}

bool CameraSource::subscribe()
{
    // Fast lock
    {
        bool expected = false;
        while (!biglock.compare_exchange_weak(expected, true))
            expected = false;
    }

    if (device)
    {
        device->open();
        ++subscriptions;
        biglock = false;
        return true;
    }

    // We need to create a new CameraDevice
    AVCodec* codec;
    if (mode)
        device = CameraDevice::open(deviceName, mode);
    else
        device = CameraDevice::open(deviceName);
    if (!device)
    {
        biglock = false;
        return false;
    }

    // Find the first video stream
    for (unsigned i=0; i<device->context->nb_streams; i++)
    {
        if(device->context->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex=i;
            break;
        }
    }
    if (videoStreamIndex == -1)
        goto fail;

    // Get a pointer to the codec context for the video stream
    cctxOrig=device->context->streams[videoStreamIndex]->codec;
    codec=avcodec_find_decoder(cctxOrig->codec_id);
    if(!codec)
        goto fail;

    // Copy context, since we apparently aren't allowed to use the original
    cctx = avcodec_alloc_context3(codec);
    if(avcodec_copy_context(cctx, cctxOrig) != 0)
        goto fail;
    cctx->refcounted_frames = 1;

    // Open codec
    if(avcodec_open2(cctx, codec, nullptr)<0)
    {
        avcodec_free_context(&cctx);
        goto fail;
    }

    if (streamFuture.isRunning())
        qCritical() << "The stream thread is already running! Keeping the current one open.";
    else
        streamFuture = QtConcurrent::run(std::bind(&CameraSource::stream, this));

    // Synchronize with our stream thread
    while (!streamFuture.isRunning())
        QThread::yieldCurrentThread();

    ++subscriptions;
    biglock = false;
    return true;

fail:
    while (!device->close()) {}
    biglock = false;
    return false;
}

void CameraSource::unsubscribe()
{
    // Fast lock
    {
        bool expected = false;
        while (!biglock.compare_exchange_weak(expected, true))
            expected = false;
    }

    if (!device)
    {
        qWarning() << "Unsubscribing with zero subscriber";
        biglock = false;
        return;
    }

    if (--subscriptions == 0)
    {
        // Free all remaining VideoFrame
        // Locking must be done precisely this way to avoid races
        for (int i=0; i<freelist.size(); i++)
        {
            std::shared_ptr<VideoFrame> vframe = freelist[i].lock();
            if (!vframe)
                continue;
            vframe->releaseFrame();
        }

        // Free our resources and close the device
        videoStreamIndex = -1;
        avcodec_free_context(&cctx);
        avcodec_close(cctxOrig);
        cctxOrig = nullptr;
        device->close();
        device = nullptr;

        biglock = false;

        // Synchronize with our stream thread
        while (streamFuture.isRunning())
            QThread::yieldCurrentThread();
    }
    else
    {
        device->close();
        biglock = false;
    }
}

void CameraSource::stream()
{
    auto streamLoop = [=]()
    {
        AVFrame* frame = av_frame_alloc();
        if (!frame)
            return;
        frame->opaque = nullptr;

        AVPacket packet;
        if (av_read_frame(device->context, &packet)<0)
            return;

        // Only keep packets from the right stream;
        if (packet.stream_index==videoStreamIndex)
        {
            // Decode video frame
            int frameFinished;
            avcodec_decode_video2(cctx, frame, &frameFinished, &packet);
            if (!frameFinished)
                return;

            // Broadcast a new VideoFrame, it takes ownership of the AVFrame
            {
                bool expected = false;
                while (!freelistLock.compare_exchange_weak(expected, true))
                    expected = false;
            }

            int freeFreelistSlot = getFreelistSlotLockless();
            auto frameFreeCb = std::bind(&CameraSource::freelistCallback, this, freeFreelistSlot);
            std::shared_ptr<VideoFrame> vframe = std::make_shared<VideoFrame>(frame, frameFreeCb);
            freelist.append(vframe);
            freelistLock = false;
            emit frameAvailable(vframe);
        }

      // Free the packet that was allocated by av_read_frame
      av_free_packet(&packet);
    };

    forever {
        // Fast lock
        {
            bool expected = false;
            while (!biglock.compare_exchange_weak(expected, true))
                expected = false;
        }

        if (!device)
        {
            biglock = false;
            return;
        }

        streamLoop();

        // Give a chance to other functions to pick up the lock if needed
        biglock = false;
        QThread::yieldCurrentThread();
    }
}

void CameraSource::freelistCallback(int freelistIndex)
{
    // Fast lock
    {
        bool expected = false;
        while (!freelistLock.compare_exchange_weak(expected, true))
            expected = false;
    }
    freelist[freelistIndex].reset();
    freelistLock = false;
}

int CameraSource::getFreelistSlotLockless()
{
    int size = freelist.size();
    for (int i=0; i<size; ++i)
        if (freelist[i].expired())
            return i;
    freelist.resize(size+(size>>1)+4); // Arbitrary growth strategy, should work well
    return size;
}
