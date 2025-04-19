#if 0

//gint64 t1 = g_get_real_time();

//gint64 t2 = g_get_real_time();
//gint64 diff = t2 - t1;
//printf("time = %d\n", (int) diff);

static void _window_action_resetdir(VnrWindow *window, GtkWidget *widget);
static void _window_action_resetdir(VnrWindow *window, GtkWidget *widget)
{
    g_return_if_fail(window != NULL);
    (void) widget;

    if (window_get_current_file(window) == NULL
        || window->mode != WINDOW_MODE_NORMAL)
        return;

    g_free(window->destdir);

    window->destdir = NULL;
}

static void _window_non_anim_new(VnrWindow *window, GdkPixbuf *pixbuf);
static void _window_non_anim_new(VnrWindow *window, GdkPixbuf *pixbuf)
{
    GdkPixbufAnimation *anim = gdk_pixbuf_non_anim_new(pixbuf);
    g_object_unref(pixbuf);

    window_load_pixbuf(window, anim, true);
    g_object_unref(anim);

    window->modified = true;

    window->current_image_width = gdk_pixbuf_get_width(pixbuf);
    window->current_image_height = gdk_pixbuf_get_height(pixbuf);

    //gtk_action_group_set_sensitive(window->action_save, TRUE);

    if (window->writable_format_name == NULL)
    {
        vnr_message_area_show(
                VNR_MESSAGE_AREA(window->msg_area),
                TRUE,
                _("Image modifications cannot be saved.\n"
                  "Writing in this format is not supported."),
                FALSE);
    }
}


#endif


