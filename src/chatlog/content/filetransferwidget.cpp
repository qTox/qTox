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

#include "filetransferwidget.h"
#include "ui_filetransferwidget.h"

#include "src/nexus.h"
#include "src/core/core.h"
#include "src/widget/gui.h"
#include "src/widget/style.h"
#include "src/widget/widget.h"
#include "src/persistence/settings.h"

#include <QMouseEvent>
#include <QFileDialog>
#include <QFile>
#include <QBuffer>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QPainter>
#include <QVariantAnimation>
#include <QDebug>

#include <math.h>

FileTransferWidget::FileTransferWidget(QWidget *parent, ToxFile file)
    : QWidget(parent)
    , ui(new Ui::FileTransferWidget)
    , fileInfo(file)
    , lastTick(QTime::currentTime())
    , backgroundColor(Style::getColor(Style::LightGrey))
    , buttonColor(Style::getColor(Style::Yellow))
    , active(true)
{
    ui->setupUi(this);

    // hide the QWidget background (background-color: transparent doesn't seem to work)
    setAttribute(Qt::WA_TranslucentBackground, true);

    ui->previewButton->hide();
    ui->filenameLabel->setText(file.fileName);
    ui->progressBar->setValue(0);
    ui->fileSizeLabel->setText(getHumanReadableSize(file.filesize));
    ui->etaLabel->setText("");

    backgroundColorAnimation = new QVariantAnimation(this);
    backgroundColorAnimation->setDuration(500);
    backgroundColorAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(backgroundColorAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
        backgroundColor = val.value<QColor>();
        update();
    });

    buttonColorAnimation = new QVariantAnimation(this);
    buttonColorAnimation->setDuration(500);
    buttonColorAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(buttonColorAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
        buttonColor = val.value<QColor>();
        update();
    });

    setBackgroundColor(Style::getColor(Style::LightGrey), false);

    connect(Core::getInstance(), &Core::fileTransferInfo, this, &FileTransferWidget::onFileTransferInfo);
    connect(Core::getInstance(), &Core::fileTransferAccepted, this, &FileTransferWidget::onFileTransferAccepted);
    connect(Core::getInstance(), &Core::fileTransferCancelled, this, &FileTransferWidget::onFileTransferCancelled);
    connect(Core::getInstance(), &Core::fileTransferPaused, this, &FileTransferWidget::onFileTransferPaused);
    connect(Core::getInstance(), &Core::fileTransferFinished, this, &FileTransferWidget::onFileTransferFinished);
    connect(Core::getInstance(), &Core::fileTransferRemotePausedUnpaused, this, &FileTransferWidget::fileTransferRemotePausedUnpaused);
    connect(Core::getInstance(), &Core::fileTransferBrokenUnbroken, this, &FileTransferWidget::fileTransferBrokenUnbroken);
    connect(ui->topButton, &QPushButton::clicked, this, &FileTransferWidget::onTopButtonClicked);
    connect(ui->bottomButton, &QPushButton::clicked, this, &FileTransferWidget::onBottomButtonClicked);
    connect(ui->previewButton, &QPushButton::clicked, this, &FileTransferWidget::onPreviewButtonClicked);

    setupButtons();

    //preview
    if (fileInfo.direction == ToxFile::SENDING)
    {
        showPreview(fileInfo.filePath);
        ui->progressLabel->setText(tr("Waiting to send...", "file transfer widget"));
    }
    else
    {
        ui->progressLabel->setText(tr("Accept to receive this file", "file transfer widget"));
    }

    setFixedHeight(64);
}

FileTransferWidget::~FileTransferWidget()
{
    delete ui;
}

