#include "avatarbroadcaster.h"
#include "src/core/core.h"
#include <QObject>
#include <QDebug>

QByteArray AvatarBroadcaster::avatarData;
QMap<uint32_t, bool> AvatarBroadcaster::friendsSentTo;

static QMetaObject::Connection autoBroadcastConn;
static auto autoBroadcast = [](uint32_t friendId, Status)
{
    AvatarBroadcaster::sendAvatarTo(friendId);
};

void AvatarBroadcaster::setAvatar(QByteArray data)
{
    avatarData = data;
    friendsSentTo.clear();

    QVector<uint32_t> friends = Core::getInstance()->getFriendList();
    for (uint32_t friendId : friends)
        sendAvatarTo(friendId);
}

void AvatarBroadcaster::sendAvatarTo(uint32_t friendId)
{
    if (friendsSentTo.contains(friendId) && friendsSentTo[friendId])
        return;
    if (!Core::getInstance()->isFriendOnline(friendId))
        return;
    qDebug() << "AvatarBroadcaster: Sending avatar to "<<friendId;
    Core::getInstance()->sendAvatarFile(friendId, avatarData);
    friendsSentTo[friendId] = true;
}

void AvatarBroadcaster::enableAutoBroadcast(bool state)
{
    QObject::disconnect(autoBroadcastConn);
    if (state)
        autoBroadcastConn = QObject::connect(Core::getInstance(), &Core::friendStatusChanged, autoBroadcast);
}
