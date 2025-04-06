/*
 * Copyright © 2009-2018 Siyan Panayotov <contact@siyanpanayotov.com>
 *
 * This file is part of ImgView.
 *
 * ImgView is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *(at your option) any later version.
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
#include "window.h"

#include "list.h"
#include "message-area.h"
#include "uni-scroll-win.h"
#include "uni-anim-view.h"
#include "uni-utils.h"
#include "vnr-tools.h"
#include "uni-exiv2.hpp"
#include "vnr-crop.h"
#include "vnr-properties-dialog.h"
#include "dialog.h"

#include <etkaction.h>
#include <sys/stat.h>
#include <sys/wait.h>

// Timeout to hide the toolbar in fullscreen mode
#define FULLSCREEN_TIMEOUT 1000
#define DARK_BACKGROUND_COLOR "#222222"

G_DEFINE_TYPE(VnrWindow, window, GTK_TYPE_WINDOW)

// creation / destruction -----------------------------------------------------

static void _window_on_realize(VnrWindow *window, gpointer user_data);
static gboolean _window_on_delete(VnrWindow *window, GdkEvent *event,
                                  gpointer data);
static void window_dispose(GObject *object);
static void window_finalize(GObject *object);

// ----------------------------------------------------------------------------

static void _window_override_background_color(VnrWindow *window,
                                              GdkRGBA *color);

// dnd ------------------------------------------------------------------------

static void _window_set_drag(VnrWindow *window);
static void _view_on_drag_begin(GtkWidget *widget,
                                GdkDragContext *drag_context,
                                GtkSelectionData *data,
                                guint info,
                                guint time,
                                gpointer user_data);
static void _window_drag_data_received(GtkWidget *widget,
                                       GdkDragContext *context,
                                       gint x, gint y,
                                       GtkSelectionData *selection_data,
                                       guint info, guint time);

// window ---------------------------------------------------------------------

static gint _window_on_key_press(GtkWidget *widget, GdkEventKey *event);
static gboolean _window_on_change_state(GtkWidget *widget,
                                        GdkEventWindowState *event,
                                        gpointer user_data);
static void _view_on_zoom_changed(UniImageView *view, VnrWindow *window);

// open / close ---------------------------------------------------------------

static void _window_set_monitor(VnrWindow *window, GList *current);
static void _window_monitor_on_change(VnrWindow *window,
                                      GFile *event_file,
                                      GFile *other_file,
                                      GFileMonitorEvent event_type,
                                      GFileMonitor *monitor);
static gboolean _window_on_idle_reload(VnrWindow *window);
static void _window_action_openfile(VnrWindow *window, GtkWidget *widget);
static void _on_update_preview(GtkFileChooser *file_chooser, gpointer data);
static gboolean _file_size_is_small(char *filename);
static void _window_action_opendir(VnrWindow *window, GtkWidget *widget);
static void _on_file_open_dialog_response(GtkWidget *dialog,
                                          gint response_id,
                                          VnrWindow *window);
static void _window_update_fs_filename_label(VnrWindow *window);
static void _window_update_openwith_menu(VnrWindow *window);
static void _on_openwith(VnrWindow *window, gpointer user_data);

// actions --------------------------------------------------------------------

static gboolean _window_on_sl_timeout(VnrWindow *window);
static void _window_action_reload(VnrWindow *window, GtkWidget *widget);
static void _window_action_resetdir(VnrWindow *window, GtkWidget *widget);
static void _window_action_selectdir(VnrWindow *window, GtkWidget *widget);
static gboolean _window_select_directory(VnrWindow *window);
static GSList* _window_file_chooser(VnrWindow *window,
                                    const gchar *title,
                                    GtkFileChooserAction action,
                                    gboolean multiple);
static void _window_action_copy_to(VnrWindow *window, GtkWidget *widget);
static void _window_action_duplicate(VnrWindow *window, GtkWidget *widget);
static void _window_copy(VnrWindow *window,
                         const char *destdir, gboolean follow);
static void _window_duplicate(VnrWindow *window, gboolean follow);
static gboolean _window_open_item(VnrWindow *window, GList *item);
void _window_autosave(VnrWindow *window);
static void _window_action_move_to(VnrWindow *window, GtkWidget *widget);
static void _window_move_to(VnrWindow *window, const char *destdir);
static void _window_action_rename(VnrWindow *window, GtkWidget *widget);
static void _window_action_delete(VnrWindow *window, GtkWidget *widget);
static gboolean _window_delete_item(VnrWindow *window);
static void _window_hide_cursor(VnrWindow *window);
static void _window_show_cursor(VnrWindow *window);
static void _window_action_properties(VnrWindow *window, GtkWidget *widget);
static void _window_action_preferences(VnrWindow *window, GtkWidget *widget);

// private Actions ------------------------------------------------------------

static void _window_rotate_pixbuf(VnrWindow *window, GdkPixbufRotation angle);
static void _window_flip_pixbuf(VnrWindow *window, gboolean horizontal);
static void _window_action_crop(VnrWindow *window, GtkWidget *widget);
static void _window_action_save_image(VnrWindow *window, GtkWidget *widget);
static void _window_action_zoom_normal(VnrWindow *window, GtkWidget *widget);
static void _window_action_zoom_fit(VnrWindow *window, GtkWidget *widget);

// set wallpaper --------------------------------------------------------------

static void _window_action_set_wallpaper(VnrWindow *window, GtkWidget *widget);

// slideshow ------------------------------------------------------------------

static void _window_action_slideshow(VnrWindow *window);
static void _window_slideshow_stop(VnrWindow *window);
static void _window_slideshow_start(VnrWindow *window);
//static void _window_slideshow_restart(VnrWindow *window);
static void _window_slideshow_allow(VnrWindow *window);
void window_slideshow_deny(VnrWindow *window);

// fullscreen -----------------------------------------------------------------

static void _window_fullscreen(VnrWindow *window);
static void _window_unfullscreen(VnrWindow *window);
static gboolean _on_fullscreen_motion(GtkWidget *widget,
                                      GdkEventMotion *ev,
                                      VnrWindow *window);
static void _window_fullscreen_set_timeout(VnrWindow *window);
static gboolean _on_fullscreen_timeout(VnrWindow *window);
static void _window_fullscreen_unset_timeout(VnrWindow *window);
static gboolean _on_leave_image_area(GtkWidget *widget,
                                     GdkEventCrossing *ev,
                                     VnrWindow *window);

#if 0
static GtkWidget* _window_get_fs_toolitem(VnrWindow *window);
static void _on_fullscreen_leave(GtkButton *button, VnrWindow *window);
static void _on_spin_value_change(GtkSpinButton *spinbutton,
                                  VnrWindow *window);
static void _on_toggle_show_next(GtkToggleButton *togglebutton,
                                 VnrWindow *window);
#endif

typedef enum
{
    WINDOW_ACTION_OPEN = 1,
    WINDOW_ACTION_OPENDIR,
    WINDOW_ACTION_OPENWITH,
    WINDOW_ACTION_SELECTDIR,
    WINDOW_ACTION_COPYTO,
    WINDOW_ACTION_MOVETO,
    WINDOW_ACTION_RENAME,
    WINDOW_ACTION_CROP,
    WINDOW_ACTION_DELETE,
    WINDOW_ACTION_SETWALLPAPER,
    WINDOW_ACTION_PROPERTIES,
    WINDOW_ACTION_PREFERENCES,
    WINDOW_ACTION_DUPLICATE,
    WINDOW_ACTION_SAVE,
    WINDOW_ACTION_RELOAD,
    WINDOW_ACTION_RESETDIR,
    WINDOW_ACTION_FULLSCREEN,
    WINDOW_ACTION_SLIDESHOW,
    WINDOW_ACTION_ZOOM_NORMAAL,
    WINDOW_ACTION_ZOOM_FIT,
    WINDOW_ACTION_ITEM6,
    WINDOW_ACTION_ITEM7,
    WINDOW_ACTION_ITEM8,

} WindowAction;

static EtkActionEntry _window_actions[] =
{
    {WINDOW_ACTION_OPEN,
     "<Actions>/AppWindow/Open", "<Control>O",
     ETK_MENU_ITEM_IMAGE, N_("Open _Image..."),
     N_("Open an Image"),
     "gtk-file",
     G_CALLBACK(_window_action_openfile)},

    {WINDOW_ACTION_OPENDIR,
     "<Actions>/AppWindow/OpenDir", "<Control>F",
     ETK_MENU_ITEM_IMAGE, N_("Open _Folder..."),
     N_("Open a Folder"),
     "gtk-directory",
     G_CALLBACK(_window_action_opendir)},

    {WINDOW_ACTION_OPENWITH,
     "<Actions>/AppWindow/OpenWith", "",
     ETK_MENU_ITEM, N_("Open _With"),
     N_("Open the selected image with a different application"),
     NULL,
     NULL},

    {WINDOW_ACTION_SELECTDIR,
     "<Actions>/AppWindow/SelectDir", "",
     ETK_MENU_ITEM, N_("Select..."),
     N_("Select directory..."),
     NULL,
     G_CALLBACK(_window_action_selectdir)},

    {WINDOW_ACTION_COPYTO,
     "<Actions>/AppWindow/Copy", "F7",
     ETK_MENU_ITEM, N_("Copy"),
     N_("Copy the current file"),
     NULL,
     G_CALLBACK(_window_action_copy_to)},

    {WINDOW_ACTION_MOVETO,
     "<Actions>/AppWindow/Move", "F8",
     ETK_MENU_ITEM, N_("Move"),
     N_("Move the current file"),
     NULL,
     G_CALLBACK(_window_action_move_to)},

    {WINDOW_ACTION_RENAME,
     "<Actions>/AppWindow/Rebame", "F2",
     ETK_MENU_ITEM, N_("Rename"),
     N_("Rename the current file"),
     NULL,
     G_CALLBACK(_window_action_rename)},

    {WINDOW_ACTION_CROP,
     "<Actions>/AppWindow/Crop", "F4",
     ETK_MENU_ITEM, N_("Crop"),
     N_("Crop image"),
     NULL,
     G_CALLBACK(_window_action_crop)},

    {WINDOW_ACTION_DELETE,
     "<Actions>/AppWindow/Delete", "Delete",
     ETK_MENU_ITEM, N_("_Delete"),
     N_("Delete the current file"),
     NULL,
     G_CALLBACK(_window_action_delete)},

    {WINDOW_ACTION_SETWALLPAPER,
     "<Actions>/AppWindow/SetWallpaper", "F10",
     ETK_MENU_ITEM, N_("_Wallpaper"),
     N_("Set image as wallpaper"),
     NULL,
     G_CALLBACK(_window_action_set_wallpaper)},

    {WINDOW_ACTION_PROPERTIES,
     "<Actions>/AppWindow/Properties", "<Control>Return",
     ETK_MENU_ITEM, N_("_Properties..."),
     N_("Show information about the current file"),
     NULL,
     G_CALLBACK(_window_action_properties)},

    {WINDOW_ACTION_PREFERENCES,
     "<Actions>/AppWindow/Preferences", "",
     ETK_MENU_ITEM, N_("_Preferences..."),
     N_("User preferences for ImgView"),
     NULL,
     G_CALLBACK(_window_action_preferences)},

    {WINDOW_ACTION_DUPLICATE,
     "<Actions>/AppWindow/Duplicate", "<Control>D",
     0, NULL,
     NULL,
     NULL,
     G_CALLBACK(_window_action_duplicate)},

    {WINDOW_ACTION_SAVE,
     "<Actions>/AppWindow/Save", "<Control>S",
     0, NULL,
     NULL,
     NULL,
     G_CALLBACK(_window_action_save_image)},

    {WINDOW_ACTION_RELOAD,
     "<Actions>/AppWindow/Reload", "F5",
     0, NULL,
     NULL,
     NULL,
     G_CALLBACK(_window_action_reload)},

    {WINDOW_ACTION_RESETDIR,
     "<Actions>/AppWindow/ResetDir", "F6",
     0, NULL,
     NULL,
     NULL,
     G_CALLBACK(_window_action_resetdir)},

    {WINDOW_ACTION_FULLSCREEN,
     "<Actions>/AppWindow/Fullscreen", "F11",
     0, NULL,
     NULL,
     NULL,
     G_CALLBACK(window_fullscreen_toggle)},

    {WINDOW_ACTION_SLIDESHOW,
     "<Actions>/AppWindow/Slideshow", "F12",
     0, NULL,
     NULL,
     NULL,
     G_CALLBACK(_window_action_slideshow)},

    {WINDOW_ACTION_ZOOM_FIT,
     "<Actions>/AppWindow/ZoomFit", "<Control>W",
     0, NULL,
     NULL,
     NULL,
     G_CALLBACK(_window_action_zoom_fit)},

    {WINDOW_ACTION_ZOOM_NORMAAL,
     "<Actions>/AppWindow/ZoomNormal", "<Control>N",
     0, NULL,
     NULL,
     NULL,
     G_CALLBACK(_window_action_zoom_normal)},

    {0},
};


// creation / destruction -----------------------------------------------------

VnrWindow* window_new()
{
    return (VnrWindow*) g_object_new(VNR_TYPE_WINDOW, NULL);
}

static void window_class_init(VnrWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = window_dispose;
    object_class->finalize = window_finalize;

    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->key_press_event = _window_on_key_press;
    widget_class->drag_data_received = _window_drag_data_received;
    
    etk_actions_translate(_window_actions);
}

static void window_init(VnrWindow *window)
{
    window->mode = WINDOW_MODE_NORMAL;
    window->accel_group = etk_actions_init(GTK_WINDOW(window),
                                           _window_actions);

    window->prefs = (VnrPrefs*) vnr_prefs_new(GTK_WIDGET(window));

    window->sl_timeout = 5;
    window->can_slideshow = TRUE;

    gtk_window_set_title((GtkWindow*) window, "ImgView");
    gtk_window_set_default_icon_name("viewnior");

    // content
    window->layout_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), window->layout_box);
    gtk_widget_show(window->layout_box);

    window->msg_area = vnr_message_area_new();
    VNR_MESSAGE_AREA(window->msg_area)->vnrwindow = window;
    gtk_box_pack_start(GTK_BOX(window->layout_box),
                       window->msg_area, FALSE, FALSE, 0);
    gtk_widget_show(GTK_WIDGET(window->msg_area));

    window->scroll_view = uni_scroll_win_new();
    gtk_box_pack_end(GTK_BOX(window->layout_box),
                     window->scroll_view, TRUE, TRUE, 0);

    window->view = uni_scroll_win_get_view(
                                UNI_SCROLL_WIN(window->scroll_view));

    gtk_widget_show_all(GTK_WIDGET(window->scroll_view));

    // popup menu
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *item = NULL;

    window->popup_menu = menu;
    gtk_menu_set_accel_group(GTK_MENU(menu), window->accel_group);

    etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                  WINDOW_ACTION_OPEN,
                                  _window_actions,
                                  G_OBJECT(window));

    etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                  WINDOW_ACTION_OPENDIR,
                                  _window_actions,
                                  G_OBJECT(window));

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_OPENWITH,
                                         _window_actions,
                                         G_OBJECT(window));
    window->openwith_item = item;

    etk_menu_append_separator(GTK_MENU_SHELL(menu));

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_SELECTDIR,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_COPYTO,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_MOVETO,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_RENAME,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_CROP,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_DELETE,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    etk_menu_append_separator(GTK_MENU_SHELL(menu));

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_SETWALLPAPER,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_PROPERTIES,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    etk_menu_append_separator(GTK_MENU_SHELL(menu));

    etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                  WINDOW_ACTION_PREFERENCES,
                                  _window_actions,
                                  G_OBJECT(window));

    gtk_widget_show_all(menu);
    gtk_widget_hide(window->openwith_item);

    etk_widget_list_set_sensitive(window->list_image, false);

    // Initialize slideshow timeout
    window->sl_timeout = window->prefs->sl_timeout;

    // Care for Properties dialog
    window->props_dlg = vnr_properties_dialog_new(window);

    window_preferences_apply(window);
    uni_scroll_win_set_show_scrollbar(UNI_SCROLL_WIN(window->scroll_view),
                                      window->prefs->show_scrollbar);

    gtk_widget_grab_focus(window->view);
    _window_set_drag(window);

    g_signal_connect_swapped(G_OBJECT(window), "realize",
                     G_CALLBACK(_window_on_realize), window);

    g_signal_connect_swapped(G_OBJECT(window), "delete-event",
                     G_CALLBACK(_window_on_delete), window);

    g_signal_connect(G_OBJECT(window), "window-state-event",
                     G_CALLBACK(_window_on_change_state), NULL);

    g_signal_connect(G_OBJECT(window->view), "zoom_changed",
                     G_CALLBACK(_view_on_zoom_changed), window);

    g_signal_connect(G_OBJECT(window->view), "drag-data-get",
                     G_CALLBACK(_view_on_drag_begin), window);
}

static void _window_on_realize(VnrWindow *window, gpointer user_data)
{
    GtkWidget *widget = GTK_WIDGET(window);

    g_signal_handlers_disconnect_by_func(widget,
                                         _window_on_realize,
                                         user_data);

    if (vnr_message_area_is_critical(VNR_MESSAGE_AREA(window->msg_area)))
        return;

    VnrPrefs *prefs = window->prefs;

    gtk_window_set_default_size(GTK_WINDOW(window),
                                prefs->window_width,
                                prefs->window_height);

    if (prefs->start_maximized)
    {
        if (window_load_file(window, FALSE))
            _window_set_monitor(window, window->filelist);
    }
    else
    {
        GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(window));
        GdkDisplay *display = gdk_screen_get_display(screen);
        GdkMonitor *monitor = gdk_display_get_monitor_at_window(
                                        display,
                                        gtk_widget_get_window(widget));

        GdkRectangle geometry;
        gdk_monitor_get_geometry(monitor, &geometry);

        window->max_width = geometry.width * 0.9 - 100;
        window->max_height = geometry.height * 0.9 - 100;

        //printf("w = %d, h = %d\n", geometry.width, geometry.height);

        gboolean fit_to_screen = false;

        if (window_load_file(window, fit_to_screen))
            _window_set_monitor(window, window->filelist);
    }

    VnrFile *current = window_get_current_file(window);

    if (!current)
        return;

    if (prefs->start_fullscreen)
    {
        _window_fullscreen(window);
    }
    else if (prefs->start_slideshow)
    {
        _window_fullscreen(window);
        window->mode = WINDOW_MODE_NORMAL;
        _window_slideshow_allow(window);
        _window_slideshow_start(window);
    }
}

static gboolean _window_on_delete(VnrWindow *window, GdkEvent *event,
                                  gpointer user_data)
{
    (void) event;
    (void) user_data;

    GtkWidget *widget = GTK_WIDGET(window);
    VnrPrefs *prefs = window->prefs;

    if (gtk_widget_get_visible(widget))
    {
        GdkWindowState state = gdk_window_get_state(
                                    gtk_widget_get_window(widget));

        prefs->start_maximized =
            ((state & (GDK_WINDOW_STATE_MAXIMIZED
                       | GDK_WINDOW_STATE_FULLSCREEN)) != 0);

        if (!prefs->start_maximized)
        {
            gtk_window_get_size(GTK_WINDOW(window),
                                &prefs->window_width,
                                &prefs->window_height);
        }
    }

    vnr_prefs_save(window->prefs);

    gtk_main_quit();

    return false;
}

static void window_dispose(GObject *object)
{
    VnrWindow *window = VNR_WINDOW(object);

    _window_set_monitor(window, NULL);
    window->accel_group = etk_actions_dispose(GTK_WINDOW(window),
                                              window->accel_group);
    window->list_image = etk_widget_list_free(window->list_image);

    G_OBJECT_CLASS(window_parent_class)->dispose(object);
}

static void window_finalize(GObject *object)
{
    VnrWindow *window = VNR_WINDOW(object);

    g_free(window->destdir);
    vnr_list_free(window->filelist);
    window_list_set_current(window, NULL);

    G_OBJECT_CLASS(window_parent_class)->finalize(object);
}

// ----------------------------------------------------------------------------

void window_preferences_apply(VnrWindow *window)
{
    if (window->prefs->dark_background)
    {
        GdkRGBA color;
        gdk_rgba_parse(&color, DARK_BACKGROUND_COLOR);
        _window_override_background_color(window, &color);
    }

    if (window->prefs->smooth_images
        && UNI_IMAGE_VIEW(window->view)->interp != GDK_INTERP_BILINEAR)
    {
        UNI_IMAGE_VIEW(window->view)->interp = GDK_INTERP_BILINEAR;
        gtk_widget_queue_draw(window->view);
    }
    else if (!window->prefs->smooth_images
             && UNI_IMAGE_VIEW(window->view)->interp != GDK_INTERP_NEAREST)
    {
        UNI_IMAGE_VIEW(window->view)->interp = GDK_INTERP_NEAREST;
        gtk_widget_queue_draw(window->view);
    }

    if (window->fs_toolitem)
    {
        gint val = gtk_spin_button_get_value_as_int(
                        GTK_SPIN_BUTTON(window->sl_timeout_widget));

        if (val != window->prefs->sl_timeout)
        {
            gtk_spin_button_set_value(
                        GTK_SPIN_BUTTON(window->sl_timeout_widget),
                        (gdouble) window->prefs->sl_timeout);
        }
    }
}

static void _window_override_background_color(VnrWindow *window,
                                              GdkRGBA *color)
{
    // https://stackoverflow.com/questions/36520637/

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_widget_override_background_color(window->view,
                                         GTK_STATE_FLAG_NORMAL,
                                         color);
    G_GNUC_END_IGNORE_DEPRECATIONS
}


// dnd ------------------------------------------------------------------------

static void _window_set_drag(VnrWindow *window)
{
    gtk_drag_dest_set(GTK_WIDGET(window),
                      GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP,
                      NULL, 0,
                      GDK_ACTION_COPY | GDK_ACTION_ASK);
    gtk_drag_dest_add_uri_targets(GTK_WIDGET(window));
}

static void _view_on_drag_begin(GtkWidget *widget,
                                  GdkDragContext *drag_context,
                                  GtkSelectionData *data,
                                  guint info,
                                  guint time,
                                  gpointer user_data)
{
    (void) widget;
    (void) drag_context;
    (void) info;
    (void) time;

    VnrFile *current = window_get_current_file(VNR_WINDOW(user_data));
    if (!current)
        return;

    gchar *uris[2];

    uris[0] = g_filename_to_uri((gchar*) current->path, NULL, NULL);
    uris[1] = NULL;

    gtk_selection_data_set_uris(data, uris);

    g_free(uris[0]);
}

static void _window_drag_data_received(GtkWidget *widget,
                                       GdkDragContext *context,
                                       gint x, gint y,
                                       GtkSelectionData *selection_data,
                                       guint info, guint time)
{
    VnrWindow *window = VNR_WINDOW(widget);

    GdkAtom target = gtk_selection_data_get_target(selection_data);

    if (!gtk_targets_include_uri(&target, 1))
        return;

    if (gtk_drag_get_source_widget(context))
        return;

    GdkDragAction suggested_action;
    suggested_action = gdk_drag_context_get_suggested_action(context);

    if (suggested_action == GDK_ACTION_COPY
        || suggested_action == GDK_ACTION_ASK)
    {
        const guchar *data = gtk_selection_data_get_data(selection_data);

        GSList *uri_list = NULL;
        uri_list = vnr_tools_parse_uri_string_list_to_file_list((gchar *)data);

        if (!uri_list)
        {
            window_close_file(window);

            //gtk_action_group_set_sensitive(
            //      VNR_WINDOW(widget)->actions_collection, FALSE);

            window_slideshow_deny(window);
            vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                                  TRUE,
                                  _("The given locations contain no images."),
                                  TRUE);
            return;
        }

        window_open_list(VNR_WINDOW(widget), uri_list);
    }
}


// ----------------------------------------------------------------------------

static gint _window_on_key_press(GtkWidget *widget, GdkEventKey *event)
{
    // Modified version of eog's eog_window_key_press

    VnrWindow *window = VNR_WINDOW(widget);
    gint result = FALSE;

    GtkWidget *toolbar_focus_child = NULL;
    GtkWidget *msg_area_focus_child = gtk_container_get_focus_child(
                                            GTK_CONTAINER(window->msg_area));
    switch (event->keyval)
    {
    case GDK_KEY_Left:
        if (!uni_scroll_win_image_fits(UNI_SCROLL_WIN(window->scroll_view)))
            break; // let scrollview handle the key

        if (toolbar_focus_child != NULL || msg_area_focus_child != NULL)
            break;

        if (event->state & GDK_CONTROL_MASK)
        {
            _window_rotate_pixbuf(window, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
            result = TRUE;
            break;
        }

        window_prev(window);
        result = TRUE;
        break;

    case GDK_KEY_Right:
        if (!uni_scroll_win_image_fits(UNI_SCROLL_WIN(window->scroll_view)))
            break; // let scrollview handle the key

        if (toolbar_focus_child != NULL || msg_area_focus_child != NULL)
            break;

        if (event->state & GDK_CONTROL_MASK)
        {
            _window_rotate_pixbuf(window, GDK_PIXBUF_ROTATE_CLOCKWISE);
            result = TRUE;
            break;
        }

        window_next(window, TRUE);
        result = TRUE;
        break;

    case GDK_KEY_Up:
        if (event->state & GDK_CONTROL_MASK)
        {
            _window_flip_pixbuf(window, TRUE);
            result = TRUE;
        }
        break;

    case GDK_KEY_Down:
        if (event->state & GDK_CONTROL_MASK)
        {
            _window_flip_pixbuf(window, FALSE);
            result = TRUE;
        }
        break;

    case GDK_KEY_Escape:
    case 'q':
        if (window->mode != WINDOW_MODE_NORMAL)
            _window_unfullscreen(window);
        else
            gtk_main_quit();
        break;

    case GDK_KEY_space:
        if (toolbar_focus_child != NULL || msg_area_focus_child != NULL)
            break;
        window_next(window, TRUE);
        result = TRUE;
        break;

    case GDK_KEY_BackSpace:
        window_prev(window);
        result = TRUE;
        break;

    case GDK_KEY_Home:
        window_first(window);
        result = TRUE;
        break;

    case GDK_KEY_End:
        window_last(window);
        result = TRUE;
        break;

    case 'h':
        _window_flip_pixbuf(window, TRUE);
        break;

    case 'v':
        _window_flip_pixbuf(window, FALSE);
        break;
    }

    if (result == FALSE
        && GTK_WIDGET_CLASS(window_parent_class)->key_press_event)
    {
        result =
        GTK_WIDGET_CLASS(window_parent_class)->key_press_event(widget, event);
    }

    return result;
}

static gboolean _window_on_change_state(GtkWidget *widget,
                                        GdkEventWindowState *event,
                                        gpointer user_data)
{
    (void) user_data;

    if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
    {
        // Detect maximized state only
        if (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED)
        {
            VNR_WINDOW(widget)->prefs->start_maximized = TRUE;
        }
        else
        {
            VNR_WINDOW(widget)->prefs->start_maximized = FALSE;
        }

        vnr_prefs_save(VNR_WINDOW(widget)->prefs);
    }

    return TRUE;
}

static void _view_on_zoom_changed(UniImageView *view, VnrWindow *window)
{
    /* Change the info, only if there is an image
     * (vnr_window_close isn't called on the current image) */

    VnrFile *current = window_get_current_file(window);
    if (!current)
        return;

    gint total = 0;
    gint position = vnr_list_get_position(window->filelist, &total);

    char *buf = g_strdup_printf("%s%s - %i/%i - %ix%i - %i%%",
                                (window->modifications) ? "*" : "",
                                current->display_name,
                                position,
                                total,
                                window->current_image_width,
                                window->current_image_height,
                                (int) (view->zoom * 100.));

    gtk_window_set_title(GTK_WINDOW(window), buf);

    //gint context_id = gtk_statusbar_get_context_id(
    //            GTK_STATUSBAR(window->statusbar), "statusbar");
    //gtk_statusbar_pop(GTK_STATUSBAR(window->statusbar),
    //                  GPOINTER_TO_INT(context_id));
    //gtk_statusbar_push(GTK_STATUSBAR(window->statusbar),
    //                   GPOINTER_TO_INT(context_id), buf);

    g_free(buf);
}


