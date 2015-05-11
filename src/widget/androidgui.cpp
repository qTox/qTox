#include "androidgui.h"
#include "ui_android.h"
#include "friendlistwidget.h"
#include "maskablepixmapwidget.h"
#include "src/core/core.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/group.h"
#include "src/grouplist.h"
#include "src/misc/settings.h"
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

void AndroidGUI::onUsernameChanged(const QString& newUsername, const QString& oldUsername)
{
    setUsername(oldUsername);               // restore old username until Core tells us to set it
    Nexus::getCore()->setUsername(newUsername);
}

void AndroidGUI::setUsername(const QString& username)
{
    QString sanename = username;
    sanename.remove(QRegExp("[\\t\\n\\v\\f\\r\\x0000]"));
             nameMention = QRegExp("\\b" + QRegExp::escape(username) + "\\b", Qt::CaseInsensitive);
    sanitizedNameMention = QRegExp("\\b" + QRegExp::escape(sanename) + "\\b", Qt::CaseInsensitive);
}

void AndroidGUI::onStatusMessageChanged(const QString& newStatusMessage, const QString& oldStatusMessage)
{
    Nexus::getCore()->setStatusMessage(newStatusMessage);
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
