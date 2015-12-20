/*
    Copyright (c) 2014 Thilo Borgmann <thilo.borgmann@mail.de>
    Copyright © 2015 by The qTox Project

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

#include "avfoundation.h"

#import <AVFoundation/AVFoundation.h>

QVector<QPair<QString, QString> > avfoundation::getDeviceList()
{
    QVector<QPair<QString, QString> > result;

    NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice* device in devices) {
        result.append({ QString::fromUtf8([[device uniqueID] UTF8String]), QString::fromUtf8([[device localizedName] UTF8String]) });
    }

    return result;
}

QVector<VideoMode> avfoundation::getDeviceModes(QString devName)
{
    QVector<VideoMode> result;

    NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    AVCaptureDevice* device = nil;

    for (AVCaptureDevice* dev in devices) {
        if (devName == QString::fromUtf8([[dev uniqueID] UTF8String])) {
            device = dev;
            break;
        }
    }
    if (device == nil) {
        return result;
    }

    for (AVCaptureDeviceFormat* format in [device formats]) {
        CMFormatDescriptionRef formatDescription;
        CMVideoDimensions dimensions;
        formatDescription = (CMFormatDescriptionRef)[format performSelector:@selector(formatDescription)];
        dimensions = CMVideoFormatDescriptionGetDimensions(formatDescription);

        for (AVFrameRateRange* range in format.videoSupportedFrameRateRanges) {
            VideoMode mode;
            mode.width = dimensions.width;
            mode.height = dimensions.height;
            mode.FPS = range.maxFrameRate;
            result.append(mode);
        }
    }

    return result;
}
