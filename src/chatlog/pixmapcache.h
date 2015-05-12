/*
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

#ifndef ICONCACHE_H
#define ICONCACHE_H

#include <QIcon>
#include <QPixmap>
#include <QHash>

class PixmapCache
{
public:
    QPixmap get(const QString& filename, QSize size);
    static PixmapCache& getInstance();

protected:
    PixmapCache() {}
    PixmapCache(PixmapCache&) = delete;
    PixmapCache& operator=(const PixmapCache&) = delete;

private:
    QHash<QString, QIcon> cache;
};

#endif // ICONCACHE_H
