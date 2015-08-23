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

#include "genericchatform.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QDebug>
#include <QShortcut>
#include <QKeyEvent>

#include "src/persistence/smileypack.h"
#include "src/widget/emoticonswidget.h"
#include "src/widget/style.h"
#include "src/widget/widget.h"
#include "src/persistence/settings.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/tool/findwidget.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/core/core.h"
#include "src/grouplist.h"
#include "src/group.h"
#include "src/friendlist.h"
#include "src/friend.h"
#include "src/chatlog/chatlog.h"
#include "src/chatlog/content/timestamp.h"
#include "src/widget/tool/flyoutoverlaywidget.h"
#include "src/widget/translator.h"
#include "src/widget/tool/indicatorscrollbar.h"

GenericChatForm::GenericChatForm(QWidget *parent)
  : QWidget(parent)
  , audioInputFlag(false)
  , audioOutputFlag(false)
{
    curRow = 0;
    headWidget = new QWidget();

    nameLabel = new CroppingLabel();
    nameLabel->setObjectName("nameLabel");
    nameLabel->setMinimumHeight(Style::getFont(Style::Medium).pixelSize());
    nameLabel->setEditable(true);
    nameLabel->setTextFormat(Qt::PlainText);

    avatar = new MaskablePixmapWidget(this, QSize(40,40), ":/img/avatar_mask.svg");
    QHBoxLayout *mainFootLayout = new QHBoxLayout(),
                *headLayout = new QHBoxLayout();

    mainLayout = new QVBoxLayout();
    QVBoxLayout *footButtonsSmall = new QVBoxLayout(),
                *micButtonsLayout = new QVBoxLayout();
                headTextLayout = new QVBoxLayout();

    QGridLayout *buttonsLayout = new QGridLayout();

    chatWidget = new ChatLog(this);
    chatWidget->setVerticalScrollBar(new IndicatorScrollBar(1));
    chatWidget->setBusyNotification(ChatMessage::createBusyNotification());

    connect(&Settings::getInstance(), &Settings::emojiFontChanged, this, [this]() { chatWidget->forceRelayout(); });

    msgEdit = new ChatTextEdit();

    sendButton = new QPushButton();
    emoteButton = new QPushButton();

    // Setting the sizes in the CSS doesn't work (glitch with high DPIs)
    fileButton = new QPushButton();
    screenshotButton = new QPushButton;
    callButton = new QPushButton();
    callButton->setFixedSize(50,40);
    videoButton = new QPushButton();
    videoButton->setFixedSize(50,40);
    volButton = new QPushButton();
    //volButton->setFixedSize(25,20);
    volButton->setToolTip("");
    micButton = new QPushButton();
    // micButton->setFixedSize(25,20);
    micButton->setToolTip("");

    fileFlyout = new FlyoutOverlayWidget;
    QHBoxLayout *fileLayout = new QHBoxLayout(fileFlyout);
    fileLayout->addWidget(screenshotButton);
    fileLayout->setContentsMargins(0, 0, 0, 0);

    footButtonsSmall->setSpacing(2);
    fileLayout->setSpacing(0);
    fileLayout->setMargin(0);

    msgEdit->setStyleSheet(Style::getStylesheet(":/ui/msgEdit/msgEdit.css"));
    msgEdit->setFixedHeight(50);
    msgEdit->setFrameStyle(QFrame::NoFrame);

    sendButton->setStyleSheet(Style::getStylesheet(":/ui/sendButton/sendButton.css"));
    fileButton->setStyleSheet(Style::getStylesheet(":/ui/fileButton/fileButton.css"));
    screenshotButton->setStyleSheet(Style::getStylesheet(":/ui/screenshotButton/screenshotButton.css"));
    emoteButton->setStyleSheet(Style::getStylesheet(":/ui/emoteButton/emoteButton.css"));

    callButton->setObjectName("green");
    callButton->setStyleSheet(Style::getStylesheet(":/ui/callButton/callButton.css"));

    videoButton->setObjectName("green");
    videoButton->setStyleSheet(Style::getStylesheet(":/ui/videoButton/videoButton.css"));

    QString volButtonStylesheet = Style::getStylesheet(":/ui/volButton/volButton.css");
    volButton->setObjectName("grey");
    volButton->setStyleSheet(volButtonStylesheet);

    QString micButtonStylesheet = Style::getStylesheet(":/ui/micButton/micButton.css");
    micButton->setObjectName("grey");
    micButton->setStyleSheet(micButtonStylesheet);

    setLayout(mainLayout);
    mainLayout->addWidget(chatWidget);
    mainLayout->addLayout(mainFootLayout);
    mainLayout->setMargin(0);

    footButtonsSmall->addWidget(emoteButton);
    footButtonsSmall->addWidget(fileButton);

    mainFootLayout->addWidget(msgEdit);
    mainFootLayout->addLayout(footButtonsSmall);
    mainFootLayout->addSpacing(5);
    mainFootLayout->addWidget(sendButton);
    mainFootLayout->setSpacing(0);

    headTextLayout->addStretch();
    headTextLayout->addWidget(nameLabel);
    headTextLayout->addStretch();

    micButtonsLayout->setSpacing(0);
    micButtonsLayout->addWidget(micButton, Qt::AlignTop | Qt::AlignRight);
    micButtonsLayout->addSpacing(4);
    micButtonsLayout->addWidget(volButton, Qt::AlignTop | Qt::AlignRight);

    buttonsLayout->addLayout(micButtonsLayout, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignRight);
    buttonsLayout->addWidget(callButton, 0, 1, 2, 1, Qt::AlignTop);
    buttonsLayout->addWidget(videoButton, 0, 2, 2, 1, Qt::AlignTop);
    buttonsLayout->setVerticalSpacing(0);
    buttonsLayout->setHorizontalSpacing(4);

    headLayout->addWidget(avatar);
    headLayout->addSpacing(5);
    headLayout->addLayout(headTextLayout);
    headLayout->addLayout(buttonsLayout);

    headWidget->setLayout(headLayout);

    //Fix for incorrect layouts on OS X as per
    //https://bugreports.qt-project.org/browse/QTBUG-14591
    sendButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    fileButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    screenshotButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    emoteButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    micButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    volButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    callButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    videoButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    menu.addActions(chatWidget->actions());
    menu.addSeparator();
    saveChatAction = menu.addAction(QIcon::fromTheme("document-save"), QString(), this, SLOT(onSaveLogClicked()));
    clearAction = menu.addAction(QIcon::fromTheme("edit-clear"), QString(), this, SLOT(clearChatArea(bool)));
    menu.addSeparator();

    connect(emoteButton, &QPushButton::clicked, this, &GenericChatForm::onEmoteButtonClicked);
    connect(chatWidget, &ChatLog::customContextMenuRequested, this, &GenericChatForm::onChatContextMenuRequested);

    new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_L, this, SLOT(clearChatArea()));

    chatWidget->setStyleSheet(Style::getStylesheet(":/ui/chatArea/chatArea.css"));
    headWidget->setStyleSheet(Style::getStylesheet(":/ui/chatArea/chatHead.css"));

    fileFlyout->setFixedSize(24, 24);
    fileFlyout->setParent(this);
    fileButton->installEventFilter(this);
    fileFlyout->installEventFilter(this);

    retranslateUi();
    Translator::registerHandler(std::bind(&GenericChatForm::retranslateUi, this), this);

    QShortcut* shortcut = new QShortcut(QKeySequence::Find, this);
    connect(shortcut, &QShortcut::activated, this, &GenericChatForm::toggleFindWidget);
}

