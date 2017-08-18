simple_test_files = files(
    'simpleTest.c',
)

simple_test_exe = executable(
    'simpleTest',
    simple_test_files,

    link_with : [
        ar_lib,
        argsub_lib,
        aricp_lib,
        ar_video_lib,
    ],

    include_directories : ar_includes,

    dependencies: [
        gl_dep,
        glut_dep,
        math_dep,
        dependency('libjpeg'),
    ],
)
