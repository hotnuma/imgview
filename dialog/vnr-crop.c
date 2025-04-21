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
#include "vnr-crop.h"

#include "uni-image-view.h"
#include "uni-utils.h"
#include "vnr-tools.h"

#define VNR_LINE_WIDTH 1

G_DEFINE_TYPE(VnrCrop, vnr_crop, G_TYPE_OBJECT)

static void vnr_crop_dispose(GObject *gobject);
static GtkWidget* _vnr_crop_dlg_new(VnrCrop *crop);

static gboolean _on_draw(GtkWidget *widget,
                                   cairo_t *cr, VnrCrop *crop);
static gboolean _on_button_press_event(GtkWidget *widget,
                                         GdkEventButton *event, VnrCrop *crop);
static gboolean _on_button_release_event(GtkWidget *widget,
                                           GdkEventButton *event, VnrCrop *crop);
static gboolean _on_motion_notify_event(GtkWidget *widget,
                                   GdkEventMotion *event, VnrCrop *crop);
static void _on_x_value_changed(GtkSpinButton *spinbutton, VnrCrop *crop);
static void _on_width_value_changed(GtkSpinButton *spinbutton, VnrCrop *crop);
static void _on_y_value_changed(GtkSpinButton *spinbutton, VnrCrop *crop);
static void _on_height_value_changed(GtkSpinButton *spinbutton, VnrCrop *crop);

static void _vnr_crop_draw_rectangle(VnrCrop *crop);
static inline void vnr_crop_clear_rectangle(VnrCrop *crop);
static void vnr_crop_check_sub_x(VnrCrop *crop);
static void vnr_crop_check_sub_y(VnrCrop *crop);
static void vnr_crop_update_spin_button_values(VnrCrop *crop);


// creation -------------------------------------------------------------------

GObject* vnr_crop_new(VnrWindow *vnr_win)
{
    VnrCrop *crop = g_object_new(VNR_TYPE_CROP, NULL);

    crop->window = vnr_win;

    return (GObject*) crop;
}

static void vnr_crop_class_init(VnrCropClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->dispose = vnr_crop_dispose;
}

static void vnr_crop_init(VnrCrop *crop)
{
    crop->drawing_rectangle = FALSE;
    crop->do_redraw = TRUE;

    crop->sub_x = -1;
    crop->sub_y = -1;
    crop->sub_height = -1;
    crop->sub_width = -1;
    crop->height = -1;
    crop->width = -1;

    crop->image = NULL;
    crop->spin_x = NULL;
    crop->spin_y = NULL;
    crop->spin_width = NULL;
    crop->spin_height = NULL;
    crop->preview_pixbuf = NULL;
}

static void vnr_crop_dispose(GObject *gobject)
{
    VnrCrop *self = VNR_CROP(gobject);

    if (self->preview_pixbuf != NULL)
        g_object_unref(self->preview_pixbuf);

    G_OBJECT_CLASS(vnr_crop_parent_class)->dispose(gobject);
}

gboolean vnr_crop_run(VnrCrop *crop)
{
    GtkWidget *dialog = _vnr_crop_dlg_new(crop);

    if (!dialog)
        return FALSE;

    gint ret = gtk_dialog_run(GTK_DIALOG(dialog));

    crop->area.x = gtk_spin_button_get_value_as_int(crop->spin_x);
    crop->area.y = gtk_spin_button_get_value_as_int(crop->spin_y);
    crop->area.width = gtk_spin_button_get_value_as_int(crop->spin_width);
    crop->area.height = gtk_spin_button_get_value_as_int(crop->spin_height);

    gtk_widget_destroy(dialog);

    if (crop->area.x == 0
        && crop->area.y == 0
        && crop->area.width == crop->window->current_image_width
        && crop->area.height == crop->window->current_image_height)
    {
        return FALSE;
    }
    else
    {
        return (ret == GTK_RESPONSE_ACCEPT);
    }
}

