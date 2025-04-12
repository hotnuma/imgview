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

#include "vnr-resize.h"
#include "vnr-tools.h"
#include "uni-utils.h"
#include "uni-image-view.h"

G_DEFINE_TYPE(VnrResize, vnr_resize, G_TYPE_OBJECT)

static GtkWidget* _vnr_resize_dlg_new(VnrResize *resize);
static void vnr_resize_dispose(GObject *gobject);

static void _on_x_value_changed(GtkSpinButton *spinbutton, VnrResize *resize);
static void _on_width_value_changed(GtkSpinButton *spinbutton, VnrResize *resize);
static void _on_y_value_changed(GtkSpinButton *spinbutton, VnrResize *resize);
static void _on_height_value_changed(GtkSpinButton *spinbutton, VnrResize *resize);

//static gboolean _on_draw(GtkWidget *widget,
//                                   cairo_t *cr, VnrResize *resize);
//static gboolean _on_button_press_event(GtkWidget *widget,
//                                         GdkEventButton *event, VnrResize *resize);
//static gboolean _on_button_release_event(GtkWidget *widget,
//                                           GdkEventButton *event, VnrResize *resize);
//static gboolean _on_motion_notify_event(GtkWidget *widget,
//                                   GdkEventMotion *event, VnrResize *resize);
//static void _vnr_resize_draw_rectangle(VnrResize *resize);
//static inline void vnr_resize_clear_rectangle(VnrResize *resize);
//static void vnr_resize_check_sub_x(VnrResize *resize);
//static void vnr_resize_check_sub_y(VnrResize *resize);
//static void vnr_resize_update_spin_button_values(VnrResize *resize);


// creation -------------------------------------------------------------------

GObject* vnr_resize_new(VnrWindow *window)
{
    VnrResize *resize = g_object_new(VNR_TYPE_RESIZE, NULL);

    resize->window = window;

    return (GObject*) resize;
}

static void vnr_resize_class_init(VnrResizeClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->dispose = vnr_resize_dispose;
}

static void vnr_resize_init(VnrResize *resize)
{
    resize->spin_x = NULL;
    resize->spin_y = NULL;
    resize->spin_width = NULL;
    resize->spin_height = NULL;

//    resize->drawing_rectangle = FALSE;
//    resize->do_redraw = TRUE;

//    resize->sub_x = -1;
//    resize->sub_y = -1;
//    resize->sub_height = -1;
//    resize->sub_width = -1;
//    resize->height = -1;
//    resize->width = -1;

//    resize->image = NULL;
//    resize->preview_pixbuf = NULL;
}

static void vnr_resize_dispose(GObject *gobject)
{
//    VnrResize *self = VNR_RESIZE(gobject);

//    if (self->preview_pixbuf != NULL)
//        g_object_unref(self->preview_pixbuf);

    G_OBJECT_CLASS(vnr_resize_parent_class)->dispose(gobject);
}

gboolean vnr_resize_run(VnrResize *resize)
{
    GtkWidget *dialog = _vnr_resize_dlg_new(resize);

    if (!dialog)
        return FALSE;

    gint ret = gtk_dialog_run(GTK_DIALOG(dialog));

//    resize->area.x = gtk_spin_button_get_value_as_int(resize->spin_x);
//    resize->area.y = gtk_spin_button_get_value_as_int(resize->spin_y);
//    resize->area.width = gtk_spin_button_get_value_as_int(resize->spin_width);
//    resize->area.height = gtk_spin_button_get_value_as_int(resize->spin_height);

    gtk_widget_destroy(dialog);

    //if (resize->area.x == 0
    //    && resize->area.y == 0
    //    && resize->area.width == resize->window->current_image_width
    //    && resize->area.height == resize->window->current_image_height)
    //{
    //    return FALSE;
    //}
    //else
    //{
    //    return (ret == GTK_RESPONSE_ACCEPT);
    //}

    return (ret == GTK_RESPONSE_ACCEPT);
}

