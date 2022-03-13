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

#include "chatformheader.h"
#include "extensionstatus.h"

#include "src/model/status.h"

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

namespace {
const QSize AVATAR_SIZE{40, 40};
const short HEAD_LAYOUT_SPACING = 5;
const short MIC_BUTTONS_LAYOUT_SPACING = 4;
const short BUTTONS_LAYOUT_HOR_SPACING = 4;

const QString STYLE_PATH = QStringLiteral("chatForm/buttons.css");

const QString STATE_NAME[] = {
    QString{},
    QStringLiteral("green"),
    QStringLiteral("red"),
    QStringLiteral("yellow"),
    QStringLiteral("yellow"),
};

const QString CALL_TOOL_TIP[] = {
    ChatFormHeader::tr("Can't start audio call"),
    ChatFormHeader::tr("Start audio call"),
    ChatFormHeader::tr("End audio call"),
    ChatFormHeader::tr("Cancel audio call"),
    ChatFormHeader::tr("Accept audio call"),
};

const QString VIDEO_TOOL_TIP[] = {
    ChatFormHeader::tr("Can't start video call"),
    ChatFormHeader::tr("Start video call"),
    ChatFormHeader::tr("End video call"),
    ChatFormHeader::tr("Cancel video call"),
    ChatFormHeader::tr("Accept video call"),
};

const QString VOL_TOOL_TIP[] = {
    ChatFormHeader::tr("Sound can be disabled only during a call"),
    ChatFormHeader::tr("Mute call"),
    ChatFormHeader::tr("Unmute call"),
};

const QString MIC_TOOL_TIP[] = {
    ChatFormHeader::tr("Microphone can be muted only during a call"),
    ChatFormHeader::tr("Mute microphone"),
    ChatFormHeader::tr("Unmute microphone"),
};

template <class T, class Fun>
QPushButton* createButton(const QString& name, T* self, Fun onClickSlot,
    Settings& settings, Style& style)
{
    QPushButton* btn = new QPushButton();
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setObjectName(name);
    btn->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    QObject::connect(btn, &QPushButton::clicked, self, onClickSlot);
    return btn;
}

template<class State>
void setStateToolTip(QAbstractButton* btn, State state, const QString toolTip[])
{
    const int index = static_cast<int>(state);
    btn->setToolTip(toolTip[index]);
}

template<class State>
void setStateName(QAbstractButton* btn, State state)
{
    const int index = static_cast<int>(state);
    btn->setProperty("state", STATE_NAME[index]);
    btn->setEnabled(index != 0);
}

}

ChatFormHeader::ChatFormHeader(Settings& settings_, Style& style_, QWidget* parent)
    : QWidget(parent)
    , mode{Mode::AV}
    , callState{CallButtonState::Disabled}
    , videoState{CallButtonState::Disabled}
    , volState{ToolButtonState::Disabled}
    , micState{ToolButtonState::Disabled}
    , settings{settings_}
    , style{style_}
{
    QHBoxLayout* headLayout = new QHBoxLayout();
    avatar = new MaskablePixmapWidget(this, AVATAR_SIZE, ":/img/avatar_mask.svg");
    avatar->setObjectName("avatar");

    nameLine = new QHBoxLayout();
    nameLine->setSpacing(3);

    extensionStatus = new ExtensionStatus();

    nameLabel = new CroppingLabel();
    nameLabel->setObjectName("nameLabel");
    nameLabel->setMinimumHeight(Style::getFont(Style::Font::Medium).pixelSize());
    nameLabel->setEditable(true);
    nameLabel->setTextFormat(Qt::PlainText);
    connect(nameLabel, &CroppingLabel::editFinished, this, &ChatFormHeader::nameChanged);

    nameLine->addWidget(extensionStatus);
    nameLine->addWidget(nameLabel);

    headTextLayout = new QVBoxLayout();
    headTextLayout->addStretch();
    headTextLayout->addLayout(nameLine);
    headTextLayout->addStretch();

    micButton = createButton("micButton", this, &ChatFormHeader::micMuteToggle, settings, style);
    volButton = createButton("volButton", this, &ChatFormHeader::volMuteToggle, settings, style);
    callButton = createButton("callButton", this, &ChatFormHeader::callTriggered, settings, style);
    videoButton = createButton("videoButton", this, &ChatFormHeader::videoCallTriggered, settings, style);

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

    updateButtonsView();
    Translator::registerHandler(std::bind(&ChatFormHeader::retranslateUi, this), this);

    connect(&style, &Style::themeReload, this, &ChatFormHeader::reloadTheme);
}

