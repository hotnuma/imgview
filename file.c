/*
 * Copyright © 2009-2018 Siyan Panayotov <contact@siyanpanayotov.com>
 *
 * This file is part of ImgView.
 *
 * ImgView is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *(at your option) any later version.
 *
 * ImgView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ImgView.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "file.h"
#include "config.h"
#include "vnr-tools.h"
#include <glib/gstdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


// Mime types
GList *_supported_mime_types;
static GList* _mime_types_get_supported();
static gint _compare_quarks(gconstpointer a, gconstpointer b);

// From libtinyc
static const char* path_sep(const char *path);
static const char* path_ext(const char *path, bool first);
static bool file_exists(const char *filepath);

static gchar* _file_get_copyname(const gchar *filepath);
static int file_copy(const char *from, const char *to);


// Mime types -----------------------------------------------------------------

gboolean mime_type_is_supported(const char *mime_type)
{
    GList *result;
    GQuark quark;

    if (mime_type == NULL)
    {
        return FALSE;
    }

    _supported_mime_types = _mime_types_get_supported();

    quark = g_quark_from_string(mime_type);

    result = g_list_find_custom(_supported_mime_types,
                                GINT_TO_POINTER(quark),
                                (GCompareFunc) _compare_quarks);

    return (result != NULL);
}

static GList* _mime_types_get_supported()
{
    // Modified version of eog's eog_image_get_supported_mime_types

    GSList *format_list;
    GSList *it;

    if (!_supported_mime_types)
    {
        format_list = gdk_pixbuf_get_formats();

        for (it = format_list; it != NULL; it = it->next)
        {
            gchar **mime_types =
                gdk_pixbuf_format_get_mime_types((GdkPixbufFormat *)it->data);

            for (int i = 0; mime_types[i] != NULL; ++i)
            {
                _supported_mime_types =
                    g_list_prepend(_supported_mime_types,
                                   g_strdup(mime_types[i]));
            }

            g_strfreev(mime_types);
        }

        _supported_mime_types = g_list_prepend(_supported_mime_types,
                                              "image/vnd.microsoft.icon");

        _supported_mime_types = g_list_sort(_supported_mime_types,
                                           (GCompareFunc) _compare_quarks);

        g_slist_free(format_list);
    }

    return _supported_mime_types;
}

static gint _compare_quarks(gconstpointer a, gconstpointer b)
{
    GQuark quark;

    quark = g_quark_from_string((const gchar *)a);

    return quark - GPOINTER_TO_INT(b);
}


// From libtinyc --------------------------------------------------------------

static const char* path_sep(const char *path)
{
    if (!path)
        return NULL;

    const char *sep = NULL;

    while (*path)
    {
        if (*path == '/')
            sep = path;

        ++path;
    }

    return sep;
}

static const char* path_ext(const char *path, bool first)
{
    const char *sep = path_sep(path);

    if (sep)
    {
        path = ++sep;

        if (*path == '\0')
            return NULL;
    }

    // hidden file.
    if (*path == '.')
        ++path;

    const char *found = NULL;

    while (*path)
    {
        if (*path == '.' && path[1] != '\0')
        {
            found = path;

            if (first)
                return found;
        }

        ++path;
    }

    return found;
}

static bool file_exists(const char *filepath)
{
    if (!filepath)
        return false;

    struct stat st;
    int result = stat(filepath, &st);

    return (result == 0);
}


// ----------------------------------------------------------------------------

static gchar* _file_get_copyname(const gchar *filepath)
{
    if (!filepath)
        return NULL;

    gchar *basepath = g_strdup(filepath);
    gchar *ext = (gchar*) path_ext(basepath, false);
    if (ext)
    {
        *ext = '\0';
        ++ext;
    }

    gchar *newpath = NULL;

    for (int i = 1; i < 100; ++i)
    {
        if (ext)
        {
            newpath = g_strdup_printf("%s-copy%02d.%s",
                                      basepath,
                                      i, ext);
        }
        else
        {
            newpath = g_strdup_printf("%s-copy%02d",
                                      basepath,
                                      i);
        }

        if (file_exists(newpath))
        {
            g_free(newpath);
            newpath = NULL;
        }
        else
        {
            break;
        }
    }

    g_free(basepath);

    return newpath;
}

static int file_copy(const char *from, const char *to)
{
    // https://stackoverflow.com/questions/2180079/

    char buf[4096];
    ssize_t nread;

    int fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    int fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }

        close(fd_from);

        // Success!

        return 0;
    }

out_error:

    int saved_errno = errno;

    close(fd_from);

    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;

    return -1;
}


// VnrFile --------------------------------------------------------------------

G_DEFINE_TYPE(VnrFile, vnr_file, G_TYPE_OBJECT)

static void vnr_file_finalize(GObject *object);
static gboolean _vnr_file_set_path(VnrFile *file, const gchar *filepath);

static void vnr_file_class_init(VnrFileClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = vnr_file_finalize;
}

static void vnr_file_init(VnrFile *file)
{
    //g_assert(file->display_name == NULL);
    //file->display_name = NULL;
}

static void vnr_file_finalize(GObject *object)
{
    VnrFile *file = VNR_FILE(object);

    g_free(file->display_name);
    g_free(file->display_name_collate);
    g_free(file->path);

    G_OBJECT_CLASS(vnr_file_parent_class)->finalize(object);
}

VnrFile* vnr_file_new()
{
    return VNR_FILE(g_object_new(VNR_TYPE_FILE, NULL));
}

VnrFile* vnr_file_new_for_path(const gchar *filepath, gboolean include_hidden)
{
    if (!filepath)
        return NULL;

    GFile *file = g_file_new_for_path(filepath);

    GFileInfo *fileinfo = g_file_query_info(
                            file,
                            G_FILE_ATTRIBUTE_STANDARD_TYPE ","
                            G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
                            G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
                            G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ","
                            G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN ","
                            G_FILE_ATTRIBUTE_TIME_MODIFIED,
                            0, NULL, NULL);

    if (fileinfo == NULL)
    {
        g_object_unref(file);
        return NULL;
    }

    GFileType filetype = g_file_info_get_file_type(fileinfo);

    if (filetype == G_FILE_TYPE_DIRECTORY)
    {
        g_object_unref(file);
        g_object_unref(fileinfo);

        return NULL;
    }

    VnrFile *vnrfile = vnr_file_new();
    const char *mimetype = g_file_info_get_content_type(fileinfo);

    if (mimetype == NULL)
    {
        mimetype = g_file_info_get_attribute_string(
                    fileinfo,
                    G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
    }

    gboolean included = (!g_file_info_get_is_hidden(fileinfo) || include_hidden);

    if (!included || !mime_type_is_supported(mimetype))
    {
        g_object_unref(file);
        g_object_unref(fileinfo);
        g_object_unref(vnrfile);

        return NULL;
    }

    vnr_file_set_display_name(
                        vnrfile,
                        (char*) g_file_info_get_display_name(fileinfo));

    vnrfile->mtime = g_file_info_get_attribute_uint64(
                        fileinfo,
                        G_FILE_ATTRIBUTE_TIME_MODIFIED);

    vnrfile->path = g_strdup(filepath);

    g_object_unref(file);
    g_object_unref(fileinfo);

    return vnrfile;
}

void vnr_file_set_display_name(VnrFile *vnr_file,
                                       const gchar *display_name)
{
    g_free(vnr_file->display_name);
    vnr_file->display_name = g_strdup(display_name);

    g_free(vnr_file->display_name_collate);
    vnr_file->display_name_collate =
            g_utf8_collate_key_for_filename(display_name, -1);
}

gboolean vnr_file_copy(VnrFile *file, const gchar *filepath, gchar **outpath)
{
    if (!file || !filepath)
        return false;

    if (!file_exists(filepath))
        return (file_copy(file->path, filepath) == 0);

    gchar *newpath = _file_get_copyname(filepath);
    if (!newpath)
        return false;

    gboolean ret = (file_copy(file->path, newpath) == 0);

    if (ret && outpath)
        *outpath = newpath;
    else
        g_free(newpath);

    return ret;
}

gboolean vnr_file_rename(VnrFile *file, const gchar *filepath)
{
    if (!file || !filepath)
        return false;

    //printf("rename %s to %s\n", file->path, fullpath);

    gboolean ret = (g_rename(file->path, filepath) == 0);

    if (ret)
        _vnr_file_set_path(file, filepath);

    return ret;
}

static gboolean _vnr_file_set_path(VnrFile *file, const gchar *filepath)
{
    GFile *gfile = g_file_new_for_path(filepath);

    GFileInfo *fileinfo = g_file_query_info(
        gfile,
        G_FILE_ATTRIBUTE_STANDARD_TYPE ","
        G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
        0, NULL, NULL);

    if (fileinfo == NULL)
    {
        g_object_unref(gfile);
        return false;
    }

    g_free(file->path);
    file->path = g_strdup(filepath);

    g_free(file->display_name);
    file->display_name = g_strdup(g_file_info_get_display_name(fileinfo));

    g_free(file->display_name_collate);
    file->display_name_collate =
            g_utf8_collate_key_for_filename(file->display_name, -1);

    file->mtime = g_file_info_get_attribute_uint64(
                                    fileinfo,
                                    G_FILE_ATTRIBUTE_TIME_MODIFIED);

    g_object_unref(gfile);
    g_object_unref(fileinfo);

    return true;
}


