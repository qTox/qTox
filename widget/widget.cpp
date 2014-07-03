#include "widget.h"
#include "ui_widget.h"
#include "settings.h"
#include "friend.h"
#include "friendlist.h"
#include "widget/tool/friendrequestdialog.h"
#include "widget/friendwidget.h"
#include "grouplist.h"
#include "group.h"
#include "widget/groupwidget.h"
#include "widget/form/groupchatform.h"
#include <QMessageBox>
#include <QDebug>
#include <QSound>
#include <QTextStream>
#include <QFile>
#include <QString>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QCursor>
#include <QSettings>
#include <QClipboard>

Widget *Widget::instance{nullptr};

Widget::Widget(QWidget *parent) :
    QWidget(parent), ui(new Ui::Widget), activeFriendWidget{nullptr}, activeGroupWidget{nullptr}
{
    ui->setupUi(this);

    QSettings settings(Settings::getInstance().getSettingsDirPath() + '/' + "windowSettings.ini", QSettings::IniFormat);
    if (!settings.contains("useNativeTheme"))
        useNativeTheme = 1;
    else
        useNativeTheme = settings.value("useNativeTheme").toInt();

    if (useNativeTheme)
    {
        ui->titleBar->hide();
        //setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint);
        this->layout()->setContentsMargins(0, 0, 0, 0);

        QString friendListStylesheet = "";
        try
        {
            QFile f(":ui/friendList/friendList.css");
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream friendListStylesheetStream(&f);
            friendListStylesheet = friendListStylesheetStream.readAll();
        }
        catch (int e) {}
        ui->friendList->setObjectName("friendList");
        ui->friendList->setStyleSheet(friendListStylesheet);
    }
    else
    {
        QString windowStylesheet = "";
        try
        {
            QFile f(":ui/window/window.css");
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream windowStylesheetStream(&f);
            windowStylesheet = windowStylesheetStream.readAll();
        }
        catch (int e) {}
        this->setObjectName("activeWindow");
        this->setStyleSheet(windowStylesheet);
        ui->statusPanel->setStyleSheet(QString(""));
        ui->friendList->setStyleSheet(QString(""));

        QString friendListStylesheet = "";
        try
        {
            QFile f(":ui/friendList/friendList.css");
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream friendListStylesheetStream(&f);
            friendListStylesheet = friendListStylesheetStream.readAll();
        }
        catch (int e) {}
        ui->friendList->setObjectName("friendList");
        ui->friendList->setStyleSheet(friendListStylesheet);

        ui->tbMenu->setIcon(QIcon(":ui/window/applicationIcon.png"));
        ui->pbMin->setObjectName("minimizeButton");
        ui->pbMax->setObjectName("maximizeButton");
        ui->pbClose->setObjectName("closeButton");

        setWindowFlags(Qt::CustomizeWindowHint);
        setWindowFlags(Qt::FramelessWindowHint);
        setAttribute(Qt::WA_DeleteOnClose);
        setMouseTracking(true);
        ui->titleBar->setMouseTracking(true);
        ui->LTitle->setMouseTracking(true);
        ui->tbMenu->setMouseTracking(true);
        ui->pbMin->setMouseTracking(true);
        ui->pbMax->setMouseTracking(true);
        ui->pbClose->setMouseTracking(true);

        addAction(ui->actionClose);

        connect(ui->pbMin, SIGNAL(clicked()), this, SLOT(minimizeBtnClicked()));
        connect(ui->pbMax, SIGNAL(clicked()), this, SLOT(maximizeBtnClicked()));
        connect(ui->pbClose, SIGNAL(clicked()), this, SLOT(close()));

        m_titleMode = FullTitle;
        moveWidget = false;
        inResizeZone = false;
        allowToResize = false;
        resizeVerSup = false;
        resizeHorEsq = false;
        resizeDiagSupEsq = false;
        resizeDiagSupDer = false;

    QSettings settings(Settings::getInstance().getSettingsDirPath() + '/' + "windowSettings.ini", QSettings::IniFormat);
    QRect geo = settings.value("geometry").toRect();

        if (geo.height() > 0 and geo.x() < QApplication::desktop()->width() and geo.width() > 0 and geo.y() < QApplication::desktop()->height())
            setGeometry(geo);

        if (settings.value("maximized").toBool())
        {
            showMaximized();
            ui->pbMax->setObjectName("restoreButton");
        }

        QList<QWidget*> widgets = this->findChildren<QWidget*>();

        foreach (QWidget *widget, widgets)
        {
            widget->setMouseTracking(true);
        }
    }

    isWindowMinimized = 0;

    centralLayout = new QHBoxLayout(ui->centralWidget);


    ui->mainContent->setLayout(new QVBoxLayout());
    ui->mainHead->setLayout(new QVBoxLayout());
    ui->mainHead->layout()->setMargin(0);
    ui->mainHead->layout()->setSpacing(0);
    QWidget* friendListWidget = new QWidget();
    friendListWidget->setLayout(new QVBoxLayout());
    friendListWidget->layout()->setSpacing(0);
    friendListWidget->layout()->setMargin(0);
    friendListWidget->setLayoutDirection(Qt::LeftToRight);
    ui->friendList->setWidget(friendListWidget);

    ui->nameLabel->setText(Settings::getInstance().getUsername());
    ui->nameLabel->label->setStyleSheet("QLabel { color : white; font-size: 11pt; font-weight:bold;}");
    ui->statusLabel->setText(Settings::getInstance().getStatusMessage());
    ui->statusLabel->label->setStyleSheet("QLabel { color : white; font-size: 8pt;}");
    ui->friendList->widget()->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    camera = new Camera;
    camview = new SelfCamView(camera);

    qRegisterMetaType<Status>("Status");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<ToxFile>("ToxFile");
    qRegisterMetaType<ToxFile::FileDirection>("ToxFile::FileDirection");

    core = new Core(camera);
    coreThread = new QThread(this);
    core->moveToThread(coreThread);
    connect(coreThread, &QThread::started, core, &Core::start);

    connect(core, &Core::connected, this, &Widget::onConnected);
    connect(core, &Core::disconnected, this, &Widget::onDisconnected);
    connect(core, &Core::failedToStart, this, &Widget::onFailedToStartCore);
    connect(core, &Core::statusSet, this, &Widget::onStatusSet);
    connect(core, &Core::usernameSet, this, &Widget::setUsername);
    connect(core, &Core::statusMessageSet, this, &Widget::setStatusMessage);
    connect(core, &Core::friendAddressGenerated, &settingsForm, &SettingsForm::setFriendAddress);
    connect(core, &Core::friendAdded, this, &Widget::addFriend);
    connect(core, &Core::failedToAddFriend, this, &Widget::addFriendFailed);
    connect(core, &Core::friendStatusChanged, this, &Widget::onFriendStatusChanged);
    connect(core, &Core::friendUsernameChanged, this, &Widget::onFriendUsernameChanged);
    connect(core, &Core::friendStatusChanged, this, &Widget::onFriendStatusChanged);
    connect(core, &Core::friendStatusMessageChanged, this, &Widget::onFriendStatusMessageChanged);
    connect(core, &Core::friendUsernameLoaded, this, &Widget::onFriendUsernameLoaded);
    connect(core, &Core::friendStatusMessageLoaded, this, &Widget::onFriendStatusMessageLoaded);
    connect(core, &Core::friendRequestReceived, this, &Widget::onFriendRequestReceived);
    connect(core, &Core::friendMessageReceived, this, &Widget::onFriendMessageReceived);
    connect(core, &Core::groupInviteReceived, this, &Widget::onGroupInviteReceived);
    connect(core, &Core::groupMessageReceived, this, &Widget::onGroupMessageReceived);
    connect(core, &Core::groupNamelistChanged, this, &Widget::onGroupNamelistChanged);
    connect(core, &Core::emptyGroupCreated, this, &Widget::onEmptyGroupCreated);

    connect(this, &Widget::statusSet, core, &Core::setStatus);
    connect(this, &Widget::friendRequested, core, &Core::requestFriendship);
    connect(this, &Widget::friendRequestAccepted, core, &Core::acceptFriendRequest);

    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(onAddClicked()));
    connect(ui->groupButton, SIGNAL(clicked()), this, SLOT(onGroupClicked()));
    connect(ui->transferButton, SIGNAL(clicked()), this, SLOT(onTransferClicked()));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(onSettingsClicked()));
    connect(ui->nameLabel, SIGNAL(textChanged(QString,QString)), this, SLOT(onUsernameChanged(QString,QString)));
    connect(ui->statusLabel, SIGNAL(textChanged(QString,QString)), this, SLOT(onStatusMessageChanged(QString,QString)));
    connect(ui->statImg, SIGNAL(clicked()), this, SLOT(onStatusImgClicked()));
    connect(&settingsForm.name, SIGNAL(textChanged(QString)), this, SLOT(onUsernameChanged(QString)));
    connect(&settingsForm.statusText, SIGNAL(textChanged(QString)), this, SLOT(onStatusMessageChanged(QString)));
    connect(&friendForm, SIGNAL(friendRequested(QString,QString)), this, SIGNAL(friendRequested(QString,QString)));

    coreThread->start();

    friendForm.show(*ui);
    isFriendWidgetActive = 0;
    isGroupWidgetActive = 0;
}

