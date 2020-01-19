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

#include "src/chatlog/chatlog.h"
#include "src/chatlog/chatmessage.h"

#include <QtTest/QtTest>
#include <QScrollBar>

class TestChatlog : public QObject
{
    Q_OBJECT
private slots:    
    void initTestCase();
    void cleanupTestCase();

    void insertChatlineAtBottomTest();
private:    
    void addMessages(int count, int delay);
    void addMessagesList(int count, int cntListMsg, int delay);
    ChatLog* chatlog;
    QScrollBar* scbar;
    int idMsg = 0;
};

void TestChatlog::initTestCase()
{
    chatlog = new ChatLog(false);
    chatlog->setBackgroundBrush(QBrush(Qt::white));
    chatlog->setScroll(true);
    chatlog->resize(800, 600);
    chatlog->show();

    scbar = chatlog->verticalScrollBar();
    scbar->setEnabled(false);
}

void TestChatlog::cleanupTestCase()
{
    chatlog->verticalScrollBar()->setEnabled(true);
    //QApplication::exec(); // No close
    delete chatlog;
}

void TestChatlog::insertChatlineAtBottomTest()
{        
    const int delayMsg = 500; // delay between messages
    const int delayWrk = 500; // delay needed for WorkerTimer
    const int cntListMsg = 3; // count of messages in list
    const int cnt = 5; // loop repeat
    int cntAllMsg = 50; // count total messages
    int scrollVal; // scrollbar value

    chatlog->clear();

    addMessages(cntAllMsg, 0);

    for (int i = 0; i < cnt; ++i) {
        cntAllMsg += (i + i * cntListMsg) * 3;

        // Stick to bottom
        chatlog->scrollToLine(chatlog->getLines().last());
        addMessages(i, delayMsg);
        QCOMPARE(scbar->maximum(), scbar->value());
        addMessagesList(i, cntListMsg, delayMsg);
        QCOMPARE(scbar->maximum(), scbar->value());
        QTest::qWait(delayWrk * i);

        // Stick to center
        chatlog->scrollToLine(chatlog->getLines().at(chatlog->getLines().size() / 2));
        scrollVal = scbar->value();
        addMessages(i, delayMsg);
        QCOMPARE(scrollVal, scbar->value());
        addMessagesList(i, cntListMsg, delayMsg);
        QCOMPARE(scrollVal, scbar->value());
        QTest::qWait(delayWrk * i);

        // Stick to top
        chatlog->scrollToLine(chatlog->getLines().first());
        scrollVal = scbar->value();
        addMessages(i, delayMsg);
        QCOMPARE(scrollVal, scbar->value());
        addMessagesList(i, cntListMsg, delayMsg);
        QCOMPARE(scrollVal, scbar->value());
        QTest::qWait(delayWrk * i);
    }

    if (cntAllMsg != chatlog->getLines().count()) {
        QFAIL("expected count(" + QString::number(cntAllMsg).toLocal8Bit() + ") != "
              "actual count(" + QString::number(chatlog->getLines().count()).toLocal8Bit() + ")");
    }
}

void TestChatlog::addMessages(int count, int delay)
{
    for (int i = 0; i < count; ++i, ++idMsg) {
        QString msgText(QStringLiteral("Bottom message "));
        ChatMessage::Ptr msg = ChatMessage::createChatMessage(QStringLiteral("Sender"),
                                                              msgText + QString::number(idMsg),
                                                              ChatMessage::NORMAL,
                                                              true,
                                                              MessageState::complete,
                                                              QDateTime::currentDateTime(),
                                                              false);
        chatlog->insertChatlineAtBottom(std::static_pointer_cast<ChatLine>(msg));
        QTest::qWait(delay);
    }
}

void TestChatlog::addMessagesList(int count, int cntListMsg, int delay)
{
    for (int i = 0; i < count; ++i) {
        QList<ChatLine::Ptr> messages;
        for (int j = 0; j < cntListMsg; ++j, ++idMsg) {
            QString msgText(QStringLiteral("Bottom message from list "));
            messages.push_back(ChatMessage::createChatMessage(QStringLiteral("Sender"),
                                                               msgText + QString::number(idMsg),
                                                               ChatMessage::NORMAL,
                                                               true,
                                                               MessageState::complete,
                                                               QDateTime::currentDateTime(),
                                                               false));

            QTest::qWait(delay);

        }
        chatlog->insertChatlineAtBottom(messages);
        QTest::qWait(delay);
    }
}

QTEST_MAIN(TestChatlog)
#include "chatlog_test.moc"
