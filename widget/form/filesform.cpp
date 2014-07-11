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

#include "filesform.h"

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
    
    main.addTab(&recvd, tr("Downloads"));
    main.addTab(&sent, tr("Uploads"));
    
    connect(&sent, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onUploadFileActivated(QListWidgetItem*)));
    connect(&recvd, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onDownloadFileActivated(QListWidgetItem*)));

}

FilesForm::~FilesForm()
{
    //delete head;
    // having this line caused a SIGABRT because free() received an invalid pointer
    // but since this is only called on program shutdown anyways, 
    // I'm not too bummed about removing it
}

void FilesForm::show(Ui::Widget& ui)
{
    ui.mainContent->layout()->addWidget(&main);
    ui.mainHead->layout()->addWidget(head);
    main.show();
    head->show();
}

void FilesForm::onFileDownloadComplete(const QString& path)
{
    QListWidgetItem* tmp = new QListWidgetItem(QIcon(":/img/checkmark.png"), path);
    recvd.addItem(tmp);
}

void FilesForm::onFileUploadComplete(const QString& path)
{
    QListWidgetItem* tmp = new QListWidgetItem(QIcon(":/img/checkmark.png"), path);
    sent.addItem(tmp);
}

// sadly, the ToxFile struct in core only has the file name, not the file path...
// so currently, these don't work as intended (though for now, downloads might work
// whenever they're not saved anywhere custom, thanks to the hack)
// I could do some digging around, but for now I'm tired and others already 
// might know it without me needing to dig, so...
void FilesForm::onDownloadFileActivated(QListWidgetItem* item)
{
    QUrl url = QUrl::fromLocalFile("./" + item->text());
    qDebug() << "Opening '" << url << "'";
    QDesktopServices::openUrl(url);
}

void FilesForm::onUploadFileActivated(QListWidgetItem* item)
{
    QUrl url = QUrl::fromLocalFile(item->text());
    qDebug() << "Opening '" << url << "'";
    QDesktopServices::openUrl(url);
}