Widget::~Widget()
{
    instance = nullptr;
    coreThread->exit();
    coreThread->wait();
    delete core;
    delete camview;

    hideMainForms();

    for (Friend* f : FriendList::friendList)
        delete f;
    FriendList::friendList.clear();
    for (Group* g : GroupList::groupList)
        delete g;
    GroupList::groupList.clear();
    QSettings settings(Settings::getInstance().getSettingsDirPath() + '/' + "windowSettings.ini", QSettings::IniFormat);
    settings.setValue("geometry", geometry());
    settings.setValue("maximized", isMaximized());
    settings.setValue("useNativeTheme", useNativeTheme);
    delete ui;
    delete centralLayout;
}

Widget* Widget::getInstance()
{
    if (!instance)
        instance = new Widget();
    return instance;
}


QString Widget::getUsername()
{
    return ui->nameLabel->text();
}

Camera* Widget::getCamera()
{
    return camera;
}

void Widget::onConnected()
{
    emit statusSet(Status::Online);
}

void Widget::onDisconnected()
{
    emit statusSet(Status::Offline);
}

void Widget::onFailedToStartCore()
{
    QMessageBox critical(this);
    critical.setText("Toxcore failed to start, the application will terminate after you close this message.");
    critical.setIcon(QMessageBox::Critical);
    critical.exec();
    qApp->quit();
}

