/*
    Copyright © 2014-2015 by The qTox Project

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

#include "filesform.h"
#include "ui_mainwindow.h"
#include "src/widget/widget.h"
#include "src/widget/translator.h"
#include <QFileInfo>

FilesForm::FilesForm()
    : QObject(), doneIcon(":/ui/fileTransferWidget/fileDone.svg")
{
    head = new QWidget();
    QFont bold;
    bold.setBold(true);
    headLabel.setFont(bold);
    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

    recvd = new QListWidget;
    sent = new QListWidget;

    main.addTab(recvd, QString());
    main.addTab(sent, QString());

    connect(sent, &QListWidget::itemActivated, this, &FilesForm::onFileActivated);
    connect(recvd, &QListWidget::itemActivated, this, &FilesForm::onFileActivated);

    retranslateUi();
    Translator::registerHandler(std::bind(&FilesForm::retranslateUi, this), this);
}

FilesForm::~FilesForm()
{
    Translator::unregister(this);
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
    QListWidgetItem* tmp = new QListWidgetItem(doneIcon, QFileInfo(path).fileName());
    tmp->setData(Qt::UserRole, path);
    recvd->addItem(tmp);
}

void FilesForm::onFileUploadComplete(const QString& path)
{
    QListWidgetItem* tmp = new QListWidgetItem(doneIcon, QFileInfo(path).fileName());
    tmp->setData(Qt::UserRole, path);
    sent->addItem(tmp);
}

// sadly, the ToxFile struct in core only has the file name, not the file path...
// so currently, these don't work as intended (though for now, downloads might work
// whenever they're not saved anywhere custom, thanks to the hack)
// I could do some digging around, but for now I'm tired and others already
// might know it without me needing to dig, so...
void FilesForm::onFileActivated(QListWidgetItem *item)
{
    Widget::confirmExecutableOpen(QFileInfo(item->data(Qt::UserRole).toString()));
}

void FilesForm::retranslateUi()
{
    headLabel.setText(tr("Transfered Files","\"Headline\" of the window"));
    main.setTabText(0, tr("Downloads"));
    main.setTabText(1, tr("Uploads"));
}
