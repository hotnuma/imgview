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
#include "vnr-properties.h"

#include "vnr-tools.h"
#include "uni-exiv2.hpp"

G_DEFINE_TYPE(VnrPropertiesDialog, vnr_propsdlg, GTK_TYPE_DIALOG)

static void vnr_propsdlg_update_metadata(VnrPropertiesDialog *dialog);

static gboolean key_press_cb(GtkWidget *widget,
                             GdkEventKey *event, gpointer user_data)
{
    if (event->keyval == GDK_KEY_Escape)
    {
        gtk_widget_hide(widget);
        return TRUE;
    }
    else
        return FALSE;
}

static void get_file_info(gchar *filename, goffset *size, const gchar **type)
{
    GFile *file;
    file = g_file_new_for_path(filename);
    GFileInfo *fileinfo;
    fileinfo = g_file_query_info(file,
                                 G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
                                 G_FILE_ATTRIBUTE_STANDARD_SIZE,
                                 0, NULL, NULL);

    if (fileinfo == NULL)
    {
        return;
    }

    *size = g_file_info_get_size(fileinfo);
    *type = g_strdup(g_file_info_get_content_type(fileinfo));

    g_object_unref(file);
    g_object_unref(fileinfo);
}

static void set_new_pixbuf(VnrPropertiesDialog *dialog, GdkPixbuf *original)
{
    if (dialog->thumbnail != NULL)
    {
        g_object_unref(dialog->thumbnail);
        dialog->thumbnail = NULL;
    }

    if (original == NULL)
    {
        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        gtk_image_set_from_stock(GTK_IMAGE(dialog->image),
                                 "gtk-missing-image", GTK_ICON_SIZE_DIALOG);
        G_GNUC_END_IGNORE_DEPRECATIONS
        return;
    }

    int width, height;

    width = gdk_pixbuf_get_width(original);
    height = gdk_pixbuf_get_height(original);

    vnr_tools_fit_to_size(&height, &width, 100, 100);

    dialog->thumbnail = gdk_pixbuf_scale_simple(original, width, height,
                                                GDK_INTERP_NEAREST);
}

static void vnr_propsdlg_class_init(VnrPropertiesDialogClass *klass)
{

}

GtkWidget* vnr_propsdlg_new(VnrWindow *window)
{
    VnrPropertiesDialog *dialog = g_object_new(VNR_TYPE_PROPERTIES_DIALOG,
                                               NULL);

    dialog->thumbnail = NULL;
    dialog->window = window;

    g_signal_connect_swapped(dialog->prev_button,
                             "clicked",
                             G_CALLBACK(window_action_prev),
                             window);

    g_signal_connect_swapped(dialog->next_button,
                             "clicked",
                             G_CALLBACK(window_action_next),
                             window);

    //gtk_activatable_set_related_action(GTK_ACTIVATABLE(dialog->next_button),
    //                                   next_action);
    //gtk_activatable_set_related_action(GTK_ACTIVATABLE(dialog->prev_button),
    //                                   prev_action);

    gtk_button_set_label(GTK_BUTTON(dialog->prev_button), _("_Previous"));
    gtk_button_set_label(GTK_BUTTON(dialog->next_button), _("_Next"));
    gtk_widget_grab_focus(dialog->close_button);

    return (GtkWidget*) dialog;
}

