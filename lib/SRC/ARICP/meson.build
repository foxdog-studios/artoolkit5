ar_icp_files = files(
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

ar_icp_lib = library(
    'ARICP',
    ar_icp_files,
    link_with : ar_lib,
    include_directories : ar_includes,
    dependencies : math_dep,
)
