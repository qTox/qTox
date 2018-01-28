/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

    This file incorporates work covered by the following copyright and  
    permission notice: 

        Copyright 2005-2018 The Mumble Developers. All rights reserved.
        Use of this source code is governed by a BSD-style license
        that can be found in the LICENSE file inside the GlobalShortcut
        directory or at <https://www.mumble.info/LICENSE>.
*/

#include "GlobalShortcut.h"
#include <QSet>
#include <QDebug>

GlobalShortcutEngine::GlobalShortcutEngine(QObject *p) : QThread(p) {
};

GlobalShortcutEngine::~GlobalShortcutEngine() {
	QSet<ShortcutKey *> qs;
	foreach(const QList<ShortcutKey*> &ql, qlShortcutList)
		qs += ql.toSet();

	foreach(ShortcutKey *sk, qs)
		delete sk;
}

/**
 * This function gets called internally to update the state
 * of a button.
 *
 * @return True if button is suppressed, otherwise false
*/
bool GlobalShortcutEngine::handleButton(const QVariant &button, bool down) {
	bool already = qlDownButtons.contains(button);
	if (already == down)
		return qlSuppressed.contains(button);
	if (down)
		qlDownButtons << button;
	else
		qlDownButtons.removeAll(button);

	int idx = qlButtonList.indexOf(button);

	if (idx == -1)
		return false;

	bool suppress = false;

	foreach(ShortcutKey *sk, qlShortcutList.at(idx)) {
		if (down) {
			sk->iNumUp--;
			if (sk->iNumUp == 0) {
				GlobalShortcut *gs = sk->gs;
				if (sk->s.bSuppress) {
					suppress = true;
					qlSuppressed << button;
				}
				if (! gs->qlActive.contains(sk->s.qvData)) {
					gs->qlActive << sk->s.qvData;
					emit gs->triggered(true, sk->s.qvData);
					emit gs->down(sk->s.qvData);
				}
			} else if (sk->iNumUp < 0) {
				sk->iNumUp = 0;
			}
		} else {
			if (qlSuppressed.contains(button)) {
				suppress = true;
				qlSuppressed.removeAll(button);
			}
			sk->iNumUp++;
			if (sk->iNumUp == 1) {
				GlobalShortcut *gs = sk->gs;
				if (gs->qlActive.contains(sk->s.qvData)) {
					gs->qlActive.removeAll(sk->s.qvData);
					emit gs->triggered(false, sk->s.qvData);
				}
			} else if (sk->iNumUp > sk->s.qlButtons.count()) {
				sk->iNumUp = sk->s.qlButtons.count();
			}
		}
	}
	return suppress;
}

void GlobalShortcutEngine::add(GlobalShortcut *gs) {
    if (qmShortcuts.contains(gs)) {
		qWarning() << "Attempted to register the same hotkey (" << gs << ") after it was already registered, ignoring";
		return;
	}
    qmShortcuts.append(gs);
    Shortcut sc = gs->shortcut;
    if (gs && ! sc.qlButtons.isEmpty()) {
        ShortcutKey *sk = new ShortcutKey;
        sk->s = sc;
        sk->iNumUp = sc.qlButtons.count();
        sk->gs = gs;

        foreach(const QVariant &button, sc.qlButtons) {
            int idx = qlButtonList.indexOf(button);
            if (idx == -1) {
                qlButtonList << button;
                qlShortcutList << QList<ShortcutKey *>();
                idx = qlButtonList.count() - 1;
            }
            qlShortcutList[idx] << sk;
        }
    }
}

void GlobalShortcutEngine::remove(GlobalShortcut *gs) {
	if (!qmShortcuts.contains(gs)) {
		qWarning() << "Attempted to remove a hotkey (" << gs << ") that was not registered, ignoring";
	} else {
		qmShortcuts.removeOne(gs);
	}
}

GlobalShortcut::GlobalShortcut(Shortcut def) : shortcut(def) {}

GlobalShortcut::~GlobalShortcut() {}
