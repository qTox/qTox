/*
    Copyright © 2017-2019 by The qTox Project Contributors

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

#include <QWidget>

#include "src/core/extension.h"

#include <memory>

class MaskablePixmapWidget;
class QVBoxLayout;
class QHBoxLayout;
class CroppingLabel;
class QPushButton;
class QToolButton;
class CallConfirmWidget;
class QLabel;
class ExtensionStatus;
class Settings;
class Style;

class ChatFormHeader : public QWidget
{
    Q_OBJECT
public:
    enum class CallButtonState {
        Disabled = 0,    // Grey
        Avaliable = 1,   // Green
        InCall = 2,      // Red
        Outgoing = 3,    // Yellow
        Incoming = 4,    // Yellow
    };
    enum class ToolButtonState {
        Disabled = 0,    // Grey
        Off = 1,         // Green
        On = 2,          // Red
    };
    enum Mode {
        None = 0,
        Audio = 1,
        Video = 2,
        AV = Audio | Video
    };

    ChatFormHeader(Settings& settings, Style& style, QWidget* parent = nullptr);
    ~ChatFormHeader();

    void setName(const QString& newName);
    void setMode(Mode mode_);

    void showOutgoingCall(bool video);
    void createCallConfirm(bool video);
    void showCallConfirm();
    void removeCallConfirm();

    void updateExtensionSupport(ExtensionSet extensions);
    void updateCallButtons(bool online, bool audio, bool video = false);
    void updateMuteMicButton(bool active, bool inputMuted);
    void updateMuteVolButton(bool active, bool outputMuted);

    void setAvatar(const QPixmap& img);
    QSize getAvatarSize() const;

    // TODO: Remove
    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
    void addLayout(QLayout* layout);
    void addStretch();

public slots:
    void reloadTheme();

signals:
    void callTriggered();
    void videoCallTriggered();
    void micMuteToggle();
    void volMuteToggle();

    void nameChanged(const QString& name);

    void callAccepted();
    void callRejected();

private slots:
    void retranslateUi();
    void updateButtonsView();

private:
    Mode mode;
    MaskablePixmapWidget* avatar;
    QVBoxLayout* headTextLayout;
    QHBoxLayout* nameLine;
    ExtensionStatus* extensionStatus;
    CroppingLabel* nameLabel;

    QPushButton* callButton;
    QPushButton* videoButton;
    QPushButton* volButton;
    QPushButton* micButton;

    CallButtonState callState;
    CallButtonState videoState;
    ToolButtonState volState;
    ToolButtonState micState;

    std::unique_ptr<CallConfirmWidget> callConfirm;
    Settings& settings;
    Style& style;
};
