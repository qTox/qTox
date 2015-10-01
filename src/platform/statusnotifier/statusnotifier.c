/*
 * statusnotifier - Copyright (C) 2014 Olivier Brunel
 *
 * statusnotifier.c
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

//#include "config.h"

#include <unistd.h>
#include <gdk/gdk.h>
#include "statusnotifier.h"
#include "enums.h"
#include "interfaces.h"
#include "closures.h"

/**
 * SECTION:statusnotifier
 * @Short_description: A StatusNotifierItem as per KDE's specifications
 *
 * Starting with Plasma Next, KDE doesn't support the XEmbed systray in favor of
 * their own Status Notifier Specification.
 *
 * A #StatusNotifier is a #GObject that can be used to represent a
 * StatusNotifierItem, handling all the DBus implementation and leaving you
 * simply dealing with regular properties and signals.
 *
 * You can simply create a new #StatusNotifier using one of the helper function,
 * e.g. status_notifier_new_from_icon_name(), or simply creating an object as
 * usual - you then just need to make sure to specify #StatusNotifier:id :
 * <programlisting>
 * sn = (StatusNotifier *) g_object_new (TYPE_STATUS_NOTIFIER,
 *      "id",                       "app-id",
 *      "status",                   STATUS_NOTIFIER_STATUS_NEEDS_ATTENTION,
 *      "main-icon-name",           "app-icon",
 *      "attention-icon-pixbuf",    pixbuf,
 *      "tooltip-title",            "My tooltip",
 *      "tooltip-body",             "This is an item about &lt;b&gt;app&lt;/b&gt;",
 *      NULL);
 * </programlisting>
 *
 * You can also set properties (other than id) after creation. Once ready, call
 * status_notifier_register() to register the item on the session bus and to the
 * StatusNotifierWatcher.
 *
 * If an error occurs, signal #StatusNotifier::registration-failed will be
 * emitted. On success, #StatusNotifier:state will be
 * %STATUS_NOTIFIER_STATE_REGISTERED. See status_notifier_register() for more.
 *
 * Once registered, you can change properties as needed, and the proper DBus
 * signal will be emitted to let visualizations (hosts) know, and connect to the
 * signals (such as #StatusNotifier::context-menu) which will be emitted when
 * the corresponding DBus method was called.
 *
 * For reference, the KDE specifications can be found at
 * http://www.notmart.org/misc/statusnotifieritem/index.html
 */

enum
{
    PROP_0,

    PROP_ID,
    PROP_TITLE,
    PROP_CATEGORY,
    PROP_STATUS,
    PROP_MAIN_ICON_NAME,
    PROP_MAIN_ICON_PIXBUF,
    PROP_OVERLAY_ICON_NAME,
    PROP_OVERLAY_ICON_PIXBUF,
    PROP_ATTENTION_ICON_NAME,
    PROP_ATTENTION_ICON_PIXBUF,
    PROP_ATTENTION_MOVIE_NAME,
    PROP_TOOLTIP_ICON_NAME,
    PROP_TOOLTIP_ICON_PIXBUF,
    PROP_TOOLTIP_TITLE,
    PROP_TOOLTIP_BODY,
    PROP_WINDOW_ID,

    PROP_STATE,

    NB_PROPS
};

static guint prop_name_from_icon[_NB_STATUS_NOTIFIER_ICONS] = {
    PROP_MAIN_ICON_NAME,
    PROP_ATTENTION_ICON_NAME,
    PROP_OVERLAY_ICON_NAME,
    PROP_TOOLTIP_ICON_NAME
};
static guint prop_pixbuf_from_icon[_NB_STATUS_NOTIFIER_ICONS] = {
    PROP_MAIN_ICON_PIXBUF,
    PROP_ATTENTION_ICON_PIXBUF,
    PROP_OVERLAY_ICON_PIXBUF,
    PROP_TOOLTIP_ICON_PIXBUF
};

enum
{
    SIGNAL_REGISTRATION_FAILED,
    SIGNAL_CONTEXT_MENU,
    SIGNAL_ACTIVATE,
    SIGNAL_SECONDARY_ACTIVATE,
    SIGNAL_SCROLL,
    NB_SIGNALS
};

struct _StatusNotifierPrivate
{
    gchar *id;
    StatusNotifierCategory category;
    gchar *title;
    StatusNotifierStatus status;
    struct {
        gboolean has_pixbuf;
        union {
            gchar *icon_name;
            GdkPixbuf *pixbuf;
        };
    } icon[_NB_STATUS_NOTIFIER_ICONS];
    gchar *attention_movie_name;
    gchar *tooltip_title;
    gchar *tooltip_body;
    guint32 window_id;

    guint tooltip_freeze;

    StatusNotifierState state;
    guint dbus_watch_id;
    guint dbus_sid;
    guint dbus_owner_id;
    guint dbus_reg_id;
    GDBusProxy *dbus_proxy;
    GDBusConnection *dbus_conn;
    GError *dbus_err;
};

static guint uniq_id = 0;

static GParamSpec *status_notifier_props[NB_PROPS] = { NULL, };
static guint status_notifier_signals[NB_SIGNALS] = { 0, };

#define notify(sn,prop) \
    g_object_notify_by_pspec ((GObject *) sn, status_notifier_props[prop])

static void     status_notifier_set_property    (GObject            *object,
                                                 guint               prop_id,
                                                 const GValue       *value,
                                                 GParamSpec         *pspec);
static void     status_notifier_get_property    (GObject            *object,
                                                 guint               prop_id,
                                                 GValue             *value,
                                                 GParamSpec         *pspec);
static void     status_notifier_finalize        (GObject            *object);

G_DEFINE_TYPE (StatusNotifier, status_notifier, G_TYPE_OBJECT)