// file list ------------------------------------------------------------------

void window_list_set(VnrWindow *window, GList *list)
{
    if (list != window->filelist)
    {
        vnr_list_free(window->filelist);
        window_list_set_current(window, NULL);
    }

    if (list && g_list_length(g_list_first(list)) > 1)
    {
        //gtk_action_group_set_sensitive(window->actions_collection, true);

        _window_slideshow_allow(window);
    }
    else
    {
        //gtk_action_group_set_sensitive(window->actions_collection, false);

        window_slideshow_deny(window);
    }

    window_list_set_current(window, list);
}

VnrFile* window_get_current_file(VnrWindow *window)
{
    g_return_val_if_fail(window != NULL, NULL);

    if (!window->filelist)
        return NULL;

    return VNR_FILE(window->filelist->data);
}

void window_list_set_current(VnrWindow *window, GList *list)
{
    g_return_if_fail(window != NULL);

    window->filelist = list;
}

static void _window_set_monitor(VnrWindow *window, GList *current)
{
    g_return_if_fail(window != NULL);

    if (window->monitor)
    {
        g_object_unref(window->monitor);
        window->monitor = NULL;
    }

    if (!current)
        return;

    VnrFile *vnrfile = VNR_FILE(current->data);
    GFile *gfile = g_file_new_for_path(vnrfile->path);

    if (!gfile)
        return;

    GFileMonitor *monitor = g_file_monitor(gfile,
                                           G_FILE_MONITOR_WATCH_MOVES,
                                           NULL, NULL);
    if (!monitor)
    {
        g_object_unref(gfile);
        return;
    }

    window->monitor = monitor;
    g_signal_connect_swapped(monitor, "changed",
                             G_CALLBACK(_window_monitor_on_change), window);

    g_object_unref(gfile);
}

