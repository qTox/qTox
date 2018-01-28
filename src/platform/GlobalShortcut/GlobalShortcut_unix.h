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

#ifndef MUMBLE_MUMBLE_GLOBALSHORTCUT_UNIX_H_
#define MUMBLE_MUMBLE_GLOBALSHORTCUT_UNIX_H_

#include <QFile>
#include <QSet>
#include <ostream>
#include "GlobalShortcut.h"

//#include <X11/Xlib.h>
// These should be coming from Xlib.h, but with conflicting types and unity build, this is easier..
typedef struct _XDisplay Display;
typedef unsigned long Window;

class GlobalShortcutX : public GlobalShortcutEngine {
	private:
		Q_OBJECT
		Q_DISABLE_COPY(GlobalShortcutX)
	public:
		Display *display;
		QSet<Window> qsRootWindows;
		int iXIopcode;
		QSet<int> qsMasterDevices;

		volatile bool bRunning;
		QSet<QString> qsKeyboards;
		QMap<QString, QFile *> qmInputDevices;

		GlobalShortcutX();
		~GlobalShortcutX() override;
		void run() override;
		QString toString(const QVariant& shortcut);

		void queryXIMasterList();
	public slots:
		void displayReadyRead(int);
		void inputReadyRead(int);
		void directoryChanged(const QString &);
};
#endif
