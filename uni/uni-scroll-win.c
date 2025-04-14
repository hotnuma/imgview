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
#include "uni-anim-view.h"
//#include "uni-image-view.h"

static void uni_scroll_win_finalize(GObject *object);
static void uni_scroll_win_set_property(GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec);
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


// creation / destruction -----------------------------------------------------

G_DEFINE_TYPE(UniScrollWin, uni_scroll_win, GTK_TYPE_GRID)

enum
{
    PROP_IMAGE_VIEW = 1
};

GtkWidget* uni_scroll_win_new()
{
    GtkWidget *view = uni_anim_view_new();
    gtk_widget_set_can_focus(view, TRUE);

    return GTK_WIDGET(g_object_new(UNI_TYPE_SCROLL_WIN,
                                   "view", view,
                                   NULL));
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
                                            G_PARAM_CONSTRUCT_ONLY
                                            | G_PARAM_WRITABLE);

    g_object_class_install_property(object_class, PROP_IMAGE_VIEW, pspec);

    GtkWidgetClass *widget_class = (GtkWidgetClass*) klass;
    widget_class->get_preferred_width = _uni_scroll_win_get_preferred_width;
    widget_class->get_preferred_height = _uni_scroll_win_get_preferred_height;
}

static void uni_scroll_win_init(UniScrollWin *window)
{
    (void) window;

    // unneeded
    //gtk_container_set_resize_mode(GTK_CONTAINER(window),
    //                              GTK_RESIZE_IMMEDIATE);
}

static void uni_scroll_win_finalize(GObject *object)
{
    // Chain up.
    G_OBJECT_CLASS(uni_scroll_win_parent_class)->finalize(object);
}

static void uni_scroll_win_set_property(GObject *object, guint prop_id,
                                        const GValue *value, GParamSpec *pspec)
{
    UniScrollWin *window = UNI_SCROLL_WIN(object);

    switch (prop_id)
    {
    case PROP_IMAGE_VIEW:
        _uni_scroll_win_set_view(window, g_value_get_object(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void _uni_scroll_win_set_view(UniScrollWin *window, UniImageView *view)
{
    // setup the scrollbars

    GtkAdjustment *hadj = (GtkAdjustment*) g_object_new(GTK_TYPE_ADJUSTMENT,
                                                        NULL);
    GtkAdjustment *vadj = (GtkAdjustment*) g_object_new(GTK_TYPE_ADJUSTMENT,
                                                        NULL);

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

    gtk_grid_attach(GTK_GRID(window), GTK_WIDGET(view), 0, 0, 1, 1);
    gtk_widget_set_hexpand(GTK_WIDGET(view), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(view), TRUE);
    gtk_grid_attach(GTK_GRID(window), window->vscroll, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(window), window->hscroll, 0, 1, 1, 1);

    window->view = GTK_WIDGET(view);
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


// public ---------------------------------------------------------------------

GtkWidget *uni_scroll_win_get_view(UniScrollWin *window)
{
    return window->view;
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
    if (!show)
    {
        gtk_widget_hide(window->vscroll);
        gtk_widget_hide(window->hscroll);

        return;
    }

    gtk_widget_show_now(window->vscroll);
    gtk_widget_show_now(window->hscroll);

}


