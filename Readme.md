<link href="style.css" rel="stylesheet"></link>

## ImgView

This is a fork of Viewnior Gtk3 :  
https://github.com/Artturin/Viewnior  

From the original project by Siyan Panayotov :  
https://github.com/hellosiyan/Viewnior  

Most of the Viewnior's sources handling image viewing are adopted from the GtkImageView library by Björn Lindqvist. The files were cleaned up and modified, so that unused functionalities were removed (GtkImageToolSelector, GtkImageToolPainter, GtkZooms). Prefixes were changed from gtk_ to uni_ for clarity.


#### Hotkeys

| Action                  | Hotkey                  |
| :---------------------- | :---------------------- |
| Open                    | Ctrl+O                  |
| Open Dir                | Ctrl+D                  |
| Save                    | Ctrl+S                  |
| Delete                  | Del                     |
| Zoom Normal             | Ctrl+N                  |
| Zoom Fit                | Ctrl+F                  |
| Properties              | Ctrl+Return             |
| Quit                    | Esc / Q                 |
| Rename                  | F2                      |
| Crop                    | F4                      |
| Reload                  | F5                      |
| Reset Directory         | F6                      |
| Copy                    | F7                      |
| Move                    | F8                      |
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


#### Install

```
sudo apt install build-essential gettext git meson \
libgtk-3-dev libexiv2-dev
```

```
git clone https://github.com/hotnuma/imgview.git
cd imgview
./install.sh
```


#### License

This program is released under the terms of the [GNU General Public License](https://opensource.org/licenses/gpl-3.0.html).

object-rotate-left.png, object-rotate-right.png are taken from Elementary icon theme by ~DanRabbit (under GPL). object-flip-horizontal.png, object-flip-vertical.png are taken from Gnome icon theme (under GPL).

<br/>


