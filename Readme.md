<link href="style.css" rel="stylesheet"></link>

## ImgView - Fast and elegant image viewer

This is a fork of Viewnior Gtk3 :  
https://github.com/Artturin/Viewnior  

From the original project by Siyan Panayotov :  
https://github.com/hellosiyan/Viewnior  

Viewnior is an image viewer, created to be simple, fast and elegant. Its minimalistic interface provides more screenspace for your images. Among its features are:

  * Rotate, flip, crop, save, delete images
  * Copy, move, rename
  * Fullscreen & Slideshow
  * Animation support
  * Browse only selected images
  * Navigation window
  * Set image as wallpaper (Cinnamon, FluxBox, Gnome 2, Gnome 3, hsetroot, LXDE, MATE, Nitrogen, Puppylinux, XFCE)
  * EXIF and IPTC metadata
  * Simple interface
  * Configurable mouse actions

Most of the Viewnior's sources handling image viewing are adopted from the GtkImageView library by Björn Lindqvist. The files were cleaned up and modified, so that unused functionalities were removed (GtkImageToolSelector, GtkImageToolPainter, GtkZooms). Prefixes were changed from gtk_ to uni_ for clarity.


#### Hotkeys

| Action                  | Hotkey                  |
| :---------------------- | :---------------------- |
| Open                    | Ctrl+O                  |
| Open Dir                | Ctrl+D                  |
| Properties              | Ctrl+Return             |
| Quit                    | Esc / Q                 |
| Rename                  | F2                      |
| Crop                    | F4                      |
| Reload                  | F5                      |
| Reset Directory         | F6                      |
| Copy                    | F7                      |
| Move                    | F8                      |
| Delete                  | Del                     |
| Set Wallpaper           | F9                      |
| Slideshow               | F10                     |
| Fullscreen              | F11                     |
| Prev Image              | Left / Back             |
| Next Image              | Right / Space           |
| First Image             | Home                    |
| Last Image              | End                     |
| Flip Horizontal         | Ctrl+Up / H             |
| Flip Vertical           | Ctrl+Down / V           |
| Rotate Left             | Ctrl+Left               |
| Rotate Right            | Ctrl+Right              |
| Zoom Normal             | Ctrl+N                  |
| Zoom Fit                | Ctrl+F                  |


#### Installation

This package requires the following modules to be installed:  
GTK+, glib, gio, exiv2, shared-mime-info.

```
sudo apt install build-essential git meson ninja-build \
shared-mime-info libgtk-3-dev libgdk-pixbuf-2.0-dev libexiv2-dev
```

```
git clone https://github.com/hotnuma/imgview.git
cd imgview
./install.sh
```


#### License

This program is released under the terms of the [GNU General Public License](https://opensource.org/licenses/gpl-3.0.html).

object-rotate-left.png, object-rotate-right.png are taken from Elementary icon theme by ~DanRabbit (under GPL). object-flip-horizontal.png, object-flip-vertical.png are taken from Gnome icon theme (under GPL).

*Last Edited - 17 February 2025*

<br/>
<br/>


