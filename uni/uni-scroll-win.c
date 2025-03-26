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

#include "uni-scroll-win.h"
#include "uni-image-view.h"
#include "uni-nav.h"

static void uni_scroll_win_finalize(GObject *object);
static void uni_scroll_win_set_property(GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec);
static void _uni_scroll_win_nav_btn_clicked(UniScrollWin *window,
                                            GdkEventButton *ev);
static void _uni_scroll_win_set_view(UniScrollWin *window,
                                     UniImageView *view);
static void _uni_scroll_win_adjustment_changed(GtkAdjustment *adj,
                                               UniScrollWin *window);
static void _uni_scroll_win_get_preferred_width(GtkWidget *widget,
                                                gint *minimal_width,
                                                gint *natural_width);
static void _uni_scroll_win_get_preferred_height(GtkWidget *widget,
                                                 gint *minimal_height,
                                                 gint *natural_height);
static void _uni_scroll_win_show_scrollbar(UniScrollWin *window, gboolean show);

static const char *nav_button[] =
    {
        // columns rows colors chars-per-pixel
        "14 14 2 1 ",
        "  c black",
        ". c None",
        // pixels
        "......  ......",
        ".....    .....",
        "....      ....",
        "......  ......",
        ".. ...  ... ..",
        ".  ...  ...  .",
        "              ",
        "              ",
        ".  ...  ...  .",
        ".. ...  ... ..",
        "......  ......",
        "....      ....",
        ".....    .....",
        "......  ......"};


// creation / destruction -----------------------------------------------------

G_GNUC_BEGIN_IGNORE_DEPRECATIONS

G_DEFINE_TYPE(UniScrollWin, uni_scroll_win, GTK_TYPE_TABLE)

G_GNUC_END_IGNORE_DEPRECATIONS

enum
{
    PROP_IMAGE_VIEW = 1
};

GtkWidget* uni_scroll_win_new(UniImageView *view)
{
    gpointer data = g_object_new(UNI_TYPE_SCROLL_WIN,
                                 "n-columns", 2,
                                 "n-rows", 2,
                                 "homogeneous", FALSE,
                                 "view", view,
                                 NULL);
    return GTK_WIDGET(data);
}

static void uni_scroll_win_class_init(UniScrollWinClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->set_property = uni_scroll_win_set_property;
    object_class->finalize = uni_scroll_win_finalize;

    GParamSpec *pspec = g_param_spec_object("view",
                                            "Image View",
                                            "Image View to navigate",
                                            UNI_TYPE_IMAGE_VIEW,
                                            G_PARAM_CONSTRUCT_ONLY |
                                                G_PARAM_WRITABLE);
    g_object_class_install_property(object_class, PROP_IMAGE_VIEW, pspec);

    GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
    widget_class->get_preferred_width = _uni_scroll_win_get_preferred_width;
    widget_class->get_preferred_height = _uni_scroll_win_get_preferred_height;
}

static void uni_scroll_win_init(UniScrollWin *window)
{
    //window->hscroll = NULL;
    //window->vscroll = NULL;
    //window->nav_box = NULL;
    //window->nav = NULL;
    //window->show_scrollbar = FALSE;

    // Setup the navigator button.
    window->nav_button = gdk_pixbuf_new_from_xpm_data(nav_button);
    window->nav_image = gtk_image_new_from_pixbuf(window->nav_button);

    window->nav_box = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(window->nav_box), window->nav_image);
    g_signal_connect_swapped(G_OBJECT(window->nav_box),
                             "button_press_event",
                             G_CALLBACK(_uni_scroll_win_nav_btn_clicked),
                             window);

    gtk_widget_set_tooltip_text(window->nav_box,
                                _("Open the navigator window"));

    // unneeded
    //gtk_container_set_resize_mode(GTK_CONTAINER(window),
    //                              GTK_RESIZE_IMMEDIATE);
}

static void _uni_scroll_win_nav_btn_clicked(UniScrollWin *window,
                                           GdkEventButton *ev)
{
    uni_nav_show_and_grab(UNI_NAV(window->nav), ev->x_root, ev->y_root);
}

static void uni_scroll_win_set_property(GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
    UniScrollWin *window = UNI_SCROLL_WIN(object);

    if (prop_id == PROP_IMAGE_VIEW)
        _uni_scroll_win_set_view(window, g_value_get_object(value));
    else
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
}

