/*
 * Copyright © 2009-2018 Siyan Panayotov <contact@siyanpanayotov.com>
 *
 * Based on code by (see README for details):
 * - Björn Lindqvist <bjourne@gmail.com>
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

#ifndef __UNI_TOOL_DRAGGER_H__
#define __UNI_TOOL_DRAGGER_H__

#include "uni-cache.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define UNI_TYPE_DRAGGER (uni_dragger_get_type())
G_DECLARE_FINAL_TYPE(UniDragger, uni_dragger, UNI, DRAGGER, GObject)

typedef struct _UniDragger UniDragger;

struct _UniDragger
{
    GObject __parent__;

    GtkWidget *view;
    UniDrawCache *cache;

    gboolean pressed;
    gboolean dragging;

    // Position where the mouse was pressed.
    int drag_base_x;
    int drag_base_y;

    // Current position of the mouse.
    int drag_ofs_x;
    int drag_ofs_y;

    // Cursor to use when grabbing.
    GdkCursor *grab_cursor;
};

GType uni_dragger_get_type() G_GNUC_CONST;

UniDragger *uni_dragger_new(GtkWidget *view);

gboolean uni_dragger_button_press(UniDragger *dragger, GdkEventButton *event);
gboolean uni_dragger_button_release(UniDragger *dragger,
                                    GdkEventButton *event);
gboolean uni_dragger_motion_notify(UniDragger *dragger,
                                   GdkEventMotion *event);

void uni_dragger_pixbuf_changed(UniDragger *dragger, gboolean reset_fit,
                                GdkRectangle *rect);
void uni_dragger_paint_image(UniDragger *dragger, UniDrawOpts *opts,
                             cairo_t *cr);

G_END_DECLS

#endif // __UNI_TOOL_DRAGGER_H__