static void _window_monitor_on_change(VnrWindow *window,
                                      GFile *event_file,
                                      GFile *other_file,
                                      GFileMonitorEvent event_type,
                                      GFileMonitor *monitor)
{
    (void) monitor;
    (void) other_file;

    if (!window_get_current_file(window))
        return;

    switch (event_type)
    {
    case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
        char *path = g_file_get_path(event_file);
        printf("_window_monitor_on_change: %s\n", path);
        g_free(path);

        if (!window->need_reload)
        {
            window->need_reload = true;
            g_idle_add((GSourceFunc) _window_on_idle_reload, window);
        }
        break;

    default:
        break;
    }
}

static gboolean _window_on_idle_reload(VnrWindow *window)
{
    g_return_val_if_fail(window != NULL, G_SOURCE_REMOVE);

    window->need_reload = false;

    if (window->no_reload)
    {
        window->no_reload = false;

        return G_SOURCE_REMOVE;
    }

    printf("_window_on_idle_reload: reload\n");

    window_load_file(window, FALSE);

    return G_SOURCE_REMOVE;
}


// open / close ---------------------------------------------------------------

static void _window_action_openfile(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
                                _("Open Image"),
                                GTK_WINDOW(window),
                                GTK_FILE_CHOOSER_ACTION_OPEN,
                                "gtk-cancel", GTK_RESPONSE_CANCEL,
                                "gtk-open", GTK_RESPONSE_ACCEPT,
                                NULL);

    GtkFileFilter *img_filter = gtk_file_filter_new();
    g_assert(img_filter != NULL);
    gtk_file_filter_add_pixbuf_formats(img_filter);
    gtk_file_filter_add_mime_type(img_filter, "image/vnd.microsoft.icon");
    gtk_file_filter_set_name(img_filter, _("All Images"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), img_filter);

    GtkFileFilter *all_filter = gtk_file_filter_new();
    g_assert(all_filter != NULL);
    gtk_file_filter_add_pattern(all_filter, "*");
    gtk_file_filter_set_name(all_filter, _("All Files"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);

    gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), img_filter);

    GtkWidget *preview = gtk_image_new();
    gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);
    g_signal_connect(GTK_FILE_CHOOSER(dialog), "update-preview",
                     G_CALLBACK(_on_update_preview), preview);

    VnrFile *current = window_get_current_file(window);
    if (current)
    {
        gchar *dirname = g_path_get_dirname(current->path);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dirname);
        g_free(dirname);
    }

    g_signal_connect(dialog, "response",
                     G_CALLBACK(_on_file_open_dialog_response), window);

    gtk_widget_show_all(GTK_WIDGET(dialog));

    // This only works when here.
    gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog),
                                     window->prefs->show_hidden);
}

