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

#ifndef __UNI_IMAGE_VIEW_H__
#define __UNI_IMAGE_VIEW_H__

#include "preferences.h"

G_BEGIN_DECLS

#define UNI_TYPE_IMAGE_VIEW (uni_image_view_get_type())

#define UNI_IMAGE_VIEW(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), UNI_TYPE_IMAGE_VIEW, UniImageView))
#define UNI_IMAGE_VIEW_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), UNI_TYPE_IMAGE_VIEW, UniImageViewClass))
#define UNI_IS_IMAGE_VIEW(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), UNI_TYPE_IMAGE_VIEW))
#define UNI_IS_IMAGE_VIEW_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), UNI_TYPE_IMAGE_VIEW))
#define UNI_IMAGE_VIEW_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), UNI_TYPE_IMAGE_VIEW, UniImageViewClass))

typedef struct _UniImageView UniImageView;
typedef struct _UniImageViewClass UniImageViewClass;
typedef struct _UniImageViewPrivate UniImageViewPrivate;

typedef enum
{
    UNI_FITTING_NONE,   // Fitting disabled
    UNI_FITTING_NORMAL, // Fitting enabled. Max zoom: 1x
    UNI_FITTING_FULL,   // Fitting enabled. Max zoom: UNI_ZOOM_MAX

} UniFittingMode;

struct _UniImageView
{
    GtkWidget __parent__;

    gboolean is_rendering;
    GdkInterpType interp;
    UniFittingMode fitting;
    GdkPixbuf *pixbuf;
    gdouble zoom;

    // offset in zoom space coordinates of the image area in the widget.
    gdouble offset_x;
    gdouble offset_y;

    gboolean show_cursor;
    GdkCursor *void_cursor;
    UniImageViewPrivate *priv;

    GObject *dragger;
};

struct _UniImageViewClass
{
    GtkWidgetClass __parent__;

    // keybinding signals
    void (*set_zoom) (UniImageView *view, gdouble zoom);
    void (*zoom_in) (UniImageView *view);
    void (*zoom_out) (UniImageView *view);

    void (*set_fitting) (UniImageView *view, UniFittingMode fitting);
    void (*scroll) (UniImageView *view,
                    GtkScrollType xscroll, GtkScrollType yscroll);

    // non-keybinding signals
    void (*set_scroll_adjustments) (UniImageView *view,
                                    GtkAdjustment *hadj,
                                    GtkAdjustment *vadj);

    void (*pixbuf_changed) (UniImageView *view);
};

GType uni_image_view_get_type() G_GNUC_CONST;

// constructors
GtkWidget* uni_image_view_new();

// read-only properties
gboolean uni_image_view_get_viewport(UniImageView *view, GdkRectangle *rect);
gboolean uni_image_view_get_draw_rect(UniImageView *view, GdkRectangle *rect);

// write-only properties
void uni_image_view_set_offset(UniImageView *view, gdouble x, gdouble y,
                               gboolean invalidate);

// read-write properties
void uni_image_view_set_fitting(UniImageView *view, UniFittingMode fitting);
GdkPixbuf* uni_image_view_get_pixbuf(UniImageView *view);
void uni_image_view_set_pixbuf(UniImageView *view, GdkPixbuf *pixbuf,
                               gboolean reset_fit);
void uni_image_view_set_zoom(UniImageView *view, gdouble zoom);
void uni_image_view_set_zoom_mode(UniImageView *view, VnrPrefsZoom mode);

// actions
void uni_image_view_zoom_in(UniImageView *view);
void uni_image_view_zoom_out(UniImageView *view);
void uni_image_view_damage_pixels(UniImageView *view, GdkRectangle *rect);
GtkAdjustment* uni_image_view_get_vadjustment(UniImageView *view);
GtkAdjustment* uni_image_view_get_hadjustment(UniImageView *view);
void uni_image_view_set_vadjustment(UniImageView *view, GtkAdjustment *vadj);
void uni_image_view_set_hadjustment(UniImageView *view, GtkAdjustment *hadj);

G_END_DECLS

#endif // __UNI_IMAGE_VIEW_H__


