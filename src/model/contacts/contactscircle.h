#ifndef CONTACTSCIRCLE_H
#define CONTACTSCIRCLE_H

#include <QObject>

#include <vector>

#include <functional>

class Friend;
class ToxPk;

class QString;

class ContactsCircle : public QObject
{
    Q_OBJECT
public:
    using Comparator = std::function<bool(const Friend*, const Friend*)>;
    using FriendSupplier = std::function<Friend*(const ToxPk&)>;

public:
    ContactsCircle(const int id, const QString& name, bool expanded = false);

    int getId() const;
    void setId(const int id);

    bool isExpanded() const;
    void setExpanded(bool expanded);

    QString getCircleName() const;
    void setCircleName(const QString& newName);

    std::vector<const Friend*> getFriends() const;

    void addFriend(const ToxPk& friendPk);
    void removeFriend(const ToxPk& friendPk);
    bool hasFriend(const ToxPk& friendPk);

    static Comparator getComparator();
    static void setComparator(Comparator comp);

    static FriendSupplier getFriendSupplier();
    static void setFriendSupplier(FriendSupplier fs);

signals:
    void friendAdded(const ToxPk& friendPk);
    void friendRemoved(const ToxPk& friendPk);
    void expansionChanged(bool expanded);
    void circleRenamed(const QString& newName);

private:
    int id;
    bool expanded;
    QString name;
    std::vector<const Friend*> friends;

    static Comparator comparator;
    static FriendSupplier friendSupplier;
};

#endif // CONTACTSCIRCLE_H
