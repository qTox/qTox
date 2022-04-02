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

#include "src/chatlog/chatmessage.h"
#include "src/core/toxpk.h"
#include "src/model/ichatlog.h"
#include "src/widget/searchtypes.h"

#include <QMenu>
#include <QWidget>

/**
 * Spacing in px inserted when the author of the last message changes
 * @note Why the hell is this a thing? surely the different font is enough?
 *        - Even a different font is not enough – TODO #1307 ~~zetok
 */

class ChatFormHeader;
class ChatWidget;
class ChatTextEdit;
class Chat;
class ContentLayout;
class CroppingLabel;
class FlyoutOverlayWidget;
class GenericNetCamView;
class MaskablePixmapWidget;
class SearchForm;
class Widget;

class QLabel;
class QPushButton;
class QSplitter;
class QToolButton;
class QVBoxLayout;

class IMessageDispatcher;
struct Message;
class DocumentCache;
class SmileyPack;
class Settings;
class Style;
class IMessageBoxManager;
class FriendList;

namespace Ui {
class MainWindow;
}

#ifdef SPELL_CHECKING
namespace Sonnet {
class SpellCheckDecorator;
}
#endif

class GenericChatForm : public QWidget
{
    Q_OBJECT
public:
    GenericChatForm(const Core& core_, const Chat* chat, IChatLog& chatLog_,
                    IMessageDispatcher& messageDispatcher_, DocumentCache& documentCache,
                    SmileyPack& smileyPack, Settings& settings, Style& style,
                    IMessageBoxManager& messageBoxmanager, FriendList& friendList,
                    GroupList& groupList, QWidget* parent_ = nullptr);
    ~GenericChatForm() override;

    void setName(const QString& newName);
    virtual void show(ContentLayout* contentLayout_);

    void addSystemInfoMessage(const QDateTime& datetime, SystemMessageType messageType,
                              SystemMessage::Args messageArgs);
    QString resolveToxPk(const ToxPk& pk);
    QDateTime getLatestTime() const;

signals:
    void messageInserted();

public slots:
    void focusInput();
    void onChatMessageFontChanged(const QFont& font);
    void setColorizedNames(bool enable);
    virtual void reloadTheme();

protected slots:
    void onChatContextMenuRequested(QPoint pos);
    virtual void onScreenshotClicked() = 0;
    void onSendTriggered();
    virtual void onAttachClicked() = 0;
    void onEmoteButtonClicked();
    void onEmoteInsertRequested(QString str);
    void onCopyLogClicked();
    void clearChatArea();
    void clearChatArea(bool confirm, bool inform);
    void onSelectAllClicked();
    void showFileMenu();
    void hideFileMenu();
    void quoteSelectedText();
    void copyLink();
    void onLoadHistory();
    void onExportChat();
    void searchFormShow();
    void updateShowDateInfo(const ChatLine::Ptr& prevLine, const ChatLine::Ptr& topLine);
    void goToCurrentDate();

private:
    void retranslateUi();
    void addSystemDateMessage(const QDate& date);
    QDateTime getTime(const ChatLine::Ptr& chatLine) const;

protected:
    ChatMessage::Ptr createMessage(const ToxPk& author, const QString& message,
                                   const QDateTime& datetime, bool isAction, bool isSent, bool colorizeName = false);
    void adjustFileMenuPosition();
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;
    bool event(QEvent* event) final;
    void resizeEvent(QResizeEvent* event) final;
    bool eventFilter(QObject* object, QEvent* event) final;
    bool searchInText(const QString& phrase, const ParameterSearch& parameter, SearchDirection direction);
    std::pair<int, int> indexForSearchInLine(const QString& txt, const QString& phrase, const ParameterSearch& parameter, SearchDirection direction);

protected:
    const Core& core;
    bool audioInputFlag;
    bool audioOutputFlag;
    int curRow;

    QAction* clearAction;
    QAction* quoteAction;
    QAction* copyLinkAction;
    QAction* searchAction;
    QAction* goToCurrentDateAction;
    QAction* loadHistoryAction;
    QAction* exportChatAction;

    QMenu menu;

    QVBoxLayout* contentLayout;
    QPushButton* emoteButton;
    QPushButton* fileButton;
    QPushButton* screenshotButton;
    QPushButton* sendButton;

    QSplitter* bodySplitter;

    ChatFormHeader* headWidget;

    SearchForm *searchForm;
    QLabel *dateInfo;
    ChatWidget* chatWidget;
    ChatTextEdit* msgEdit;
#ifdef SPELL_CHECKING
    Sonnet::SpellCheckDecorator* decorator{nullptr};
#endif
    FlyoutOverlayWidget* fileFlyout;
    Widget* parent;

    IChatLog& chatLog;
    IMessageDispatcher& messageDispatcher;
    SmileyPack& smileyPack;
    Settings& settings;
    Style& style;
    FriendList& friendList;
    GroupList& groupList;
};
