/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H

#include <QIODevice>
#include <QByteArray>
#include <QMutex>

class AudioBuffer : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioBuffer();
    ~AudioBuffer();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;
    qint64 bufferSize() const;
    void clear();

private:
    QByteArray buffer;
    mutable QMutex bufferMutex;
};

#endif // AUDIOBUFFER_H
