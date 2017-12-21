#ifndef I_VIDEO_SETTINGS_H
#define I_VIDEO_SETTINGS_H

#include "src/model/interface.h"

#include <QString>
#include <QRect>

class IVideoSettings {
public:
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

#endif // I_VIDEO_SETTINGS_H
