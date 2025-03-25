TEMPLATE = app
TARGET = imgview
CONFIG = c99 link_pkgconfig
DEFINES = _GNU_SOURCE
INCLUDEPATH = . src
PKGCONFIG =

PKGCONFIG += gtk+-3.0
PKGCONFIG += gdk-pixbuf-2.0
PKGCONFIG += gio-2.0
PKGCONFIG += glib-2.0
PKGCONFIG += exiv2
PKGCONFIG += tinyui
PKGCONFIG += shared-mime-info

HEADERS = \
    uni/uni-anim-view.h \
    uni/uni-cache.h \
    uni/uni-dragger.h \
    uni/uni-exiv2.hpp \
    uni/uni-image-view.h \
    uni/uni-nav.h \
    uni/uni-scroll-win.h \
    uni/uni-utils.h \
    uni/uni-zoom.h \
    vnr/vnr-crop.h \
    vnr/vnr-message-area.h \
    vnr/vnr-properties-dialog.h \
    vnr/vnr-tools.h \
    vnr/xfce-filename-input.h \
    config.h.in \
    dialog.h \
    file.h \
    list.h \
    preferences.h \
    window.h \

SOURCES = \
    uni/uni-anim-view.c \
    uni/uni-cache.c \
    uni/uni-dragger.c \
    uni/uni-exiv2.cpp \
    uni/uni-image-view.c \
    uni/uni-nav.c \
    uni/uni-scroll-win.c \
    uni/uni-utils.c \
    vnr/vnr-crop.c \
    vnr/vnr-message-area.c \
    vnr/vnr-properties-dialog.c \
    vnr/vnr-tools.c \
    vnr/xfce-filename-input.c \
    0temp.c \
    dialog.c \
    file.c \
    list.c \
    main.c \
    preferences.c \
    window.c \

DISTFILES = \
    Notes.md \
    Readme.md \
    deprecations.txt \
    install.sh \
    meson.build \
    meson_post_install.py \


