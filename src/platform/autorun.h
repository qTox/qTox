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

#ifdef QTOX_PLATFORM_EXT

#ifndef PLATFORM_AUTORUN_H
#define PLATFORM_AUTORUN_H


namespace Platform
{
    bool setAutorun(bool on);
    bool getAutorun();
}

#endif // PLATFORM_AUTORUN_H

#endif // QTOX_PLATFORM_EXT