static GtkWidget* _vnr_resize_dlg_new(VnrResize *resize)
{
    GtkWidget *dialog = g_object_new(GTK_TYPE_DIALOG,
                                     "border-width", 5,
                                     "title", "Resize Image",
                                     "resizable", false,
                                     "modal", true,
                                     NULL);

    gtk_window_set_transient_for(GTK_WINDOW(dialog),
                                 GTK_WINDOW(resize->window));

    gdouble width = resize->window->current_image_width;
    gdouble height = resize->window->current_image_height;

    resize->width = width;
    resize->height = height;

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_box_pack_start(GTK_BOX(content), grid, true, true, 0);

    int row = 0;
    GtkWidget *widget = NULL;

    widget = gtk_label_new("X: ");
    gtk_grid_attach(GTK_GRID(grid), widget, 0, row, 1, 1);

    resize->spin_x = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(
                                     0,
                                     resize->window->current_image_width - 1,
                                     1));
    gtk_spin_button_set_increments(resize->spin_x, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(resize->spin_x), 1, row, 1, 1);

    widget = gtk_label_new("Width: ");
    gtk_grid_attach(GTK_GRID(grid), widget, 2, row, 1, 1);

    resize->spin_width = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(
                                    1,
                                    resize->window->current_image_width,
                                    1));
    gtk_spin_button_set_increments(resize->spin_width, 1, 10);
    gtk_spin_button_set_value(resize->spin_width,
                              resize->window->current_image_width);
    gtk_grid_attach(GTK_GRID(grid),
                    GTK_WIDGET(resize->spin_width), 3, row, 1, 1);

    ++row;

    widget = gtk_label_new("Y: ");
    gtk_grid_attach(GTK_GRID(grid), widget, 0, row, 1, 1);

    resize->spin_y = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(
                                    0,
                                    resize->window->current_image_height - 1,
                                    1));
    gtk_spin_button_set_increments(resize->spin_y, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(resize->spin_y), 1, row, 1, 1);

    widget = gtk_label_new("Height: ");
    gtk_grid_attach(GTK_GRID(grid), widget, 2, row, 1, 1);

    resize->spin_height = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(
                                    1,
                                    resize->window->current_image_height,
                                    1));
    gtk_spin_button_set_increments(resize->spin_height, 1, 10);
    gtk_spin_button_set_value(resize->spin_height,
                              resize->window->current_image_height);
    gtk_grid_attach(GTK_GRID(grid),
                    GTK_WIDGET(resize->spin_height), 3, row, 1, 1);

    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                           _("Cancel"), GTK_RESPONSE_CANCEL,
                           _("Crop"), GTK_RESPONSE_ACCEPT,
                           NULL);

    //gtk_widget_set_events(resize->image,
    //                      GDK_BUTTON_PRESS_MASK
    //                      | GDK_BUTTON_RELEASE_MASK
    //                      | GDK_BUTTON_MOTION_MASK);

    //g_signal_connect(resize->image, "draw",
    //                 G_CALLBACK(_on_draw), resize);
    //g_signal_connect(resize->image, "button-press-event",
    //                 G_CALLBACK(_on_button_press_event), resize);
    //g_signal_connect(resize->image, "button-release-event",
    //                 G_CALLBACK(_on_button_release_event), resize);
    //g_signal_connect(resize->image, "motion-notify-event",
    //                 G_CALLBACK(_on_motion_notify_event), resize);

    g_signal_connect(resize->spin_x, "value-changed",
                     G_CALLBACK(_on_x_value_changed), resize);
    g_signal_connect(resize->spin_width, "value-changed",
                     G_CALLBACK(_on_width_value_changed), resize);
    g_signal_connect(resize->spin_y, "value-changed",
                     G_CALLBACK(_on_y_value_changed), resize);
    g_signal_connect(resize->spin_height, "value-changed",
                     G_CALLBACK(_on_height_value_changed), resize);

    gtk_widget_show_all(dialog);

    return dialog;
}

static void _on_x_value_changed(GtkSpinButton *spinbutton, VnrResize *resize)
{
//    if (resize->drawing_rectangle)
//        return;

//    vnr_resize_clear_rectangle(resize);

//    gboolean old_do_redraw = resize->do_redraw;

//    resize->do_redraw = FALSE;

//    gtk_spin_button_set_range(
//        resize->spin_width,
//        1,
//        resize->window->current_image_width
//            - gtk_spin_button_get_value(spinbutton));

//    resize->do_redraw = old_do_redraw;

//    resize->sub_x = gtk_spin_button_get_value(spinbutton) * resize->zoom;

//    vnr_resize_check_sub_x(resize);

//    _vnr_resize_draw_rectangle(resize);
}

//static void vnr_resize_check_sub_x(VnrResize *resize)
//{
//    if (gtk_spin_button_get_value(resize->spin_width) + gtk_spin_button_get_value(resize->spin_x) == resize->window->current_image_width)
//    {
//        resize->sub_x = (int)resize->width - (int)resize->sub_width;
//    }
//}

static void _on_width_value_changed(GtkSpinButton *spinbutton, VnrResize *resize)
{
//    if (resize->drawing_rectangle)
//        return;

//    vnr_resize_clear_rectangle(resize);

//    resize->sub_width = gtk_spin_button_get_value(spinbutton) * resize->zoom;

//    if (resize->sub_width < 1)
//        resize->sub_width = 1;

//    _vnr_resize_draw_rectangle(resize);
}

static void _on_y_value_changed(GtkSpinButton *spinbutton, VnrResize *resize)
{
//    if (resize->drawing_rectangle)
//        return;

//    vnr_resize_clear_rectangle(resize);

//    gboolean old_do_redraw = resize->do_redraw;

//    resize->do_redraw = FALSE;
//    gtk_spin_button_set_range(
//        resize->spin_height,
//        1,
//        resize->window->current_image_height
//            - gtk_spin_button_get_value(spinbutton));
//    resize->do_redraw = old_do_redraw;

//    resize->sub_y = gtk_spin_button_get_value(spinbutton) * resize->zoom;

//    vnr_resize_check_sub_y(resize);

//    _vnr_resize_draw_rectangle(resize);
}

