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
#include <QRect>

class IVideoSettings {
public:
    virtual ~IVideoSettings() = default;

    virtual QString getVideoDev() const = 0;
    virtual void setVideoDev(const QString& deviceSpecifier) = 0;

    virtual QRect getScreenRegion() const = 0;
    virtual void setScreenRegion(const QRect& value) = 0;

    virtual bool getScreenGrabbed() const = 0;
    virtual void setScreenGrabbed(bool value) = 0;

    virtual QRect getCamVideoRes() const = 0;
    virtual void setCamVideoRes(QRect newValue) = 0;

    virtual float getCamVideoFPS() const = 0;
    virtual void setCamVideoFPS(float newValue) = 0;

    DECLARE_SIGNAL(videoDevChanged, const QString& device);
    DECLARE_SIGNAL(screenRegionChanged, const QRect& region);
    DECLARE_SIGNAL(screenGrabbedChanged, bool enabled);
    DECLARE_SIGNAL(camVideoResChanged, const QRect& region);
    DECLARE_SIGNAL(camVideoFPSChanged, unsigned short fps);
};