void Widget::onStatusSet(Status status)
{
    if (status == Status::Online)
        ui->statImg->setPixmap(QPixmap(":img/status/dot_online_2x.png"));
    else if (status == Status::Away)
        ui->statImg->setPixmap(QPixmap(":img/status/dot_idle_2x.png"));
    else if (status == Status::Busy)
        ui->statImg->setPixmap(QPixmap(":img/status/dot_busy_2x.png"));
    else if (status == Status::Offline)
        ui->statImg->setPixmap(QPixmap(":img/status/dot_away_2x.png"));
}

void Widget::onAddClicked()
{
    hideMainForms();
    friendForm.show(*ui);
}

void Widget::onGroupClicked()
{
    core->createGroup();
}

void Widget::onTransferClicked()
{

}

void Widget::onSettingsClicked()
{
    hideMainForms();
    settingsForm.show(*ui);
    isFriendWidgetActive = 0;
    isGroupWidgetActive = 0;
}

void Widget::hideMainForms()
{
    QLayoutItem* item;
    while ((item = ui->mainHead->layout()->takeAt(0)) != 0)
        item->widget()->hide();
    while ((item = ui->mainContent->layout()->takeAt(0)) != 0)
        item->widget()->hide();
    
    if (activeFriendWidget != nullptr)
    {
        Friend* f = FriendList::findFriend(activeFriendWidget->friendId);
        if (f != nullptr)
            activeFriendWidget->setAsInactiveChatroom();
    }
    if (activeGroupWidget != nullptr)
    {
        Group* g = GroupList::findGroup(activeGroupWidget->groupId);
        if (g != nullptr)
            activeGroupWidget->setAsInactiveChatroom();
    }
}

void Widget::onUsernameChanged(const QString& newUsername)
{
    ui->nameLabel->setText(newUsername);
    settingsForm.name.setText(newUsername);
    core->setUsername(newUsername);
}

void Widget::onUsernameChanged(const QString& newUsername, const QString& oldUsername)
{
    ui->nameLabel->setText(oldUsername); // restore old username until Core tells us to set it
    settingsForm.name.setText(oldUsername);
    core->setUsername(newUsername);
}

void Widget::setUsername(const QString& username)
{
    ui->nameLabel->setText(username);
    settingsForm.name.setText(username);
    Settings::getInstance().setUsername(username);
}

void Widget::onStatusMessageChanged(const QString& newStatusMessage)
{
    ui->statusLabel->setText(newStatusMessage);
    settingsForm.statusText.setText(newStatusMessage);
    core->setStatusMessage(newStatusMessage);
}

void Widget::onStatusMessageChanged(const QString& newStatusMessage, const QString& oldStatusMessage)
{
    ui->statusLabel->setText(oldStatusMessage); // restore old status message until Core tells us to set it
    settingsForm.statusText.setText(oldStatusMessage);
    core->setStatusMessage(newStatusMessage);
}

void Widget::setStatusMessage(const QString &statusMessage)
{
    ui->statusLabel->setText(statusMessage);
    settingsForm.statusText.setText(statusMessage);
    Settings::getInstance().setStatusMessage(statusMessage);
}

