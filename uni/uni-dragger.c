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

#include "uni-dragger.h"

#include "uni-image-view.h"
#include "uni-utils.h"
#include <math.h>

static void uni_dragger_finalize(GObject *object);
static void uni_dragger_set_property(GObject *object,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *pspec);
static void _uni_dragger_grab_pointer(UniDragger *dragger, GdkEventButton *event);
static void _uni_dragger_get_drag_delta(UniDragger *dragger, int *x, int *y);

#define NEW_FUNCS

static GtkTargetEntry target_table[] =
{
    {"text/uri-list", 0, 0},
};

enum
{
    PROP_IMAGE_VIEW = 1
};


// creation / destruction -----------------------------------------------------

G_DEFINE_TYPE(UniDragger, uni_dragger, G_TYPE_OBJECT)

UniDragger* uni_dragger_new(GtkWidget *view)
{
    g_return_val_if_fail(view != NULL, NULL);

    return UNI_DRAGGER(g_object_new(UNI_TYPE_DRAGGER,
                                    "view", view, NULL));
}

static void uni_dragger_class_init(UniDraggerClass *klass)
{
    GObjectClass *object_class = (GObjectClass *)klass;
    object_class->finalize = uni_dragger_finalize;
    object_class->set_property = uni_dragger_set_property;

    GParamSpec *pspec = g_param_spec_object("view",
                                            "Image View",
                                            "Image View to navigate",
                                            UNI_TYPE_IMAGE_VIEW,
                                            G_PARAM_CONSTRUCT_ONLY
                                            | G_PARAM_WRITABLE);

    g_object_class_install_property(object_class, PROP_IMAGE_VIEW, pspec);
}

static void uni_dragger_init(UniDragger *dragger)
{
    dragger->cache = uni_pixbuf_draw_cache_new();

    dragger->grab_cursor = gdk_cursor_new_for_display(
                                gdk_display_get_default(), GDK_FLEUR);
}

static void uni_dragger_set_property(GObject *object, guint prop_id,
                                     const GValue *value, GParamSpec *pspec)
{
    UniDragger *dragger = UNI_DRAGGER(object);

    if (prop_id == PROP_IMAGE_VIEW)
        dragger->view = g_value_get_object(value);
    else
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
}

static void uni_dragger_finalize(GObject *object)
{
    UniDragger *dragger = UNI_DRAGGER(object);
    uni_pixbuf_draw_cache_free(dragger->cache);

    if (dragger->grab_cursor)
    {
        g_object_unref(dragger->grab_cursor);
        dragger->grab_cursor = NULL;
    }

    // Chain up
    G_OBJECT_CLASS(uni_dragger_parent_class)->finalize(object);
}


// public ---------------------------------------------------------------------

gboolean uni_dragger_button_press(UniDragger *dragger, GdkEventButton *event)
{
    _uni_dragger_grab_pointer(dragger, event);

    dragger->pressed = TRUE;
    dragger->drag_base_x = event->x;
    dragger->drag_base_y = event->y;
    dragger->drag_ofs_x = event->x;
    dragger->drag_ofs_y = event->y;

    return TRUE;
}

static void _uni_dragger_grab_pointer(UniDragger *dragger, GdkEventButton *event)
{
    if (event->button != 1)
        return;

#ifdef NEW_FUNCS

    GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default());

    GdkGrabStatus ret = gdk_seat_grab(seat,
                  event->window,
                  GDK_SEAT_CAPABILITY_POINTER
                  | GDK_SEAT_CAPABILITY_KEYBOARD,
                  FALSE,
                  dragger->grab_cursor,
                  NULL,
                  NULL,
                  NULL);

    if (ret != GDK_GRAB_SUCCESS)
        gdk_seat_ungrab(seat);

#else
    int mask = (GDK_POINTER_MOTION_MASK
                | GDK_POINTER_MOTION_HINT_MASK
                | GDK_BUTTON_RELEASE_MASK);

    gdk_pointer_grab(event->window,
                     FALSE,
                     mask,
                     NULL,
                     tool->grab_cursor,
                     event->time);
#endif

    //printf("grab\n");
}

gboolean uni_dragger_button_release(UniDragger *dragger, GdkEventButton *event)
{
    //printf("enter ungrab\n");

#ifdef NEW_FUNCS

    GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default());
    gdk_seat_ungrab(seat);

#else

    if (event->button != 1)
        return FALSE;

    gdk_pointer_ungrab(event->time);

#endif

    //printf("ungrab\n");

    dragger->pressed = FALSE;
    dragger->dragging = FALSE;

    return TRUE;
}

gboolean uni_dragger_motion_notify(UniDragger *dragger, GdkEventMotion *event)
{
    if (!dragger->pressed)
        return FALSE;

    dragger->dragging = TRUE;

    dragger->drag_ofs_x = event->x;
    dragger->drag_ofs_y = event->y;

    int dx;
    int dy;
    _uni_dragger_get_drag_delta(dragger, &dx, &dy);

    if (abs(dx) < 1 && abs(dy) < 1)
        return FALSE;

    GtkAdjustment *vadj = uni_image_view_get_vadjustment(
                UNI_IMAGE_VIEW(dragger->view));
    GtkAdjustment *hadj = uni_image_view_get_hadjustment(
                UNI_IMAGE_VIEW(dragger->view));

    if (pow(dx, 2) + pow(dy, 2) > 7
        && UNI_IMAGE_VIEW(dragger->view)->pixbuf != NULL
        && gtk_adjustment_get_upper(vadj)
            <= gtk_adjustment_get_page_size(vadj)
        && gtk_adjustment_get_upper(hadj)
            <= gtk_adjustment_get_page_size(hadj))
    {
        uni_dragger_button_release(dragger, (GdkEventButton*) event);

        gtk_drag_begin_with_coordinates(
                            GTK_WIDGET(dragger->view),
                            gtk_target_list_new(target_table,
                                                G_N_ELEMENTS(target_table)),
                            GDK_ACTION_COPY,
                            1,
                            (GdkEvent*) event,
                            -1,
                            -1);

        return TRUE;
    }

    GdkRectangle viewport;

    uni_image_view_get_viewport(UNI_IMAGE_VIEW(dragger->view), &viewport);

    int offset_x = viewport.x + dx;
    int offset_y = viewport.y + dy;

    // move image...
    uni_image_view_set_offset(UNI_IMAGE_VIEW(dragger->view),
                              offset_x, offset_y, uni_is_wayland());

    dragger->drag_base_x = dragger->drag_ofs_x;
    dragger->drag_base_y = dragger->drag_ofs_y;

    return TRUE;
}

static void _uni_dragger_get_drag_delta(UniDragger *dragger, int *x, int *y)
{
    *x = dragger->drag_base_x - dragger->drag_ofs_x;
    *y = dragger->drag_base_y - dragger->drag_ofs_y;
}


// ----------------------------------------------------------------------------

void uni_dragger_pixbuf_changed(UniDragger *dragger, gboolean reset_fit,
                                GdkRectangle *rect)
{
    uni_pixbuf_draw_cache_invalidate(dragger->cache);
}

void uni_dragger_paint_image(UniDragger *dragger, UniPixbufDrawOpts *opts,
                             cairo_t *cr)
{
    uni_pixbuf_draw_cache_draw(dragger->cache, opts, cr);
}


