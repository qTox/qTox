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

#ifndef LOADHISTORYDIALOG_H
#define LOADHISTORYDIALOG_H

#include <QDialog>
#include <QDateTime>

namespace Ui {
class LoadHistoryDialog;
}

class LoadHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadHistoryDialog(QWidget *parent = 0);
    ~LoadHistoryDialog();

    QDateTime getFromDate();

private:
    Ui::LoadHistoryDialog *ui;
};

#endif // LOADHISTORYDIALOG_H
