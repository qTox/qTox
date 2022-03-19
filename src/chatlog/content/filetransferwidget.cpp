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

#include "filetransferwidget.h"
#include "ui_filetransferwidget.h"

#include "src/core/corefile.h"
#include "src/persistence/settings.h"
#include "src/widget/gui.h"
#include "src/widget/style.h"
#include "src/widget/widget.h"
#include "src/model/exiftransform.h"
#include "util/display.h"

#include <QBuffer>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QVariantAnimation>

#include <cassert>
#include <math.h>


// The leftButton is used to accept, pause, or resume a file transfer, as well as to open a
// received file.
// The rightButton is used to cancel a file transfer, or to open the directory a file was
// downloaded to.

FileTransferWidget::FileTransferWidget(QWidget* parent, CoreFile& _coreFile,
    ToxFile file, Settings& settings_, Style& style_)
    : QWidget(parent)
    , coreFile{_coreFile}
    , ui(new Ui::FileTransferWidget)
    , fileInfo(file)
    , backgroundColor(style_.getColor(Style::ColorPalette::TransferMiddle))
    , buttonColor(style_.getColor(Style::ColorPalette::TransferWait))
    , buttonBackgroundColor(style_.getColor(Style::ColorPalette::GroundBase))
    , active(true)
    , settings(settings_)
    , style{style_}
{
    ui->setupUi(this);

    // hide the QWidget background (background-color: transparent doesn't seem to work)
    setAttribute(Qt::WA_TranslucentBackground, true);

    ui->previewButton->hide();
    ui->filenameLabel->setText(file.fileName);
    ui->progressBar->setValue(0);
    ui->fileSizeLabel->setText(getHumanReadableSize(file.progress.getFileSize()));
    ui->etaLabel->setText("");

    backgroundColorAnimation = new QVariantAnimation(this);
    backgroundColorAnimation->setDuration(500);
    backgroundColorAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(backgroundColorAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& val) {
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

    connect(ui->leftButton, &QPushButton::clicked, this, &FileTransferWidget::onLeftButtonClicked);
    connect(ui->rightButton, &QPushButton::clicked, this, &FileTransferWidget::onRightButtonClicked);
    connect(ui->previewButton, &QPushButton::clicked, this,
            &FileTransferWidget::onPreviewButtonClicked);

    connect(&GUI::getInstance(), &GUI::themeReload, this, &FileTransferWidget::reloadTheme);

    // Set lastStatus to anything but the file's current value, this forces an update
    lastStatus = file.status == ToxFile::FINISHED ? ToxFile::INITIALIZING : ToxFile::FINISHED;
    updateWidget(file);

    setFixedHeight(64);
}

FileTransferWidget::~FileTransferWidget()
{
    delete ui;
}

// TODO(sudden6): remove file IO from the UI
/**
 * @brief Dangerous way to find out if a path is writable.
 * @param filepath Path to file which should be deleted.
 * @return True, if file writeable, false otherwise.
 */
bool FileTransferWidget::tryRemoveFile(const QString& filepath)
{
    QFile tmp(filepath);
    bool writable = tmp.open(QIODevice::WriteOnly);
    tmp.remove();
    return writable;
}

void FileTransferWidget::onFileTransferUpdate(ToxFile file)
{
    updateWidget(file);
}

bool FileTransferWidget::isActive() const
{
    return active;
}

void FileTransferWidget::acceptTransfer(const QString& filepath)
{
    if (filepath.isEmpty()) {
        return;
    }

    // test if writable
    if (!tryRemoveFile(filepath)) {
        GUI::showWarning(tr("Location not writable", "Title of permissions popup"),
                         tr("You do not have permission to write that location. Choose another, or "
                            "cancel the save dialog.",
                            "text of permissions popup"));
        return;
    }

    // everything ok!
    coreFile.acceptFileRecvRequest(fileInfo.friendId, fileInfo.fileNum, filepath);
}

void FileTransferWidget::setBackgroundColor(const QColor& c, bool whiteFont)
{
    if (c != backgroundColor) {
        backgroundColorAnimation->setStartValue(backgroundColor);
        backgroundColorAnimation->setEndValue(c);
        backgroundColorAnimation->start();
    }

    setProperty("fontColor", whiteFont ? "white" : "black");

    setStyleSheet(style.getStylesheet("fileTransferInstance/filetransferWidget.css", settings));
    Style::repolish(this);

    update();
}

void FileTransferWidget::setButtonColor(const QColor& c)
{
    if (c != buttonColor) {
        buttonColorAnimation->setStartValue(buttonColor);
        buttonColorAnimation->setEndValue(c);
        buttonColorAnimation->start();
    }
}

bool FileTransferWidget::drawButtonAreaNeeded() const
{
    return (ui->rightButton->isVisible() || ui->leftButton->isVisible())
           && !(ui->leftButton->isVisible() && ui->leftButton->objectName() == "ok");
}

void FileTransferWidget::paintEvent(QPaintEvent* event)
{
    std::ignore = event;
    // required by Hi-DPI support as border-image doesn't work.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    qreal ratio = static_cast<qreal>(geometry().height()) / static_cast<qreal>(geometry().width());
    const int r = 24;
    const int buttonFieldWidth = 32;
    const int lineWidth = 1;

    // Draw the widget background:
    painter.setClipRect(QRect(0, 0, width(), height()));
    painter.setBrush(QBrush(backgroundColor));
    painter.drawRoundedRect(geometry(), r * ratio, r, Qt::RelativeSize);

    if (drawButtonAreaNeeded()) {
        // Draw the button background:
        QPainterPath buttonBackground;
        buttonBackground.addRoundedRect(width() - 2 * buttonFieldWidth - lineWidth * 2, 0,
                                      buttonFieldWidth, buttonFieldWidth + lineWidth, 50, 50,
                                      Qt::RelativeSize);
        buttonBackground.addRect(width() - 2 * buttonFieldWidth - lineWidth * 2, 0,
                                 buttonFieldWidth * 2, buttonFieldWidth / 2);
        buttonBackground.addRect(width() - 1.5 * buttonFieldWidth - lineWidth * 2, 0,
                                 buttonFieldWidth * 2, buttonFieldWidth + 1);
        buttonBackground.setFillRule(Qt::WindingFill);
        painter.setBrush(QBrush(buttonBackgroundColor));
        painter.drawPath(buttonBackground);

        // Draw the left button:
        QPainterPath leftButton;
        leftButton.addRoundedRect(QRect(width() - 2 * buttonFieldWidth - lineWidth, 0,
                                      buttonFieldWidth, buttonFieldWidth),
                                50, 50, Qt::RelativeSize);
        leftButton.addRect(QRect(width() - 2 * buttonFieldWidth - lineWidth, 0,
                                 buttonFieldWidth / 2, buttonFieldWidth / 2));
        leftButton.addRect(QRect(width() - 1.5 * buttonFieldWidth - lineWidth, 0,
                                 buttonFieldWidth / 2, buttonFieldWidth));
        leftButton.setFillRule(Qt::WindingFill);
        painter.setBrush(QBrush(buttonColor));
        painter.drawPath(leftButton);

        // Draw the right button:
        painter.setBrush(QBrush(buttonColor));
        painter.setClipRect(QRect(width() - buttonFieldWidth, 0, buttonFieldWidth, buttonFieldWidth));
        painter.drawRoundedRect(geometry(), r * ratio, r, Qt::RelativeSize);
    }
}

void FileTransferWidget::reloadTheme()
{
    updateBackgroundColor(lastStatus);
}

void FileTransferWidget::updateWidgetColor(ToxFile const& file)
{
    if (lastStatus == file.status) {
        return;
    }

    updateBackgroundColor(file.status);
}

void FileTransferWidget::updateWidgetText(ToxFile const& file)
{
    if (lastStatus == file.status && file.status != ToxFile::PAUSED) {
        return;
    }

    switch (file.status) {
    case ToxFile::INITIALIZING:
        if (file.direction == ToxFile::SENDING) {
            ui->progressLabel->setText(tr("Waiting to send...", "file transfer widget"));
        } else {
            ui->progressLabel->setText(tr("Accept to receive this file", "file transfer widget"));
        }
        break;
    case ToxFile::PAUSED:
        ui->etaLabel->setText("");
        if (file.pauseStatus.localPaused()) {
            ui->progressLabel->setText(tr("Paused", "file transfer widget"));
        } else {
            ui->progressLabel->setText(tr("Remote paused", "file transfer widget"));
        }
        break;
    case ToxFile::TRANSMITTING:
        ui->etaLabel->setText("");
        ui->progressLabel->setText(tr("Resuming...", "file transfer widget"));
        break;
    case ToxFile::BROKEN:
    case ToxFile::CANCELED:
        break;
    case ToxFile::FINISHED:
        break;
    default:
        qCritical() << "Invalid file status";
        assert(false);
    }
}

void FileTransferWidget::updatePreview(ToxFile const& file)
{
    if (lastStatus == file.status) {
        return;
    }

    switch (file.status) {
    case ToxFile::INITIALIZING:
    case ToxFile::PAUSED:
    case ToxFile::TRANSMITTING:
    case ToxFile::BROKEN:
    case ToxFile::CANCELED:
        if (file.direction == ToxFile::SENDING) {
            showPreview(file.filePath);
        }
        break;
    case ToxFile::FINISHED:
        showPreview(file.filePath);
        break;
    default:
        qCritical() << "Invalid file status";
        assert(false);
    }
}

void FileTransferWidget::updateFileProgress(ToxFile const& file)
{
    switch (file.status) {
    case ToxFile::INITIALIZING:
    case ToxFile::PAUSED:
        break;
    case ToxFile::TRANSMITTING: {
        auto speed = file.progress.getSpeed();
        auto progress = file.progress.getProgress();
        auto remainingTime = file.progress.getTimeLeftSeconds();

        ui->progressBar->setValue(static_cast<int>(progress * 100.0));

        // update UI
        if (speed > 0) {
            // ETA
            QTime toGo = QTime(0, 0).addSecs(remainingTime);
            QString format = toGo.hour() > 0 ? "hh:mm:ss" : "mm:ss";
            ui->etaLabel->setText(toGo.toString(format));
        } else {
            ui->etaLabel->setText("");
        }

        ui->progressLabel->setText(getHumanReadableSize(speed) + "/s");
        break;
    }
    case ToxFile::BROKEN:
    case ToxFile::CANCELED:
    case ToxFile::FINISHED: {
        ui->progressBar->hide();
        ui->progressLabel->hide();
        ui->etaLabel->hide();
        break;
    }
    default:
        qCritical() << "Invalid file status";
        assert(false);
    }
}

void FileTransferWidget::updateSignals(ToxFile const& file)
{
    if (lastStatus == file.status) {
        return;
    }

    switch (file.status) {
    case ToxFile::CANCELED:
    case ToxFile::BROKEN:
    case ToxFile::FINISHED:
        active = false;
        disconnect(&coreFile, nullptr, this, nullptr);
        break;
    case ToxFile::INITIALIZING:
    case ToxFile::PAUSED:
    case ToxFile::TRANSMITTING:
        break;
    default:
        qCritical() << "Invalid file status";
        assert(false);
    }
}

void FileTransferWidget::setupButtons(ToxFile const& file)
{
    if (lastStatus == file.status && file.status != ToxFile::PAUSED) {
        return;
    }

    switch (file.status) {
    case ToxFile::TRANSMITTING:
        ui->leftButton->setIcon(QIcon(style.getImagePath("fileTransferInstance/pause.svg", settings)));
        ui->leftButton->setObjectName("pause");
        ui->leftButton->setToolTip(tr("Pause transfer"));

        ui->rightButton->setIcon(QIcon(style.getImagePath("fileTransferInstance/no.svg", settings)));
        ui->rightButton->setObjectName("cancel");
        ui->rightButton->setToolTip(tr("Cancel transfer"));

        setButtonColor(style.getColor(Style::ColorPalette::TransferGood));
        break;

    case ToxFile::PAUSED:
        if (file.pauseStatus.localPaused()) {
            ui->leftButton->setIcon(QIcon(style.getImagePath("fileTransferInstance/arrow_white.svg", settings)));
            ui->leftButton->setObjectName("resume");
            ui->leftButton->setToolTip(tr("Resume transfer"));
        } else {
            ui->leftButton->setIcon(QIcon(style.getImagePath("fileTransferInstance/pause.svg", settings)));
            ui->leftButton->setObjectName("pause");
            ui->leftButton->setToolTip(tr("Pause transfer"));
        }

        ui->rightButton->setIcon(QIcon(style.getImagePath("fileTransferInstance/no.svg", settings)));
        ui->rightButton->setObjectName("cancel");
        ui->rightButton->setToolTip(tr("Cancel transfer"));

        setButtonColor(style.getColor(Style::ColorPalette::TransferMiddle));
        break;

    case ToxFile::INITIALIZING:
        ui->rightButton->setIcon(QIcon(style.getImagePath("fileTransferInstance/no.svg", settings)));
        ui->rightButton->setObjectName("cancel");
        ui->rightButton->setToolTip(tr("Cancel transfer"));

        if (file.direction == ToxFile::SENDING) {
            ui->leftButton->setIcon(QIcon(style.getImagePath("fileTransferInstance/pause.svg", settings)));
            ui->leftButton->setObjectName("pause");
            ui->leftButton->setToolTip(tr("Pause transfer"));
        } else {
            ui->leftButton->setIcon(QIcon(style.getImagePath("fileTransferInstance/yes.svg", settings)));
            ui->leftButton->setObjectName("accept");
            ui->leftButton->setToolTip(tr("Accept transfer"));
        }
        break;
    case ToxFile::CANCELED:
    case ToxFile::BROKEN:
        ui->leftButton->hide();
        ui->rightButton->hide();
        break;
    case ToxFile::FINISHED:
        ui->leftButton->setIcon(QIcon(style.getImagePath("fileTransferInstance/yes.svg", settings)));
        ui->leftButton->setObjectName("ok");
        ui->leftButton->setToolTip(tr("Open file"));
        ui->leftButton->show();

        ui->rightButton->setIcon(QIcon(style.getImagePath("fileTransferInstance/dir.svg", settings)));
        ui->rightButton->setObjectName("dir");
        ui->rightButton->setToolTip(tr("Open file directory"));
        ui->rightButton->show();

        break;
    default:
        qCritical() << "Invalid file status";
        assert(false);
    }
}

void FileTransferWidget::handleButton(QPushButton* btn)
{
    if (fileInfo.direction == ToxFile::SENDING) {
        if (btn->objectName() == "cancel") {
            coreFile.cancelFileSend(fileInfo.friendId, fileInfo.fileNum);
        } else if (btn->objectName() == "pause") {
            coreFile.pauseResumeFile(fileInfo.friendId, fileInfo.fileNum);
        } else if (btn->objectName() == "resume") {
            coreFile.pauseResumeFile(fileInfo.friendId, fileInfo.fileNum);
        }
    } else // receiving or paused
    {
        if (btn->objectName() == "cancel") {
            coreFile.cancelFileRecv(fileInfo.friendId, fileInfo.fileNum);
        } else if (btn->objectName() == "pause") {
            coreFile.pauseResumeFile(fileInfo.friendId, fileInfo.fileNum);
        } else if (btn->objectName() == "resume") {
            coreFile.pauseResumeFile(fileInfo.friendId, fileInfo.fileNum);
        } else if (btn->objectName() == "accept") {
            QString path =
                QFileDialog::getSaveFileName(Q_NULLPTR,
                                             tr("Save a file", "Title of the file saving dialog"),
                                             settings.getGlobalAutoAcceptDir() + "/"
                                                 + fileInfo.fileName);
            acceptTransfer(path);
        }
    }

    if (btn->objectName() == "ok" || btn->objectName() == "previewButton") {
        Widget::confirmExecutableOpen(QFileInfo(fileInfo.filePath));
    } else if (btn->objectName() == "dir") {
        QString dirPath = QFileInfo(fileInfo.filePath).dir().path();
        QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
    }
}

void FileTransferWidget::showPreview(const QString& filename)
{
    ui->previewButton->setIconFromFile(filename);
    ui->previewButton->show();
}

void FileTransferWidget::onLeftButtonClicked()
{
    handleButton(ui->leftButton);
}

void FileTransferWidget::onRightButtonClicked()
{
    handleButton(ui->rightButton);
}

void FileTransferWidget::onPreviewButtonClicked()
{
    handleButton(ui->previewButton);
}

void FileTransferWidget::updateWidget(ToxFile const& file)
{
    assert(file == fileInfo);

    fileInfo = file;

    bool shouldUpdateFileProgress = file.status != ToxFile::TRANSMITTING || lastTransmissionUpdate ==
        QTime() || lastTransmissionUpdate.msecsTo(file.progress.lastSampleTime()) > 1000;

    updatePreview(file);
    if (shouldUpdateFileProgress)
        updateFileProgress(file);
    updateWidgetText(file);
    updateWidgetColor(file);
    setupButtons(file);
    updateSignals(file);

    lastStatus = file.status;

    if (shouldUpdateFileProgress) {
        lastTransmissionUpdate = QTime::currentTime();
        update();
    }
}

void FileTransferWidget::updateBackgroundColor(const ToxFile::FileStatus status)
{
    switch (status) {
    case ToxFile::INITIALIZING:
    case ToxFile::PAUSED:
    case ToxFile::TRANSMITTING:
        setBackgroundColor(style.getColor(Style::ColorPalette::TransferMiddle), false);
        break;
    case ToxFile::BROKEN:
    case ToxFile::CANCELED:
        setBackgroundColor(style.getColor(Style::ColorPalette::TransferBad), true);
        break;
    case ToxFile::FINISHED:
        setBackgroundColor(style.getColor(Style::ColorPalette::TransferGood), true);
        break;
    default:
        qCritical() << "Invalid file status";
        assert(false);
    }
}