//static void vnr_resize_check_sub_y(VnrResize *resize)
//{
//    if (gtk_spin_button_get_value(resize->spin_height) + gtk_spin_button_get_value(resize->spin_y) == resize->window->current_image_height)
//    {
//        resize->sub_y = (int)resize->height - (int)resize->sub_height;
//    }
//}

static void _on_height_value_changed(GtkSpinButton *spinbutton, VnrResize *resize)
{
//    if (resize->drawing_rectangle)
//        return;

//    vnr_resize_clear_rectangle(resize);

//    resize->sub_height = gtk_spin_button_get_value(spinbutton) * resize->zoom;

//    if (resize->sub_height < 1)
//        resize->sub_height = 1;

//    _vnr_resize_draw_rectangle(resize);
}


// ----------------------------------------------------------------------------

#if 0
static gboolean _on_draw(GtkWidget *widget, cairo_t *cr, VnrResize *resize)
{
    (void) widget;

    cairo_save(cr);

    gdk_cairo_set_source_pixbuf(cr, resize->preview_pixbuf, 0, 0);
    cairo_paint(cr);

    if (resize->sub_width == -1)
    {
        resize->sub_x = 0;
        resize->sub_y = 0;
        resize->sub_width = resize->width;
        resize->sub_height = resize->height;
    }

    cairo_restore(cr);

    vnr_resize_clear_rectangle(resize);

    return FALSE;
}

static gboolean _on_button_press_event(GtkWidget *widget,
                                      GdkEventButton *event, VnrResize *resize)
{
    (void) widget;

    if (event->button != 1)
        return FALSE;

    resize->drawing_rectangle = TRUE;
    resize->start_x = event->x;
    resize->start_y = event->y;

    return FALSE;
}

static gboolean _on_button_release_event(GtkWidget *widget,
                                        GdkEventButton *event, VnrResize *resize)
{
    (void) widget;

    if (event->button != 1)
        return FALSE;

    resize->drawing_rectangle = FALSE;

    gtk_spin_button_set_range(resize->spin_width, 1,
                              (resize->width - resize->sub_x) / resize->zoom);
    gtk_spin_button_set_range(resize->spin_height, 1,
                              (resize->height - resize->sub_y) / resize->zoom);

    vnr_resize_update_spin_button_values(resize);

    return FALSE;
}

static gboolean _on_motion_notify_event(GtkWidget *widget,
                                       GdkEventMotion *event, VnrResize *resize)
{
    (void) widget;

    if (!resize->drawing_rectangle)
        return FALSE;

    gdouble x, y;
    x = event->x;
    y = event->y;

    x = CLAMP(x, 0, resize->width);
    y = CLAMP(y, 0, resize->height);

    vnr_resize_clear_rectangle(resize);

    if (x > resize->start_x)
    {
        resize->sub_x = resize->start_x;
        resize->sub_width = x - resize->start_x;
    }
    else if (x == resize->start_x)
    {
        resize->sub_x = x;
        resize->sub_width = 1;
    }
    else
    {
        resize->sub_x = x;
        resize->sub_width = resize->start_x - x;
    }

    if (y > resize->start_y)
    {
        resize->sub_y = resize->start_y;
        resize->sub_height = y - resize->start_y;
    }
    else if (y == resize->start_y)
    {
        resize->sub_y = y;
        resize->sub_height = 1;
    }
    else
    {
        resize->sub_y = y;
        resize->sub_height = resize->start_y - y;
    }

    resize->drawing_rectangle = FALSE;
    resize->do_redraw = FALSE;

    vnr_resize_update_spin_button_values(resize);

    resize->drawing_rectangle = TRUE;
    resize->do_redraw = TRUE;

    _vnr_resize_draw_rectangle(resize);

    return FALSE;
}

static void vnr_resize_update_spin_button_values(VnrResize *resize)
{
    gtk_spin_button_set_value(resize->spin_height, resize->sub_height / resize->zoom);
    gtk_spin_button_set_value(resize->spin_width, resize->sub_width / resize->zoom);

    gtk_spin_button_set_value(resize->spin_x, resize->sub_x / resize->zoom);
    gtk_spin_button_set_value(resize->spin_y, resize->sub_y / resize->zoom);
}

// ----------------------------------------------------------------------------

static void _vnr_resize_draw_rectangle(VnrResize *resize)
{
    if (!resize->do_redraw)
        return;

    GdkWindow *window = gtk_widget_get_window(resize->image);

    //cairo_t *cr = gdk_cairo_create(window);

    cairo_region_t *region = cairo_region_create();
    GdkDrawingContext *context;
    context = gdk_window_begin_draw_frame(window,region);
    cairo_t *cr = gdk_drawing_context_get_cairo_context(context);

    cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
    cairo_set_line_width(cr, 3);
    cairo_rectangle(cr,
                    (int) resize->sub_x + 0.5,
                    (int) resize->sub_y + 0.5,
                    (int) resize->sub_width,
                    (int) resize->sub_height);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_stroke(cr);

    //cairo_destroy(cr);

    gdk_window_end_draw_frame(window, context);
    cairo_region_destroy(region);
}

static inline void vnr_resize_clear_rectangle(VnrResize *resize)
{
    _vnr_resize_draw_rectangle(resize);
}

#endif


