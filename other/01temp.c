
#if 0

static void _action_scrollbar(GtkAction *action, VnrWindow *window)
{
    gboolean show = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));
    vnr_prefs_set_show_scrollbar(window->prefs, show);
    uni_scroll_win_set_show_scrollbar(UNI_SCROLL_WIN(window->scroll_view), show);
}

static void _action_first(GtkAction *action, gpointer user_data)
{
    window_first(VNR_WINDOW(user_data));
}

static void _action_last(GtkAction *action, gpointer user_data)
{
    window_last(VNR_WINDOW(user_data));
}

static void _action_zoom_in(GtkAction *action, gpointer user_data)
{
    uni_image_view_zoom_in(UNI_IMAGE_VIEW(VNR_WINDOW(user_data)->view));
}

static void _action_zoom_out(GtkAction *action, gpointer user_data)
{
    uni_image_view_zoom_out(UNI_IMAGE_VIEW(VNR_WINDOW(user_data)->view));
}

static void _action_about(GtkAction *action, VnrWindow *window)
{
    static const char *authors[] =
    {
        "Programming &amp; icon design",
        "\tSiyan Panayotov",
        "\nRefer to source code from GtkImageView",
        NULL
    };

    char *license =
        ("Viewnior is free software: you can redistribute it and/or modify "
         "it under the terms of the GNU General Public License as published by "
         "the Free Software Foundation, either version 3 of the License, or "
         "(at your option) any later version.\n\n"
         "Viewnior is distributed in the hope that it will be useful, "
         "but WITHOUT ANY WARRANTY; without even the implied warranty of "
         "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
         "GNU General Public License for more details.\n\n"
         "You should have received a copy of the GNU General Public License "
         "along with Viewnior.  If not, see <http://www.gnu.org/licenses/>.\n");

    gtk_show_about_dialog(
                GTK_WINDOW(window),
                "program-name", "Viewnior",
                "version", VERSION,
                "copyright", "Copyright \xc2\xa9 2009-2018 Siyan Panayotov",
                "comments", _("Elegant Image Viewer"),
                "authors", authors,
                "logo-icon-name", "viewnior",
                "wrap-license", TRUE,
                "license", license,
                "website", "https://github.com/hotnuma/viewnior",
                "translator-credits", _("translator-credits"),
                NULL);
}

static const GtkActionEntry _action_entries_window[] =
{
    {"FileOpen", "gtk-file",
     N_("Open _Image..."), "<control>O",
     N_("Open an Image"),
     G_CALLBACK(_action_open)},

    {"FileOpenDir", "gtk-directory",
     N_("Open _Folder..."), "<control>F",
     N_("Open a Folder"),
     G_CALLBACK(_action_open_dir)},

    {"FileOpenWith", NULL,
     N_("Open _With"), NULL,
     N_("Open the selected image with a different application"),
     NULL},

    {"FileSelectDirectory", NULL,
     N_("Select..."), "F7",
     N_("Select a directory to move files"),
     G_CALLBACK(_action_select_directory)},

    {"FileRename", NULL,
     N_("Rename"), "F2",
     N_("Rename the current file"),
     G_CALLBACK(_action_rename)},

    {"FileMove", NULL,
     N_("Move"), "F8",
     N_("Move the current file"),
     G_CALLBACK(_action_move)},

    {"FileDelete", "gtk-delete",
     N_("_Delete"), "Delete",
     N_("Delete the current file"),
     G_CALLBACK(_action_delete)},

    {"FileProperties", "gtk-properties",
     N_("_Properties..."), "<control>Return",
     N_("Show information about the current file"),
     G_CALLBACK(_action_properties)},

    {"EditPreferences", "gtk-properties",
     N_("_Preferences..."), NULL,
     N_("User preferences for Viewnior"),
     G_CALLBACK(_action_preferences)},

    {"HelpAbout", "gtk-about",
     N_("_About"), "F1",
     N_("About this application"),
     G_CALLBACK(_action_about)},
};

// clang-format off
const gchar* _ui_definition =
    "<ui>"
        "<popup name=\"PopupMenu\">"
            "<menuitem action=\"FileOpen\"/>"
            "<menuitem action=\"FileOpenDir\"/>"
            "<menu action=\"FileOpenWith\">"
                "<placeholder name=\"AppEntries\"/>"
            "</menu>"
            "<separator/>"
            "<menuitem action=\"FileRename\"/>"
            "<menuitem action=\"FileSelectDirectory\"/>"
            "<menuitem action=\"FileMove\"/>"
            "<menuitem action=\"FileDelete\"/>"
            "<separator/>"
            "<menuitem action=\"FileProperties\"/>"
            "<menuitem action=\"EditPreferences\"/>"
        "</popup>"

        "<accelerator name=\"ControlHelpAbout\" action=\"HelpAbout\"/>"

    "</ui>";
// clang-format on

