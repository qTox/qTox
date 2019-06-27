/*
    Copyright © 2019 by The qTox Project Contributors

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

#ifndef SESSION_CHAT_LOG_H
#define SESSION_CHAT_LOG_H

#include "ichatlog.h"
#include "imessagedispatcher.h"

#include <QList>
#include <QObject>

struct SessionChatLogMetadata;


class SessionChatLog : public IChatLog
{
    Q_OBJECT
public:
    SessionChatLog(const ICoreIdHandler& coreIdHandler);
    SessionChatLog(ChatLogIdx initialIdx, const ICoreIdHandler& coreIdHandler);

    ~SessionChatLog();
    const ChatLogItem& at(ChatLogIdx idx) const override;
    SearchResult searchForward(SearchPos startIdx, const QString& phrase,
                               const ParameterSearch& parameter) const override;
    SearchResult searchBackward(SearchPos startIdx, const QString& phrase,
                                const ParameterSearch& parameter) const override;
    ChatLogIdx getFirstIdx() const override;
    ChatLogIdx getNextIdx() const override;
    std::vector<DateChatLogIdxPair> getDateIdxs(const QDate& startDate, size_t maxDates) const override;
    std::size_t size() const override;

    void insertMessageAtIdx(ChatLogIdx idx, ToxPk sender, QString senderName, ChatLogMessage message);
    void insertFileAtIdx(ChatLogIdx idx, ToxPk sender, QString senderName, ChatLogFile file);

public slots:
    void onMessageReceived(const ToxPk& sender, const Message& message);
    void onMessageSent(DispatchedMessageId id, const Message& message);
    void onMessageComplete(DispatchedMessageId id);

    void onFileUpdated(const ToxPk& sender, const ToxFile& file);
    void onFileTransferRemotePausedUnpaused(const ToxPk& sender, const ToxFile& file, bool paused);
    void onFileTransferBrokenUnbroken(const ToxPk& sender, const ToxFile& file, bool broken);

private:
    const ICoreIdHandler& coreIdHandler;

    ChatLogIdx nextIdx = ChatLogIdx(0);

    std::map<ChatLogIdx, ChatLogItem> items;

    struct CurrentFileTransfer
    {
        ChatLogIdx idx;
        ToxFile file;
    };

    /**
     * Short list of active file transfers in given log. This is to make it
     * so we don't have to search through all files that have ever been transferred
     * in order to find our existing transfers
     */
    std::vector<CurrentFileTransfer> currentFileTransfers;

    /**
     * Maps DispatchedMessageIds back to ChatLogIdxs. Messages are removed when the message
     * is marked as completed
     */
    QMap<DispatchedMessageId, ChatLogIdx> outgoingMessages;
};

#endif /*SESSION_CHAT_LOG_H*/
