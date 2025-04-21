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

#include "config.h"
#include "uni-utils.h"

#include <gdk/gdkx.h>
#include <gdk/gdkwayland.h>

static gint _uni_get_session_type();

gboolean uni_is_x11()
{
    return (_uni_get_session_type() == 0);
}

gboolean uni_is_wayland()
{
    return (_uni_get_session_type() == 1);
}

static gint sessiontype = -1;

static gint _uni_get_session_type()
{
    // must be called after gtk_init or g_option_context_parse

    if (sessiontype != -1)
        return sessiontype;

    GdkDisplay *display = gdk_display_get_default();

    if (GDK_IS_X11_DISPLAY(display))
        sessiontype = 0;
    else if (GDK_IS_WAYLAND_DISPLAY(display))
        sessiontype = 1;
    else
        sessiontype = -2;

    return sessiontype;
}

/**
 * uni_pixbuf_scale_blend:
 *
 * A utility function that either scales or composites color depending
 * on the number of channels in the source image. The last four
 * parameters are only used in the composite color case.
 **/
void uni_pixbuf_scale_blend(GdkPixbuf *src,
                            GdkPixbuf *dst,
                            int dst_x,
                            int dst_y,
                            int dst_width,
                            int dst_height,
                            gdouble offset_x,
                            gdouble offset_y,
                            gdouble zoom,
                            GdkInterpType interp, int check_x, int check_y)
{
    if (gdk_pixbuf_get_has_alpha(src))
        gdk_pixbuf_composite_color(src, dst,
                                   dst_x, dst_y, dst_width, dst_height,
                                   offset_x, offset_y,
                                   zoom, zoom,
                                   interp,
                                   255,
                                   check_x, check_y,
                                   CHECK_SIZE, CHECK_LIGHT, CHECK_DARK);
    else
        gdk_pixbuf_scale(src, dst,
                         dst_x, dst_y, dst_width, dst_height,
                         offset_x, offset_y, zoom, zoom, interp);
}

/**
 * uni_draw_rect:
 *
 * This function is a fixed version of gdk_draw_rectangle. The GDK
 * function is broken in that drawing a the rectangle (0,0)-[0,0] will
 * draw a pixel at position (0,0).
 **/
void uni_draw_rect(cairo_t *cr, gboolean filled, GdkRectangle *rect)
{
    if (rect->width <= 0 || rect->height <= 0)
        return;
    cairo_save(cr);
    cairo_rectangle(cr, rect->x, rect->y, rect->width - 1, rect->height - 1);
    cairo_clip(cr);
    if (filled)
        cairo_paint(cr);
    else
        cairo_stroke(cr);
    cairo_restore(cr);
}

void uni_rectangle_get_rects_around(GdkRectangle *outer,
                                    GdkRectangle *inner,
                                    GdkRectangle around[4])
{
    // top
    around[0].x = outer->x;
    around[0].y = outer->y;
    around[0].width = outer->width;
    around[0].height = inner->y - outer->y;

    // left
    around[1].x = outer->x;
    around[1].y = inner->y;
    around[1].width = inner->x - outer->x;
    around[1].height = inner->height;

    // right
    around[2].x = inner->x + inner->width;
    around[2].y = inner->y;
    around[2].width = (outer->x + outer->width) - (inner->x + inner->width);
    around[2].height = inner->height;

    // bottom
    around[3].x = outer->x;
    around[3].y = inner->y + inner->height;
    around[3].width = outer->width;
    around[3].height = (outer->y + outer->height) - (inner->y + inner->height);
}

VnrPrefsDesktop uni_detect_desktop_environment()
{
    VnrPrefsDesktop environment = VNR_PREFS_DESKTOP_WALLSET;

    gchar *xdg_current_desktop = g_ascii_strup(
                g_getenv("XDG_CURRENT_DESKTOP"), -1);
    gchar *xdg_session_desktop = g_ascii_strup(
                g_getenv("XDG_SESSION_DESKTOP"), -1);
    gchar *desktop_session = g_ascii_strdown(
                g_getenv("DESKTOP_SESSION"), -1);
    gchar *gdmsession = g_ascii_strdown(
                g_getenv("GDMSESSION"), -1);

    if (!g_strcmp0(xdg_current_desktop, "GNOME")
        || !g_strcmp0(xdg_session_desktop, "GNOME"))
    {
        if (!g_strcmp0(gdmsession, "gnome-classic")
            || !g_strcmp0(gdmsession, "gnome-fallback"))
        {
            environment = VNR_PREFS_DESKTOP_GNOME2;
        }
        else if (!g_strcmp0(gdmsession, "cinnamon"))
        {
            environment = VNR_PREFS_DESKTOP_CINNAMON;
        }
    }
    else if (!g_strcmp0(xdg_current_desktop, "XFCE")
             || !g_strcmp0(xdg_session_desktop, "XFCE"))
    {
        environment = VNR_PREFS_DESKTOP_XFCE;
    }
    else if (!g_strcmp0(xdg_current_desktop, "MATE")
             || !g_strcmp0(xdg_session_desktop, "MATE"))
    {
        environment = VNR_PREFS_DESKTOP_MATE;
    }
    else if (!g_strcmp0(xdg_current_desktop, "LXDE")
             || !g_strcmp0(xdg_session_desktop, "LXDE"))
    {
        environment = VNR_PREFS_DESKTOP_LXDE;
    }
    else if (!g_strcmp0(desktop_session, "fluxbox"))
    {
        environment = VNR_PREFS_DESKTOP_FLUXBOX;
    }

    g_free(xdg_current_desktop);
    g_free(xdg_session_desktop);
    g_free(desktop_session);
    g_free(gdmsession);

    return environment;
}