GenericChatForm::~GenericChatForm()
{
    Translator::unregister(this);
}

void GenericChatForm::adjustFileMenuPosition()
{
    QPoint pos = fileButton->pos();
    QSize size = fileFlyout->size();
    fileFlyout->move(pos.x() - size.width(), pos.y());
}

void GenericChatForm::showFileMenu()
{
    if (!fileFlyout->isShown() && !fileFlyout->isBeingShown()) {
        adjustFileMenuPosition();
    }

    fileFlyout->animateShow();
}

void GenericChatForm::hideFileMenu()
{
    if(fileFlyout->isShown() || fileFlyout->isBeingShown())
        fileFlyout->animateHide();

}

bool GenericChatForm::isEmpty()
{
    return chatWidget->isEmpty();
}

ChatLog *GenericChatForm::getChatLog() const
{
    return chatWidget;
}

QDate GenericChatForm::getLatestDate() const
{
    return chatWidget->getLatestDate();
}

QDateTime GenericChatForm::getEarliestDate() const
{
    QVector<ChatLine::Ptr> lines = chatWidget->getLines();
    int index = 0;

    while (index < lines.size() - 1)
    {
        Timestamp* timestamp = dynamic_cast<Timestamp*>(lines[index++]->getContent(2));

        if (timestamp && timestamp->getTime().isValid())
            return timestamp->getTime();
    }

    return QDateTime::currentDateTime();
}

