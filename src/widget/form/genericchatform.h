/*
    Copyright © 2014-2017 by The qTox Project Contributors

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

#ifndef GENERICCHATFORM_H
#define GENERICCHATFORM_H

#include "src/chatlog/chatmessage.h"
#include "src/core/toxid.h"

#include <QMenu>
#include <QWidget>

/**
 * Spacing in px inserted when the author of the last message changes
 * @note Why the hell is this a thing? surely the different font is enough?
 *        - Even a different font is not enough – TODO #1307 ~~zetok
 */

QString resolveToxId(const ToxPk& id);

class ChatLog;
class ChatTextEdit;
class ContentLayout;
class CroppingLabel;
class FlyoutOverlayWidget;
class GenericNetCamView;
class MaskablePixmapWidget;
class Widget;

class QLabel;
class QPushButton;
class QSplitter;
class QToolButton;
class QVBoxLayout;

namespace Ui {
class MainWindow;
}

class GenericChatForm : public QWidget
{
    Q_OBJECT
public:
    explicit GenericChatForm(QWidget* parent = nullptr);
    ~GenericChatForm();

    void setName(const QString& newName);
    virtual void show() final
    {
    }
    virtual void show(ContentLayout* contentLayout);

    void addMessage(const ToxPk& author, const QString& message, const QDateTime& datetime,
                    bool isAction, bool isSent);
    void addSelfMessage(const QString& message, const QDateTime& datetime,
                        bool isAction, bool isSent);
    void addSystemInfoMessage(const QString& message, ChatMessage::SystemMessageType type,
                              const QDateTime& datetime);
    void addAlertMessage(const ToxPk& author, const QString& message, const QDateTime& datetime);
    QDate getLatestDate() const;

signals:
    void sendMessage(uint32_t, QString);
    void sendAction(uint32_t, QString);
    void chatAreaCleared();
    void messageInserted();

public slots:
    void focusInput();
    void onChatMessageFontChanged(const QFont& font);

protected slots:
    void onChatContextMenuRequested(QPoint pos);
    void onEmoteButtonClicked();
    void onEmoteInsertRequested(QString str);
    void onSaveLogClicked();
    void onCopyLogClicked();
    virtual void clearChatArea(bool);
    void clearChatArea();
    void onSelectAllClicked();
    void showFileMenu();
    void hideFileMenu();
    void onShowMessagesClicked();
    void onSplitterMoved(int pos, int index);
    void quoteSelectedText();
    void copyLink();

private:
    void retranslateUi();

protected:
    ChatMessage::Ptr createMessage(const ToxPk& author, const QString& message,
                                   const QDateTime& datetime, bool isAction, bool isSent);
    ChatMessage::Ptr createSelfMessage(const QString& message, const QDateTime& datetime,
                                       bool isAction, bool isSent);
    void showNetcam();
    void hideNetcam();
    virtual GenericNetCamView* createNetcam() = 0;
    virtual void insertChatMessage(ChatMessage::Ptr msg);
    void adjustFileMenuPosition();
    virtual void hideEvent(QHideEvent* event) override;
    virtual void showEvent(QShowEvent*) override;
    virtual bool event(QEvent*) final override;
    virtual void resizeEvent(QResizeEvent* event) final override;
    virtual bool eventFilter(QObject* object, QEvent* event) final override;

protected:
    bool audioInputFlag;
    bool audioOutputFlag;
    int curRow;

    QAction* saveChatAction;
    QAction* clearAction;
    QAction* quoteAction;
    QAction* copyLinkAction;

    ToxPk previousId;

    QDateTime prevMsgDateTime;
    QDateTime earliestMessage;
    QDateTime historyBaselineDate = QDateTime::currentDateTime();

    QMenu menu;

    QPushButton* callButton;
    QPushButton* emoteButton;
    QPushButton* fileButton;
    QPushButton* screenshotButton;
    QPushButton* sendButton;
    QPushButton* videoButton;

    QSplitter* bodySplitter;

    QToolButton* volButton;
    QToolButton* micButton;

    QVBoxLayout* headTextLayout;

    QWidget* headWidget;

    ChatLog* chatWidget;
    ChatTextEdit* msgEdit;
    CroppingLabel* nameLabel;
    FlyoutOverlayWidget* fileFlyout;
    GenericNetCamView* netcam;
    MaskablePixmapWidget* avatar;
    Widget* parent;
};

#endif // GENERICCHATFORM_H
