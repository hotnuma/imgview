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

#ifndef __VNR_PREFERENCES_H__
#define __VNR_PREFERENCES_H__

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <gdk/gdkkeysyms.h>

G_BEGIN_DECLS

typedef struct _VnrPrefs VnrPrefs;

#define VNR_TYPE_PREFS (vnr_prefs_get_type())
G_DECLARE_FINAL_TYPE(VnrPrefs, vnr_prefs, VNR, PREFS, GObject)

typedef enum
{
    VNR_PREFS_ZOOM_SMART,
    VNR_PREFS_ZOOM_NORMAL,
    VNR_PREFS_ZOOM_FIT,
    VNR_PREFS_ZOOM_LAST_USED,

} VnrPrefsZoom;

typedef enum
{
    VNR_PREFS_DESKTOP_AUTO,
    VNR_PREFS_DESKTOP_CINNAMON,
    VNR_PREFS_DESKTOP_FLUXBOX,
    VNR_PREFS_DESKTOP_GNOME2,
    VNR_PREFS_DESKTOP_GNOME3,
    VNR_PREFS_DESKTOP_LXDE,
    VNR_PREFS_DESKTOP_MATE,
    VNR_PREFS_DESKTOP_NITROGEN,
    VNR_PREFS_DESKTOP_PUPPY,
    VNR_PREFS_DESKTOP_WALLSET,
    VNR_PREFS_DESKTOP_XFCE,

} VnrPrefsDesktop;

typedef enum
{
    VNR_PREFS_WHEEL_NAVIGATE,
    VNR_PREFS_WHEEL_ZOOM,
    VNR_PREFS_WHEEL_SCROLL,

} VnrPrefsWheel;

typedef enum
{
    VNR_PREFS_CLICK_ZOOM,
    VNR_PREFS_CLICK_FULLSCREEN,
    VNR_PREFS_CLICK_NEXT,

} VnrPrefsClick;

typedef enum
{
    VNR_PREFS_MODIFY_AUTOSAVE,
    VNR_PREFS_MODIFY_IGNORE,

} VnrPrefsModify;

struct _VnrPrefs
{
    GObject __parent__;

    GtkWidget *window;
    gboolean start_maximized;
    gint window_width;
    gint window_height;

    VnrPrefsZoom zoom;
    VnrPrefsDesktop desktop;
    gboolean smooth_images;
    gboolean confirm_delete;
    gboolean show_hidden;
    gboolean dark_background;

    gint sl_timeout;
    GtkSpinButton *sl_timeout_widget;
    gboolean fit_on_fullscreen;

    VnrPrefsWheel wheel_behavior;
    VnrPrefsClick click_behavior;
    VnrPrefsModify modify_behavior;

    gboolean reload_on_save;
    gint jpeg_quality;
    gint png_compression;

    gboolean start_slideshow;
    gboolean start_fullscreen;
    gboolean auto_resize;
    gboolean show_scrollbar;

};

struct _VnrPrefsClass
{
    GObjectClass parent_class;
};

GType vnr_prefs_get_type(void) G_GNUC_CONST;

GObject *vnr_prefs_new(GtkWidget *window);
void vnr_prefs_dialog_run(VnrPrefs *prefs);
void vnr_prefs_set_slideshow_timeout(VnrPrefs *prefs, int value);
void vnr_prefs_set_show_scrollbar(VnrPrefs *prefs, gboolean show_scollbar);
gboolean vnr_prefs_save(VnrPrefs *prefs);

G_END_DECLS

#endif // __VNR_PREFERENCES_H__


