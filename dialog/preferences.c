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

#define PREFS_GROUP             "Prefs"
#define PREFS_WINDOW_WIDTH      "window-width"
#define PREFS_WINDOW_HEIGHT     "window-height"
#define PREFS_START_MAXIMIZED   "start-maximized"
#define PREFS_SHOW_SCROLLBAR    "show-scrollbar"

#define PREFS_ZOOM              "zoom-mode"
#define PREFS_DESKTOP           "desktop"
#define PREFS_SMOOTH_IMAGES     "smooth-images"
#define PREFS_CONFIRM_DELETE    "confirm-delete"
#define PREFS_SHOW_HIDDEN       "show-hidden"
#define PREFS_DARK_BACKGROUND   "dark-background"

#define PREFS_SL_TIMEOUT        "slideshow-timeout"
#define PREFS_FIT_ON_FULLSCREEN "fit-on-fullscreen"

#define PREFS_WHEEL_BEHAVIOR    "wheel-behavior"
#define PREFS_CLICK_BEHAVIOR    "click-behavior"
#define PREFS_MODIFY_BEHAVIOR   "modify-behavior"

#define PREFS_RELOAD_ON_SAVE    "reload-on-save"
#define PREFS_JPEG_QUALITY      "jpeg-quality"
#define PREFS_PNG_COMPRESSION   "png-compression"

#define PREFS_RESIZE_LINK       "resize-link"


#define VNR_PREFS_LOAD_KEY(PK, PT, KN, DEF)                              \
    prefs->PK = g_key_file_get_##PT(conf, PREFS_GROUP, KN, &read_error); \
    if (read_error != NULL)                                              \
    {                                                                    \
        prefs->PK = DEF;                                                 \
        g_clear_error(&read_error);                                      \
    }

G_DEFINE_TYPE(VnrPrefs, vnr_prefs, G_TYPE_OBJECT)

static void vnr_prefs_set_default(VnrPrefs *prefs);
static gboolean vnr_prefs_load(VnrPrefs *prefs);

static void _prefs_zoom_mode_changed(VnrPrefs *prefs, GtkComboBox *combo);
static void _prefs_desktop_env_changed(VnrPrefs *prefs, GtkComboBox *combo);
static void _prefs_smooth_images_toggled(VnrPrefs *prefs,
                                         GtkToggleButton *togglebtn);
static void _prefs_confirm_delete_toggled(VnrPrefs *prefs,
                                          GtkToggleButton *togglebtn);
static void _prefs_show_hidden_toggled(VnrPrefs *prefs,
                                       GtkToggleButton *togglebtn);
static void _prefs_dark_background_toggled(VnrPrefs *prefs,
                                      GtkToggleButton *togglebtn);

static void _prefs_timeout_changed(VnrPrefs *prefs, GtkSpinButton *spinbutton);
static void toggle_fit_on_fullscreen_cb(VnrPrefs *prefs,
                                        GtkToggleButton *togglebtn);
static void _prefs_wheel_action_changed(VnrPrefs *prefs, GtkComboBox *combo);
static void _prefs_click_action_changed(VnrPrefs *prefs, GtkComboBox *combo);
static void _prefs_modify_action_changed(VnrPrefs *prefs, GtkComboBox *combo);

static void _prefs_reload_toggled(VnrPrefs *prefs, GtkToggleButton *togglebtn);
static void _prefs_jpeg_quality_changed(VnrPrefs *prefs,
                                        GtkSpinButton *spinbtn);
static void _prefs_png_comp_changed(VnrPrefs *prefs, GtkSpinButton *spinbtn);


// creation -------------------------------------------------------------------

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
    if (vnr_prefs_load(prefs))
        return;

    vnr_prefs_set_default(prefs);
    vnr_prefs_save(prefs);
}

