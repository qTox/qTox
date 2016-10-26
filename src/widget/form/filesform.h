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

#ifndef FILESFORM_H
#define FILESFORM_H

#include <QTabWidget>
#include <QString>
#include <QLabel>

#include "src/widget/contentwidget.h"

class QListWidget;
class QListWidgetItem;

class FilesForm : public ContentWidget
{
    Q_OBJECT

public:
    explicit FilesForm(QWidget* parent = nullptr);
    ~FilesForm();

public slots:
    void onFileDownloadComplete(const QString& path);
    void onFileUploadComplete(const QString& path);

private slots:
    void onFileActivated(QListWidgetItem* item);

private:
    void retranslateUi();

private:
    QIcon doneIcon;
    QLabel headLabel;
    QTabWidget main;
    QListWidget* sent;
    QListWidget* recvd;
};

#endif // FILESFORM_H