static void
status_notifier_class_init (StatusNotifierClass *klass)
{
    GObjectClass *o_class;

    o_class = G_OBJECT_CLASS (klass);
    o_class->set_property   = status_notifier_set_property;
    o_class->get_property   = status_notifier_get_property;
    o_class->finalize       = status_notifier_finalize;

    /**
     * StatusNotifier:id:
     *
     * It's a name that should be unique for this application and consistent
     * between sessions, such as the application name itself.
     */
    status_notifier_props[PROP_ID] =
        g_param_spec_string ("id", "id", "Unique application identifier",
                NULL,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    /**
     * StatusNotifier:title:
     *
     * It's a name that describes the application, it can be more descriptive
     * than #StatusNotifier:id.
     */
    status_notifier_props[PROP_TITLE] =
        g_param_spec_string ("title", "title", "Descriptive name for the item",
                NULL,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:category:
     *
     * Describes the category of this item.
     */
    status_notifier_props[PROP_CATEGORY] =
        g_param_spec_enum ("category", "category", "Category of the item",
                TYPE_STATUS_NOTIFIER_CATEGORY,
                STATUS_NOTIFIER_CATEGORY_APPLICATION_STATUS,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    /**
     * StatusNotifier:status:
     *
     * Describes the status of this item or of the associated application.
     */
    status_notifier_props[PROP_STATUS] =
        g_param_spec_enum ("status", "status", "Status of the item",
                TYPE_STATUS_NOTIFIER_STATUS,
                STATUS_NOTIFIER_STATUS_PASSIVE,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:main-icon-name:
     *
     * The item can carry an icon that can be used by the visualization to
     * identify the item.
     *
     * An icon can either be identified by its Freedesktop-compliant icon name,
     * set by this property, or by the icon data itself, set by the property
     * #StatusNotifier:main-icon-pixbuf.
     *
     * It is currently not possible to set both, as setting one will unset the
     * other.
     */
    status_notifier_props[PROP_MAIN_ICON_NAME] =
        g_param_spec_string ("main-icon-name", "main-icon-name",
                "Icon name for the main icon",
                NULL,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:main-icon-pixbuf:
     *
     * The item can carry an icon that can be used by the visualization to
     * identify the item.
     *
     * An icon can either be identified by its Freedesktop-compliant icon name,
     * set by property #StatusNotifier:main-icon-name, or by the icon data
     * itself, set by this property.
     *
     * It is currently not possible to set both, as setting one will unset the
     * other.
     */
    status_notifier_props[PROP_MAIN_ICON_PIXBUF] =
        g_param_spec_object ("main-icon-pixbuf", "main-icon-pixbuf",
                "Pixbuf for the main icon",
                GDK_TYPE_PIXBUF,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:overlay-icon-name:
     *
     * This can be used by the visualization to indicate extra state
     * information, for instance as an overlay for the main icon.
     *
     * An icon can either be identified by its Freedesktop-compliant icon name,
     * set by this property, or by the icon data itself, set by property
     * #StatusNotifier:overlay-icon-pixbuf.
     *
     * It is currently not possible to set both, as setting one will unset the
     * other.
     */
    status_notifier_props[PROP_OVERLAY_ICON_NAME] =
        g_param_spec_string ("overlay-icon-name", "overlay-icon-name",
                "Icon name for the overlay icon",
                NULL,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:overlay-icon-pixbuf:
     *
     * This can be used by the visualization to indicate extra state
     * information, for instance as an overlay for the main icon.
     *
     * An icon can either be identified by its Freedesktop-compliant icon name,
     * set by property #StatusNotifier:overlay-icon-name, or by the icon data
     * itself, set by this property.
     *
     * It is currently not possible to set both, as setting one will unset the
     * other.
     */
    status_notifier_props[PROP_OVERLAY_ICON_PIXBUF] =
        g_param_spec_object ("overlay-icon-pixbuf", "overlay-icon-pixbuf",
                "Pixbuf for the overlay icon",
                GDK_TYPE_PIXBUF,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:attention-icon-name:
     *
     * This can be used by the visualization to indicate that the item is in
     * %STATUS_NOTIFIER_STATUS_NEEDS_ATTENTION status.
     *
     * An icon can either be identified by its Freedesktop-compliant icon name,
     * set by this property, or by the icon data itself, set by property
     * #StatusNotifier:attention-icon-pixbuf.
     *
     * It is currently not possible to set both, as setting one will unset the
     * other.
     */
    status_notifier_props[PROP_ATTENTION_ICON_NAME] =
        g_param_spec_string ("attention-icon-name", "attention-icon-name",
                "Icon name for the attention icon",
                NULL,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:attention-icon-pixbuf:
     *
     * This can be used by the visualization to indicate that the item is in
     * %STATUS_NOTIFIER_STATUS_NEEDS_ATTENTION status.
     *
     * An icon can either be identified by its Freedesktop-compliant icon name,
     * set by property #StatusNotifier:attention-icon-name, or by the icon data
     * itself, set by this property.
     *
     * It is currently not possible to set both, as setting one will unset the
     * other.
     */
    status_notifier_props[PROP_ATTENTION_ICON_PIXBUF] =
        g_param_spec_object ("attention-icon-pixbuf", "attention-icon-pixbuf",
                "Pixbuf for the attention icon",
                GDK_TYPE_PIXBUF,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:attention-movie-name:
     *
     * In addition to the icon, the item can also specify an animation
     * associated to the #STATUS_NOTIFIER_STATUS_NEEDS_ATTENTION status.
     *
     * This should be either a Freedesktop-compliant icon name or a full path.
     * The visualization can chose between the movie or icon (or using neither
     * of those) at its discretion.
     */
    status_notifier_props[PROP_ATTENTION_MOVIE_NAME] =
        g_param_spec_string ("attention-movie-name", "attention-movie-name",
                "Animation name/full path when the item is in needs-attention status",
                NULL,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:tooltip-icon-name:
     *
     * A tooltip can be defined on the item; It can be used by the visualization
     * to show as a tooltip (or by any other mean it considers appropriate).
     *
     * The tooltip is composed of a title, a body, and an icon. Note that
     * changing any of these will trigger a DBus signal NewToolTip (for hosts to
     * refresh DBus property ToolTip), see status_notifier_freeze_tooltip() for
     * changing more than one and only emitting one DBus signal at the end.
     *
     * The icon can either be identified by its Freedesktop-compliant icon name,
     * set by this property, or by the icon data itself, set by property
     * #StatusNotifier:tooltip-icon-pixbuf.
     *
     * It is currently not possible to set both, as setting one will unset the
     * other.
     */
    status_notifier_props[PROP_TOOLTIP_ICON_NAME] =
        g_param_spec_string ("tooltip-icon-name", "tooltip-icon-name",
                "Icon name for the tooltip icon",
                NULL,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:tooltip-icon-pixbuf:
     *
     * A tooltip can be defined on the item; It can be used by the visualization
     * to show as a tooltip (or by any other mean it considers appropriate).
     *
     * The tooltip is composed of a title, a body, and an icon. Note that
     * changing any of these will trigger a DBus signal NewToolTip (for hosts to
     * refresh DBus property ToolTip), see status_notifier_freeze_tooltip() for
     * changing more than one and only emitting one DBus signal at the end.
     *
     * The icon can either be identified by its Freedesktop-compliant icon name,
     * set by property #StatusNotifier:tooltip-icon-name, or by the icon data
     * itself, set by this property.
     *
     * It is currently not possible to set both, as setting one will unset the
     * other.
     */
    status_notifier_props[PROP_TOOLTIP_ICON_PIXBUF] =
        g_param_spec_object ("tooltip-icon-pixbuf", "tooltip-icon-pixbuf",
                "Pixbuf for the tooltip icon",
                GDK_TYPE_PIXBUF,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:tooltip-title:
     *
     * A tooltip can be defined on the item; It can be used by the visualization
     * to show as a tooltip (or by any other mean it considers appropriate).
     *
     * The tooltip is composed of a title, a body, and an icon. Note that
     * changing any of these will trigger a DBus signal NewToolTip (for hosts to
     * refresh DBus property ToolTip), see status_notifier_freeze_tooltip() for
     * changing more than one and only emitting one DBus signal at the end.
     */
    status_notifier_props[PROP_TOOLTIP_TITLE] =
        g_param_spec_string ("tooltip-title", "tooltip-title",
                "Title of the tooltip",
                NULL,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:tooltip-body:
     *
     * A tooltip can be defined on the item; It can be used by the visualization
     * to show as a tooltip (or by any other mean it considers appropriate).
     *
     * The tooltip is composed of a title, a body, and an icon. Note that
     * changing any of these will trigger a DBus signal NewToolTip (for hosts to
     * refresh DBus property ToolTip), see status_notifier_freeze_tooltip() for
     * changing more than one and only emitting one DBus signal at the end.
     *
     * This body can contain some markup, which consists of a small subset of
     * XHTML. See http://www.notmart.org/misc/statusnotifieritem/markup.html for
     * more.
     */
    status_notifier_props[PROP_TOOLTIP_BODY] =
        g_param_spec_string ("tooltip-body", "tooltip-body",
                "Body of the tooltip",
                NULL,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:window-id:
     *
     * It's the windowing-system dependent identifier for a window, the
     * application can chose one of its windows to be available trough this
     * property or just set 0 if it's not interested.
     */
    status_notifier_props[PROP_WINDOW_ID] =
        g_param_spec_uint ("window-id", "window-id", "Window ID",
                0, G_MAXUINT32,
                0,
                G_PARAM_READWRITE);

    /**
     * StatusNotifier:state:
     *
     * The state of the item, regarding its DBus registration on the
     * StatusNotifierWatcher. After you've created the item, you need to call
     * status_notifier_register() to have it registered via DBus on the watcher.
     *
     * See status_notifier_register() for more.
     */
    status_notifier_props[PROP_STATE] =
        g_param_spec_enum ("state", "state",
                "DBus registration state of the item",
                TYPE_STATUS_NOTIFIER_STATE,
                STATUS_NOTIFIER_STATE_NOT_REGISTERED,
                G_PARAM_READABLE);

    g_object_class_install_properties (o_class, NB_PROPS, status_notifier_props);

    /**
     * StatusNotifier::registration-failed:
     * @sn: The #StatusNotifier
     * @error: A #GError with the reason of failure
     *
     * This signal is emited after a call to status_notifier_register() when
     * registering the item eventually failed (e.g. if there wasn't (yet) any
     * StatusNotifierHost registered.)
     *
     * When this happens, you should fallback to using the systray. You should
     * also check #StatusNotifier:state as it might still be
     * %STATUS_NOTIFIER_STATE_REGISTERING if the registration remains eventually
     * possible (e.g. waiting for a StatusNotifierHost to register)
     *
     * See status_notifier_register() for more.
     */
    status_notifier_signals[SIGNAL_REGISTRATION_FAILED] = g_signal_new (
            "registration-failed",
            TYPE_STATUS_NOTIFIER,
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (StatusNotifierClass, registration_failed),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__BOXED,
            G_TYPE_NONE,
            1,
            G_TYPE_ERROR);

    /**
     * StatusNotifier::context-menu:
     * @sn: The #StatusNotifier
     * @x: screen coordinates X
     * @y: screen coordinates Y
     *
     * Emitted when the ContextMenu method was called on the item. Item should
     * then show a context menu, this is typically a consequence of user input,
     * such as mouse right click over the graphical representation of the item.
     *
     * @x and @y are to be considered an hint to the item about where to show
     * the context menu.
     */
    status_notifier_signals[SIGNAL_CONTEXT_MENU] = g_signal_new (
            "context-menu",
            TYPE_STATUS_NOTIFIER,
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (StatusNotifierClass, context_menu),
            g_signal_accumulator_true_handled,
            NULL,
            g_cclosure_user_marshal_BOOLEAN__INT_INT,
            G_TYPE_BOOLEAN,
            2,
            G_TYPE_INT,
            G_TYPE_INT);

    /**
     * StatusNotifier::activate:
     * @sn: The #StatusNotifier
     * @x: screen coordinates X
     * @y: screen coordinates Y
     *
     * Emitted when the Activate method was called on the item. Activation of
     * the item was requested, this is typically a consequence of user input,
     * such as mouse left click over the graphical representation of the item.
     *
     * @x and @y are to be considered an hint to the item about where to show
     * the context menu.
     */
    status_notifier_signals[SIGNAL_ACTIVATE] = g_signal_new (
            "activate",
            TYPE_STATUS_NOTIFIER,
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (StatusNotifierClass, activate),
            g_signal_accumulator_true_handled,
            NULL,
            g_cclosure_user_marshal_BOOLEAN__INT_INT,
            G_TYPE_BOOLEAN,
            2,
            G_TYPE_INT,
            G_TYPE_INT);

    /**
     * StatusNotifier::secondary-activate:
     * @sn: The #StatusNotifier
     * @x: screen coordinates X
     * @y: screen coordinates Y
     *
     * Emitted when the SecondaryActivate method was called on the item.
     * Secondary and less important form of activation (compared to
     * #StatusNotifier::activate) of the item was requested. This is typically a
     * consequence of user input, such as mouse middle click over the graphical
     * representation of the item.
     *
     * @x and @y are to be considered an hint to the item about where to show
     * the context menu.
     */
    status_notifier_signals[SIGNAL_SECONDARY_ACTIVATE] = g_signal_new (
            "secondary-activate",
            TYPE_STATUS_NOTIFIER,
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (StatusNotifierClass, secondary_activate),
            g_signal_accumulator_true_handled,
            NULL,
            g_cclosure_user_marshal_BOOLEAN__INT_INT,
            G_TYPE_BOOLEAN,
            2,
            G_TYPE_INT,
            G_TYPE_INT);

    /**
     * StatusNotifier::scroll:
     * @sn: The #StatusNotifier
     * @delta: the amount of scroll
     * @orientation: orientation of the scroll request
     *
     * Emitted when the Scroll method was called on the item. The user asked for
     * a scroll action. This is caused from input such as mouse wheel over the
     * graphical representation of the item.
     */
    status_notifier_signals[SIGNAL_SCROLL] = g_signal_new (
            "scroll",
            TYPE_STATUS_NOTIFIER,
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (StatusNotifierClass, scroll),
            g_signal_accumulator_true_handled,
            NULL,
            g_cclosure_user_marshal_BOOLEAN__INT_INT,
            G_TYPE_BOOLEAN,
            2,
            G_TYPE_INT,
            TYPE_STATUS_NOTIFIER_SCROLL_ORIENTATION);

    g_type_class_add_private (klass, sizeof (StatusNotifierPrivate));
}

static void
status_notifier_init (StatusNotifier *sn)
{
    sn->priv = G_TYPE_INSTANCE_GET_PRIVATE (sn,
            TYPE_STATUS_NOTIFIER, StatusNotifierPrivate);
}

static void
status_notifier_set_property (GObject            *object,
                              guint               prop_id,
                              const GValue       *value,
                              GParamSpec         *pspec)
{
    StatusNotifier *sn = (StatusNotifier *) object;
    StatusNotifierPrivate *priv = sn->priv;

    switch (prop_id)
    {
        case PROP_ID:   /* G_PARAM_CONSTRUCT_ONLY */
            priv->id = g_value_dup_string (value);
            break;
        case PROP_TITLE:
            status_notifier_set_title (sn, g_value_get_string (value));
            break;
        case PROP_CATEGORY: /* G_PARAM_CONSTRUCT_ONLY */
            priv->category = g_value_get_enum (value);
            break;
        case PROP_STATUS:
            status_notifier_set_status (sn, g_value_get_enum (value));
            break;
        case PROP_MAIN_ICON_NAME:
            status_notifier_set_from_icon_name (sn, STATUS_NOTIFIER_ICON,
                    g_value_get_string (value));
            break;
        case PROP_MAIN_ICON_PIXBUF:
            status_notifier_set_from_pixbuf (sn, STATUS_NOTIFIER_ICON,
                    g_value_get_object (value));
            break;
        case PROP_OVERLAY_ICON_NAME:
            status_notifier_set_from_icon_name (sn, STATUS_NOTIFIER_OVERLAY_ICON,
                    g_value_get_string (value));
            break;
        case PROP_OVERLAY_ICON_PIXBUF:
            status_notifier_set_from_pixbuf (sn, STATUS_NOTIFIER_OVERLAY_ICON,
                    g_value_get_object (value));
            break;
        case PROP_ATTENTION_ICON_NAME:
            status_notifier_set_from_icon_name (sn, STATUS_NOTIFIER_ATTENTION_ICON,
                    g_value_get_string (value));
            break;
        case PROP_ATTENTION_ICON_PIXBUF:
            status_notifier_set_from_pixbuf (sn, STATUS_NOTIFIER_ATTENTION_ICON,
                    g_value_get_object (value));
            break;
        case PROP_ATTENTION_MOVIE_NAME:
            status_notifier_set_attention_movie_name (sn, g_value_get_string (value));
            break;
        case PROP_TOOLTIP_ICON_NAME:
            status_notifier_set_from_icon_name (sn, STATUS_NOTIFIER_TOOLTIP_ICON,
                    g_value_get_string (value));
            break;
        case PROP_TOOLTIP_ICON_PIXBUF:
            status_notifier_set_from_pixbuf (sn, STATUS_NOTIFIER_TOOLTIP_ICON,
                    g_value_get_object (value));
            break;
        case PROP_TOOLTIP_TITLE:
            status_notifier_set_tooltip_title (sn, g_value_get_string (value));
            break;
        case PROP_TOOLTIP_BODY:
            status_notifier_set_tooltip_body (sn, g_value_get_string (value));
            break;
        case PROP_WINDOW_ID:
            status_notifier_set_window_id (sn, g_value_get_uint (value));
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
status_notifier_get_property (GObject            *object,
                              guint               prop_id,
                              GValue             *value,
                              GParamSpec         *pspec)
{
    StatusNotifier *sn = (StatusNotifier *) object;
    StatusNotifierPrivate *priv = sn->priv;

    switch (prop_id)
    {
        case PROP_ID:
            g_value_set_string (value, priv->id);
            break;
        case PROP_TITLE:
            g_value_set_string (value, priv->title);
            break;
        case PROP_CATEGORY:
            g_value_set_enum (value, priv->category);
            break;
        case PROP_STATUS:
            g_value_set_enum (value, priv->status);
            break;
        case PROP_MAIN_ICON_NAME:
            g_value_take_string (value, status_notifier_get_icon_name (sn,
                        STATUS_NOTIFIER_ICON));
            break;
        case PROP_MAIN_ICON_PIXBUF:
            g_value_take_object (value, status_notifier_get_pixbuf (sn,
                        STATUS_NOTIFIER_ICON));
            break;
        case PROP_OVERLAY_ICON_NAME:
            g_value_take_string (value, status_notifier_get_icon_name (sn,
                        STATUS_NOTIFIER_OVERLAY_ICON));
            break;
        case PROP_OVERLAY_ICON_PIXBUF:
            g_value_take_object (value, status_notifier_get_pixbuf (sn,
                        STATUS_NOTIFIER_OVERLAY_ICON));
            break;
        case PROP_ATTENTION_ICON_NAME:
            g_value_take_string (value, status_notifier_get_icon_name (sn,
                        STATUS_NOTIFIER_ATTENTION_ICON));
            break;
        case PROP_ATTENTION_ICON_PIXBUF:
            g_value_take_object (value, status_notifier_get_pixbuf (sn,
                        STATUS_NOTIFIER_ATTENTION_ICON));
            break;
        case PROP_ATTENTION_MOVIE_NAME:
            g_value_set_string (value, priv->attention_movie_name);
            break;
        case PROP_TOOLTIP_ICON_NAME:
            g_value_take_string (value, status_notifier_get_icon_name (sn,
                        STATUS_NOTIFIER_TOOLTIP_ICON));
            break;
        case PROP_TOOLTIP_ICON_PIXBUF:
            g_value_take_object (value, status_notifier_get_pixbuf (sn,
                        STATUS_NOTIFIER_TOOLTIP_ICON));
            break;
        case PROP_TOOLTIP_TITLE:
            g_value_set_string (value, priv->tooltip_title);
            break;
        case PROP_TOOLTIP_BODY:
            g_value_set_string (value, priv->tooltip_body);
            break;
        case PROP_WINDOW_ID:
            g_value_set_uint (value, priv->window_id);
        case PROP_STATE:
            g_value_set_enum (value, priv->state);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
free_icon (StatusNotifier *sn, StatusNotifierIcon icon)
{
    StatusNotifierPrivate *priv = sn->priv;

    if (priv->icon[icon].has_pixbuf)
        g_object_unref (priv->icon[icon].pixbuf);
    else
        g_free (priv->icon[icon].icon_name);
    priv->icon[icon].has_pixbuf = FALSE;
    priv->icon[icon].icon_name = NULL;
}

static void
dbus_free (StatusNotifier *sn)
{
    StatusNotifierPrivate *priv = sn->priv;

    if (priv->dbus_watch_id > 0)
    {
        g_bus_unwatch_name (priv->dbus_watch_id);
        priv->dbus_watch_id = 0;
    }
    if (priv->dbus_sid > 0)
    {
        g_signal_handler_disconnect (priv->dbus_proxy, priv->dbus_sid);
        priv->dbus_sid = 0;
    }
    if (G_LIKELY (priv->dbus_owner_id > 0))
    {
        g_bus_unown_name (priv->dbus_owner_id);
        priv->dbus_owner_id = 0;
    }
    if (priv->dbus_proxy)
    {
        g_object_unref (priv->dbus_proxy);
        priv->dbus_proxy = NULL;
    }
    if (priv->dbus_reg_id > 0)
    {
        g_dbus_connection_unregister_object (priv->dbus_conn, priv->dbus_reg_id);
        priv->dbus_reg_id = 0;
    }
    if (priv->dbus_conn)
    {
        g_object_unref (priv->dbus_conn);
        priv->dbus_conn = NULL;
    }
}

static void
status_notifier_finalize (GObject *object)
{
    StatusNotifier *sn = (StatusNotifier *) object;
    StatusNotifierPrivate *priv = sn->priv;
    guint i;

    g_free (priv->id);
    g_free (priv->title);
    for (i = 0; i < _NB_STATUS_NOTIFIER_ICONS; ++i)
        free_icon (sn, i);
    g_free (priv->attention_movie_name);
    g_free (priv->tooltip_title);
    g_free (priv->tooltip_body);

    dbus_free (sn);

    G_OBJECT_CLASS (status_notifier_parent_class)->finalize (object);
}

static void
dbus_notify (StatusNotifier *sn, guint prop)
{
    StatusNotifierPrivate *priv = sn->priv;
    const gchar *signal;

    if (priv->state !=  STATUS_NOTIFIER_STATE_REGISTERED)
        return;

    switch (prop)
    {
        case PROP_STATUS:
            {
                const gchar *s_status[] = {
                    "Passive",
                    "Active",
                    "NeedsAttention"
                };
                signal = "NewStatus";
                g_dbus_connection_emit_signal (priv->dbus_conn,
                        NULL,
                        ITEM_OBJECT,
                        ITEM_INTERFACE,
                        signal,
                        g_variant_new ("(s)", s_status[priv->status]),
                        NULL);
                return;
            }
        case PROP_TITLE:
            signal = "NewTitle";
            break;
        case PROP_MAIN_ICON_NAME:
        case PROP_MAIN_ICON_PIXBUF:
            signal = "NewIcon";
            break;
        case PROP_ATTENTION_ICON_NAME:
        case PROP_ATTENTION_ICON_PIXBUF:
            signal = "NewAttentionIcon";
            break;
        case PROP_OVERLAY_ICON_NAME:
        case PROP_OVERLAY_ICON_PIXBUF:
            signal = "NewOverlayIcon";
            break;
        case PROP_TOOLTIP_TITLE:
        case PROP_TOOLTIP_BODY:
        case PROP_TOOLTIP_ICON_NAME:
        case PROP_TOOLTIP_ICON_PIXBUF:
            signal = "NewToolTip";
            break;
        default:
            g_return_if_reached ();
    }

    g_dbus_connection_emit_signal (priv->dbus_conn,
            NULL,
            ITEM_OBJECT,
            ITEM_INTERFACE,
            signal,
            NULL,
            NULL);
}

/**
 * status_notifier_new_from_pixbuf:
 * @id: The application id
 * @category: The category for the item
 * @pixbuf: The icon to use as main icon
 *
 * Creates a new item
 *
 * Returns: (transfer full): A new #StatusNotifier
 */
StatusNotifier *
status_notifier_new_from_pixbuf (const gchar             *id,
                                 StatusNotifierCategory   category,
                                 GdkPixbuf               *pixbuf)
{
    return (StatusNotifier *) g_object_new (TYPE_STATUS_NOTIFIER,
            "id",               id,
            "category",         category,
            "main-icon-pixbuf", pixbuf,
            NULL);
}

/**
 * status_notifier_new_from_icon_name:
 * @id: The application id
 * @category: The category for the item
 * @icon_name: The name of the icon to use as main icon
 *
 * Creates a new item
 *
 * Returns: (transfer full): A new #StatusNotifier
 */
StatusNotifier *
status_notifier_new_from_icon_name (const gchar             *id,
                                    StatusNotifierCategory   category,
                                    const gchar             *icon_name)
{
    return (StatusNotifier *) g_object_new (TYPE_STATUS_NOTIFIER,
            "id",               id,
            "category",         category,
            "main-icon-name",   icon_name,
            NULL);
}

/**
 * status_notifier_get_id:
 * @sn: A #StatusNotifier
 *
 * Returns the application id of @sn
 *
 * Returns: The application id of @sn. The string is owned by @sn, you should
 * not free it
 */
const gchar *
status_notifier_get_id (StatusNotifier          *sn)
{
    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), NULL);
    return sn->priv->id;
}

/**
 * status_notifier_get_category:
 * @sn: A #StatusNotifier
 *
 * Returns the category of @sn
 *
 * Returns: The category of @sn
 */
StatusNotifierCategory
status_notifier_get_category (StatusNotifier          *sn)
{
    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), -1);
    return sn->priv->category;
}

/**
 * status_notifier_set_from_pixbuf:
 * @sn: A #StatusNotifier
 * @icon: Which icon to set
 * @pixbuf: A #GdkPixbuf to use for @icon
 *
 * Sets the icon @icon to @pixbuf.
 *
 * An icon can either be identified by its Freedesktop-compliant icon name,
 * or by the icon data itself (via #GdkPixbuf).
 *
 * It is currently not possible to set both, as setting one will unset the
 * other.
 */
void
status_notifier_set_from_pixbuf (StatusNotifier          *sn,
                                 StatusNotifierIcon       icon,
                                 GdkPixbuf               *pixbuf)
{
    StatusNotifierPrivate *priv;

    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    priv = sn->priv;

    free_icon (sn, icon);
    priv->icon[icon].has_pixbuf = TRUE;
    priv->icon[icon].pixbuf = g_object_ref (pixbuf);

    notify (sn, prop_name_from_icon[icon]);
    if (icon != STATUS_NOTIFIER_TOOLTIP_ICON || priv->tooltip_freeze == 0)
        dbus_notify (sn, prop_name_from_icon[icon]);
}

/**
 * status_notifier_set_from_icon_name:
 * @sn: A #StatusNotifier
 * @icon: Which icon to set
 * @icon_name: Name of an icon to use for @icon
 *
 * Sets the icon @icon to be @icon_name.
 *
 * An icon can either be identified by its Freedesktop-compliant icon name,
 * or by the icon data itself (via #GdkPixbuf).
 *
 * It is currently not possible to set both, as setting one will unset the
 * other.
 */
void
status_notifier_set_from_icon_name (StatusNotifier          *sn,
                                    StatusNotifierIcon       icon,
                                    const gchar             *icon_name)
{
    StatusNotifierPrivate *priv;

    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    priv = sn->priv;

    free_icon (sn, icon);
    priv->icon[icon].icon_name = g_strdup (icon_name);

    notify (sn, prop_pixbuf_from_icon[icon]);
    if (icon != STATUS_NOTIFIER_TOOLTIP_ICON || priv->tooltip_freeze == 0)
        dbus_notify (sn, prop_name_from_icon[icon]);
}

/**
 * status_notifier_has_pixbuf:
 * @sn: A #StatusNotifier
 * @icon: Which icon
 *
 * Returns whether icon @icon currently has a #GdkPixbuf set or not. If so, the
 * icon data will be sent via DBus, else the icon name (if any) will be used.
 *
 * Returns: %TRUE is a #GdkPixbuf is set for @icon, else %FALSE
 */
gboolean
status_notifier_has_pixbuf (StatusNotifier          *sn,
                            StatusNotifierIcon       icon)
{
    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), FALSE);
    return sn->priv->icon[icon].has_pixbuf;
}

/**
 * status_notifier_get_pixbuf:
 * @sn: A #StatusNotifier
 * @icon: The icon to get
 *
 * Returns the #GdkPixbuf set for @icon, if there's one. Not that it will return
 * %NULL if an icon name is set.
 *
 * Returns: (transfer full): The #GdkPixbuf set for @icon, or %NULL
 */
GdkPixbuf *
status_notifier_get_pixbuf (StatusNotifier          *sn,
                            StatusNotifierIcon       icon)
{
    StatusNotifierPrivate *priv;

    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), NULL);
    priv = sn->priv;

    if (!priv->icon[icon].has_pixbuf)
        return NULL;

    return g_object_ref (priv->icon[icon].pixbuf);
}

/**
 * status_notifier_get_icon_name:
 * @sn: A #StatusNotifier
 * @icon: The icon to get
 *
 * Returns the icon name set for @icon, if there's one. Not that it will return
 * %NULL if a #GdkPixbuf is set.
 *
 * Returns: (transfer full): A newly allocated string of the icon name set for
 * @icon, free using g_free()
 */
gchar *
status_notifier_get_icon_name (StatusNotifier          *sn,
                               StatusNotifierIcon       icon)
{
    StatusNotifierPrivate *priv;

    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), NULL);
    priv = sn->priv;

    if (priv->icon[icon].has_pixbuf)
        return NULL;

    return g_strdup (priv->icon[icon].icon_name);
}

/**
 * status_notifier_set_attention_movie_name:
 * @sn: A #StatusNotifier
 * @movie_name: The name of the movie
 *
 * In addition to the icon, the item can also specify an animation associated to
 * the #STATUS_NOTIFIER_STATUS_NEEDS_ATTENTION status.
 *
 * This should be either a Freedesktop-compliant icon name or a full path.  The
 * visualization can chose between the movie or icon (or using neither of those)
 * at its discretion.
 */
void
status_notifier_set_attention_movie_name (StatusNotifier          *sn,
                                          const gchar             *movie_name)
{
    StatusNotifierPrivate *priv;

    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    priv = sn->priv;

    g_free (priv->attention_movie_name);
    priv->attention_movie_name = g_strdup (movie_name);

    notify (sn, PROP_ATTENTION_MOVIE_NAME);
}

/**
 * status_notifier_get_attention_movie_name:
 * @sn: A #StatusNotifier
 *
 * Returns the movie name set for animation associated with the
 * #STATUS_NOTIFIER_STATUS_NEEDS_ATTENTION status
 *
 * Returns: A newly allocated string with the movie name, free using g_free()
 * when done
 */
gchar *
status_notifier_get_attention_movie_name (StatusNotifier          *sn)
{
    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), NULL);
    return g_strdup (sn->priv->attention_movie_name);
}

/**
 * status_notifier_set_title:
 * @sn: A #StatusNotifier
 * @title: The title
 *
 * Sets the title of the item (might be used by visualization e.g. in menu of
 * hidden items when #STATUS_NOTIFIER_STATUS_PASSIVE)
 */
void
status_notifier_set_title (StatusNotifier          *sn,
                           const gchar             *title)
{
    StatusNotifierPrivate *priv;

    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    priv = sn->priv;

    g_free (priv->title);
    priv->title = g_strdup (title);

    notify (sn, PROP_TITLE);
    dbus_notify (sn, PROP_TITLE);
}

/**
 * status_notifier_get_title:
 * @sn: A #StatusNotifier
 *
 * Returns the title of the item
 *
 * Returns: A newly allocated string, free with g_free() when done
 */
gchar *
status_notifier_get_title (StatusNotifier          *sn)
{
    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), NULL);
    return g_strdup (sn->priv->title);
}

/**
 * status_notifier_set_status:
 * @sn: A #StatusNotifier
 * @status: The new status
 *
 * Sets the item status to @status, describing the status of this item or of the
 * associated application.
 */
void
status_notifier_set_status (StatusNotifier          *sn,
                            StatusNotifierStatus     status)
{
    StatusNotifierPrivate *priv;

    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    priv = sn->priv;

    priv->status = status;

    notify (sn, PROP_STATUS);
    dbus_notify (sn, PROP_STATUS);
}

/**
 * status_notifier_get_status:
 * @sn: A #StatusNotifier
 *
 * Returns the status of @sn
 *
 * Returns: Current status of @sn
 */
StatusNotifierStatus
status_notifier_get_status (StatusNotifier          *sn)
{
    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), -1);
    return sn->priv->status;
}

/**
 * status_notifier_set_window_id:
 * @sn: A #StatusNotifier
 * @window_id: The window ID
 *
 * Sets the window ID for @sn
 *
 * It's the windowing-system dependent identifier for a window, the application
 * can chose one of its windows to be available trough this property or just set
 * 0 if it's not interested.
 */
void
status_notifier_set_window_id (StatusNotifier          *sn,
                               guint32                  window_id)
{
    StatusNotifierPrivate *priv;

    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    priv = sn->priv;

    priv->window_id = window_id;

    notify (sn, PROP_WINDOW_ID);
}

/**
 * status_notifier_get_window_id:
 * @sn: A #StatusNotifier
 *
 * Returns the windowing-system dependent idnetifier for a window associated
 * with @sn
 *
 * Returns: The window ID associated with @sn
 */
guint32
status_notifier_get_window_id (StatusNotifier          *sn)
{
    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), 0);
    return sn->priv->window_id;
}

/**
 * status_notifier_freeze_tooltip:
 * @sn:A #StatusNotifier
 *
 * Increases the freeze count for tooltip on @sn. If the freeze count is
 * non-zero, the emission of a DBus signal for StatusNotifierHost to refresh the
 * ToolTip property will be blocked until the freeze count drops back to zero
 * (via status_notifier_thaw_tooltip())
 *
 * This is to allow to set the different properties forming the tooltip (title,
 * body and icon) without triggering a refresh afetr each change (as there is a
 * single property ToolTip on the DBus item, with all data).
 *
 * Every call to status_notifier_freeze_tooltip() should later be followed by a
 * call to status_notifier_thaw_tooltip()
 */
void
status_notifier_freeze_tooltip (StatusNotifier          *sn)
{
    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    ++sn->priv->tooltip_freeze;
}

/**
 * status_notifier_thaw_tooltip:
 * @sn: A #StatusNotifier
 *
 * Reverts the effect of a previous call to status_notifier_freeze_tooltip(). If
 * the freeze count drops back to zero, a signal NewToolTip will be emitted on
 * the DBus object for @sn, for StatusNotifierHost to refresh its ToolTip
 * property.
 *
 * It is an error to call this function when the freeze count is zero.
 */
void
status_notifier_thaw_tooltip (StatusNotifier          *sn)
{
    StatusNotifierPrivate *priv;

    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    priv = sn->priv;
    g_return_if_fail (priv->tooltip_freeze > 0);

    if (--priv->tooltip_freeze == 0)
        dbus_notify (sn, PROP_TOOLTIP_TITLE);
}

/**
 * status_notifier_set_tooltip:
 * @sn: A #StatusNotifier
 * @icon_name: The icon name to be used for #STATUS_NOTIFIER_TOOLTIP_ICON
 * @title: The title of the tooltip
 * @body: The body of the tooltip
 *
 * This is an helper function that allows to set icon name, title and body of
 * the tooltip and then emit one DBus signal NewToolTip.
 *
 * It is equivalent to the following code, and similar code can be used e.g. to
 * set the icon from a #GdkPixbuf instead:
 * <programlisting>
 * status_notifier_freeze_tooltip (sn);
 * status_notifier_set_from_icon_name (sn, STATUS_NOTIFIER_TOOLTIP_ICON, icon_name);
 * status_notifier_set_tooltip_title (sn, title);
 * status_notifier_set_tooltip_body (sn, body);
 * status_notifier_thaw_tooltip (sn);
 * </programlisting>
 */
void
status_notifier_set_tooltip (StatusNotifier          *sn,
                             const gchar             *icon_name,
                             const gchar             *title,
                             const gchar             *body)
{
    StatusNotifierPrivate *priv;

    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    priv = sn->priv;

    ++priv->tooltip_freeze;
    status_notifier_set_from_icon_name (sn, STATUS_NOTIFIER_TOOLTIP_ICON, icon_name);
    status_notifier_set_tooltip_title (sn, title);
    status_notifier_set_tooltip_body (sn, body);
    status_notifier_thaw_tooltip (sn);
}

/**
 * status_notifier_set_tooltip_title:
 * @sn: A #StatusNotifier
 * @title: The tooltip title
 *
 * Sets the title of the tooltip
 *
 * The tooltip is composed of a title, a body, and an icon. Note that changing
 * any of these will trigger a DBus signal NewToolTip (for hosts to refresh DBus
 * property ToolTip), see status_notifier_freeze_tooltip() for changing more
 * than one and only emitting one DBus signal at the end.
 */
void
status_notifier_set_tooltip_title (StatusNotifier          *sn,
                                   const gchar             *title)
{
    StatusNotifierPrivate *priv;

    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    priv = sn->priv;

    g_free (priv->tooltip_title);
    priv->tooltip_title = g_strdup (title);

    notify (sn, PROP_TOOLTIP_TITLE);
    if (priv->tooltip_freeze == 0)
        dbus_notify (sn, PROP_TOOLTIP_TITLE);
}

/**
 * status_notifier_get_tooltip_title:
 * @sn: A #StatusNotifier
 *
 * Returns the tooltip title
 *
 * Returns: A newly allocated string of the tooltip title, use g_free() when
 * done
 */
gchar *
status_notifier_get_tooltip_title (StatusNotifier          *sn)
{
    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), NULL);
    return g_strdup (sn->priv->tooltip_title);
}

