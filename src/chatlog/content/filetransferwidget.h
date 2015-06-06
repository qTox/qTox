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

#ifndef FILETRANSFERWIDGET_H
#define FILETRANSFERWIDGET_H

#include <QWidget>
#include <QTime>

#include "src/chatlog/chatlinecontent.h"
#include "src/core/corestructs.h"


namespace Ui {
class FileTransferWidget;
}

class QVariantAnimation;
class QPushButton;

class FileTransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileTransferWidget(QWidget *parent, ToxFile file);
    virtual ~FileTransferWidget();
    void autoAcceptTransfer(const QString& path);

protected slots:
    void onFileTransferInfo(ToxFile file);
    void onFileTransferAccepted(ToxFile file);
    void onFileTransferCancelled(ToxFile file);
    void onFileTransferPaused(ToxFile file);
    void onFileTransferResumed(ToxFile file);
    void onFileTransferFinished(ToxFile file);
    void fileTransferRemotePausedUnpaused(ToxFile file, bool paused);
    void fileTransferBrokenUnbroken(ToxFile file, bool broken);

protected:
    QString getHumanReadableSize(qint64 size);
    void hideWidgets();
    void setupButtons();
    void handleButton(QPushButton* btn);
    void showPreview(const QString& filename);
    void acceptTransfer(const QString& filepath);
    void setBackgroundColor(const QColor& c, bool whiteFont);
    void setButtonColor(const QColor& c);

    bool drawButtonAreaNeeded() const;

    virtual void paintEvent(QPaintEvent*);

private slots:
    void on_topButton_clicked();
    void on_bottomButton_clicked();

private:
    Ui::FileTransferWidget *ui;
    ToxFile fileInfo;
    QTime lastTick;
    quint64 lastBytesSent = 0;
    QVariantAnimation* backgroundColorAnimation = nullptr;
    QVariantAnimation* buttonColorAnimation = nullptr;
    QColor backgroundColor;
    QColor buttonColor;

    static const uint8_t TRANSFER_ROLLING_AVG_COUNT = 4;
    uint8_t meanIndex = 0;
    qreal meanData[TRANSFER_ROLLING_AVG_COUNT] = {0.0};
};

#endif // FILETRANSFERWIDGET_H
