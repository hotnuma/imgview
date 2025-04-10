TEMPLATE = app
TARGET = imgview
CONFIG = c99 link_pkgconfig
DEFINES = _GNU_SOURCE
INCLUDEPATH = dlg uni
PKGCONFIG =

PKGCONFIG += shared-mime-info
PKGCONFIG += gtk+-3.0
PKGCONFIG += gdk-pixbuf-2.0
PKGCONFIG += glib-2.0
PKGCONFIG += gio-2.0
PKGCONFIG += exiv2
PKGCONFIG += tinyui

HEADERS = \
    dlg/dialog.h \
    dlg/vnr-crop.h \
    dlg/vnr-properties-dialog.h \
    dlg/xfce-filename-input.h \
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
    uni/uni-zoom.h \
    config.h.in \
    file.h \
    list.h \
    message-area.h \
    preferences.h \
    vnr-tools.h \
    window.h \

SOURCES = \
    dlg/dialog.c \
    dlg/vnr-crop.c \
    dlg/vnr-properties-dialog.c \
    dlg/xfce-filename-input.c \
    libgd/gd-helpers.c \
    libgd/gd-image.c \
    libgd/gd-resize.c \
    libgd/gd-temp.c \
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
    message-area.c \
    preferences.c \
    vnr-tools.c \
    window.c \

DISTFILES = \
    Readme.md \
    deprecations.txt \
    install.sh \
    meson.build \
    meson_post_install.py \