void FileTransferWidget::autoAcceptTransfer(const QString &path)
{
    QString filepath;
    int number = 0;

    QString suffix = QFileInfo(fileInfo.fileName).completeSuffix();
    QString base = QFileInfo(fileInfo.fileName).baseName();

    do
    {
        filepath = QString("%1/%2%3.%4").arg(path, base, number > 0 ? QString(" (%1)").arg(QString::number(number)) : QString(), suffix);
        number++;
    }
    while (QFileInfo(filepath).exists());

    //Do not automatically accept the file-transfer if the path is not writable.
    //The user can still accept it manually.
    if (Nexus::tryRemoveFile(filepath))
        Core::getInstance()->acceptFileRecvRequest(fileInfo.friendId, fileInfo.fileNum, filepath);
    else
        qWarning() << "Cannot write to " << filepath;
}

bool FileTransferWidget::isActive() const
{
    return active;
}

void FileTransferWidget::acceptTransfer(const QString &filepath)
{
    if (filepath.isEmpty())
        return;

    //test if writable
    if (!Nexus::tryRemoveFile(filepath))
    {
        GUI::showWarning(tr("Location not writable", "Title of permissions popup"),
                         tr("You do not have permission to write that location. Choose another, or cancel the save dialog.", "text of permissions popup"));
        return;
    }

    //everything ok!
    Core::getInstance()->acceptFileRecvRequest(fileInfo.friendId, fileInfo.fileNum, filepath);
}

void FileTransferWidget::setBackgroundColor(const QColor &c, bool whiteFont)
{
    if (c != backgroundColor)
    {
        backgroundColorAnimation->setStartValue(backgroundColor);
        backgroundColorAnimation->setEndValue(c);
        backgroundColorAnimation->start();
    }

    setProperty("fontColor", whiteFont ? "white" : "black");

    setStyleSheet(Style::getStylesheet(":/ui/fileTransferInstance/filetransferWidget.css"));
    Style::repolish(this);

    update();
}

void FileTransferWidget::setButtonColor(const QColor &c)
{
    if (c != buttonColor)
    {
        buttonColorAnimation->setStartValue(buttonColor);
        buttonColorAnimation->setEndValue(c);
        buttonColorAnimation->start();
    }
}

bool FileTransferWidget::drawButtonAreaNeeded() const
{
    return (ui->bottomButton->isVisible() || ui->topButton->isVisible()) &&
            !(ui->topButton->isVisible() && ui->topButton->objectName() == "ok");
}

void FileTransferWidget::paintEvent(QPaintEvent *)
{
    // required by Hi-DPI support as border-image doesn't work.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    qreal ratio = static_cast<qreal>(geometry().height()) / static_cast<qreal>(geometry().width());
    const int r = 24;
    const int buttonFieldWidth = 34;
    const int lineWidth = 1;

    // draw background
    if (drawButtonAreaNeeded())
        painter.setClipRect(QRect(0, 0, width()-buttonFieldWidth, height()));

    painter.setBrush(QBrush(backgroundColor));
    painter.drawRoundRect(geometry(), r * ratio, r);

    if (drawButtonAreaNeeded())
    {
        // draw button background (top)
        painter.setBrush(QBrush(buttonColor));
        painter.setClipRect(QRect(width()-buttonFieldWidth+lineWidth,0,buttonFieldWidth,height()/2-ceil(lineWidth/2.0)));
        painter.drawRoundRect(geometry(), r * ratio, r);

        // draw button background (bottom)
        painter.setBrush(QBrush(buttonColor));
        painter.setClipRect(QRect(width()-buttonFieldWidth+lineWidth,height()/2+lineWidth/2,buttonFieldWidth,height()/2));
        painter.drawRoundRect(geometry(), r * ratio, r);
    }
}

