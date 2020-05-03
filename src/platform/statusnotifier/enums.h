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
#include "statusnotifier.h"


GType status_notifier_error_get_type(void);
#define TYPE_STATUS_NOTIFIER_ERROR (status_notifier_error_get_type())
GType status_notifier_state_get_type(void);
#define TYPE_STATUS_NOTIFIER_STATE (status_notifier_state_get_type())
GType status_notifier_icon_get_type(void);
#define TYPE_STATUS_NOTIFIER_ICON (status_notifier_icon_get_type())
GType status_notifier_category_get_type(void);
#define TYPE_STATUS_NOTIFIER_CATEGORY (status_notifier_category_get_type())
GType status_notifier_status_get_type(void);
#define TYPE_STATUS_NOTIFIER_STATUS (status_notifier_status_get_type())
GType status_notifier_scroll_orientation_get_type(void);
#define TYPE_STATUS_NOTIFIER_SCROLL_ORIENTATION (status_notifier_scroll_orientation_get_type())
G_END_DECLS
