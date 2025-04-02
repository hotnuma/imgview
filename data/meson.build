subdir('icons')

po_dir = join_paths(meson.project_source_root(), 'po')

install_data(
    'vnr-preferences-dialog.ui',
    install_dir: join_paths(datadir, 'imgview')
)

install_data(
    'pixmaps/object-rotate-right.png',
    'pixmaps/object-rotate-left.png',
    'pixmaps/object-flip-horizontal.png',
    'pixmaps/object-flip-vertical.png',
    install_dir: join_paths(datadir, 'imgview', 'pixmaps')
)

install_data(
    'imgview.metainfo.xml',
    install_dir: join_paths(datadir, 'metainfo')
)

desktop = i18n.merge_file(
    input: 'imgview.desktop.in',
    output: 'imgview.desktop',
    type: 'desktop',
    po_dir: po_dir,
    install: true,
    install_dir: join_paths(datadir, 'applications')
)

desktop_file_validate = find_program('desktop-file-validate', required: false)
if desktop_file_validate.found()
    test(
        'validate-desktop',
        desktop_file_validate,
        args: [
            desktop.full_path()
        ]
    )
endif


