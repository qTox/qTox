/*
    Copyright © 2014-2018 by The qTox Project Contributors

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

#include "src/chatlog/chatlog.h"
#include "src/chatlog/content/timestamp.h"
#include "src/core/core.h"
#include "src/model/friend.h"
#include "src/friendlist.h"
#include "src/model/group.h"
#include "src/grouplist.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/video/genericnetcamview.h"
#include "src/widget/chatformheader.h"
#include "src/widget/contentdialog.h"
#include "src/widget/contentlayout.h"
#include "src/widget/emoticonswidget.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/style.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/tool/flyoutoverlaywidget.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"
#include "src/widget/searchform.h"

#include <QClipboard>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QStringBuilder>

#ifdef SPELL_CHECKING
#include <KF5/SonnetUi/sonnet/spellcheckdecorator.h>
#endif

/**
 * @class GenericChatForm
 * @brief Parent class for all chatforms. It's provide the minimum required UI
 * elements and methods to work with chat messages.
 */

#define SET_STYLESHEET(x) (x)->setStyleSheet(Style::getStylesheet(":/ui/" #x "/" #x ".css"))

static const QSize FILE_FLYOUT_SIZE{24, 24};
static const short FOOT_BUTTONS_SPACING = 2;
static const short MESSAGE_EDIT_HEIGHT = 50;
static const short MAIN_FOOT_LAYOUT_SPACING = 5;
static const QString FONT_STYLE[]{"normal", "italic", "oblique"};

/**
 * @brief Creates CSS style string for needed class with specified font
 * @param font Font that needs to be represented for a class
 * @param name Class name
 * @return Style string
 */
static QString fontToCss(const QFont& font, const QString& name)
{
    QString result{"%1{"
                   "font-family: \"%2\"; "
                   "font-size: %3px; "
                   "font-style: \"%4\"; "
                   "font-weight: normal;}"};
    return result.arg(name).arg(font.family()).arg(font.pixelSize()).arg(FONT_STYLE[font.style()]);
}

/**
 * @brief Searches for name (possibly alias) of someone with specified public key among all of your
 * friends or groups you are participated
 * @param pk Searched public key
 * @return Name or alias of someone with such public key, or public key string representation if no
 * one was found
 */
QString GenericChatForm::resolveToxPk(const ToxPk& pk)
{
    Friend* f = FriendList::findFriend(pk);
    if (f) {
        return f->getDisplayedName();
    }

    for (Group* it : GroupList::getAllGroups()) {
        QString res = it->resolveToxId(pk);
        if (!res.isEmpty()) {
            return res;
        }
    }

    return pk.toString();
}

namespace
{
const QString STYLE_PATH = QStringLiteral(":/ui/chatForm/buttons.css");
}

namespace
{

template <class T, class Fun>
QPushButton* createButton(const QString& name, T* self, Fun onClickSlot)
{
    QPushButton* btn = new QPushButton();
    // Fix for incorrect layouts on OS X as per
    // https://bugreports.qt-project.org/browse/QTBUG-14591
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setObjectName(name);
    btn->setProperty("state", "green");
    btn->setStyleSheet(Style::getStylesheet(STYLE_PATH));
    QObject::connect(btn, &QPushButton::clicked, self, onClickSlot);
    return btn;
}

}