static void _on_update_preview(GtkFileChooser *file_chooser, gpointer data)
{
    char *filename = gtk_file_chooser_get_preview_filename(file_chooser);
    gboolean has_preview = FALSE;

    if (_file_size_is_small(filename))
    {
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(filename,
                                                             256, 256, NULL);
        has_preview = (pixbuf != NULL);

        GtkWidget *preview = GTK_WIDGET(data);

        gtk_image_set_from_pixbuf(GTK_IMAGE(preview), pixbuf);
        if (pixbuf)
            g_object_unref(pixbuf);
    }

    gtk_file_chooser_set_preview_widget_active(file_chooser, has_preview);

    g_free(filename);
}

static gboolean _file_size_is_small(char *filename)
{
    struct stat st;

    if (!filename || stat(filename, &st) != 0)
        return false;

    int four_mb = 4 * 1024 * 1024;

    return (st.st_size < four_mb);
}

static void _window_action_opendir(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
                                _("Open Folder"),
                                GTK_WINDOW(window),
                                GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                "gtk-cancel", GTK_RESPONSE_CANCEL,
                                "gtk-open", GTK_RESPONSE_ACCEPT,
                                NULL);

    gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    VnrFile *current = window_get_current_file(window);
    if (current)
    {
        gchar *dirname = g_path_get_dirname(current->path);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dirname);
        g_free(dirname);
    }

    g_signal_connect(dialog, "response",
                     G_CALLBACK(_on_file_open_dialog_response),
                     window);

    gtk_widget_show_all(GTK_WIDGET(dialog));

    // This only works when here
    gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog),
                                     window->prefs->show_hidden);
}

static void _on_file_open_dialog_response(GtkWidget *dialog,
                                          gint response_id,
                                          VnrWindow *window)
{
    if (response_id == GTK_RESPONSE_ACCEPT)
    {
        GSList *uri_list = gtk_file_chooser_get_filenames(
                                            GTK_FILE_CHOOSER(dialog));

        g_return_if_fail(uri_list != NULL);

        window_open_list(window, uri_list);

        g_slist_free_full(uri_list, g_free);
    }

    gtk_widget_destroy(dialog);
}

void window_open_list(VnrWindow *window, GSList *uri_list)
{
    if (!uri_list)
        return;

    _window_set_monitor(window, NULL);

    GList *file_list = NULL;
    GError *error = NULL;

    if (g_slist_length(uri_list) == 1)
    {
        file_list = vnr_list_new_for_path(uri_list->data,
                                          window->prefs->show_hidden,
                                          &error);
    }
    else
    {
        file_list = vnr_list_new_for_list(uri_list,
                                          window->prefs->show_hidden,
                                          &error);
    }

    if (error)
    {
        window_close_file(window);
        //gtk_action_group_set_sensitive(window->actions_collection, FALSE);

        window_list_set(window, file_list);
        //window_slideshow_deny(window);

        if (!file_list)
        {
            vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                                  TRUE,
                                  _("The given locations contain no images."),
                                  TRUE);
        }
        else
        {
            vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                                  TRUE,
                                  error->message,
                                  TRUE);
        }

        // free error

        return;
    }

    window_list_set(window, file_list);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    window_close_file(window);

    if (window_load_file(window, FALSE))
        _window_set_monitor(window, file_list);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);
}

gboolean window_load_file(VnrWindow *window, gboolean fit_to_screen)
{
    g_return_val_if_fail(window != NULL, false);

    VnrFile *current = window_get_current_file(window);
    if (!current)
        return false;

    _window_update_fs_filename_label(window);

    GError *error = NULL;
    GdkPixbufAnimation *pixbuf =
            gdk_pixbuf_animation_new_from_file(current->path, &error);

    if (error != NULL)
    {
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE,
                              error->message,
                              TRUE);

        if (gtk_widget_get_visible(window->props_dlg))
            vnr_properties_dialog_clear(
                        VNR_PROPERTIES_DIALOG(window->props_dlg));

        return FALSE;
    }

    if (vnr_message_area_is_visible(VNR_MESSAGE_AREA(window->msg_area)))
    {
        vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));
    }

    etk_widget_list_set_sensitive(window->list_image, true);
    //gtk_action_group_set_sensitive(window->actions_image, TRUE);
    //gtk_action_group_set_sensitive(window->action_wallpaper, TRUE);

    GdkPixbufFormat *format = gdk_pixbuf_get_file_info(current->path,
                                                       NULL, NULL);

    g_free(window->writable_format_name);

    if (gdk_pixbuf_format_is_writable(format))
        window->writable_format_name = gdk_pixbuf_format_get_name(format);
    else
        window->writable_format_name = NULL;

    vnr_tools_apply_embedded_orientation(&pixbuf);

    window->current_image_width = gdk_pixbuf_animation_get_width(pixbuf);
    window->current_image_height = gdk_pixbuf_animation_get_height(pixbuf);

    window->modifications = 0;

    if (fit_to_screen)
    {
        // Width and Height of the pixbuf

        gint img_w = window->current_image_width;
        gint img_h = window->current_image_height;

        vnr_tools_fit_to_size(&img_w, &img_h,
                              window->max_width, window->max_height);

        gtk_window_resize(GTK_WINDOW(window),
                          img_w,
                          img_h /*+ _window_get_top_widgets_height(window)*/);
    }

    UniFittingMode last_fit_mode = UNI_IMAGE_VIEW(window->view)->fitting;

    // returns true if the image is static
    window->can_edit = uni_anim_view_set_anim(UNI_ANIM_VIEW(window->view),
                                              pixbuf);

    if (window->mode != WINDOW_MODE_NORMAL && window->prefs->fit_on_fullscreen)
    {
        uni_image_view_set_zoom_mode(UNI_IMAGE_VIEW(window->view),
                                     VNR_PREFS_ZOOM_FIT);
    }
    else if (window->prefs->zoom == VNR_PREFS_ZOOM_LAST_USED)
    {
        uni_image_view_set_fitting(UNI_IMAGE_VIEW(window->view),
                                   last_fit_mode);
        _view_on_zoom_changed(UNI_IMAGE_VIEW(window->view), window);
    }
    else
    {
        uni_image_view_set_zoom_mode(UNI_IMAGE_VIEW(window->view),
                                     window->prefs->zoom);
    }

    if (gtk_widget_get_visible(window->props_dlg))
        vnr_properties_dialog_update(VNR_PROPERTIES_DIALOG(window->props_dlg));

    _window_update_openwith_menu(window);

    g_object_unref(pixbuf);

    return TRUE;
}

