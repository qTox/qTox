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

#ifndef MUMBLE_MUMBLE_GLOBALSHORTCUT_H_
#define MUMBLE_MUMBLE_GLOBALSHORTCUT_H_

#include <QtCore/QtGlobal>
#include <QtCore/QThread>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QStyledItemDelegate>

struct Shortcut {
    QList<QVariant> qlButtons;
    QVariant qvData;
    bool bSuppress;
    bool operator <(const Shortcut &) const;
    bool operator ==(const Shortcut &) const;
};


class GlobalShortcut : public QObject {
		friend class GlobalShortcutEngine;
	private:
		Q_OBJECT
		Q_DISABLE_COPY(GlobalShortcut)
	protected:
		QList<QVariant> qlActive;
	signals:
		void down(QVariant);
		void triggered(bool, QVariant);
	public:
		GlobalShortcut(Shortcut def);
		~GlobalShortcut() override;
		Shortcut shortcut;

		QString toString()	;
		bool active() const {
			return ! qlActive.isEmpty();
		}
};

struct ShortcutKey {
    Shortcut s;
    int iNumUp;
    GlobalShortcut *gs;
};

/**
 * Creates a background thread which handles global shortcut behaviour. This class inherits
 * a system unspecific interface and basic functionality to the actually used native backend
 * classes (GlobalShortcutPlatform).
 *
 * @see GlobalShortcutX
 * @see GlobalShortcutMac
 * @see GlobalShortcutWin
 */
class GlobalShortcutEngine : public QThread {
	private:
		Q_OBJECT
		Q_DISABLE_COPY(GlobalShortcutEngine)
	protected:
		GlobalShortcutEngine(QObject *p = NULL);
	public:
		static GlobalShortcutEngine *platformInit();

		QList<GlobalShortcut*> qmShortcuts; // list of shortcuts
		QList<QVariant> qlDownButtons; // button currently depressed, needed for multi-key shortcuts
		QList<QVariant> qlSuppressed; // buttons which block system from seeing them

		QList<QVariant> qlButtonList; // list of buttons that are part of any shortcut?
		QList<QList<ShortcutKey *> > qlShortcutList; // list of shortcuts?


		~GlobalShortcutEngine() override;

		bool handleButton(const QVariant &, bool);
		void add(GlobalShortcut *);
		void remove(GlobalShortcut *);

	signals:
		void buttonPressed(bool last); // should be shortcut pressed? should just call lambda?
};

#endif