static void vnr_prefs_set_default(VnrPrefs *prefs)
{
    prefs->window_width = 480;
    prefs->window_height = 300;
    prefs->start_maximized = false;
    prefs->show_scrollbar = false;
    prefs->start_fullscreen = false;
    prefs->start_slideshow = false;

    prefs->zoom = VNR_PREFS_ZOOM_SMART;
    prefs->desktop = VNR_PREFS_DESKTOP_AUTO;
    prefs->smooth_images = TRUE;
    prefs->confirm_delete = FALSE;
    prefs->show_hidden = FALSE;
    prefs->dark_background = FALSE;

    prefs->sl_timeout = 5;
    prefs->fit_on_fullscreen = TRUE;

    prefs->wheel_behavior = VNR_PREFS_WHEEL_ZOOM;
    prefs->click_behavior = VNR_PREFS_CLICK_ZOOM;
    prefs->modify_behavior = VNR_PREFS_MODIFY_DISCARD;

    prefs->reload_on_save = FALSE;
    prefs->jpeg_quality = 90;
    prefs->png_compression = 9;

    prefs->resize_link = TRUE;
}

static gboolean vnr_prefs_load(VnrPrefs *prefs)
{
    (void) prefs;

    GKeyFile *conf = g_key_file_new();
    const gchar *path = g_build_filename(g_get_user_config_dir(),
                                         PACKAGE, "imgview.conf", NULL);

    gboolean ret = g_key_file_load_from_file(
                            conf, path, G_KEY_FILE_NONE, NULL);

    g_free((char*) path);

    if (!ret)
    {
        g_key_file_free(conf);

        return FALSE;
    }

    GError *read_error = NULL;
    (void) read_error;

    VNR_PREFS_LOAD_KEY(window_width, integer,
                       PREFS_WINDOW_WIDTH, 480);
    VNR_PREFS_LOAD_KEY(window_height, integer,
                       PREFS_WINDOW_HEIGHT, 300);
    VNR_PREFS_LOAD_KEY(show_scrollbar, boolean,
                       PREFS_SHOW_SCROLLBAR, FALSE);
    VNR_PREFS_LOAD_KEY(start_maximized, boolean,
                       PREFS_START_MAXIMIZED, FALSE);

    VNR_PREFS_LOAD_KEY(zoom, integer,
                       PREFS_ZOOM, VNR_PREFS_ZOOM_SMART);
    VNR_PREFS_LOAD_KEY(desktop, integer,
                       PREFS_DESKTOP, VNR_PREFS_DESKTOP_AUTO);
    VNR_PREFS_LOAD_KEY(smooth_images, boolean,
                       PREFS_SMOOTH_IMAGES, TRUE);
    VNR_PREFS_LOAD_KEY(confirm_delete, boolean,
                       PREFS_CONFIRM_DELETE, TRUE);
    VNR_PREFS_LOAD_KEY(show_hidden, boolean,
                       PREFS_SHOW_HIDDEN, FALSE);
    VNR_PREFS_LOAD_KEY(dark_background, boolean,
                       PREFS_DARK_BACKGROUND, FALSE);

    VNR_PREFS_LOAD_KEY(sl_timeout, integer,
                       PREFS_SL_TIMEOUT, 5);
    VNR_PREFS_LOAD_KEY(fit_on_fullscreen, boolean,
                       PREFS_FIT_ON_FULLSCREEN, TRUE);

    VNR_PREFS_LOAD_KEY(wheel_behavior, integer,
                       PREFS_WHEEL_BEHAVIOR, VNR_PREFS_WHEEL_ZOOM);
    VNR_PREFS_LOAD_KEY(click_behavior, integer,
                       PREFS_CLICK_BEHAVIOR, VNR_PREFS_CLICK_ZOOM);
    VNR_PREFS_LOAD_KEY(modify_behavior, integer,
                       PREFS_MODIFY_BEHAVIOR, VNR_PREFS_MODIFY_DISCARD);

    VNR_PREFS_LOAD_KEY(reload_on_save, boolean,
                       PREFS_RELOAD_ON_SAVE, FALSE);
    VNR_PREFS_LOAD_KEY(jpeg_quality, integer,
                       PREFS_JPEG_QUALITY, 90);
    VNR_PREFS_LOAD_KEY(png_compression, integer,
                       PREFS_PNG_COMPRESSION, 9);

    VNR_PREFS_LOAD_KEY(resize_link, boolean,
                       PREFS_RESIZE_LINK, TRUE);

    g_key_file_free(conf);

    return TRUE;
}

