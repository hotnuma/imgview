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

#ifndef __VNR_TOOLS_H__
#define __VNR_TOOLS_H__

#include <gtk/gtk.h>

void vnr_tools_set_cursor(GtkWidget *widget,
                          GdkCursorType type, gboolean flush);
void vnr_tools_fit_to_size(gint *w, gint *h, gint mw, gint mh);
void vnr_tools_fit_to_size_double(gdouble *w, gdouble *h, gint mw, gint mh);

GSList *vnr_tools_get_list_from_array(gchar **files);
GSList *vnr_tools_parse_uri_string_list_to_file_list(const gchar *uri_list);
void vnr_tools_apply_embedded_orientation(GdkPixbufAnimation **anim);

#endif // __VNR_TOOLS_H__