/**
 * status_notifier_set_tooltip_body:
 * @sn: A #StatusNotifier
 * @body: The tooltip body
 *
 * Sets the body of the tooltip
 *
 * This body can contain some markup, which consists of a small subset of XHTML.
 * See http://www.notmart.org/misc/statusnotifieritem/markup.html for more.
 *
 * The tooltip is composed of a title, a body, and an icon. Note that changing
 * any of these will trigger a DBus signal NewToolTip (for hosts to refresh DBus
 * property ToolTip), see status_notifier_freeze_tooltip() for changing more
 * than one and only emitting one DBus signal at the end.
 */
void
status_notifier_set_tooltip_body (StatusNotifier          *sn,
                                  const gchar             *body)
{
    StatusNotifierPrivate *priv;

    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    priv = sn->priv;

    g_free (priv->tooltip_body);
    priv->tooltip_body = g_strdup (body);

    notify (sn, PROP_TOOLTIP_BODY);
    if (priv->tooltip_freeze == 0)
        dbus_notify (sn, PROP_TOOLTIP_BODY);
}

/**
 * status_notifier_get_tooltip_body:
 * @sn: A #StatusNotifier
 *
 * Returns the tooltip body
 *
 * Returns: A newly allocated string of the tooltip body, use g_free() when done
 */