void GenericChatForm::setName(const QString &newName)
{
    nameLabel->setText(newName);
    nameLabel->setToolTip(newName); // for overlength names
}

void GenericChatForm::show(Ui::MainWindow &ui)
{
    ui.mainContent->layout()->addWidget(this);
    ui.mainHead->layout()->addWidget(headWidget);
    headWidget->show();
    QWidget::show();
}

void GenericChatForm::showEvent(QShowEvent *)
{
    msgEdit->setFocus();
}

bool GenericChatForm::event(QEvent* e)
{
    // If the user accidentally starts typing outside of the msgEdit, focus it automatically
    if (e->type() == QEvent::KeyRelease && !msgEdit->hasFocus())
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(e);
        if ((ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::ShiftModifier)
                && !ke->text().isEmpty() && !hasFindWidget())
            msgEdit->setFocus();
    }
    return QWidget::event(e);
}

void GenericChatForm::onChatContextMenuRequested(QPoint pos)
{
    QWidget* sender = (QWidget*)QObject::sender();
    pos = sender->mapToGlobal(pos);

    menu.exec(pos);
}

ChatMessage::Ptr GenericChatForm::addMessage(const ToxId& author, const QString &message, bool isAction,
                                             const QDateTime &datetime, bool isSent)
{
    QString authorStr = author.isActiveProfile() ? Core::getInstance()->getUsername() : resolveToxId(author);

    ChatMessage::Ptr msg;
    if (isAction)
    {
        msg = ChatMessage::createChatMessage(authorStr, message, ChatMessage::ACTION, false);
        previousId.clear();
    }
    else
    {
        msg = ChatMessage::createChatMessage(authorStr, message, ChatMessage::NORMAL, author.isActiveProfile());
        if ( (author == previousId) && (prevMsgDateTime.secsTo(QDateTime::currentDateTime()) < getChatLog()->repNameAfter) )
            msg->hideSender();

        previousId = author;
        prevMsgDateTime = QDateTime::currentDateTime();
    }

    insertChatMessage(msg, true);

    if (isSent)
        msg->markAsSent(datetime);

    static_cast<IndicatorScrollBar*>(chatWidget->verticalScrollBar())->setTotal(chatWidget->height());

    return msg;
}

ChatMessage::Ptr GenericChatForm::addSelfMessage(const QString &message, bool isAction, const QDateTime &datetime, bool isSent)
{
    return addMessage(Core::getInstance()->getSelfId(), message, isAction, datetime, isSent);
}

void GenericChatForm::addAlertMessage(const ToxId &author, QString message, QDateTime datetime)
{
    QString authorStr = resolveToxId(author);
    ChatMessage::Ptr msg = ChatMessage::createChatMessage(authorStr, message, ChatMessage::ALERT, author.isActiveProfile(), datetime);
    insertChatMessage(msg);

    if ((author == previousId) && (prevMsgDateTime.secsTo(QDateTime::currentDateTime()) < getChatLog()->repNameAfter))
        msg->hideSender();

    previousId = author;
    prevMsgDateTime = QDateTime::currentDateTime();
}

