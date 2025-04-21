#if 0

gint64 t1 = g_get_monotonic_time();

gint64 t2 = g_get_monotonic_time();
gint64 diff = t2 - t1;
printf("time = %d\n", (int) diff);

// ----------------------------------------------------------------------------

static void _window_filter_grayscale(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;

    if (!window->can_edit)
        return;

    GdkPixbuf *src_pixbuf = uni_image_view_get_pixbuf(
                                        UNI_IMAGE_VIEW(window->view));

    GdkPixbuf *dest_pixbuf = _window_pixbuf_new(window);

    int height = gdk_pixbuf_get_height(dest_pixbuf);
    int stride = gdk_pixbuf_get_rowstride(dest_pixbuf);
    int max_h = height * stride;

    int width = gdk_pixbuf_get_width(dest_pixbuf);
    int channels = gdk_pixbuf_get_n_channels(dest_pixbuf);
    int max_w = width * channels;

    assert(stride == gdk_pixbuf_get_rowstride(src_pixbuf));
    assert(channels == gdk_pixbuf_get_n_channels(src_pixbuf));

    gboolean has_alpha = gdk_pixbuf_get_has_alpha(dest_pixbuf);

    guchar *src = gdk_pixbuf_get_pixels(src_pixbuf);
    guchar *dest = gdk_pixbuf_get_pixels(dest_pixbuf);

    //gint64 t1 = g_get_monotonic_time();

    for (int y = 0; y < max_h; y+=stride)
    {
        for (int x = 0; x < max_w; x+=channels)
        {
            int offset = y + x;

            guchar *src_color = src + offset;
            guchar *dest_color = dest + offset;

            float level =    src_color[0] * 0.3086
                           + src_color[1] * 0.6094
                           + src_color[2] * 0.0820;

            dest_color[0] = (uint8_t) CLAMP(level, 0, 255);
            dest_color[1] = dest_color[0];
            dest_color[2] = dest_color[0];

            if (has_alpha)
                dest_color[3] = src_color[3];
        }
    }

    //gint64 t2 = g_get_monotonic_time();
    //gint64 diff = t2 - t1;
    //printf("time = %d\n", (int) diff);

    _window_view_set_static(window, dest_pixbuf);
}

static void _window_filter_sepia(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;

    if (!window->can_edit)
        return;

    GdkPixbuf *src_pixbuf = uni_image_view_get_pixbuf(
                                        UNI_IMAGE_VIEW(window->view));

    GdkPixbuf *dest_pixbuf = _window_pixbuf_new(window);

    int height = gdk_pixbuf_get_height(dest_pixbuf);
    int stride = gdk_pixbuf_get_rowstride(dest_pixbuf);
    int max_h = height * stride;

    int width = gdk_pixbuf_get_width(dest_pixbuf);
    int channels = gdk_pixbuf_get_n_channels(dest_pixbuf);
    int max_w = width * channels;

    assert(stride == gdk_pixbuf_get_rowstride(src_pixbuf));
    assert(channels == gdk_pixbuf_get_n_channels(src_pixbuf));

    gboolean has_alpha = gdk_pixbuf_get_has_alpha(dest_pixbuf);

    guchar *src = gdk_pixbuf_get_pixels(src_pixbuf);
    guchar *dest = gdk_pixbuf_get_pixels(dest_pixbuf);

    //gint64 t1 = g_get_monotonic_time();

    for (int y = 0; y < max_h; y+=stride)
    {
        for (int x = 0; x < max_w; x+=channels)
        {
            int offset = y + x;

            guchar *src_color = src + offset;
            guchar *dest_color = dest + offset;

            float level;

            level =   (src_color[0] * 0.393)
                    + (src_color[1] * 0.769)
                    + (src_color[2] * 0.189);

            dest_color[0] = (uint8_t) CLAMP(level, 0, 255);

            level =   (src_color[0] * 0.349)
                    + (src_color[1] * 0.686)
                    + (src_color[2] * 0.168);

            dest_color[1] = (uint8_t) CLAMP(level, 0, 255);

            level =   (src_color[0] * 0.272)
                    + (src_color[1] * 0.534)
                    + (src_color[2] * 0.131);

            dest_color[2] = (uint8_t) CLAMP(level, 0, 255);

            if (has_alpha)
                dest_color[3] = src_color[3];
        }
    }

    //gint64 t2 = g_get_monotonic_time();
    //gint64 diff = t2 - t1;
    //printf("time = %d\n", (int) diff);

    _window_view_set_static(window, dest_pixbuf);
}

// ----------------------------------------------------------------------------

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


