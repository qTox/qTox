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

#ifndef STATUS_HPP
#define STATUS_HPP

#include <QObject>

enum class Status : int {Online = 0, Away, Busy, Offline};

class StatusHelper
{
public:

    static const int MAX_STATUS = static_cast<int>(Status::Offline);

    struct Info {
        QString name;
        QString iconPath;
    };

    static Info getInfo(int status);
    static Info getInfo(Status status);

private:
    const static QList<Info> info;

};

Q_DECLARE_METATYPE(Status)

#endif // STATUS_HPP
