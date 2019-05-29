/*
    Copyright © 2019 by The qTox Project Contributors

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

#ifndef I_DIALOGS_H
#define I_DIALOGS_H

class ContactId;
class GroupId;
class ToxPk;

class IDialogs
{
public:
    virtual bool hasContact(const ContactId& contactId) const = 0;
    virtual bool isContactActive(const ContactId& contactId) const = 0;

    virtual void removeFriend(const ToxPk& friendPk) = 0;
    virtual void removeGroup(const GroupId& groupId) = 0;

    virtual int chatroomCount() const = 0;
};

#endif // I_DIALOGS_H
