/*
 * from Thunar by XFCE developpers
 */

#include "dialog.h"
#include "config.h"
#include "xfce-filename-input.h"

static void _dialog_select_filename(GtkWidget *entry, const gchar *filename);
static inline gchar *_util_strrchr_offset(const gchar *str,
                                          const gchar *offset, gchar c);

gboolean dialog_file_rename(GtkWindow *window, VnrFile *file)
{
    gchar *filename = g_path_get_basename(file->path);

    // create a new dialog window
    gchar *title = g_strdup_printf(_("Rename \"%s\""), filename);

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        title,
        window,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        _("_Cancel"),
        GTK_RESPONSE_CANCEL,
        _("_Rename"),
        GTK_RESPONSE_OK,
        NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

    g_free(title);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 3);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 6);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), grid, TRUE, TRUE, 0);
    gtk_widget_show(grid);

    GtkWidget *label = gtk_label_new(_("Enter the new name:"));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 1, 0, 1, 1);
    gtk_widget_show(label);

    // set up the widget for entering the filename
    XfceFilenameInput *filename_input;
    filename_input = g_object_new(XFCE_TYPE_FILENAME_INPUT,
                                  "original-filename", filename,
                                  NULL);
    gtk_widget_set_hexpand(GTK_WIDGET(filename_input), TRUE);
    gtk_widget_set_valign(GTK_WIDGET(filename_input), GTK_ALIGN_CENTER);

    /* connect to signals so that the sensitivity of the Create button is updated according to whether there
     * is a valid file name entered */
    g_signal_connect_swapped(
        filename_input, "text-invalid",
        G_CALLBACK(xfce_filename_input_desensitise_widget),
        gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK));

    g_signal_connect_swapped(
        filename_input, "text-valid",
        G_CALLBACK(xfce_filename_input_sensitise_widget),
        gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK));

    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(filename_input), 1, 1, 1, 1);

    gtk_widget_show_all(GTK_WIDGET(filename_input));

    // ensure that the sensitivity of the Create button is set correctly
    xfce_filename_input_check(filename_input);

    // select the filename without the extension
    GtkEntry *entry = xfce_filename_input_get_entry(filename_input);
    _dialog_select_filename(GTK_WIDGET(entry), filename);

    // get the size the entry requires to render the full text
    PangoLayout *layout = gtk_entry_get_layout(entry);

    gint layout_width;
    pango_layout_get_pixel_size(layout, &layout_width, NULL);
    gint layout_offset;
    gtk_entry_get_layout_offsets(entry, &layout_offset, NULL);
    // 12px free space in entry
    layout_width += (layout_offset * 2) + (12 * 4) + 48;

    gint parent_width = 500;

    // parent window width
    if (window != NULL)
    {
        // keep below 90% of the parent window width
        gtk_window_get_size(GTK_WINDOW(window), &parent_width, NULL);
        parent_width *= 0.90f;
    }

    // resize the dialog to make long names fit as much as possible
    gtk_window_set_default_size(GTK_WINDOW(dialog),
                                CLAMP(layout_width, 300, parent_width),
                                -1);

    // automatically close the dialog when the file is destroyed
    // g_signal_connect_swapped(G_OBJECT(file), "destroy",
    //                         G_CALLBACK(gtk_widget_destroy), dialog);

    // run the dialog
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gboolean ret = false;

    if (response == GTK_RESPONSE_OK)
    {
        // hide the dialog
        gtk_widget_hide(dialog);

        // determine the new filename
        const gchar *newname = xfce_filename_input_get_text(filename_input);

        // check if we have a new name here
        if (newname != NULL
            && g_strcmp0(filename, newname) != 0
            && g_utf8_validate(newname, -1, NULL))
        {
            gchar *dir = g_path_get_dirname(file->path);
            gchar *fullpath = g_build_filename(dir, newname, NULL);
            g_free(dir);

            ret = vnr_file_rename(file, fullpath);

            g_free(fullpath);
        }
    }

    // cleanup
    g_free(filename);

    if (response != GTK_RESPONSE_NONE)
        gtk_widget_destroy(dialog);

    return ret;
}