static GtkWidget* _vnr_crop_dlg_new(VnrCrop *crop)
{
    GtkWidget *dialog = g_object_new(GTK_TYPE_DIALOG,
                                     "border-width", 5,
                                     "title", _("Crop Image"),
                                     "resizable", false,
                                     "modal", true,
                                     NULL);

    gtk_window_set_transient_for(GTK_WINDOW(dialog),
                                 GTK_WINDOW(crop->window));

    GtkWidget *vbox0 = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(vbox0), vbox1);

    // ------------------------------------------------------------------------

    GdkPixbuf *original = uni_image_view_get_pixbuf(
                                UNI_IMAGE_VIEW(crop->window->view));

    GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(crop->window));
    GdkDisplay *display = gdk_screen_get_display(screen);
    GdkMonitor *monitor = gdk_display_get_monitor_at_window(
                        display,
                        gtk_widget_get_window(GTK_WIDGET(crop->window)));

    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);

    gint max_width = geometry.width * 0.9 - 100;
    gint max_height = geometry.height * 0.9 - 200;

    gdouble width = crop->window->current_image_width;
    gdouble height = crop->window->current_image_height;
    vnr_tools_fit_to_size_double(&width, &height, max_width, max_height);

    crop->width = width;
    crop->height = height;

    crop->zoom = (width / crop->window->current_image_width
                  + height / crop->window->current_image_height) / 2;

    GdkPixbuf *preview = gdk_pixbuf_new(
                                gdk_pixbuf_get_colorspace(original),
                                gdk_pixbuf_get_has_alpha(original),
                                gdk_pixbuf_get_bits_per_sample(original),
                                width,
                                height);

    uni_pixbuf_scale_blend(original, preview, 0, 0, width, height, 0, 0,
                           crop->zoom, GDK_INTERP_BILINEAR, 0, 0);
    crop->preview_pixbuf = preview;

    crop->image = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox1), crop->image, false, false, 0);
    gtk_widget_set_size_request(crop->image, width, height);

    // ------------------------------------------------------------------------

    GtkWidget *widget = NULL;

    // bottom grid
    GtkWidget *toolbox = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(vbox1), toolbox, true, true, 0);
    gtk_widget_set_halign(toolbox, GTK_ALIGN_CENTER);
    gtk_grid_set_column_spacing(GTK_GRID(toolbox), 8);
    gtk_grid_set_row_spacing(GTK_GRID(toolbox), 8);

    widget = gtk_label_new("X: ");
    gtk_grid_attach(GTK_GRID(toolbox), widget, 0, 0, 1, 1);

    crop->spin_x = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(
                                    0,
                                    crop->window->current_image_width - 1,
                                    1));
    gtk_spin_button_set_increments(crop->spin_x, 1, 10);
    gtk_grid_attach(GTK_GRID(toolbox), GTK_WIDGET(crop->spin_x), 1, 0, 1, 1);

    widget = gtk_label_new("Width: ");
    gtk_grid_attach(GTK_GRID(toolbox), widget, 2, 0, 1, 1);

    crop->spin_width = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(
                                    1,
                                    crop->window->current_image_width,
                                    1));
    gtk_spin_button_set_increments(crop->spin_width, 1, 10);
    gtk_spin_button_set_value(crop->spin_width,
                              crop->window->current_image_width);
    gtk_grid_attach(GTK_GRID(toolbox),
                    GTK_WIDGET(crop->spin_width), 3, 0, 1, 1);

    widget = gtk_label_new("Y: ");
    gtk_grid_attach(GTK_GRID(toolbox), widget, 0, 1, 1, 1);

    crop->spin_y = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(
                                    0,
                                    crop->window->current_image_height - 1,
                                    1));
    gtk_spin_button_set_increments(crop->spin_y, 1, 10);
    gtk_grid_attach(GTK_GRID(toolbox), GTK_WIDGET(crop->spin_y), 1, 1, 1, 1);

    widget = gtk_label_new("Height: ");
    gtk_grid_attach(GTK_GRID(toolbox), widget, 2, 1, 1, 1);

    crop->spin_height = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(
                                    1,
                                    crop->window->current_image_height,
                                    1));
    gtk_spin_button_set_increments(crop->spin_height, 1, 10);
    gtk_spin_button_set_value(crop->spin_height,
                              crop->window->current_image_height);
    gtk_grid_attach(GTK_GRID(toolbox),
                    GTK_WIDGET(crop->spin_height), 3, 1, 1, 1);

    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                           _("Cancel"), GTK_RESPONSE_CANCEL,
                           _("Crop"), GTK_RESPONSE_ACCEPT,
                           NULL);

    gtk_widget_set_events(crop->image,
                          GDK_BUTTON_PRESS_MASK
                          | GDK_BUTTON_RELEASE_MASK
                          | GDK_BUTTON_MOTION_MASK);

    g_signal_connect(crop->image, "draw",
                     G_CALLBACK(_on_draw), crop);
    g_signal_connect(crop->image, "button-press-event",
                     G_CALLBACK(_on_button_press_event), crop);
    g_signal_connect(crop->image, "button-release-event",
                     G_CALLBACK(_on_button_release_event), crop);
    g_signal_connect(crop->image, "motion-notify-event",
                     G_CALLBACK(_on_motion_notify_event), crop);

    g_signal_connect(crop->spin_x, "value-changed",
                     G_CALLBACK(_on_x_value_changed), crop);
    g_signal_connect(crop->spin_width, "value-changed",
                     G_CALLBACK(_on_width_value_changed), crop);
    g_signal_connect(crop->spin_y, "value-changed",
                     G_CALLBACK(_on_y_value_changed), crop);
    g_signal_connect(crop->spin_height, "value-changed",
                     G_CALLBACK(_on_height_value_changed), crop);

    gtk_widget_show_all(dialog);

    return dialog;
}

