ar2_files = files(
    'coord.c',
    'featureMap.c',
    'featureSet.c',
    'handle.c',
    'imageSet.c',
    'jpeg.c',
    'marker.c',
    'matching.c',
    'matching2.c',
    'searchPoint.c',
    'selectTemplate.c',
    'surface.c',
    'template.c',
    'tracking.c',
    'tracking2d.c',
    'util.c',
)

ar2_lib = library('AR2', ar2_files,
    link_with : [ar_lib, ar_icp_lib, ar_util_lib],
    include_directories : ar_includes,
    dependencies : [libjpeg_dep, math_dep],
)