void Widget::addFriend(int friendId, const QString &userId)
{
    qDebug() << "Adding friend with id "+userId;
    Friend* newfriend = FriendList::addFriend(friendId, userId);
    QWidget* widget = ui->friendList->widget();
    QLayout* layout = widget->layout();
    layout->addWidget(newfriend->widget);
    connect(newfriend->widget, SIGNAL(friendWidgetClicked(FriendWidget*)), this, SLOT(onFriendWidgetClicked(FriendWidget*)));
    connect(newfriend->widget, SIGNAL(removeFriend(int)), this, SLOT(removeFriend(int)));
    connect(newfriend->widget, SIGNAL(copyFriendIdToClipboard(int)), this, SLOT(copyFriendIdToClipboard(int)));
    connect(newfriend->chatForm, SIGNAL(sendMessage(int,QString)), core, SLOT(sendMessage(int,QString)));
    connect(newfriend->chatForm, SIGNAL(sendFile(int32_t,QString,QByteArray)), core, SLOT(sendFile(int32_t,QString,QByteArray)));
    connect(newfriend->chatForm, SIGNAL(answerCall(int)), core, SLOT(answerCall(int)));
    connect(newfriend->chatForm, SIGNAL(hangupCall(int)), core, SLOT(hangupCall(int)));
    connect(newfriend->chatForm, SIGNAL(startCall(int)), core, SLOT(startCall(int)));
    connect(newfriend->chatForm, SIGNAL(startVideoCall(int,bool)), core, SLOT(startCall(int,bool)));
    connect(newfriend->chatForm, SIGNAL(cancelCall(int,int)), core, SLOT(cancelCall(int,int)));
    connect(core, &Core::fileReceiveRequested, newfriend->chatForm, &ChatForm::onFileRecvRequest);
    connect(core, &Core::avInvite, newfriend->chatForm, &ChatForm::onAvInvite);
    connect(core, &Core::avStart, newfriend->chatForm, &ChatForm::onAvStart);
    connect(core, &Core::avCancel, newfriend->chatForm, &ChatForm::onAvCancel);
    connect(core, &Core::avEnd, newfriend->chatForm, &ChatForm::onAvEnd);
    connect(core, &Core::avRinging, newfriend->chatForm, &ChatForm::onAvRinging);
    connect(core, &Core::avStarting, newfriend->chatForm, &ChatForm::onAvStarting);
    connect(core, &Core::avEnding, newfriend->chatForm, &ChatForm::onAvEnding);
    connect(core, &Core::avRequestTimeout, newfriend->chatForm, &ChatForm::onAvRequestTimeout);
    connect(core, &Core::avPeerTimeout, newfriend->chatForm, &ChatForm::onAvPeerTimeout);
}

void Widget::addFriendFailed(const QString&)
{
    QMessageBox::critical(0,"Error","Couldn't request friendship");
}

void Widget::onFriendStatusChanged(int friendId, Status status)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->friendStatus = status;
    updateFriendStatusLights(friendId);
}

void Widget::onFriendStatusMessageChanged(int friendId, const QString& message)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->setStatusMessage(message);
}

void Widget::onFriendUsernameChanged(int friendId, const QString& username)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->setName(username);
}

void Widget::onFriendStatusMessageLoaded(int friendId, const QString& message)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->setStatusMessage(message);
}

void Widget::onFriendUsernameLoaded(int friendId, const QString& username)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->setName(username);
}

void Widget::onFriendWidgetClicked(FriendWidget *widget)
{
    Friend* f = FriendList::findFriend(widget->friendId);
    if (!f)
        return;

    hideMainForms();
    f->chatForm->show(*ui);
    if (activeFriendWidget != nullptr)
    {
        activeFriendWidget->setAsInactiveChatroom();
    }
    activeFriendWidget = widget;
    widget->setAsActiveChatroom();
    isFriendWidgetActive = 1;
    isGroupWidgetActive = 0;

    if (f->hasNewMessages != 0)
        f->hasNewMessages = 0;

    updateFriendStatusLights(f->friendId);
}

void Widget::onFriendMessageReceived(int friendId, const QString& message)
{
    Friend* f = FriendList::findFriend(friendId);
    if (!f)
        return;

    f->chatForm->addFriendMessage(message);

    if (activeFriendWidget != nullptr)
    {
        Friend* f2 = FriendList::findFriend(activeFriendWidget->friendId);
        if (((f->friendId != f2->friendId) || isFriendWidgetActive == 0) || isWindowMinimized)
        {
            f->hasNewMessages = 1;
            newMessageAlert();
        }
    }
    else
    {
        f->hasNewMessages = 1;
        newMessageAlert();
    }

    updateFriendStatusLights(friendId);
}

