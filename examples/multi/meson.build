multi_files = files(
    'multiTest.c',
)

multi_exe = executable(
    'multiTest',
    multi_files,

    link_with : [
        ar_gsub_lib,
        ar_icp_lib,
        ar_lib,
        ar_multi_lib,
        ar_video_lib,
    ],

    include_directories : ar_includes,

    dependencies : [
        gl_dep,
        glut_dep,
        libjpeg_dep,
        math_dep,
    ]
)
