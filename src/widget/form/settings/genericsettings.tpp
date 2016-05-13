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

#include "genericsettings.h"
#include "src/persistence/settings.h"

/*
 * Convenience wrapper for ::connect that also creates a secondary
 * connection to call `Settings::getInstance().saveGlobal();` after
 * the requested callback.
 */
template<typename Sender, typename Signal, typename... Rest>
QMetaObject::Connection GenericForm::connect_global_saver(const Sender sender, const Signal signal, const Rest... receiver)
{
    connect(sender, signal, receiver...);
    return connect(sender, signal, []() {
        Settings::getInstance().saveGlobal();
    });
}

/*
 * Convenience wrapper for ::connect that also creates a secondary
 * connection to call `Settings::getInstance().savePersonal();` after
 * the requested callback.
 */
template<typename Sender, typename Signal, typename... Rest>
QMetaObject::Connection GenericForm::connect_personal_saver(const Sender sender, const Signal signal, const Rest... receiver)
{
    connect(sender, signal, receiver...);
    return connect(sender, signal, []() {
        Settings::getInstance().savePersonal();
    });
}