static void _window_update_fs_filename_label(VnrWindow *window)
{
    VnrFile *current = window_get_current_file(window);

    if (!current || window->mode == WINDOW_MODE_NORMAL)
        return;

    gint total = 0;
    gint position = vnr_list_get_position(window->filelist, &total);

    char *buf = g_strdup_printf("%s - %i/%i",
                                current->display_name,
                                position,
                                total);

    if (window->fs_toolitem)
        gtk_label_set_text(GTK_LABEL(window->fs_filename_label), buf);

    g_free(buf);
}

void window_close_file(VnrWindow *window)
{
    _window_set_monitor(window, NULL);

    gtk_window_set_title(GTK_WINDOW(window), "ImgView");
    uni_anim_view_set_anim(UNI_ANIM_VIEW(window->view), NULL);

    //gtk_action_group_set_sensitive(window->actions_static_image, FALSE);
    _window_update_openwith_menu(window);
    window->can_edit = false;

    etk_widget_list_set_sensitive(window->list_image, false);
    //gtk_action_group_set_sensitive(window->actions_image, FALSE);
    //gtk_action_group_set_sensitive(window->action_wallpaper, FALSE);
}

static void _window_update_openwith_menu(VnrWindow *window)
{
    // Modified version of eog's eog_window_update_openwith_menu

    gtk_widget_hide(window->openwith_item);

    VnrFile *current = window_get_current_file(window);
    if (!current)
        return;

    GFile *file = g_file_new_for_path((gchar*) current->path);
    GFileInfo *file_info = g_file_query_info(
                            file,
                            G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                            0, NULL, NULL);
    g_object_unref(file);

    if (!file_info)
        return;

    const gchar *mime_type = g_file_info_get_content_type(file_info);

    if (mime_type == NULL)
    {
        g_object_unref(file_info);
        return;
    }

    GList *apps = g_app_info_get_all_for_type(mime_type);
    g_object_unref(file_info);

    if (!apps)
        return;

    GtkWidget *menu = gtk_menu_new();
    gint count = 0;

    for (GList *iter = apps; iter; iter = iter->next)
    {
        GAppInfo *app = iter->data;

        // do not include imgview itself
        if (g_ascii_strcasecmp(g_app_info_get_executable(app),
                               g_get_prgname()) == 0)
        {
            g_object_unref(app);
            continue;
        }

        gchar *label = g_strdup(g_app_info_get_name(app));
        gchar *tooltip = g_strdup_printf(
                                _("Use \"%s\" to open the selected image"),
                                g_app_info_get_name(app));

        GtkWidget *item = etk_menu_item_new(GTK_MENU_SHELL(menu),
                                            label,
                                            tooltip,
                                            NULL,
                                            G_CALLBACK(_on_openwith),
                                            G_OBJECT(window));

        //printf("add item : %s\n", label);

        g_free(label);
        g_free(tooltip);

        g_object_set_data_full(G_OBJECT(item),
                               "app",
                               app,
                               (GDestroyNotify) g_object_unref);

        ++count;
    }

    g_list_free(apps);

    // see launcher.c: 816
    // launcher_append_open_section
    // gtk_menu_item_set_submenu
    // Sets or replaces the menu item’s submenu,
    // or removes it when a NULL submenu is passed.

    if (!count)
    {
        gtk_widget_destroy(menu);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(window->openwith_item), NULL);

        return;
    }

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(window->openwith_item), menu);
    gtk_widget_show_all(window->openwith_item);
}

static void _on_openwith(VnrWindow *window, gpointer user_data)
{
    g_return_if_fail(VNR_IS_WINDOW(window) || !GTK_IS_WIDGET(user_data));

    VnrFile *current = window_get_current_file(window);
    if (!current)
        return;

    GFile *file = g_file_new_for_path((gchar*) current->path);
    GList *files = g_list_append(NULL, file);

    GtkWidget *item = GTK_WIDGET(user_data);
    GAppInfo *app = g_object_get_data(G_OBJECT(item), "app");
    if (app)
        g_app_info_launch(app, files, NULL, NULL);

    g_object_unref(file);
    g_list_free(files);
}


// ----------------------------------------------------------------------------

void window_action_prev(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;

    window_prev(window);
}

void window_action_next(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;

    window_next(window, true);
}

gboolean window_prev(VnrWindow *window)
{
    // don't reload if there's less than 2 images
    if (g_list_length(g_list_first(window->filelist)) < 2)
        return FALSE;

    if (window->mode == WINDOW_MODE_SLIDESHOW)
        g_source_remove(window->sl_source_id);

    _window_autosave(window);

    GList *prev = g_list_previous(window->filelist);
    if (!prev)
        prev = g_list_last(window->filelist);

    _window_open_item(window, prev);

    if (window->mode == WINDOW_MODE_SLIDESHOW)
    {
        window->sl_source_id =
            g_timeout_add_seconds(window->sl_timeout,
                                  (GSourceFunc) _window_on_sl_timeout,
                                  window);
    }

    return TRUE;
}

void _window_autosave(VnrWindow *window)
{
    if (window->modifications != 0
        && window->prefs->modify_behavior == VNR_PREFS_MODIFY_AUTOSAVE)
    {
        _window_action_save_image(window, NULL);
        window->modifications = 0;
    }
}

static gboolean _window_open_item(VnrWindow *window, GList *item)
{
    _window_set_monitor(window, NULL);

    window_list_set_current(window, item);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    if (window_load_file(window, FALSE))
        _window_set_monitor(window, item);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);

    return true;
}

gboolean window_next(VnrWindow *window, gboolean reset_timer)
{
    // don't reload if there's less than 2 images
    if (g_list_length(g_list_first(window->filelist)) < 2)
        return FALSE;

    if (reset_timer && window->mode == WINDOW_MODE_SLIDESHOW)
        g_source_remove(window->sl_source_id);

    _window_autosave(window);

    GList *next = g_list_next(window->filelist);
    if (!next)
        next = g_list_first(window->filelist);

    _window_open_item(window, next);

    if (reset_timer && window->mode == WINDOW_MODE_SLIDESHOW)
    {
        window->sl_source_id =
            g_timeout_add_seconds(window->sl_timeout,
                                  (GSourceFunc) _window_on_sl_timeout,
                                  window);
    }

    return TRUE;
}

static gboolean _window_on_sl_timeout(VnrWindow *window)
{
    if (g_list_length(g_list_first(window->filelist)) < 2)
        return G_SOURCE_REMOVE;
    else
        window_next(window, FALSE);

    window->sl_source_id =
        g_timeout_add_seconds(window->sl_timeout,
                              (GSourceFunc) _window_on_sl_timeout,
                              window);

    return G_SOURCE_REMOVE;
}

gboolean window_first(VnrWindow *window)
{
    _window_autosave(window);

    GList *first = g_list_first(window->filelist);

    if (vnr_message_area_is_critical(VNR_MESSAGE_AREA(window->msg_area)))
        vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));

    _window_open_item(window, first);

    return true;
}

gboolean window_last(VnrWindow *window)
{
    _window_autosave(window);

    GList *last = g_list_last(window->filelist);

    if (vnr_message_area_is_critical(VNR_MESSAGE_AREA(window->msg_area)))
        vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));

    _window_open_item(window, last);

    return true;
}

static void _window_action_reload(VnrWindow *window, GtkWidget *widget)
{
    g_return_if_fail(window != NULL);
    (void) widget;

    _window_set_monitor(window, NULL);

    VnrFile *vnrfile = window_get_current_file(window);
    if (!vnrfile)
        return;

    GList *list = vnr_list_new_for_file(vnrfile->path,
                                        window->prefs->show_hidden,
                                        true);
    window_list_set(window, list);

    if (window_load_file(window, FALSE))
        _window_set_monitor(window, window->filelist);
}

static void _window_action_resetdir(VnrWindow *window, GtkWidget *widget)
{
    g_return_if_fail(window != NULL);
    (void) widget;

    if (window_get_current_file(window) == NULL
        || window->mode != WINDOW_MODE_NORMAL)
        return;

    g_free(window->destdir);

    window->destdir = NULL;
}

static void _window_action_selectdir(VnrWindow *window, GtkWidget *widget)
{
    g_return_if_fail(window != NULL);
    (void) widget;

    if (window_get_current_file(window) == NULL
        || window->mode != WINDOW_MODE_NORMAL)
        return;

    _window_select_directory(window);
}

static gboolean _window_select_directory(VnrWindow *window)
{
    g_return_val_if_fail(window != NULL, false);

    GSList *list = _window_file_chooser(window,
                                        "bla",
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                        false);
    if (!list)
        return false;

    g_free(window->destdir);

    window->destdir = g_strdup((const gchar*) list->data);

    g_slist_free_full(list, g_free);

    return true;
}

static GSList* _window_file_chooser(VnrWindow *window,
                                    const gchar *title,
                                    GtkFileChooserAction action,
                                    gboolean multiple)
{
    // https://docs.gtk.org/gtk3/class.FileChooserDialog.html

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
                                title,
                                GTK_WINDOW(window),
                                action,
                                "gtk-cancel", GTK_RESPONSE_CANCEL,
                                "gtk-open", GTK_RESPONSE_ACCEPT,
                                NULL);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_show_hidden(chooser, window->prefs->show_hidden);
    gtk_file_chooser_set_select_multiple(chooser, multiple);

    VnrFile *current = window_get_current_file(window);
    if (current)
    {
        gchar *dirname = g_path_get_dirname(current->path);
        gtk_file_chooser_set_current_folder(chooser, dirname);
        g_free(dirname);
    }

    gint res = gtk_dialog_run(GTK_DIALOG(dialog));

    GSList *uri_list = NULL;

    if (res == GTK_RESPONSE_ACCEPT)
        uri_list = gtk_file_chooser_get_filenames(chooser);

    gtk_widget_destroy(dialog);

    return uri_list;
}

