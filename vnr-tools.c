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

#include "config.h"
#include "vnr-tools.h"

void vnr_tools_set_cursor(GtkWidget *widget,
                          GdkCursorType type, gboolean flush)
{
    GdkDisplay *display = gtk_widget_get_display(widget);
    GdkCursor *cursor = gdk_cursor_new_for_display(display, type);

    gdk_window_set_cursor(gtk_widget_get_window(widget), cursor);

    // mem leak ???

    if (flush)
        gdk_display_flush(display);
}

void vnr_tools_fit_to_size(gint *width, gint *height,
                           gint max_width, gint max_height)
{
    gfloat ratio, max_ratio;

    // if size fits well, then exit
    if (*width < max_width && *height < max_height)
        return;

    // check if dividing by 0
    if (*width == 0 || max_height == 0)
        return;

    ratio = 1. * (*height) / (*width);
    max_ratio = 1. * max_height / max_width;

    if (max_ratio > ratio)
    {
        *width = max_width;
        *height = ratio * (*width);
    }
    else if (ratio > max_ratio)
    {
        *height = max_height;
        *width = (*height) / ratio;
    }
    else
    {
        *width = max_width;
        *height = max_height;
    }

    return;
}

void vnr_tools_fit_to_size_double(gdouble *width, gdouble *height,
                                  gint max_width, gint max_height)
{
    gdouble ratio, max_ratio;

    // if size fits well, then exit
    if (*width < max_width && *height < max_height)
        return;

    // check if dividing by 0
    if (*width == 0 || max_height == 0)
        return;

    ratio = 1. * (*height) / (*width);
    max_ratio = 1. * max_height / max_width;

    if (max_ratio > ratio)
    {
        *width = max_width;
        *height = ratio * (*width);
    }
    else if (ratio > max_ratio)
    {
        *height = max_height;
        *width = (*height) / ratio;
    }
    else
    {
        *width = max_width;
        *height = max_height;
    }

    return;
}

GSList* vnr_tools_get_list_from_array(gchar **files)
{
    GSList *uri_list = NULL;
    gint i;

    if (files == NULL)
        return NULL;

    for (i = 0; files[i]; i++)
    {
        char *uri_string;

        GFile *file;

        file = g_file_new_for_commandline_arg(files[i]);

        uri_string = g_file_get_path(file);

        g_object_unref(file);

        if (uri_string)
        {
            uri_list = g_slist_prepend(uri_list, g_strdup(uri_string));
            g_free(uri_string);
        }
    }

    return g_slist_reverse(uri_list);
}

// modified version of eog's eog_util_parse_uri_string_list_to_file_list
GSList* vnr_tools_parse_uri_string_list_to_file_list(const gchar *uri_list)
{
    GSList *file_list = NULL;
    gsize i = 0;
    gchar **uris;

    uris = g_uri_list_extract_uris(uri_list);

    while (uris[i] != NULL)
    {
        gchar *current_path = g_file_get_path(g_file_new_for_uri(uris[i]));

        if (current_path != NULL)
            file_list = g_slist_append(file_list, current_path);

        i++;
    }

    g_strfreev(uris);

    return g_slist_reverse(file_list);
}

void vnr_tools_apply_embedded_orientation(GdkPixbufAnimation **anim)
{
    GdkPixbuf *pixbuf;
    GdkPixbuf *original;

    if (!gdk_pixbuf_animation_is_static_image(*anim))
        return;

    pixbuf = gdk_pixbuf_animation_get_static_image(*anim);
    original = pixbuf;
    pixbuf = gdk_pixbuf_apply_embedded_orientation(pixbuf);

    if (original == pixbuf)
    {
        g_object_unref(pixbuf);
        return;
    }

    GdkPixbufSimpleAnim *s_anim;

    s_anim = gdk_pixbuf_simple_anim_new(gdk_pixbuf_get_width(pixbuf),
                                        gdk_pixbuf_get_height(pixbuf),
                                        -1);
    gdk_pixbuf_simple_anim_add_frame(s_anim, pixbuf);

    g_object_unref(pixbuf);
    g_object_unref(*anim);

    *anim = GDK_PIXBUF_ANIMATION(s_anim);
}

gdImage* pixbuf_to_gd(GdkPixbuf *pixbuf)
{
    if (!pixbuf)
        return NULL;

    int sx = gdk_pixbuf_get_width(pixbuf);
    int sy = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    gdImage *img = gdImageCreateTrueColor(sx, sy);

    for (int y = 0; y < sy; ++y)
    {
        for (int x = 0; x < sx; ++x)
        {
            guchar *p = pixels + (y * rowstride) + (x * n_channels);
            img->tpixels[y][x] = gdTrueColorAlpha(p[0], p[1], p[2], 127);
        }
    }

    return img;
}

GdkPixbuf* gd_to_pixbuf(gdImage *src)
{
    g_return_val_if_fail(src != NULL, NULL);

    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8,
                                       src->sx, src->sy);

    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    for (int y = 0; y < src->sy; ++y)
    {
        for (int x = 0; x < src->sx; ++x)
        {
            int c = src->tpixels[y][x];

            guchar *p = pixels + (y * rowstride) + (x * n_channels);
            p[0] = gdTrueColorGetRed(c);
            p[1] = gdTrueColorGetGreen(c);
            p[2] = gdTrueColorGetBlue(c);
            p[3] = 2 * gdTrueColorGetAlpha(c);
        }
    }

    return pixbuf;
}


