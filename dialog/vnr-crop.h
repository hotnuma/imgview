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

#ifndef VNR_CROP_H
#define VNR_CROP_H

#include "window.h"

G_BEGIN_DECLS

typedef struct _VnrCrop VnrCrop;

#define VNR_TYPE_CROP (vnr_crop_get_type())
G_DECLARE_FINAL_TYPE(VnrCrop, vnr_crop, VNR, CROP, GObject)

struct _VnrCrop
{
    GObject __parent__;

    VnrWindow *window;

    GdkPixbuf *preview_pixbuf;

    gdouble zoom;
    gdouble width;
    gdouble height;

    GtkWidget *image;
    GtkSpinButton *spin_x;
    GtkSpinButton *spin_y;
    GtkSpinButton *spin_width;
    GtkSpinButton *spin_height;

    gdouble sub_x;
    gdouble sub_y;
    gdouble sub_width;
    gdouble sub_height;

    gboolean drawing_rectangle;
    gboolean do_redraw;
    gdouble start_x;
    gdouble start_y;

    GdkRectangle area;
};

struct _VnrCropClass
{
    GObjectClass parent_class;
};

GType vnr_crop_get_type() G_GNUC_CONST;

GObject* vnr_crop_new(VnrWindow *vnr_win);
gboolean vnr_crop_run(VnrCrop *crop);

G_END_DECLS

#endif // VNR_CROP_H


