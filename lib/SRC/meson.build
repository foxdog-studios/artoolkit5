subdir('AR')
subdir('Util')

# Requires ar_lib
subdir('ARICP')

# Requires ar_lib, ar_icp_lib
subdir('AR2')
subdir('ARMulti')
subdir('Gl')

# -- Video ---------------------------------------------------------------------

subdir('Video')
subdir('VideoLinuxV4L2')

ar_video_files = video_files + video_linux_v4l2_files

ar_video_lib = library('ARvideo', ar_video_files,
    link_with : ar_lib,
    c_args: video_c_args,
    include_directories : ar_includes,
    dependencies : [libjpeg_dep, math_dep],
)
