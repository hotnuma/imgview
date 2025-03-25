#include "list.h"
#include "config.h"

static gint _file_compare_func(VnrFile *file, char *uri);
static gint _list_compare_func(gconstpointer a, gconstpointer b,
                               gpointer user_data);

// create ---------------------------------------------------------------------

GList* vnr_list_new_for_path(gchar *filepath, gboolean include_hidden,
                             GError **error)
{
    GFile *file = g_file_new_for_path(filepath);

    GFileInfo *fileinfo = g_file_query_info(
        file,
        G_FILE_ATTRIBUTE_STANDARD_TYPE ","
        G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
        0, NULL, error);

    if (fileinfo == NULL)
        return NULL;

    GFileType filetype = g_file_info_get_file_type(fileinfo);

    GList *filelist = NULL;

    if (filetype == G_FILE_TYPE_DIRECTORY)
    {
        filelist = vnr_list_new_for_dir(filepath, TRUE, include_hidden);
    }
    else
    {
        filelist = vnr_list_new_for_file(filepath, include_hidden, false);
    }

    g_object_unref(fileinfo);
    g_object_unref(file);

    return filelist;
}

GList* vnr_list_new_for_file(gchar *filepath,
                             gboolean include_hidden, gboolean try_first)
{
    if (!filepath)
        return NULL;

    gchar *directory = g_path_get_dirname(filepath);
    if (!directory)
        return NULL;

    GList *filelist = vnr_list_new_for_dir(directory, TRUE, include_hidden);
    g_free(directory);

    if (!filelist)
        return NULL;

    GList *find = vnr_list_find(filelist, filepath);

    if (!find)
    {
        if (try_first)
            return g_list_first(filelist);
        else
            return vnr_list_free(filelist);
    }

    return find;
}

GList* vnr_list_new_for_dir(gchar *directory, gboolean sort,
                            gboolean include_hidden)
{
    if (!directory)
        return NULL;

    GFile *gfile = g_file_new_for_path(directory);

    GFileEnumerator *file_enum = g_file_enumerate_children(
                        gfile,
                        G_FILE_ATTRIBUTE_STANDARD_NAME ","
                        G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
                        G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
                        G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ","
                        G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN ","
                        G_FILE_ATTRIBUTE_TIME_MODIFIED,
                        G_FILE_QUERY_INFO_NONE,
                        NULL, NULL);

    GList *list = NULL;

    GFileInfo *fileinfo = g_file_enumerator_next_file(file_enum, NULL, NULL);

    while (fileinfo)
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

            vnrfile->path = g_strjoin(G_DIR_SEPARATOR_S, directory,
                                       vnrfile->display_name, NULL);

            list = g_list_prepend(list, vnrfile);
        }

        g_object_unref(fileinfo);

        fileinfo = g_file_enumerator_next_file(file_enum, NULL, NULL);
    }

    g_object_unref(gfile);
    g_file_enumerator_close(file_enum, NULL, NULL);
    g_object_unref(file_enum);

    if (sort)
        list = vnr_list_sort(list);

    return list;
}

GList* vnr_list_new_for_list(GSList *uri_list,
                             gboolean include_hidden,
                             GError **error)
{
    (void) error;

    GList *file_list = NULL;

    while (uri_list != NULL)
    {
        VnrFile *vnrfile = vnr_file_new_for_path(uri_list->data,
                                                 include_hidden);
        if (vnrfile)
            file_list = g_list_prepend(file_list, vnrfile);

        uri_list = g_slist_next(uri_list);

    }

    file_list = vnr_list_sort(file_list);

    return file_list;
}

// delete ---------------------------------------------------------------------

GList* vnr_list_delete_link(GList *list)
{
    if (list == NULL)
        return NULL;

    VnrFile *file = VNR_FILE(list->data);
    if (file)
        g_object_unref(file);

    return g_list_delete_link(list, list);
}

GList* vnr_list_delete_item(GList *list)
{
    GList *first = g_list_first(list);

    // empty list
    if (first == NULL)
        return NULL;

    // only one item in the list
    if (g_list_length(first) == 1)
        return vnr_list_free(first);

    GList *next = g_list_next(list);
    if (next == NULL)
        next = first;

    g_assert(next != list);

    vnr_list_delete_link(list);

    return next;
}

GList* vnr_list_free(GList *list)
{
    if (!list)
        return NULL;

    GList *first = g_list_first(list);
    g_list_free_full(first, g_object_unref);

    return NULL;
}

// ----------------------------------------------------------------------------

GList* vnr_list_find(GList *list, const char *filepath)
{
    return g_list_find_custom(list, filepath,
                              (GCompareFunc) _file_compare_func);
}

static gint _file_compare_func(VnrFile *file, char *uri)
{
    if (g_strcmp0(uri, file->path) == 0)
        return 0;
    else
        return 1;
}

gint vnr_list_get_position(GList *list, gint *total)
{
    gint after = 0;
    gint before = 0;

    for (GList *it = list; it != NULL; it = it->next)
        ++after;

    for (GList *it = list; it != NULL; it = it->prev)
        ++before;

    if (total)
        *total = before + after - 1;

    return before;
}

GList* vnr_list_insert(GList *list, VnrFile *newfile)
{
    GList *first = g_list_first(list);
    GList *l = first;

    while (l)
    {
        VnrFile *file = VNR_FILE(l->data);

        if (g_strcmp0(file->path, newfile->path) == 0)
            return NULL;

        l = l->next;
    }

    return g_list_insert_sorted_with_data(
                                    first,
                                    newfile,
                                    _list_compare_func,
                                    NULL);
}

GList* vnr_list_sort(GList *list)
{
    return g_list_sort_with_data(list, _list_compare_func, NULL);
}

static gint _list_compare_func(gconstpointer a, gconstpointer b,
                               gpointer user_data)
{
    (void) user_data;

    return g_strcmp0(VNR_FILE((void*) a)->display_name_collate,
                     VNR_FILE((void*) b)->display_name_collate);
}


