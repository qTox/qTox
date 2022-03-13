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

#include "src/net/toxuri.h"
#include "src/core/core.h"
#include "src/widget/tool/imessageboxmanager.h"
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

/**
 * @brief Shows a dialog asking whether or not to add this tox address as a friend.
 * @note Will wait until the core is ready first.
 * @param toxURI Tox URI to try to add.
 * @return True, if tox URI is correct, false otherwise.
 */
bool ToxURIDialog::handleToxURI(const QString& toxURI)
{
    QString toxaddr = toxURI.mid(4);

    ToxId toxId(toxaddr);
    QString error = QString();
    if (!toxId.isValid()) {
        error = QMessageBox::tr("%1 is not a valid Tox address.").arg(toxaddr);
    } else if (toxId == core.getSelfId()) {
        error = QMessageBox::tr("You can't add yourself as a friend!",
                                "When trying to add your own Tox ID as friend");
    }

    if (!error.isEmpty()) {
        messageBoxManager.showWarning(QMessageBox::tr("Couldn't add friend"), error);
        return false;
    }

    setUserId(toxURI);

    int result = exec();
    if (result == QDialog::Accepted) {
        core.requestFriendship(toxId, getRequestMessage());
    }

    return true;
}

void ToxURIDialog::setUserId(const QString& userId)
{
    friendsLabel->setText(tr("Do you want to add %1 as a friend?").arg(userId));
    userIdEdit->setText(userId);
}

ToxURIDialog::ToxURIDialog(QWidget* parent, Core& core_, IMessageBoxManager& messageBoxManager_)
    : QDialog(parent)
    , core{core_}
    , messageBoxManager{messageBoxManager_}
{
    const QString defaultMessage =
        QObject::tr("%1 here! Tox me maybe?",
                    "Default message in Tox URI friend requests. Write something appropriate!");
    const QString username = core.getUsername();
    const QString message = defaultMessage.arg(username);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Add a friend", "Title of the window to add a friend through Tox URI"));

    friendsLabel = new QLabel("", this);
    userIdEdit = new QLineEdit("", this);
    userIdEdit->setCursorPosition(0);
    userIdEdit->setReadOnly(true);

    QLabel* userIdLabel = new QLabel(tr("User ID:"), this);
    QLabel* messageLabel = new QLabel(tr("Friend request message:"), this);
    messageEdit = new QPlainTextEdit(message, this);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal, this);

    buttonBox->addButton(tr("Send", "Send a friend request"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Cancel", "Don't send a friend request"), QDialogButtonBox::RejectRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

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