void Widget::updateFriendStatusLights(int friendId)
{
    Friend* f = FriendList::findFriend(friendId);
    Status status = f->friendStatus;
    if (status == Status::Online && f->hasNewMessages == 0)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_online.png"));
    else if (status == Status::Online && f->hasNewMessages == 1)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_online_notification.png"));
    else if (status == Status::Away && f->hasNewMessages == 0)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_idle.png"));
    else if (status == Status::Away && f->hasNewMessages == 1)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_idle_notification.png"));
    else if (status == Status::Busy && f->hasNewMessages == 0)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_busy.png"));
    else if (status == Status::Busy && f->hasNewMessages == 1)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_busy_notification.png"));
    else if (status == Status::Offline && f->hasNewMessages == 0)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_away.png"));
    else if (status == Status::Offline && f->hasNewMessages == 1)
        f->widget->statusPic.setPixmap(QPixmap(":img/status/dot_away_notification.png"));
}

void Widget::newMessageAlert()
{
    QApplication::alert(this);
    QSound::play(":audio/notification.wav");
}

void Widget::onFriendRequestReceived(const QString& userId, const QString& message)
{
    FriendRequestDialog dialog(this, userId, message);

    if (dialog.exec() == QDialog::Accepted)
        emit friendRequestAccepted(userId);
}

void Widget::removeFriend(int friendId)
{
    Friend* f = FriendList::findFriend(friendId);
    if (f->widget == activeFriendWidget)
        activeFriendWidget = nullptr;
    FriendList::removeFriend(friendId);
    core->removeFriend(friendId);
    delete f;
    if (ui->mainHead->layout()->isEmpty())
        onAddClicked();
}

void Widget::copyFriendIdToClipboard(int friendId)
{
    Friend* f = FriendList::findFriend(friendId);
    if (f != nullptr)
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(f->userId, QClipboard::Clipboard);
        clipboard->deleteLater();
    }
}

void Widget::onGroupInviteReceived(int32_t friendId, const uint8_t* publicKey)
{
    int groupId = core->joinGroupchat(friendId, publicKey);
    if (groupId == -1)
    {
        qWarning() << "Widget::onGroupInviteReceived: Unable to accept invitation";
        return;
    }
}

void Widget::onGroupMessageReceived(int groupnumber, int friendgroupnumber, const QString& message)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
        return;

    g->chatForm->addGroupMessage(message, friendgroupnumber);

    if ((isGroupWidgetActive != 1 || (g->groupId != activeGroupWidget->groupId)) || isWindowMinimized)
    {
        if (message.contains(Settings::getInstance().getUsername(), Qt::CaseInsensitive))
        {
            newMessageAlert();
            g->hasNewMessages = 1;
            g->userWasMentioned = 1;
            g->widget->statusPic.setPixmap(QPixmap(":img/status/dot_groupchat_notification.png"));
        }
        else
            if (g->hasNewMessages == 0)
            {
                g->hasNewMessages = 1;
                g->widget->statusPic.setPixmap(QPixmap(":img/status/dot_groupchat_newmessages.png"));
            }
    }
}

void Widget::onGroupNamelistChanged(int groupnumber, int peernumber, uint8_t Change)
{
    Group* g = GroupList::findGroup(groupnumber);
    if (!g)
    {
        qDebug() << "Widget::onGroupNamelistChanged: Group not found, creating it";
        g = createGroup(groupnumber);
    }

    TOX_CHAT_CHANGE change = static_cast<TOX_CHAT_CHANGE>(Change);
    if (change == TOX_CHAT_CHANGE_PEER_ADD)
        g->addPeer(peernumber,"<Unknown>");
    else if (change == TOX_CHAT_CHANGE_PEER_DEL)
        g->removePeer(peernumber);
    else if (change == TOX_CHAT_CHANGE_PEER_NAME)
        g->updatePeer(peernumber,core->getGroupPeerName(groupnumber, peernumber));
}

void Widget::onGroupWidgetClicked(GroupWidget* widget)
{
    Group* g = GroupList::findGroup(widget->groupId);
    if (!g)
        return;

    hideMainForms();
    g->chatForm->show(*ui);
    if (activeGroupWidget != nullptr)
    {
        activeGroupWidget->setAsInactiveChatroom();
    }
    activeGroupWidget = widget;
    widget->setAsActiveChatroom();
    isFriendWidgetActive = 0;
    isGroupWidgetActive = 1;

    if (g->hasNewMessages != 0)
    {
        g->hasNewMessages = 0;
        g->userWasMentioned = 0;
        g->widget->statusPic.setPixmap(QPixmap(":img/status/dot_groupchat.png"));
    }
}

