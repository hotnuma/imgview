#if 0

// ----------------------------------------------------------------------------

gchar *directory = g_path_get_dirname(filepath);
filelist = vnr_list_new_for_dir(directory, TRUE, include_hidden);
g_free(directory);

if (filelist)
{
    GList *find = vnr_list_find(filelist, filepath);

    if (!find)
        filelist = vnr_list_free(filelist);
    else
        filelist = find;
}

// ----------------------------------------------------------------------------

if (!list)
    return NULL;

GList *found = vnr_list_find(list, filepath);

if (!found)
{
    vnr_list_free(list);

    return NULL;
}

return found;

// ----------------------------------------------------------------------------

vnr-crop.c

        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        gdk_screen_get_monitor_geometry(
                        screen,
                        gdk_screen_get_monitor_at_window(screen,
                        gtk_widget_get_window(GTK_WIDGET(crop->vnr_win))),
                        &geometry);
        G_GNUC_END_IGNORE_DEPRECATIONS

// ----------------------------------------------------------------------------

window.c

        G_GNUC_BEGIN_IGNORE_DEPRECATIONS

        gdk_screen_get_monitor_geometry(
                            screen,
                            gdk_screen_get_monitor_at_window(screen,
                            gtk_widget_get_window(widget)),
                            &geometry);

        G_GNUC_END_IGNORE_DEPRECATIONS

// ----------------------------------------------------------------------------

file_copy

g_autoptr(GFile) src = g_file_new_for_path(file->path);
g_autoptr(GFile) dest = g_file_new_for_path(fullpath);

gboolean ret = g_file_copy(src,
                           dest,
                           G_FILE_COPY_ALL_METADATA,
                           NULL, NULL, NULL,
                           NULL);

// ----------------------------------------------------------------------------

keycodes

case GDK_KEY_F10:
    _window_action_slideshow(window);
    result = TRUE;
    break;

case GDK_KEY_F11:
    window_fullscreen_toggle(window);
    result = TRUE;
    break;

// ----------------------------------------------------------------------------

vnf_list_new_multiple

gchar *filepath = uri_list->data;

GFile *file = g_file_new_for_path(filepath);

GFileInfo *fileinfo = g_file_query_info(
                        file,
                        G_FILE_ATTRIBUTE_STANDARD_TYPE ","
                        G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
                        G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
                        G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ","
                        G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN ","
                        G_FILE_ATTRIBUTE_TIME_MODIFIED,
                        0, NULL, error);

if (fileinfo == NULL)
{
    g_clear_error(error);
    g_object_unref(file);

    uri_list = g_slist_next(uri_list);
    continue;
}

GFileType filetype = g_file_info_get_file_type(fileinfo);

if (filetype != G_FILE_TYPE_DIRECTORY)
{
    VnrFile *vnrfile = vnr_file_new();
    const char *mimetype = g_file_info_get_content_type(fileinfo);

    if (mimetype == NULL)
    {
        mimetype = g_file_info_get_attribute_string(
                    fileinfo,
                    G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
    }

    if (mime_type_is_supported(mimetype)
        && (include_hidden || !g_file_info_get_is_hidden(fileinfo)))
    {
        vnr_file_set_display_name(
                            vnrfile,
                            (char*) g_file_info_get_display_name(fileinfo));

        vnrfile->mtime = g_file_info_get_attribute_uint64(
                            fileinfo,
                            G_FILE_ATTRIBUTE_TIME_MODIFIED);

        vnrfile->path = g_strdup(filepath);

        file_list = g_list_prepend(file_list, vnrfile);
    }
}

g_object_unref(file);
g_object_unref(fileinfo);

uri_list = g_slist_next(uri_list);

#endif


