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

#include "src/net/toxuri.h"
#include "src/core/core.h"
#include "src/net/toxme.h"
#include "src/nexus.h"
#include "src/widget/gui.h"
#include "src/widget/tool/friendrequestdialog.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QString>
#include <QThread>
#include <QVBoxLayout>

bool toxURIEventHandler(const QByteArray& eventData)
{
    if (!eventData.startsWith("tox:"))
        return false;

    handleToxURI(eventData);
    return true;
}

/**
 * @brief Shows a dialog asking whether or not to add this tox address as a friend.
 * @note Will wait until the core is ready first.
 * @param toxURI Tox URI to try to add.
 * @return True, if tox URI is correct, false otherwise.
 */
bool handleToxURI(const QString& toxURI)
{
    Nexus& nexus = Nexus::getInstance();
    Core* core = nexus.getCore();

    while (!core) {
        core = nexus.getCore();
        qApp->processEvents();
        QThread::msleep(10);
    }

    QString toxaddr = toxURI.mid(4);

    ToxId toxId(toxaddr);
    QString error = QString();
    if (!toxId.isValid()) {
        toxId = Toxme::lookup(toxaddr);
        if (!toxId.isValid()) {
            error = QMessageBox::tr("%1 is not a valid Toxme address.").arg(toxaddr);
        }
    } else if (toxId == core->getSelfId()) {
        error = QMessageBox::tr("You can't add yourself as a friend!",
                                "When trying to add your own Tox ID as friend");
    }

    if (!error.isEmpty()) {
        GUI::showWarning(QMessageBox::tr("Couldn't add friend"), error);
        return false;
    }

    const QString defaultMessage =
        QObject::tr("%1 here! Tox me maybe?",
                    "Default message in Tox URI friend requests. Write something appropriate!");
    const QString username = Nexus::getCore()->getUsername();
    ToxURIDialog* dialog = new ToxURIDialog(nullptr, toxaddr, defaultMessage.arg(username));
    QObject::connect(dialog, &ToxURIDialog::finished, [=](int result) {
        if (result == QDialog::Accepted) {
            Core::getInstance()->requestFriendship(toxId, dialog->getRequestMessage());
        }

        dialog->deleteLater();
    });

    dialog->open();

    return true;
}

ToxURIDialog::ToxURIDialog(QWidget* parent, const QString& userId, const QString& message)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Add a friend", "Title of the window to add a friend through Tox URI"));

    QLabel* friendsLabel = new QLabel(tr("Do you want to add %1 as a friend?").arg(userId), this);
    QLabel* userIdLabel = new QLabel(tr("User ID:"), this);
    QLineEdit* userIdEdit = new QLineEdit(userId, this);
    userIdEdit->setCursorPosition(0);
    userIdEdit->setReadOnly(true);
    QLabel* messageLabel = new QLabel(tr("Friend request message:"), this);
    messageEdit = new QPlainTextEdit(message, this);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal, this);

    buttonBox->addButton(tr("Send", "Send a friend request"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Cancel", "Don't send a friend request"), QDialogButtonBox::RejectRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FriendRequestDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FriendRequestDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(friendsLabel);
    layout->addSpacing(12);
    layout->addWidget(userIdLabel);
    layout->addWidget(userIdEdit);
    layout->addWidget(messageLabel);
    layout->addWidget(messageEdit);
    layout->addWidget(buttonBox);

    resize(300, 200);
}

QString ToxURIDialog::getRequestMessage()
{
    return messageEdit->toPlainText();
}
