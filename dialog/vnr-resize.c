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
#include "vnr-resize.h"

G_DEFINE_TYPE(VnrResize, vnr_resize, G_TYPE_OBJECT)

static GtkWidget* _vnr_resize_dlg_new(VnrResize *resize);

static void _on_width_value_changed(GtkSpinButton *spinbutton,
                                    VnrResize *resize);
static void _on_height_value_changed(GtkSpinButton *spinbutton,
                                     VnrResize *resize);
static void _on_link_check_toggled(GtkWidget *checkbutton, VnrResize *resize);


// creation -------------------------------------------------------------------

GObject* vnr_resize_new(VnrWindow *window)
{
    VnrResize *resize = g_object_new(VNR_TYPE_RESIZE, NULL);

    resize->window = window;

    return (GObject*) resize;
}

static void vnr_resize_class_init(VnrResizeClass *klass)
{
}

static void vnr_resize_init(VnrResize *resize)
{
    (void) resize;
}

gboolean vnr_resize_run(VnrResize *resize)
{
    GtkWidget *dialog = _vnr_resize_dlg_new(resize);

    if (!dialog)
        return FALSE;

    gint ret = gtk_dialog_run(GTK_DIALOG(dialog));

    resize->new_width =
            gtk_spin_button_get_value_as_int(resize->spin_width);
    resize->new_height =
            gtk_spin_button_get_value_as_int(resize->spin_height);

    gtk_widget_destroy(dialog);

    if (resize->new_width == resize->window->current_image_width
        && resize->new_height == resize->window->current_image_height)
    {
        return FALSE;
    }
    else
    {
        return (ret == GTK_RESPONSE_ACCEPT);
    }
}

static GtkWidget* _vnr_resize_dlg_new(VnrResize *resize)
{
    VnrWindow *window = resize->window;
    VnrPrefs *prefs = window->prefs;

    resize->orig_width = window->current_image_width;
    resize->orig_height = window->current_image_height;

    g_return_val_if_fail(resize->orig_width != 0, NULL);
    g_return_val_if_fail(resize->orig_height != 0, NULL);

    resize->ratio = resize->orig_width / resize->orig_height;


    GtkWidget *dialog = g_object_new(GTK_TYPE_DIALOG,
                                     "border-width", 5,
                                     "title", "Resize Image",
                                     "resizable", false,
                                     "modal", true,
                                     NULL);

    gtk_window_set_transient_for(GTK_WINDOW(dialog),
                                 GTK_WINDOW(window));

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_box_pack_start(GTK_BOX(content), grid, true, true, 0);

    int row = 0;
    GtkWidget *widget = NULL;

    widget = gtk_label_new(_("Width: "));
    gtk_grid_attach(GTK_GRID(grid), widget, 0, row, 1, 1);

    resize->spin_width =
        GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(
                                    1,
                                    VNR_MAX_SIZE,
                                    1));
    gtk_spin_button_set_increments(resize->spin_width, 1, 10);
    gtk_spin_button_set_value(resize->spin_width,
                              window->current_image_width);
    gtk_grid_attach(GTK_GRID(grid),
                    GTK_WIDGET(resize->spin_width), 1, row, 1, 1);

    resize->link_check = gtk_check_button_new_with_label(_("Link"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(resize->link_check),
                                 prefs->resize_link);
    gtk_grid_attach(GTK_GRID(grid),
                    resize->link_check, 2, row, 1, 1);

    ++row;

    widget = gtk_label_new(_("Height: "));
    gtk_grid_attach(GTK_GRID(grid), widget, 0, row, 1, 1);

    resize->spin_height =
        GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(
                                    1,
                                    VNR_MAX_SIZE,
                                    1));
    gtk_spin_button_set_increments(resize->spin_height, 1, 10);
    gtk_spin_button_set_value(resize->spin_height,
                              window->current_image_height);
    gtk_grid_attach(GTK_GRID(grid),
                    GTK_WIDGET(resize->spin_height), 1, row, 1, 1);

    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                           _("Cancel"), GTK_RESPONSE_CANCEL,
                           _("Resize"), GTK_RESPONSE_ACCEPT,
                           NULL);

    g_signal_connect(resize->spin_width, "value-changed",
                     G_CALLBACK(_on_width_value_changed), resize);
    g_signal_connect(resize->spin_height, "value-changed",
                     G_CALLBACK(_on_height_value_changed), resize);
    g_signal_connect(resize->link_check, "toggled",
                     G_CALLBACK(_on_link_check_toggled), resize);

    gtk_widget_show_all(dialog);

    return dialog;
}

static void _on_width_value_changed(GtkSpinButton *spinbutton,
                                    VnrResize *resize)
{
    VnrPrefs *prefs = resize->window->prefs;
    if (!prefs->resize_link)
        return;

    gdouble width = gtk_spin_button_get_value(spinbutton);
    gtk_spin_button_set_value(resize->spin_height, width / resize->ratio);
}

static void _on_height_value_changed(GtkSpinButton *spinbutton,
                                     VnrResize *resize)
{
    VnrPrefs *prefs = resize->window->prefs;
    if (!prefs->resize_link)
        return;

    gdouble height = gtk_spin_button_get_value(spinbutton);
    gtk_spin_button_set_value(resize->spin_width, height * resize->ratio);
}

static void _on_link_check_toggled(GtkWidget *checkbutton, VnrResize *resize)
{
    VnrPrefs *prefs = resize->window->prefs;
    prefs->resize_link = gtk_toggle_button_get_active(
                                        GTK_TOGGLE_BUTTON(checkbutton));
}


