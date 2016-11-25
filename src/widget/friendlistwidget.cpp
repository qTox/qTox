/*
    Copyright © 2014-2015 by The qTox Project

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

#include "friendlistwidget.h"

#include <cassert>

#include <QGridLayout>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QTimer>

#include "src/persistence/settings.h"
#include "src/widget/circlewidget.h"
#include "src/widget/friendlistlayout.h"
#include "src/widget/friendwidget.h"
#include "src/widget/groupwidget.h"
#include "src/widget/widget.h"

enum Time : int
{
    Today = 0,
    Yesterday = 1,
    ThisWeek = 2,
    ThisMonth = 3,
    Month1Ago = 4,
    Month2Ago = 5,
    Month3Ago = 6,
    Month4Ago = 7,
    Month5Ago = 8,
    LongAgo = 9,
    Never = 10
};

bool last7DaysWasLastMonth()
{
    return QDate::currentDate().addDays(-7).month() == QDate::currentDate().month();
}

Time getTime(const QDate& date)
{
    if (date == QDate())
        return Never;

    QDate today = QDate::currentDate();

    if (date == today)
        return Today;

    today = today.addDays(-1);

    if (date == today)
        return Yesterday;

    today = today.addDays(-6);

    if (date >= today)
        return ThisWeek;

    today = today.addDays(-today.day() + 1); // Go to the beginning of the month.

    if (last7DaysWasLastMonth())
    {
        if (date >= today)
            return ThisMonth;

        today = today.addMonths(-1);
    }

    if (date >= today)
        return Month1Ago;

    today = today.addMonths(-1);

    if (date >= today)
        return Month2Ago;

    today = today.addMonths(-1);

    if (date >= today)
        return Month3Ago;

    today = today.addMonths(-1);

    if (date >= today)
        return Month4Ago;

    today = today.addMonths(-1);

    if (date >= today)
        return Month5Ago;

    return LongAgo;
}

QDate getDateFriend(Friend contact)
{
    ToxId toxId = contact.getToxId();
    return Settings::getInstance().getFriendActivity(toxId);
}

qint64 timeUntilTomorrow()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime tomorrow = now.addDays(1); // Tomorrow.
    tomorrow.setTime(QTime()); // Midnight.
    return now.msecsTo(tomorrow);
}

FriendListWidget::FriendListWidget(Widget* parent, bool groupsOnTop)
    : QWidget(parent)
    // Prevent Valgrind from complaining. We're changing this to Name here.
    // Must be Activity for change to take effect.
    , mode(Activity)
    , groupsOnTop(groupsOnTop)
    , dayTimer(new QTimer(this))
{
    listLayout = new FriendListLayout();
    setLayout(listLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    groupLayout.getLayout()->setSpacing(0);
    groupLayout.getLayout()->setMargin(0);

    // Prevent QLayout's add child warning before setting the mode.
    listLayout->removeItem(listLayout->getLayoutOnline());
    listLayout->removeItem(listLayout->getLayoutOffline());

    setMode(Name);

    onGroupchatPositionChanged(groupsOnTop);

    dayTimer->setTimerType(Qt::VeryCoarseTimer);
    connect(dayTimer, &QTimer::timeout, this, &FriendListWidget::dayTimeout);
    dayTimer->start(static_cast<int>(timeUntilTomorrow()));

    const FriendNotify* notify = Friend::notify();
    connect(notify, &FriendNotify::removed,
            this, &FriendListWidget::onFriendRemoved);
    connect(notify, &FriendNotify::nameChanged,
            this, &FriendListWidget::onFriendAliasChanged);
    connect(notify, &FriendNotify::aliasChanged,
            this, &FriendListWidget::onFriendAliasChanged);
    connect(notify, &FriendNotify::statusChanged,
            this, &FriendListWidget::onFriendStatusChanged);
    connect(notify, &FriendNotify::avatarChanged,
            this, &FriendListWidget::onFriendAvatarChanged);

    setAcceptDrops(true);
}

FriendListWidget::~FriendListWidget()
{
    if (activityLayout != nullptr)
    {
        QLayoutItem* item;
        while ((item = activityLayout->takeAt(0)) != nullptr)
        {
            delete item->widget();
            delete item;
        }
        delete activityLayout;
    }

    if (circleLayout != nullptr)
    {
        QLayoutItem* item;
        while ((item = circleLayout->getLayout()->takeAt(0)) != nullptr)
        {
            delete item->widget();
            delete item;
        }
        delete circleLayout;
    }
}

void FriendListWidget::setMode(Mode mode)
{
    if (this->mode == mode)
        return;

    this->mode = mode;

    if (mode == Name)
    {
        circleLayout = new GenericChatItemLayout;
        circleLayout->getLayout()->setSpacing(0);
        circleLayout->getLayout()->setMargin(0);

        for (int i = 0; i < Settings::getInstance().getCircleCount(); ++i)
        {
            addCircleWidget(i);
            CircleWidget::getFromID(i)->setVisible(false);
        }

        // Only display circles once all created to avoid artifacts.
        for (int i = 0; i < Settings::getInstance().getCircleCount(); ++i)
            CircleWidget::getFromID(i)->setVisible(true);

        int count = activityLayout ? activityLayout->count() : 0;
        for (int i = 0; i < count; i++)
        {
            QWidget *widget = activityLayout->itemAt(i)->widget();
            CategoryWidget *categoryWidget = qobject_cast<CategoryWidget*>(widget);
            if (categoryWidget)
                categoryWidget->moveFriendWidgets(this);
            else
                qWarning() << "Unexpected widget";
        }

        listLayout->addLayout(listLayout->getLayoutOnline());
        listLayout->addLayout(listLayout->getLayoutOffline());
        listLayout->addLayout(circleLayout->getLayout());
        onGroupchatPositionChanged(groupsOnTop);

        if (activityLayout != nullptr)
        {
            QLayoutItem* item;
            while ((item = activityLayout->takeAt(0)) != nullptr)
            {
                delete item->widget();
                delete item;
            }
            delete activityLayout;
            activityLayout = nullptr;
        }

        reloadTheme();
    }
    else if (mode == Activity)
    {
        activityLayout = new QVBoxLayout();

        CategoryWidget* categoryToday = new CategoryWidget(this);
        categoryToday->setObjectName("Todddd");
        categoryToday->setName(tr("Today", "Category for sorting friends by activity"));
        activityLayout->addWidget(categoryToday);

        CategoryWidget* categoryYesterday = new CategoryWidget(this);
        categoryYesterday->setName(tr("Yesterday", "Category for sorting friends by activity"));
        activityLayout->addWidget(categoryYesterday);

        CategoryWidget* categoryLastWeek = new CategoryWidget(this);
        categoryLastWeek->setName(tr("Last 7 days", "Category for sorting friends by activity"));
        activityLayout->addWidget(categoryLastWeek);

        QDate currentDate = QDate::currentDate();
        //if (last7DaysWasLastMonth())
        {
            CategoryWidget* categoryThisMonth = new CategoryWidget(this);
            categoryThisMonth->setName(tr("This month", "Category for sorting friends by activity"));
            activityLayout->addWidget(categoryThisMonth);
            categoryThisMonth->setVisible(last7DaysWasLastMonth());

            if (categoryThisMonth->isVisible())
                currentDate = currentDate.addMonths(-1);
        }

        QLocale *ql = new QLocale(Settings::getInstance().getTranslation());

        CategoryWidget* categoryLast1Month = new CategoryWidget(this);
        categoryLast1Month->setName(ql->monthName(currentDate.month()));
        activityLayout->addWidget(categoryLast1Month);

        currentDate = currentDate.addMonths(-1);
        CategoryWidget* categoryLast2Month = new CategoryWidget(this);
        categoryLast2Month->setName(ql->monthName(currentDate.month()));
        activityLayout->addWidget(categoryLast2Month);

        currentDate = currentDate.addMonths(-1);
        CategoryWidget* categoryLast3Month = new CategoryWidget(this);
        categoryLast3Month->setName(ql->monthName(currentDate.month()));
        activityLayout->addWidget(categoryLast3Month);

        currentDate = currentDate.addMonths(-1);
        CategoryWidget* categoryLast4Month = new CategoryWidget(this);
        categoryLast4Month->setName(ql->monthName(currentDate.month()));
        activityLayout->addWidget(categoryLast4Month);

        currentDate = currentDate.addMonths(-1);
        CategoryWidget* categoryLast5Month = new CategoryWidget(this);
        categoryLast5Month->setName(ql->monthName(currentDate.month()));
        activityLayout->addWidget(categoryLast5Month);

        CategoryWidget* categoryOlder = new CategoryWidget(this);
        categoryOlder->setName(tr("Older than 6 Months", "Category for sorting friends by activity"));
        activityLayout->addWidget(categoryOlder);

        CategoryWidget* categoryNever = new CategoryWidget(this);
        categoryNever->setName(tr("Unknown", "Category for sorting friends by activity"));
        activityLayout->addWidget(categoryNever);

        delete ql;
        ql = nullptr;

        moveFriends(listLayout->getLayoutOffline());
        moveFriends(listLayout->getLayoutOnline());
        moveFriends(circleLayout->getLayout());

        for (int i = 0; i < activityLayout->count(); ++i)
        {
            CategoryWidget* categoryWidget = qobject_cast<CategoryWidget*>(activityLayout->itemAt(i)->widget());
            categoryWidget->setVisible(categoryWidget->hasChatrooms());
        }

        listLayout->removeItem(listLayout->getLayoutOnline());
        listLayout->removeItem(listLayout->getLayoutOffline());

        if (circleLayout != nullptr)
        {
            listLayout->removeItem(circleLayout->getLayout());

            QLayoutItem* item;
            while ((item = circleLayout->getLayout()->takeAt(0)) != nullptr)
            {
                delete item->widget();
                delete item;
            }
            delete circleLayout;
            circleLayout = nullptr;
        }

        reloadTheme();
    }
}

void FriendListWidget::moveFriends(QLayout *layout)
{
    for (int i = 0; i < layout->count(); i++)
    {
        QWidget *widget = layout->itemAt(i)->widget();
        FriendWidget *friendWidget = qobject_cast<FriendWidget*>(widget);
        CircleWidget *circleWidget = qobject_cast<CircleWidget*>(widget);
        if (circleWidget)
        {
            circleWidget->moveFriendWidgets(this);
        }
        else if (friendWidget)
        {
            Friend contact = friendWidget->getFriend();
            QDate activityDate = getDateFriend(contact);
            Time time = getTime(activityDate);

            QWidget *w = activityLayout->itemAt(time)->widget();
            CategoryWidget* categoryWidget = qobject_cast<CategoryWidget*>(w);
            categoryWidget->addFriendWidget(friendWidget, contact.getStatus());
        }
    }
}

FriendListWidget::Mode FriendListWidget::getMode() const
{
    return mode;
}

void FriendListWidget::clear()
{
    qDeleteAll(friendWidgets);
    friendWidgets.clear();

    qDeleteAll(groupWidgets);
    groupWidgets.clear();
}

GroupWidget* FriendListWidget::getGroupWidget(int groupId) const
{
    return groupWidgets[groupId];
}

GroupWidget* FriendListWidget::requestGroupWidget(int groupId)
{
    if (groupWidgets.contains(groupId))
        return groupWidgets[groupId];

    Group* g = Group::get(groupId);
    if (!g)
        return nullptr;

    GroupWidget* w = new GroupWidget(g, this);
    groupWidgets[groupId] = w;

    groupLayout.addSortedWidget(w);

    return w;
}

FriendWidget* FriendListWidget::getFriendWidget(uint32_t friendId) const
{
    return friendWidgets[friendId];
}

FriendWidget* FriendListWidget::requestFriendWidget(const Friend& f,
                                                    int circleIndex)
{
    if (!f)
        return nullptr;

    Friend::ID friendId = f.getFriendId();
    if (friendWidgets.contains(friendId))
        return friendWidgets[friendId];

    FriendWidget* w = new FriendWidget(f, this);
    friendWidgets[friendId] = w;

    CircleWidget* circleWidget = CircleWidget::getFromID(circleIndex);
    if (circleWidget == nullptr)
        moveWidget(w, f.getStatus(), true);
    else
        circleWidget->addFriendWidget(w, f.getStatus());

    return w;
}

void FriendListWidget::removeFriendWidget(uint32_t friendId)
{
    FriendWidget *widget = friendWidgets.take(friendId);

    if (widget)
    {
        Friend f = widget->getFriend();

        if (mode == Activity)
        {
            QDate activityDate = getDateFriend(f);
            Time time = getTime(activityDate);
            CategoryWidget* categoryWidget =
                    qobject_cast<CategoryWidget*>(activityLayout->itemAt(time)->widget());
            categoryWidget->removeFriendWidget(widget, f.getStatus());
            categoryWidget->setVisible(categoryWidget->hasChatrooms());
        }
        else
        {
            int id = Settings::getInstance().getFriendCircleID(f.getToxId());
            CircleWidget* circleWidget = CircleWidget::getFromID(id);
            if (circleWidget != nullptr)
            {
                circleWidget->removeFriendWidget(widget, f.getStatus());
                Widget::getInstance()->searchCircle(circleWidget);
            }
        }
    }

    delete widget;
}

void FriendListWidget::removeGroupWidget(int groupId)
{
    delete groupWidgets.take(groupId);
}

void FriendListWidget::addCircleWidget(int id)
{
    createCircleWidget(id);
}

void FriendListWidget::addCircleWidget(FriendWidget* friendWidget)
{
    CircleWidget* circleWidget = createCircleWidget();

    if (circleWidget != nullptr)
    {
        if (friendWidget != nullptr)
        {
            Friend f = friendWidget->getFriend();
            ToxId toxId = f.getToxId();
            int circleId = Settings::getInstance().getFriendCircleID(toxId);
            CircleWidget* circleOriginal = CircleWidget::getFromID(circleId);

            circleWidget->addFriendWidget(friendWidget, f.getStatus());
            circleWidget->setExpanded(true);

            if (circleOriginal != nullptr)
                Widget::getInstance()->searchCircle(circleOriginal);

        }

        Widget::getInstance()->searchCircle(circleWidget);

        if (window()->isActiveWindow())
            circleWidget->editName();
    }

    reloadTheme();
}

void FriendListWidget::removeCircleWidget(CircleWidget* widget)
{
    circleLayout->removeSortedWidget(widget);
    widget->deleteLater();
}

void FriendListWidget::searchChatrooms(const QString &searchString, bool hideOnline, bool hideOffline, bool hideGroups)
{
    groupLayout.search(searchString, hideGroups);
    listLayout->searchChatrooms(searchString, hideOnline, hideOffline);

    if (circleLayout != nullptr)
    {
        for (int i = 0; i != circleLayout->getLayout()->count(); ++i)
        {
            CircleWidget* circleWidget = static_cast<CircleWidget*>(circleLayout->getLayout()->itemAt(i)->widget());
            circleWidget->search(searchString, true, hideOnline, hideOffline);
        }
    }
    else if (activityLayout != nullptr)
    {
        for (int i = 0; i != activityLayout->count(); ++i)
        {
            CategoryWidget* categoryWidget = static_cast<CategoryWidget*>(activityLayout->itemAt(i)->widget());
            categoryWidget->search(searchString, true, hideOnline, hideOffline);
            categoryWidget->setVisible(categoryWidget->hasChatrooms());
        }
    }
}

void FriendListWidget::renameCircleWidget(CircleWidget* circleWidget, const QString &newName)
{
    circleLayout->removeSortedWidget(circleWidget);
    circleWidget->setName(newName);
    circleLayout->addSortedWidget(circleWidget);
}

void FriendListWidget::onGroupchatPositionChanged(bool top)
{
    groupsOnTop = top;

    if (mode != Name)
        return;

    listLayout->removeItem(groupLayout.getLayout());

    if (top)
        listLayout->insertLayout(0, groupLayout.getLayout());
    else
        listLayout->insertLayout(1, groupLayout.getLayout());

    reloadTheme();
}

void FriendListWidget::cycleContacts(GenericChatroomWidget* activeChatroomWidget, bool forward)
{
    if (activeChatroomWidget == nullptr)
        return;

    int index = -1;
    FriendWidget* friendWidget = qobject_cast<FriendWidget*>(activeChatroomWidget);

    if (mode == Activity)
    {
        if (friendWidget == nullptr)
            return;

        QDate activityDate = getDateFriend(friendWidget->getFriend());
        index = getTime(activityDate);
        CategoryWidget* categoryWidget = qobject_cast<CategoryWidget*>(activityLayout->itemAt(index)->widget());

        if (categoryWidget == nullptr || categoryWidget->cycleContacts(friendWidget, forward))
            return;

        index += forward ? 1 : -1;

        for (;;)
        {
            // Bounds checking.
            if (index < Today)
            {
                index = Never;
                continue;
            }
            else if (index > Never)
            {
                index = Today;
                continue;
            }

            CategoryWidget* categoryWidget = qobject_cast<CategoryWidget*>(activityLayout->itemAt(index)->widget());

            if (categoryWidget != nullptr)
            {
                if (!categoryWidget->cycleContacts(forward))
                {
                    // Skip empty or finished categories.
                    index += forward ? 1 : -1;
                    continue;
                }
            }

            break;
        }

        return;
    }

    QLayout* currentLayout = nullptr;
    CircleWidget* circleWidget = nullptr;

    if (friendWidget != nullptr)
    {
        ToxId toxId = friendWidget->getFriend().getToxId();
        int circleId = Settings::getInstance().getFriendCircleID(toxId);
        circleWidget = CircleWidget::getFromID(circleId);
        if (circleWidget != nullptr)
        {
            if (circleWidget->cycleContacts(friendWidget, forward))
                return;

            index = circleLayout->indexOfSortedWidget(circleWidget);
            currentLayout = circleLayout->getLayout();
        }
        else
        {
            currentLayout = listLayout->getLayoutOnline();
            index = listLayout->indexOfFriendWidget(friendWidget, true);
            if (index == -1)
            {
                currentLayout = listLayout->getLayoutOffline();
                index = listLayout->indexOfFriendWidget(friendWidget, false);
            }
        }
    }
    else
    {
        GroupWidget* groupWidget = qobject_cast<GroupWidget*>(activeChatroomWidget);
        if (!groupWidget)
            return;

        currentLayout = groupLayout.getLayout();
        index = groupLayout.indexOfSortedWidget(groupWidget);
    }

    index += forward ? 1 : -1;

    for (;;)
    {
        // Bounds checking.
        if (index < 0)
        {
            currentLayout = nextLayout(currentLayout, forward);
            index = currentLayout->count() - 1;
            continue;
        }
        else if (index >= currentLayout->count())
        {
            currentLayout = nextLayout(currentLayout, forward);
            index = 0;
            continue;
        }

        // Go to the actual next index.
        if (currentLayout == listLayout->getLayoutOnline() || currentLayout == listLayout->getLayoutOffline() || currentLayout == groupLayout.getLayout())
        {
            GenericChatroomWidget* chatWidget = qobject_cast<GenericChatroomWidget*>(currentLayout->itemAt(index)->widget());

            if (chatWidget != nullptr)
                emit chatWidget->chatroomWidgetClicked(chatWidget);

            return;
        }
        else if (currentLayout == circleLayout->getLayout())
        {
            circleWidget = qobject_cast<CircleWidget*>(currentLayout->itemAt(index)->widget());
            if (circleWidget != nullptr)
            {
                if (!circleWidget->cycleContacts(forward))
                {
                    // Skip empty or finished circles.
                    index += forward ? 1 : -1;
                    continue;
                }
            }
            return;
        }
        else
        {
            return;
        }
    }
}

void FriendListWidget::dragEnterEvent(QDragEnterEvent* event)
{
    ToxId toxId(event->mimeData()->text());
    Friend frnd = Friend::get(toxId);
    if (frnd)
        event->acceptProposedAction();
}

void FriendListWidget::dropEvent(QDropEvent* event)
{
    // Check, that the element is dropped from qTox
    QObject *o = event->source();
    FriendWidget *widget = qobject_cast<FriendWidget*>(o);
    if (!widget)
        return;

    // Check, that the user has a friend with the same ToxId
    ToxId toxId(event->mimeData()->text());
    Friend f = Friend::get(toxId);
    if (f)
    {
        // Save CircleWidget before changing the Id
        int circleId = Settings::getInstance().getFriendCircleID(f.getToxId());
        CircleWidget* circleWidget = CircleWidget::getFromID(circleId);

        moveWidget(widget, f.getStatus(), true);

        if (circleWidget)
            circleWidget->updateStatus();
    }
}

void FriendListWidget::dayTimeout()
{
    if (mode == Activity)
    {
        setMode(Name);
        setMode(Activity); // Refresh all.
    }

    dayTimer->start(timeUntilTomorrow());
}

void FriendListWidget::moveWidget(FriendWidget* widget, Status s, bool add)
{
    if (mode == Name)
    {
        Friend f = widget->getFriend();
        int circleId = Settings::getInstance().getFriendCircleID(f.getToxId());
        CircleWidget* circleWidget = CircleWidget::getFromID(circleId);

        if (circleWidget == nullptr || add)
        {
            if (circleId != -1)
                Settings::getInstance().setFriendCircleID(f.getToxId(), -1);

            listLayout->addFriendWidget(widget, s);
            return;
        }

        circleWidget->addFriendWidget(widget, s);
    }
    else
    {
        Friend f = widget->getFriend();
        QDate activityDate = getDateFriend(f);
        Time time = getTime(activityDate);
        QWidget *w = activityLayout->itemAt(time)->widget();
        CategoryWidget* categoryWidget = qobject_cast<CategoryWidget*>(w);
        categoryWidget->addFriendWidget(widget, f.getStatus());
        categoryWidget->show();
    }
}

void FriendListWidget::onFriendRemoved(uint32_t friendId)
{
    removeFriendWidget(friendId);
}

void FriendListWidget::onGroupRemoved(int groupId)
{
    removeGroupWidget(groupId);
}

void FriendListWidget::updateActivityDate(const QDate& date)
{
    if (mode != Activity)
        return;

    CategoryWidget* categoryWidget = static_cast<CategoryWidget*>(activityLayout->itemAt(getTime(date))->widget());
    categoryWidget->updateStatus();

    categoryWidget->setVisible(categoryWidget->hasChatrooms());
}

void FriendListWidget::reloadTheme()
{
    for (FriendWidget* w : friendWidgets)
    {
        w->reloadTheme();
    }

    for (GroupWidget* w : groupWidgets)
    {
        w->reloadTheme();
    }
}

void FriendListWidget::onFriendAliasChanged(const Friend& f, QString alias)
{
    alias = f.getDisplayedName();

    FriendWidget* fw = friendWidgets[f.getFriendId()];
    if (fw)
    {
        fw->setName(alias);

        for (Group *g : Group::getAll())
        {
            g->regeneratePeerList();
        }
    }
}

void FriendListWidget::onFriendAvatarChanged(const Friend& f, const QPixmap& avatar)
{
    FriendWidget* fw = friendWidgets[f.getFriendId()];

    if (fw)
        fw->updateAvatar(f.getFriendId(), avatar);
}

void FriendListWidget::onFriendStatusChanged(const Friend& f, Status status)
{
    FriendWidget* fw = friendWidgets[f.getFriendId()];

    if (fw)
    {
        if (status == Status::Offline)
            moveWidget(fw, Status::Online);
        else if (status == Status::Offline)
            moveWidget(fw, Status::Offline);

        fw->updateStatusLight(/*status*/);
    }
}

