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

#ifndef FILETRANSFERWIDGET_H
#define FILETRANSFERWIDGET_H

#include <QWidget>
#include <QTime>
#include "../chatlinecontent.h"
#include "../../corestructs.h"

namespace Ui {
class FileTransferWidget;
}

class QPushButton;

class FileTransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileTransferWidget(QWidget *parent, ToxFile file);
    virtual ~FileTransferWidget();

    void acceptTransfer(const QString& path);

protected slots:
    void onFileTransferInfo(ToxFile file);
    void onFileTransferAccepted(ToxFile file);
    void onFileTransferCancelled(ToxFile file);
    void onFileTransferPaused(ToxFile file);
    void onFileTransferFinished(ToxFile file);

protected:
    QString getHumanReadableSize(qint64 size);
    void hideWidgets();
    void setupButtons();
    void handleButton(QPushButton* btn);
    void showPreview(const QString& filename);

private slots:
    void on_topButton_clicked();
    void on_bottomButton_clicked();

private:
    Ui::FileTransferWidget *ui;
    ToxFile fileInfo;
    QTime lastTick;
    qint64 lastBytesSent = 0;

};

#endif // FILETRANSFERWIDGET_H
