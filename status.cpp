/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    
    This file is part of Tox Qt GUI.
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    
    See the COPYING file for more details.
*/

#include "status.h"

const QList<StatusHelper::Info> StatusHelper::info =
{
    {"Online", ":/icons/status_online.png"},
    {"Away", ":/icons/status_away.png"},
    {"Busy", ":/icons/status_busy.png"},
    {"Offline", ":/icons/status_offline.png"}
};

StatusHelper::Info StatusHelper::getInfo(int status)
{
    return info.at(status);
}

StatusHelper::Info StatusHelper::getInfo(Status status)
{
    return StatusHelper::getInfo(static_cast<int>(status));
}
