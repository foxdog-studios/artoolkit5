ar_multi_files = files(
    'arMultiFreeConfig.c',
    'arMultiGetTransMat.c',
    'arMultiGetTransMatStereo.c',
    'arMultiReadConfigFile.c',
)

ar_multi_lib = library('ARMulti', ar_multi_files,
    link_with : [ar_lib, ar_icp_lib],
    include_directories : ar_includes,
    dependencies : math_dep,
    override_options : 'unity=on',
)
