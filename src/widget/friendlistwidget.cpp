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
#include "friendlistlayout.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/persistence/settings.h"
#include "friendwidget.h"
#include "groupwidget.h"
#include "circlewidget.h"
#include "widget.h"
#include "src/persistence/historykeeper.h"
#include <QGridLayout>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <cassert>

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

Time getTime(const QDate date)
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

QDate getDateFriend(Friend* contact)
{
    QDate date = Settings::getInstance().getFriendActivity(contact->getToxId());

    if (date.isNull() && Settings::getInstance().getEnableLogging())
        date = HistoryKeeper::getInstance()->getLatestDate(contact->getToxId().publicKey);

    return date;
}

FriendListWidget::FriendListWidget(Widget* parent, bool groupsOnTop)
    : QWidget(parent)
    // Prevent Valgrind from complaining. We're changing this to Name here.
    // Must be Activity for change to take effect.
    , mode(Activity)
    , groupsOnTop(groupsOnTop)
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

        QList<Friend*> friendList = FriendList::getAllFriends();
        for (Friend* contact : friendList)
        {
            int circleId = Settings::getInstance().getFriendCircleID(contact->getToxId());
            addFriendWidget(contact->getFriendWidget(), contact->getStatus(), circleId);
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

        reDraw();
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
        if (last7DaysWasLastMonth())
        {
            CategoryWidget* categoryThisMonth = new CategoryWidget(this);
            categoryThisMonth ->setName(tr("This month", "Category for sorting friends by activity"));
            activityLayout->addWidget(categoryThisMonth);
            currentDate = currentDate.addMonths(-1);
        }

        CategoryWidget* categoryLast1Month = new CategoryWidget(this);
        categoryLast1Month ->setName(QDate::longMonthName(currentDate.month()));
        activityLayout->addWidget(categoryLast1Month);

        currentDate = currentDate.addMonths(-1);
        CategoryWidget* categoryLast2Month = new CategoryWidget(this);
        categoryLast2Month ->setName(QDate::longMonthName(currentDate.month()));
        activityLayout->addWidget(categoryLast2Month);

        currentDate = currentDate.addMonths(-1);
        CategoryWidget* categoryLast3Month = new CategoryWidget(this);
        categoryLast3Month ->setName(QDate::longMonthName(currentDate.month()));
        activityLayout->addWidget(categoryLast3Month);

        currentDate = currentDate.addMonths(-1);
        CategoryWidget* categoryLast4Month = new CategoryWidget(this);
        categoryLast4Month ->setName(QDate::longMonthName(currentDate.month()));
        activityLayout->addWidget(categoryLast4Month);

        currentDate = currentDate.addMonths(-1);
        CategoryWidget* categoryLast5Month = new CategoryWidget(this);
        categoryLast5Month ->setName(QDate::longMonthName(currentDate.month()));
        activityLayout->addWidget(categoryLast5Month);

        CategoryWidget* categoryOlder = new CategoryWidget(this);
        categoryOlder->setName(tr("Older than 6 Months", "Category for sorting friends by activity"));
        activityLayout->addWidget(categoryOlder);

        CategoryWidget* categoryNever = new CategoryWidget(this);
        categoryNever->setName(tr("Unknown", "Category for sorting friends by activity"));
        activityLayout->addWidget(categoryNever);

        QList<Friend*> friendList = FriendList::getAllFriends();
        for (Friend* contact : friendList)
        {
            QDate activityDate = getDateFriend(contact);
            Time time = getTime(activityDate);
            CategoryWidget* categoryWidget = dynamic_cast<CategoryWidget*>(activityLayout->itemAt(time)->widget());
            categoryWidget->addFriendWidget(contact->getFriendWidget(), contact->getStatus());
        }

        for (int i = 0; i < activityLayout->count(); ++i)
        {
            CategoryWidget* categoryWidget = dynamic_cast<CategoryWidget*>(activityLayout->itemAt(i)->widget());
            categoryWidget->setVisible(categoryWidget->hasChatrooms());
        }

        listLayout->removeItem(listLayout->getLayoutOnline());
        listLayout->removeItem(listLayout->getLayoutOffline());
        listLayout->removeItem(circleLayout->getLayout());
        listLayout->insertLayout(1, activityLayout);

        if (circleLayout != nullptr)
        {
            QLayoutItem* item;
            while ((item = circleLayout->getLayout()->takeAt(0)) != nullptr)
            {
                delete item->widget();
                delete item;
            }
            delete circleLayout;
            circleLayout = nullptr;
        }

        reDraw();
    }
}

FriendListWidget::Mode FriendListWidget::getMode() const
{
    return mode;
}

void FriendListWidget::addGroupWidget(GroupWidget* widget)
{
    groupLayout.addSortedWidget(widget);
    connect(widget, &GroupWidget::renameRequested, this, &FriendListWidget::renameGroupWidget);
}

void FriendListWidget::addFriendWidget(FriendWidget* w, Status s, int circleIndex)
{
    CircleWidget* circleWidget = CircleWidget::getFromID(circleIndex);
    if (circleWidget == nullptr)
        moveWidget(w, s, true);
    else
        circleWidget->addFriendWidget(w, s);
}

