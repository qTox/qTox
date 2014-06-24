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

#ifndef FRIENDREQUESTDIALOG_HPP
#define FRIENDREQUESTDIALOG_HPP

#include <QDialog>

class FriendRequestDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FriendRequestDialog(QWidget *parent, const QString &userId, const QString &message);
};

#endif // FRIENDREQUESTDIALOG_HPP
