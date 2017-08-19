simple_lite_files = files(
    'simpleLite.c',
)

simple_lite_exe = executable(
    'simpleLite',
    simple_lite_files,

    link_with: [
        ar_gsub_lite_lib,
        ar_icp_lib,
        ar_lib,
        ar_video_lib,
    ],

    include_directories : ar_includes,

    dependencies: [
        gl_dep,
        glut_dep,
        libjpeg_dep,
        math_dep,
    ]
)
