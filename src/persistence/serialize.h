/*
    Copyright © 2014 by The qTox Project

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


#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <cstdint>
#include <QByteArray>
#include <QString>

/// Most of those functions are unsafe unless otherwise specified
/// Do not use them on untrusted data (e.g. check a signature first)

QByteArray doubleToData(double num);
QByteArray floatToData(float num);
float dataToFloat(QByteArray data);
QByteArray stringToData(QString str);
QString dataToString(QByteArray data);
float dataToRangedSingle(float min, float max, int numberOfBits, QByteArray data);
QByteArray rangedSingleToData(float value, float min, float max, int numberOfBits);
uint8_t dataToUint8(QByteArray data);
uint16_t dataToUint16(QByteArray data);
uint32_t dataToUint32(QByteArray data);
uint64_t dataToUint64(QByteArray data);
unsigned getVUint32Size(QByteArray data);
QByteArray uint8ToData(uint8_t num);
QByteArray uint16ToData(uint16_t num);
QByteArray uint32ToData(uint32_t num);
QByteArray uint64ToData(uint64_t num);

#endif // SERIALIZE_H