static void vnr_propsdlg_init(VnrPropertiesDialog *dialog)
{
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    GtkWidget *action_area = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS

    GtkWidget *temp_box;
    GtkWidget *temp_label;

    gtk_window_set_title(GTK_WINDOW(dialog), _("Image Properties"));

    // VBox containing the Location labels
    temp_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(temp_box), 10);
    gtk_box_pack_start(GTK_BOX(content_area), temp_box, FALSE, FALSE, 0);

    temp_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(temp_label), _("<b>Location:</b>"));
    gtk_label_set_xalign(GTK_LABEL(temp_label), 0);
    gtk_label_set_yalign(GTK_LABEL(temp_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box), temp_label, FALSE, FALSE, 0);

    dialog->location_label = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(dialog->location_label), 0);
    gtk_label_set_yalign(GTK_LABEL(dialog->location_label), 0);
    gtk_label_set_selectable(GTK_LABEL(dialog->location_label), TRUE);
    gtk_label_set_ellipsize(GTK_LABEL(dialog->location_label),
                            PANGO_ELLIPSIZE_END);
    gtk_box_pack_start(GTK_BOX(temp_box),
                       dialog->location_label, FALSE, FALSE, 0);

    // VBox containing the image and meta data
    dialog->layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(dialog->layout), 10);
    gtk_box_pack_start(GTK_BOX(content_area), dialog->layout, FALSE, FALSE, 0);

    // HBox containing the image and the two columns with labels
    dialog->image_layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(dialog->image_layout), 10);
    gtk_box_pack_start(GTK_BOX(dialog->layout),
                       dialog->image_layout, FALSE, FALSE, 0);

    // The frame around the image
    temp_box = gtk_frame_new(NULL);
    gtk_widget_set_size_request(temp_box, 105, 105);
    gtk_box_pack_start(GTK_BOX(dialog->image_layout),
                       temp_box, FALSE, FALSE, 0);

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    dialog->image = gtk_image_new_from_stock("gtk-missing-image",
                                             GTK_ICON_SIZE_DIALOG);
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(temp_box), dialog->image);

    // Buttons
    dialog->prev_button = gtk_button_new();
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_button_set_image(GTK_BUTTON(dialog->prev_button),
                         gtk_image_new_from_stock("gtk-go-back",
                                                  GTK_ICON_SIZE_BUTTON));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(action_area), dialog->prev_button);

    dialog->next_button = gtk_button_new();
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_button_set_image(GTK_BUTTON(dialog->next_button),
                         gtk_image_new_from_stock("gtk-go-forward",
                                                  GTK_ICON_SIZE_BUTTON));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(action_area), dialog->next_button);

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    dialog->close_button = gtk_button_new_from_stock("gtk-close");
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(action_area), dialog->close_button);

    // Image Data Labels
    temp_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(dialog->image_layout),
                       temp_box, FALSE, FALSE, 0);

    temp_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(temp_label), _("<b>Name:</b>"));
    gtk_label_set_xalign(GTK_LABEL(temp_label), 0);
    gtk_label_set_yalign(GTK_LABEL(temp_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box), temp_label, FALSE, FALSE, 0);
    temp_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(temp_label), _("<b>Type:</b>"));
    gtk_label_set_xalign(GTK_LABEL(temp_label), 0);
    gtk_label_set_yalign(GTK_LABEL(temp_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box), temp_label, FALSE, FALSE, 0);
    temp_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(temp_label), _("<b>Size:</b>"));
    gtk_label_set_xalign(GTK_LABEL(temp_label), 0);
    gtk_label_set_yalign(GTK_LABEL(temp_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box), temp_label, FALSE, FALSE, 0);
    temp_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(temp_label), _("<b>Width:</b>"));
    gtk_label_set_xalign(GTK_LABEL(temp_label), 0);
    gtk_label_set_yalign(GTK_LABEL(temp_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box), temp_label, FALSE, FALSE, 0);
    temp_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(temp_label), _("<b>Height:</b>"));
    gtk_label_set_xalign(GTK_LABEL(temp_label), 0);
    gtk_label_set_yalign(GTK_LABEL(temp_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box), temp_label, FALSE, FALSE, 0);
    temp_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(temp_label), _("<b>Modified:</b>"));
    gtk_label_set_xalign(GTK_LABEL(temp_label), 0);
    gtk_label_set_yalign(GTK_LABEL(temp_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box), temp_label, FALSE, FALSE, 0);

    temp_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(dialog->image_layout),
                       temp_box, FALSE, FALSE, 0);

    dialog->name_label = gtk_label_new(NULL);
    gtk_label_set_selectable(GTK_LABEL(dialog->name_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(dialog->name_label), 0);
    gtk_label_set_yalign(GTK_LABEL(dialog->name_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box),
                       dialog->name_label, FALSE, FALSE, 0);

    dialog->type_label = gtk_label_new(NULL);
    gtk_label_set_selectable(GTK_LABEL(dialog->type_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(dialog->type_label), 0);
    gtk_label_set_yalign(GTK_LABEL(dialog->type_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box),
                       dialog->type_label, FALSE, FALSE, 0);

    dialog->size_label = gtk_label_new(NULL);
    gtk_label_set_selectable(GTK_LABEL(dialog->size_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(dialog->size_label), 0);
    gtk_label_set_yalign(GTK_LABEL(dialog->size_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box),
                       dialog->size_label, FALSE, FALSE, 0);

    dialog->width_label = gtk_label_new(NULL);
    gtk_label_set_selectable(GTK_LABEL(dialog->width_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(dialog->width_label), 0);
    gtk_label_set_yalign(GTK_LABEL(dialog->width_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box),
                       dialog->width_label, FALSE, FALSE, 0);

    dialog->height_label = gtk_label_new(NULL);
    gtk_label_set_selectable(GTK_LABEL(dialog->height_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(dialog->height_label), 0);
    gtk_label_set_yalign(GTK_LABEL(dialog->height_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box),
                       dialog->height_label, FALSE, FALSE, 0);

    dialog->modified_label = gtk_label_new(NULL);
    gtk_label_set_selectable(GTK_LABEL(dialog->modified_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(dialog->modified_label), 0);
    gtk_label_set_yalign(GTK_LABEL(dialog->modified_label), 0);
    gtk_box_pack_start(GTK_BOX(temp_box),
                       dialog->modified_label, FALSE, FALSE, 0);

    // Metadata Labels

    temp_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(dialog->layout),
                       temp_box, FALSE, FALSE, 0);

    dialog->meta_names_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(temp_box),
                       dialog->meta_names_box, FALSE, FALSE, 0);

    dialog->meta_values_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(temp_box),
                       dialog->meta_values_box, FALSE, FALSE, 0);

    // Events and rest

    g_signal_connect(G_OBJECT(dialog), "delete-event",
                     G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect_swapped(G_OBJECT(dialog->close_button), "clicked",
                             G_CALLBACK(gtk_widget_hide_on_delete), dialog);

    g_signal_connect(G_OBJECT(dialog),
                     "key-press-event", G_CALLBACK(key_press_cb), NULL);

    gtk_widget_show_all(content_area);
    gtk_widget_show_all(action_area);
}

void vnr_propsdlg_update(VnrPropertiesDialog *dialog)
{
    VnrFile *current = window_get_current_file(dialog->window);
    if (!current)
        return;

    gchar *filetype_desc = NULL;
    gchar *filesize_str = NULL;

    goffset filesize = 0;
    const gchar *filetype = NULL;
    get_file_info((gchar*) current->path, &filesize, &filetype);

    if (filetype == NULL && filesize == 0)
    {
        vnr_propsdlg_clear(dialog);
        return;
    }

    vnr_propsdlg_update_image(dialog);
    vnr_propsdlg_update_metadata(dialog);

    filesize_str = g_format_size(filesize);

    filetype_desc = g_content_type_get_description(filetype);

    gtk_label_set_text(GTK_LABEL(dialog->name_label),
                       (gchar *)current->display_name);

    gtk_label_set_text(GTK_LABEL(dialog->location_label),
                       (gchar *)current->path);

    gtk_label_set_text(GTK_LABEL(dialog->type_label), filetype_desc);
    gtk_label_set_text(GTK_LABEL(dialog->size_label), filesize_str);

    g_free(filesize_str);
    g_free((gchar*) filetype);
    g_free(filetype_desc);
}

static void vnr_propsdlg_clear_metadata(VnrPropertiesDialog *dialog)
{
    GList *children = gtk_container_get_children(
                GTK_CONTAINER(dialog->meta_values_box));

    for (GList *iter = children; iter != NULL; iter = g_list_next(iter))
    {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }

    g_list_free(children);

    children = gtk_container_get_children(
                GTK_CONTAINER(dialog->meta_names_box));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter))
    {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }

    g_list_free(children);
}

static void vnr_cb_add_metadata(const char *label,
                                const char *value, void *user_data)
{
    VnrPropertiesDialog *dialog = VNR_PROPERTIES_DIALOG(user_data);
    GtkWidget *temp_label;
    gchar *formatted_label;

    // value
    temp_label = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(temp_label), value);
    gtk_label_set_selectable(GTK_LABEL(temp_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(temp_label), 0);
    gtk_label_set_yalign(GTK_LABEL(temp_label), 0);
    gtk_box_pack_start(GTK_BOX(dialog->meta_values_box),
                       temp_label, FALSE, FALSE, 0);

    gtk_widget_show(temp_label);

    // label
    formatted_label = g_strdup_printf("<b>%s:</b>", label);

    temp_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(temp_label), formatted_label);
    gtk_label_set_xalign(GTK_LABEL(temp_label), 0);
    gtk_label_set_yalign(GTK_LABEL(temp_label), 0);
    gtk_box_pack_start(GTK_BOX(dialog->meta_names_box),
                       temp_label, FALSE, FALSE, 0);

    g_free(formatted_label);
    gtk_widget_show(temp_label);
}

