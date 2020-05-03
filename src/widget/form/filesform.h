/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#pragma once

#include <QLabel>
#include <QListWidgetItem>
#include <QString>
#include <QTabWidget>
#include <QVBoxLayout>

class ContentLayout;
class QListWidget;

class FilesForm : public QObject
{
    Q_OBJECT

public:
    FilesForm();
    ~FilesForm();

    bool isShown() const;
    void show(ContentLayout* contentLayout);

public slots:
    void onFileDownloadComplete(const QString& path);
    void onFileUploadComplete(const QString& path);

private slots:
    void onFileActivated(QListWidgetItem* item);

private:
    void retranslateUi();

private:
    QWidget* head;
    QIcon doneIcon;
    QLabel headLabel;
    QVBoxLayout headLayout;
    QTabWidget main;
    QListWidget *sent, *recvd;
};