void FileTransferWidget::onFileTransferInfo(ToxFile file)
{
    QTime now = QTime::currentTime();
    qint64 dt = lastTick.msecsTo(now); //ms

    if (fileInfo != file || dt < 1000)
        return;

    fileInfo = file;

    if (fileInfo.status == ToxFile::TRANSMITTING)
    {
        // update progress
        qreal progress = static_cast<qreal>(file.bytesSent) / static_cast<qreal>(file.filesize);
        ui->progressBar->setValue(static_cast<int>(progress * 100.0));

        // ETA, speed
        qreal deltaSecs = dt / 1000.0;

        // (can't use ::abs or ::max on unsigned types substraction, they'd just overflow)
        quint64 deltaBytes = file.bytesSent > lastBytesSent
                                    ? file.bytesSent - lastBytesSent
                                    : lastBytesSent - file.bytesSent;
        qreal bytesPerSec = static_cast<int>(static_cast<qreal>(deltaBytes) / deltaSecs);

        // calculate mean
        meanIndex = meanIndex % TRANSFER_ROLLING_AVG_COUNT;
        meanData[meanIndex++] = bytesPerSec;

        qreal meanBytesPerSec = 0.0;
        for (size_t i = 0; i < TRANSFER_ROLLING_AVG_COUNT; ++i)
            meanBytesPerSec += meanData[i];

        meanBytesPerSec /= static_cast<qreal>(TRANSFER_ROLLING_AVG_COUNT);

        // update UI
        if (meanBytesPerSec > 0)
        {
            // ETA
            QTime toGo = QTime(0,0).addSecs((file.filesize - file.bytesSent) / meanBytesPerSec);
            QString format = toGo.hour() > 0 ? "hh:mm:ss" : "mm:ss";
            ui->etaLabel->setText(toGo.toString(format));
        }
        else
        {
            ui->etaLabel->setText("");
        }

        ui->progressLabel->setText(getHumanReadableSize(meanBytesPerSec) + "/s");

        lastBytesSent = file.bytesSent;
    }

    lastTick = now;

    // trigger repaint
    update();
}

void FileTransferWidget::onFileTransferAccepted(ToxFile file)
{
    if (fileInfo != file)
        return;

    fileInfo = file;

    setBackgroundColor(Style::getColor(Style::LightGrey), false);

    setupButtons();
}

void FileTransferWidget::onFileTransferCancelled(ToxFile file)
{
    if (fileInfo != file)
        return;

    fileInfo = file;
    active = false;

    setBackgroundColor(Style::getColor(Style::Red), true);

    setupButtons();
    hideWidgets();

    disconnect(Core::getInstance(), 0, this, 0);
}

void FileTransferWidget::onFileTransferPaused(ToxFile file)
{
    if (fileInfo != file)
        return;

    fileInfo = file;

    ui->etaLabel->setText("");
    ui->progressLabel->setText(tr("Paused", "file transfer widget"));

    // reset mean
    meanIndex = 0;
    for (size_t i=0; i<TRANSFER_ROLLING_AVG_COUNT; ++i)
        meanData[i] = 0.0;

    setBackgroundColor(Style::getColor(Style::LightGrey), false);

    setupButtons();
}

void FileTransferWidget::onFileTransferResumed(ToxFile file)
{
    if (fileInfo != file)
        return;

    fileInfo = file;

    ui->etaLabel->setText("");
    ui->progressLabel->setText(tr("Resuming...", "file transfer widget"));

    // reset mean
    meanIndex = 0;
    for (size_t i=0; i<TRANSFER_ROLLING_AVG_COUNT; ++i)
        meanData[i] = 0.0;

    setBackgroundColor(Style::getColor(Style::LightGrey), false);

    setupButtons();
}

void FileTransferWidget::onFileTransferFinished(ToxFile file)
{
    if (fileInfo != file)
        return;

    fileInfo = file;
    active = false;

    setBackgroundColor(Style::getColor(Style::Green), true);

    setupButtons();
    hideWidgets();

    ui->topButton->setIcon(QIcon(":/ui/fileTransferInstance/yes.svg"));
    ui->topButton->setObjectName("ok");
    ui->topButton->setToolTip(tr("Open file"));
    ui->topButton->show();

    ui->bottomButton->setIcon(QIcon(":/ui/fileTransferInstance/dir.svg"));
    ui->bottomButton->setObjectName("dir");
    ui->bottomButton->setToolTip(tr("Open file directory"));
    ui->bottomButton->show();

    // preview
    if (fileInfo.direction == ToxFile::RECEIVING)
        showPreview(fileInfo.filePath);

    disconnect(Core::getInstance(), 0, this, 0);
}