gchar *
status_notifier_get_tooltip_body (StatusNotifier          *sn)
{
    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), NULL);
    return g_strdup (sn->priv->tooltip_body);
}

static void
method_call (GDBusConnection        *conn,
             const gchar            *sender,
             const gchar            *object,
             const gchar            *interface,
             const gchar            *method,
             GVariant               *params,
             GDBusMethodInvocation  *invocation,
             gpointer                data)
{
    (void)conn;
    (void)sender;
    (void)object;
    (void)interface;
    StatusNotifier *sn = (StatusNotifier *) data;
    guint signal;
    gint x, y;
    gboolean ret;

    if (!g_strcmp0 (method, "ContextMenu"))
        signal = SIGNAL_CONTEXT_MENU;
    else if (!g_strcmp0 (method, "Activate"))
        signal = SIGNAL_ACTIVATE;
    else if (!g_strcmp0 (method, "SecondaryActivate"))
        signal = SIGNAL_SECONDARY_ACTIVATE;
    else if (!g_strcmp0 (method, "Scroll"))
    {
        gint delta, orientation;
        gchar *s_orientation;

        g_variant_get (params, "(is)", &delta, &s_orientation);
        if (!g_strcmp0 (s_orientation, "vertical"))
            orientation = STATUS_NOTIFIER_SCROLL_ORIENTATION_VERTICAL;
        else
            orientation = STATUS_NOTIFIER_SCROLL_ORIENTATION_HORIZONTAL;
        g_free (s_orientation);

        g_signal_emit (sn, status_notifier_signals[SIGNAL_SCROLL], 0,
                delta, orientation, &ret);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }
    else
        /* should never happen */
        g_return_if_reached ();

    g_variant_get (params, "(ii)", &x, &y);
    g_signal_emit (sn, status_notifier_signals[signal], 0, x, y, &ret);
    g_dbus_method_invocation_return_value (invocation, NULL);
}

