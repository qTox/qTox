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

#include "filetransferwidget.h"
#include "ui_filetransferwidget.h"

#include <QMouseEvent>
#include <QDebug>

FileTransferWidget::FileTransferWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileTransferWidget)
{
    ui->setupUi(this);

    setFixedHeight(100);
}

FileTransferWidget::~FileTransferWidget()
{
    delete ui;
}

void FileTransferWidget::on_pushButton_2_clicked()
{
    qDebug() << "Button Cancel Clicked";
}

void FileTransferWidget::on_pushButton_clicked()
{
    qDebug() << "Button Resume Clicked";
}

void FileTransferWidget::on_pushButton_2_pressed()
{
    qDebug() << "Button Resume Clicked";
}
