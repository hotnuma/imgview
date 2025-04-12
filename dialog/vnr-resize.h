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

#ifndef __VNR_RESIZE_H__
#define __VNR_RESIZE_H__

#include "window.h"

G_BEGIN_DECLS

typedef struct _VnrResize VnrResize;

#define VNR_TYPE_RESIZE (vnr_resize_get_type())
G_DECLARE_FINAL_TYPE(VnrResize, vnr_resize, VNR, RESIZE, GObject)

struct _VnrResize
{
    GObject __parent__;

    VnrWindow *window;

    GtkSpinButton *spin_x;
    GtkSpinButton *spin_y;
    GtkSpinButton *spin_width;
    GtkSpinButton *spin_height;

    gdouble width;
    gdouble height;

    GdkRectangle area;

    //GdkPixbuf *preview_pixbuf;

    //gdouble zoom;

    //GtkWidget *image;

    //gdouble sub_x;
    //gdouble sub_y;
    //gdouble sub_width;
    //gdouble sub_height;

    //gboolean drawing_rectangle;
    //gboolean do_redraw;
    //gdouble start_x;
    //gdouble start_y;

};

struct _VnrResizeClass
{
    GObjectClass parent_class;
};

GType vnr_resize_get_type() G_GNUC_CONST;

GObject* vnr_resize_new(VnrWindow *window);
gboolean vnr_resize_run(VnrResize *resize);

G_END_DECLS

#endif // __VNR_RESIZE_H__