GenericChatForm::GenericChatForm(const Contact* contact, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , audioInputFlag(false)
    , audioOutputFlag(false)
{
    curRow = 0;
    headWidget = new ChatFormHeader();
    searchForm = new SearchForm();
    chatWidget = new ChatLog(this);
    chatWidget->setBusyNotification(ChatMessage::createBusyNotification());
    searchForm->hide();

    // settings
    const Settings& s = Settings::getInstance();
    connect(&s, &Settings::emojiFontPointSizeChanged, chatWidget, &ChatLog::forceRelayout);
    connect(&s, &Settings::chatMessageFontChanged, this, &GenericChatForm::onChatMessageFontChanged);

    msgEdit = new ChatTextEdit();
#ifdef SPELL_CHECKING
    if (s.getSpellCheckingEnabled()) {
        decorator = new Sonnet::SpellCheckDecorator(msgEdit);
    }
#endif

    sendButton = createButton("sendButton", this, &GenericChatForm::onSendTriggered);
    emoteButton = createButton("emoteButton", this, &GenericChatForm::onEmoteButtonClicked);

    fileButton = createButton("fileButton", this, &GenericChatForm::onAttachClicked);
    screenshotButton = createButton("screenshotButton", this, &GenericChatForm::onScreenshotClicked);

    // TODO: Make updateCallButtons (see ChatForm) abstract
    //       and call here to set tooltips.

    fileFlyout = new FlyoutOverlayWidget;
    QHBoxLayout* fileLayout = new QHBoxLayout(fileFlyout);
    fileLayout->addWidget(screenshotButton);
    fileLayout->setContentsMargins(0, 0, 0, 0);
    fileLayout->setSpacing(0);
    fileLayout->setMargin(0);

    msgEdit->setStyleSheet(Style::getStylesheet(":/ui/msgEdit/msgEdit.css")
                           + fontToCss(s.getChatMessageFont(), "QTextEdit"));
    msgEdit->setFixedHeight(MESSAGE_EDIT_HEIGHT);
    msgEdit->setFrameStyle(QFrame::NoFrame);

    bodySplitter = new QSplitter(Qt::Vertical, this);
    connect(bodySplitter, &QSplitter::splitterMoved, this, &GenericChatForm::onSplitterMoved);
    QWidget* contentWidget = new QWidget(this);
    bodySplitter->addWidget(contentWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(bodySplitter);
    mainLayout->setMargin(0);

    setLayout(mainLayout);

    QVBoxLayout* footButtonsSmall = new QVBoxLayout();
    footButtonsSmall->setSpacing(FOOT_BUTTONS_SPACING);
    footButtonsSmall->addWidget(emoteButton);
    footButtonsSmall->addWidget(fileButton);

    QHBoxLayout* mainFootLayout = new QHBoxLayout();
    mainFootLayout->addWidget(msgEdit);
    mainFootLayout->addLayout(footButtonsSmall);
    mainFootLayout->addSpacing(MAIN_FOOT_LAYOUT_SPACING);
    mainFootLayout->addWidget(sendButton);
    mainFootLayout->setSpacing(0);

    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->addWidget(searchForm);
    contentLayout->addWidget(chatWidget);
    contentLayout->addLayout(mainFootLayout);

    quoteAction = menu.addAction(QIcon(), QString(), this, SLOT(quoteSelectedText()),
                                 QKeySequence(Qt::ALT + Qt::Key_Q));
    addAction(quoteAction);
    menu.addSeparator();

    searchAction = menu.addAction(QIcon(), QString(), this, SLOT(searchFormShow()),
                                  QKeySequence(Qt::CTRL + Qt::Key_F));
    addAction(searchAction);

    menu.addSeparator();

    menu.addActions(chatWidget->actions());
    menu.addSeparator();

    saveChatAction = menu.addAction(QIcon::fromTheme("document-save"), QString(),
                                    this, SLOT(onSaveLogClicked()));
    clearAction = menu.addAction(QIcon::fromTheme("edit-clear"), QString(),
                                 this, SLOT(clearChatArea(bool)),
                                 QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
    addAction(clearAction);

    copyLinkAction = menu.addAction(QIcon(), QString(), this, SLOT(copyLink()));
    menu.addSeparator();

    connect(chatWidget, &ChatLog::customContextMenuRequested, this,
            &GenericChatForm::onChatContextMenuRequested);

    connect(searchForm, &SearchForm::searchInBegin, this, &GenericChatForm::searchInBegin);
    connect(searchForm, &SearchForm::searchUp, this, &GenericChatForm::onSearchUp);
    connect(searchForm, &SearchForm::searchDown, this, &GenericChatForm::onSearchDown);
    connect(searchForm, &SearchForm::visibleChanged, this, &GenericChatForm::onSearchTriggered);

    connect(chatWidget, &ChatLog::workerTimeoutFinished, this, &GenericChatForm::onContinueSearch);

    chatWidget->setStyleSheet(Style::getStylesheet(":/ui/chatArea/chatArea.css"));
    headWidget->setStyleSheet(Style::getStylesheet(":/ui/chatArea/chatHead.css"));

    fileFlyout->setFixedSize(FILE_FLYOUT_SIZE);
    fileFlyout->setParent(this);
    fileButton->installEventFilter(this);
    fileFlyout->installEventFilter(this);

    retranslateUi();
    Translator::registerHandler(std::bind(&GenericChatForm::retranslateUi, this), this);

    // update header on name/title change
    connect(contact, &Contact::displayedNameChanged, this, &GenericChatForm::setName);

    netcam = nullptr;
}

GenericChatForm::~GenericChatForm()
{
    Translator::unregister(this);
    delete searchForm;
}

void GenericChatForm::adjustFileMenuPosition()
{
    QPoint pos = fileButton->mapTo(bodySplitter, QPoint());
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
    if (fileFlyout->isShown() || fileFlyout->isBeingShown())
        fileFlyout->animateHide();
}

QDate GenericChatForm::getLatestDate() const
{
    ChatLine::Ptr chatLine = chatWidget->getLatestLine();

    if (chatLine) {
        Timestamp* timestamp = qobject_cast<Timestamp*>(chatLine->getContent(2));

        if (timestamp)
            return timestamp->getTime().date();
        else
            return QDate::currentDate();
    }

    return QDate();
}

void GenericChatForm::setName(const QString& newName)
{
    headWidget->setName(newName);
}

void GenericChatForm::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(this);
    contentLayout->mainHead->layout()->addWidget(headWidget);
    headWidget->show();
    QWidget::show();
}

void GenericChatForm::showEvent(QShowEvent*)
{
    msgEdit->setFocus();
    headWidget->showCallConfirm();
}

bool GenericChatForm::event(QEvent* e)
{
    // If the user accidentally starts typing outside of the msgEdit, focus it automatically
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(e);
        if ((ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::ShiftModifier)
                && !ke->text().isEmpty()) {
            if (searchForm->isHidden()) {
                msgEdit->sendKeyEvent(ke);
                msgEdit->setFocus();
            } else {
                searchForm->insertEditor(ke->text());
                searchForm->setFocusEditor();
            }
        }
    }
    return QWidget::event(e);
}

void GenericChatForm::onChatContextMenuRequested(QPoint pos)
{
    QWidget* sender = static_cast<QWidget*>(QObject::sender());
    pos = sender->mapToGlobal(pos);

    // If we right-clicked on a link, give the option to copy it
    bool clickedOnLink = false;
    Text* clickedText = qobject_cast<Text*>(chatWidget->getContentFromGlobalPos(pos));
    if (clickedText) {
        QPointF scenePos = chatWidget->mapToScene(chatWidget->mapFromGlobal(pos));
        QString linkTarget = clickedText->getLinkAt(scenePos);
        if (!linkTarget.isEmpty()) {
            clickedOnLink = true;
            copyLinkAction->setData(linkTarget);
        }
    }
    copyLinkAction->setVisible(clickedOnLink);

    menu.exec(pos);
}

/**
 * @brief Show, is it needed to hide message author name or not
 * @param messageAuthor Author of the sent message
 * @oaran messageTime DateTime of the sent message
 * @return True if it's needed to hide name, false otherwise
 */
bool GenericChatForm::needsToHideName(const ToxPk& messageAuthor, const QDateTime& messageTime) const
{
    qint64 messagesTimeDiff = prevMsgDateTime.secsTo(messageTime);
    return messageAuthor == previousId && messagesTimeDiff < chatWidget->repNameAfter;
}

/**
 * @brief Creates ChatMessage shared object and inserts it into ChatLog
 * @param author Author of the message
 * @param message Message text
 * @param dt Date and time when message was sent
 * @param isAction True if this is an action message, false otherwise
 * @param isSent True if message was received by your friend
 * @return ChatMessage object
 */
ChatMessage::Ptr GenericChatForm::createMessage(const ToxPk& author, const QString& message,
                                                const QDateTime& dt, bool isAction, bool isSent)
{
    const Core* core = Core::getInstance();
    bool isSelf = author == core->getSelfId().getPublicKey();
    QString authorStr = isSelf ? core->getUsername() : resolveToxPk(author);
    if (getLatestDate() != QDate::currentDate()) {
        addSystemDateMessage();
    }

    ChatMessage::Ptr msg;
    if (isAction) {
        msg = ChatMessage::createChatMessage(authorStr, message, ChatMessage::ACTION, isSelf);
        previousId = ToxPk{};
    } else {
        msg = ChatMessage::createChatMessage(authorStr, message, ChatMessage::NORMAL, isSelf);
        const QDateTime newMsgDateTime = QDateTime::currentDateTime();
        if (needsToHideName(author, newMsgDateTime)) {
            msg->hideSender();
        }

        previousId = author;
        prevMsgDateTime = newMsgDateTime;
    }

    if (isSent) {
        msg->markAsSent(dt);
    }

    insertChatMessage(msg);
    return msg;
}

/**
 * @brief Same, as createMessage, but creates message that you will send to someone
 */
ChatMessage::Ptr GenericChatForm::createSelfMessage(const QString& message, const QDateTime& dt,
                                                    bool isAction, bool isSent)
{
    ToxPk selfPk = Core::getInstance()->getSelfId().getPublicKey();
    return createMessage(selfPk, message, dt, isAction, isSent);
}

/**
 * @brief Inserts message into ChatLog
 */
void GenericChatForm::addMessage(const ToxPk& author, const QString& message, const QDateTime& dt,
                                 bool isAction)
{
    createMessage(author, message, dt, isAction, true);
}

/**
 * @brief Inserts int ChatLog message that you have sent
 */
void GenericChatForm::addSelfMessage(const QString& message, const QDateTime& datetime, bool isAction)
{
    createSelfMessage(message, datetime, isAction, true);
}

void GenericChatForm::addAlertMessage(const ToxPk& author, const QString& msg, const QDateTime& dt)
{
    QString authorStr = resolveToxPk(author);
    bool isSelf = author == Core::getInstance()->getSelfId().getPublicKey();
    auto chatMsg = ChatMessage::createChatMessage(authorStr, msg, ChatMessage::ALERT, isSelf, dt);
    const QDateTime newMsgDateTime = QDateTime::currentDateTime();
    if (needsToHideName(author, newMsgDateTime)) {
        chatMsg->hideSender();
    }

    insertChatMessage(chatMsg);
    previousId = author;
    prevMsgDateTime = newMsgDateTime;
}

void GenericChatForm::onEmoteButtonClicked()
{
    // don't show the smiley selection widget if there are no smileys available
    if (SmileyPack::getInstance().getEmoticons().empty())
        return;

    EmoticonsWidget widget;
    connect(&widget, SIGNAL(insertEmoticon(QString)), this, SLOT(onEmoteInsertRequested(QString)));
    widget.installEventFilter(this);

    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) {
        QPoint pos =
            -QPoint(widget.sizeHint().width() / 2, widget.sizeHint().height()) - QPoint(0, 10);
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
    QString path = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Save chat log"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QString plainText;
    auto lines = chatWidget->getLines();
    for (ChatLine::Ptr l : lines) {
        Timestamp* rightCol = qobject_cast<Timestamp*>(l->getContent(2));

        ChatLineContent* middleCol = l->getContent(1);
        ChatLineContent* leftCol = l->getContent(0);

        QString nick = leftCol->getText().isNull() ? tr("[System message]") : leftCol->getText();

        QString msg = middleCol->getText();

        QString timestamp = (rightCol == nullptr) ? tr("Not sent") : rightCol->getText();

        plainText += QString{nick % "\t" % timestamp % "\t" % msg % "\n"};
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

void GenericChatForm::onChatMessageFontChanged(const QFont& font)
{
    // chat log
    chatWidget->fontChanged(font);
    chatWidget->forceRelayout();
    // message editor
    msgEdit->setStyleSheet(Style::getStylesheet(":/ui/msgEdit/msgEdit.css")
                           + fontToCss(font, "QTextEdit"));
}

void GenericChatForm::addSystemInfoMessage(const QString& message, ChatMessage::SystemMessageType type,
                                           const QDateTime& datetime)
{
    if (getLatestDate() != QDate::currentDate()) {
        addSystemDateMessage();
    }

    previousId = ToxPk();
    insertChatMessage(ChatMessage::createChatInfoMessage(message, type, datetime));
}

void GenericChatForm::addSystemDateMessage()
{
    const Settings& s = Settings::getInstance();
    QString dateText = QDate::currentDate().toString(s.getDateFormat());

    previousId = ToxPk();
    insertChatMessage(ChatMessage::createChatInfoMessage(dateText, ChatMessage::INFO, QDateTime()));
}

void GenericChatForm::disableSearchText()
{
    if (searchPoint != QPoint(1, -1)) {
        QVector<ChatLine::Ptr> lines = chatWidget->getLines();
        int numLines = lines.size();
        int index = numLines - searchPoint.x();
        if (index >= 0 && numLines > index) {
            ChatLine::Ptr l = lines[index];
            if (l->getColumnCount() >= 2) {
                ChatLineContent* content = l->getContent(1);
                Text* text = static_cast<Text*>(content);
                text->deselectText();
            }
        }
    }
}

bool GenericChatForm::searchInText(const QString& phrase, bool searchUp)
{
    bool isSearch = false;

    if (phrase.isEmpty()) {
        disableSearchText();
    }

    QVector<ChatLine::Ptr> lines = chatWidget->getLines();

    if (lines.isEmpty()) {
        return isSearch;
    }

    int numLines = lines.size();
    int startLine = numLines - searchPoint.x();

    if (startLine < 0 || startLine >= numLines) {
        return isSearch;
    }

    for (int i = startLine; searchUp ? i >= 0 : i < numLines; searchUp ? --i : ++i) {
        ChatLine::Ptr l = lines[i];

        if (l->getColumnCount() < 2) {
            continue;
        }

        ChatLineContent* content = l->getContent(1);
        Text* text = static_cast<Text*>(content);

        if (searchUp && searchPoint.y() == 0) {
            text->deselectText();
            searchPoint.setY(-1);

            continue;
        }

        QString txt = content->getText();

        if (!txt.contains(phrase, Qt::CaseInsensitive)) {
            continue;
        }

        int index = indexForSearchInLine(txt, phrase, searchUp);
        if ((index == -1 && searchPoint.y() > -1)) {
            text->deselectText();
            searchPoint.setY(-1);
        } else {
            chatWidget->scrollToLine(l);
            text->deselectText();
            text->selectText(phrase, index);
            searchPoint = QPoint(numLines - i, index);
            isSearch = true;

            break;
        }
    }

    return isSearch;
}

int GenericChatForm::indexForSearchInLine(const QString& txt, const QString& phrase, bool searchUp)
{
    int index = 0;

    if (searchUp) {
        int startIndex = -1;
        if (searchPoint.y() > -1) {
            startIndex = searchPoint.y() - 1;
        }

        index = txt.lastIndexOf(phrase, startIndex, Qt::CaseInsensitive);
    } else {
        int startIndex = 0;
        if (searchPoint.y() > -1) {
            startIndex = searchPoint.y() + 1;
        }

        index = txt.indexOf(phrase, startIndex, Qt::CaseInsensitive);
    }

    return index;
}

void GenericChatForm::clearChatArea()
{
    clearChatArea(true);
}

void GenericChatForm::clearChatArea(bool notinform)
{
    QMessageBox::StandardButton mboxResult =
        QMessageBox::question(this, tr("Confirmation"),
                              tr("You are sure that you want to clear all displayed messages?"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (mboxResult == QMessageBox::No) {
        return;
    }
    chatWidget->clear();
    previousId = ToxPk();

    if (!notinform)
        addSystemInfoMessage(tr("Cleared"), ChatMessage::INFO, QDateTime::currentDateTime());

    earliestMessage = QDateTime(); // null

    emit chatAreaCleared();
}

void GenericChatForm::onSelectAllClicked()
{
    chatWidget->selectAll();
}

void GenericChatForm::insertChatMessage(ChatMessage::Ptr msg)
{
    chatWidget->insertChatlineAtBottom(std::static_pointer_cast<ChatLine>(msg));
    emit messageInserted();
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
    EmoticonsWidget* ev = qobject_cast<EmoticonsWidget*>(object);
    if (ev && event->type() == QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        msgEdit->sendKeyEvent(key);
        msgEdit->setFocus();
        return false;
    }

    if (object != this->fileButton && object != this->fileFlyout)
        return false;

    if (!qobject_cast<QWidget*>(object)->isEnabled())
        return false;

    switch (event->type()) {
    case QEvent::Enter:
        showFileMenu();
        break;

    case QEvent::Leave: {
        QPoint flyPos = fileFlyout->mapToGlobal(QPoint());
        QSize flySize = fileFlyout->size();

        QPoint filePos = fileButton->mapToGlobal(QPoint());
        QSize fileSize = fileButton->size();

        QRect region = QRect(flyPos, flySize).united(QRect(filePos, fileSize));

        if (!region.contains(QCursor::pos()))
            hideFileMenu();

        break;
    }

    case QEvent::MouseButtonPress:
        hideFileMenu();
        break;

    default:
        break;
    }

    return false;
}

void GenericChatForm::onSplitterMoved(int, int)
{
    if (netcam)
        netcam->setShowMessages(bodySplitter->sizes()[1] == 0);
}

void GenericChatForm::onShowMessagesClicked()
{
    if (netcam) {
        if (bodySplitter->sizes()[1] == 0)
            bodySplitter->setSizes({1, 1});
        else
            bodySplitter->setSizes({1, 0});

        onSplitterMoved(0, 0);
    }
}

void GenericChatForm::quoteSelectedText()
{
    QString selectedText = chatWidget->getSelectedText();

    if (selectedText.isEmpty())
        return;

    // forming pretty quote text
    // 1. insert "> " to the begining of quote;
    // 2. replace all possible line terminators with "\n> ";
    // 3. append new line to the end of quote.
    QString quote = selectedText;

    quote.insert(0, "> ");
    quote.replace(QRegExp(QString("\r\n|[\r\n\u2028\u2029]")), QString("\n> "));
    quote.append("\n");

    msgEdit->append(quote);
}

/**
 * @brief Callback of GenericChatForm::copyLinkAction
 */
void GenericChatForm::copyLink()
{
    QString linkText = copyLinkAction->data().toString();
    QApplication::clipboard()->setText(linkText);
}

void GenericChatForm::searchFormShow()
{
    if (searchForm->isHidden()) {
        searchForm->show();
        searchForm->setFocusEditor();
    }
}

void GenericChatForm::onSearchTriggered()
{
    if (searchForm->isHidden()) {
        searchForm->removeSearchPhrase();

        disableSearchText();
    } else {
        searchPoint = QPoint(1, -1);
        searchAfterLoadHistory = false;
    }
}

void GenericChatForm::searchInBegin(const QString& phrase)
{
    disableSearchText();

    searchPoint = QPoint(1, -1);
    onSearchUp(phrase);
}

void GenericChatForm::onContinueSearch()
{
    QString phrase = searchForm->getSearchPhrase();
    if (!phrase.isEmpty() && searchAfterLoadHistory) {
        searchAfterLoadHistory = false;
        onSearchUp(phrase);
    }
}

void GenericChatForm::retranslateUi()
{
    sendButton->setToolTip(tr("Send message"));
    emoteButton->setToolTip(tr("Smileys"));
    fileButton->setToolTip(tr("Send file(s)"));
    screenshotButton->setToolTip(tr("Send a screenshot"));
    saveChatAction->setText(tr("Save chat log"));
    clearAction->setText(tr("Clear displayed messages"));
    quoteAction->setText(tr("Quote selected text"));
    copyLinkAction->setText(tr("Copy link address"));
    searchAction->setText(tr("Search in text"));
}

void GenericChatForm::showNetcam()
{
    if (!netcam)
        netcam = createNetcam();

    connect(netcam, &GenericNetCamView::showMessageClicked, this,
            &GenericChatForm::onShowMessagesClicked);

    bodySplitter->insertWidget(0, netcam);
    bodySplitter->setCollapsible(0, false);

    QSize minSize = netcam->getSurfaceMinSize();
    ContentDialog* current = ContentDialog::current();
    if (current)
        current->onVideoShow(minSize);
}

void GenericChatForm::hideNetcam()
{
    if (!netcam)
        return;

    ContentDialog* current = ContentDialog::current();
    if (current)
        current->onVideoHide();

    netcam->close();
    netcam->hide();
    delete netcam;
    netcam = nullptr;
}
