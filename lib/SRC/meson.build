subdir('AR')
subdir('AR2')
subdir('ARICP')
subdir('ARMulti')
subdir('Gl')
subdir('Video')
subdir('VideoLinuxV4L2')

ar_video_files = video_files + video_linux_v4l2_files

ar_video_lib = static_library(
    'ARvideo',
    ar_video_files,
    include_directories : ar_includes,
)