// string functions  ----------------------------------------------------------

/**
 * util_str_get_extension
 * @filename : an UTF-8 filename
 *
 * Returns a pointer to the extension in @filename.
 *
 * This is an improved version of g_utf8_strrchr with
 * improvements to recognize compound extensions like
 * ".tar.gz" and ".desktop.in.in".
 *
 * Return value: pointer to the extension in @filename
 *               or NULL.
**/
gchar* util_str_get_extension(const gchar *filename)
{
    static const gchar *compressed[] =
                        {"gz", "bz2", "lzma", "lrz", "rpm", "lzo", "xz", "z"};
    gchar *dot;
    gchar *ext;
    guint i;
    gchar *dot2;
    gsize len;
    gboolean is_in;

    // check if there is an possible extension part in the name
    dot = strrchr(filename, '.');
    if (dot == NULL || dot == filename || dot[1] == '\0')
        return NULL;

    // skip the .
    ext = dot + 1;

    // check if this looks like a compression mime-type
    for (i = 0; i < G_N_ELEMENTS(compressed); i++)
    {
        if (strcasecmp(ext, compressed[i]) == 0)
        {
            // look for a possible container part(tar, psd, epsf)
            dot2 = _util_strrchr_offset(filename, dot - 1, '.');
            if (dot2 != NULL && dot2 != filename)
            {
                // check the 2nd part range, keep it between 2 and 5 chars
                len = dot - dot2 - 1;
                if (len >= 2 && len <= 5)
                    dot = dot2;
            }

            // that's it for compression types
            return dot;
        }
    }

    /* for coders, .in are quite common, so check for those too
     * with a max of 3 rounds(2x .in and the possibly final extension) */
    if (strcasecmp(ext, "in") == 0)
    {
        for (i = 0, is_in = TRUE; is_in && i < 3; i++)
        {
            dot2 = _util_strrchr_offset(filename, dot - 1, '.');
            // the extension before .in could be long. check that it's at least 2 chars
            len = dot - dot2 - 1;
            if (dot2 == NULL || dot2 == filename || len < 2)
                break;

            // continue if another .in was found
            is_in = dot - dot2 == 3 && strncasecmp(dot2, ".in", 3) == 0;

            dot = dot2;
        }
    }

    return dot;
}

/**
 * _util_strrchr_offset:
 * @str:    haystack
 * @offset: pointer offset in @str
 * @c:      search needle
 *
 * Return the last occurrence of the character @c in
 * the string @str starting at @offset.
 *
 * There are also Glib functions for this like g_strrstr_len
 * and g_utf8_strrchr, but these work internally the same
 * as this function(tho, less efficient).
 *
 * Return value: pointer in @str or NULL.
 **/
static inline gchar* _util_strrchr_offset(const gchar *str, const gchar *offset,
                                          gchar c)
{
    const gchar *p;

    for (p = offset; p > str; --p)
    {
        if (*p == c)
            return (gchar *)p;
    }

    return NULL;
}


static void _dialog_select_filename(GtkWidget *entry, const gchar *filename)
{
#if 0
    const gchar *filename;
    check if we have a directory here

    if (th_file_is_directory(file))
    {
        gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
        return;
    }

    filename = th_file_get_display_name(file);
#endif

    // check if the filename contains an extension
    const gchar *ext = util_str_get_extension(filename);
    if (ext == NULL)
        return;

    // grab focus to the entry first, else the selection will be altered later
    gtk_widget_grab_focus(entry);

    // determine the UTF-8 char offset
    glong offset = g_utf8_pointer_to_offset(filename, ext);

    // select the text prior to the dot
    if (offset > 0)
        gtk_editable_select_region(GTK_EDITABLE(entry), 0, offset);
}


