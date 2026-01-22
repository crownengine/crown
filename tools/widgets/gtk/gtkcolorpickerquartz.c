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

#include "config.h"

#include <AppKit/AppKit.h>

#include "gtkcolorpickerquartzprivate.h"

#include <gio/gio.h>

struct _GtkColorPickerQuartz
{
  GObject parent_instance;

  NSColorSampler *sampler;
  GTask *task;
};

struct _GtkColorPickerQuartzClass
{
  GObjectClass parent_class;
};

static GInitableIface *initable_parent_iface;
static void gtk_color_picker_quartz_initable_iface_init (GInitableIface *iface);
static void gtk_color_picker_quartz_iface_init (GtkColorPickerInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GtkColorPickerQuartz, gtk_color_picker_quartz, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, gtk_color_picker_quartz_initable_iface_init)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_COLOR_PICKER, gtk_color_picker_quartz_iface_init))

static gboolean
gtk_color_picker_quartz_initable_init (GInitable     *initable,
                                       GCancellable  *cancellable,
                                       GError       **error)
{
  GtkColorPickerQuartz *picker = GTK_COLOR_PICKER_QUARTZ (initable);

  picker->sampler = [[NSColorSampler alloc] init];

  return TRUE;
}

static void
gtk_color_picker_quartz_initable_iface_init (GInitableIface *iface)
{
  initable_parent_iface = g_type_interface_peek_parent (iface);
  iface->init = gtk_color_picker_quartz_initable_init;
}

static void
gtk_color_picker_quartz_init (GtkColorPickerQuartz *picker)
{
}

static void
gtk_color_picker_quartz_finalize (GObject *object)
{
  GtkColorPickerQuartz *picker = GTK_COLOR_PICKER_QUARTZ (object);

  g_clear_object (&picker->task);
  if (picker->sampler != NULL)
    {
      [picker->sampler release];
      picker->sampler = NULL;
    }

  G_OBJECT_CLASS (gtk_color_picker_quartz_parent_class)->finalize (object);
}

static void
gtk_color_picker_quartz_class_init (GtkColorPickerQuartzClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = gtk_color_picker_quartz_finalize;
}

GtkColorPicker *
gtk_color_picker_quartz_new (void)
{
  return GTK_COLOR_PICKER (g_initable_new (GTK_TYPE_COLOR_PICKER_QUARTZ, NULL, NULL, NULL));
}


static void
gtk_color_picker_quartz_pick (GtkColorPicker      *cp,
                              GAsyncReadyCallback  callback,
                              gpointer             user_data)
{
  GtkColorPickerQuartz *picker = GTK_COLOR_PICKER_QUARTZ (cp);

  if (picker->task)
    return;

  picker->task = g_task_new (picker, NULL, callback, user_data);

  [picker->sampler showSamplerWithSelectionHandler:^(NSColor *selectedColor) {
    if (selectedColor == NULL)
      {
        g_task_return_new_error (picker->task,
                                 G_IO_ERROR,
                                 G_IO_ERROR_FAILED,
                                 "No color received");
      }
    else
      {
        GdkRGBA rgba = (GdkRGBA){
          [selectedColor redComponent],
          [selectedColor greenComponent],
          [selectedColor blueComponent],
          [selectedColor alphaComponent],
        };

        g_task_return_pointer (picker->task,
                               gdk_rgba_copy (&rgba),
                               (GDestroyNotify) gdk_rgba_free);
      }

    g_clear_object (&picker->task);
  }];
}

static GdkRGBA *
gtk_color_picker_quartz_pick_finish (GtkColorPicker  *cp,
                                     GAsyncResult    *res,
                                     GError         **error)
{
  g_return_val_if_fail (g_task_is_valid (res, cp), NULL);

  return g_task_propagate_pointer (G_TASK (res), error);
}

static void
gtk_color_picker_quartz_iface_init (GtkColorPickerInterface *iface)
{
  iface->pick = gtk_color_picker_quartz_pick;
  iface->pick_finish = gtk_color_picker_quartz_pick_finish;
}
