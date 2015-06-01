#include "v4l2.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

/**
 * Most of this file is adapted from libavdevice's v4l2.c,
 * which retrieves useful information but only exposes it to
 * stdout and is not part of the public API for some reason.
 */

static int deviceOpen(QString devName)
{
    struct v4l2_capability cap;
    int fd;
    int err;

    fd = open(devName.toStdString().c_str(), O_RDWR, 0);
    if (fd < 0)
        return errno;

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        err = errno;
        goto fail;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        err = ENODEV;
        goto fail;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        err = ENOSYS;
        goto fail;
    }

    return fd;

fail:
    close(fd);
    return err;
}

static QVector<unsigned short> getDeviceModeFramerates(int fd, unsigned w, unsigned h, uint32_t pixelFormat)
{
    QVector<unsigned short> rates;
    v4l2_frmivalenum vfve = { .pixel_format = pixelFormat, .height = h, .width = w };

    while(!ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &vfve)) {
        int rate;
        switch (vfve.type) {
        case V4L2_FRMSIZE_TYPE_DISCRETE:
            rate = vfve.discrete.denominator / vfve.discrete.numerator;
            if (!rates.contains(rate))
                rates.append(rate);
        break;
        case V4L2_FRMSIZE_TYPE_CONTINUOUS:
        case V4L2_FRMSIZE_TYPE_STEPWISE:
            rate = vfve.stepwise.min.denominator / vfve.stepwise.min.numerator;
            if (!rates.contains(rate))
                rates.append(rate);
        }
        vfve.index++;
    }

    return rates;
}

QVector<VideoMode> v4l2::getDeviceModes(QString devName)
{
    QVector<VideoMode> modes;

    int fd = deviceOpen(devName);
    if (fd < 0)
        return modes;
    v4l2_fmtdesc vfd = { .type = V4L2_BUF_TYPE_VIDEO_CAPTURE };

    while(!ioctl(fd, VIDIOC_ENUM_FMT, &vfd)) {
        vfd.index++;

        v4l2_frmsizeenum vfse = { .pixel_format = vfd.pixelformat };

        while(!ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &vfse)) {
            VideoMode mode;
            switch (vfse.type) {
            case V4L2_FRMSIZE_TYPE_DISCRETE:
                mode.width = vfse.discrete.width;
                mode.height = vfse.discrete.height;
            break;
            case V4L2_FRMSIZE_TYPE_CONTINUOUS:
            case V4L2_FRMSIZE_TYPE_STEPWISE:
                mode.width = vfse.stepwise.max_width;
                mode.height = vfse.stepwise.max_height;
            }
            QVector<unsigned short> rates = getDeviceModeFramerates(fd, mode.width, mode.height, vfd.pixelformat);
            for (unsigned short rate : rates)
            {
                mode.FPS = rate;
                if (!modes.contains(mode))
                    modes.append(std::move(mode));
            }
            vfse.index++;
        }
    }

    return modes;
}
