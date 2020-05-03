/*
 * statusnotifier - Copyright (C) 2014 Olivier Brunel
 *
 * statusnotifier.h
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

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

typedef struct _StatusNotifier StatusNotifier;
typedef struct _StatusNotifierPrivate StatusNotifierPrivate;
typedef struct _StatusNotifierClass StatusNotifierClass;

#define TYPE_STATUS_NOTIFIER (status_notifier_get_type())
#define STATUS_NOTIFIER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_STATUS_NOTIFIER, StatusNotifier))
#define STATUS_NOTIFIER_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_STATUS_NOTIFIER, StatusNotiferClass))
#define IS_STATUS_NOTIFIER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_STATUS_NOTIFIER))
#define IS_STATUS_NOTIFIER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((obj), TYPE_STATUS_NOTIFIER))
#define STATUS_NOTIFIER_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_STATUS_NOTIFIER, StatusNotifierClass))

GType status_notifier_get_type(void) G_GNUC_CONST;

#define STATUS_NOTIFIER_ERROR g_quark_from_static_string("StatusNotifier error")
/**
 * StatusNotifierError:
 * @STATUS_NOTIFIER_ERROR_NO_CONNECTION: Failed to establish connection to
 * register service on session bus
 * @STATUS_NOTIFIER_ERROR_NO_NAME: Failed to acquire name for the item on the
 * session bus
 * @STATUS_NOTIFIER_ERROR_NO_WATCHER: No StatusNotifierWatcher found on the
 * session bus
 * @STATUS_NOTIFIER_ERROR_NO_HOST: No StatusNotifierHost registered with the
 * StatusNotifierWatcher
 *
 * Errors that can occur while trying to register the item. Note that errors
 * other the #StatusNotifierError might be returned.
 */
typedef enum {
    STATUS_NOTIFIER_ERROR_NO_CONNECTION = 0,
    STATUS_NOTIFIER_ERROR_NO_NAME,
    STATUS_NOTIFIER_ERROR_NO_WATCHER,
    STATUS_NOTIFIER_ERROR_NO_HOST
} StatusNotifierError;

/**
 * StatusNotifierState:
 * @STATUS_NOTIFIER_STATE_NOT_REGISTERED: Item hasn't yet been asked to
 * register, i.e. no call to status_notifier_register() have been made yet
 * @STATUS_NOTIFIER_STATE_REGISTERING: Item is in the process of registering.
 * This state is also valid after #StatusNotifier::registration-failed was
 * emitted, if the item is waiting for possible "recovery" (e.g. if no host was
 * registered on watcher, waiting for one to do so)
 * @STATUS_NOTIFIER_STATE_REGISTERED: Item was sucessfully registered on DBus
 * and StatusNotifierWatcher
 * @STATUS_NOTIFIER_STATE_FAILED: Registration failed, with no possible pending
 * recovery
 *
 * State in which a #StatusNotifier item can be. See status_notifier_register()
 * for more
 */
typedef enum {
    STATUS_NOTIFIER_STATE_NOT_REGISTERED = 0,
    STATUS_NOTIFIER_STATE_REGISTERING,
    STATUS_NOTIFIER_STATE_REGISTERED,
    STATUS_NOTIFIER_STATE_FAILED
} StatusNotifierState;

/**
 * StatusNotifierIcon:
 * @STATUS_NOTIFIER_ICON: The icon that can be used by the visualization to
 * identify the item.
 * @STATUS_NOTIFIER_ATTENTION_ICON: The icon that can be used by the
 * visualization when the item's status is
 * %STATUS_NOTIFIER_STATUS_NEEDS_ATTENTION.
 * @STATUS_NOTIFIER_OVERLAY_ICON: This can be used by the visualization to
 * indicate extra state information, for instance as an overlay for the main
 * icon.
 * @STATUS_NOTIFIER_TOOLTIP_ICON: The icon that can be used be the visualization
 * in the tooltip of the item.
 *
 * Possible icons that can be used on a status notifier item.
 */
typedef enum {
    STATUS_NOTIFIER_ICON = 0,
    STATUS_NOTIFIER_ATTENTION_ICON,
    STATUS_NOTIFIER_OVERLAY_ICON,
    STATUS_NOTIFIER_TOOLTIP_ICON,
    /*< private >*/
    _NB_STATUS_NOTIFIER_ICONS
} StatusNotifierIcon;

/**
 * StatusNotifierCategory:
 * @STATUS_NOTIFIER_CATEGORY_APPLICATION_STATUS: The item describes the status
 * of a generic application, for instance the current state of a media player.
 * In the case where the category of the item can not be known, such as when the
 * item is being proxied from another incompatible or emulated system, this can
 * be used a sensible default fallback.
 * @STATUS_NOTIFIER_CATEGORY_COMMUNICATIONS: The item describes the status of
 * communication oriented applications, like an instant messenger or an email
 * client.
 * @STATUS_NOTIFIER_CATEGORY_SYSTEM_SERVICES: The item describes services of the
 * system not seen as a stand alone application by the user, such as an
 * indicator for the activity of a disk indexing service.
 * @STATUS_NOTIFIER_CATEGORY_HARDWARE: The item describes the state and control
 * of a particular hardware, such as an indicator of the battery charge or sound
 * card volume control.
 *
 * The category of the status notifier item.
 */
typedef enum {
    STATUS_NOTIFIER_CATEGORY_APPLICATION_STATUS = 0,
    STATUS_NOTIFIER_CATEGORY_COMMUNICATIONS,
    STATUS_NOTIFIER_CATEGORY_SYSTEM_SERVICES,
    STATUS_NOTIFIER_CATEGORY_HARDWARE
} StatusNotifierCategory;

