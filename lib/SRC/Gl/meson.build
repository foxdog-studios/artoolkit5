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
    pic : true,
)

argsub_lite_files = files(
    'gsub_lite.c',
    'gsub_mtx.c',
)

argsub_lite_lib = static_library(
    'ARgsub_lite', argsub_lite_files,
    include_directories: ar_includes,
    pic : true,
)