static void vnr_propsdlg_update_metadata(VnrPropertiesDialog *dialog)
{
    vnr_propsdlg_clear_metadata(dialog);

    VnrFile *current = window_get_current_file(dialog->window);
    if (!current)
        return;

    uni_read_exiv2_map(current->path, vnr_cb_add_metadata, (void*) dialog);
}

void vnr_propsdlg_update_image(VnrPropertiesDialog *dialog)
{
    VnrFile *current = window_get_current_file(dialog->window);
    if (!current)
        return;

    gchar *width_str, *height_str;
    int date_modified_buf_size = 80;
    gchar date_modified[date_modified_buf_size];

    strftime(date_modified,
             date_modified_buf_size * sizeof(gchar),
             "%Ec",
             localtime(&current->mtime));

    gtk_label_set_text(GTK_LABEL(dialog->modified_label), date_modified);

    set_new_pixbuf(
            dialog,
            uni_image_view_get_pixbuf(UNI_IMAGE_VIEW(dialog->window->view)));
    gtk_image_set_from_pixbuf(GTK_IMAGE(dialog->image), dialog->thumbnail);

    width_str = g_strdup_printf("%i px",
                                dialog->window->current_image_width);
    height_str = g_strdup_printf("%i px",
                                 dialog->window->current_image_height);

    gtk_label_set_text(GTK_LABEL(dialog->width_label), width_str);
    gtk_label_set_text(GTK_LABEL(dialog->height_label), height_str);

    g_free(width_str);
    g_free(height_str);
}

void vnr_propsdlg_clear(VnrPropertiesDialog *dialog)
{
    set_new_pixbuf(dialog, NULL);
    vnr_propsdlg_clear_metadata(dialog);

    gtk_label_set_text(GTK_LABEL(dialog->location_label), _("None"));
    gtk_label_set_text(GTK_LABEL(dialog->name_label), _("None"));
    gtk_label_set_text(GTK_LABEL(dialog->type_label), _("None"));
    gtk_label_set_text(GTK_LABEL(dialog->size_label), _("None"));
    gtk_label_set_text(GTK_LABEL(dialog->width_label), _("None"));
    gtk_label_set_text(GTK_LABEL(dialog->height_label), _("None"));
    gtk_label_set_text(GTK_LABEL(dialog->modified_label), _("None"));
}

void vnr_propsdlg_show(VnrPropertiesDialog *dialog)
{
    vnr_propsdlg_update(dialog);
    gtk_window_present(GTK_WINDOW(dialog));
}


