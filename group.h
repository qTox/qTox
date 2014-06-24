#ifndef GROUP_H
#define GROUP_H

#include <QList>
#include <QMap>
#include <QTimer>

#define RETRY_PEER_INFO_INTERVAL 500

class Friend;
class GroupWidget;
class GroupChatForm;

class Group : public QObject
{
    Q_OBJECT
public:
    Group(int GroupId, QString Name);
    ~Group();
    void addPeer(int peerId, QString name);
    void removePeer(int peerId);
    void updatePeer(int peerId, QString newName);

private slots:
    void queryPeerInfo();

public:
    int groupId;
    QMap<int,QString> peers;
    int nPeers;
    GroupWidget* widget;
    GroupChatForm* chatForm;
    bool hasPeerInfo;
    QTimer peerInfoTimer;
};

#endif // GROUP_H
