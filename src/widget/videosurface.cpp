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

#include "videosurface.h"
#include "src/video/camera.h"
#include <QTimer>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QDebug>

VideoSurface::VideoSurface(QWidget* parent)
    : QGLWidget(QGLFormat(QGL::SingleBuffer), parent)
    , source{nullptr}
    , pbo{nullptr, nullptr}
    , bgrProgramm{nullptr}
    , yuvProgramm{nullptr}
    , textureId{0}
    , pboAllocSize{0}
    , hasSubscribed{false}
    , pboIndex{0}
{
    
}

VideoSurface::VideoSurface(VideoSource *source, QWidget* parent)
    : VideoSurface(parent)
{
    setSource(source);
}

VideoSurface::~VideoSurface()
{
    if (pbo[0])
    {
        delete pbo[0];
        delete pbo[1];
    }

    if (textureId != 0)
        glDeleteTextures(1, &textureId);

    delete bgrProgramm;
    delete yuvProgramm;

    unsubscribe();
}

void VideoSurface::setSource(VideoSource *src)
{
    if (source == src)
        return;

    unsubscribe();
    source = src;
    subscribe();
}

void VideoSurface::initializeGL()
{
    QGLWidget::initializeGL();
    qDebug() << "Init";
    // pbo
    pbo[0] = new QOpenGLBuffer(QOpenGLBuffer::PixelUnpackBuffer);
    pbo[0]->setUsagePattern(QOpenGLBuffer::StreamDraw);
    pbo[0]->create();

    pbo[1] = new QOpenGLBuffer(QOpenGLBuffer::PixelUnpackBuffer);
    pbo[1]->setUsagePattern(QOpenGLBuffer::StreamDraw);
    pbo[1]->create();

    // shaders
    bgrProgramm = new QOpenGLShaderProgram;
    bgrProgramm->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                     "attribute vec4 vertices;"
                                     "varying vec2 coords;"
                                     "void main() {"
                                     "    gl_Position = vec4(vertices.xy, 0.0, 1.0);"
                                     "    coords = vertices.xy*vec2(0.5, 0.5) + vec2(0.5, 0.5);"
                                     "}");

    // brg frag-shader
    bgrProgramm->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                     "uniform sampler2D texture0;"
                                     "varying vec2 coords;"
                                     "void main() {"
                                     "    vec4 color = texture2D(texture0,coords*vec2(1.0, -1.0));"
                                     "    gl_FragColor = vec4(color.bgr, 1.0);"
                                     "}");

    bgrProgramm->bindAttributeLocation("vertices", 0);
    bgrProgramm->link();

    // shaders
    yuvProgramm = new QOpenGLShaderProgram;
    yuvProgramm->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                     "attribute vec4 vertices;"
                                     "varying vec2 coords;"
                                     "void main() {"
                                     "    gl_Position = vec4(vertices.xy, 0.0, 1.0);"
                                     "    coords = vertices.xy*vec2(0.5, 0.5) + vec2(0.5, 0.5);"
                                     "}");

    // yuv frag-shader
    yuvProgramm->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                     "uniform sampler2D texture0;"
                                     "varying vec2 coords;"
                                     "void main() {"
                                     "      vec3 yuv = texture2D(texture0,coords*vec2(1.0, -1.0)).rgb - vec3(0.0, 0.5, 0.5);"
                                     "      vec3 rgb = mat3(1.0, 1.0, 1.0, 0.0, -0.21482, 2.12798, 1.28033, -0.38059, 0.0)*yuv;"
                                     "      gl_FragColor = vec4(rgb, 1.0);"
                                     "}");

    yuvProgramm->bindAttributeLocation("vertices", 0);
    yuvProgramm->link();
}

void VideoSurface::paintGL()
{
    mutex.lock();
    VideoFrame currFrame = frame;
    frame.invalidate();
    mutex.unlock();

    if (currFrame.isValid() && res != currFrame.resolution)
    {
        res = currFrame.resolution;

        // delete old texture
        if (textureId != 0)
            glDeleteTextures(1, &textureId);

        // a texture used to render the pbo (has the match the pixelformat of the source)
        glGenTextures(1,&textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D,0, GL_RGB, res.width(), res.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    if (currFrame.isValid())
    {
        pboIndex = (pboIndex + 1) % 2;
        int nextPboIndex = (pboIndex + 1) % 2;

        if (pboAllocSize != currFrame.frameData.size())
        {
            qDebug() << "Resize pbo " << currFrame.frameData.size() << "(" << currFrame.resolution << ")" << "bytes (before" << pboAllocSize << ")";

            pbo[0]->bind();
            pbo[0]->allocate(currFrame.frameData.size());
            pbo[0]->release();

            pbo[1]->bind();
            pbo[1]->allocate(currFrame.frameData.size());
            pbo[1]->release();

            pboAllocSize = currFrame.frameData.size();
        }


        pbo[pboIndex]->bind();
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0, res.width(), res.height(), GL_RGB, GL_UNSIGNED_BYTE, 0);
        pbo[pboIndex]->unmap();
        pbo[pboIndex]->release();

        // transfer data
        pbo[nextPboIndex]->bind();
        void* ptr = pbo[nextPboIndex]->map(QOpenGLBuffer::WriteOnly);
        if (ptr)
            memcpy(ptr, currFrame.frameData.data(), currFrame.frameData.size());
        pbo[nextPboIndex]->unmap();
        pbo[nextPboIndex]->release();
    }

    // background
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // keep aspect ratio
    float aspectRatio = float(res.width()) / float(res.height());
    if (width() < float(height()) * aspectRatio)
    {
        float h = float(width()) / aspectRatio;
        glViewport(0, (height() - h)*0.5f, width(), h);
    }
    else
    {
        float w = float(height()) * float(aspectRatio);
        glViewport((width() - w)*0.5f, 0, w, height());
    }

    QOpenGLShaderProgram* programm = nullptr;
    switch (frame.format)
    {
    case VideoFrame::YUV:
        programm = yuvProgramm;
        break;
    case VideoFrame::BGR:
        programm = bgrProgramm;
        break;
    default:
        break;
    }

    if (programm)
    {
        // render pbo
        static float values[] = {
            -1, -1,
            1, -1,
            -1, 1,
            1, 1
        };

        programm->bind();
        programm->setAttributeArray(0, GL_FLOAT, values, 2);
        programm->enableAttributeArray(0);
    }

    glBindTexture(GL_TEXTURE_2D, textureId);

    //draw fullscreen quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);

    if (programm)
    {
        programm->disableAttributeArray(0);
        programm->release();
    }
}

void VideoSurface::subscribe()
{
    if (source && !hasSubscribed)
    {
        source->subscribe();
        hasSubscribed = true;
        connect(source, &VideoSource::frameAvailable, this, &VideoSurface::onNewFrameAvailable);
    }
}

void VideoSurface::unsubscribe()
{
    if (source && hasSubscribed)
    {
        source->unsubscribe();
        hasSubscribed = false;
        disconnect(source, &VideoSource::frameAvailable, this, &VideoSurface::onNewFrameAvailable);
    }
}

void VideoSurface::onNewFrameAvailable(const VideoFrame& newFrame)
{
    mutex.lock();
    frame = newFrame;
    mutex.unlock();

    updateGL();
}



