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

#include <config.h>
#include <gtk/gtk.h>
#include "config.h"
#include "window.h"
#include "vnr-message-area.h"
#include "file.h"
#include "list.h"
#include "vnr-tools.h"

#define PIXMAP_DIR PACKAGE_DATA_DIR "/imgview/pixmaps/"

static gchar **files = NULL; // array of files specified to be opened
static gboolean version = FALSE;
static gboolean slideshow = FALSE;
static gboolean fullscreen = FALSE;

// List of option entries The only option is for specifying file to be opened.
static GOptionEntry opt_entries[] =
{
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, "[FILE]"},
    {"version", 0, 0, G_OPTION_ARG_NONE, &version, NULL, NULL},
    {"slideshow", 0, 0, G_OPTION_ARG_NONE, &slideshow, NULL, NULL},
    {"fullscreen", 0, 0, G_OPTION_ARG_NONE, &fullscreen, NULL, NULL},
    {NULL}
};

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);

    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    GError *error = NULL;
    GOptionContext *opt_context = g_option_context_new("- Elegant Image Viewer");
    g_option_context_add_main_entries(opt_context, opt_entries, NULL);
    g_option_context_add_group(opt_context, gtk_get_option_group(TRUE));
    g_option_context_parse(opt_context, &argc, &argv, &error);

    if (error != NULL)
    {
        printf("%s\nRun 'imgview --help' to see a full list of available command line options.\n",
               error->message);
        return 1;
    }
    else if (version)
    {
        printf("%s\n", PACKAGE_STRING);
        return 0;
    }

    gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), PIXMAP_DIR);

    VnrWindow *window = window_new();
    GtkWindow *gtkwindow = GTK_WINDOW(window);

    gtk_window_set_default_size(gtkwindow, 480, 300);
    //gtk_window_set_position(window, GTK_WIN_POS_CENTER);

    GSList *uri_list = vnr_tools_get_list_from_array(files);

    GList *file_list = NULL;

    if (uri_list)
    {
        if (g_slist_length(uri_list) == 1)
        {
            file_list = vnr_list_new_for_path(uri_list->data,
                                              window->prefs->show_hidden,
                                              &error);
        }
        else
        {
            file_list = vnr_list_new_for_list(uri_list,
                                              window->prefs->show_hidden,
                                              &error);
        }

        if (error)
        {
            if (!file_list)
            {
                vnr_message_area_show(
                                VNR_MESSAGE_AREA(window->msg_area),
                                TRUE,
                                _("The given locations contain no images."),
                                TRUE);
            }
            else
            {
                vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                                      TRUE,
                                      error->message,
                                      TRUE);
            }

            // free error
        }
    }

    window_list_set(window, file_list);

    window->prefs->start_slideshow = slideshow;
    window->prefs->start_fullscreen = fullscreen;

    if (window->prefs->start_maximized)
    {
        gtk_window_maximize(gtkwindow);
    }

    gtk_widget_show(GTK_WIDGET(gtkwindow));

    gtk_main();

    return 0;
}


