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

#include "identicon.h"
#include "src/core/toxpk.h"

#include <cassert>

#include <QColor>
#include <QCryptographicHash>
#include <QDebug>
#include <QImage>

// The following constants change the appearance of the identicon
// they have been choosen by trying to make the output look nice.
/**
 * @var Identicon::IDENTICON_COLOR_BYTES
 * Specifies how many bytes should define the foreground color
 * must be smaller than 8, else there'll be overflows
 */

/**
 * @var Identicon::COLORS
 * Number of colors to use for the identicon
 */

/**
 * @var Identicon::IDENTICON_ROWS
 * Specifies how many rows of blocks the identicon should have
 */

/**
 * @var Identicon::ACTIVE_COLS
 * Width from the center to the outside, for 5 columns it's 3, 6 -> 3, 7 -> 4
 */

/**
 * @var Identicon::HASH_MIN_LEN
 * Min length of the hash in bytes, 7 bytes control the color,
 * the rest controls the pixel placement
 */

/**
 * @brief Creates an Identicon, that visualizes a hash in graphical form.
 * @param data Data to visualize
 */
Identicon::Identicon(const QByteArray& data)
{
    static_assert(Identicon::COLORS == 2, "Only two colors are implemented");
    // hash with sha256
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    for (int colorIndex = 0; colorIndex < COLORS; ++colorIndex) {
        const QByteArray hashPart = hash.right(IDENTICON_COLOR_BYTES);
        hash.truncate(hash.length() - IDENTICON_COLOR_BYTES);

        const qreal hue = bytesToColor(hashPart);
        // change offset when COLORS != 2
        const qreal lig = static_cast<qreal>(colorIndex) / COLORS + 0.3;
        const qreal sat = 0.5;
        colors[colorIndex].setHslF(hue, sat, lig);
    }

    const uint8_t* const hashBytes = reinterpret_cast<const uint8_t*>(hash.constData());
    // compute the block colors from the hash
    for (int row = 0; row < IDENTICON_ROWS; ++row) {
        for (int col = 0; col < ACTIVE_COLS; ++col) {
            const int hashIdx = row * ACTIVE_COLS + col;
            const uint8_t colorIndex = hashBytes[hashIdx] % COLORS;
            identiconColors[row][col] = colorIndex;
        }
    }
}

/**
 * @brief Converts a series of IDENTICON_COLOR_BYTES bytes to a value in the range 0.0..1.0
 * @param bytes Bytes to convert to a color
 * @return Value in the range of 0.0..1.0
 */
qreal Identicon::bytesToColor(QByteArray bytes)
{
    static_assert(IDENTICON_COLOR_BYTES <= 8, "IDENTICON_COLOR max value is 8");
    const uint8_t* const bytesChr = reinterpret_cast<const uint8_t*>(bytes.constData());
    assert(bytes.length() == IDENTICON_COLOR_BYTES);

    // get foreground color
    uint64_t hue = bytesChr[0];

    // convert the last bytes to an uint
    for (int i = 1; i < IDENTICON_COLOR_BYTES; ++i) {
        hue = hue << 8;
        hue += bytesChr[i];
    }

    // normalize to 0.0 ... 1.0
    return (static_cast<qreal>(hue))
           / (static_cast<qreal>(((static_cast<uint64_t>(1)) << (8 * IDENTICON_COLOR_BYTES)) - 1));
}

/**
 * @brief Writes the Identicon to a QImage
 * @param scaleFactor the image will be a square with scaleFactor * IDENTICON_ROWS pixels,
 *                    must be >= 1
 * @return a QImage with the identicon
 */
QImage Identicon::toImage(int scaleFactor)
{
    if (scaleFactor < 1) {
        qDebug() << "Can't scale with values <1, clamping to 1";
        scaleFactor = 1;
    }

    scaleFactor *= IDENTICON_ROWS;

    QImage pixels(IDENTICON_ROWS, IDENTICON_ROWS, QImage::Format_RGB888);

    for (int row = 0; row < IDENTICON_ROWS; ++row) {
        for (int col = 0; col < IDENTICON_ROWS; ++col) {
            // mirror on vertical axis
            const int columnIdx = abs((col * 2 - (IDENTICON_ROWS - 1)) / 2);
            const int colorIdx = identiconColors[row][columnIdx];
            pixels.setPixel(col, row, colors[colorIdx].rgb());
        }
    }

    // scale up without smoothing to make it look sharp
    return pixels.scaled(scaleFactor, scaleFactor, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}
