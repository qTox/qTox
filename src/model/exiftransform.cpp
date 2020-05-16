/*
    Copyright © 2020 by The qTox Project Contributors

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

#include "exiftransform.h"

#include <libexif/exif-loader.h>

#include <QDebug>

namespace ExifTransform
{
    Orientation getOrientation(QByteArray imageData)
    {
        auto data = imageData.constData();
        auto size = imageData.size();

        ExifData* exifData = exif_data_new_from_data(reinterpret_cast<const unsigned char*>(data), size);

        if (!exifData) {
            return Orientation::TopLeft;
        }

        int orientation = 0;
        const ExifByteOrder byteOrder = exif_data_get_byte_order(exifData);
        const ExifEntry* const exifEntry = exif_data_get_entry(exifData, EXIF_TAG_ORIENTATION);
        if (exifEntry) {
            orientation = exif_get_short(exifEntry->data, byteOrder);
        }
        exif_data_free(exifData);

        switch (orientation){
        case 1:
            return Orientation::TopLeft;
        case 2:
            return Orientation::TopRight;
        case 3:
            return Orientation::BottomRight;
        case 4:
            return Orientation::BottomLeft;
        case 5:
            return Orientation::LeftTop;
        case 6:
            return Orientation::RightTop;
        case 7:
            return Orientation::RightBottom;
        case 8:
            return Orientation::LeftBottom;
        default:
            qWarning() << "Invalid exif orientation";
            return Orientation::TopLeft;
        }
    }

    QImage applyTransformation(QImage image, Orientation orientation)
    {
        QTransform exifTransform;
        switch (orientation) {
        case Orientation::TopLeft:
            break;
        case Orientation::TopRight:
            image = image.mirrored(1, 0);
            break;
        case Orientation::BottomRight:
            exifTransform.rotate(180);
            break;
        case Orientation::BottomLeft:
            image = image.mirrored(0, 1);
            break;
        case Orientation::LeftTop:
            exifTransform.rotate(-90);
            image = image.mirrored(1, 0);
            break;
        case Orientation::RightTop:
            exifTransform.rotate(90);
            break;
        case Orientation::RightBottom:
            exifTransform.rotate(90);
            image = image.mirrored(1, 0);
            break;
        case Orientation::LeftBottom:
            exifTransform.rotate(-90);
            break;
        }
        image = image.transformed(exifTransform);
        return image;
    }
};