static void _window_action_copy_to(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    if (window_get_current_file(window) == NULL
        || window->mode != WINDOW_MODE_NORMAL)
        return;

    if (!window->destdir)
        _window_select_directory(window);

    if (!window->destdir)
        return;

    _window_copy(window, window->destdir, false);
}

static void _window_action_duplicate(VnrWindow *window, GtkWidget *widget)
{
    g_return_if_fail(window != NULL);
    (void) widget;

    if (window_get_current_file(window) == NULL
        || window->mode != WINDOW_MODE_NORMAL)
        return;

    _window_copy(window, NULL, true);
}

static void _window_copy(VnrWindow *window,
                         const char *destdir, gboolean follow)
{
    g_return_if_fail(window != NULL);

    VnrFile *current = window_get_current_file(window);

    if (!current || window->mode != WINDOW_MODE_NORMAL)
        return;

    const gchar *display_name = current->display_name;

    // duplicate...
    if (!destdir)
    {
        _window_duplicate(window, follow);

        return;
    }

    // copy to...
    gchar *newpath = g_build_filename(destdir, display_name, NULL);

    if (g_strcmp0(current->path, newpath) != 0)
        vnr_file_copy(current, newpath, NULL);

    g_free(newpath);
}

static void _window_duplicate(VnrWindow *window, gboolean follow)
{
    g_return_if_fail(window != NULL);

    VnrFile *current = window_get_current_file(window);

    if (!current || window->mode != WINDOW_MODE_NORMAL)
        return;

    const gchar *display_name = current->display_name;

    gchar *dirname = g_path_get_dirname(current->path);
    g_assert(dirname != NULL);

    gchar *newpath = g_build_filename(dirname, display_name, NULL);
    g_free(dirname);

    gchar *outpath;
    vnr_file_copy(current, newpath, &outpath);
    g_free(newpath);

    if (!outpath)
        return;

    //printf("%s\n", outpath);

    VnrFile *newfile = vnr_file_new_for_path(
                                        outpath,
                                        window->prefs->show_hidden);
    g_free(outpath);

    GList *item = vnr_list_insert(window->filelist, newfile);
    if (!item)
    {
        g_object_unref(newfile);
        return;
    }

    if (follow)
        _window_open_item(window, item);

    return;
}

static void _window_action_move_to(VnrWindow *window, GtkWidget *widget)
{
    g_return_if_fail(window != NULL);
    (void) widget;

    if (window_get_current_file(window) == NULL
        || window->mode != WINDOW_MODE_NORMAL)
        return;

    if (!window->destdir)
        _window_select_directory(window);

    if (!window->destdir)
        return;

    _window_move_to(window, window->destdir);
}

static void _window_move_to(VnrWindow *window, const char *destdir)
{
    g_return_if_fail(window != NULL);

    VnrFile *current = window_get_current_file(window);
    if (!current || !destdir || window->mode != WINDOW_MODE_NORMAL)
        return;

    const gchar *display_name = current->display_name;
    gchar *newpath = g_build_filename(window->destdir, display_name, NULL);

    if (g_strcmp0(current->path, newpath) == 0)
        goto cleanup;

    gboolean ret = vnr_file_rename(current, newpath);

    if (ret)
    {
        _window_set_monitor(window, NULL);
        _window_delete_item(window);
        window_close_file(window);

        if (window_load_file(window, FALSE))
            _window_set_monitor(window, window->filelist);
    }

cleanup:

    g_free(newpath);
}

static void _window_action_rename(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    VnrFile *current = window_get_current_file(window);

    if (!current || window->mode != WINDOW_MODE_NORMAL)
        return;

    gboolean result = dialog_file_rename(GTK_WINDOW(window), current);

    if (!result)
        return;

    vnr_list_sort(window->filelist);

    _view_on_zoom_changed(UNI_IMAGE_VIEW(window->view), window);
}

static void _window_action_delete(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    VnrFile *current = window_get_current_file(window);

    if (!current || window->mode != WINDOW_MODE_NORMAL)
        return;

    gboolean restart_slideshow = FALSE;

    if (window->mode == WINDOW_MODE_SLIDESHOW)
    {
        _window_slideshow_stop(window);
        restart_slideshow = TRUE;
    }

    gboolean cursor_was_hidden = FALSE;
    if (window->cursor_is_hidden)
    {
        cursor_was_hidden = TRUE;
        _window_show_cursor(window);
    }

    window->disable_autohide = TRUE;
    gboolean restart_autohide_timeout = FALSE;

    if (window->fs_source != NULL)
        restart_autohide_timeout = TRUE;

    const gchar *file_path = current->path;

    gchar *prompt = NULL;
    gchar *markup = NULL;
    GtkWidget *dlg = NULL;

    if (window->prefs->confirm_delete)
    {
        gchar *warning = NULL;
        warning = _("If you delete an item, it will be permanently lost.");

        // I18N: The '%s' is replaced with the name of the file to be deleted.
        prompt = g_strdup_printf(_("Are you sure you want to\n"
                                 "permanently delete \"%s\"?"),
                                 current->display_name);
        markup = g_markup_printf_escaped(
                    "<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s",
                    prompt, warning);

        dlg = gtk_message_dialog_new(GTK_WINDOW(window),
                                     GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_WARNING,
                                     GTK_BUTTONS_NONE,
                                     NULL);

        gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dlg),
                                      markup);

        gtk_dialog_add_buttons(GTK_DIALOG(dlg),
                               "gtk-cancel", GTK_RESPONSE_CANCEL,
                               "gtk-delete", GTK_RESPONSE_YES,
                               NULL);
    }

    if (!window->prefs->confirm_delete
        || gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_YES)
    {
        GFile *file = g_file_new_for_path(file_path);

        GError *error = NULL;
        g_file_trash(file, NULL, &error);

        if (error != NULL)
        {
            vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                                  TRUE,
                                  error->message,
                                  FALSE);
            restart_slideshow = FALSE;
        }
        else
        {
            _window_set_monitor(window, NULL);
            gboolean ret = _window_delete_item(window);

            if (!ret)
            {
                restart_slideshow = FALSE;
            }
            else
            {
                if (window->prefs->confirm_delete && !window->cursor_is_hidden)
                    vnr_tools_set_cursor(GTK_WIDGET(dlg), GDK_WATCH, true);

                window_close_file(window);

                if (window_load_file(window, FALSE))
                    _window_set_monitor(window, window->filelist);

                if (window->prefs->confirm_delete && !window->cursor_is_hidden)
                    vnr_tools_set_cursor(GTK_WIDGET(dlg), GDK_LEFT_PTR, false);
            }
        }
    }

    window->disable_autohide = FALSE;

    if (restart_slideshow)
        _window_slideshow_start(window);

    if (cursor_was_hidden)
        _window_hide_cursor(window);

    if (restart_autohide_timeout)
        _window_fullscreen_set_timeout(window);

    if (window->prefs->confirm_delete)
    {
        g_free(prompt);
        g_free(markup);
        gtk_widget_destroy(dlg);
    }
}

static gboolean _window_delete_item(VnrWindow *window)
{
    GList *next = vnr_list_delete_item(window->filelist);

    // ensure we won't free the list
    window_list_set_current(window, NULL);

    if (!next)
    {
        window_close_file(window);
        //gtk_action_group_set_sensitive(window->actions_collection, FALSE);

        window_list_set(window, NULL);
        //window_slideshow_deny(window);

        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE,
                              _("The given locations contain no images."),
                              TRUE);

        //restart_slideshow = FALSE;

        if (gtk_widget_get_visible(window->props_dlg))
        {
            vnr_properties_dialog_clear(
                        VNR_PROPERTIES_DIALOG(window->props_dlg));
        }

        return false;
    }

    window_list_set(window, next);

    return true;
}

static void _window_hide_cursor(VnrWindow *window)
{
    vnr_tools_set_cursor(GTK_WIDGET(window), GDK_BLANK_CURSOR, true);
    window->cursor_is_hidden = TRUE;
}

static void _window_show_cursor(VnrWindow *window)
{
    vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, true);
    window->cursor_is_hidden = FALSE;
}

static void _window_action_properties(VnrWindow *window, GtkWidget *widget)
{
    g_return_if_fail(window != NULL);
    (void) widget;

    if (window_get_current_file(window) == NULL
        || window->mode != WINDOW_MODE_NORMAL)
        return;

    vnr_properties_dialog_show(VNR_PROPERTIES_DIALOG(window->props_dlg));
}

static void _window_action_preferences(VnrWindow *window, GtkWidget *widget)
{
    g_return_if_fail(window != NULL);
    (void) widget;

    if (window->mode != WINDOW_MODE_NORMAL)
        return;

    vnr_prefs_dialog_run(window->prefs);
}


// pixbuf ---------------------------------------------------------------------

static void _window_rotate_pixbuf(VnrWindow *window,
                                  GdkPixbufRotation angle)
{
    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    //gdk_display_flush(display);

    // Stop slideshow while editing the image
    _window_slideshow_stop(window);

    GdkPixbuf *result = gdk_pixbuf_rotate_simple(
                            UNI_IMAGE_VIEW(window->view)->pixbuf,
                            angle);

    if (result == NULL)
    {
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE, _("Not enough virtual memory."),
                              FALSE);
        return;
    }

    uni_anim_view_set_static(UNI_ANIM_VIEW(window->view), result);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);

    g_object_unref(result);

    window->current_image_width = gdk_pixbuf_get_width(result);
    window->current_image_height = gdk_pixbuf_get_height(result);

    if (gtk_widget_get_visible(window->props_dlg))
        vnr_properties_dialog_update_image(
                            VNR_PROPERTIES_DIALOG(window->props_dlg));

    // Extra conditions. Rotating 180 degrees is also flipping horizontal
    // and vertical
    if ((window->modifications & (4))
        ^ ((angle == GDK_PIXBUF_ROTATE_CLOCKWISE) << 2))
        window->modifications ^= 3;

    window->modifications ^= 4;

    //gtk_action_group_set_sensitive(
    //      window->action_save, window->modifications);

