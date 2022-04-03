/*
    Copyright © 2010 Ramiro Polla
    Copyright © 2015-2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "directshow.h"

// Because of replacing to incorrect order, which leads to building failing,
// this region is ignored for clang-format
// clang-format off
#include <cstdint>
#include <objbase.h>
#include <strmif.h>
#include <amvideo.h>
#include <dvdmedia.h>
#include <uuids.h>
#include <cassert>
#include <QDebug>
// clang-format on

/**
 * Most of this file is adapted from libavdevice's dshow.c,
 * which retrieves useful information but only exposes it to
 * stdout and is not part of the public API for some reason.
 */

namespace {
char* wcharToUtf8(wchar_t* w)
{
    int l = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    char* s = new char[l];
    if (s)
        WideCharToMultiByte(CP_UTF8, 0, w, -1, s, l, nullptr, nullptr);
    return s;
}
} // namespace

QVector<QPair<QString, QString>> DirectShow::getDeviceList()
{
    IMoniker* m = nullptr;
    QVector<QPair<QString, QString>> devices;

    ICreateDevEnum* devenum = nullptr;
    if (CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
                         reinterpret_cast<void**>(&devenum))
        != S_OK)
        return devices;

    IEnumMoniker* classenum = nullptr;
    if (devenum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, reinterpret_cast<IEnumMoniker**>(&classenum), 0)
        != S_OK)
        return devices;

    while (classenum->Next(1, &m, nullptr) == S_OK) {
        VARIANT var;
        IPropertyBag* bag = nullptr;
        LPMALLOC coMalloc = nullptr;
        IBindCtx* bindCtx = nullptr;
        LPOLESTR olestr = nullptr;
        char *devIdString = nullptr, *devHumanName = nullptr;

        if (CoGetMalloc(1, &coMalloc) != S_OK)
            goto fail;
        if (CreateBindCtx(0, &bindCtx) != S_OK)
            goto fail;

        // Get an uuid for the device that we can pass to ffmpeg directly
        if (m->GetDisplayName(bindCtx, nullptr, &olestr) != S_OK)
            goto fail;
        devIdString = wcharToUtf8(olestr);

        // replace ':' with '_' since FFmpeg uses : to delimitate sources
        for (size_t i = 0; i < strlen(devIdString); ++i)
            if (devIdString[i] == ':')
                devIdString[i] = '_';

        // Get a human friendly name/description
        if (m->BindToStorage(nullptr, nullptr, IID_IPropertyBag, reinterpret_cast<void**>(&bag)) != S_OK)
            goto fail;

        var.vt = VT_BSTR;
        if (bag->Read(L"FriendlyName", &var, nullptr) != S_OK)
            goto fail;
        devHumanName = wcharToUtf8(var.bstrVal);

        devices += {QString("video=") + QString::fromUtf8(devIdString), QString::fromUtf8(devHumanName)};

    fail:
        if (olestr && coMalloc)
            coMalloc->Free(olestr);
        if (bindCtx)
            bindCtx->Release();
        delete[] devIdString;
        delete[] devHumanName;
        if (bag)
            bag->Release();
        m->Release();
    }
    classenum->Release();

    return devices;
}