void GenericChatForm::onEmoteButtonClicked()
{
    // don't show the smiley selection widget if there are no smileys available
    if (SmileyPack::getInstance().getEmoticons().empty())
        return;

    EmoticonsWidget widget;
    connect(&widget, SIGNAL(insertEmoticon(QString)), this, SLOT(onEmoteInsertRequested(QString)));

    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender)
    {
        QPoint pos = -QPoint(widget.sizeHint().width() / 2, widget.sizeHint().height()) - QPoint(0, 10);
        widget.exec(sender->mapToGlobal(pos));
    }
}

void GenericChatForm::onEmoteInsertRequested(QString str)
{
    // insert the emoticon
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender)
        msgEdit->insertPlainText(str);

    msgEdit->setFocus(); // refocus so that we can continue typing
}

void GenericChatForm::onSaveLogClicked()
{
    QString path = QFileDialog::getSaveFileName(0, tr("Save chat log"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QString plainText;
    auto lines = chatWidget->getLines();
    for (ChatLine::Ptr l : lines)
    {
        Timestamp* rightCol = dynamic_cast<Timestamp*>(l->getContent(2));

        if (!rightCol)
            return;

        ChatLineContent* middleCol = l->getContent(1);
        ChatLineContent* leftCol = l->getContent(0);

        QString timestamp = (!rightCol || rightCol->getTime().isNull()) ? tr("Not sent") : rightCol->getText();
        QString nick = leftCol->getText();
        QString msg = middleCol->getText();

        plainText += QString("[%2] %1\n%3\n\n").arg(nick, timestamp, msg);
    }

    file.write(plainText.toUtf8());
    file.close();
}

void GenericChatForm::onCopyLogClicked()
{
    chatWidget->copySelectedText();
}

void GenericChatForm::focusInput()
{
    msgEdit->setFocus();
}

void GenericChatForm::toggleFindWidget()
{
    if (!hasFindWidget())
        showFindWidget();
    else
        removeFindWidget(QString());
}

void GenericChatForm::showFindWidget()
{
    if (!hasFindWidget())
    {
        FindWidget *findWidget = new FindWidget(this);

        connect(findWidget, &FindWidget::findText, this, &GenericChatForm::findText);
        connect(findWidget, &FindWidget::findNext, this, &GenericChatForm::findNext);
        connect(findWidget, &FindWidget::findPrevious, this, &GenericChatForm::findPrevious);
        connect(findWidget, &FindWidget::close, this, &GenericChatForm::removeFindWidget);
        connect(this, &GenericChatForm::findMatchesChanged, findWidget, &FindWidget::setMatches);
        connect(this, &GenericChatForm::findIndexChanged, findWidget, &FindWidget::setIndex);

        mainLayout->insertWidget(0, findWidget);
    }
}

void GenericChatForm::removeFindWidget(const QString& text)
{
    if (hasFindWidget())
    {
        QWidget* findWidget = mainLayout->itemAt(0)->widget();
        mainLayout->removeWidget(findWidget);
        findWidget->deleteLater();

        IndicatorScrollBar* indicatorScroll =
        static_cast<IndicatorScrollBar*>(chatWidget->verticalScrollBar());
        indicatorScroll->clearIndicators();

        QVector<ChatLine::Ptr> chatLines = getChatLog()->getLines();

        for (ChatLine::Ptr chatLine : chatLines)
            chatLine.get()->setHighlight(QString(), Qt::CaseInsensitive);

        if (getChatLog()->getSelectedText() == text)
            getChatLog()->clearSelection();

    }
}

void GenericChatForm::findText(const QString &text, Qt::CaseSensitivity sensitivity)
{
    int index;
    int total = getChatLog()->findText(text, sensitivity, index);

    IndicatorScrollBar* indicatorScroll =
    static_cast<IndicatorScrollBar*>(chatWidget->verticalScrollBar());
    indicatorScroll->clearIndicators();

    for (ChatLine::Ptr chatLine : getChatLog()->getFoundLines().values())
    {
        indicatorScroll->setTotal(chatWidget->sceneRect().height());
        indicatorScroll->addIndicator(chatLine.get()->sceneBoundingRect().top() + 20);
        indicatorScroll->update();
    }

    emit findMatchesChanged(index + 1, total);
}

void GenericChatForm::findNext(const QString& text, int to, int total, Qt::CaseSensitivity sensitivity)
{
    emit findIndexChanged(getChatLog()->findNext(text, to, total, sensitivity));
}

void GenericChatForm::findPrevious(const QString& text, int to, int total, Qt::CaseSensitivity sensitivity)
{
    emit findIndexChanged(getChatLog()->findPrevious(text, to, total, sensitivity));
}

void GenericChatForm::addSystemInfoMessage(const QString &message, ChatMessage::SystemMessageType type, const QDateTime &datetime)
{
    previousId.clear();
    insertChatMessage(ChatMessage::createChatInfoMessage(message, type, datetime));
}

void GenericChatForm::clearChatArea()
{
    clearChatArea(true);
}

void GenericChatForm::clearChatArea(bool notinform)
{
    chatWidget->clear();
    previousId = ToxId();

    if (!notinform)
        addSystemInfoMessage(tr("Cleared"), ChatMessage::INFO, QDateTime::currentDateTime());

    earliestMessage = QDateTime(); //null
    historyBaselineDate = QDateTime::currentDateTime();

    emit chatAreaCleared();
}

void GenericChatForm::onSelectAllClicked()
{
    chatWidget->selectAll();
}

QString GenericChatForm::resolveToxId(const ToxId &id)
{
    Friend *f = FriendList::findFriend(id);
    if (f)
    {
        return f->getDisplayedName();
    }
    else
    {
        for (auto it : GroupList::getAllGroups())
        {
            QString res = it->resolveToxId(id);
            if (res.size())
                return res;
        }
    }

    return QString();
}

void GenericChatForm::insertChatMessage(ChatMessage::Ptr msg, bool notify)
{
    chatWidget->insertChatlineAtBottom(std::dynamic_pointer_cast<ChatLine>(msg));

    if (notify)
        chatWidget->showNotification(msg);
}

void GenericChatForm::hideEvent(QHideEvent* event)
{
    hideFileMenu();
    QWidget::hideEvent(event);
}

void GenericChatForm::resizeEvent(QResizeEvent* event)
{
    adjustFileMenuPosition();
    QWidget::resizeEvent(event);
}

bool GenericChatForm::eventFilter(QObject* object, QEvent* event)
{
    if (object != this->fileButton && object != this->fileFlyout)
        return false;

    if (!qobject_cast<QWidget*>(object)->isEnabled())
        return false;

    switch(event->type())
    {
    case QEvent::Enter:
        showFileMenu();
        break;

    case QEvent::Leave: {
        QPoint pos = mapFromGlobal(QCursor::pos());
        QRect fileRect(fileFlyout->pos(), fileFlyout->size());
        fileRect = fileRect.united(QRect(fileButton->pos(), fileButton->size()));

        if (!fileRect.contains(pos))
            hideFileMenu();

    } break;

    case QEvent::MouseButtonPress:
        hideFileMenu();
        break;

    default:
        break;
    }

    return false;
}

bool GenericChatForm::hasFindWidget() const
{
    return mainLayout->count() == 3;
}

void GenericChatForm::retranslateUi()
{
    sendButton->setToolTip(tr("Send message"));
    emoteButton->setToolTip(tr("Smileys"));
    fileButton->setToolTip(tr("Send file(s)"));
    screenshotButton->setToolTip(tr("Send a screenshot"));
    callButton->setToolTip(tr("Start an audio call"));
    videoButton->setToolTip(tr("Start a video call"));
    saveChatAction->setText(tr("Save chat log"));
    clearAction->setText(tr("Clear displayed messages"));
}
