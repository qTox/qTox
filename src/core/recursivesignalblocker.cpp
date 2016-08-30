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

#include "recursivesignalblocker.h"

#include <QObject>
#if HAVE_QSIGNAL_BLOCKER
#include <QSignalBlocker>
#endif

/**
@class  RecursiveSignalBlocker
@brief  Recursively blocks all signals from an object and its children.
@note   All children must be created before the blocker is used.

Wraps a QSignalBlocker on each object. Signals will be unblocked when the
blocker gets destroyed. According to QSignalBlocker, we are also exception safe.
*/

/**
@brief      Creates a QSignalBlocker recursively on the object and child objects.
@param[in]  object  the object, which signals should be blocked
*/
RecursiveSignalBlocker::RecursiveSignalBlocker(QObject* object)
{
#if HAVE_QSIGNAL_BLOCKER
    recursiveBlock(object);
#endif
}

RecursiveSignalBlocker::~RecursiveSignalBlocker()
{
#if HAVE_QSIGNAL_BLOCKER
    qDeleteAll(mBlockers);
#endif
}

#if HAVE_QSIGNAL_BLOCKER
/**
@brief      Recursively blocks all signals of the object.
@param[in]  object  the object to block
*/
void RecursiveSignalBlocker::recursiveBlock(QObject* object)
{
    mBlockers << new QSignalBlocker(object);

    for (QObject* child : object->children())
    {
        recursiveBlock(child);
    }
}
#endif
