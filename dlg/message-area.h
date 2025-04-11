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

#ifndef __VNR_MESSAGE_AREA_H__
#define __VNR_MESSAGE_AREA_H__

#include "window.h"

G_BEGIN_DECLS

typedef struct _VnrMessageArea VnrMessageArea;

#define VNR_TYPE_MESSAGE_AREA (vnr_message_area_get_type())

G_DECLARE_FINAL_TYPE(VnrMessageArea,
                     vnr_message_area, VNR, MESSAGE_AREA, GtkEventBox)

struct _VnrMessageArea
{
    GtkEventBox __parent__;

    VnrWindow *vnrwindow;
    GtkWidget *hbox;

    GtkWidget *image;
    GtkWidget *message;

    GtkWidget *button_box;
    GtkWidget *user_button;
    GtkWidget *cancel_button;
    GCallback c_handler;
    gboolean with_button;

    gboolean initialized;
    gboolean is_critical;
};

GType vnr_message_area_get_type() G_GNUC_CONST;

GtkWidget *vnr_message_area_new();

void vnr_message_area_show(VnrMessageArea *msg_area,
                           gboolean critical,
                           const char *message,
                           gboolean close_image);

void vnr_message_area_show_with_button(VnrMessageArea *msg_area,
                                       gboolean critical,
                                       const char *message,
                                       gboolean close_image,
                                       const gchar *button_stock_id,
                                       GCallback c_handler);

void vnr_message_area_hide(VnrMessageArea *msg_area);

gboolean vnr_message_area_is_critical(VnrMessageArea *msg_area);
gboolean vnr_message_area_is_visible(VnrMessageArea *msg_area);

G_END_DECLS

#endif // __VNR_MESSAGE_AREA_H__


