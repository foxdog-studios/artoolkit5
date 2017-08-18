aricp_files = files(
    'icpCalibStereo.c',
    'icpCore.c',
    'icpHandle.c',
    'icpPoint.c',
    'icpPointRobust.c',
    'icpStereoHandle.c',
    'icpStereoPoint.c',
    'icpStereoPointRobust.c',
    'icpUtil.c',
)

aricp_lib = static_library(
    'ARICP', aricp_files,
    include_directories : ar_includes,
    pic : true,
)