#if 0
    if (window->modifications == 0
            && window->prefs->modify_behavior != VNR_PREFS_MODIFY_DEFAULT)
    {
        vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));
        return;
    }

    if (window->writable_format_name == NULL)
    {
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE,
                              _("Image modifications cannot be saved.\n"
                                "Writing in this format is not supported."),
                              FALSE);
    }
    else if (window->prefs->modify_behavior == VNR_PREFS_MODIFY_AUTOSAVE)
    {
        _window_action_save_image(window, NULL);
    }
    else if (window->prefs->modify_behavior == VNR_PREFS_MODIFY_ASK)
    {
        vnr_message_area_show_with_button(
                VNR_MESSAGE_AREA(window->msg_area),
                FALSE,
                _("Save modifications?\nThis will overwrite the image "
                  "and may reduce its quality!"),
                FALSE, "gtk-save",
                G_CALLBACK(_window_action_save_image));
    }
#endif
}

static void _window_flip_pixbuf(VnrWindow *window, gboolean horizontal)
{
    if (!window->can_edit)
        return;

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    GdkPixbuf *result = gdk_pixbuf_flip(
                                UNI_IMAGE_VIEW(window->view)->pixbuf,
                                horizontal);

    if (result == NULL)
    {
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE, _("Not enough virtual memory."),
                              FALSE);
        return;
    }

    uni_anim_view_set_static(UNI_ANIM_VIEW(window->view), result);

    if (gtk_widget_get_visible(window->props_dlg))
        vnr_properties_dialog_update_image(
                    VNR_PROPERTIES_DIALOG(window->props_dlg));

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);

    g_object_unref(result);

    // Extra conditions. Rotating 180 degrees is also flipping horizontal
    // and vertical
    window->modifications ^= (window->modifications & 4)
            ? 1 + horizontal : 2 - horizontal;

    //gtk_action_group_set_sensitive(window->action_save,
    //                               window->modifications);

#if 0
    if (window->modifications == 0)
    {
        vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));

        return;
    }

    if (window->writable_format_name == NULL)
    {
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE,
                              _("Image modifications cannot be saved.\n"
                                "Writing in this format is not supported."),
                              FALSE);
    }
    else if (window->prefs->modify_behavior == VNR_PREFS_MODIFY_AUTOSAVE)
    {
        _window_action_save_image(window, NULL);
    }
    else if (window->prefs->modify_behavior == VNR_PREFS_MODIFY_ASK)
    {
        vnr_message_area_show_with_button(
                VNR_MESSAGE_AREA(window->msg_area),
                FALSE,
                _("Save modifications?\nThis will overwrite"
                  " the image and may reduce its quality!"),
                FALSE, "gtk-save",
                G_CALLBACK(_window_action_save_image));
    }
#endif
}

static void _window_action_crop(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;

    if (!window->can_edit)
        return;

    VnrCrop *crop = (VnrCrop*) vnr_crop_new(window);

    if (!vnr_crop_run(crop))
    {
        g_object_unref(crop);
        return;
    }

    GdkPixbuf *cropped;
    GdkPixbuf *original;

    original = uni_image_view_get_pixbuf(UNI_IMAGE_VIEW(window->view));

    cropped = gdk_pixbuf_new(gdk_pixbuf_get_colorspace(original),
                             gdk_pixbuf_get_has_alpha(original),
                             gdk_pixbuf_get_bits_per_sample(original),
                             crop->area.width, crop->area.height);

    gdk_pixbuf_copy_area((const GdkPixbuf*) original,
                         crop->area.x, crop->area.y,
                         crop->area.width, crop->area.height,
                         cropped, 0, 0);

    uni_anim_view_set_static(UNI_ANIM_VIEW(window->view), cropped);

    g_object_unref(cropped);

    window->modifications |= 8;

    window->current_image_width = crop->area.width;
    window->current_image_height = crop->area.height;

    //gtk_action_group_set_sensitive(window->action_save, TRUE);

    if (window->writable_format_name == NULL)
    {
        vnr_message_area_show(
                VNR_MESSAGE_AREA(window->msg_area),
                TRUE,
                _("Image modifications cannot be saved.\n"
                  "Writing in this format is not supported."),
                FALSE);
    }

#if 0
    else if (window->prefs->modify_behavior == VNR_PREFS_MODIFY_AUTOSAVE)
    {
        _window_action_save_image(window, NULL);
    }
    else if (window->prefs->modify_behavior == VNR_PREFS_MODIFY_ASK)
    {
        vnr_message_area_show_with_button(
                VNR_MESSAGE_AREA(window->msg_area),
                FALSE,
                _("Save modifications?\nThis will overwrite"
                  " the image and may reduce its quality!"),
                FALSE,
                "gtk-save",
                G_CALLBACK(_window_action_save_image));
    }
#endif

    g_object_unref(crop);
}

static void _window_action_save_image(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;

    VnrFile *current = window_get_current_file(window);
    if (!current)
        return;

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    //if (window->prefs->modify_behavior == VNR_PREFS_MODIFY_ASK)
    //    vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));

    // Store exiv2 metadata to cache, so we can restore it afterwards
    uni_read_exiv2_to_cache(current->path);

    GError *error = NULL;

    if (g_strcmp0(window->writable_format_name, "jpeg") == 0)
    {
        gchar *quality = g_strdup_printf("%i", window->prefs->jpeg_quality);

        gdk_pixbuf_save(
                uni_image_view_get_pixbuf(UNI_IMAGE_VIEW(window->view)),
                current->path, "jpeg",
                &error, "quality", quality, NULL);

        g_free(quality);
    }
    else if (g_strcmp0(window->writable_format_name, "png") == 0)
    {
        gchar *compression;
        compression = g_strdup_printf("%i", window->prefs->png_compression);

        gdk_pixbuf_save(
                uni_image_view_get_pixbuf(UNI_IMAGE_VIEW(window->view)),
                current->path, "png",
                &error, "compression", compression, NULL);

        g_free(compression);
    }
    else
    {
        gdk_pixbuf_save(
                uni_image_view_get_pixbuf(UNI_IMAGE_VIEW(window->view)),
                current->path,
                window->writable_format_name, &error, NULL);
    }

    uni_write_exiv2_from_cache(current->path);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);

    if (error != NULL)
    {
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area), TRUE,
                              error->message, FALSE);
        return;
    }

    if (!window->prefs->reload_on_save)
    {
        window->no_reload = true;

        //window_load_file(window, FALSE);
        //return;
    }

    window->modifications = 0;

    //gtk_action_group_set_sensitive(window->action_save, FALSE);

    //if (window->prefs->modify_behavior != VNR_PREFS_MODIFY_ASK)
    _view_on_zoom_changed(UNI_IMAGE_VIEW(window->view), window);

    if (gtk_widget_get_visible(window->props_dlg))
        vnr_properties_dialog_update(VNR_PROPERTIES_DIALOG(window->props_dlg));
}

static void _window_action_zoom_normal(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;

    uni_image_view_set_zoom(UNI_IMAGE_VIEW(window->view), 1);
    uni_image_view_set_fitting(UNI_IMAGE_VIEW(window->view),
                               UNI_FITTING_NONE);
}

static void _window_action_zoom_fit(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;

    uni_image_view_set_fitting(UNI_IMAGE_VIEW(window->view), UNI_FITTING_FULL);
}


// set wallpaper --------------------------------------------------------------