// ----------------------------------------------------------------------------

static gboolean _on_draw(GtkWidget *widget, cairo_t *cr, VnrCrop *crop)
{
    (void) widget;

    cairo_save(cr);

    gdk_cairo_set_source_pixbuf(cr, crop->preview_pixbuf, 0, 0);
    cairo_paint(cr);

    if (crop->sub_width == -1)
    {
        crop->sub_x = 0;
        crop->sub_y = 0;
        crop->sub_width = crop->width;
        crop->sub_height = crop->height;
    }

    cairo_restore(cr);

    vnr_crop_clear_rectangle(crop);

    return FALSE;
}

static gboolean _on_button_press_event(GtkWidget *widget,
                                      GdkEventButton *event, VnrCrop *crop)
{
    (void) widget;

    if (event->button != 1)
        return FALSE;

    crop->drawing_rectangle = TRUE;
    crop->start_x = event->x;
    crop->start_y = event->y;

    return FALSE;
}

static gboolean _on_button_release_event(GtkWidget *widget,
                                        GdkEventButton *event, VnrCrop *crop)
{
    (void) widget;

    if (event->button != 1)
        return FALSE;

    crop->drawing_rectangle = FALSE;

    gtk_spin_button_set_range(crop->spin_width, 1,
                              (crop->width - crop->sub_x) / crop->zoom);
    gtk_spin_button_set_range(crop->spin_height, 1,
                              (crop->height - crop->sub_y) / crop->zoom);

    vnr_crop_update_spin_button_values(crop);

    return FALSE;
}

static gboolean _on_motion_notify_event(GtkWidget *widget,
                                       GdkEventMotion *event, VnrCrop *crop)
{
    (void) widget;

    if (!crop->drawing_rectangle)
        return FALSE;

    gdouble x = event->x;
    gdouble y = event->y;

    x = CLAMP(x, 0, crop->width);
    y = CLAMP(y, 0, crop->height);

    vnr_crop_clear_rectangle(crop);

    if (x > crop->start_x)
    {
        crop->sub_x = crop->start_x;
        crop->sub_width = x - crop->start_x;
    }
    else if (x == crop->start_x)
    {
        crop->sub_x = x;
        crop->sub_width = 1;
    }
    else
    {
        crop->sub_x = x;
        crop->sub_width = crop->start_x - x;
    }

    if (y > crop->start_y)
    {
        crop->sub_y = crop->start_y;
        crop->sub_height = y - crop->start_y;
    }
    else if (y == crop->start_y)
    {
        crop->sub_y = y;
        crop->sub_height = 1;
    }
    else
    {
        crop->sub_y = y;
        crop->sub_height = crop->start_y - y;
    }

    crop->drawing_rectangle = FALSE;
    crop->do_redraw = FALSE;

    vnr_crop_update_spin_button_values(crop);

    crop->drawing_rectangle = TRUE;
    crop->do_redraw = TRUE;

    _vnr_crop_draw_rectangle(crop);

    return FALSE;
}

