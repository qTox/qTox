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

#include "chatformheader.h"

#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/style.h"
#include "src/widget/tool/callconfirmwidget.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/translator.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QTextDocument>
#include <QToolButton>

static const QSize AVATAR_SIZE{40, 40};
static const QSize CALL_BUTTONS_SIZE{50, 40};
static const QSize VOL_MIC_BUTTONS_SIZE{22, 18};
static const short HEAD_LAYOUT_SPACING = 5;
static const short MIC_BUTTONS_LAYOUT_SPACING = 4;
static const short BUTTONS_LAYOUT_HOR_SPACING = 4;
static const QString Green = QStringLiteral("green");

#define STYLE_SHEET(x) Style::getStylesheet(":/ui/" #x "/" #x ".css")
#define SET_STYLESHEET(x) (x)->setStyleSheet(STYLE_SHEET(x))

ChatFormHeader::ChatFormHeader(QWidget* parent)
    : QWidget(parent)
    , mode{Mode::AV}
{
    QHBoxLayout* headLayout = new QHBoxLayout();
    avatar = new MaskablePixmapWidget(this, AVATAR_SIZE, ":/img/avatar_mask.svg");

    nameLabel = new CroppingLabel();
    nameLabel->setObjectName("nameLabel");
    nameLabel->setMinimumHeight(Style::getFont(Style::Medium).pixelSize());
    nameLabel->setEditable(true);
    nameLabel->setTextFormat(Qt::PlainText);
    connect(nameLabel, &CroppingLabel::editFinished, this, &ChatFormHeader::onNameChanged);

    headTextLayout = new QVBoxLayout();
    headTextLayout->addStretch();
    headTextLayout->addWidget(nameLabel);
    headTextLayout->addStretch();

    micButton = new QToolButton();
    micButton->setFixedSize(VOL_MIC_BUTTONS_SIZE);
    micButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    SET_STYLESHEET(micButton);
    connect(micButton, &QPushButton::clicked, this, &ChatFormHeader::micMuteToggle);

    volButton = new QToolButton();
    volButton->setFixedSize(VOL_MIC_BUTTONS_SIZE);
    volButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    SET_STYLESHEET(volButton);
    connect(volButton, &QPushButton::clicked, this, &ChatFormHeader::volMuteToggle);

    callButton = new QPushButton();
    callButton->setFixedSize(CALL_BUTTONS_SIZE);
    callButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    SET_STYLESHEET(callButton);
    connect(callButton, &QPushButton::clicked, this, &ChatFormHeader::callTriggered);

    videoButton = new QPushButton();
    videoButton->setFixedSize(CALL_BUTTONS_SIZE);
    videoButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    SET_STYLESHEET(videoButton);
    connect(videoButton, &QPushButton::clicked, this, &ChatFormHeader::videoCallTriggered);

    QVBoxLayout* micButtonsLayout = new QVBoxLayout();
    micButtonsLayout->setSpacing(MIC_BUTTONS_LAYOUT_SPACING);
    micButtonsLayout->addWidget(micButton, Qt::AlignTop | Qt::AlignRight);
    micButtonsLayout->addWidget(volButton, Qt::AlignTop | Qt::AlignRight);

    QGridLayout* buttonsLayout = new QGridLayout();
    buttonsLayout->addLayout(micButtonsLayout, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignRight);
    buttonsLayout->addWidget(callButton, 0, 1, 2, 1, Qt::AlignTop);
    buttonsLayout->addWidget(videoButton, 0, 2, 2, 1, Qt::AlignTop);
    buttonsLayout->setVerticalSpacing(0);
    buttonsLayout->setHorizontalSpacing(BUTTONS_LAYOUT_HOR_SPACING);

    headLayout->addWidget(avatar);
    headLayout->addSpacing(HEAD_LAYOUT_SPACING);
    headLayout->addLayout(headTextLayout);
    headLayout->addLayout(buttonsLayout);

    setLayout(headLayout);

    updateCallButtons(false, false, false);
    updateMuteMicButton(false, false);
    updateMuteVolButton(false, false);

    retranslateUi();
    Translator::registerHandler(std::bind(&ChatFormHeader::retranslateUi, this), this);
}

ChatFormHeader::~ChatFormHeader()
{
    delete callConfirm;
}

void ChatFormHeader::setName(const QString& newName)
{
    nameLabel->setText(newName);
    // for overlength names
    nameLabel->setToolTip(Qt::convertFromPlainText(newName, Qt::WhiteSpaceNormal));
}

void ChatFormHeader::setMode(ChatFormHeader::Mode mode)
{
    this->mode = mode;
    if (mode == Mode::None) {
        callButton->hide();
        videoButton->hide();
        volButton->hide();
        micButton->hide();
    }
}

