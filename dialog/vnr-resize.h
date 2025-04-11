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

#ifndef __VNR_RESIZE_DLG_H__
#define __VNR_RESIZE_DLG_H__

#include "window.h"

G_BEGIN_DECLS

#define VNR_TYPE_RESIZE_DLG (vnr_resize_dlg_get_type())
G_DECLARE_FINAL_TYPE(VnrResizeDlg, vnr_resize_dlg, VNR, RESIZE_DLG, GObject)

struct _VnrResizeDlg
{
    GtkDialog __parent__;

    VnrWindow *window;

//    GtkWidget *layout;
//    GtkWidget *image_layout;
//    GtkWidget *image;
//    GtkWidget *meta_names_box;
//    GtkWidget *meta_values_box;

//    GtkWidget *close_button;
//    GtkWidget *next_button;
//    GtkWidget *prev_button;

//    GtkWidget *location_label;
//    GtkWidget *name_label;
//    GtkWidget *type_label;
//    GtkWidget *size_label;
//    GtkWidget *width_label;
//    GtkWidget *height_label;
//    GtkWidget *modified_label;

//    GdkPixbuf *thumbnail;
};

GType vnr_resize_dlg_get_type() G_GNUC_CONST;

gboolean vnr_resize_dlg_run(VnrWindow *window);
GtkWidget *vnr_resize_dlg_new(VnrWindow *vnr_win);

G_END_DECLS

#endif // __VNR_RESIZE_DLG_H__