gboolean vnr_prefs_save(VnrPrefs *prefs)
{
    const gchar *dir = g_build_filename(g_get_user_config_dir(),
                                        PACKAGE, NULL);
    const gchar *path = g_build_filename(dir, "imgview.conf", NULL);

    GKeyFile *conf = g_key_file_new();

    g_key_file_set_integer(conf, PREFS_GROUP, PREFS_WINDOW_WIDTH,
                           prefs->window_width);
    g_key_file_set_integer(conf, PREFS_GROUP, PREFS_WINDOW_HEIGHT,
                           prefs->window_height);
    g_key_file_set_boolean(conf, PREFS_GROUP, PREFS_SHOW_SCROLLBAR,
                           prefs->show_scrollbar);
    g_key_file_set_boolean(conf, PREFS_GROUP, PREFS_START_MAXIMIZED,
                           prefs->start_maximized);

    g_key_file_set_integer(conf, PREFS_GROUP, PREFS_ZOOM,
                           prefs->zoom);
    g_key_file_set_integer(conf, PREFS_GROUP, PREFS_DESKTOP,
                           prefs->desktop);
    g_key_file_set_boolean(conf, PREFS_GROUP, PREFS_SMOOTH_IMAGES,
                           prefs->smooth_images);
    g_key_file_set_boolean(conf, PREFS_GROUP, PREFS_CONFIRM_DELETE,
                           prefs->confirm_delete);
    g_key_file_set_boolean(conf, PREFS_GROUP, PREFS_SHOW_HIDDEN,
                           prefs->show_hidden);
    g_key_file_set_boolean(conf, PREFS_GROUP, PREFS_DARK_BACKGROUND,
                           prefs->dark_background);

    g_key_file_set_integer(conf, PREFS_GROUP, PREFS_SL_TIMEOUT,
                           prefs->sl_timeout);
    g_key_file_set_boolean(conf, PREFS_GROUP, PREFS_FIT_ON_FULLSCREEN,
                           prefs->fit_on_fullscreen);

    g_key_file_set_integer(conf, PREFS_GROUP, PREFS_WHEEL_BEHAVIOR,
                           prefs->wheel_behavior);
    g_key_file_set_integer(conf, PREFS_GROUP, PREFS_CLICK_BEHAVIOR,
                           prefs->click_behavior);
    g_key_file_set_integer(conf, PREFS_GROUP, PREFS_MODIFY_BEHAVIOR,
                           prefs->modify_behavior);

    g_key_file_set_boolean(conf, PREFS_GROUP, PREFS_RELOAD_ON_SAVE,
                           prefs->reload_on_save);
    g_key_file_set_integer(conf, PREFS_GROUP, PREFS_JPEG_QUALITY,
                           prefs->jpeg_quality);
    g_key_file_set_integer(conf, PREFS_GROUP, PREFS_PNG_COMPRESSION,
                           prefs->png_compression);

    g_key_file_set_boolean(conf, PREFS_GROUP, PREFS_RESIZE_LINK,
                           prefs->resize_link);

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

void vnr_prefs_set_slideshow_timeout(VnrPrefs *prefs, int value)
{
    gtk_spin_button_set_value(prefs->sl_timeout_widget, (gdouble) value);
}

void vnr_prefs_set_show_scrollbar(VnrPrefs *prefs, gboolean show_scrollbar)
{
    if (prefs->show_scrollbar == show_scrollbar)
        return;

    prefs->show_scrollbar = show_scrollbar;
    vnr_prefs_save(prefs);
}


// dialog ---------------------------------------------------------------------

void vnr_prefs_dialog_run(VnrPrefs *prefs)
{
    GtkBuilder *builder = gtk_builder_new();
    GError *error = NULL;

    gtk_builder_add_from_file(builder, UI_PATH, &error);

    if (error != NULL)
    {
        g_warning("%s\n", error->message);
        g_object_unref(builder);
        return;
    }

    GtkWidget *dialog = GTK_WIDGET(gtk_builder_get_object(builder, "window"));

    GObject *object = gtk_builder_get_object(builder, "close_button");
    g_signal_connect_swapped(object, "clicked",
                             G_CALLBACK(gtk_widget_hide_on_delete), dialog);

    GtkBox *box = NULL;
    GtkComboBoxText *combotext = NULL;
    GtkToggleButton *togglebtn = NULL;
    GtkSpinButton *spinbtn = NULL;
    GtkGrid *grid = NULL;

    // ------------------------------------------------------------------------

    // zoom mode
    box = GTK_BOX(gtk_builder_get_object(builder, "zoom_mode_box"));
    combotext = (GtkComboBoxText*) gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(combotext, _("Smart Mode"));
    gtk_combo_box_text_append_text(combotext, _("1:1 Mode"));
    gtk_combo_box_text_append_text(combotext, _("Fit To Window Mode"));
    gtk_combo_box_text_append_text(combotext, _("Last Used Mode"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combotext), prefs->zoom);
    gtk_box_pack_end(box, GTK_WIDGET(combotext), FALSE, FALSE, 0);
    gtk_widget_show(GTK_WIDGET(combotext));
    g_signal_connect_swapped(G_OBJECT(combotext), "changed",
                             G_CALLBACK(_prefs_zoom_mode_changed), prefs);

    // desktop
    box = GTK_BOX(gtk_builder_get_object(builder, "desktop_box"));
    combotext = (GtkComboBoxText*) gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(combotext, _("Autodetect"));
    gtk_combo_box_text_append_text(combotext, "Cinnamon");
    gtk_combo_box_text_append_text(combotext, "FluxBox");
    gtk_combo_box_text_append_text(combotext, "GNOME 2");
    gtk_combo_box_text_append_text(combotext, "GNOME 3");
    gtk_combo_box_text_append_text(combotext, "LXDE");
    gtk_combo_box_text_append_text(combotext, "MATE");
    gtk_combo_box_text_append_text(combotext, "Nitrogen");
    gtk_combo_box_text_append_text(combotext, "PUPPY");
    gtk_combo_box_text_append_text(combotext, "Wallset");
    gtk_combo_box_text_append_text(combotext, "XFCE");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combotext), prefs->desktop);
    gtk_box_pack_end(box, GTK_WIDGET(combotext), FALSE, FALSE, 0);
    gtk_widget_show(GTK_WIDGET(combotext));
    g_signal_connect_swapped(G_OBJECT(combotext), "changed",
                             G_CALLBACK(_prefs_desktop_env_changed), prefs);

    // smooth images
    togglebtn = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
                                                         "smooth_images"));
    gtk_toggle_button_set_active(togglebtn, prefs->smooth_images);
    g_signal_connect_swapped(G_OBJECT(togglebtn), "toggled",
                             G_CALLBACK(_prefs_smooth_images_toggled), prefs);

    // confirm delete
    togglebtn = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
                                                         "confirm_delete"));
    gtk_toggle_button_set_active(togglebtn, prefs->confirm_delete);
    g_signal_connect_swapped(G_OBJECT(togglebtn), "toggled",
                             G_CALLBACK(_prefs_confirm_delete_toggled), prefs);

    // show hidden
    togglebtn = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
                                                         "show_hidden"));
    gtk_toggle_button_set_active(togglebtn, prefs->show_hidden);
    g_signal_connect_swapped(G_OBJECT(togglebtn), "toggled",
                             G_CALLBACK(_prefs_show_hidden_toggled), prefs);

    // dark background
    togglebtn = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
                                                         "dark_background"));
    gtk_toggle_button_set_active(togglebtn, prefs->dark_background);
    g_signal_connect_swapped(G_OBJECT(togglebtn), "toggled",
                             G_CALLBACK(_prefs_dark_background_toggled),
                             prefs);

    // ------------------------------------------------------------------------

    // slideshow timeout
    spinbtn = GTK_SPIN_BUTTON(gtk_builder_get_object(builder,
                                                     "slideshow_timeout"));
    gtk_spin_button_set_value(spinbtn, (gdouble) prefs->sl_timeout);
    prefs->sl_timeout_widget = spinbtn;
    g_signal_connect_swapped(G_OBJECT(spinbtn), "value-changed",
                             G_CALLBACK(_prefs_timeout_changed), prefs);

    // fit on fullscreen
    togglebtn = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,
                                                         "fit_on_fullscreen"));
    gtk_toggle_button_set_active(togglebtn, prefs->fit_on_fullscreen);
    g_signal_connect_swapped(G_OBJECT(togglebtn), "toggled",
                             G_CALLBACK(toggle_fit_on_fullscreen_cb), prefs);

    // ------------------------------------------------------------------------

    grid = GTK_GRID(gtk_builder_get_object(builder, "behavior_grid"));

    combotext = (GtkComboBoxText*) gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(combotext, _("Navigate images"));
    gtk_combo_box_text_append_text(combotext, _("Zoom image"));
    gtk_combo_box_text_append_text(combotext, _("Scroll image up/down"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combotext), prefs->wheel_behavior);
    gtk_grid_attach(grid, GTK_WIDGET(combotext), 1, 0, 1, 1);
    gtk_widget_show(GTK_WIDGET(combotext));
    g_signal_connect_swapped(G_OBJECT(combotext), "changed",
                             G_CALLBACK(_prefs_wheel_action_changed), prefs);

    combotext = (GtkComboBoxText*) gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(combotext, _("Switch zoom modes"));
    gtk_combo_box_text_append_text(combotext, _("Enter fullscreen mode"));
    gtk_combo_box_text_append_text(combotext, _("Navigate images"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combotext),
                             prefs->click_behavior);
    gtk_grid_attach(grid, GTK_WIDGET(combotext), 1, 1, 1, 1);
    gtk_widget_show(GTK_WIDGET(combotext));
    g_signal_connect_swapped(G_OBJECT(combotext), "changed",
                             G_CALLBACK(_prefs_click_action_changed), prefs);

    combotext = (GtkComboBoxText*) gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(combotext, _("Autosave"));
    gtk_combo_box_text_append_text(combotext, _("Ignore"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(combotext), prefs->modify_behavior);
    gtk_grid_attach(grid, GTK_WIDGET(combotext), 1, 2, 1, 1);
    gtk_widget_show(GTK_WIDGET(combotext));
    g_signal_connect_swapped(G_OBJECT(combotext), "changed",
                             G_CALLBACK(_prefs_modify_action_changed), prefs);

    // ------------------------------------------------------------------------

    // reload image after save
    togglebtn = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "reload"));
    gtk_toggle_button_set_active(togglebtn, prefs->reload_on_save);
    g_signal_connect_swapped(G_OBJECT(togglebtn), "toggled",
                             G_CALLBACK(_prefs_reload_toggled), prefs);

    // jpg quality
    spinbtn = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "jpeg_scale"));
    gtk_spin_button_set_value(spinbtn, (gdouble) prefs->jpeg_quality);
    g_signal_connect_swapped(G_OBJECT(spinbtn), "value-changed",
                             G_CALLBACK(_prefs_jpeg_quality_changed), prefs);

    // png compression
    spinbtn = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "png_scale"));
    gtk_spin_button_set_value(spinbtn, (gdouble) prefs->png_compression);
    g_signal_connect_swapped(G_OBJECT(spinbtn), "value-changed",
                             G_CALLBACK(_prefs_png_comp_changed), prefs);

    // ------------------------------------------------------------------------

    // window signals
    //g_signal_connect(G_OBJECT(dialog), "delete-event",
    //                 G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    g_object_unref(G_OBJECT(builder));

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}