void FriendListWidget::removeFriendWidget(FriendWidget* w)
{
    Friend* contact = FriendList::findFriend(w->friendId);
    if (mode == Activity)
    {
        QDate activityDate = getDateFriend(contact);
        Time time = getTime(activityDate);
        CategoryWidget* categoryWidget = dynamic_cast<CategoryWidget*>(activityLayout->itemAt(time)->widget());
        categoryWidget->removeFriendWidget(w, contact->getStatus());
        categoryWidget->setVisible(categoryWidget->hasChatrooms());
    }
    else
    {
        int id = Settings::getInstance().getFriendCircleID(contact->getToxId());
        CircleWidget* circleWidget = CircleWidget::getFromID(id);
        if (circleWidget != nullptr)
        {
            circleWidget->removeFriendWidget(w, contact->getStatus());
            Widget::getInstance()->searchCircle(circleWidget);
        }
    }
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
            CircleWidget* circleOriginal = CircleWidget::getFromID(Settings::getInstance().getFriendCircleID(FriendList::findFriend(friendWidget->friendId)->getToxId()));

            circleWidget->addFriendWidget(friendWidget, FriendList::findFriend(friendWidget->friendId)->getStatus());
            circleWidget->setExpanded(true);

            if (circleOriginal != nullptr)
                Widget::getInstance()->searchCircle(circleOriginal);

        }

        Widget::getInstance()->searchCircle(circleWidget);
        circleWidget->editName();
    }
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

void FriendListWidget::renameGroupWidget(GroupWidget* groupWidget, const QString &newName)
{
    groupLayout.removeSortedWidget(groupWidget);
    groupWidget->setName(newName);
    groupLayout.addSortedWidget(groupWidget);
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

    reDraw();
}

void FriendListWidget::cycleContacts(GenericChatroomWidget* activeChatroomWidget, bool forward)
{
    if (activeChatroomWidget == nullptr)
        return;

    int index = -1;
    FriendWidget* friendWidget = dynamic_cast<FriendWidget*>(activeChatroomWidget);

    if (mode == Activity)
    {
        if (friendWidget == nullptr)
            return;

        QDate activityDate = getDateFriend(FriendList::findFriend(friendWidget->friendId));
        index = getTime(activityDate);
        CategoryWidget* categoryWidget = dynamic_cast<CategoryWidget*>(activityLayout->itemAt(index)->widget());

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

            CategoryWidget* categoryWidget = dynamic_cast<CategoryWidget*>(activityLayout->itemAt(index)->widget());

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
        circleWidget = CircleWidget::getFromID(Settings::getInstance().getFriendCircleID(FriendList::findFriend(friendWidget->friendId)->getToxId()));
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
        GroupWidget* groupWidget = dynamic_cast<GroupWidget*>(activeChatroomWidget);
        if (groupWidget != nullptr)
        {
            currentLayout = groupLayout.getLayout();
            index = groupLayout.indexOfSortedWidget(groupWidget);
        }
        else
        {
            return;
        };
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
            GenericChatroomWidget* chatWidget = dynamic_cast<GenericChatroomWidget*>(currentLayout->itemAt(index)->widget());

            if (chatWidget != nullptr)
                emit chatWidget->chatroomWidgetClicked(chatWidget);

            return;
        }
        else if (currentLayout == circleLayout->getLayout())
        {
            circleWidget = dynamic_cast<CircleWidget*>(currentLayout->itemAt(index)->widget());
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

QVector<CircleWidget*> FriendListWidget::getAllCircles()
{
    QVector<CircleWidget*> vec;
    vec.reserve(circleLayout->getLayout()->count());
    for (int i = 0; i < circleLayout->getLayout()->count(); ++i)
    {
        vec.push_back(dynamic_cast<CircleWidget*>(circleLayout->getLayout()->itemAt(i)->widget()));
    }
    return vec;
}

void FriendListWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("friend"))
        event->acceptProposedAction();
}

void FriendListWidget::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasFormat("friend"))
    {
        int friendId = event->mimeData()->data("friend").toInt();
        Friend* f = FriendList::findFriend(friendId);
        assert(f != nullptr);

        FriendWidget* widget = f->getFriendWidget();
        assert(widget != nullptr);

        // Update old circle after moved.
        CircleWidget* circleWidget = CircleWidget::getFromID(Settings::getInstance().getFriendCircleID(f->getToxId()));

        moveWidget(widget, f->getStatus(), true);

        if (circleWidget != nullptr)
            circleWidget->updateStatus();
    }
}

void FriendListWidget::moveWidget(FriendWidget* w, Status s, bool add)
{
    if (mode == Name)
    {
        int circleId = Settings::getInstance().getFriendCircleID(FriendList::findFriend(w->friendId)->getToxId());
        CircleWidget* circleWidget = CircleWidget::getFromID(circleId);

        if (circleWidget == nullptr || add)
        {
            if (circleId != -1)
                Settings::getInstance().setFriendCircleID(FriendList::findFriend(w->friendId)->getToxId(), -1);

            listLayout->addFriendWidget(w, s);
            return;
        }

        circleWidget->addFriendWidget(w, s);
    }
    else
    {
        Friend* contact = FriendList::findFriend(w->friendId);
        QDate activityDate = getDateFriend(contact);
        Time time = getTime(activityDate);
        CategoryWidget* categoryWidget = dynamic_cast<CategoryWidget*>(activityLayout->itemAt(time)->widget());
        categoryWidget->addFriendWidget(contact->getFriendWidget(), contact->getStatus());
        categoryWidget->show();
    }
}

// update widget after add/delete/hide/show
void FriendListWidget::reDraw()
{
    hide();
    show();
    resize(QSize()); //lifehack
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
    connect(this, &FriendListWidget::onCompactChanged, circleWidget, &CircleWidget::onCompactChanged);
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
