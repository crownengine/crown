/* GTK - The GIMP Toolkit
 * Copyright (C) 2024 the GTK team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once


#if !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#include <gtk/gtkcolorpickerprivate.h>

G_BEGIN_DECLS


#define GTK_TYPE_COLOR_PICKER_QUARTZ gtk_color_picker_quartz_get_type ()
G_DECLARE_FINAL_TYPE (GtkColorPickerQuartz, gtk_color_picker_quartz, GTK, COLOR_PICKER_QUARTZ, GObject)

GDK_AVAILABLE_IN_ALL
GtkColorPicker * gtk_color_picker_quartz_new (void);

G_END_DECLS
