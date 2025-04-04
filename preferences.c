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
#include "preferences.h"

#include "window.h"

#define UI_PATH PACKAGE_DATA_DIR "/imgview/vnr-preferences-dialog.ui"

G_DEFINE_TYPE(VnrPrefs, vnr_prefs, G_TYPE_OBJECT)

/**
 * VNR_PREF_LOAD_KEY:
 * @PK: Name of the VnrPrefs member.
 * @PT: Type of the VnrPrefs member, a suffix for g_key_file_get_.
 * @KN: Name of the key in the key file.
 * @DEF: Default value for the preference.
 */
#define VNR_PREF_LOAD_KEY(PK, PT, KN, DEF)                           \
    prefs->PK = g_key_file_get_##PT(conf, "prefs", KN, &read_error); \
    if (read_error != NULL)                                          \
    {                                                                \
        prefs->PK = DEF;                                             \
        g_clear_error(&read_error);                                  \
    }


static void vnr_prefs_set_default(VnrPrefs *prefs);
static GtkWidget* _prefs_build(VnrPrefs *prefs);
static gboolean vnr_prefs_load(VnrPrefs *prefs, GError **error);

static void toggle_show_hidden_cb(GtkToggleButton *togglebutton,
                                  gpointer user_data);
static void toggle_dark_background_cb(GtkToggleButton *togglebutton,
                                      gpointer user_data);
static void toggle_fit_on_fullscreen_cb(GtkToggleButton *togglebutton,
                                        gpointer user_data);
static void toggle_smooth_images_cb(GtkToggleButton *togglebutton,
                                    gpointer user_data);
static void toggle_confirm_delete_cb(GtkToggleButton *togglebutton,
                                     gpointer user_data);
static void toggle_reload_on_save_cb(GtkToggleButton *togglebutton,
                                     gpointer user_data);
static void change_zoom_mode_cb(GtkComboBox *widget, gpointer user_data);
static void change_desktop_env_cb(GtkComboBox *widget, gpointer user_data);
static void _on_change_jpeg_quality(GtkSpinButton *range, gpointer user_data);
static void change_png_compression_cb(GtkSpinButton *range, gpointer user_data);
static void change_action_wheel_cb(GtkComboBox *widget, gpointer user_data);
static void change_action_click_cb(GtkComboBox *widget, gpointer user_data);
static void change_action_modify_cb(GtkComboBox *widget, gpointer user_data);
static void change_spin_value_cb(GtkSpinButton *spinbutton, gpointer user_data);
static gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event,
                             gpointer user_data);


// Creation -------------------------------------------------------------------

GObject* vnr_prefs_new(GtkWidget *window)
{
    VnrPrefs *prefs = g_object_new(VNR_TYPE_PREFS, NULL);

    prefs->window = window;

    return (GObject*) prefs;
}

static void vnr_prefs_class_init(VnrPrefsClass *klass)
{
}

static void vnr_prefs_init(VnrPrefs *prefs)
{
    GError *error = NULL;

    if (vnr_prefs_load(prefs, &error))
        return;

    g_warning("Error loading config file: %s. All preferences are set"
              " to their default values. Saving ...",
              error->message);
    vnr_prefs_set_default(prefs);
    vnr_prefs_save(prefs);
}

static void vnr_prefs_set_default(VnrPrefs *prefs)
{
    prefs->start_maximized = false;
    prefs->window_width = 480;
    prefs->window_height = 300;

    prefs->zoom = VNR_PREFS_ZOOM_SMART;
    prefs->show_hidden = FALSE;
    prefs->dark_background = FALSE;
    prefs->fit_on_fullscreen = TRUE;
    prefs->smooth_images = TRUE;
    prefs->confirm_delete = FALSE;
    prefs->slideshow_timeout = 5;
    prefs->behavior_wheel = VNR_PREFS_WHEEL_ZOOM;
    prefs->behavior_click = VNR_PREFS_CLICK_ZOOM;
    prefs->behavior_modify = VNR_PREFS_MODIFY_ASK;
    prefs->jpeg_quality = 90;
    prefs->png_compression = 9;
    prefs->reload_on_save = FALSE;
    prefs->show_scrollbar = false;
    prefs->start_slideshow = FALSE;
    prefs->start_fullscreen = FALSE;
    prefs->auto_resize = FALSE;
    prefs->desktop = VNR_PREFS_DESKTOP_AUTO;
}

