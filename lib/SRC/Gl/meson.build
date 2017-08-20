ar_gsub_files = files(
    'argDraw.c',
    'argDrawImage.c',
    'argBase.c',
    'argDrawMode.c',
    'argFunction.c',
    'argWindow.c',
)

ar_gsub_lib = library('ARgsub', ar_gsub_files,
    link_with : ar_lib,
    include_directories : ar_includes,
    dependencies : [gl_dep, glut_dep, math_dep],
    override_options : 'unity=on',
)

ar_gsub_lite_files = files(
    'gsub_lite.c',
    'gsub_mtx.c',
)

ar_gsub_lite_lib = library('ARgsub_lite', ar_gsub_lite_files,
    link_with: [ar_lib],
    include_directories: ar_includes,
    dependencies : [gl_dep, math_dep],
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
