/*
 * statusnotifier - Copyright (C) 2014 Olivier Brunel
 *
 * interfaces.h
 * Copyright (C) 2014 Olivier Brunel <jjk@jjacky.com>
 *
 * This file is part of statusnotifier.
 *
 * statusnotifier is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * statusnotifier is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * statusnotifier. If not, see http://www.gnu.org/licenses/
 */

#pragma once

G_BEGIN_DECLS

#define WATCHER_NAME "org.kde.StatusNotifierWatcher"
#define WATCHER_OBJECT "/StatusNotifierWatcher"
#define WATCHER_INTERFACE "org.kde.StatusNotifierWatcher"

#define ITEM_NAME "org.kde.StatusNotifierItem"
#define ITEM_OBJECT "/StatusNotifierItem"
#define ITEM_INTERFACE "org.kde.StatusNotifierItem"

static const gchar watcher_xml[] =
    "<node>"
    "   <interface name='org.kde.StatusNotifierWatcher'>"
    "       <property name='IsStatusNotifierHostRegistered' type='b' access='read' />"
    "       <method name='RegisterStatusNotifierItem'>"
    "           <arg name='service' type='s' direction='in' />"
    "       </method>"
    "       <signal name='StatusNotifierHostRegistered' />"
    "       <signal name='StatusNotifierHostUnregistered' />"
    "   </interface>"
    "</node>";


static const gchar item_xml[] =
    "<node>"
    "   <interface name='org.kde.StatusNotifierItem'>"
    "       <property name='Id' type='s' access='read' />"
    "       <property name='Category' type='s' access='read' />"
    "       <property name='Title' type='s' access='read' />"
    "       <property name='Status' type='s' access='read' />"
    "       <property name='WindowId' type='i' access='read' />"
    "       <property name='IconName' type='s' access='read' />"
    "       <property name='IconPixmap' type='(iiay)' access='read' />"
    "       <property name='OverlayIconName' type='s' access='read' />"
    "       <property name='OverlayIconPixmap' type='(iiay)' access='read' />"
    "       <property name='AttentionIconName' type='s' access='read' />"
    "       <property name='AttentionIconPixmap' type='(iiay)' access='read' />"
    "       <property name='AttentionMovieName' type='s' access='read' />"
    "       <property name='ToolTip' type='(s(iiay)ss)' access='read' />"
    "       <method name='ContextMenu'>"
    "           <arg name='x' type='i' direction='in' />"
    "           <arg name='y' type='i' direction='in' />"
    "       </method>"
    "       <method name='Activate'>"
    "           <arg name='x' type='i' direction='in' />"
    "           <arg name='y' type='i' direction='in' />"
    "       </method>"
    "       <method name='SecondaryActivate'>"
    "           <arg name='x' type='i' direction='in' />"
    "           <arg name='y' type='i' direction='in' />"
    "       </method>"
    "       <method name='Scroll'>"
    "           <arg name='delta' type='i' direction='in' />"
    "           <arg name='orientation' type='s' direction='in' />"
    "       </method>"
    "       <signal name='NewTitle' />"
    "       <signal name='NewIcon' />"
    "       <signal name='NewAttentionIcon' />"
    "       <signal name='NewOverlayIcon' />"
    "       <signal name='NewToolTip' />"
    "       <signal name='NewStatus'>"
    "           <arg name='status' type='s' />"
    "       </signal>"
    "   </interface>"
    "</node>";

G_END_DECLS
