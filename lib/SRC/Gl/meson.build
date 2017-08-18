argsub_files = files(
    'argDraw.c',
    'argDrawImage.c',
    'argBase.c',
    'argDrawMode.c',
    'argFunction.c',
    'argWindow.c',
)

argsub_lib = static_library(
    'ARgsub', argsub_files,
    include_directories : ar_includes,
)

argsub_lite_files = files(
    'gsub_lite.c',
    'gsub_mtx.c',
)

argsub_lite_lib = static_library(
    'ARgsub_lite', argsub_lite_files,
    include_directories: ar_includes,
)

gsub_test_files = files(
    'gsubTest.c'
)

gsub_test_exe = executable(
    'gsubTest', gsub_test_files,
    link_with : [ar_lib, aricp_lib, argsub_lib],
    include_directories : ar_includes,
    dependencies: [
        gl_dep,
        glut_dep,
        math_dep,
    ]
)
