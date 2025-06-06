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
#include "message-area.h"

static void _vnr_message_area_show_basic(VnrMessageArea *msg_area,
                                        gboolean critical,
                                        const char *message,
                                        gboolean close_image);
static void _vnr_message_area_initialize(VnrMessageArea *msg_area);
static void cancel_button_cb(GtkWidget *widget, VnrMessageArea *msg_area);


// creation -------------------------------------------------------------------

G_DEFINE_TYPE(VnrMessageArea, vnr_message_area, GTK_TYPE_EVENT_BOX)

GtkWidget* vnr_message_area_new()
{
    return (GtkWidget*) g_object_new(VNR_TYPE_MESSAGE_AREA, NULL);
}

static void vnr_message_area_class_init(VnrMessageAreaClass *klass)
{
}

static void vnr_message_area_init(VnrMessageArea *msg_area)
{
    msg_area->initialized = FALSE;
}


// public ---------------------------------------------------------------------

void vnr_message_area_show(VnrMessageArea *msg_area, gboolean critical,
                           const char *message, gboolean close_image)
{
    _vnr_message_area_show_basic(msg_area, critical, message, close_image);

    if (msg_area->with_button)
    {
        g_signal_handlers_disconnect_by_func(msg_area->user_button,
                                             msg_area->c_handler,
                                             msg_area);
        msg_area->with_button = FALSE;
    }

    gtk_widget_show_all(GTK_WIDGET(msg_area));
    gtk_widget_hide(GTK_WIDGET(msg_area->button_box));
}

static void _vnr_message_area_show_basic(VnrMessageArea *msg_area,
                                        gboolean critical,
                                        const char *message,
                                        gboolean close_image)
{
    // remove the file monitor early ?

    char *warning;

    if (!msg_area->initialized)
    {
        _vnr_message_area_initialize(msg_area);
    }

    msg_area->is_critical = critical;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    if (critical)
    {
        gtk_image_set_from_stock(GTK_IMAGE(msg_area->image),
                                 "gtk-dialog-error",
                                 GTK_ICON_SIZE_DIALOG);
    }
    else
    {
        gtk_image_set_from_stock(GTK_IMAGE(msg_area->image),
                                 "gtk-dialog-info",
                                 GTK_ICON_SIZE_DIALOG);
    }
    G_GNUC_END_IGNORE_DEPRECATIONS

    msg_area->is_critical = critical;

    warning = g_markup_printf_escaped("<span weight=\"bold\">%s</span>",
                                      message);

    gtk_label_set_markup(GTK_LABEL(msg_area->message), warning);

    g_free(warning);

    if (close_image == TRUE)
        window_close_file(msg_area->vnrwindow);
}

static void _vnr_message_area_initialize(VnrMessageArea *msg_area)
{
    msg_area->with_button = FALSE;

    msg_area->hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 7);

    gtk_container_add(GTK_CONTAINER(msg_area), msg_area->hbox);
    gtk_container_set_border_width(GTK_CONTAINER(msg_area->hbox), 7);

    msg_area->image = gtk_image_new();
    gtk_box_pack_start(GTK_BOX(msg_area->hbox), msg_area->image,
                       FALSE, FALSE, 0);

    msg_area->message = gtk_label_new(NULL);
    gtk_label_set_line_wrap(GTK_LABEL(msg_area->message), TRUE);
    gtk_label_set_selectable(GTK_LABEL(msg_area->message), TRUE);
    gtk_box_pack_start(GTK_BOX(msg_area->hbox), msg_area->message,
                       FALSE, FALSE, 0);

    msg_area->button_box = gtk_button_box_new(GTK_ORIENTATION_VERTICAL);

    gtk_box_pack_end(GTK_BOX(msg_area->hbox), msg_area->button_box,
                     FALSE, FALSE, 0);

    msg_area->user_button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(msg_area->button_box),
                      msg_area->user_button);


    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    msg_area->cancel_button = gtk_button_new_from_stock("gtk-cancel");
    G_GNUC_END_IGNORE_DEPRECATIONS

    g_signal_connect(msg_area->cancel_button, "clicked",
                     G_CALLBACK(cancel_button_cb), msg_area);
    gtk_container_add(GTK_CONTAINER(msg_area->button_box),
                      msg_area->cancel_button);

    gtk_widget_hide(msg_area->hbox);

    gtk_widget_set_state_flags(GTK_WIDGET(msg_area),
                               GTK_STATE_FLAG_SELECTED, TRUE);
    gtk_widget_set_state_flags(msg_area->button_box,
                               GTK_STATE_FLAG_NORMAL, TRUE);

    msg_area->initialized = TRUE;
}

static void cancel_button_cb(GtkWidget *widget, VnrMessageArea *msg_area)
{
    vnr_message_area_hide(msg_area);
}

void vnr_message_area_show_with_button(VnrMessageArea *msg_area,
                                       gboolean critical,
                                       const char *message,
                                       gboolean close_image,
                                       const gchar *button_stock_id,
                                       GCallback c_handler)
{
    _vnr_message_area_show_basic(msg_area, critical, message, close_image);

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_button_set_use_stock(GTK_BUTTON(msg_area->user_button), TRUE);
    G_GNUC_END_IGNORE_DEPRECATIONS

    gtk_button_set_label(GTK_BUTTON(msg_area->user_button), button_stock_id);

    if (msg_area->with_button)
        g_signal_handlers_disconnect_by_func(msg_area->user_button,
                                             msg_area->c_handler,
                                             msg_area->vnrwindow);
    else
        msg_area->with_button = TRUE;

    msg_area->c_handler = c_handler;
    g_signal_connect_swapped(msg_area->user_button, "clicked",
                             c_handler, msg_area->vnrwindow);

    gtk_widget_show_all(GTK_WIDGET(msg_area));
}

void vnr_message_area_hide(VnrMessageArea *msg_area)
{
    gtk_widget_hide(GTK_WIDGET(msg_area));

    if (!msg_area->with_button)
        return;

    g_signal_handlers_disconnect_by_func(msg_area->user_button,
                                         msg_area->c_handler,
                                         msg_area->vnrwindow);
    msg_area->with_button = FALSE;
}

gboolean vnr_message_area_is_critical(VnrMessageArea *msg_area)
{
    if (msg_area->initialized
        && gtk_widget_get_visible(GTK_WIDGET(msg_area))
        && msg_area->is_critical)
        return TRUE;
    else
        return FALSE;
}

gboolean vnr_message_area_is_visible(VnrMessageArea *msg_area)
{
    if (msg_area->initialized && gtk_widget_get_visible(GTK_WIDGET(msg_area)))
        return TRUE;
    else
        return FALSE;
}