void FileTransferWidget::fileTransferRemotePausedUnpaused(ToxFile file, bool paused)
{
    if (paused)
        onFileTransferPaused(file);
    else
        onFileTransferResumed(file);
}

void FileTransferWidget::fileTransferBrokenUnbroken(ToxFile file, bool broken)
{
    /// TODO: Handle broken transfer differently once we have resuming code
    if (broken)
        onFileTransferCancelled(file);
}

QString FileTransferWidget::getHumanReadableSize(qint64 size)
{
    static const char* suffix[] = {"B","kiB","MiB","GiB","TiB"};
    int exp = 0;

    if (size > 0)
        exp = std::min( (int) (log(size) / log(1024)), (int) (sizeof(suffix) / sizeof(suffix[0]) - 1));

    return QString().setNum(size / pow(1024, exp), 'f', exp > 1 ? 2 : 0).append(suffix[exp]);
}

void FileTransferWidget::hideWidgets()
{
    ui->topButton->hide();
    ui->bottomButton->hide();
    ui->progressBar->hide();
    ui->progressLabel->hide();
    ui->etaLabel->hide();
}

void FileTransferWidget::setupButtons()
{
    switch(fileInfo.status)
    {
    case ToxFile::TRANSMITTING:
        ui->topButton->setIcon(QIcon(":/ui/fileTransferInstance/pause.svg"));
        ui->topButton->setObjectName("pause");
        ui->topButton->setToolTip(tr("Pause transfer"));

        ui->bottomButton->setIcon(QIcon(":/ui/fileTransferInstance/no.svg"));
        ui->bottomButton->setObjectName("cancel");
        ui->bottomButton->setToolTip(tr("Cancel transfer"));

        setButtonColor(Style::getColor(Style::Green));
        break;

    case ToxFile::PAUSED:
        ui->topButton->setIcon(QIcon(":/ui/fileTransferInstance/arrow_white.svg"));
        ui->topButton->setObjectName("resume");
        ui->topButton->setToolTip(tr("Resume transfer"));

        ui->bottomButton->setIcon(QIcon(":/ui/fileTransferInstance/no.svg"));
        ui->bottomButton->setObjectName("cancel");
        ui->bottomButton->setToolTip(tr("Cancel transfer"));

        setButtonColor(Style::getColor(Style::LightGrey));
        break;

    case ToxFile::STOPPED:
    case ToxFile::BROKEN:
        ui->bottomButton->setIcon(QIcon(":/ui/fileTransferInstance/no.svg"));
        ui->bottomButton->setObjectName("cancel");
        ui->bottomButton->setToolTip(tr("Cancel transfer"));

        if (fileInfo.direction == ToxFile::SENDING)
        {
            ui->topButton->setIcon(QIcon(":/ui/fileTransferInstance/pause.svg"));
            ui->topButton->setObjectName("pause");
            ui->topButton->setToolTip(tr("Pause transfer"));
        }
        else
        {
            ui->topButton->setIcon(QIcon(":/ui/fileTransferInstance/yes.svg"));
            ui->topButton->setObjectName("accept");
            ui->topButton->setToolTip(tr("Accept transfer"));
        }
        break;
    }
}

