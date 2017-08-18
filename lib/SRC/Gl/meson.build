ar_gsub_files = files(
    'argDraw.c',
    'argDrawImage.c',
    'argBase.c',
    'argDrawMode.c',
    'argFunction.c',
    'argWindow.c',
)

ar_gsub_lib = static_library(
    'ARgsub', ar_gsub_files,
    include_directories : ar_includes,
)

ar_gsub_lite_files = files(
    'gsub_lite.c',
    'gsub_mtx.c',
)

ar_gsub_lite_lib = static_library(
    'ARgsub_lite', ar_gsub_lite_files,
    include_directories: ar_includes,
)

gsub_test_files = files(
    'gsubTest.c'
)

gsub_test_exe = executable(
    'gsubTest',
    gsub_test_files,

    link_with : [
        ar_lib,
        ar_icp_lib,
        ar_gsub_lib,
    ],

    include_directories : ar_includes,

    dependencies: [
        gl_dep,
        glut_dep,
        math_dep,
    ]
)