void Widget::removeGroup(int groupId)
{
    Group* g = GroupList::findGroup(groupId);
    if (g->widget == activeGroupWidget)
        activeGroupWidget = nullptr;
    GroupList::removeGroup(groupId);
    core->removeGroup(groupId);
    delete g;
    if (ui->mainHead->layout()->isEmpty())
        onAddClicked();
}

Core *Widget::getCore()
{
    return core;
}

Group *Widget::createGroup(int groupId)
{
    Group* g = GroupList::findGroup(groupId);
    if (g)
    {
        qWarning() << "Widget::createGroup: Group already exists";
        return g;
    }

    QString groupName = QString("Groupchat #%1").arg(groupId);
    Group* newgroup = GroupList::addGroup(groupId, groupName);
    QWidget* widget = ui->friendList->widget();
    QLayout* layout = widget->layout();
    layout->addWidget(newgroup->widget);
    connect(newgroup->widget, SIGNAL(groupWidgetClicked(GroupWidget*)), this, SLOT(onGroupWidgetClicked(GroupWidget*)));
    connect(newgroup->widget, SIGNAL(removeGroup(int)), this, SLOT(removeGroup(int)));
    connect(newgroup->chatForm, SIGNAL(sendMessage(int,QString)), core, SLOT(sendGroupMessage(int,QString)));
    return newgroup;
}

void Widget::showTestCamview()
{
    camview->show();
}

void Widget::onEmptyGroupCreated(int groupId)
{
    createGroup(groupId);
}

bool Widget::isFriendWidgetCurActiveWidget(Friend* f)
{
    if (!f)
        return false;
    if (activeFriendWidget != nullptr)
    {
        Friend* f2 = FriendList::findFriend(activeFriendWidget->friendId);
        if ((f->friendId != f2->friendId) || isFriendWidgetActive == 0)
            return false;
    }
    else
        return false;
    return true;
}



void Widget::mouseMoveEvent(QMouseEvent *e)
{
    if (!useNativeTheme)
    {
        int xMouse = e->pos().x();
        int yMouse = e->pos().y();
        int wWidth = this->geometry().width();
        int wHeight = this->geometry().height();

        if (moveWidget)
        {
            inResizeZone = false;
            moveWindow(e);
        }
        else if (allowToResize)
            resizeWindow(e);
        //right
        else if (xMouse >= wWidth - PIXELS_TO_ACT or allowToResize)
        {
            inResizeZone = true;

            if (yMouse >= wHeight - PIXELS_TO_ACT)
                setCursor(Qt::SizeFDiagCursor);
            else if (yMouse <= PIXELS_TO_ACT)
                setCursor(Qt::SizeBDiagCursor);
            else
                setCursor(Qt::SizeHorCursor);

            resizeWindow(e);
        }
        //left
        else if (xMouse <= PIXELS_TO_ACT or allowToResize)
        {
            inResizeZone = true;

            if (yMouse >= wHeight - PIXELS_TO_ACT)
                setCursor(Qt::SizeBDiagCursor);
            else if (yMouse <= PIXELS_TO_ACT)
                setCursor(Qt::SizeFDiagCursor);
            else
                setCursor(Qt::SizeHorCursor);

            resizeWindow(e);
        }
        //bottom edge
        else if ((yMouse >= wHeight - PIXELS_TO_ACT) or allowToResize)
        {
            inResizeZone = true;
            setCursor(Qt::SizeVerCursor);

            resizeWindow(e);
        }
        //Cursor part top
        else if (yMouse <= PIXELS_TO_ACT or allowToResize)
        {
            inResizeZone = true;
            setCursor(Qt::SizeVerCursor);

            resizeWindow(e);
        }
        else
        {
            inResizeZone = false;
            setCursor(Qt::ArrowCursor);
        }

        e->accept();
    }
}

bool Widget::event(QEvent * e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
        if(windowState().testFlag(Qt::WindowMinimized) == true)
        {
            isWindowMinimized = 1;
        }
    }
    else if (e->type() == QEvent::WindowActivate)
    {
        if (!useNativeTheme)
        {
            this->setObjectName("activeWindow");
            this->style()->polish(this);
        }
        isWindowMinimized = 0;
        if (isFriendWidgetActive && activeFriendWidget != nullptr)
        {
            Friend* f = FriendList::findFriend(activeFriendWidget->friendId);
            f->hasNewMessages = 0;
            updateFriendStatusLights(f->friendId);
        }
        else if (isGroupWidgetActive && activeGroupWidget != nullptr)
        {
            Group* g = GroupList::findGroup(activeGroupWidget->groupId);
            g->hasNewMessages = 0;
            g->userWasMentioned = 0;
            g->widget->statusPic.setPixmap(QPixmap("img/status/dot_groupchat.png"));
        }
    }
    else if (e->type() == QEvent::WindowDeactivate && !useNativeTheme)
    {
        this->setObjectName("inactiveWindow");
        this->style()->polish(this);
    }

    return QWidget::event(e);
}