// dialog callbacks -----------------------------------------------------------

static void _prefs_zoom_mode_changed(VnrPrefs *prefs, GtkComboBox *combo)
{
    prefs->zoom = gtk_combo_box_get_active(combo);
    vnr_prefs_save(prefs);
}

static void _prefs_desktop_env_changed(VnrPrefs *prefs, GtkComboBox *combo)
{
    prefs->desktop = gtk_combo_box_get_active(combo);
    vnr_prefs_save(prefs);
}

static void _prefs_smooth_images_toggled(VnrPrefs *prefs,
                                         GtkToggleButton *togglebtn)
{
    prefs->smooth_images = gtk_toggle_button_get_active(togglebtn);
    vnr_prefs_save(prefs);
    window_preferences_apply(VNR_WINDOW(prefs->window));
}

static void _prefs_confirm_delete_toggled(VnrPrefs *prefs,
                                     GtkToggleButton *togglebtn)
{
    prefs->confirm_delete = gtk_toggle_button_get_active(togglebtn);
    vnr_prefs_save(prefs);
}

static void _prefs_show_hidden_toggled(VnrPrefs *prefs,
                                       GtkToggleButton *togglebtn)
{
    prefs->show_hidden = gtk_toggle_button_get_active(togglebtn);
    vnr_prefs_save(prefs);
}