static GVariant *
get_icon_pixmap (StatusNotifier *sn, StatusNotifierIcon icon)
{
    StatusNotifierPrivate *priv = sn->priv;
    GVariantBuilder *builder;
    cairo_surface_t *surface;
    cairo_t *cr;
    gint width, height, stride;
    guint *data;

    if (G_UNLIKELY (!priv->icon[icon].has_pixbuf))
        return NULL;

    width = gdk_pixbuf_get_width (priv->icon[icon].pixbuf);
    height = gdk_pixbuf_get_height (priv->icon[icon].pixbuf);

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create (surface);
    gdk_cairo_set_source_pixbuf (cr, priv->icon[icon].pixbuf, 0, 0);
    cairo_paint (cr);
    cairo_destroy (cr);

    stride = cairo_image_surface_get_stride (surface);
    cairo_surface_flush (surface);
    data = (guint *) cairo_image_surface_get_data (surface);
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    guint i, max;

    max = (stride * height) / sizeof (guint);
    for (i = 0; i < max; ++i)
        data[i] = GUINT_TO_BE (data[i]);
#endif

    builder = g_variant_builder_new (G_VARIANT_TYPE ("a(iiay)"));
    g_variant_builder_open (builder, G_VARIANT_TYPE ("(iiay)"));
    g_variant_builder_add (builder, "i", width);
    g_variant_builder_add (builder, "i", height);
    g_variant_builder_add_value (builder,
            g_variant_new_from_data (G_VARIANT_TYPE ("ay"),
                data,
                stride * height,
                TRUE,
                (GDestroyNotify) cairo_surface_destroy,
                surface));
    g_variant_builder_close (builder);
    GVariant *pixmap = g_variant_new ("a(iiay)", builder);

    g_variant_builder_unref(builder);   // Allow the builder to be deallocated

    return pixmap;
}