static GtkWidget* _prefs_build(VnrPrefs *prefs)
{
    GtkBuilder *builder;
    GtkWidget *window;
    GError *error = NULL;

    GObject *close_button;
    GtkToggleButton *show_hidden;
    GtkToggleButton *dark_background;
    GtkToggleButton *fit_on_fullscreen;
    GtkBox *zoom_mode_box;
    GtkComboBoxText *zoom_mode;
    GtkToggleButton *smooth_images;
    GtkToggleButton *confirm_delete;
    GtkToggleButton *reload_on_save;
    GtkSpinButton *slideshow_timeout;
    GtkComboBoxText *action_wheel;
    GtkComboBoxText *action_click;
    GtkComboBoxText *action_modify;

    GtkBox *desktop_box;
    GtkComboBoxText *desktop_env;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, UI_PATH, &error);

    if (error != NULL)
    {
        g_warning("%s\n", error->message);
        g_object_unref(builder);
        return NULL;
    }

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

    // Close button
    close_button = gtk_builder_get_object(builder, "close_button");
    g_signal_connect_swapped(close_button, "clicked",
                             G_CALLBACK(gtk_widget_hide_on_delete), window);

    // Show hidden files checkbox
    show_hidden = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "show_hidden"));
    gtk_toggle_button_set_active(show_hidden, prefs->show_hidden);
    g_signal_connect(G_OBJECT(show_hidden), "toggled", G_CALLBACK(toggle_show_hidden_cb), prefs);

    // Show dark background checkbox
    dark_background = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "dark_background"));
    gtk_toggle_button_set_active(dark_background, prefs->dark_background);
    g_signal_connect(G_OBJECT(dark_background), "toggled", G_CALLBACK(toggle_dark_background_cb), prefs);

    // Fit on fullscreen checkbox
    fit_on_fullscreen = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "fit_on_fullscreen"));
    gtk_toggle_button_set_active(fit_on_fullscreen, prefs->fit_on_fullscreen);
    g_signal_connect(G_OBJECT(fit_on_fullscreen), "toggled", G_CALLBACK(toggle_fit_on_fullscreen_cb), prefs);

    // Smooth images checkbox
    smooth_images = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "smooth_images"));
    gtk_toggle_button_set_active(smooth_images, prefs->smooth_images);
    g_signal_connect(G_OBJECT(smooth_images), "toggled", G_CALLBACK(toggle_smooth_images_cb), prefs);

    // Confirm delete checkbox
    confirm_delete = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "confirm_delete"));
    gtk_toggle_button_set_active(confirm_delete, prefs->confirm_delete);
    g_signal_connect(G_OBJECT(confirm_delete), "toggled", G_CALLBACK(toggle_confirm_delete_cb), prefs);

    // Reload image after save checkbox
    reload_on_save = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "reload"));
    gtk_toggle_button_set_active(reload_on_save, prefs->reload_on_save);
    g_signal_connect(G_OBJECT(reload_on_save), "toggled", G_CALLBACK(toggle_reload_on_save_cb), prefs);

    // Slideshow timeout spin button
    slideshow_timeout = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "slideshow_timeout"));
    gtk_spin_button_set_value(slideshow_timeout, (gdouble)prefs->slideshow_timeout);
    prefs->slideshow_timeout_widget = slideshow_timeout;
    g_signal_connect(G_OBJECT(slideshow_timeout), "value-changed", G_CALLBACK(change_spin_value_cb), prefs);

    GtkSpinButton *spin_btn = NULL;

    // JPEG quality scale
    spin_btn = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "jpeg_scale"));
    gtk_spin_button_set_value(spin_btn, (gdouble) prefs->jpeg_quality);
    g_signal_connect(G_OBJECT(spin_btn), "value-changed",
                     G_CALLBACK(_on_change_jpeg_quality), prefs);

    // PNG compression scale
    spin_btn = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "png_scale"));
    gtk_spin_button_set_value(spin_btn, (gdouble) prefs->png_compression);
    g_signal_connect(G_OBJECT(spin_btn), "value-changed",
                     G_CALLBACK(change_png_compression_cb), prefs);

    // Zoom mode combo box
    zoom_mode_box = GTK_BOX(gtk_builder_get_object(builder, "zoom_mode_box"));

    zoom_mode = (GtkComboBoxText *)gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(zoom_mode, _("Smart Mode"));
    gtk_combo_box_text_append_text(zoom_mode, _("1:1 Mode"));
    gtk_combo_box_text_append_text(zoom_mode, _("Fit To Window Mode"));
    gtk_combo_box_text_append_text(zoom_mode, _("Last Used Mode"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(zoom_mode), prefs->zoom);

    gtk_box_pack_end(zoom_mode_box, GTK_WIDGET(zoom_mode), FALSE, FALSE, 0);
    gtk_widget_show(GTK_WIDGET(zoom_mode));

    g_signal_connect(G_OBJECT(zoom_mode), "changed", G_CALLBACK(change_zoom_mode_cb), prefs);

    // Desktop combo box
    desktop_box = GTK_BOX(gtk_builder_get_object(builder, "desktop_box"));

    desktop_env = (GtkComboBoxText*) gtk_combo_box_text_new();

    gtk_combo_box_text_append_text(desktop_env, _("Autodetect"));
    gtk_combo_box_text_append_text(desktop_env, "Cinnamon");
    gtk_combo_box_text_append_text(desktop_env, "FluxBox");
    gtk_combo_box_text_append_text(desktop_env, "GNOME 2");
    gtk_combo_box_text_append_text(desktop_env, "GNOME 3");
    gtk_combo_box_text_append_text(desktop_env, "hsetroot");
    gtk_combo_box_text_append_text(desktop_env, "LXDE");
    gtk_combo_box_text_append_text(desktop_env, "MATE");
    gtk_combo_box_text_append_text(desktop_env, "Nitrogen");
    gtk_combo_box_text_append_text(desktop_env, "PUPPY");
    gtk_combo_box_text_append_text(desktop_env, "XFCE");

    gtk_combo_box_set_active(GTK_COMBO_BOX(desktop_env), prefs->desktop);

    gtk_box_pack_end(desktop_box, GTK_WIDGET(desktop_env), FALSE, FALSE, 0);
    gtk_widget_show(GTK_WIDGET(desktop_env));

    g_signal_connect(G_OBJECT(desktop_env), "changed", G_CALLBACK(change_desktop_env_cb), prefs);

    // Behavior combo boxes
    GtkGrid *behavior_grid =
        GTK_GRID(gtk_builder_get_object(builder, "behavior_grid"));

    action_wheel = (GtkComboBoxText *)gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(action_wheel, _("Navigate images"));
    gtk_combo_box_text_append_text(action_wheel, _("Zoom image"));
    gtk_combo_box_text_append_text(action_wheel, _("Scroll image up/down"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(action_wheel), prefs->behavior_wheel);

    gtk_grid_attach(behavior_grid,
                    GTK_WIDGET(action_wheel),
                    1, 0,
                    1, 1);

    gtk_widget_show(GTK_WIDGET(action_wheel));
    g_signal_connect(G_OBJECT(action_wheel), "changed",
                     G_CALLBACK(change_action_wheel_cb), prefs);

    action_click = (GtkComboBoxText *)gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(action_click, _("Switch zoom modes"));
    gtk_combo_box_text_append_text(action_click, _("Enter fullscreen mode"));
    gtk_combo_box_text_append_text(action_click, _("Navigate images"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(action_click), prefs->behavior_click);

    gtk_grid_attach(behavior_grid,
                    GTK_WIDGET(action_click),
                    1, 1,
                    1, 1);

    gtk_widget_show(GTK_WIDGET(action_click));
    g_signal_connect(G_OBJECT(action_click), "changed", G_CALLBACK(change_action_click_cb), prefs);

    action_modify = (GtkComboBoxText *)gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(action_modify, _("Ask every time"));
    gtk_combo_box_text_append_text(action_modify, _("Autosave"));
    gtk_combo_box_text_append_text(action_modify, _("Ignore changes"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(action_modify), prefs->behavior_modify);

    gtk_grid_attach(behavior_grid,
                    GTK_WIDGET(action_modify),
                    1, 2,
                    1, 1);

    gtk_widget_show(GTK_WIDGET(action_modify));
    g_signal_connect(G_OBJECT(action_modify), "changed", G_CALLBACK(change_action_modify_cb), prefs);

    // Window signals
    g_signal_connect(G_OBJECT(window), "delete-event",
                     G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(key_press_cb), NULL);

    g_object_unref(G_OBJECT(builder));

    return window;
}

static gboolean vnr_prefs_load(VnrPrefs *prefs, GError **error)
{
    const gchar *path = g_build_filename(g_get_user_config_dir(),
                                         PACKAGE,
                                         "imgview.conf",
                                         NULL);

    GKeyFile *conf = g_key_file_new();
    GError *load_file_error = NULL;
    g_key_file_load_from_file(conf, path, G_KEY_FILE_NONE, &load_file_error);

    g_free((char*) path);

    if (load_file_error != NULL)
    {
        g_propagate_error(error, load_file_error);
        g_key_file_free(conf);
        return FALSE;
    }

    GError *read_error = NULL;
    (void) read_error;

    VNR_PREF_LOAD_KEY(start_maximized, boolean, "start-maximized", FALSE);
    VNR_PREF_LOAD_KEY(window_width, integer, "window-width", 480);
    VNR_PREF_LOAD_KEY(window_height, integer, "window-height", 300);

    VNR_PREF_LOAD_KEY(zoom, integer, "zoom-mode", VNR_PREFS_ZOOM_SMART);
    VNR_PREF_LOAD_KEY(fit_on_fullscreen, boolean, "fit-on-fullscreen", TRUE);
    VNR_PREF_LOAD_KEY(show_hidden, boolean, "show-hidden", FALSE);
    VNR_PREF_LOAD_KEY(dark_background, boolean, "dark-background", FALSE);
    VNR_PREF_LOAD_KEY(smooth_images, boolean, "smooth-images", TRUE);
    VNR_PREF_LOAD_KEY(confirm_delete, boolean, "confirm-delete", TRUE);
    VNR_PREF_LOAD_KEY(reload_on_save, boolean, "reload-on-save", FALSE);
    VNR_PREF_LOAD_KEY(show_scrollbar, boolean, "show-scrollbar", TRUE);
    VNR_PREF_LOAD_KEY(slideshow_timeout, integer, "slideshow-timeout", 5);
    VNR_PREF_LOAD_KEY(auto_resize, boolean, "auto-resize", FALSE);
    VNR_PREF_LOAD_KEY(behavior_wheel, integer, "behavior-wheel", VNR_PREFS_WHEEL_ZOOM);
    VNR_PREF_LOAD_KEY(behavior_click, integer, "behavior-click", VNR_PREFS_CLICK_ZOOM);
    VNR_PREF_LOAD_KEY(behavior_modify, integer, "behavior-modify", VNR_PREFS_MODIFY_ASK);
    VNR_PREF_LOAD_KEY(jpeg_quality, integer, "jpeg-quality", 90);
    VNR_PREF_LOAD_KEY(png_compression, integer, "png-compression", 9);
    VNR_PREF_LOAD_KEY(desktop, integer, "desktop", VNR_PREFS_DESKTOP_AUTO);

    g_key_file_free(conf);

    return TRUE;
}

gboolean vnr_prefs_save(VnrPrefs *prefs)
{
    const gchar *dir = g_build_filename(g_get_user_config_dir(), PACKAGE, NULL);
    const gchar *path = g_build_filename(dir, "imgview.conf", NULL);

    GKeyFile *conf = g_key_file_new();
    g_key_file_set_boolean(conf, "prefs", "start-maximized",
                           prefs->start_maximized);
    g_key_file_set_integer(conf, "prefs", "window-width",
                           prefs->window_width);
    g_key_file_set_integer(conf, "prefs", "window-height",
                           prefs->window_height);
    g_key_file_set_integer(conf, "prefs", "zoom-mode",
                           prefs->zoom);
    g_key_file_set_boolean(conf, "prefs", "fit-on-fullscreen",
                           prefs->fit_on_fullscreen);
    g_key_file_set_boolean(conf, "prefs", "show-hidden",
                           prefs->show_hidden);
    g_key_file_set_boolean(conf, "prefs", "dark-background",
                           prefs->dark_background);
    g_key_file_set_boolean(conf, "prefs", "smooth-images",
                           prefs->smooth_images);
    g_key_file_set_boolean(conf, "prefs", "confirm-delete",
                           prefs->confirm_delete);
    g_key_file_set_boolean(conf, "prefs", "reload-on-save",
                           prefs->reload_on_save);
    g_key_file_set_boolean(conf, "prefs", "show-scrollbar",
                           prefs->show_scrollbar);
    g_key_file_set_integer(conf, "prefs", "slideshow-timeout",
                           prefs->slideshow_timeout);
    g_key_file_set_boolean(conf, "prefs", "auto-resize",
                           prefs->auto_resize);
    g_key_file_set_integer(conf, "prefs", "behavior-wheel",
                           prefs->behavior_wheel);
    g_key_file_set_integer(conf, "prefs", "behavior-click",
                           prefs->behavior_click);
    g_key_file_set_integer(conf, "prefs", "behavior-modify",
                           prefs->behavior_modify);
    g_key_file_set_integer(conf, "prefs", "jpeg-quality",
                           prefs->jpeg_quality);
    g_key_file_set_integer(conf, "prefs", "png-compression",
                           prefs->png_compression);
    g_key_file_set_integer(conf, "prefs", "desktop",
                           prefs->desktop);

    if (g_mkdir_with_parents(dir, 0700) != 0)
        g_warning("Error creating config file's parent directory (%s)\n", dir);

    FILE *rcfile = fopen(path, "w");

    if (rcfile != NULL)
    {
        gchar *data = g_key_file_to_data(conf, NULL, NULL);
        fputs(data, rcfile);
        fclose(rcfile);
        g_free(data);
    }
    else
    {
        g_warning("Saving config file: Unable to open the configuration file"
                  " for writing!\n");
    }

    g_key_file_free(conf);
    g_free((char*) dir);
    g_free((char*) path);

    return TRUE;
}

void vnr_prefs_show_dialog(VnrPrefs *prefs)
{
    GtkWidget *dialog = _prefs_build(prefs);

    gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);

}

void vnr_prefs_set_slideshow_timeout(VnrPrefs *prefs, int value)
{
    gtk_spin_button_set_value(prefs->slideshow_timeout_widget,
                              (gdouble) value);
}

void vnr_prefs_set_show_scrollbar(VnrPrefs *prefs, gboolean show_scrollbar)
{
    if (prefs->show_scrollbar != show_scrollbar)
    {
        prefs->show_scrollbar = show_scrollbar;
        vnr_prefs_save(prefs);
    }
}


// Private signal handlers ----------------------------------------------------

static void toggle_show_hidden_cb(GtkToggleButton *togglebutton, gpointer user_data)
{
    VNR_PREFS(user_data)->show_hidden = gtk_toggle_button_get_active(togglebutton);
    vnr_prefs_save(VNR_PREFS(user_data));
}

static void toggle_dark_background_cb(GtkToggleButton *togglebutton, gpointer user_data)
{
    VNR_PREFS(user_data)->dark_background = gtk_toggle_button_get_active(togglebutton);
    vnr_prefs_save(VNR_PREFS(user_data));
    window_preferences_apply(VNR_WINDOW(VNR_PREFS(user_data)->window));
}

static void toggle_fit_on_fullscreen_cb(GtkToggleButton *togglebutton, gpointer user_data)
{
    VNR_PREFS(user_data)->fit_on_fullscreen = gtk_toggle_button_get_active(togglebutton);
    vnr_prefs_save(VNR_PREFS(user_data));
}

static void toggle_smooth_images_cb(GtkToggleButton *togglebutton, gpointer user_data)
{
    VNR_PREFS(user_data)->smooth_images = gtk_toggle_button_get_active(togglebutton);
    vnr_prefs_save(VNR_PREFS(user_data));
    window_preferences_apply(VNR_WINDOW(VNR_PREFS(user_data)->window));
}

static void toggle_confirm_delete_cb(GtkToggleButton *togglebutton, gpointer user_data)
{
    VNR_PREFS(user_data)->confirm_delete = gtk_toggle_button_get_active(togglebutton);
    vnr_prefs_save(VNR_PREFS(user_data));
}

static void toggle_reload_on_save_cb(GtkToggleButton *togglebutton, gpointer user_data)
{
    VNR_PREFS(user_data)->reload_on_save = gtk_toggle_button_get_active(togglebutton);
    vnr_prefs_save(VNR_PREFS(user_data));
}

static void change_zoom_mode_cb(GtkComboBox *widget, gpointer user_data)
{
    VNR_PREFS(user_data)->zoom = gtk_combo_box_get_active(widget);
    vnr_prefs_save(VNR_PREFS(user_data));
}

static void change_desktop_env_cb(GtkComboBox *widget, gpointer user_data)
{
    VNR_PREFS(user_data)->desktop = gtk_combo_box_get_active(widget);
    vnr_prefs_save(VNR_PREFS(user_data));
}

static void _on_change_jpeg_quality(GtkSpinButton *range, gpointer user_data)
{
    VnrPrefs *prefs = VNR_PREFS(user_data);

    prefs->jpeg_quality = (int) gtk_spin_button_get_value(range);

    vnr_prefs_save(VNR_PREFS(user_data));
}

static void change_png_compression_cb(GtkSpinButton *range, gpointer user_data)
{
    VnrPrefs *prefs = VNR_PREFS(user_data);

    prefs->png_compression = (int) gtk_spin_button_get_value(range);

    vnr_prefs_save(VNR_PREFS(user_data));
}

static void change_action_wheel_cb(GtkComboBox *widget, gpointer user_data)
{
    VNR_PREFS(user_data)->behavior_wheel = gtk_combo_box_get_active(widget);
    vnr_prefs_save(VNR_PREFS(user_data));
}

static void change_action_click_cb(GtkComboBox *widget, gpointer user_data)
{
    VNR_PREFS(user_data)->behavior_click = gtk_combo_box_get_active(widget);
    vnr_prefs_save(VNR_PREFS(user_data));
}

static void change_action_modify_cb(GtkComboBox *widget, gpointer user_data)
{
    VNR_PREFS(user_data)->behavior_modify = gtk_combo_box_get_active(widget);
    vnr_prefs_save(VNR_PREFS(user_data));
}

static void change_spin_value_cb(GtkSpinButton *spinbutton, gpointer user_data)
{
    int new_value = gtk_spin_button_get_value_as_int(spinbutton);

    VNR_PREFS(user_data)->slideshow_timeout = new_value;
    vnr_prefs_save(VNR_PREFS(user_data));
    window_preferences_apply(VNR_WINDOW(VNR_PREFS(user_data)->window));
}

static gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if (event->keyval == GDK_KEY_Escape)
    {
        gtk_widget_hide(widget);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


