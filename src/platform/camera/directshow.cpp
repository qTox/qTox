#include "directshow.h"

#include <cstdint>
#include <Objbase.h>
#include <Strmif.h>
#include <uuids.h>

/**
 * Most of this file is adapted from libavdevice's dshow.c,
 * which retrieves useful information but only exposes it to
 * stdout and is not part of the public API for some reason.
 */

static char *wcharToUtf8(wchar_t *w)
{
    int l = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
    char *s = new char[l];
    if (s)
        WideCharToMultiByte(CP_UTF8, 0, w, -1, s, l, 0, 0);
    return s;
}

QVector<QPair<QString,QString>> DirectShow::getDeviceList()
{
    IMoniker* m = nullptr;
    QVector<QPair<QString,QString>> devices;

    ICreateDevEnum* devenum = nullptr;
    if (CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
                             IID_ICreateDevEnum, (void**) &devenum) != S_OK)
        return devices;

    IEnumMoniker* classenum = nullptr;
    if (devenum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
                                (IEnumMoniker**)&classenum, 0) != S_OK)
        return devices;

    while (classenum->Next(1, &m, nullptr) == S_OK)
    {
        VARIANT var;
        IPropertyBag* bag = nullptr;
        LPMALLOC coMalloc = nullptr;
        IBindCtx* bindCtx = nullptr;
        LPOLESTR olestr = nullptr;
        char *devIdString=nullptr, *devHumanName=nullptr;

        if (CoGetMalloc(1, &coMalloc) != S_OK)
            goto fail;
        if (CreateBindCtx(0, &bindCtx) != S_OK)
            goto fail;
        if (m->GetDisplayName(bindCtx, nullptr, &olestr) != S_OK)
            goto fail;

        devIdString = wcharToUtf8(olestr);

        // replace ':' with '_' since FFmpeg uses : to delimitate sources
        for (unsigned i = 0; i < strlen(devIdString); i++)
            if (devIdString[i] == ':')
                devIdString[i] = '_';

        if (m->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void**)&bag) != S_OK)
            goto fail;

        var.vt = VT_BSTR;
        if (bag->Read(L"FriendlyName", &var, nullptr) != S_OK)
            goto fail;
        devHumanName = wcharToUtf8(var.bstrVal);

        devices += {QString("video=")+devIdString, devHumanName};

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