// Used (by getDeviceModes) to select a device
// so we can list its properties
static IBaseFilter* getDevFilter(QString devName)
{
    IBaseFilter* devFilter = nullptr;
    devName = devName.mid(6); // Remove the "video="
    IMoniker* m = nullptr;

    ICreateDevEnum* devenum = nullptr;
    if (CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
                         reinterpret_cast<void**>(&devenum))
        != S_OK)
        return devFilter;

    IEnumMoniker* classenum = nullptr;
    if (devenum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, reinterpret_cast<IEnumMoniker**>(&classenum), 0)
        != S_OK)
        return devFilter;

    while (classenum->Next(1, &m, nullptr) == S_OK) {
        LPMALLOC coMalloc = nullptr;
        IBindCtx* bindCtx = nullptr;
        LPOLESTR olestr = nullptr;
        char* devIdString = nullptr;

        if (CoGetMalloc(1, &coMalloc) != S_OK)
            goto fail;
        if (CreateBindCtx(0, &bindCtx) != S_OK)
            goto fail;

        if (m->GetDisplayName(bindCtx, nullptr, &olestr) != S_OK)
            goto fail;
        devIdString = wcharToUtf8(olestr);

        // replace ':' with '_' since FFmpeg uses : to delimitate sources
        for (size_t i = 0; i < strlen(devIdString); ++i)
            if (devIdString[i] == ':')
                devIdString[i] = '_';

        if (devName.toUtf8().constData() != devIdString)
            goto fail;

        if (m->BindToObject(nullptr, nullptr, IID_IBaseFilter, reinterpret_cast<void**>(&devFilter)) != S_OK)
            goto fail;

    fail:
        if (olestr && coMalloc)
            coMalloc->Free(olestr);
        if (bindCtx)
            bindCtx->Release();
        delete[] devIdString;
        m->Release();
    }
    classenum->Release();

    if (!devFilter)
        qWarning() << "Couldn't find the device " << devName;

    return devFilter;
}

QVector<VideoMode> DirectShow::getDeviceModes(QString devName)
{
    QVector<VideoMode> modes;

    IBaseFilter* devFilter = getDevFilter(devName);
    if (!devFilter)
        return modes;

    // The outter loop tries to find a valid output pin
    GUID category;
    DWORD r2;
    IEnumPins* pins = nullptr;
    IPin* pin;
    if (devFilter->EnumPins(&pins) != S_OK)
        return modes;

    while (pins->Next(1, &pin, nullptr) == S_OK) {
        IKsPropertySet* p = nullptr;
        PIN_INFO info;

        pin->QueryPinInfo(&info);
        info.pFilter->Release();
        if (info.dir != PINDIR_OUTPUT)
            goto next;
        if (pin->QueryInterface(IID_IKsPropertySet, reinterpret_cast<void**>(&p)) != S_OK)
            goto next;
        if (p->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, nullptr, 0, &category, sizeof(GUID), &r2)
            != S_OK)
            goto next;
        if (!IsEqualGUID(category, PIN_CATEGORY_CAPTURE))
            goto next;

        // Now we can list the video modes for the current pin
        // Prepare for another wall of spaghetti DIRECT SHOW QUALITY code
        {
            IAMStreamConfig* config = nullptr;
            VIDEO_STREAM_CONFIG_CAPS* vcaps = nullptr;
            int size, n;
            if (pin->QueryInterface(IID_IAMStreamConfig, reinterpret_cast<void**>(&config)) != S_OK)
                goto next;
            if (config->GetNumberOfCapabilities(&n, &size) != S_OK)
                goto pinend;

            assert(size == sizeof(VIDEO_STREAM_CONFIG_CAPS));
            vcaps = new VIDEO_STREAM_CONFIG_CAPS;

            for (int i = 0; i < n; ++i) {
                AM_MEDIA_TYPE* type = nullptr;
                VideoMode mode;
                if (config->GetStreamCaps(i, &type, reinterpret_cast<BYTE*>(vcaps)) != S_OK)
                    goto nextformat;

                if (!IsEqualGUID(type->formattype, FORMAT_VideoInfo)
                    && !IsEqualGUID(type->formattype, FORMAT_VideoInfo2))
                    goto nextformat;

                mode.width = vcaps->MaxOutputSize.cx;
                mode.height = vcaps->MaxOutputSize.cy;
                mode.FPS = 1e7 / vcaps->MinFrameInterval;
                if (!modes.contains(mode))
                    modes.append(std::move(mode));

            nextformat:
                if (type->pbFormat)
                    CoTaskMemFree(type->pbFormat);
                CoTaskMemFree(type);
            }
        pinend:
            config->Release();
            delete vcaps;
        }
    next:
        if (p)
            p->Release();
        pin->Release();
    }

    return modes;
}