/**
 * StatusNotifierStatus:
 * @STATUS_NOTIFIER_STATUS_PASSIVE: The item doesn't convey important
 * information to the user, it can be considered an "idle" status and is likely
 * that visualizations will chose to hide it.
 * @STATUS_NOTIFIER_STATUS_ACTIVE: The item is active, is more important that
 * the item will be shown in some way to the user.
 * @STATUS_NOTIFIER_STATUS_NEEDS_ATTENTION: The item carries really important
 * information for the user, such as battery charge running out and is wants to
 * incentive the direct user intervention. Visualizations should emphasize in
 * some way the items with this status.
 *
 * The status of the status notifier item or its associated application.
 */
typedef enum {
    STATUS_NOTIFIER_STATUS_PASSIVE = 0,
    STATUS_NOTIFIER_STATUS_ACTIVE,
    STATUS_NOTIFIER_STATUS_NEEDS_ATTENTION
} StatusNotifierStatus;

/**
 * StatusNotifierScrollOrientation:
 * @STATUS_NOTIFIER_SCROLL_ORIENTATION_HORIZONTAL: Scroll request was
 * horizontal.
 * @STATUS_NOTIFIER_SCROLL_ORIENTATION_VERTICAL: Scroll request was vertical.
 *
 * The orientation of a scroll request performed on the representation of the
 * item in the visualization.
 */
typedef enum {
    STATUS_NOTIFIER_SCROLL_ORIENTATION_HORIZONTAL = 0,
    STATUS_NOTIFIER_SCROLL_ORIENTATION_VERTICAL
} StatusNotifierScrollOrientation;

struct _StatusNotifier
{
    /*< private >*/
    GObject parent;
    StatusNotifierPrivate* priv;
};

/**
 * StatusNotifierClass:
 * @parent_class: Parent class
 * @registration_failed: When registering the item failed, e.g. because there's
 * no StatusNotifierHost registered (yet); If this occurs, you should fallback
 * to using the systray
 * @context_menu: Item should show a context menu, this is typically a
 * consequence of user input, such as mouse right click over the graphical
 * representation of the item.
 * @activate: Activation of the item was requested, this is typically a
 * consequence of user input, such as mouse left click over the graphical
 * representation of the item.
 * @secondary_activate: Secondary and less important form of activation
 * (compared to @activate) of the item was requested. This is typically a
 * consequence of user input, such as mouse middle click over the graphical
 * representation of the item.
 * @scroll: The user asked for a scroll action. This is caused from input such
 * as mouse wheel over the graphical representation of the item.
 */
struct _StatusNotifierClass
{
    GObjectClass parent_class;

    /* signals */
    void (*registration_failed)(StatusNotifier* sn, GError* error);

    gboolean (*context_menu)(StatusNotifier* sn, gint x, gint y);
    gboolean (*activate)(StatusNotifier* sn, gint x, gint y);
    gboolean (*secondary_activate)(StatusNotifier* sn, gint x, gint y);
    gboolean (*scroll)(StatusNotifier* sn, gint delta, StatusNotifierScrollOrientation orientation);
};

StatusNotifier* status_notifier_new_from_pixbuf(const gchar* id, StatusNotifierCategory category,
                                                GdkPixbuf* pixbuf);
StatusNotifier* status_notifier_new_from_icon_name(const gchar* id, StatusNotifierCategory category,
                                                   const gchar* icon_name);
const gchar* status_notifier_get_id(StatusNotifier* sn);
StatusNotifierCategory status_notifier_get_category(StatusNotifier* sn);
void status_notifier_set_from_pixbuf(StatusNotifier* sn, StatusNotifierIcon icon, GdkPixbuf* pixbuf);
void status_notifier_set_from_icon_name(StatusNotifier* sn, StatusNotifierIcon icon,
                                        const gchar* icon_name);
gboolean status_notifier_has_pixbuf(StatusNotifier* sn, StatusNotifierIcon icon);
GdkPixbuf* status_notifier_get_pixbuf(StatusNotifier* sn, StatusNotifierIcon icon);
gchar* status_notifier_get_icon_name(StatusNotifier* sn, StatusNotifierIcon icon);
void status_notifier_set_attention_movie_name(StatusNotifier* sn, const gchar* movie_name);
gchar* status_notifier_get_attention_movie_name(StatusNotifier* sn);
void status_notifier_set_title(StatusNotifier* sn, const gchar* title);
gchar* status_notifier_get_title(StatusNotifier* sn);
void status_notifier_set_status(StatusNotifier* sn, StatusNotifierStatus status);
StatusNotifierStatus status_notifier_get_status(StatusNotifier* sn);
void status_notifier_set_window_id(StatusNotifier* sn, guint32 window_id);
guint32 status_notifier_get_window_id(StatusNotifier* sn);
void status_notifier_freeze_tooltip(StatusNotifier* sn);
void status_notifier_thaw_tooltip(StatusNotifier* sn);
void status_notifier_set_tooltip(StatusNotifier* sn, const gchar* icon_name, const gchar* title,
                                 const gchar* body);
void status_notifier_set_tooltip_title(StatusNotifier* sn, const gchar* title);
gchar* status_notifier_get_tooltip_title(StatusNotifier* sn);
void status_notifier_set_tooltip_body(StatusNotifier* sn, const gchar* body);
gchar* status_notifier_get_tooltip_body(StatusNotifier* sn);
void status_notifier_register(StatusNotifier* sn);
StatusNotifierState status_notifier_get_state(StatusNotifier* sn);

G_END_DECLS