void FileTransferWidget::handleButton(QPushButton *btn)
{
    if (fileInfo.direction == ToxFile::SENDING)
    {
        if (btn->objectName() == "cancel")
            Core::getInstance()->cancelFileSend(fileInfo.friendId, fileInfo.fileNum);
        else if (btn->objectName() == "pause")
            Core::getInstance()->pauseResumeFileSend(fileInfo.friendId, fileInfo.fileNum);
        else if (btn->objectName() == "resume")
            Core::getInstance()->pauseResumeFileSend(fileInfo.friendId, fileInfo.fileNum);
    }
    else // receiving or paused
    {
        if (btn->objectName() == "cancel")
            Core::getInstance()->cancelFileRecv(fileInfo.friendId, fileInfo.fileNum);
        else if (btn->objectName() == "pause")
            Core::getInstance()->pauseResumeFileRecv(fileInfo.friendId, fileInfo.fileNum);
        else if (btn->objectName() == "resume")
            Core::getInstance()->pauseResumeFileRecv(fileInfo.friendId, fileInfo.fileNum);
        else if (btn->objectName() == "accept")
        {
            QString path = QFileDialog::getSaveFileName(parentWidget(),
                                                        tr("Save a file", "Title of the file saving dialog"),
                                                        Settings::getInstance().getGlobalAutoAcceptDir() + "/" + fileInfo.fileName,
                                                        0,
                                                        0,
                                                        QFileDialog::DontUseNativeDialog);
            acceptTransfer(path);
        }
    }

    if (btn->objectName() == "ok" || btn->objectName() == "previewButton")
    {
        Widget::confirmExecutableOpen(QFileInfo(fileInfo.filePath));
    }
    else if (btn->objectName() == "dir")
    {
        QString dirPath = QFileInfo(fileInfo.filePath).dir().path();
        QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
    }

}

void FileTransferWidget::showPreview(const QString &filename)
{
    static const QStringList previewExtensions = { "png", "jpeg", "jpg", "gif", "svg",
                                                   "PNG", "JPEG", "JPG", "GIF", "SVG" };

    if (previewExtensions.contains(QFileInfo(filename).suffix()))
    {
        // Subtract to make border visible
        const int size = qMax(ui->previewButton->width(), ui->previewButton->height()) - 4;

        const QImage image = QImage(filename);
        const QPixmap iconPixmap = scaleCropIntoSquare(QPixmap::fromImage(image), size);

        ui->previewButton->setIcon(QIcon(iconPixmap));
        ui->previewButton->setIconSize(iconPixmap.size());
        ui->previewButton->show();
        // Show mouseover preview, but make sure it's not larger than 50% of the screen width/height
        const QRect desktopSize = QApplication::desktop()->screenGeometry();
        const QImage previewImage = image.scaled(0.5 * desktopSize.width(),
                                               0.5 * desktopSize.height(),
                                               Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QByteArray imageData;
        QBuffer buffer(&imageData);
        buffer.open(QIODevice::WriteOnly);
        previewImage.save(&buffer, "PNG");
        buffer.close();
        ui->previewButton->setToolTip("<img src=data:image/png;base64," + imageData.toBase64() + "/>");
    }
}

void FileTransferWidget::onTopButtonClicked()
{
    handleButton(ui->topButton);
}

void FileTransferWidget::onBottomButtonClicked()
{
    handleButton(ui->bottomButton);
}

void FileTransferWidget::onPreviewButtonClicked()
{
    handleButton(ui->previewButton);
}

QPixmap FileTransferWidget::scaleCropIntoSquare(const QPixmap &source, const int targetSize)
{
    QPixmap result;

    // Make sure smaller-than-icon images (at least one dimension is smaller) will not be upscaled
    if (source.width() < targetSize || source.height() < targetSize)
    {
        result = source;
    }
    else
    {
        result = source.scaled(targetSize, targetSize,
                               Qt::KeepAspectRatioByExpanding,
                               Qt::SmoothTransformation);
    }

    // Then, image has to be cropped (if needed) so it will not overflow rectangle
    // Only one dimension will be bigger after Qt::KeepAspectRatioByExpanding
    if (result.width() > targetSize)
        return result.copy((result.width() - targetSize) / 2, 0, targetSize, targetSize);
    else if (result.height() > targetSize)
        return result.copy(0, (result.height() - targetSize) / 2, targetSize, targetSize);

    // Picture was rectangle in the first place, no cropping
    return result;
}
