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

#ifndef FILESFORM_H
#define FILESFORM_H

#include "ui_widget.h"

#include <QListWidget>
#include <QTabWidget>
#include <QString>
#include <QDesktopServices>
#include <QLabel>
#include <QVBoxLayout>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>

class FilesForm : public QObject
{
    Q_OBJECT

public:
    FilesForm();
    ~FilesForm();

    void show(Ui::Widget& ui);

public slots:
    void onFileDownloadComplete(const QString& path);
    void onFileUploadComplete(const QString& path);
    
private slots:
    void onFileActivated(QListWidgetItem* item);

private:
    QWidget* head;
    QLabel headLabel;
    QVBoxLayout headLayout;

    /* If we really do go whole hog with listing in progress transers,
    I should really look into the new fangled list thingy, to deactivate
    specific items in the list */
    QTabWidget main;
    QListWidget* sent, * recvd;

};

class ListWidgetItem : public QListWidgetItem
{
    using QListWidgetItem::QListWidgetItem;
  public:
    QString path;
};

#endif // FILESFORM_H
