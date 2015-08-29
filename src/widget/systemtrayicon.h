/*
    Copyright © 2015 by The qTox Project

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


#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include "systemtrayicon_private.h"
#include "widget.h"
#include <QObject>

class QSystemTrayIcon;
class QMenu;

class SystemTrayIcon : public QObject
{
    Q_OBJECT
public:
    SystemTrayIcon(Widget *parent);
    ~SystemTrayIcon();
    void setContextMenu(QMenu* menu);
    void show();
    void hide();
    void setVisible(bool);
    void setIcon(QIcon &icon);

signals:
    void activated(QSystemTrayIcon::ActivationReason);

private:
    QString extractIconToFile(QIcon icon, QString name="icon");

private:
    SystrayBackendType backendType;
    QSystemTrayIcon* qtIcon;
    Widget *parent;

#ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    AppIndicator *unityIndicator;
    GtkWidget *unityMenu;
#endif
#ifdef ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND
    StatusNotifier* statusNotifier;
    GtkWidget* snMenu;
#endif
#ifdef ENABLE_SYSTRAY_GTK_BACKEND
    GtkStatusIcon* gtkIcon;
    GtkWidget* gtkMenu;
#endif
};

#endif // SYSTEMTRAYICON_H
