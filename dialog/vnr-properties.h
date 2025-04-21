/*
 * Copyright © 2009-2018 Siyan Panayotov <contact@siyanpanayotov.com>
 *
 * This file is part of ImgView.
 *
 * ImgView is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ImgView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ImgView.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VNR_PROPERTIES_DIALOG_H__
#define __VNR_PROPERTIES_DIALOG_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <gdk/gdkkeysyms.h>
#include "uni-image-view.h"
#include "window.h"

G_BEGIN_DECLS

typedef struct _VnrPropertiesDialog VnrPropertiesDialog;

#define VNR_TYPE_PROPERTIES_DIALOG \
    (vnr_propsdlg_get_type())

G_DECLARE_FINAL_TYPE(VnrPropertiesDialog, vnr_propsdlg, VNR, PROPERTIES_DIALOG, GtkDialog)

struct _VnrPropertiesDialog
{
    GtkDialog __parent__;

    GtkWidget *layout;
    GtkWidget *image_layout;
    GtkWidget *image;
    GtkWidget *meta_names_box;
    GtkWidget *meta_values_box;

    GtkWidget *close_button;
    GtkWidget *next_button;
    GtkWidget *prev_button;

    GtkWidget *location_label;
    GtkWidget *name_label;
    GtkWidget *type_label;
    GtkWidget *size_label;
    GtkWidget *width_label;
    GtkWidget *height_label;
    GtkWidget *modified_label;

    GdkPixbuf *thumbnail;

    VnrWindow *window;
};

GType vnr_propsdlg_get_type(void) G_GNUC_CONST;

GtkWidget *vnr_propsdlg_new(VnrWindow *vnr_win);

void vnr_propsdlg_update(VnrPropertiesDialog *dialog);
void vnr_propsdlg_update_image(VnrPropertiesDialog *dialog);
void vnr_propsdlg_clear(VnrPropertiesDialog *dialog);
void vnr_propsdlg_show(VnrPropertiesDialog *dialog);

G_END_DECLS

#endif // __VNR_PROPERTIES_DIALOG_H__


