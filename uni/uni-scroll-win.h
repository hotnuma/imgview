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

#include "uni-image-view.h"

G_BEGIN_DECLS

#define UNI_TYPE_SCROLL_WIN (uni_scroll_win_get_type())
G_DECLARE_FINAL_TYPE(UniScrollWin, uni_scroll_win, UNI, SCROLL_WIN, GtkGrid)

typedef struct _UniScrollWin UniScrollWin;

struct _UniScrollWin
{
    GtkGrid __parent__;

    GtkWidget *hscroll;
    GtkWidget *vscroll;
    GtkWidget *view;

    gboolean show_scrollbar;
};


GType uni_scroll_win_get_type() G_GNUC_CONST;

GtkWidget* uni_scroll_win_new();
GtkWidget* uni_scroll_win_get_view(UniScrollWin *window);

gboolean uni_scroll_win_image_fits(UniScrollWin *window);
void uni_scroll_win_set_show_scrollbar(UniScrollWin *window, gboolean show);

G_END_DECLS

#endif // __UNI_SCROLL_WIN_H__


