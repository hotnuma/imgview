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

#ifndef __UNI_SCROLL_WIN_H__
#define __UNI_SCROLL_WIN_H__

// UniScrollWin is a class that implements a type of
// GtkScrolledWindow which is more integrated with UniImageView.

#include <gdk/gdk.h>
#include <gtk/gtk.h>

//#include "uni-nav.h"
#include "uni-image-view.h"

G_BEGIN_DECLS

#define WITH_GRID

#define UNI_TYPE_SCROLL_WIN (uni_scroll_win_get_type())

#define UNI_SCROLL_WIN(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), UNI_TYPE_SCROLL_WIN, UniScrollWin))
#define UNI_SCROLL_WIN_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), UNI_TYPE_SCROLL_WIN, UniScrollWinClass))
#define UNI_IS_SCROLL_WIN(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), UNI_TYPE_SCROLL_WIN))
#define UNI_IS_SCROLL_WIN_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), UNI_TYPE_SCROLL_WIN))
#define UNI_SCROLL_WIN_GET_CLASS(obj) \
    (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), UNI_TYPE_SCROLL_WIN, \
                                     UniScrollWinClass))

typedef struct _UniScrollWin UniScrollWin;
typedef struct _UniScrollWinClass UniScrollWinClass;

struct _UniScrollWin
{
#ifdef WITH_GRID
    GtkGrid __parent__;
#else
    GtkTable __parent__;
#endif

    GtkWidget *hscroll;
    GtkWidget *vscroll;
    GtkWidget *nav_box;
    //GtkWidget *nav;

    // The GtkImage that shows the nav_button icon.
    GtkWidget *nav_image;

    // The normal and the highlighted nav_button pixbuf.
    GdkPixbuf *nav_button;

    gboolean show_scrollbar;
};

struct _UniScrollWinClass
{
#ifdef WITH_GRID
    GtkGridClass __parent_class__;
#else
    GtkTableClass __parent_class__;
#endif
};

GType uni_scroll_win_get_type() G_GNUC_CONST;

// Constructors
GtkWidget* uni_scroll_win_new(UniImageView *view);

gboolean uni_scroll_win_image_fits(UniScrollWin *window);
void uni_scroll_win_set_show_scrollbar(UniScrollWin *window, gboolean show);

G_END_DECLS

#endif // __UNI_SCROLL_WIN_H__


