#include "androidgui.h"
#include "ui_android.h"
#include "friendlistwidget.h"
#include "maskablepixmapwidget.h"
#include "src/core.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/group.h"
#include "src/grouplist.h"
#include "src/misc/settings.h"
#include "src/misc/style.h"
#include "src/nexus.h"
#include "src/widget/friendwidget.h"
#include "src/widget/groupwidget.h"
#include <QLabel>
#include <QMenu>

AndroidGUI::AndroidGUI(QWidget *parent) :
    QWidget(parent),
    ui{new Ui::Android}
{
    ui->setupUi(this);

    ui->friendList->setStyleSheet(Style::resolve(Style::getStylesheet(":ui/friendList/friendList.css")));

    profilePicture = new MaskablePixmapWidget(this, QSize(40, 40), ":/img/avatar_mask.png");
    profilePicture->setPixmap(QPixmap(":/img/contact_dark.png"));
    profilePicture->setClickable(true);
    ui->myProfile->insertWidget(0, profilePicture);
    ui->myProfile->insertSpacing(1, 7);

    ui->tooliconsZone->setStyleSheet(Style::resolve("QPushButton{background-color:@themeDark;border:none;}QPushButton:hover{background-color:@themeMediumDark;border:none;}"));
    ui->statusHead->setStyleSheet(Style::getStylesheet(":/ui/window/statusPanel.css"));

    contactListWidget = new FriendListWidget();
    ui->friendList->setWidget(contactListWidget);
    ui->friendList->setLayoutDirection(Qt::RightToLeft);

    ui->nameLabel->setEditable(true);
    ui->statusLabel->setEditable(true);

    ui->statusPanel->setStyleSheet(Style::getStylesheet(":/ui/window/statusPanel.css"));

    QMenu *statusButtonMenu = new QMenu(ui->statusButton);
    QAction* setStatusOnline = statusButtonMenu->addAction(AndroidGUI::tr("Online","Button to set your status to 'Online'"));
    setStatusOnline->setIcon(QIcon(":ui/statusButton/dot_online.png"));
    QAction* setStatusAway = statusButtonMenu->addAction(AndroidGUI::tr("Away","Button to set your status to 'Away'"));
    setStatusAway->setIcon(QIcon(":ui/statusButton/dot_idle.png"));
    QAction* setStatusBusy = statusButtonMenu->addAction(AndroidGUI::tr("Busy","Button to set your status to 'Busy'"));
    setStatusBusy->setIcon(QIcon(":ui/statusButton/dot_busy.png"));
    ui->statusButton->setMenu(statusButtonMenu);

    ui->statusButton->setProperty("status", "offline");
    Style::repolish(ui->statusButton);

    // Disable some widgets until we're connected to the DHT
    ui->statusButton->setEnabled(false);

    Style::setThemeColor(Settings::getInstance().getThemeColor());
    Style::setThemeColor(1);
    reloadTheme();

    connect(ui->nameLabel, &CroppingLabel::textChanged, this, &AndroidGUI::onUsernameChanged);
    connect(ui->statusLabel, &CroppingLabel::textChanged, this, &AndroidGUI::onStatusMessageChanged);
}

AndroidGUI::~AndroidGUI()
{
    delete profilePicture;
    delete contactListWidget;
}

void AndroidGUI::reloadTheme()
{
    QString statusPanelStyle = Style::getStylesheet(":/ui/window/statusPanel.css");
    ui->tooliconsZone->setStyleSheet(Style::resolve("QPushButton{background-color:@themeDark;border:none;}QPushButton:hover{background-color:@themeMediumDark;border:none;}"));
    ui->statusPanel->setStyleSheet(statusPanelStyle);
    ui->statusHead->setStyleSheet(statusPanelStyle);
    ui->friendList->setStyleSheet(Style::getStylesheet(":ui/friendList/friendList.css"));
    ui->statusButton->setStyleSheet(Style::getStylesheet(":ui/statusButton/statusButton.css"));

    for (Friend* f : FriendList::getAllFriends())
        f->getFriendWidget()->reloadTheme();

    for (Group* g : GroupList::getAllGroups())
        g->getGroupWidget()->reloadTheme();
}

void AndroidGUI::onSelfAvatarLoaded(const QPixmap& pic)
{
    profilePicture->setPixmap(pic);
}

void AndroidGUI::onConnected()
{
    ui->statusButton->setEnabled(true);
    if (beforeDisconnect == Status::Offline)
        emit statusSet(Status::Online);
    else
        emit statusSet(beforeDisconnect);
}

void AndroidGUI::onDisconnected()
{
    QString stat = ui->statusButton->property("status").toString();
    if      (stat == "online")
        beforeDisconnect = Status::Online;
    else if (stat == "busy")
        beforeDisconnect = Status::Busy;
    else if (stat == "away")
        beforeDisconnect = Status::Away;
    else
        beforeDisconnect = Status::Offline;

    ui->statusButton->setEnabled(false);
    emit statusSet(Status::Offline);
}

void AndroidGUI::onUsernameChanged(const QString& newUsername, const QString& oldUsername)
{
    setUsername(oldUsername);               // restore old username until Core tells us to set it
    Nexus::getCore()->setUsername(newUsername);
}

void AndroidGUI::setUsername(const QString& username)
{
    ui->nameLabel->setText(username);
    ui->nameLabel->setToolTip(username);    // for overlength names
    QString sanename = username;
    sanename.remove(QRegExp("[\\t\\n\\v\\f\\r\\x0000]"));
             nameMention = QRegExp("\\b" + QRegExp::escape(username) + "\\b", Qt::CaseInsensitive);
    sanitizedNameMention = QRegExp("\\b" + QRegExp::escape(sanename) + "\\b", Qt::CaseInsensitive);
}

void AndroidGUI::onStatusMessageChanged(const QString& newStatusMessage, const QString& oldStatusMessage)
{
    ui->statusLabel->setText(oldStatusMessage); // restore old status message until Core tells us to set it
    ui->statusLabel->setToolTip(oldStatusMessage); // for overlength messsages
    Nexus::getCore()->setStatusMessage(newStatusMessage);
}

void AndroidGUI::setStatusMessage(const QString &statusMessage)
{
    ui->statusLabel->setText(statusMessage);
    ui->statusLabel->setToolTip(statusMessage); // for overlength messsages
}

void AndroidGUI::onStatusSet(Status status)
{
    //We have to use stylesheets here, there's no way to
    //prevent the button icon from moving when pressed otherwise
    switch (status)
    {
    case Status::Online:
        ui->statusButton->setProperty("status" ,"online");
        break;
    case Status::Away:
        ui->statusButton->setProperty("status" ,"away");
        break;
    case Status::Busy:
        ui->statusButton->setProperty("status" ,"busy");
        break;
    case Status::Offline:
        ui->statusButton->setProperty("status" ,"offline");
        break;
    }
    Style::repolish(ui->statusButton);
}