void FriendListWidget::onFriendStatusMessageChanged(const Friend& f, QString message)
{
    FriendWidget* fw = friendWidgets[f.getFriendId()];

    if (fw)
        fw->setStatusMsg(message);
}

CircleWidget* FriendListWidget::createCircleWidget(int id)
{
    if (id == -1)
        id = Settings::getInstance().addCircle();

    // Stop, after it has been created. Code after this is for displaying.
    if (mode == Activity)
        return nullptr;

    assert(circleLayout != nullptr);

    CircleWidget* circleWidget = new CircleWidget(this, id);
    circleLayout->addSortedWidget(circleWidget);
    connect(circleWidget, &CircleWidget::renameRequested, this, &FriendListWidget::renameCircleWidget);
    circleWidget->show(); // Avoid flickering.

    return circleWidget;
}

QLayout* FriendListWidget::nextLayout(QLayout* layout, bool forward) const
{
    if (layout == groupLayout.getLayout())
    {
        if (forward)
        {
            if (groupsOnTop)
                return listLayout->getLayoutOnline();

            return listLayout->getLayoutOffline();
        }
        else
        {
            if (groupsOnTop)
                return circleLayout->getLayout();

            return listLayout->getLayoutOnline();
        }
    }
    else if (layout == listLayout->getLayoutOnline())
    {
        if (forward)
        {
            if (groupsOnTop)
                return listLayout->getLayoutOffline();

            return groupLayout.getLayout();
        }
        else
        {
            if (groupsOnTop)
                return groupLayout.getLayout();

            return circleLayout->getLayout();
        }
    }
    else if (layout == listLayout->getLayoutOffline())
    {
        if (forward)
            return circleLayout->getLayout();
        else if (groupsOnTop)
            return listLayout->getLayoutOnline();

        return groupLayout.getLayout();
    }
    else if (layout == circleLayout->getLayout())
    {
        if (forward)
        {
            if (groupsOnTop)
                return groupLayout.getLayout();

            return listLayout->getLayoutOnline();
        }
        else
            return listLayout->getLayoutOffline();
    }
    return nullptr;
}
