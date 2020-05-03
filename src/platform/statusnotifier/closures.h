/*
 * statusnotifier - Copyright (C) 2014 Olivier Brunel
 *
 * closures.h
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

#include <glib-object.h>

G_BEGIN_DECLS

/* BOOLEAN:INT,INT (closures.def:1) */
extern void g_cclosure_user_marshal_BOOLEAN__INT_INT(GClosure* closure, GValue* return_value,
                                                     guint n_param_values, const GValue* param_values,
                                                     gpointer invocation_hint, gpointer marshal_data);

G_END_DECLS
