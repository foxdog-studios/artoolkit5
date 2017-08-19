ar_util_files = files(
    'profile.c',
    'thread_sub.c',
)

threads_dep = dependency('threads')

ar_util_lib = library('ARUtil', ar_util_files,
    include_directories : ar_includes,
    dependencies : threads_dep,
)
