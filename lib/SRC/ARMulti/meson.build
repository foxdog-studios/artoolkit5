ar_multi_files = files(
    './arMultiReadConfigFile.c',
    './arMultiFreeConfig.c',
    './arMultiGetTransMat.c',
    './arMultiGetTransMatStereo.c',
)

ar_multi_lib = static_library(
    'ARMulti',
    ar_multi_files,
    include_directories : ar_includes,
)
