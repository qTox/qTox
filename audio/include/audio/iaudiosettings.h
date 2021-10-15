/*
    Copyright © 2019 by The qTox Project Contributors

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

#pragma once

#include "util/interface.h"

#include <QString>

class IAudioSettings {
public:
    IAudioSettings() = default;
    virtual ~IAudioSettings();
    IAudioSettings(const IAudioSettings&) = default;
    IAudioSettings& operator=(const IAudioSettings&) = default;
    IAudioSettings(IAudioSettings&&) = default;
    IAudioSettings& operator=(IAudioSettings&&) = default;

    virtual QString getInDev() const = 0;
    virtual void setInDev(const QString& deviceSpecifier) = 0;

    virtual bool getAudioInDevEnabled() const = 0;
    virtual void setAudioInDevEnabled(bool enabled) = 0;

    virtual QString getOutDev() const = 0;
    virtual void setOutDev(const QString& deviceSpecifier) = 0;

    virtual bool getAudioOutDevEnabled() const = 0;
    virtual void setAudioOutDevEnabled(bool enabled) = 0;

    virtual qreal getAudioInGainDecibel() const = 0;
    virtual void setAudioInGainDecibel(qreal dB) = 0;

    virtual qreal getAudioThreshold() const = 0;
    virtual void setAudioThreshold(qreal percent) = 0;

    virtual int getOutVolume() const = 0;
    virtual int getOutVolumeMin() const = 0;
    virtual int getOutVolumeMax() const = 0;
    virtual void setOutVolume(int volume) = 0;

    virtual int getAudioBitrate() const = 0;
    virtual void setAudioBitrate(int bitrate) = 0;

    virtual bool getEnableTestSound() const = 0;
    virtual void setEnableTestSound(bool newValue) = 0;

    DECLARE_SIGNAL(inDevChanged, const QString& device);
    DECLARE_SIGNAL(audioInDevEnabledChanged, bool enabled);

    DECLARE_SIGNAL(outDevChanged, const QString& device);
    DECLARE_SIGNAL(audioOutDevEnabledChanged, bool enabled);

    DECLARE_SIGNAL(audioInGainDecibelChanged, qreal dB);
    DECLARE_SIGNAL(audioThresholdChanged, qreal dB);
    DECLARE_SIGNAL(outVolumeChanged, int volume);
    DECLARE_SIGNAL(audioBitrateChanged, int bitrate);
    DECLARE_SIGNAL(enableTestSoundChanged, bool newValue);
};