static GVariant *
get_prop (GDBusConnection        *conn,
          const gchar            *sender,
          const gchar            *object,
          const gchar            *interface,
          const gchar            *property,
          GError                **error,
          gpointer                data)
{
    (void)conn;
    (void)sender;
    (void)object;
    (void)interface;
    (void)error;
    StatusNotifier *sn = (StatusNotifier *) data;
    StatusNotifierPrivate *priv = sn->priv;

    if (!g_strcmp0 (property, "Id"))
        return g_variant_new ("s", priv->id);
    else if (!g_strcmp0 (property, "Category"))
    {
        const gchar *s_category[] = {
            "ApplicationStatus",
            "Communications",
            "SystemServices",
            "Hardware"
        };
        return g_variant_new ("s", s_category[priv->category]);
    }
    else if (!g_strcmp0 (property, "Title"))
        return g_variant_new ("s", (priv->title) ? priv->title : "");
    else if (!g_strcmp0 (property, "Status"))
    {
        const gchar *s_status[] = {
            "Passive",
            "Active",
            "NeedsAttention"
        };
        return g_variant_new ("s", s_status[priv->status]);
    }
    else if (!g_strcmp0 (property, "WindowId"))
        return g_variant_new ("i", priv->window_id);
    else if (!g_strcmp0 (property, "IconName"))
        return g_variant_new ("s", (!priv->icon[STATUS_NOTIFIER_ICON].has_pixbuf)
                ? ((priv->icon[STATUS_NOTIFIER_ICON].icon_name)
                    ? priv->icon[STATUS_NOTIFIER_ICON].icon_name : "") : "");
    else if (!g_strcmp0 (property, "IconPixmap"))
        return get_icon_pixmap (sn, STATUS_NOTIFIER_ICON);
    else if (!g_strcmp0 (property, "OverlayIconName"))
        return g_variant_new ("s", (!priv->icon[STATUS_NOTIFIER_OVERLAY_ICON].has_pixbuf)
                ? ((priv->icon[STATUS_NOTIFIER_OVERLAY_ICON].icon_name)
                    ? priv->icon[STATUS_NOTIFIER_OVERLAY_ICON].icon_name : "") : "");
    else if (!g_strcmp0 (property, "OverlayIconPixmap"))
        return get_icon_pixmap (sn, STATUS_NOTIFIER_OVERLAY_ICON);
    else if (!g_strcmp0 (property, "AttentionIconName"))
        return g_variant_new ("s", (!priv->icon[STATUS_NOTIFIER_ATTENTION_ICON].has_pixbuf)
                ? ((priv->icon[STATUS_NOTIFIER_ATTENTION_ICON].icon_name)
                    ? priv->icon[STATUS_NOTIFIER_ATTENTION_ICON].icon_name : "") : "");
    else if (!g_strcmp0 (property, "AttentionIconPixmap"))
        return get_icon_pixmap (sn, STATUS_NOTIFIER_ATTENTION_ICON);
    else if (!g_strcmp0 (property, "AttentionMovieName"))
        return g_variant_new ("s", (priv->attention_movie_name)
                ? priv->attention_movie_name : "");
    else if (!g_strcmp0 (property, "ToolTip"))
    {
        GVariant *variant;
        GVariantBuilder *builder;

        if (!priv->icon[STATUS_NOTIFIER_TOOLTIP_ICON].has_pixbuf)
        {
            variant = g_variant_new ("(sa(iiay)ss)",
                    (priv->icon[STATUS_NOTIFIER_TOOLTIP_ICON].icon_name)
                    ? priv->icon[STATUS_NOTIFIER_TOOLTIP_ICON].icon_name : "",
                    NULL,
                    (priv->tooltip_title) ? priv->tooltip_title : "",
                    (priv->tooltip_body) ? priv->tooltip_body : "");
            return variant;
        }

        builder = get_icon_pixmap (sn, STATUS_NOTIFIER_TOOLTIP_ICON);
        variant = g_variant_new ("(sa(iiay)ss)",
                "",
                builder,
                (priv->tooltip_title) ? priv->tooltip_title : "",
                (priv->tooltip_body) ? priv->tooltip_body : "");
        g_variant_builder_unref (builder);

        return variant;
    }

    g_return_val_if_reached (NULL);
}

