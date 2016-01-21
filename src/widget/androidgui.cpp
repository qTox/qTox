/*
    Copyright © 2015 by The qTox Project

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


#include "androidgui.h"
#include "ui_android.h"
#include "friendlistwidget.h"
#include "maskablepixmapwidget.h"
#include "src/core/core.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/group.h"
#include "src/grouplist.h"
#include "src/persistence/settings.h"
#include "src/misc/style.h"
#include "src/nexus.h"
#include "src/widget/friendwidget.h"
#include "src/widget/groupwidget.h"
#include <QDebug>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QFontDatabase>
#include <QFont>

AndroidGUI::AndroidGUI(QWidget *parent) :
    QWidget(parent),
    ui{new Ui::Android}
{
    ui->setupUi(this);

    QFontDatabase::addApplicationFont(":/res/android/Roboto-Bold.ttf");
    QFont font("Roboto1200310", 24);
    qDebug() << "Font: "<<font.family();
    ui->headTitle->setFont(font);

    Q_INIT_RESOURCE(android);
}

AndroidGUI::~AndroidGUI()
{

}

void AndroidGUI::reloadTheme()
{

}

void AndroidGUI::onSelfAvatarLoaded(const QPixmap& pic)
{

}

void AndroidGUI::onConnected()
{
    if (beforeDisconnect == Status::Offline)
        emit statusSet(Status::Online);
    else
        emit statusSet(beforeDisconnect);
}

void AndroidGUI::onDisconnected()
{
    emit statusSet(Status::Offline);
}

void AndroidGUI::setUsername(const QString& username)
{
    QString sanename = username;
    sanename.remove(QRegExp("[\\t\\n\\v\\f\\r\\x0000]"));
             nameMention = QRegExp("\\b" + QRegExp::escape(username) + "\\b", Qt::CaseInsensitive);
    sanitizedNameMention = QRegExp("\\b" + QRegExp::escape(sanename) + "\\b", Qt::CaseInsensitive);
}

void AndroidGUI::setStatusMessage(const QString &statusMessage)
{

}

void AndroidGUI::onStatusSet(Status status)
{

}

void AndroidGUI::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Back)
    {
        qDebug() << "Back key pressed, quitting";
        qApp->exit(0);
    }
    else if (event->key() == Qt::Key_Menu)
    {
        qDebug() << "Menu key pressed";
    }
}
