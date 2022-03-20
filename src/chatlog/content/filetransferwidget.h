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

#include <QTime>
#include <QWidget>

#include "src/chatlog/chatlinecontent.h"
#include "src/core/toxfile.h"

class CoreFile;

namespace Ui {
class FileTransferWidget;
}

class QVariantAnimation;
class QPushButton;
class Settings;
class Style;
class IMessageBoxManager;

class FileTransferWidget : public QWidget
{
    Q_OBJECT

public:
    FileTransferWidget(QWidget* parent, CoreFile& _coreFile, ToxFile file,
        Settings& settings, Style& style, IMessageBoxManager& messageBoxManager);
    virtual ~FileTransferWidget();
    bool isActive() const;
    void onFileTransferUpdate(ToxFile file);

protected:
    void updateWidgetColor(ToxFile const& file);
    void updateWidgetText(ToxFile const& file);
    void updateFileProgress(ToxFile const& file);
    void updateSignals(ToxFile const& file);
    void updatePreview(ToxFile const& file);
    void setupButtons(ToxFile const& file);
    void handleButton(QPushButton* btn);
    void showPreview(const QString& filename);
    void acceptTransfer(const QString& filepath);
    void setBackgroundColor(const QColor& c, bool whiteFont);
    void setButtonColor(const QColor& c);

    bool drawButtonAreaNeeded() const;

    void paintEvent(QPaintEvent* event) final;

public slots:
    void reloadTheme();

private slots:
    void onLeftButtonClicked();
    void onRightButtonClicked();
    void onPreviewButtonClicked();

private:
    static bool tryRemoveFile(const QString &filepath);

    void updateWidget(ToxFile const& file);
    void updateBackgroundColor(const ToxFile::FileStatus status);

private:
    CoreFile& coreFile;
    Ui::FileTransferWidget* ui;
    ToxFile fileInfo;
    QVariantAnimation* backgroundColorAnimation = nullptr;
    QVariantAnimation* buttonColorAnimation = nullptr;
    QColor backgroundColor;
    QColor buttonColor;
    QColor buttonBackgroundColor;

    bool active;
    QTime lastTransmissionUpdate;
    ToxFile::FileStatus lastStatus = ToxFile::INITIALIZING;
    Settings& settings;
    Style& style;
    IMessageBoxManager& messageBoxManager;
};
