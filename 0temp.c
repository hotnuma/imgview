#if 0

static void _action_resize(VnrWindow *window, GtkWidget *widget);
static void _action_resize(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;

    //if (action != NULL && !gtk_toggle_action_get_active(action))
    //{
    //    window->prefs->auto_resize = FALSE;
    //    return;
    //}

    // width and Height of the pixbuf
    gint img_w = window->current_image_width;
    gint img_h = window->current_image_height;

    if (img_w == 0 || img_h == 0)
        return;

    window->prefs->auto_resize = TRUE;
    vnr_tools_fit_to_size(&img_w, &img_h,
                          window->max_width, window->max_height);

    // _window_get_top_widgets_height(window)

    gtk_window_resize(GTK_WINDOW(window), img_w, img_h);
}

#endif


