/*
    Copyright © 2017 by The qTox Project Contributors

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

#ifndef CHAT_FORM_HEADER
#define CHAT_FORM_HEADER

#include <QPointer>
#include <QWidget>

class MaskablePixmapWidget;
class QVBoxLayout;
class CroppingLabel;
class QPushButton;
class QToolButton;
class CallConfirmWidget;

class ChatFormHeader : public QWidget
{
    Q_OBJECT
public:
    enum Mode {
        None = 0,
        Audio = 1,
        Video = 2,
        AV = Audio | Video
    };

    ChatFormHeader(QWidget* parent = nullptr);
    ~ChatFormHeader() override;

    void setName(const QString& newName);
    void setMode(Mode mode);

    void showOutgoingCall(bool video);
    void showCallConfirm(bool video);
    void removeCallConfirm();

    void updateCallButtons(bool online, bool audio, bool video = false);
    void updateMuteMicButton(bool active, bool inputMuted);
    void updateMuteVolButton(bool active, bool outputMuted);

    void setAvatar(const QPixmap& img);
    QSize getAvatarSize() const;

    // TODO: Remove
    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
    void addLayout(QLayout* layout);
    void addStretch();

signals:
    void callTriggered();
    void videoCallTriggered();
    void micMuteToggle();
    void volMuteToggle();

    void nameChanged(const QString& name);

    void callAccepted(bool video);
    void callRejected();

private slots:
    void onNameChanged(const QString& name);
    void retranslateUi();

private:
    Mode mode;
    MaskablePixmapWidget* avatar;
    QVBoxLayout* headTextLayout;
    CroppingLabel* nameLabel;

    QPushButton* callButton;
    QPushButton* videoButton;

    QToolButton* volButton;
    QToolButton* micButton;

    QPointer<CallConfirmWidget> callConfirm;
};

#endif // CHAT_FORM_HEADER
