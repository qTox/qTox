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


#include "enums.h"
GType
status_notifier_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0)
  {
    static const GEnumValue values[] = {
        { STATUS_NOTIFIER_ERROR_NO_CONNECTION, "STATUS_NOTIFIER_ERROR_NO_CONNECTION", "connection" },
        { STATUS_NOTIFIER_ERROR_NO_NAME, "STATUS_NOTIFIER_ERROR_NO_NAME", "name" },
        { STATUS_NOTIFIER_ERROR_NO_WATCHER, "STATUS_NOTIFIER_ERROR_NO_WATCHER", "watcher" },
        { STATUS_NOTIFIER_ERROR_NO_HOST, "STATUS_NOTIFIER_ERROR_NO_HOST", "host" },
        { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("StatusNotifierError", values);
  }
  return etype;
}
GType
status_notifier_state_get_type (void)
{
  static GType etype = 0;
  if (etype == 0)
  {
    static const GEnumValue values[] = {
        { STATUS_NOTIFIER_STATE_NOT_REGISTERED, "STATUS_NOTIFIER_STATE_NOT_REGISTERED", "not-registered" },
        { STATUS_NOTIFIER_STATE_REGISTERING, "STATUS_NOTIFIER_STATE_REGISTERING", "registering" },
        { STATUS_NOTIFIER_STATE_REGISTERED, "STATUS_NOTIFIER_STATE_REGISTERED", "registered" },
        { STATUS_NOTIFIER_STATE_FAILED, "STATUS_NOTIFIER_STATE_FAILED", "failed" },
        { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("StatusNotifierState", values);
  }
  return etype;
}
GType
status_notifier_icon_get_type (void)
{
  static GType etype = 0;
  if (etype == 0)
  {
    static const GEnumValue values[] = {
        { STATUS_NOTIFIER_ICON, "STATUS_NOTIFIER_ICON", "status-notifier-icon" },
        { STATUS_NOTIFIER_ATTENTION_ICON, "STATUS_NOTIFIER_ATTENTION_ICON", "status-notifier-attention-icon" },
        { STATUS_NOTIFIER_OVERLAY_ICON, "STATUS_NOTIFIER_OVERLAY_ICON", "status-notifier-overlay-icon" },
        { STATUS_NOTIFIER_TOOLTIP_ICON, "STATUS_NOTIFIER_TOOLTIP_ICON", "status-notifier-tooltip-icon" },
        { _NB_STATUS_NOTIFIER_ICONS, "_NB_STATUS_NOTIFIER_ICONS", "-nb-status-notifier-icons" },
        { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("StatusNotifierIcon", values);
  }
  return etype;
}
GType
status_notifier_category_get_type (void)
{
  static GType etype = 0;
  if (etype == 0)
  {
    static const GEnumValue values[] = {
        { STATUS_NOTIFIER_CATEGORY_APPLICATION_STATUS, "STATUS_NOTIFIER_CATEGORY_APPLICATION_STATUS", "application-status" },
        { STATUS_NOTIFIER_CATEGORY_COMMUNICATIONS, "STATUS_NOTIFIER_CATEGORY_COMMUNICATIONS", "communications" },
        { STATUS_NOTIFIER_CATEGORY_SYSTEM_SERVICES, "STATUS_NOTIFIER_CATEGORY_SYSTEM_SERVICES", "system-services" },
        { STATUS_NOTIFIER_CATEGORY_HARDWARE, "STATUS_NOTIFIER_CATEGORY_HARDWARE", "hardware" },
        { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("StatusNotifierCategory", values);
  }
  return etype;
}
GType
status_notifier_status_get_type (void)
{
  static GType etype = 0;
  if (etype == 0)
  {
    static const GEnumValue values[] = {
        { STATUS_NOTIFIER_STATUS_PASSIVE, "STATUS_NOTIFIER_STATUS_PASSIVE", "passive" },
        { STATUS_NOTIFIER_STATUS_ACTIVE, "STATUS_NOTIFIER_STATUS_ACTIVE", "active" },
        { STATUS_NOTIFIER_STATUS_NEEDS_ATTENTION, "STATUS_NOTIFIER_STATUS_NEEDS_ATTENTION", "needs-attention" },
        { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("StatusNotifierStatus", values);
  }
  return etype;
}
GType
status_notifier_scroll_orientation_get_type (void)
{
  static GType etype = 0;
  if (etype == 0)
  {
    static const GEnumValue values[] = {
        { STATUS_NOTIFIER_SCROLL_ORIENTATION_HORIZONTAL, "STATUS_NOTIFIER_SCROLL_ORIENTATION_HORIZONTAL", "horizontal" },
        { STATUS_NOTIFIER_SCROLL_ORIENTATION_VERTICAL, "STATUS_NOTIFIER_SCROLL_ORIENTATION_VERTICAL", "vertical" },
        { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("StatusNotifierScrollOrientation", values);
  }
  return etype;
}
