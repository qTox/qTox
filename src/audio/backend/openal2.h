/*
    Copyright © 2014-2017 by The qTox Project Contributors

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


#ifndef OPENAL2_H
#define OPENAL2_H

#include "openal.h"
#include "src/audio/audio.h"

#include <atomic>
#include <cmath>

#include <QMutex>
#include <QObject>
#include <QTimer>

#include <cassert>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

extern "C" {
#include <filter_audio.h>
}

// needed because Ubuntu 14.04 lacks the AL_SOFT_source_latency extension
#ifndef AL_SOFT_source_latency
#define AL_SAMPLE_OFFSET_LATENCY_SOFT 0x1200
#define AL_SEC_OFFSET_LATENCY_SOFT 0x1201
extern "C" typedef void(AL_APIENTRY* LPALGETSOURCEDVSOFT)(ALuint, ALenum, const ALdouble*);
#endif

class OpenAL2 : public OpenAL
{
    Q_OBJECT

public:
    OpenAL2();

protected:
    bool initInput(const QString& deviceName) override;
    bool initOutput(const QString& outDevDescr) override;
    void cleanupOutput() override;
    void playMono16SoundCleanup();
    void doAudio() override;
    void doInput();
    void doOutput();
    bool loadOpenALExtensions(ALCdevice* dev);
    bool initOutputEchoCancel();

private:
    ALCdevice* alProxyDev;
    ALCcontext* alProxyContext;
    ALuint alProxySource;
    ALuint alProxyBuffer;
    bool echoCancelSupported = false;

    Filter_Audio* filterer = nullptr;
    LPALCLOOPBACKOPENDEVICESOFT alcLoopbackOpenDeviceSOFT = nullptr;
    LPALCISRENDERFORMATSUPPORTEDSOFT alcIsRenderFormatSupportedSOFT = nullptr;
    LPALGETSOURCEDVSOFT alGetSourcedvSOFT = nullptr;
    LPALCRENDERSAMPLESSOFT alcRenderSamplesSOFT = nullptr;
};

#endif // OPENAL2_H
