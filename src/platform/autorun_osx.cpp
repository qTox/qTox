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

#if defined(__APPLE__) && defined(__MACH__)
#include "src/platform/autorun.h"


bool Platform::setAutorun(bool on)
{
    return false;
}

bool Platform::getAutorun()
{
    return false;
}

#endif  // defined(__APPLE__) && defined(__MACH__)
