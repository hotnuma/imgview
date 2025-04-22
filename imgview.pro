TEMPLATE = app
TARGET = imgview
CONFIG = c99 link_pkgconfig
DEFINES = _GNU_SOURCE
INCLUDEPATH = dialog libgd uni
PKGCONFIG =

PKGCONFIG += shared-mime-info
PKGCONFIG += gtk+-3.0
PKGCONFIG += gdk-pixbuf-2.0
PKGCONFIG += glib-2.0
PKGCONFIG += gio-2.0
PKGCONFIG += exiv2
PKGCONFIG += tinyui

HEADERS = \
    dialog/dlg-file-rename.h \
    dialog/message-area.h \
    dialog/preferences.h \
    dialog/vnr-crop.h \
    dialog/vnr-properties.h \
    dialog/vnr-resize.h \
    dialog/xfce-filename-input.h \
    libgd/gd-helpers.h \
    libgd/gd-image.h \
    libgd/gd-resize.h \
    uni/uni-anim-view.h \
    uni/uni-cache.h \
    uni/uni-dragger.h \
    uni/uni-exiv2.hpp \
    uni/uni-image-view.h \
    uni/uni-scroll-win.h \
    uni/uni-utils.h \
    config.h.in \
    file.h \
    list.h \
    vnr-tools.h \
    window.h \

SOURCES = \
    dialog/dlg-file-rename.c \
    dialog/message-area.c \
    dialog/preferences.c \
    dialog/vnr-crop.c \
    dialog/vnr-properties.c \
    dialog/vnr-resize.c \
    dialog/xfce-filename-input.c \
    libgd/gd-helpers.c \
    libgd/gd-image.c \
    libgd/gd-resize.c \
    uni/uni-anim-view.c \
    uni/uni-cache.c \
    uni/uni-dragger.c \
    uni/uni-exiv2.cpp \
    uni/uni-image-view.c \
    uni/uni-scroll-win.c \
    uni/uni-utils.c \
    0temp.c \
    file.c \
    list.c \
    main.c \
    vnr-tools.c \
    window.c \

DISTFILES = \
    Readme.md \
    deprecations.txt \
    install.sh \
    meson.build \
    meson_post_install.py \