static const GtkActionEntry _action_entries_static_image[] =
{
    {"ImageRotateCW", "object-rotate-right",
     N_("Rotate _Clockwise"), "<control>R",
     N_("Rotate image clockwise"),
     G_CALLBACK(_action_rotate_cw)},

    {"ImageRotateCCW", "object-rotate-left",
     N_("Rotate _Anti-clockwise"), "<control><shift>R",
     N_("Rotate image anti-clockwise"),
     G_CALLBACK(_action_rotate_ccw)},

    {"ImageFlipVertical", "object-flip-vertical",
     N_("Flip _Vertical"), NULL,
     N_("Flip image vertically"),
     G_CALLBACK(_action_flip_vertical)},

    {"ImageFlipHorizontal", "object-flip-horizontal",
     N_("Flip _Horizontal"), NULL,
     N_("Flip image horizontally"),
     G_CALLBACK(_action_flip_horizontal)},

    {"ImageCrop", NULL,
     N_("Crop..."), NULL,
     N_("Crop"),
     G_CALLBACK(_action_crop)},
};

static const GtkToggleActionEntry _toggle_entries_collection[] =
{
    {"ViewSlideshow", "gtk-network",
     N_("Sli_deshow"), "F6",
     N_("Show in slideshow mode"),
     G_CALLBACK(_action_slideshow)},
};

static const GtkActionEntry _action_entries_collection[] =
{
    {"GoPrevious", "gtk-go-back",
     N_("_Previous Image"), NULL,
     N_("Go to the previous image of the collection"),
     G_CALLBACK(_action_prev)},

    {"GoNext", "gtk-go-forward",
     N_("_Next Image"), NULL,
     N_("Go to the next image of the collection"),
     G_CALLBACK(_action_next)},

    {"GoFirst", "gtk-go-first",
     N_("_First Image"), NULL,
     N_("Go to the first image of the collection"),
     G_CALLBACK(_action_first)},

    {"GoLast", "gtk-goto-last",
     N_("_Last Image"), NULL,
     N_("Go to the last image of the collection"),
     G_CALLBACK(_action_last)},
};

static const GtkActionEntry _action_entry_save[] =
{
    {"FileSave", "gtk-save",
     N_("_Save"), "<control>S",
     N_("Save changes"),
     G_CALLBACK(_action_save_image)},
};

static const GtkActionEntry _action_entry_wallpaper[] =
{
    {"SetAsWallpaper", NULL,
     N_("Set as _Wallpaper"), NULL,
     N_("Set the selected image as the desktop background"),
     G_CALLBACK(_action_set_wallpaper)},
};

static const GtkToggleActionEntry _toggle_entries_image[] =
{
    {"ViewFullscreen", "gtk-fullscreen",
     N_("Full _Screen"), "F11",
     N_("Show in fullscreen mode"),
     G_CALLBACK(_action_fullscreen)},

    {"ViewResizeWindow", NULL,
     N_("_Adjust window size"), NULL,
     N_("Adjust window size to fit the image"),
     G_CALLBACK(_action_resize)},
};

static const GtkToggleActionEntry _toggle_entries_window[] =
{
    {"ViewScrollbar", NULL,
     N_("Scrollbar"), NULL,
     N_("Show Scrollbar"),
     G_CALLBACK(_action_scrollbar)},
};

    {"FileReload", "gtk-refresh",
     N_("_Reload"), NULL,
     N_("Reload the current file"),
     G_CALLBACK(_action_reload)},

    {"ViewZoomIn", "gtk-zoom-in",
     N_("_Zoom In"), "<control>plus",
     N_("Enlarge the image"),
     G_CALLBACK(_action_zoom_in)},

    {"ViewZoomOut", "gtk-zoom-out",
     N_("Zoom _Out"), NULL,
     N_("Shrink the image"),
     G_CALLBACK(_action_zoom_out)},

    {"ViewZoomNormal", "gtk-zoom-100",
     N_("_Normal Size"), "<control>0",
     N_("Show the image at its normal size"),
     G_CALLBACK(_action_normal_size)},

    {"ViewZoomFit", "gtk-zoom-fit",
     N_("Best _Fit"), NULL,
     N_("Fit the image to the window"),
     G_CALLBACK(_action_fit)},

    {"ControlEqual", "gtk-zoom-in",
     N_("_Zoom In"), NULL,
     N_("Shrink the image"),
     G_CALLBACK(_action_zoom_in)},

    {"ControlKpAdd", "gtk-zoom-in",
     N_("_Zoom In"), NULL,
     N_("Shrink the image"),
     G_CALLBACK(_action_zoom_in)},

    {"ControlKpSub", "gtk-zoom-out",
     N_("Zoom _Out"), NULL,
     N_("Shrink the image"),
     G_CALLBACK(_action_zoom_out)},

g_signal_connect(action,
                 "activate",
                 G_CALLBACK(_on_open_with_launch_application),
                 window);
gtk_action_group_add_action(window->actions_open_with, action);
g_object_unref(action);

gtk_ui_manager_add_ui(window->ui_manager,
                      window->open_with_menu_id,
                      "/MainMenu/File/FileOpenWith/AppEntries",
                      name,
                      name,
                      GTK_UI_MANAGER_MENUITEM,
                      FALSE);

gtk_ui_manager_add_ui(window->ui_manager,
                      window->open_with_menu_id,
                      "/MainMenu/FileOpenWith/AppEntries",
                      name,
                      name,
                      GTK_UI_MANAGER_MENUITEM,
                      FALSE);

gtk_ui_manager_add_ui(window->ui_manager,
                      window->open_with_menu_id,
                      "/PopupMenu/FileOpenWith/AppEntries",
                      name,
                      name,
                      GTK_UI_MANAGER_MENUITEM,
                      FALSE);


#endif