static void
dbus_failed (StatusNotifier *sn, GError *error, gboolean fatal)
{
    StatusNotifierPrivate *priv = sn->priv;

    dbus_free (sn);
    if (fatal)
    {
        priv->state = STATUS_NOTIFIER_STATE_FAILED;
        notify (sn, PROP_STATE);
    }
    g_signal_emit (sn, status_notifier_signals[SIGNAL_REGISTRATION_FAILED], 0,
            error);
    g_error_free (error);
}

static void
bus_acquired (GDBusConnection *conn, const gchar *name, gpointer data)
{
    (void)name;
    GError *err = NULL;
    StatusNotifier *sn = (StatusNotifier *) data;
    StatusNotifierPrivate *priv = sn->priv;
    GDBusInterfaceVTable interface_vtable = {
        .method_call = method_call,
        .get_property = get_prop,
        .set_property = NULL
    };
    GDBusNodeInfo *info;

    info = g_dbus_node_info_new_for_xml (item_xml, NULL);
    priv->dbus_reg_id = g_dbus_connection_register_object (conn,
            ITEM_OBJECT,
            info->interfaces[0],
            &interface_vtable,
            sn, NULL,
            &err);
    g_dbus_node_info_unref (info);
    if (priv->dbus_reg_id == 0)
    {
        dbus_failed (sn, err, TRUE);
        return;
    }

    priv->dbus_conn = g_object_ref (conn);
}

static void
register_item_cb (GObject *sce, GAsyncResult *result, gpointer data)
{
    GError *err = NULL;
    StatusNotifier *sn = (StatusNotifier *) data;
    StatusNotifierPrivate *priv = sn->priv;
    GVariant *variant;

    variant = g_dbus_proxy_call_finish ((GDBusProxy *) sce, result, &err);
    if (!variant)
    {
        dbus_failed (sn, err, TRUE);
        return;
    }
    g_variant_unref (variant);

    priv->state = STATUS_NOTIFIER_STATE_REGISTERED;
    notify (sn, PROP_STATE);
}

static void
name_acquired (GDBusConnection *conn, const gchar *name, gpointer data)
{
    (void)conn;
    StatusNotifier *sn = (StatusNotifier *) data;
    StatusNotifierPrivate *priv = sn->priv;

    g_dbus_proxy_call (priv->dbus_proxy,
            "RegisterStatusNotifierItem",
            g_variant_new ("(s)", name),
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            register_item_cb,
            sn);
    g_object_unref (priv->dbus_proxy);
    priv->dbus_proxy = NULL;
}

