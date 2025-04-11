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

#ifndef __VNR_WINDOW_H__
#define __VNR_WINDOW_H__

#include <etkwidgetlist.h>
#include "preferences.h"
#include "file.h"

G_BEGIN_DECLS

typedef struct _VnrWindow VnrWindow;

#define VNR_TYPE_WINDOW (window_get_type())
G_DECLARE_FINAL_TYPE(VnrWindow, window, VNR, WINDOW, GtkWindow)

typedef enum
{
    WINDOW_MODE_NORMAL,
    WINDOW_MODE_FULLSCREEN,
    WINDOW_MODE_SLIDESHOW,

} WindowMode;

struct _VnrWindow
{
    GtkWindow __parent__;

    // data
    GList *filelist;
    gchar *destdir;
    WindowMode mode;
    GtkAccelGroup *accel_group;
    VnrPrefs *prefs;
    GList *list_image;
    gboolean can_edit;
    gboolean can_slideshow;
    gint max_width;
    gint max_height;
    gint current_image_height;
    gint current_image_width;
    gboolean cursor_is_hidden;
    guint8 modifications;
    gchar *writable_format_name;

    // reload
    GFileMonitor *monitor;
    gboolean need_reload;
    gboolean no_reload;

    // widgets
    GtkWidget *layout_box;
    GtkWidget *msg_area;
    GtkWidget *view;
    GtkWidget *scroll_view;
    GtkWidget *popup_menu;
    GtkWidget *openwith_item;
    GtkWidget *props_dlg;

    // fullscreen variables
    GtkWidget *fs_toolitem;
    //GtkWidget *fs_seconds_label;
    GtkWidget *toggle_btn;
    gboolean disable_autohide;
    GtkWidget *fs_filename_label;
    GSource *fs_source;

    // slideshow variables
    GtkWidget *sl_timeout_widget;
    guint sl_source_id;
    gint sl_timeout;
};

GType window_get_type() G_GNUC_CONST;

// creation
VnrWindow* window_new();

void window_list_set(VnrWindow *window, GList *list);
VnrFile *window_get_current_file(VnrWindow *window);
void window_list_set_current(VnrWindow *window, GList *list);

// open / close
void window_open_list(VnrWindow *window, GSList *uri_list);
gboolean window_load_file(VnrWindow *window);
void window_close_file(VnrWindow *window);

// browse images
void window_action_prev(VnrWindow *window, GtkWidget *widget);
void window_action_next(VnrWindow *window, GtkWidget *widget);
gboolean window_prev(VnrWindow *window);
gboolean window_next(VnrWindow *window, gboolean reset_timer);
gboolean window_first(VnrWindow *window);
gboolean window_last(VnrWindow *window);

void window_preferences_apply(VnrWindow *window);

void window_slideshow_deny(VnrWindow *window);
void window_fullscreen_toggle(VnrWindow *window);

G_END_DECLS

#endif // __VNR_WINDOW_H__