static void vnr_crop_update_spin_button_values(VnrCrop *crop)
{
    gtk_spin_button_set_value(crop->spin_height, crop->sub_height / crop->zoom);
    gtk_spin_button_set_value(crop->spin_width, crop->sub_width / crop->zoom);

    gtk_spin_button_set_value(crop->spin_x, crop->sub_x / crop->zoom);
    gtk_spin_button_set_value(crop->spin_y, crop->sub_y / crop->zoom);
}

static void _on_x_value_changed(GtkSpinButton *spinbutton, VnrCrop *crop)
{
    if (crop->drawing_rectangle)
        return;

    vnr_crop_clear_rectangle(crop);

    gboolean old_do_redraw = crop->do_redraw;

    crop->do_redraw = FALSE;

    gtk_spin_button_set_range(
        crop->spin_width,
        1,
        crop->window->current_image_width
            - gtk_spin_button_get_value(spinbutton));

    crop->do_redraw = old_do_redraw;

    crop->sub_x = gtk_spin_button_get_value(spinbutton) * crop->zoom;

    vnr_crop_check_sub_x(crop);

    _vnr_crop_draw_rectangle(crop);
}

static void vnr_crop_check_sub_x(VnrCrop *crop)
{
    if (gtk_spin_button_get_value(crop->spin_width) + gtk_spin_button_get_value(crop->spin_x) == crop->window->current_image_width)
    {
        crop->sub_x = (int)crop->width - (int)crop->sub_width;
    }
}

static void _on_width_value_changed(GtkSpinButton *spinbutton, VnrCrop *crop)
{
    if (crop->drawing_rectangle)
        return;

    vnr_crop_clear_rectangle(crop);

    crop->sub_width = gtk_spin_button_get_value(spinbutton) * crop->zoom;

    if (crop->sub_width < 1)
        crop->sub_width = 1;

    _vnr_crop_draw_rectangle(crop);
}

static void _on_y_value_changed(GtkSpinButton *spinbutton, VnrCrop *crop)
{
    if (crop->drawing_rectangle)
        return;

    vnr_crop_clear_rectangle(crop);

    gboolean old_do_redraw = crop->do_redraw;

    crop->do_redraw = FALSE;
    gtk_spin_button_set_range(
        crop->spin_height,
        1,
        crop->window->current_image_height
            - gtk_spin_button_get_value(spinbutton));
    crop->do_redraw = old_do_redraw;

    crop->sub_y = gtk_spin_button_get_value(spinbutton) * crop->zoom;

    vnr_crop_check_sub_y(crop);

    _vnr_crop_draw_rectangle(crop);
}

static void vnr_crop_check_sub_y(VnrCrop *crop)
{
    if (gtk_spin_button_get_value(crop->spin_height) + gtk_spin_button_get_value(crop->spin_y) == crop->window->current_image_height)
    {
        crop->sub_y = (int)crop->height - (int)crop->sub_height;
    }
}

static void _on_height_value_changed(GtkSpinButton *spinbutton, VnrCrop *crop)
{
    if (crop->drawing_rectangle)
        return;

    vnr_crop_clear_rectangle(crop);

    crop->sub_height = gtk_spin_button_get_value(spinbutton) * crop->zoom;

    if (crop->sub_height < 1)
        crop->sub_height = 1;

    _vnr_crop_draw_rectangle(crop);
}

// ----------------------------------------------------------------------------

static void _vnr_crop_draw_rectangle(VnrCrop *crop)
{
    if (!crop->do_redraw)
        return;

    GdkWindow *window = gtk_widget_get_window(crop->image);

    cairo_region_t *region = cairo_region_create();
    GdkDrawingContext *context;
    context = gdk_window_begin_draw_frame(window,region);
    cairo_t *cr = gdk_drawing_context_get_cairo_context(context);

    cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
    cairo_set_line_width(cr, VNR_LINE_WIDTH);
    cairo_rectangle(cr,
                    (int) crop->sub_x + 0.5,
                    (int) crop->sub_y + 0.5,
                    (int) crop->sub_width,
                    (int) crop->sub_height);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_stroke(cr);

    gdk_window_end_draw_frame(window, context);
    cairo_region_destroy(region);

    if (uni_is_wayland())
        gdk_window_invalidate_rect(window, NULL, TRUE);
}

static inline void vnr_crop_clear_rectangle(VnrCrop *crop)
{
    _vnr_crop_draw_rectangle(crop);
}


