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

#ifndef VNR_FILE_H
#define VNR_FILE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


// Mime types -----------------------------------------------------------------

gboolean mime_type_is_supported(const char *mime_type);

// VnrFile -------------------------------------------------------------------

#define VNR_TYPE_FILE (vnr_file_get_type())
G_DECLARE_FINAL_TYPE(VnrFile, vnr_file, VNR, FILE, GObject)

typedef struct _VnrFile VnrFile;

struct _VnrFile
{
    GObject __parent__;

    gchar *display_name;
    gchar *display_name_collate;
    gchar *path;
    time_t mtime;
};

GType vnr_file_get_type() G_GNUC_CONST;

VnrFile* vnr_file_new();
VnrFile* vnr_file_new_for_path(const gchar *filepath, gboolean include_hidden);
void vnr_file_set_display_name(VnrFile *vnr_file, const gchar *display_name);
gboolean vnr_file_copy(VnrFile *file, const gchar *filepath, gchar **newpath);
gboolean vnr_file_rename(VnrFile *file, const gchar *filepath);

G_END_DECLS

#endif // VNR_FILE_H