static void
name_lost (GDBusConnection *conn, const gchar *name, gpointer data)
{
    (void)name;
    GError *err = NULL;
    StatusNotifier *sn = (StatusNotifier *) data;

    if (!conn)
        g_set_error (&err, STATUS_NOTIFIER_ERROR,
                STATUS_NOTIFIER_ERROR_NO_CONNECTION,
                "Failed to establish DBus connection");
    else
        g_set_error (&err, STATUS_NOTIFIER_ERROR,
                STATUS_NOTIFIER_ERROR_NO_NAME,
                "Failed to acquire name for item");
    dbus_failed (sn, err, TRUE);
}

static void
dbus_reg_item (StatusNotifier *sn)
{
    StatusNotifierPrivate *priv = sn->priv;
    gchar buf[64], *b = buf;

    if (G_UNLIKELY (g_snprintf (buf, 64, "org.kde.StatusNotifierItem-%u-%u",
                    getpid (), ++uniq_id) >= 64))
        b = g_strdup_printf ("org.kde.StatusNotifierItem-%u-%u",
            getpid (), uniq_id);
    priv->dbus_owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
            b,
            G_BUS_NAME_OWNER_FLAGS_NONE,
            bus_acquired,
            name_acquired,
            name_lost,
            sn, NULL);
    if (G_UNLIKELY (b != buf))
        g_free (b);
}

static void
watcher_signal (GDBusProxy      *proxy,
                const gchar     *sender,
                const gchar     *signal,
                GVariant        *params,
                StatusNotifier  *sn)
{
    (void)proxy;
    (void)sender;
    (void)params;
    StatusNotifierPrivate *priv = sn->priv;

    if (!g_strcmp0 (signal, "StatusNotifierHostRegistered"))
    {
        g_signal_handler_disconnect (priv->dbus_proxy, priv->dbus_sid);
        priv->dbus_sid = 0;

        dbus_reg_item (sn);
    }
}

static void
proxy_cb (GObject *sce, GAsyncResult *result, gpointer data)
{
    (void)sce;
    GError *err = NULL;
    StatusNotifier *sn = (StatusNotifier *) data;
    StatusNotifierPrivate *priv = sn->priv;
    GVariant *variant;

    priv->dbus_proxy = g_dbus_proxy_new_for_bus_finish (result, &err);
    if (!priv->dbus_proxy)
    {
        dbus_failed (sn, err, TRUE);
        return;
    }

    variant = g_dbus_proxy_get_cached_property (priv->dbus_proxy,
            "IsStatusNotifierHostRegistered");
    if (!variant || !g_variant_get_boolean (variant))
    {
        GDBusProxy *proxy;

        g_set_error (&err, STATUS_NOTIFIER_ERROR,
                STATUS_NOTIFIER_ERROR_NO_HOST,
                "No Host registered on the Watcher");
        if (variant)
            g_variant_unref (variant);

        /* keep the proxy, we'll wait for the signal when a host registers */
        proxy = priv->dbus_proxy;
        /* (so dbus_free() from dbus_failed() doesn't unref) */
        priv->dbus_proxy = NULL;
        dbus_failed (sn, err, FALSE);
        priv->dbus_proxy = proxy;

        priv->dbus_sid = g_signal_connect (priv->dbus_proxy, "g-signal",
                (GCallback) watcher_signal, sn);
        return;
    }
    g_variant_unref (variant);

    dbus_reg_item (sn);
}

static void
watcher_appeared (GDBusConnection   *conn,
                  const gchar       *name,
                  const gchar       *owner,
                  gpointer           data)
{
    (void)conn;
    (void)name;
    (void)owner;
    StatusNotifier *sn = data;
    StatusNotifierPrivate *priv = sn->priv;
    GDBusNodeInfo *info;

    g_bus_unwatch_name (priv->dbus_watch_id);
    priv->dbus_watch_id = 0;

    info = g_dbus_node_info_new_for_xml (watcher_xml, NULL);
    g_dbus_proxy_new_for_bus (G_BUS_TYPE_SESSION,
            G_DBUS_PROXY_FLAGS_NONE,
            info->interfaces[0],
            WATCHER_NAME,
            WATCHER_OBJECT,
            WATCHER_INTERFACE,
            NULL,
            proxy_cb,
            sn);
    g_dbus_node_info_unref (info);
}

static void
watcher_vanished (GDBusConnection   *conn,
                  const gchar       *name,
                  gpointer           data)
{
    (void)conn;
    (void)name;
    GError *err = NULL;
    StatusNotifier *sn = data;
    StatusNotifierPrivate *priv = sn->priv;
    guint id;

    /* keep the watch active, so if a watcher shows up we'll resume the
     * registering automatically */
    id = priv->dbus_watch_id;
    /* (so dbus_free() from dbus_failed() doesn't unwatch) */
    priv->dbus_watch_id = 0;

    g_set_error (&err, STATUS_NOTIFIER_ERROR,
            STATUS_NOTIFIER_ERROR_NO_WATCHER,
            "No Watcher found");
    dbus_failed (sn, err, FALSE);

    priv->dbus_watch_id = id;
}

/**
 * status_notifier_register:
 * @sn: A #StatusNotifier
 *
 * Registers @sn to the StatusNotifierWatcher over DBus.
 *
 * Once you have created your #StatusNotifier you need to register it, so any
 * host/visualization can use it and update their GUI as needed.
 *
 * This function will connect to the StatusNotifierWatcher and make sure at
 * least one StatusNotifierHost is registered. Then, it will register a new
 * StatusNotifierItem on the session bus and register it with the watcher.
 *
 * When done, property #StatusNotifier:state will change to
 * %STATUS_NOTIFIER_STATE_REGISTERED. If something fails, signal
 * #StatusNotifier::registration-failed will be emitted, at which point you
 * should fallback to using the systray.
 *
 * However there are two possible types of failures: fatal and non-fatal ones.
 * Fatal error means that #StatusNotifier:state will be
 * %STATUS_NOTIFIER_STATE_FAILED and you can unref @sn.
 *
 * Non-fatal error means it will still be %STATUS_NOTIFIER_STATE_REGISTERING as
 * the registration process could still eventually succeed. For example, if
 * there was no host registered on the watcher, as soon as signal
 * StatusNotifierHostRegistered is emitted on the watcher, the registration
 * process for @sn will complete and #StatusNotifier:state set to
 * %STATUS_NOTIFIER_STATE_REGISTERED, at which point you should stop using the
 * systray.
 *
 * This also means it is possible to have multiple signals
 * #StatusNotifier::registration-failed emitted on the same #StatusNotifier.
 *
 * Note that you can call status_notifier_register() after a fatal error
 * occured, to try again. You can also unref @sn while it is
 * %STATUS_NOTIFIER_STATE_REGISTERING safely.
 */
void
status_notifier_register (StatusNotifier          *sn)

{
    StatusNotifierPrivate *priv;

    g_return_if_fail (IS_STATUS_NOTIFIER (sn));
    priv = sn->priv;

    if (priv->state == STATUS_NOTIFIER_STATE_REGISTERING
            || priv->state == STATUS_NOTIFIER_STATE_REGISTERED)
        return;
    priv->state = STATUS_NOTIFIER_STATE_REGISTERING;

    priv->dbus_watch_id = g_bus_watch_name (G_BUS_TYPE_SESSION,
            WATCHER_NAME,
            G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
            watcher_appeared,
            watcher_vanished,
            sn, NULL);
}

/**
 * status_notifier_get_state:
 * @sn: A #StatusNotifier
 *
 * Returns the DBus registration state of @sn. See status_notifier_register()
 * for more.
 *
 * Returns: The DBus registration state of @sn
 */
StatusNotifierState
status_notifier_get_state (StatusNotifier          *sn)
{
    g_return_val_if_fail (IS_STATUS_NOTIFIER (sn), FALSE);
    return sn->priv->state;
}