static void _prefs_dark_background_toggled(VnrPrefs *prefs,
                                           GtkToggleButton *togglebtn)
{
    prefs->dark_background = gtk_toggle_button_get_active(togglebtn);
    vnr_prefs_save(prefs);
    window_preferences_apply(VNR_WINDOW(prefs->window));
}

static void _prefs_timeout_changed(VnrPrefs *prefs, GtkSpinButton *spinbutton)
{
    int value = gtk_spin_button_get_value_as_int(spinbutton);
    prefs->sl_timeout = value;
    vnr_prefs_save(prefs);
    window_preferences_apply(VNR_WINDOW(prefs->window));
}

static void toggle_fit_on_fullscreen_cb(VnrPrefs *prefs,
                                        GtkToggleButton *togglebtn)
{
    prefs->fit_on_fullscreen = gtk_toggle_button_get_active(togglebtn);
    vnr_prefs_save(prefs);
}

static void _prefs_wheel_action_changed(VnrPrefs *prefs, GtkComboBox *combo)
{
    prefs->wheel_behavior = gtk_combo_box_get_active(combo);
    vnr_prefs_save(prefs);
}

static void _prefs_click_action_changed(VnrPrefs *prefs, GtkComboBox *combo)
{
    prefs->click_behavior = gtk_combo_box_get_active(combo);
    vnr_prefs_save(prefs);
}

static void _prefs_modify_action_changed(VnrPrefs *prefs, GtkComboBox *combo)
{
    prefs->modify_behavior = gtk_combo_box_get_active(combo);
    vnr_prefs_save(prefs);
}

static void _prefs_reload_toggled(VnrPrefs *prefs,
                                     GtkToggleButton *togglebtn)
{
    prefs->reload_on_save = gtk_toggle_button_get_active(togglebtn);
    vnr_prefs_save(prefs);
}

static void _prefs_jpeg_quality_changed(VnrPrefs *prefs, GtkSpinButton *spinbtn)
{
    prefs->jpeg_quality = (int) gtk_spin_button_get_value(spinbtn);

    vnr_prefs_save(prefs);
}

static void _prefs_png_comp_changed(VnrPrefs *prefs, GtkSpinButton *spinbtn)
{
    prefs->png_compression = (int) gtk_spin_button_get_value(spinbtn);

    vnr_prefs_save(prefs);
}


