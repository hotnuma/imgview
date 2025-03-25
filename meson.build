project(
    'imgview',
    ['c', 'cpp'],
    version: '1.0',
    license: 'GPL3+',
)

add_global_arguments(
    #'-Wno-deprecated-declarations',
    language: 'c')

datadir = get_option('datadir')
cc = meson.get_compiler('c')
gnome = import('gnome')
i18n = import('i18n')

glib_ver = '>= 2.32'

app_deps = [
    dependency('gtk+-3.0'),
    dependency('glib-2.0', version: glib_ver),
    dependency('gio-2.0', version: glib_ver),
    dependency('shared-mime-info', version: '>= 0.20'),
    dependency('gdk-pixbuf-2.0', version: '>= 0.21'),
    dependency('exiv2', version: '>= 0.21'),
    dependency('tinyui'),
]

config = configuration_data()
config.set_quoted('VERSION', meson.project_version())
config.set_quoted('PACKAGE', 'imgview')
config.set_quoted('PACKAGE_VERSION', meson.project_version())
config.set_quoted('PACKAGE_STRING', 'imgview ' + meson.project_version())
config.set_quoted('GETTEXT_PACKAGE', 'imgview')
config.set_quoted(
    'PACKAGE_DATA_DIR',
    join_paths(get_option('prefix'), get_option('datadir'))
)
config.set_quoted(
    'PACKAGE_LOCALE_DIR',
    join_paths(get_option('prefix'), get_option('datadir'), 'locale')
)

configure_file(
    input: 'config.h.in',
    output: 'config.h',
    configuration: config
)

app_includes = [
    'dlg',
    'uni',
    'vnr',
]

subdir('po')
subdir('data')

app_sources = [
    'dlg/dialog.c',
    'dlg/vnr-crop.c',
    'dlg/vnr-properties-dialog.c',
    'dlg/xfce-filename-input.c',
    'uni/uni-anim-view.c',
    'uni/uni-cache.c',
    'uni/uni-dragger.c',
    'uni/uni-exiv2.cpp',
    'uni/uni-image-view.c',
    'uni/uni-nav.c',
    'uni/uni-scroll-win.c',
    'uni/uni-utils.c',
    'vnr/vnr-message-area.c',
    'vnr/vnr-tools.c',
    'file.c',
    'list.c',
    'main.c',
    'preferences.c',
    'window.c',
]

app_sources += gnome.genmarshal(
    'uni-marshal',
    sources: 'uni/uni-marshal.list',
    prefix: 'uni_marshal',
    internal: true
)

executable(
    meson.project_name(),
    include_directories: app_includes,
    sources: app_sources,
    dependencies: app_deps,
    install: true
)

meson.add_install_script('meson_post_install.py')


