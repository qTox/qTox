#include "contactscircle.h"

#include "src/friendlist.h"
#include "src/model/contacts/friend.h"

#include <QRegularExpression>

/**
  * @brief Functor for sorting Friends by name
  */
ContactsCircle::Comparator ContactsCircle::comparator = [] (const Friend* f1, const Friend* f2)
{
    return QString::compare(f1->getDisplayedName(),f2->getDisplayedName(),Qt::CaseInsensitive) <= 0;
};

ContactsCircle::Comparator ContactsCircle::getComparator()
{
    return comparator;
}

void ContactsCircle::setComparator(Comparator comp)
{
    comparator = comp;
}

/**
 * @brief Wrapper around dependency function supposed to find friends by ID
 * @param friendId ID of a friend to find
 * @return Friend if friend with such ID is found or nullptr otherwise
 */
ContactsCircle::FriendSupplier ContactsCircle::friendSupplier = [] (const ToxPk& pk)
{
    return FriendList::findFriend(pk);
};

ContactsCircle::FriendSupplier ContactsCircle::getFriendSupplier()
{
    return friendSupplier;
}

void ContactsCircle::setFriendSupplier(FriendSupplier fs)
{
    friendSupplier = fs;
}

/**
 * @class This class is a model describing several friends united by some user-defined reason
 * Do not confuse this with a group - circle is just a set of contacts to be displayed within
 * same CircleWidget
 */

ContactsCircle::ContactsCircle(const int id, const QString& name, bool expanded)
    : id{id}
    , name{name}
    , expanded{expanded}
{
}

int ContactsCircle::getId() const
{
    return id;
}

void ContactsCircle::setId(const int id)
{
    this->id = id;
}

/**
 * @brief Shows whether circle widget is expanded or not
 * TODO: this property belongs to CircleWidget and should not be here, in model
 */
bool ContactsCircle::isExpanded() const
{
    return expanded;
}

void ContactsCircle::setExpanded(bool expanded)
{
    if (expanded != this->expanded) {
        this->expanded = expanded;
        emit expansionChanged(expanded);
    }
}

QString ContactsCircle::getCircleName() const
{
    return name;
}

static bool isBlankString(const QString& str)
{
    static const QRegularExpression exp{"^\\s+$"};
    return exp.match(str).hasMatch();
}

void ContactsCircle::setCircleName(const QString &newName)
{
    if (newName.isEmpty() || isBlankString(newName) || newName == name) {
        return;
    }

    name = newName;
    emit circleRenamed(newName);
}

std::vector<const Friend*> ContactsCircle::getFriends() const
{
    return friends;
}


/**
 * @brief Adds friend to the circle with sorting by its name
 */
void ContactsCircle::addFriend(const ToxPk& friendPk)
{
    if (hasFriend(friendPk)) {
        return;
    }

    const Friend* newFriend = friendSupplier(friendPk);
    if (newFriend == nullptr) {
        return;
    }

    const auto cbegin = friends.cbegin();
    const auto cend = friends.cend();
    const auto sortedPosition = std::upper_bound(cbegin, cend, newFriend, comparator);
    friends.insert(sortedPosition, newFriend);
    emit friendAdded(friendPk);
}

using IteratorType = std::vector<const Friend*>::const_iterator;

/**
 * @brief Returns iterator for Friend* object with specified ID from vector of Friend's
 */
static IteratorType getFriendByPk(const std::vector<const Friend*>& friends,
                                  const ToxPk& friendPk)
{
    return std::find_if(friends.cbegin(), friends.cend(), [friendPk] (const Friend* f)
    {
        return f->getPublicKey() == friendPk;
    });
}

/**
 * @brief Removes friend from the circle
 */
void ContactsCircle::removeFriend(const ToxPk& friendPk)
{
    const auto removingFriendIt = getFriendByPk(friends, friendPk);
    if (removingFriendIt == friends.cend()) {
        return;
    }

    friends.erase(removingFriendIt);
    emit friendRemoved(friendPk);
}

/**
 * @brief Shows whether friend with specified ID belongs to the circle
 */
bool ContactsCircle::hasFriend(const ToxPk& friendPk)
{
    return getFriendByPk(friends, friendPk) != friends.cend();
}
