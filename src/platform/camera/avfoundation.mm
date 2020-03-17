/*
    Copyright © 2014 Thilo Borgmann <thilo.borgmann@mail.de>
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

#include "avfoundation.h"
#include <QObject>

#import <AVFoundation/AVFoundation.h>

QVector<QPair<QString, QString> > avfoundation::getDeviceList()
{
    QVector<QPair<QString, QString> > result;

    NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice* device in devices) {
        result.append({ QString::fromNSString([device uniqueID]), QString::fromNSString([device localizedName]) });
    }

    uint32_t numScreens = 0;
    CGGetActiveDisplayList(0, NULL, &numScreens);
    if (numScreens > 0) {
        CGDirectDisplayID screens[numScreens];
        CGGetActiveDisplayList(numScreens, screens, &numScreens);
        for (uint32_t i = 0; i < numScreens; i++) {
            result.append({ QString("%1 %2").arg(CAPTURE_SCREEN).arg(i), QObject::tr("Capture screen %1").arg(i) });
        }
    }

    return result;
}

QVector<VideoMode> avfoundation::getDeviceModes(QString devName)
{
    QVector<VideoMode> result;

    if (devName.startsWith(CAPTURE_SCREEN)) {
        return result;
    }
    else {
        NSString* deviceName = [NSString stringWithCString:devName.toUtf8() encoding:NSUTF8StringEncoding];
        AVCaptureDevice* device = [AVCaptureDevice deviceWithUniqueID:deviceName];

        if (device == nil) {
            return result;
        }

        for (AVCaptureDeviceFormat* format in [device formats]) {
            CMFormatDescriptionRef formatDescription;
            CMVideoDimensions dimensions;
            formatDescription = static_cast<CMFormatDescriptionRef>([format performSelector:@selector(formatDescription)]);
            dimensions = CMVideoFormatDescriptionGetDimensions(formatDescription);

            for (AVFrameRateRange* range in format.videoSupportedFrameRateRanges) {
                VideoMode mode;
                mode.width = dimensions.width;
                mode.height = dimensions.height;
                mode.FPS = range.maxFrameRate;
                result.append(mode);
            }
        }
    }

    return result;
}