void Widget::mousePressEvent(QMouseEvent *e)
{
    if (!useNativeTheme)
    {
        if (e->button() == Qt::LeftButton)
        {
            if (inResizeZone)
            {
                allowToResize = true;

                if (e->pos().y() <= PIXELS_TO_ACT)
                {
                    if (e->pos().x() <= PIXELS_TO_ACT)
                        resizeDiagSupEsq = true;
                    else if (e->pos().x() >= geometry().width() - PIXELS_TO_ACT)
                        resizeDiagSupDer = true;
                    else
                        resizeVerSup = true;
                }
                else if (e->pos().x() <= PIXELS_TO_ACT)
                    resizeHorEsq = true;
            }
            else if (e->pos().x() >= PIXELS_TO_ACT and e->pos().x() < ui->titleBar->geometry().width()
                     and e->pos().y() >= PIXELS_TO_ACT and e->pos().y() < ui->titleBar->geometry().height())
            {
                moveWidget = true;
                dragPosition = e->globalPos() - frameGeometry().topLeft();
            }
        }

        e->accept();
    }
}

void Widget::mouseReleaseEvent(QMouseEvent *e)
{
    if (!useNativeTheme)
    {
        moveWidget = false;
        allowToResize = false;
        resizeVerSup = false;
        resizeHorEsq = false;
        resizeDiagSupEsq = false;
        resizeDiagSupDer = false;

        e->accept();
    }
}

void Widget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (!useNativeTheme)
    {
        if (e->pos().x() < ui->tbMenu->geometry().right() and e->pos().y() < ui->tbMenu->geometry().bottom()
                and e->pos().x() >=  ui->tbMenu->geometry().x() and e->pos().y() >= ui->tbMenu->geometry().y()
                and ui->tbMenu->isVisible())
            close();
        else if (e->pos().x() < ui->titleBar->geometry().width()
                 and e->pos().y() < ui->titleBar->geometry().height()
                 and m_titleMode != FullScreenMode)
            maximizeBtnClicked();
        e->accept();
    }
}

void Widget::paintEvent (QPaintEvent *)
{
    QStyleOption opt;
    opt.init (this);
    QPainter p(this);
    style()->drawPrimitive (QStyle::PE_Widget, &opt, &p, this);
}

void Widget::moveWindow(QMouseEvent *e)
{
    if (!useNativeTheme)
    {
        if (e->buttons() & Qt::LeftButton)
        {
            move(e->globalPos() - dragPosition);
            e->accept();
        }
    }
}

void Widget::resizeWindow(QMouseEvent *e)
{
    if (!useNativeTheme)
    {
        if (allowToResize)
        {
            int xMouse = e->pos().x();
            int yMouse = e->pos().y();
            int wWidth = geometry().width();
            int wHeight = geometry().height();

            if (cursor().shape() == Qt::SizeVerCursor)
            {
                if (resizeVerSup)
                {
                    int newY = geometry().y() + yMouse;
                    int newHeight = wHeight - yMouse;

                    if (newHeight > minimumSizeHint().height())
                    {
                        resize(wWidth, newHeight);
                        move(geometry().x(), newY);
                    }
                }
                else
                    resize(wWidth, yMouse+1);
            }
            else if (cursor().shape() == Qt::SizeHorCursor)
            {
                if (resizeHorEsq)
                {
                    int newX = geometry().x() + xMouse;
                    int newWidth = wWidth - xMouse;

                    if (newWidth > minimumSizeHint().width())
                    {
                        resize(newWidth, wHeight);
                        move(newX, geometry().y());
                    }
                }
                else
                    resize(xMouse, wHeight);
            }
            else if (cursor().shape() == Qt::SizeBDiagCursor)
            {
                int newX = 0;
                int newWidth = 0;
                int newY = 0;
                int newHeight = 0;

                if (resizeDiagSupDer)
                {
                    newX = geometry().x();
                    newWidth = xMouse;
                    newY = geometry().y() + yMouse;
                    newHeight = wHeight - yMouse;
                }
                else
                {
                    newX = geometry().x() + xMouse;
                    newWidth = wWidth - xMouse;
                    newY = geometry().y();
                    newHeight = yMouse;
                }

                if (newWidth >= minimumSizeHint().width() and newHeight >= minimumSizeHint().height())
                {
                    resize(newWidth, newHeight);
                    move(newX, newY);
                }
                else if (newWidth >= minimumSizeHint().width())
                {
                    resize(newWidth, wHeight);
                    move(newX, geometry().y());
                }
                else if (newHeight >= minimumSizeHint().height())
                {
                    resize(wWidth, newHeight);
                    move(geometry().x(), newY);
                }
            }
            else if (cursor().shape() == Qt::SizeFDiagCursor)
            {
                if (resizeDiagSupEsq)
                {
                    int newX = geometry().x() + xMouse;
                    int newWidth = wWidth - xMouse;
                    int newY = geometry().y() + yMouse;
                    int newHeight = wHeight - yMouse;

                    if (newWidth >= minimumSizeHint().width() and newHeight >= minimumSizeHint().height())
                    {
                        resize(newWidth, newHeight);
                        move(newX, newY);
                    }
                    else if (newWidth >= minimumSizeHint().width())
                    {
                        resize(newWidth, wHeight);
                        move(newX, geometry().y());
                    }
                    else if (newHeight >= minimumSizeHint().height())
                    {
                        resize(wWidth, newHeight);
                        move(geometry().x(), newY);
                    }
                }
                else
                    resize(xMouse+1, yMouse+1);
            }

            e->accept();
        }
    }
}

