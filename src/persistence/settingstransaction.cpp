/*
    Copyright © 2016 by The qTox Project

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

#include "settingstransaction.h"
#include "settings.h"
#include <climits>

/*
 * This family of classes represents an intent to make changes to
 * settings that need to be saved to disk after the changes have
 * been made. To use it, simply instantiate a local object before
 * making changes and let it fall out of scope afterwards.
 *
 * The motivation behind this design is to easily prevent cascading
 * changes from triggering multiple writes to disk. The destructor
 * function only makes a save call if the object being destroyed
 * was the last instance of its class alive.
 *
 * Think: "last one out closes the door."
 */
SettingsTransaction::SettingsTransaction()
{}

QSemaphore PersonalSettingsTransaction::activeTransactions(INT_MAX);
QMutex PersonalSettingsTransaction::lock;

PersonalSettingsTransaction::PersonalSettingsTransaction()
{
    QMutexLocker locker(&lock);

    activeTransactions.acquire();
}

PersonalSettingsTransaction::~PersonalSettingsTransaction()
{
    if (canceled)
        return;

    QMutexLocker locker(&lock);

    activeTransactions.release();

    bool save = activeTransactions.available() == INT_MAX;

    if (save)
        Settings::getInstance().savePersonal();
}

void PersonalSettingsTransaction::cancel()
{
    QMutexLocker locker(&lock);

    activeTransactions.release();
    canceled = true;
}

QSemaphore GlobalSettingsTransaction::activeTransactions(INT_MAX);
QMutex GlobalSettingsTransaction::lock;

GlobalSettingsTransaction::GlobalSettingsTransaction()
{
    QMutexLocker locker(&lock);

    activeTransactions.acquire();
}

GlobalSettingsTransaction::~GlobalSettingsTransaction()
{
    if (canceled)
        return;

    QMutexLocker locker(&lock);

    activeTransactions.release();

    bool save = activeTransactions.available() == INT_MAX;

    if (save)
        Settings::getInstance().saveGlobal();
}

void GlobalSettingsTransaction::cancel()
{
    QMutexLocker locker(&lock);

    activeTransactions.release();
    canceled = true;
}