static void _uni_scroll_win_set_view(UniScrollWin *window,
                                    UniImageView *view)
{
    // setup the scrollbars

    GtkAdjustment *hadj = (GtkAdjustment*) g_object_new(GTK_TYPE_ADJUSTMENT,
                                                        NULL);
    GtkAdjustment *vadj = (GtkAdjustment*) g_object_new(GTK_TYPE_ADJUSTMENT,
                                                        NULL);

    //window->hscroll = gtk_hscrollbar_new(hadj);
    //window->vscroll = gtk_vscrollbar_new(vadj);

    window->hscroll = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, hadj);
    window->vscroll = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, vadj);

    // We want to be notified when the adjustments change.
    g_signal_connect(hadj, "changed",
                     G_CALLBACK(_uni_scroll_win_adjustment_changed), window);
    g_signal_connect(vadj, "changed",
                     G_CALLBACK(_uni_scroll_win_adjustment_changed), window);

    // Output the adjustments to the widget.
    gtk_scrollable_set_hadjustment(GTK_SCROLLABLE(view), hadj);
    gtk_scrollable_set_vadjustment(GTK_SCROLLABLE(view), vadj);

    // Add the widgets to the table.
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS

    gtk_widget_push_composite_child();

    gtk_table_attach(GTK_TABLE(window), GTK_WIDGET(view), 0, 1, 0, 1,
                     GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(window), window->vscroll, 1, 2, 0, 1,
                     GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(window), window->hscroll, 0, 1, 1, 2,
                     GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(window), window->nav_box, 1, 2, 1, 2,
                     GTK_SHRINK, GTK_SHRINK, 0, 0);

    gtk_widget_pop_composite_child();

    G_GNUC_END_IGNORE_DEPRECATIONS

    // Create the UniNav popup.
    window->nav = uni_nav_new(view);
}

static void _uni_scroll_win_adjustment_changed(GtkAdjustment *adj,
                                              UniScrollWin *window)
{
    _uni_scroll_win_show_scrollbar(window,
                                  window->show_scrollbar
                                  && !uni_scroll_win_image_fits(window));
}

/* The size request signal needs to be implemented and return two
   constant dummy values, otherwise an infinite loop may occur when
   UniScrollWin is placed in a non-bounded container.

   When the scroll adjustments of UniImageView changes,
   uni_scroll_win_adjustment_changed will be called which may
   instruct GTK to show the scrollbars. When the scrollbars are shown
   the size has to be renegotiated. Because UniScrollWin now
   shows the widgets which it didn't before, it will be allocated a
   bigger space.

   The bigger space allocation is propagated down to UniImageView
   which updates its adjustments accordingly. Because of the bigger
   space, the scrollbars might not be needed
   anymore. uni_scroll_win_adjustment_changed is invoked again
   which may hide the scrollbars. Because UniScrollWin now hides
   widgets it previously showed, the size has to be
   renegotiated. UniScrollWin finds out that the size is now to
   small so the scrollbars has to be shown after all.

   And so it continues.
 */

static void _uni_scroll_win_get_preferred_width(GtkWidget *widget,
                                               gint *minimal_width,
                                               gint *natural_width)
{
    // chain up

    GtkWidgetClass *klass = GTK_WIDGET_CLASS(uni_scroll_win_parent_class);
    klass->get_preferred_width(widget, minimal_width, natural_width);

    *minimal_width = *natural_width = 200;
}

static void _uni_scroll_win_get_preferred_height(GtkWidget *widget,
                                                gint *minimal_height,
                                                gint *natural_height)
{
    // chain up

    GtkWidgetClass *klass = GTK_WIDGET_CLASS(uni_scroll_win_parent_class);
    klass->get_preferred_height(widget, minimal_height, natural_height);

    *minimal_height = *natural_height = 200;
}

static void uni_scroll_win_finalize(GObject *object)
{
    UniScrollWin *window = UNI_SCROLL_WIN(object);
    g_object_unref(window->nav_button);

    // Maybe window->nav should be unreferenced here.. But uh I don't
    // know how.

    gtk_widget_destroy(window->nav);

    // Chain up.
    G_OBJECT_CLASS(uni_scroll_win_parent_class)->finalize(object);
}

gboolean uni_scroll_win_image_fits(UniScrollWin *window)
{
    // Check if the current image fits in the window without the need to scoll.
    // The check is performed as if scrollbars are not visible.

    GtkAdjustment *hadj;
    hadj = gtk_range_get_adjustment(GTK_RANGE(window->hscroll));
    GtkAdjustment *vadj;
    vadj = gtk_range_get_adjustment(GTK_RANGE(window->vscroll));

    GtkAllocation allocation;

    gtk_widget_get_allocation(GTK_WIDGET(window), &allocation);

    /* We compare with the allocation size for the window instead of
       hadj->page_size and vadj->page_size. If the scrollbars are
       shown the views size is about 15 pixels shorter and thinner,
       which makes the page sizes inaccurate. The scroll windows
       allocation, on the other hand, always gives the correct number
       of pixels that COULD be shown if the scrollbars weren't
       there.
     */

    return gtk_adjustment_get_upper(hadj) <= allocation.width
           && gtk_adjustment_get_upper(vadj) <= allocation.height;
}

void uni_scroll_win_set_show_scrollbar(UniScrollWin *window, gboolean show)
{
    window->show_scrollbar = show;

    _uni_scroll_win_show_scrollbar(window,
                                  window->show_scrollbar
                                  && !uni_scroll_win_image_fits(window));
}

static void _uni_scroll_win_show_scrollbar(UniScrollWin *window, gboolean show)
{
    if (show)
    {
        gtk_widget_show_now(window->vscroll);
        gtk_widget_show_now(window->hscroll);
        gtk_widget_show_now(window->nav_box);
    }
    else
    {
        gtk_widget_hide(window->vscroll);
        gtk_widget_hide(window->hscroll);
        gtk_widget_hide(window->nav_box);
    }
}