static void _window_action_set_wallpaper(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;

    pid_t pid = fork();

    if (pid == 0)
    {
        gchar *tmp;

        VnrPrefsDesktop desktop_environment = window->prefs->desktop;

        if (desktop_environment == VNR_PREFS_DESKTOP_AUTO)
            desktop_environment = uni_detect_desktop_environment();

        VnrFile *current = window_get_current_file(window);

        switch (desktop_environment)
        {

        case VNR_PREFS_DESKTOP_CINNAMON:
            tmp = g_strdup_printf("file://%s",
                                  current->path);
            execlp("gsettings", "gsettings",
                   "set", "org.cinnamon.desktop.background",
                   "picture-uri", tmp,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_FLUXBOX:
            execlp("fbsetbg", "fbsetbg",
                   "-f", current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_GNOME2:
            execlp("gconftool-2", "gconftool-2",
                   "--set", "/desktop/gnome/background/picture_filename",
                   "--type", "string",
                   current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_GNOME3:
            tmp = g_strdup_printf("file://%s", current->path);
            execlp("gsettings", "gsettings",
                   "set", "org.gnome.desktop.background",
                   "picture-uri", tmp,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_LXDE:
            execlp("pcmanfm", "pcmanfm",
                   "--set-wallpaper",
                   current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_MATE:
            execlp("gsettings", "gsettings",
                   "set", "org.mate.background",
                   "picture-filename", current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_NITROGEN:
            execlp("nitrogen", "nitrogen",
                   "--set-zoom-fill", "--save",
                   current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_PUPPY:
            execlp("set_bg", "set_bg",
                   current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_WALLSET:
            execlp("wallset", "wallset",
                   current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_XFCE:
            execlp("xfconf-query", "xfconf-query",
                   "-c", "xfce4-desktop",
                   "-p", "/backdrop/screen0/monitor0/workspace0/last-image",
                   "--type", "string",
                   "--set",
                   current->path,
                   NULL);
            break;

        default:
            _exit(0);
        }
    }
    else
    {
        wait(NULL);
    }
}


// slideshow ------------------------------------------------------------------

static void _window_action_slideshow(VnrWindow *window)
{
    g_return_if_fail(window != NULL);

    if (!window->can_slideshow)
        return;

    VnrFile *current = window_get_current_file(window);
    if (!current)
        return;

    if (window->mode != WINDOW_MODE_SLIDESHOW)
    {
        // Uncomment to force Fullscreen along with Slideshow
        if (window->mode == WINDOW_MODE_NORMAL)
            _window_fullscreen(window);

        _window_slideshow_start(window);
    }
    else if (window->mode == WINDOW_MODE_SLIDESHOW)
    {
        // Uncomment to force Fullscreen along with Slideshow
        _window_unfullscreen(window);

        _window_slideshow_stop(window);
    }
}

static void _window_slideshow_start(VnrWindow *window)
{
    if (!window->can_slideshow)
        return;

    if (window->mode == WINDOW_MODE_SLIDESHOW)
        return;

    window->mode = WINDOW_MODE_SLIDESHOW;

    window->sl_source_id =
        g_timeout_add_seconds(window->sl_timeout,
                              (GSourceFunc) _window_on_sl_timeout,
                              window);


    window->can_slideshow = FALSE;

    if (window->fs_toolitem)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(window->toggle_btn),
                                     TRUE);
    }

    window->can_slideshow = TRUE;
}

static void _window_slideshow_stop(VnrWindow *window)
{
    if (!window->can_slideshow)
        return;

    if (window->mode != WINDOW_MODE_SLIDESHOW)
        return;

    window->can_slideshow = FALSE;

    if (window->fs_toolitem)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(window->toggle_btn),
                                     FALSE);
    }

    window->can_slideshow = TRUE;

    window->mode = WINDOW_MODE_FULLSCREEN;

    g_source_remove(window->sl_source_id);
}

#if 0
static void _window_slideshow_restart(VnrWindow *window)
{
    if (!window->slideshow)
        return;

    if (window->mode != WINDOW_MODE_SLIDESHOW)
        return;

    g_source_remove(window->sl_source_tag);
    window->sl_source_tag =
        g_timeout_add_seconds(
                        window->sl_timeout,
                        (GSourceFunc) _window_next_image_src,
                        window);
}
#endif

static void _window_slideshow_allow(VnrWindow *window)
{
    if (window->can_slideshow)
        return;

    window->can_slideshow = TRUE;

    if (window->fs_toolitem)
        gtk_widget_set_sensitive(window->toggle_btn, TRUE);
}

void window_slideshow_deny(VnrWindow *window)
{
    if (!window->can_slideshow)
        return;

    window->can_slideshow = FALSE;

    if (window->fs_toolitem)
        gtk_widget_set_sensitive(window->toggle_btn, FALSE);
}


// fullscreen -----------------------------------------------------------------

void window_fullscreen_toggle(VnrWindow *window)
{
    if (window->mode == WINDOW_MODE_NORMAL)
        _window_fullscreen(window);
    else
        _window_unfullscreen(window);
}

static void _window_fullscreen(VnrWindow *window)
{
    if (window_get_current_file(window) == NULL)
        return;

    gtk_window_fullscreen(GTK_WINDOW(window));

    window->mode = WINDOW_MODE_FULLSCREEN;

    GdkRGBA color;
    gdk_rgba_parse(&color, "black");
    _window_override_background_color(window, &color);

    if (window->prefs->fit_on_fullscreen)
        uni_image_view_set_zoom_mode(UNI_IMAGE_VIEW(window->view),
                                     VNR_PREFS_ZOOM_FIT);

    _window_update_fs_filename_label(window);

    //gtk_widget_hide(window->statusbar);
    //gtk_widget_show(window->properties_button);

    if (window->fs_toolitem)
        gtk_widget_show(window->fs_toolitem);

    _window_slideshow_stop(window);

    /* Reset timeouts for the toolbar autohide when the mouse
     * moves over the UniImageviewer.
     * "after" because it must be called after the uniImageView's
     * callback(when the image is dragged).*/
    g_signal_connect_after(window->view,
                           "motion-notify-event",
                           G_CALLBACK(_on_fullscreen_motion),
                           window);

    g_signal_connect(window->msg_area,
                     "enter-notify-event",
                     G_CALLBACK(_on_leave_image_area),
                     window);

    _window_fullscreen_set_timeout(window);
}

static void _window_unfullscreen(VnrWindow *window)
{
    if (window->mode == WINDOW_MODE_NORMAL)
        return;

    _window_slideshow_stop(window);
    window->mode = WINDOW_MODE_NORMAL;

    gtk_window_unfullscreen(GTK_WINDOW(window));

    if (window->prefs->dark_background)
    {
        GdkRGBA color;
        gdk_rgba_parse(&color, DARK_BACKGROUND_COLOR);
        _window_override_background_color(window, &color);
    }
    else
    {
        _window_override_background_color(window, NULL);
    }

    if (window->prefs->fit_on_fullscreen)
    {
        uni_image_view_set_zoom_mode(UNI_IMAGE_VIEW(window->view),
                                     window->prefs->zoom);
    }

    if (window->fs_toolitem)
        gtk_widget_hide(window->fs_toolitem);

    //if (!window->prefs->show_statusbar)
    //    gtk_widget_hide(window->statusbar);
    //else
    //    gtk_widget_show(window->statusbar);

    g_signal_handlers_disconnect_by_func(window->view,
                                         G_CALLBACK(_on_fullscreen_motion),
                                         window);

    g_signal_handlers_disconnect_by_func(window->msg_area,
                                         G_CALLBACK(_on_leave_image_area),
                                         window);

    _window_fullscreen_unset_timeout(window);
    _window_show_cursor(window);
}

static gboolean _on_fullscreen_motion(GtkWidget *widget,
                                      GdkEventMotion *ev,
                                      VnrWindow *window)
{
    if (window->disable_autohide)
        return FALSE;

    if (window->cursor_is_hidden)
        _window_show_cursor(window);

    _window_fullscreen_set_timeout(window);

    return FALSE;
}

static void _window_fullscreen_set_timeout(VnrWindow *window)
{
    _window_fullscreen_unset_timeout(window);

    window->fs_source = g_timeout_source_new(FULLSCREEN_TIMEOUT);
    g_source_set_callback(window->fs_source,
                          (GSourceFunc)_on_fullscreen_timeout,
                          window, NULL);

    g_source_attach(window->fs_source, NULL);
}

static gboolean _on_fullscreen_timeout(VnrWindow *window)
{
    // hides the toolbar
    _window_fullscreen_unset_timeout(window);

    if (window->disable_autohide)
        return FALSE;

    _window_hide_cursor(window);

    return FALSE;
}

static void _window_fullscreen_unset_timeout(VnrWindow *window)
{
    if (window->fs_source != NULL)
    {
        g_source_unref(window->fs_source);
        g_source_destroy(window->fs_source);
        window->fs_source = NULL;
    }
}

static gboolean _on_leave_image_area(GtkWidget *widget,
                                     GdkEventCrossing *ev,
                                     VnrWindow *window)
{
    _window_fullscreen_unset_timeout(window);
    return FALSE;
}


// toolitem -------------------------------------------------------------------

#if 0
static GtkWidget* _window_get_fs_toolitem(VnrWindow *window)
{
    if (window->fs_toolitem != NULL)
        return window->fs_toolitem;

    // Tool item, that contains the hbox
    GtkToolItem *item = gtk_tool_item_new();
    gtk_tool_item_set_expand(item, TRUE);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(item), box);

    GtkWidget *widget = gtk_button_new_from_stock("gtk-leave-fullscreen");

    g_signal_connect(widget, "clicked",
                     G_CALLBACK(_on_fullscreen_leave), window);
    gtk_box_pack_end(GTK_BOX(box), widget, FALSE, FALSE, 0);

    // create label for the current image's filename
    widget = gtk_label_new(NULL);
    gtk_label_set_ellipsize(GTK_LABEL(widget), PANGO_ELLIPSIZE_END);
    gtk_label_set_selectable(GTK_LABEL(widget), TRUE);
    window->fs_filename_label = widget;
    gtk_box_pack_end(GTK_BOX(box), widget, TRUE, TRUE, 10);

    widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

    widget = gtk_check_button_new_with_label(_("Show next image after: "));
    g_signal_connect(widget, "toggled",
                     G_CALLBACK(_on_toggle_show_next), window);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    window->toggle_btn = widget;

    // create spin button to adjust slideshow's timeout
    GtkAdjustment *spinner_adj =
        (GtkAdjustment*) gtk_adjustment_new(
                                    window->prefs->slideshow_timeout,
                                    1.0, 30.0, 1.0, 1.0, 0);

    widget = gtk_spin_button_new(spinner_adj, 1.0, 0);
    gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(widget), TRUE);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(widget),
                                      GTK_UPDATE_ALWAYS);
    g_signal_connect(widget, "value-changed",
                     G_CALLBACK(_on_spin_value_change), window);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    window->sl_timeout_widget = widget;

    window->fs_seconds_label = gtk_label_new(ngettext(" second",
                                                      " seconds", 5));
    gtk_box_pack_start(GTK_BOX(box),
                       window->fs_seconds_label,
                       FALSE, FALSE, 0);

    window->fs_toolitem = GTK_WIDGET(item);

    gtk_widget_show_all(window->fs_toolitem);

    return window->fs_toolitem;
}

static void _on_fullscreen_leave(GtkButton *button, VnrWindow *window)
{
    _window_unfullscreen(window);
}

static void _on_spin_value_change(GtkSpinButton *spinbutton,
                                  VnrWindow *window)
{
    int new_value = gtk_spin_button_get_value_as_int(spinbutton);

    if (new_value != window->prefs->slideshow_timeout)
        vnr_prefs_set_slideshow_timeout(window->prefs, new_value);

    gtk_label_set_text(GTK_LABEL(window->fs_seconds_label),
                       ngettext(" second", " seconds", new_value));
    window->sl_timeout = new_value;
    _window_slideshow_restart(window);
}

static void _on_toggle_show_next(GtkToggleButton *togglebutton,
                                 VnrWindow *window)
{
    if (!window->slideshow)
        return;

    if (window->mode == WINDOW_MODE_FULLSCREEN)
        _window_slideshow_start(window);
    else if (window->mode == WINDOW_MODE_SLIDESHOW)
        _window_slideshow_stop(window);
}

#endif


