#ifndef LIST_H
#define LIST_H

#include "file.h"
#include <glib.h>

G_BEGIN_DECLS

// create ---------------------------------------------------------------------

GList* vnr_list_new_for_path(gchar *filepath,
                             gboolean include_hidden, GError **error);
GList* vnr_list_new_for_file(gchar *filepath,
                             gboolean include_hidden, gboolean try_first);
GList* vnr_list_new_for_dir(gchar *directory, gboolean sort,
                            gboolean include_hidden);
GList* vnr_list_new_for_list(GSList *uri_list,
                             gboolean include_hidden, GError **error);

// delete ---------------------------------------------------------------------

GList* vnr_list_delete_link(GList *list);
GList* vnr_list_delete_item(GList *list);
GList* vnr_list_free(GList *list);

// ----------------------------------------------------------------------------

GList* vnr_list_find(GList *list, const char *filepath);
gint vnr_list_get_position(GList *list, gint *total);
GList* vnr_list_insert(GList *list, VnrFile *newfile);
GList* vnr_list_sort(GList *list);

G_END_DECLS

#endif // LIST_H


