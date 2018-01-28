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


#ifndef MUMBLE_MUMBLE_GLOBALSHORTCUT_MACX_H_
#define MUMBLE_MUMBLE_GLOBALSHORTCUT_MACX_H_

#include <QtCore/qsystemdetection.h>
#if defined(__APPLE__) && defined(__MACH__)
#include <stdlib.h>
#include <QtCore/QObject>

#include <ApplicationServices/ApplicationServices.h>

#include "GlobalShortcut.h"
#include "Global.h"

class GlobalShortcutMac : public GlobalShortcutEngine {
	private:
		Q_OBJECT
		Q_DISABLE_COPY(GlobalShortcutMac)
	public:
		GlobalShortcutMac();
		~GlobalShortcutMac() Q_DECL_OVERRIDE;
		QString buttonName(const QVariant &) Q_DECL_OVERRIDE;
		void dumpEventTaps();
		void needRemap() Q_DECL_OVERRIDE;
		bool handleModButton(CGEventFlags newmask);
		bool canSuppress() Q_DECL_OVERRIDE;

	void setEnabled(bool) Q_DECL_OVERRIDE;
	bool enabled() Q_DECL_OVERRIDE;
	bool canDisable() Q_DECL_OVERRIDE;

	public slots:
		void forwardEvent(void *evt);

	protected:
		CFRunLoopRef loop;
		CFMachPortRef port;
		CGEventFlags modmask;
		UCKeyboardLayout *kbdLayout;

		void run() Q_DECL_OVERRIDE;

		static CGEventRef callback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *udata);
		QString translateMouseButton(const unsigned int keycode) const;
		QString translateModifierKey(const unsigned int keycode) const;
		QString translateKeyName(const unsigned int keycode) const;
};

#endif // defined(__APPLE__) && defined(__MACH__)
#endif