void ChatFormHeader::retranslateUi()
{
    const QString callObjectName = callButton->objectName();
    const QString videoObjectName = videoButton->objectName();

    if (callObjectName == QStringLiteral("green")) {
        callButton->setToolTip(tr("Start audio call"));
    } else if (callObjectName == QStringLiteral("yellow")) {
        callButton->setToolTip(tr("Accept audio call"));
    } else if (callObjectName == QStringLiteral("red")) {
        callButton->setToolTip(tr("End audio call"));
    } else if (callObjectName.isEmpty()) {
        callButton->setToolTip(QString{});
    }

    if (videoObjectName == QStringLiteral("green")) {
        videoButton->setToolTip(tr("Start video call"));
    } else if (videoObjectName == QStringLiteral("yellow")) {
        videoButton->setToolTip(tr("Accept video call"));
    } else if (videoObjectName == QStringLiteral("red")) {
        videoButton->setToolTip(tr("End video call"));
    } else if (videoObjectName.isEmpty()) {
        videoButton->setToolTip(QString{});
    }
}

void ChatFormHeader::showOutgoingCall(bool video)
{
    QPushButton* btn = video ? videoButton : callButton;
    btn->setObjectName("yellow");
    btn->setStyleSheet(video ? STYLE_SHEET(videoButton) : STYLE_SHEET(callButton));
    btn->setToolTip(video ? tr("Cancel video call") : tr("Cancel audio call"));
}

void ChatFormHeader::showCallConfirm(bool video)
{
    callConfirm = new CallConfirmWidget(video ? videoButton : callButton);
    callConfirm->show();
    CallConfirmWidget* confirmData = callConfirm.data();
    connect(confirmData, &CallConfirmWidget::accepted, this, [this, video]{
        emit callAccepted(video);
    });
    connect(confirmData, &CallConfirmWidget::rejected, this, &ChatFormHeader::callRejected);
}

void ChatFormHeader::removeCallConfirm()
{
    delete callConfirm;
}

void ChatFormHeader::updateCallButtons(bool online, bool audio, bool video)
{
    callButton->setEnabled(audio && !video);
    videoButton->setEnabled(video);
    if (audio) {
        callButton->setObjectName(!video ? "red" : "");
        callButton->setToolTip(!video ? tr("End audio call") : tr("Can't start audio call"));

        videoButton->setObjectName(video ? "red" : "");
        videoButton->setToolTip(video ? tr("End video call") : tr("Can't start video call"));
    } else {
        const bool audioAvaliable = online && (mode & Mode::Audio);
        callButton->setEnabled(audioAvaliable);
        callButton->setObjectName(audioAvaliable ? "green" : "");
        callButton->setToolTip(audioAvaliable ? tr("Start audio call") : tr("Can't start audio call"));

        const bool videoAvaliable = online && (mode & Mode::Video);
        videoButton->setEnabled(videoAvaliable);
        videoButton->setObjectName(videoAvaliable ? "green" : "");
        videoButton->setToolTip(videoAvaliable ? tr("Start video call") : tr("Can't start video call"));
    }

    callButton->setStyleSheet(STYLE_SHEET(callButton));
    videoButton->setStyleSheet(STYLE_SHEET(videoButton));
}

void ChatFormHeader::updateMuteMicButton(bool active, bool inputMuted)
{
    micButton->setEnabled(active);
    if (micButton->isEnabled()) {
        micButton->setObjectName(inputMuted ? "red" : "green");
        micButton->setToolTip(inputMuted ? tr("Unmute microphone") : tr("Mute microphone"));
    } else {
        micButton->setObjectName("");
        micButton->setToolTip(tr("Microphone can be muted only during a call"));
    }

    micButton->setStyleSheet(STYLE_SHEET(micButton));
}

void ChatFormHeader::updateMuteVolButton(bool active, bool outputMuted)
{
    volButton->setEnabled(active);
    if (volButton->isEnabled()) {
        volButton->setObjectName(outputMuted ? "red" : "green");
        volButton->setToolTip(outputMuted ? tr("Unmute call") : tr("Mute call"));
    } else {
        volButton->setObjectName("");
        volButton->setToolTip(tr("Sound can be disabled only during a call"));
    }

    volButton->setStyleSheet(STYLE_SHEET(volButton));
}

void ChatFormHeader::setAvatar(const QPixmap &img)
{
    avatar->setPixmap(img);
}

QSize ChatFormHeader::getAvatarSize() const
{
    return QSize{avatar->width(), avatar->height()};
}

void ChatFormHeader::addWidget(QWidget* widget, int stretch, Qt::Alignment alignment)
{
    headTextLayout->addWidget(widget, stretch, alignment);
}

void ChatFormHeader::addLayout(QLayout* layout)
{
    headTextLayout->addLayout(layout);
}

void ChatFormHeader::addStretch()
{
    headTextLayout->addStretch();
}

void ChatFormHeader::onNameChanged(const QString& name)
{
    if (!name.isEmpty()) {
        nameLabel->setText(name);
        emit nameChanged(name);
    }
}