ChatFormHeader::~ChatFormHeader() = default;

void ChatFormHeader::setName(const QString& newName)
{
    nameLabel->setText(newName);
    // for overlength names
    nameLabel->setToolTip(Qt::convertFromPlainText(newName, Qt::WhiteSpaceNormal));
}

void ChatFormHeader::setMode(ChatFormHeader::Mode mode_)
{
    mode = mode_;
    if (mode == Mode::None) {
        callButton->hide();
        videoButton->hide();
        volButton->hide();
        micButton->hide();
    }
}

void ChatFormHeader::retranslateUi()
{
    setStateToolTip(callButton, callState, CALL_TOOL_TIP);
    setStateToolTip(videoButton, videoState, VIDEO_TOOL_TIP);
    setStateToolTip(micButton, micState, MIC_TOOL_TIP);
    setStateToolTip(volButton, volState, VOL_TOOL_TIP);
}

void ChatFormHeader::updateButtonsView()
{
    setStateName(callButton, callState);
    setStateName(videoButton, videoState);
    setStateName(micButton, micState);
    setStateName(volButton, volState);
    retranslateUi();
    Style::repolish(this);
}

void ChatFormHeader::showOutgoingCall(bool video)
{
    CallButtonState& state = video ? videoState : callState;
    state = CallButtonState::Outgoing;
    updateButtonsView();
}

void ChatFormHeader::createCallConfirm(bool video)
{
    QWidget* btn = video ? videoButton : callButton;
    callConfirm = std::unique_ptr<CallConfirmWidget>(new CallConfirmWidget(settings, style, btn));
    connect(callConfirm.get(), &CallConfirmWidget::accepted, this, &ChatFormHeader::callAccepted);
    connect(callConfirm.get(), &CallConfirmWidget::rejected, this, &ChatFormHeader::callRejected);
}

void ChatFormHeader::showCallConfirm()
{
    if (callConfirm && !callConfirm->isVisible())
        callConfirm->show();
}

void ChatFormHeader::removeCallConfirm()
{
    callConfirm.reset(nullptr);
}

void ChatFormHeader::updateExtensionSupport(ExtensionSet extensions)
{
    extensionStatus->onExtensionSetUpdate(extensions);
}

void ChatFormHeader::updateCallButtons(bool online, bool audio, bool video)
{
    const bool audioAvaliable = online && (mode & Mode::Audio);
    const bool videoAvaliable = online && (mode & Mode::Video);
    if (!audioAvaliable) {
        callState = CallButtonState::Disabled;
    } else if (video) {
        callState = CallButtonState::Disabled;
    } else if (audio) {
        callState = CallButtonState::InCall;
    } else {
        callState = CallButtonState::Avaliable;
    }

    if (!videoAvaliable) {
        videoState = CallButtonState::Disabled;
    } else if (video) {
        videoState = CallButtonState::InCall;
    } else if (audio) {
        videoState = CallButtonState::Disabled;
    } else {
        videoState = CallButtonState::Avaliable;
    }

    updateButtonsView();
}

void ChatFormHeader::updateMuteMicButton(bool active, bool inputMuted)
{
    micButton->setEnabled(active);
    if (active) {
        micState = inputMuted ? ToolButtonState::On : ToolButtonState::Off;
    } else {
        micState = ToolButtonState::Disabled;
    }

    updateButtonsView();
}

void ChatFormHeader::updateMuteVolButton(bool active, bool outputMuted)
{
    volButton->setEnabled(active);
    if (active) {
        volState = outputMuted ? ToolButtonState::On : ToolButtonState::Off;
    } else {
        volState = ToolButtonState::Disabled;
    }

    updateButtonsView();
}

void ChatFormHeader::setAvatar(const QPixmap &img)
{
    avatar->setPixmap(img);
}

QSize ChatFormHeader::getAvatarSize() const
{
    return QSize{avatar->width(), avatar->height()};
}

void ChatFormHeader::reloadTheme()
{
    setStyleSheet(style.getStylesheet("chatArea/chatHead.css", settings));
    callButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    videoButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    volButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    micButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
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