void Widget::setCentralWidget(QWidget *widget, const QString &widgetName)
{
    connect(widget, SIGNAL(cancelled()), this, SLOT(close()));

    centralLayout->addWidget(widget);
    ui->centralWidget->setLayout(centralLayout);
    ui->LTitle->setText(widgetName);
}

void Widget::setTitlebarMode(const TitleMode &flag)
{
    m_titleMode = flag;

    switch (m_titleMode)
    {
        case CleanTitle:
            ui->tbMenu->setHidden(true);
            ui->pbMin->setHidden(true);
            ui->pbMax->setHidden(true);
            ui->pbClose->setHidden(true);
            break;
        case OnlyCloseButton:
            ui->tbMenu->setHidden(true);
            ui->pbMin->setHidden(true);
            ui->pbMax->setHidden(true);
            break;
        case MenuOff:
            ui->tbMenu->setHidden(true);
            break;
        case MaxMinOff:
            ui->pbMin->setHidden(true);
            ui->pbMax->setHidden(true);
            break;
        case FullScreenMode:
            ui->pbMax->setHidden(true);
            showMaximized();
            break;
        case MaximizeModeOff:
            ui->pbMax->setHidden(true);
            break;
        case MinimizeModeOff:
            ui->pbMin->setHidden(true);
            break;
        case FullTitle:
            ui->tbMenu->setVisible(true);
            ui->pbMin->setVisible(true);
            ui->pbMax->setVisible(true);
            ui->pbClose->setVisible(true);
            break;
            break;
        default:
            ui->tbMenu->setVisible(true);
            ui->pbMin->setVisible(true);
            ui->pbMax->setVisible(true);
            ui->pbClose->setVisible(true);
            break;
    }
    ui->LTitle->setVisible(true);
}

void Widget::setTitlebarMenu(QMenu *menu, const QString &icon)
{
    ui->tbMenu->setMenu(menu);
    ui->tbMenu->setIcon(QIcon(icon));
}

void Widget::maximizeBtnClicked()
{
    if (isFullScreen() or isMaximized())
    {
        ui->pbMax->setIcon(QIcon(":/ui/images/app_max.png"));
        setWindowState(windowState() & ~Qt::WindowFullScreen & ~Qt::WindowMaximized);
    }
    else
    {
        ui->pbMax->setIcon(QIcon(":/ui/images/app_rest.png"));
        setWindowState(windowState() | Qt::WindowFullScreen | Qt::WindowMaximized);
    }
}

void Widget::minimizeBtnClicked()
{
    if (isMinimized())
    {
        setWindowState(windowState() & ~Qt::WindowMinimized);
    }
    else
    {
        setWindowState(windowState() | Qt::WindowMinimized);
    }
}

void Widget::onStatusImgClicked()
{
    QMenu menu;
    menu.addAction(tr("Online","Button to set your status to 'Online'"));
    menu.addAction(tr("Away","Button to set your status to 'Away'"));
    menu.addAction(tr("Busy","Button to set your status to 'Busy'"));

    QPoint pos = QCursor::pos();
    QAction* selectedItem = menu.exec(pos);
    if (selectedItem)
    {
        if (selectedItem->text() == "Online")
            core->setStatus(Status::Online);
        else if (selectedItem->text() == "Away")
            core->setStatus(Status::Away);
        else if (selectedItem->text() == "Busy")
            core->setStatus(Status::Busy);
    }
}
