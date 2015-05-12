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

#include "filesform.h"
#include "ui_mainwindow.h"
#include "src/widget/widget.h"
#include <QFileInfo>
#include <QUrl>
#include <QDebug>
#include <QPainter>

FilesForm::FilesForm()
    : QObject()
{
    head = new QWidget();
    QFont bold;
    bold.setBold(true);
    headLabel.setText(tr("Transfered Files","\"Headline\" of the window"));
    headLabel.setFont(bold);
    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);
    
    recvd = new QListWidget;
    sent = new QListWidget;
    
    main.addTab(recvd, tr("Downloads"));
    main.addTab(sent, tr("Uploads"));
    
    connect(sent, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onFileActivated(QListWidgetItem*)));
    connect(recvd, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onFileActivated(QListWidgetItem*)));

}

FilesForm::~FilesForm()
{
    delete recvd;
    delete sent;
    head->deleteLater();
}

void FilesForm::show(Ui::MainWindow& ui)
{
    ui.mainContent->layout()->addWidget(&main);
    ui.mainHead->layout()->addWidget(head);
    main.show();
    head->show();
}

void FilesForm::onFileDownloadComplete(const QString& path)
{
    ListWidgetItem* tmp = new ListWidgetItem(QIcon(":/ui/fileTransferWidget/fileDone.svg"), QFileInfo(path).fileName());
    tmp->path = path;
    recvd->addItem(tmp);
}

void FilesForm::onFileUploadComplete(const QString& path)
{
    ListWidgetItem* tmp = new ListWidgetItem(QIcon(":/ui/fileTransferWidget/fileDone.svg"), QFileInfo(path).fileName());
    tmp->path = path;
    sent->addItem(tmp);
}

// sadly, the ToxFile struct in core only has the file name, not the file path...
// so currently, these don't work as intended (though for now, downloads might work
// whenever they're not saved anywhere custom, thanks to the hack)
// I could do some digging around, but for now I'm tired and others already 
// might know it without me needing to dig, so...
void FilesForm::onFileActivated(QListWidgetItem* item)
{
    ListWidgetItem* tmp = dynamic_cast<ListWidgetItem*> (item);

    Widget::confirmExecutableOpen(QFileInfo(tmp->path));
}
