/*
 * Copyright © 2009-2018 Siyan Panayotov <contact@siyanpanayotov.com>
 *
 * Based on code by (see README for details):
 * - Björn Lindqvist <bjourne@gmail.com>
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
#include "uni-image-view.h"

#include "uni-dragger.h"
#include "uni-anim-view.h"
#include "uni-marshal.h"
#include "uni-utils.h"
#include "window.h"
#include <math.h>

#define UNI_ZOOM_MIN    0.02
#define UNI_ZOOM_MAX    20.0
#define UNI_ZOOM_STEP   1.1

// clang-format off
#define g_signal_handlers_disconnect_by_data(instance, data) \
    g_signal_handlers_disconnect_matched ((instance), G_SIGNAL_MATCH_DATA, \
                                         0, 0, NULL, NULL, (data))
#define g_signal_handlers_block_by_data(instance, data) \
    g_signal_handlers_block_matched ((instance), G_SIGNAL_MATCH_DATA, \
                                    0, 0, NULL, NULL, (data))
#define g_signal_handlers_unblock_by_data(instance, data) \
    g_signal_handlers_unblock_matched ((instance), G_SIGNAL_MATCH_DATA, \
                                      0, 0, NULL, NULL, (data))
// clang-format on

static void uni_image_view_init_signals(UniImageViewClass *klass);
static void uni_image_view_get_property(GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec);
static void uni_image_view_set_property(GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec);
static void uni_image_view_realize(GtkWidget *widget);
static void uni_image_view_unrealize(GtkWidget *widget);
static void uni_image_view_finalize(GObject *object);

static void widget_size_allocate(GtkWidget *widget, GtkAllocation *alloc);
static VnrWindow* _uni_get_appwindow(GtkWidget *widget);
static GtkWidget* _uni_get_scrollwin(GtkWidget *widget);
static int widget_draw(GtkWidget *widget, cairo_t *cr);
static int widget_button_press(GtkWidget *widget,
                                       GdkEventButton *event);
static int widget_button_release(GtkWidget *widget,
                                         GdkEventButton *ev);
static int widget_motion_notify(GtkWidget *widget, GdkEventMotion *ev);
static int widget_scroll_event(GtkWidget *widget, GdkEventScroll *ev);


static void uni_image_view_scroll(UniImageView *view,
                                  GtkScrollType xscroll,
                                  GtkScrollType yscroll);

static Size _uni_image_view_get_allocated_size(UniImageView *view);
static Size _uni_image_view_get_pixbuf_size(UniImageView *view);
static Size _uni_image_view_get_zoomed_size(UniImageView *view);
static void _uni_image_view_clamp_offset(UniImageView *view,
                                        gdouble *x, gdouble *y);
static void _uni_image_view_update_adjustments(UniImageView *view);

static void _uni_image_view_set_zoom_with_center(UniImageView *view,
                                    gdouble zoom,
                                    gdouble center_x,
                                    gdouble center_y, gboolean is_allocating);
static void _uni_image_view_set_zoom_no_center(UniImageView *view,
                                              gdouble zoom,
                                              gboolean is_allocating);
static void _uni_image_view_zoom_to_fit(UniImageView *view,
                                       gboolean is_allocating);

static void _uni_image_view_draw_background(UniImageView *view,
                                           GdkRectangle *image_area,
                                           Size alloc,
                                           cairo_t *cr);
static int _uni_image_view_repaint_area(UniImageView *view,
                                       GdkRectangle *paint_rect,
                                       cairo_t *cr);

static void _uni_image_view_fast_scroll(UniImageView *view,
                                       int delta_x, int delta_y);
static void _uni_image_view_scroll_to(UniImageView *view,
                                     gdouble offset_x,
                                     gdouble offset_y,
                                     gboolean set_adjustments,
                                     gboolean invalidate);

static gboolean _on_hadj_value_changed(UniImageView *view,
                                                GtkAdjustment *adj);
static gboolean _on_vadj_value_changed(UniImageView *view,
                                                GtkAdjustment *adj);
static void uni_image_view_set_scroll_adjustments(UniImageView *view,
                                                  GtkAdjustment *hadj,
                                                  GtkAdjustment *vadj);

enum
{
    SET_ZOOM,
    ZOOM_IN,
    ZOOM_OUT,
    SET_FITTING,
    SCROLL,
    ZOOM_CHANGED,
    PIXBUF_CHANGED,
    LAST_SIGNAL
};

enum
{
    P_0,
    P_HADJUSTMENT,
    P_VADJUSTMENT,
    P_HSCROLLPOLICY,
    P_VSCROLLPOLICY
};

struct _UniImageViewPrivate
{
    // Properties
    GtkAdjustment *hadjustment;
    GtkAdjustment *vadjustment;

    // GtkScrollablePolicy needs to be checked when
    // driving the scrollable adjustment values
    GtkScrollablePolicy hscroll_policy : 1;
    GtkScrollablePolicy vscroll_policy : 1;
};

static guint uni_image_view_signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE_WITH_CODE(
                UniImageView,
                uni_image_view,
                GTK_TYPE_WIDGET,
                G_ADD_PRIVATE(UniImageView)
                G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, NULL))

GtkWidget* uni_image_view_new()
{
    return g_object_new(UNI_TYPE_IMAGE_VIEW, NULL);
}

static void uni_image_view_class_init(UniImageViewClass *klass)
{
    uni_image_view_init_signals(klass);

    GObjectClass *object_class = (GObjectClass *)klass;
    object_class->set_property = uni_image_view_set_property;
    object_class->get_property = uni_image_view_get_property;
    object_class->finalize = uni_image_view_finalize;

    GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
    widget_class->realize = uni_image_view_realize;
    widget_class->unrealize = uni_image_view_unrealize;

    widget_class->size_allocate = widget_size_allocate;
    widget_class->draw = widget_draw;

    widget_class->button_press_event = widget_button_press;
    widget_class->button_release_event = widget_button_release;
    widget_class->motion_notify_event = widget_motion_notify;
    widget_class->scroll_event = widget_scroll_event;

    klass->set_zoom = uni_image_view_set_zoom;
    klass->zoom_in = uni_image_view_zoom_in;
    klass->zoom_out = uni_image_view_zoom_out;
    klass->set_fitting = uni_image_view_set_fitting;
    klass->scroll = uni_image_view_scroll;
    klass->pixbuf_changed = NULL;

    g_object_class_override_property(object_class,
                                     P_HADJUSTMENT, "hadjustment");
    g_object_class_override_property(object_class,
                                     P_VADJUSTMENT, "vadjustment");
    g_object_class_override_property(object_class,
                                     P_HSCROLLPOLICY, "hscroll-policy");
    g_object_class_override_property(object_class,
                                     P_VSCROLLPOLICY, "vscroll-policy");

    // Set up scrolling.
    klass->set_scroll_adjustments = uni_image_view_set_scroll_adjustments;

    // Add keybindings.
    GtkBindingSet *binding_set = gtk_binding_set_by_class(klass);

    // Set zoom.
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_1, 0,
                                 "set_zoom", 1, G_TYPE_DOUBLE, 1.0);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_2, 0,
                                 "set_zoom", 1, G_TYPE_DOUBLE, 2.0);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_3, 0,
                                 "set_zoom", 1, G_TYPE_DOUBLE, 3.0);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_KP_1, 0,
                                 "set_zoom", 1, G_TYPE_DOUBLE, 1.0);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_KP_2, 0,
                                 "set_zoom", 1, G_TYPE_DOUBLE, 2.0);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_KP_3, 0,
                                 "set_zoom", 1, G_TYPE_DOUBLE, 3.0);

    // Zoom in
    gtk_binding_entry_add_signal(binding_set,
                                 GDK_KEY_plus, 0, "zoom_in", 0);
    gtk_binding_entry_add_signal(binding_set,
                                 GDK_KEY_equal, 0, "zoom_in", 0);
    gtk_binding_entry_add_signal(binding_set,
                                 GDK_KEY_KP_Add, 0, "zoom_in", 0);

    // Zoom out
    gtk_binding_entry_add_signal(binding_set,
                                 GDK_KEY_minus, 0, "zoom_out", 0);
    gtk_binding_entry_add_signal(binding_set,
                                 GDK_KEY_KP_Subtract, 0, "zoom_out", 0);

    // Set fitting
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_f, 0,
                                 "set_fitting", 1,
                                 G_TYPE_ENUM, UNI_FITTING_FULL);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_0, 0,
                                 "set_fitting", 1,
                                 G_TYPE_ENUM, UNI_FITTING_FULL);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_KP_0, 0,
                                 "set_fitting", 1,
                                 G_TYPE_ENUM, UNI_FITTING_FULL);

    // Unmodified scrolling
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Right, 0,
                                 "scroll", 2,
                                 GTK_TYPE_SCROLL_TYPE,
                                 GTK_SCROLL_STEP_RIGHT,
                                 GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_NONE);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Left, 0,
                                 "scroll", 2,
                                 GTK_TYPE_SCROLL_TYPE,
                                 GTK_SCROLL_STEP_LEFT,
                                 GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_NONE);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Down, 0,
                                 "scroll", 2,
                                 GTK_TYPE_SCROLL_TYPE,
                                 GTK_SCROLL_NONE,
                                 GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_STEP_DOWN);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Up, 0,
                                 "scroll", 2,
                                 GTK_TYPE_SCROLL_TYPE,
                                 GTK_SCROLL_NONE,
                                 GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_STEP_UP);

    // Shifted scrolling
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Right, GDK_SHIFT_MASK,
                                 "scroll", 2,
                                 GTK_TYPE_SCROLL_TYPE,
                                 GTK_SCROLL_PAGE_RIGHT,
                                 GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_NONE);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Left, GDK_SHIFT_MASK,
                                 "scroll", 2,
                                 GTK_TYPE_SCROLL_TYPE,
                                 GTK_SCROLL_PAGE_LEFT,
                                 GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_NONE);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Up, GDK_SHIFT_MASK,
                                 "scroll", 2,
                                 GTK_TYPE_SCROLL_TYPE,
                                 GTK_SCROLL_NONE,
                                 GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_PAGE_UP);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Down, GDK_SHIFT_MASK,
                                 "scroll", 2,
                                 GTK_TYPE_SCROLL_TYPE,
                                 GTK_SCROLL_NONE,
                                 GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_PAGE_DOWN);

    // Page Up & Down
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Page_Up, 0,
                                 "scroll", 2,
                                 GTK_TYPE_SCROLL_TYPE,
                                 GTK_SCROLL_NONE,
                                 GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_PAGE_UP);
    gtk_binding_entry_add_signal(binding_set, GDK_KEY_Page_Down, 0,
                                 "scroll", 2,
                                 GTK_TYPE_SCROLL_TYPE,
                                 GTK_SCROLL_NONE,
                                 GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_PAGE_DOWN);
}

static void uni_image_view_init_signals(UniImageViewClass *klass)
{
    uni_image_view_signals[SET_ZOOM] =
        g_signal_new("set_zoom",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(UniImageViewClass, set_zoom),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__DOUBLE,
                     G_TYPE_NONE, 1, G_TYPE_DOUBLE);
    uni_image_view_signals[ZOOM_IN] =
        g_signal_new("zoom_in",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(UniImageViewClass, zoom_in),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
    uni_image_view_signals[ZOOM_OUT] =
        g_signal_new("zoom_out",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(UniImageViewClass, zoom_out),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
    uni_image_view_signals[SET_FITTING] =
        g_signal_new("set_fitting",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(UniImageViewClass, set_fitting),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__ENUM,
                     G_TYPE_NONE, 1, G_TYPE_INT);
    uni_image_view_signals[SCROLL] =
        g_signal_new("scroll",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(UniImageViewClass, scroll),
                     NULL, NULL,
                     uni_marshal_VOID__ENUM_ENUM,
                     G_TYPE_NONE,
                     2, GTK_TYPE_SCROLL_TYPE, GTK_TYPE_SCROLL_TYPE);
    uni_image_view_signals[ZOOM_CHANGED] =
        g_signal_new("zoom_changed",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_LAST,
                     0,
                     NULL, NULL,
                     g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
    /**
     * UniImageView::pixbuf-changed:
     * @view: The view that emitted the signal.
     *
     * The ::pixbuf-changed signal is emitted when the pixbuf the
     * image view shows is changed and when its image data is changed.
     * Listening to this signal is useful if you, for example, have a
     * label that displays the width and height of the pixbuf in the
     * view.
     **/
    uni_image_view_signals[PIXBUF_CHANGED] =
        g_signal_new("pixbuf_changed",
                     G_TYPE_FROM_CLASS(klass),
                     G_SIGNAL_RUN_LAST,
                     G_STRUCT_OFFSET(UniImageViewClass, pixbuf_changed),
                     NULL, NULL,
                     g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void uni_image_view_init(UniImageView *view)
{
    gtk_widget_set_can_focus(GTK_WIDGET(view), TRUE);

    view->interp = GDK_INTERP_BILINEAR;
    view->fitting = UNI_FITTING_NORMAL;
    view->pixbuf = NULL;
    view->zoom = 1.0;
    view->offset_x = 0.0;
    view->offset_y = 0.0;
    view->is_rendering = FALSE;
    view->show_cursor = TRUE;
    view->void_cursor = NULL;
    view->dragger = G_OBJECT(uni_dragger_new((GtkWidget*) view));

    view->priv =
        (UniImageViewPrivate*) g_type_instance_get_private(
                                                (GTypeInstance*) view,
                                                UNI_TYPE_IMAGE_VIEW);

    view->priv->hadjustment = view->priv->vadjustment = NULL;
    uni_image_view_set_scroll_adjustments(
                            view,
                            GTK_ADJUSTMENT(gtk_adjustment_new(0.0,
                                                              1.0,
                                                              0.0,
                                                              1.0,
                                                              1.0,
                                                              1.0)),
                            GTK_ADJUSTMENT(gtk_adjustment_new(0.0,
                                                              1.0,
                                                              0.0,
                                                              1.0,
                                                              1.0,
                                                              1.0)));
    g_object_ref_sink(view->priv->hadjustment);
    g_object_ref_sink(view->priv->vadjustment);
}

static void uni_image_view_get_property(GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
    UniImageView *iv = UNI_IMAGE_VIEW(object);
    UniImageViewPrivate *priv = iv->priv;

    switch (prop_id)
    {
    case P_HADJUSTMENT:
        g_value_set_object(value, priv->hadjustment);
        break;

    case P_VADJUSTMENT:
        g_value_set_object(value, priv->vadjustment);
        break;

    case P_HSCROLLPOLICY:
        g_value_set_enum(value, priv->hscroll_policy);
        break;

    case P_VSCROLLPOLICY:
        g_value_set_enum(value, priv->vscroll_policy);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void uni_image_view_set_property(GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
    UniImageView *iv = UNI_IMAGE_VIEW(object);
    UniImageViewPrivate *priv = iv->priv;

    switch (prop_id)
    {
    case P_HADJUSTMENT:
        uni_image_view_set_hadjustment(
                            iv,
                            (GtkAdjustment*) g_value_get_object(value));
        break;

    case P_VADJUSTMENT:
        uni_image_view_set_vadjustment(
                            iv,
                            (GtkAdjustment*) g_value_get_object(value));
        break;

    case P_HSCROLLPOLICY:
        priv->hscroll_policy = g_value_get_enum(value);
        gtk_widget_queue_resize(GTK_WIDGET(iv));
        break;

    case P_VSCROLLPOLICY:
        priv->vscroll_policy = g_value_get_enum(value);
        gtk_widget_queue_resize(GTK_WIDGET(iv));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void uni_image_view_realize(GtkWidget *widget)
{
    UniImageView *view = UNI_IMAGE_VIEW(widget);
    gtk_widget_set_realized(widget, TRUE);

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    GdkWindowAttr attrs;
    attrs.window_type = GDK_WINDOW_CHILD;
    attrs.x = allocation.x;
    attrs.y = allocation.y;
    attrs.width = allocation.width;
    attrs.height = allocation.height;
    attrs.wclass = GDK_INPUT_OUTPUT;
    attrs.visual = gtk_widget_get_visual(widget);
    attrs.event_mask = (gtk_widget_get_events(widget)
                        | GDK_EXPOSURE_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_BUTTON_MOTION_MASK
                        | GDK_POINTER_MOTION_MASK
                        | GDK_SCROLL_MASK);

    int attr_mask = (GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL);
    GdkWindow *parent = gtk_widget_get_parent_window(widget);

    GdkWindow *window = gdk_window_new(parent, &attrs, attr_mask);
    gtk_widget_set_window(widget, window);
    gdk_window_set_user_data(window, view);

    GtkStyleContext *context = gtk_widget_get_style_context(widget);

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_style_context_set_background(context, window);
    G_GNUC_END_IGNORE_DEPRECATIONS

    view->void_cursor = gdk_cursor_new_for_display(
                                gdk_display_get_default(), GDK_ARROW);
}

static void uni_image_view_unrealize(GtkWidget *widget)
{
    UniImageView *view = UNI_IMAGE_VIEW(widget);

    g_object_unref(view->void_cursor);

    GTK_WIDGET_CLASS(uni_image_view_parent_class)->unrealize(widget);
}

static void uni_image_view_finalize(GObject *object)
{
    UniImageView *view = UNI_IMAGE_VIEW(object);
    if (view->priv->hadjustment)
    {
        g_signal_handlers_disconnect_by_data(G_OBJECT(view->priv->hadjustment),
                                             view);
        g_object_unref(view->priv->hadjustment);
        view->priv->hadjustment = NULL;
    }
    if (view->priv->vadjustment)
    {
        g_signal_handlers_disconnect_by_data(G_OBJECT(view->priv->vadjustment),
                                             view);
        g_object_unref(view->priv->vadjustment);
        view->priv->vadjustment = NULL;
    }
    if (view->pixbuf)
    {
        g_object_unref(view->pixbuf);
        view->pixbuf = NULL;
    }
    g_object_unref(view->dragger);
    // Chain up.
    G_OBJECT_CLASS(uni_image_view_parent_class)->finalize(object);
}


// ----------------------------------------------------------------------------

static void widget_size_allocate(GtkWidget *widget, GtkAllocation *alloc)
{
    UniImageView *view = UNI_IMAGE_VIEW(widget);

    if (gtk_widget_get_realized(widget))
    {
        gtk_widget_set_allocation(widget, alloc);
    }
    else
    {
        GtkWidget *scroll_view = _uni_get_scrollwin(widget);

        GtkAllocation allocation;
        gtk_widget_get_allocation(scroll_view, &allocation);
        gtk_widget_set_allocation(widget, &allocation);
    }

    if (view->pixbuf && view->fitting != UNI_FITTING_NONE)
        _uni_image_view_zoom_to_fit(view, TRUE);

    _uni_image_view_clamp_offset(view, &view->offset_x, &view->offset_y);

    _uni_image_view_update_adjustments(view);

    if (gtk_widget_get_realized(widget))
        gdk_window_move_resize(gtk_widget_get_window(widget),
                               alloc->x, alloc->y,
                               alloc->width, alloc->height);
}

static VnrWindow* _uni_get_appwindow(GtkWidget *widget)
{
    return VNR_WINDOW(gtk_widget_get_toplevel(widget));
}

static GtkWidget* _uni_get_scrollwin(GtkWidget *widget)
{
    VnrWindow *window = VNR_WINDOW(gtk_widget_get_toplevel(widget));

    return window->scroll_view;
}

static void _uni_image_view_clamp_offset(UniImageView *view,
                                         gdouble *x, gdouble *y)
{
    Size alloc = _uni_image_view_get_allocated_size(view);
    Size zoomed = _uni_image_view_get_zoomed_size(view);

    *x = MIN(*x, zoomed.width - alloc.width);
    *y = MIN(*y, zoomed.height - alloc.height);
    *x = MAX(*x, 0);
    *y = MAX(*y, 0);
}

static void _uni_image_view_update_adjustments(UniImageView *view)
{
    Size zoomed = _uni_image_view_get_zoomed_size(view);
    Size alloc = _uni_image_view_get_allocated_size(view);

    gtk_adjustment_configure(view->priv->hadjustment,
                             view->offset_x,
                             0.0,
                             zoomed.width,
                             20.0,
                             alloc.width / 2,
                             alloc.width);

    gtk_adjustment_configure(view->priv->vadjustment,
                             view->offset_y,
                             0.0,
                             zoomed.height,
                             20.0,
                             alloc.height / 2,
                             alloc.height);

    // https://docs.gtk.org/gtk3/method.Adjustment.changed.html
    //g_signal_handlers_block_by_data(
    //      G_OBJECT(view->priv->hadjustment), view);
    //g_signal_handlers_block_by_data(
    //      G_OBJECT(view->priv->vadjustment), view);
    //gtk_adjustment_changed(view->priv->hadjustment);
    //gtk_adjustment_changed(view->priv->vadjustment);
    //g_signal_handlers_unblock_by_data(
    //      G_OBJECT(view->priv->hadjustment), view);
    //g_signal_handlers_unblock_by_data(
    //      G_OBJECT(view->priv->vadjustment), view);
}

static void _uni_image_view_zoom_to_fit(UniImageView *view,
                                        gboolean is_allocating)
{
    GtkWidget *scrollwin = _uni_get_scrollwin(GTK_WIDGET(view));

    GtkAllocation allocation;
    gtk_widget_get_allocation(scrollwin, &allocation);

    Size imgsize = _uni_image_view_get_pixbuf_size(view);
    gdouble ratio_x = (gdouble) allocation.width / imgsize.width;
    gdouble ratio_y = (gdouble) allocation.height / imgsize.height;

    gdouble zoom = MIN(ratio_y, ratio_x);

    if (view->fitting == UNI_FITTING_NORMAL)
        zoom = CLAMP(zoom, UNI_ZOOM_MIN, 1.0);
    else if (view->fitting == UNI_FITTING_FULL)
        zoom = CLAMP(zoom, UNI_ZOOM_MIN, UNI_ZOOM_MAX);

    _uni_image_view_set_zoom_no_center(view, zoom, is_allocating);
}

static Size _uni_image_view_get_pixbuf_size(UniImageView *view)
{
    Size s = {0, 0};

    if (!view->pixbuf)
        return s;

    s.width = gdk_pixbuf_get_width(view->pixbuf);
    s.height = gdk_pixbuf_get_height(view->pixbuf);

    return s;
}

static void _uni_image_view_set_zoom_no_center(UniImageView *view,
                                               gdouble zoom,
                                               gboolean is_allocating)
{
    Size alloc = _uni_image_view_get_allocated_size(view);
    gdouble center_x = alloc.width / 2.0;
    gdouble center_y = alloc.height / 2.0;
    _uni_image_view_set_zoom_with_center(view, zoom,
                                        center_x, center_y, is_allocating);
}

static Size _uni_image_view_get_allocated_size(UniImageView *view)
{
    GtkAllocation allocation;
    gtk_widget_get_allocation(GTK_WIDGET(view), &allocation);

    Size size = {
        .width = allocation.width,
        .height = allocation.height};

    return size;
}

static void _uni_image_view_set_zoom_with_center(UniImageView *view,
                                                 gdouble zoom,
                                                 gdouble center_x,
                                                 gdouble center_y,
                                                 gboolean is_allocating)
{
    // this method must only be used by uni_image_view_zoom_to_fit()
    // and uni_image_view_set_zoom()

    gdouble zoom_ratio = zoom / view->zoom;

    Size zoomed = _uni_image_view_get_zoomed_size(view);
    Size alloc = _uni_image_view_get_allocated_size(view);
    gint x, y;

    x = alloc.width - zoomed.width;
    y = alloc.height - zoomed.height;
    x = (x < 0) ? 0 : x;
    y = (y < 0) ? 0 : y;

    gdouble offset_x, offset_y;
    offset_x = (view->offset_x + center_x - x / 2) * zoom_ratio - center_x;
    offset_y = (view->offset_y + center_y - y / 2) * zoom_ratio - center_y;
    view->zoom = zoom;

    _uni_image_view_clamp_offset(view, &offset_x, &offset_y);
    view->offset_x = offset_x;
    view->offset_y = offset_y;

    if (!is_allocating && zoom_ratio != 1.0)
    {
        view->fitting = UNI_FITTING_NONE;
        _uni_image_view_update_adjustments(view);
        gtk_widget_queue_draw(GTK_WIDGET(view));
    }

    g_signal_emit(G_OBJECT(view),
                  uni_image_view_signals[ZOOM_CHANGED], 0);
}

static Size _uni_image_view_get_zoomed_size(UniImageView *view)
{
    Size size = _uni_image_view_get_pixbuf_size(view);

    size.width = (int) (size.width * view->zoom + 0.5);
    size.height = (int) (size.height * view->zoom + 0.5);

    return size;
}

static int widget_draw(GtkWidget *widget, cairo_t *cr)
{
    GtkWidget *scrollwin = _uni_get_scrollwin(widget);

    GtkAllocation allocation;
    gtk_widget_get_allocation(scrollwin, &allocation);

    allocation.x = 0;
    allocation.y = 0;

    return _uni_image_view_repaint_area(UNI_IMAGE_VIEW(widget),
                                        &allocation, cr);
}

static int _uni_image_view_repaint_area(UniImageView *view,
                                        GdkRectangle *paint_rect,
                                        cairo_t *cr)
{
    // redraws the portion of the widget defined by paint_rect

    if (view->is_rendering)
        return FALSE;

    // do not draw zero size rectangles
    if (!paint_rect->width || !paint_rect->height)
        return FALSE;

    view->is_rendering = TRUE;

    // image area is the area on the widget occupied by the pixbuf
    GdkRectangle image_area = {0, 0, 0, 0};
    Size alloc = _uni_image_view_get_allocated_size(view);

    uni_image_view_get_draw_rect(view, &image_area);

    if (image_area.x > 0
        || image_area.y > 0
        || image_area.width < alloc.width
        || image_area.height < alloc.height)
    {
        _uni_image_view_draw_background(view, &image_area, alloc, cr);
    }

    // paint area is the area on the widget that should be redrawn
    GdkRectangle paint_area = {0, 0, 0, 0};
    gboolean intersects = gdk_rectangle_intersect(&image_area,
                                                  paint_rect,
                                                  &paint_area);
    if (intersects && view->pixbuf)
    {
        int src_x = (int) ((view->offset_x
                            + (gdouble) paint_area.x
                            - (gdouble) image_area.x) + 0.5);
        int src_y = (int) ((view->offset_y
                            + (gdouble) paint_area.y
                            - (gdouble) image_area.y) + 0.5);

        UniDrawOpts opts;

        opts.zoom = view->zoom;
        opts.zoom_rect.x = src_x;
        opts.zoom_rect.y = src_y;
        opts.zoom_rect.width = paint_area.width;
        opts.zoom_rect.height = paint_area.height;
        opts.widget_x = paint_area.x;
        opts.widget_y = paint_area.y;
        opts.interp = view->interp;
        opts.pixbuf = view->pixbuf;

        uni_dragger_paint_image(UNI_DRAGGER(view->dragger), &opts, cr);
    }

    view->is_rendering = FALSE;

    return TRUE;
}

static void _uni_image_view_draw_background(UniImageView *view,
                                            GdkRectangle *image_area,
                                            Size alloc,
                                            cairo_t *cr)
{
    cairo_save(cr);

    GtkWidget *widget = GTK_WIDGET(view);
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    GtkStateFlags state = gtk_widget_get_state_flags(widget);

    GdkRGBA rgba;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_style_context_get_background_color(context, state, &rgba);
    G_GNUC_END_IGNORE_DEPRECATIONS

    gdk_cairo_set_source_rgba(cr, &rgba);

    GdkRectangle borders[4];
    GdkRectangle outer = {0, 0, alloc.width, alloc.height};
    uni_rectangle_get_rects_around(&outer, image_area, borders);

    for (int n = 0; n < 4; n++)
    {
        // Not sure why incrementing the size is necessary.
        borders[n].width++;
        borders[n].height++;

        uni_draw_rect(cr, TRUE, &borders[n]);
    }

    cairo_restore(cr);
}

static int widget_button_press(GtkWidget *widget, GdkEventButton *event)
{
    gtk_widget_grab_focus(widget);

    VnrWindow *appwindow = _uni_get_appwindow(widget);
    g_assert(gtk_widget_is_toplevel(GTK_WIDGET(appwindow)));

    UniImageView *view = UNI_IMAGE_VIEW(widget);

    if (event->type == GDK_2BUTTON_PRESS
            && event->button == 1
            && appwindow->prefs->click_behavior == VNR_PREFS_CLICK_FULLSCREEN)
    {
        window_fullscreen_toggle(appwindow);
        return 1;
    }
    else if (event->type == GDK_2BUTTON_PRESS
             && event->button == 1
             && appwindow->prefs->click_behavior == VNR_PREFS_CLICK_NEXT)
    {
        int width = gdk_window_get_width(gtk_widget_get_window(widget));

        if (event->x / width < 0.5)
            window_prev(appwindow);
        else
            window_next(appwindow, TRUE);

        return 1;
    }
    else if (event->type == GDK_BUTTON_PRESS && event->button == 1)
    {
        return uni_dragger_button_press(UNI_DRAGGER(view->dragger), event);
    }
    else if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
    {
        if (view->fitting == UNI_FITTING_FULL
            || (view->fitting == UNI_FITTING_NORMAL && view->zoom != 1.0))
            _uni_image_view_set_zoom_with_center(view, 1.,
                                                event->x, event->y,
                                                FALSE);
        else
            uni_image_view_set_fitting(view, UNI_FITTING_FULL);

        return 1;
    }
    else if (event->type == GDK_BUTTON_PRESS && event->button == 3)
    {
        gtk_menu_popup_at_pointer(GTK_MENU(appwindow->popup_menu),
                                  (const GdkEvent*) event);
    }
    else if (event->type == GDK_BUTTON_PRESS && event->button == 8)
    {
        window_prev(appwindow);
    }
    else if (event->type == GDK_BUTTON_PRESS && event->button == 9)
    {
        window_next(appwindow, TRUE);
    }

    return 0;
}

static int widget_button_release(GtkWidget *widget, GdkEventButton *ev)
{
    UniImageView *view = UNI_IMAGE_VIEW(widget);

    return uni_dragger_button_release(UNI_DRAGGER(view->dragger), ev);
}

static int widget_motion_notify(GtkWidget *widget, GdkEventMotion *ev)
{
    UniImageView *view = UNI_IMAGE_VIEW(widget);

    if (view->is_rendering)
        return FALSE;

    return uni_dragger_motion_notify(UNI_DRAGGER(view->dragger), ev);
}

static int widget_scroll_event(GtkWidget *widget, GdkEventScroll *ev)
{
    VnrWindow *appwindow = _uni_get_appwindow(widget);
    g_assert(gtk_widget_is_toplevel(GTK_WIDGET(appwindow)));

    gdouble zoom;
    UniImageView *view = UNI_IMAGE_VIEW(widget);

    /* Horizontal scroll left is equivalent to scroll up and right is
     * like scroll down. No idea if that is correct -- I have no input
     * device that can do horizontal scrolls. */

    if (appwindow->prefs->wheel_behavior == VNR_PREFS_WHEEL_ZOOM
            || (ev->state & GDK_CONTROL_MASK) != 0)
    {
        switch (ev->direction)
        {
        case GDK_SCROLL_LEFT:
            // In Zoom mode left/right scroll is used for navigation
            window_prev(appwindow);
            break;

        case GDK_SCROLL_RIGHT:
            window_next(appwindow, TRUE);
            break;

        case GDK_SCROLL_UP:
            if (ev->state & GDK_SHIFT_MASK)
            {
                window_prev(appwindow);
            }
            else
            {
                zoom = CLAMP(view->zoom * UNI_ZOOM_STEP,
                             UNI_ZOOM_MIN, UNI_ZOOM_MAX);
                _uni_image_view_set_zoom_with_center(view,
                                                     zoom,
                                                     ev->x, ev->y,
                                                     FALSE);
            }
            break;

        default:
            if (ev->state & GDK_SHIFT_MASK)
            {
                window_next(appwindow, TRUE);
            }
            else
            {
                zoom = CLAMP(view->zoom / UNI_ZOOM_STEP,
                             UNI_ZOOM_MIN, UNI_ZOOM_MAX);
                _uni_image_view_set_zoom_with_center(view,
                                                     zoom,
                                                     ev->x, ev->y,
                                                     FALSE);
            }
        }
    }
    else if (appwindow->prefs->wheel_behavior == VNR_PREFS_WHEEL_NAVIGATE)
    {
        switch (ev->direction)
        {
        case GDK_SCROLL_LEFT:
            zoom = CLAMP(view->zoom * UNI_ZOOM_STEP,
                         UNI_ZOOM_MIN, UNI_ZOOM_MAX);
            _uni_image_view_set_zoom_with_center(view,
                                                 zoom,
                                                 ev->x, ev->y,
                                                 FALSE);
            break;

        case GDK_SCROLL_RIGHT:
            zoom = CLAMP(view->zoom / UNI_ZOOM_STEP,
                         UNI_ZOOM_MIN, UNI_ZOOM_MAX);
            _uni_image_view_set_zoom_with_center(view,
                                                 zoom,
                                                 ev->x, ev->y,
                                                 FALSE);
            break;

        case GDK_SCROLL_UP:
            if (ev->state & GDK_SHIFT_MASK)
            {
                zoom = CLAMP(view->zoom * UNI_ZOOM_STEP,
                             UNI_ZOOM_MIN, UNI_ZOOM_MAX);
                _uni_image_view_set_zoom_with_center(view,
                                                     zoom,
                                                     ev->x, ev->y,
                                                     FALSE);
            }
            else
            {
                window_prev(appwindow);
            }

            break;

        default:
            if (ev->state & GDK_SHIFT_MASK)
            {
                zoom = CLAMP(view->zoom / UNI_ZOOM_STEP,
                             UNI_ZOOM_MIN, UNI_ZOOM_MAX);
                _uni_image_view_set_zoom_with_center(view,
                                                     zoom,
                                                     ev->x, ev->y,
                                                     FALSE);
            }
            else
            {
                window_next(appwindow, TRUE);
            }
        }
    }
    else
    {
        switch (ev->direction)
        {
        case GDK_SCROLL_LEFT:
            uni_image_view_scroll(view,
                                  GTK_SCROLL_PAGE_LEFT, GTK_SCROLL_NONE);
            break;

        case GDK_SCROLL_RIGHT:
            uni_image_view_scroll(view,
                                  GTK_SCROLL_PAGE_RIGHT, GTK_SCROLL_NONE);
            break;

        case GDK_SCROLL_UP:
            if (ev->state & GDK_SHIFT_MASK)
                uni_image_view_scroll(view,
                                      GTK_SCROLL_PAGE_LEFT, GTK_SCROLL_NONE);
            else
                uni_image_view_scroll(view,
                                      GTK_SCROLL_NONE, GTK_SCROLL_PAGE_UP);
            break;

        default:
            if (ev->state & GDK_SHIFT_MASK)
                uni_image_view_scroll(view,
                                      GTK_SCROLL_PAGE_RIGHT, GTK_SCROLL_NONE);
            else
                uni_image_view_scroll(view,
                                      GTK_SCROLL_NONE, GTK_SCROLL_PAGE_DOWN);
        }
    }

    return TRUE;
}

static void uni_image_view_set_scroll_adjustments(UniImageView *view,
                                                  GtkAdjustment *hadj,
                                                  GtkAdjustment *vadj)
{
    if (hadj && (view->priv->hadjustment != hadj))
    {
        if (view->priv->hadjustment)
        {
            g_signal_handlers_disconnect_by_data(
                        G_OBJECT(view->priv->hadjustment), view);
            g_object_unref(view->priv->hadjustment);
        }

        g_signal_connect_swapped(hadj,
                         "value_changed",
                         G_CALLBACK(_on_hadj_value_changed), view);

        view->priv->hadjustment = hadj;
        g_object_ref_sink(view->priv->hadjustment);
    }

    if (vadj && (view->priv->vadjustment != vadj))
    {
        if (view->priv->vadjustment)
        {
            g_signal_handlers_disconnect_by_data(
                        G_OBJECT(view->priv->vadjustment), view);
            g_object_unref(view->priv->vadjustment);
        }

        g_signal_connect_swapped(vadj,
                         "value_changed",
                         G_CALLBACK(_on_vadj_value_changed), view);

        view->priv->vadjustment = vadj;
        g_object_ref_sink(view->priv->vadjustment);
    }
}

static gboolean _on_hadj_value_changed(UniImageView *view, GtkAdjustment *adj)
{
    int offset_x = gtk_adjustment_get_value(adj);

    _uni_image_view_scroll_to(view, offset_x, view->offset_y, FALSE, FALSE);

    return FALSE;
}

static gboolean _on_vadj_value_changed(UniImageView *view, GtkAdjustment *adj)
{
    int offset_y = gtk_adjustment_get_value(adj);

    _uni_image_view_scroll_to(view, view->offset_x, offset_y, FALSE, FALSE);

    return FALSE;
}

/**
 * uni_image_view_scroll_to:
 * @offset_x: X part of the offset in zoom space coordinates.
 * @offset_y: Y part of the offset in zoom space coordinates.
 * @set_adjustments: whether to update the adjustments. Because this
 *   function is called from the adjustments callbacks, it needs to be
 *   %FALSE to prevent infinite recursion.
 * @invalidate: whether to invalidate the view or redraw immedately,
 *  see uni_image_view_set_offset()
 *
 * Set the offset of where in the image the #UniImageView should begin
 * to display image data.
 **/
static void _uni_image_view_scroll_to(UniImageView *view,
                                      gdouble offset_x,
                                      gdouble offset_y,
                                      gboolean set_adjustments,
                                      gboolean invalidate)
{
    _uni_image_view_clamp_offset(view, &offset_x, &offset_y);

    // Round avoids floating point to integer conversion errors.
    int delta_x = floor(offset_x - view->offset_x + 0.5);
    int delta_y = floor(offset_y - view->offset_y + 0.5);

    // Exit early if the scroll was smaller than one (zoom space) pixel.
    if (delta_x == 0 && delta_y == 0)
        return;

    view->offset_x = offset_x;
    view->offset_y = offset_y;

    GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(view));

    if (set_adjustments)
    {
        g_signal_handlers_block_by_data(
                    G_OBJECT(view->priv->hadjustment), view);
        g_signal_handlers_block_by_data(
                    G_OBJECT(view->priv->vadjustment), view);

        gtk_adjustment_set_value(view->priv->hadjustment, view->offset_x);
        gtk_adjustment_set_value(view->priv->vadjustment, view->offset_y);

        g_signal_handlers_unblock_by_data(
                    G_OBJECT(view->priv->hadjustment), view);
        g_signal_handlers_unblock_by_data(
                    G_OBJECT(view->priv->vadjustment), view);
    }

    if (window)
    {
        if (invalidate)
            gdk_window_invalidate_rect(window, NULL, TRUE);
        else
            _uni_image_view_fast_scroll(view, delta_x, delta_y);
    }
}

/**
 * uni_image_view_fast_scroll:
 *
 * Actually scroll the views window using gdk_draw_drawable().
 * GTK_WIDGET (view)->window is guaranteed to be non-NULL in this
 * function.
 **/
static void _uni_image_view_fast_scroll(UniImageView *view,
                                        int delta_x, int delta_y)
{
    int src_x, src_y;
    int dest_x, dest_y;

    if (delta_x < 0)
    {
        src_x = 0;
        dest_x = -delta_x;
    }
    else
    {
        src_x = delta_x;
        dest_x = 0;
    }
    if (delta_y < 0)
    {
        src_y = 0;
        dest_y = -delta_y;
    }
    else
    {
        src_y = delta_y;
        dest_y = 0;
    }

    GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(view));

    //cairo_t *cr = gdk_cairo_create(window);

    cairo_region_t *region = cairo_region_create();
    GdkDrawingContext *context;
    context = gdk_window_begin_draw_frame(window,region);
    cairo_t *cr = gdk_drawing_context_get_cairo_context(context);

    Size alloc = _uni_image_view_get_allocated_size(view);

    GdkPixbuf *win = gdk_pixbuf_get_from_window(window,
                                                src_x,
                                                src_y,
                                                alloc.width - abs(delta_x),
                                                alloc.height - abs(delta_y));

    gdk_cairo_set_source_pixbuf(cr, win, dest_x, dest_y);
    cairo_paint(cr);
    g_object_unref(win);

    // if we moved in both the x and y directions, two "strips" of the image
    // becomes visible. One horizontal strip and one vertical strip.

    GdkRectangle horiz_strip = {
        0,
        (delta_y < 0) ? 0 : alloc.height - abs(delta_y),
        alloc.width,
        abs(delta_y)};

    _uni_image_view_repaint_area(view, &horiz_strip, cr);

    GdkRectangle vert_strip = {
        (delta_x < 0) ? 0 : alloc.width - abs(delta_x),
        0,
        abs(delta_x),
        alloc.height};

    _uni_image_view_repaint_area(view, &vert_strip, cr);

    gdk_window_end_draw_frame(window, context);
    cairo_region_destroy(region);

    //cairo_destroy(cr); ???
}


// public class members -------------------------------------------------------

/**
 * uni_image_view_set_zoom:
 * @view: a #UniImageView
 * @zoom: the new zoom factor
 *
 * Sets the zoom of the view.
 *
 * Fitting is always disabled after this method has run. The
 * ::zoom-changed signal is unconditionally emitted.
 **/
void uni_image_view_set_zoom(UniImageView *view, gdouble zoom)
{
    g_return_if_fail(UNI_IS_IMAGE_VIEW(view));

    zoom = CLAMP(zoom, UNI_ZOOM_MIN, UNI_ZOOM_MAX);

    _uni_image_view_set_zoom_no_center(view, zoom, FALSE);
}

/**
 * uni_image_view_zoom_in:
 * @view: a #UniImageView
 *
 * Zoom in the view one step. Calling this method causes the widget to
 * immediately repaint itself.
 **/
void uni_image_view_zoom_in(UniImageView *view)
{
    gdouble zoom = CLAMP(view->zoom * UNI_ZOOM_STEP,
                         UNI_ZOOM_MIN, UNI_ZOOM_MAX);
    uni_image_view_set_zoom(view, zoom);
}

/**
 * uni_image_view_zoom_out:
 * @view: a #UniImageView
 *
 * Zoom out the view one step. Calling this method causes the widget to
 * immediately repaint itself.
 **/
void uni_image_view_zoom_out(UniImageView *view)
{
    gdouble zoom = CLAMP(view->zoom / UNI_ZOOM_STEP,
                         UNI_ZOOM_MIN, UNI_ZOOM_MAX);
    uni_image_view_set_zoom(view, zoom);
}

void uni_image_view_set_fitting(UniImageView *view, UniFittingMode fitting)
{
    g_return_if_fail(UNI_IS_IMAGE_VIEW(view));

    view->fitting = fitting;

    gtk_widget_queue_resize(GTK_WIDGET(view));
}

static void uni_image_view_scroll(UniImageView *view,
                                  GtkScrollType xscroll,
                                  GtkScrollType yscroll)
{
    GtkAdjustment *hadj = view->priv->hadjustment;
    GtkAdjustment *vadj = view->priv->vadjustment;

    gdouble h_step = gtk_adjustment_get_step_increment(hadj);
    gdouble v_step = gtk_adjustment_get_step_increment(vadj);
    gdouble h_page = gtk_adjustment_get_page_increment(hadj);
    gdouble v_page = gtk_adjustment_get_page_increment(vadj);

    int xstep = 0;

    if (xscroll == GTK_SCROLL_STEP_LEFT)
        xstep = -h_step;
    else if (xscroll == GTK_SCROLL_STEP_RIGHT)
        xstep = h_step;
    else if (xscroll == GTK_SCROLL_PAGE_LEFT)
        xstep = -h_page;
    else if (xscroll == GTK_SCROLL_PAGE_RIGHT)
        xstep = h_page;

    int ystep = 0;

    if (yscroll == GTK_SCROLL_STEP_UP)
        ystep = -v_step;
    else if (yscroll == GTK_SCROLL_STEP_DOWN)
        ystep = v_step;
    else if (yscroll == GTK_SCROLL_PAGE_UP)
        ystep = -v_page;
    else if (yscroll == GTK_SCROLL_PAGE_DOWN)
        ystep = v_page;

    _uni_image_view_scroll_to(view,
                              view->offset_x + xstep,
                              view->offset_y + ystep, TRUE, FALSE);
}


// public ---------------------------------------------------------------------

/**
 * uni_image_view_get_viewport:
 * @view: a #UniImageView
 * @rect: a #GdkRectangle to fill in with the current viewport or
 *   %NULL.
 * @returns: %TRUE if a #GdkPixbuf is shown, %FALSE otherwise.
 *
 * Fills in the rectangle with the current viewport. If pixbuf is
 * %NULL, there is no viewport, @rect is left untouched and %FALSE is
 * returned.
 *
 * The current viewport is defined as the rectangle, in zoomspace
 * coordinates as the area of the loaded pixbuf the #UniImageView is
 * currently showing.
 **/
gboolean uni_image_view_get_viewport(UniImageView *view, GdkRectangle *rect)
{
    gboolean ret_val = (view->pixbuf != NULL);

    if (!rect || !ret_val)
        return ret_val;

    Size alloc = _uni_image_view_get_allocated_size(view);
    Size zoomed = _uni_image_view_get_zoomed_size(view);
    rect->x = view->offset_x;
    rect->y = view->offset_y;
    rect->width = MIN(alloc.width, zoomed.width);
    rect->height = MIN(alloc.height, zoomed.height);

    return TRUE;
}

/**
 * uni_image_view_get_draw_rect:
 * @view: a #UniImageView
 * @rect: a #GdkRectangle to fill in with the area of the widget in
 *   which the pixbuf is drawn.
 * @returns: %TRUE if the view is allocated and has a pixbuf, %FALSE
 *   otherwise.
 *
 * Get the rectangle in the widget where the pixbuf is painted.
 *
 * For example, if the widgets allocated size is 100, 100 and the
 * pixbufs size is 50, 50 and the zoom factor is 1.0, then the pixbuf
 * will be drawn centered on the widget. @rect will then be
 * (25,25)-[50,50].
 *
 * This method is useful when converting from widget to image or zoom
 * space coordinates.
 **/
gboolean uni_image_view_get_draw_rect(UniImageView *view, GdkRectangle *rect)
{
    if (!view->pixbuf)
        return FALSE;

    Size alloc = _uni_image_view_get_allocated_size(view);
    Size zoomed = _uni_image_view_get_zoomed_size(view);

    rect->x = (alloc.width - zoomed.width) / 2;
    rect->y = (alloc.height - zoomed.height) / 2;
    rect->x = MAX(rect->x, 0);
    rect->y = MAX(rect->y, 0);
    rect->width = MIN(zoomed.width, alloc.width);
    rect->height = MIN(zoomed.height, alloc.height);

    return TRUE;
}

/**
 * uni_image_view_set_offset:
 * @view: A #UniImageView.
 * @x: X-component of the offset in zoom space coordinates.
 * @y: Y-component of the offset in zoom space coordinates.
 * @invalidate: whether to invalidate the view or redraw immediately.
 *
 * Sets the offset of where in the image the #UniImageView should
 * begin displaying image data.
 *
 * The offset is clamped so that it will never cause the #UniImageView
 * to display pixels outside the pixbuf. Setting this attribute causes
 * the widget to repaint itself if it is realized.
 *
 * If @invalidate is %TRUE, the views entire area will be invalidated
 * instead of redrawn immediately. The view is then queued for redraw,
 * which means that additional operations can be performed on it
 * before it is redrawn.
 *
 * The difference can sometimes be important like when you are
 * overlaying data and get flicker or artifacts when setting the
 * offset. If that happens, setting @invalidate to %TRUE could fix the
 * problem. See the source code to #GtkImageToolSelector for an
 * example.
 *
 * Normally, @invalidate should always be %FALSE because it is much
 * faster to repaint immedately than invalidating.
 **/
void uni_image_view_set_offset(UniImageView *view,
                               gdouble offset_x,
                               gdouble offset_y, gboolean invalidate)
{
    _uni_image_view_scroll_to(view, offset_x, offset_y, TRUE, invalidate);
}

GtkAdjustment* uni_image_view_get_hadjustment(UniImageView *view)
{
    return view->priv->hadjustment;
}

GtkAdjustment* uni_image_view_get_vadjustment(UniImageView *view)
{
    return view->priv->vadjustment;
}

void uni_image_view_set_hadjustment(UniImageView *view, GtkAdjustment *hadj)
{
    uni_image_view_set_scroll_adjustments(view, hadj, NULL);
}

void uni_image_view_set_vadjustment(UniImageView *view, GtkAdjustment *vadj)
{
    uni_image_view_set_scroll_adjustments(view, NULL, vadj);
}

GdkPixbuf* uni_image_view_get_pixbuf(UniImageView *view)
{
    if (!view)
        return NULL;

    return view->pixbuf;
}

/**
 * uni_image_view_set_pixbuf:
 * @view: A #UniImageView.
 * @pixbuf: The pixbuf to display.
 * @reset_fit: Whether to reset fitting or not.
 *
 * Sets the @pixbuf to display, or %NULL to not display any pixbuf.
 * Normally, @reset_fit should be %TRUE which enables fitting. Which
 * means that, initially, the whole pixbuf will be shown.
 *
 * Sometimes, the fit mode should not be reset. For example, if
 * UniImageView is showing an animation, it would be bad to reset the
 * fit mode for each new frame. The parameter should then be %FALSE
 * which leaves the fit mode of the view untouched.
 *
 * This method should not be used if merely the contents of the pixbuf
 * has changed. See uni_image_view_damage_pixels() for that.
 *
 * If @reset_fit is %TRUE, the ::zoom-changed signal is emitted,
 * otherwise not. The ::pixbuf-changed signal is also emitted.
 *
 * The default pixbuf is %NULL.
 **/
void uni_image_view_set_pixbuf(UniImageView *view,
                               GdkPixbuf *pixbuf, gboolean reset_fit)
{
    if (view->pixbuf != pixbuf)
    {
        if (view->pixbuf)
            g_object_unref(view->pixbuf);

        view->pixbuf = pixbuf;

        if (view->pixbuf)
            g_object_ref(pixbuf);
    }

    if (reset_fit)
    {
        uni_image_view_set_fitting(view, UNI_FITTING_NORMAL);
    }
    else
    {
        /*
           If the size of the pixbuf changes, the offset might point to
           pixels outside it so we use uni_image_view_scroll_to() to
           make it valid again. And if the size is different, naturally
           we must also update the adjustments.
         */
        _uni_image_view_scroll_to(view, view->offset_x, view->offset_y,
                                 FALSE, FALSE);
        _uni_image_view_update_adjustments(view);
        gtk_widget_queue_draw(GTK_WIDGET(view));
    }

    g_signal_emit(G_OBJECT(view),
                  uni_image_view_signals[PIXBUF_CHANGED], 0);

    uni_dragger_pixbuf_changed(UNI_DRAGGER(view->dragger), reset_fit, NULL);
}

void uni_image_view_set_zoom_mode(UniImageView *view, VnrPrefsZoom mode)
{
    switch (mode)
    {
    case VNR_PREFS_ZOOM_NORMAL:
        uni_image_view_set_fitting(view, UNI_FITTING_NONE);
        // view->zoom = 1.0;
        uni_image_view_set_zoom(view, 1.0);
        break;

    case VNR_PREFS_ZOOM_FIT:
        uni_image_view_set_fitting(view, UNI_FITTING_FULL);
        break;

    case VNR_PREFS_ZOOM_SMART:
        uni_image_view_set_fitting(view, UNI_FITTING_NORMAL);
        break;

    default:
        break;
    }
}


