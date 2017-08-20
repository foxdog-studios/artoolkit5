project('ARToolkit5', 'c')

c_compiler = meson.get_compiler('c')

ar_includes = include_directories('include')

gl_dep = dependency('GL')
glut_dep = dependency('freeglut')
libjpeg_dep = dependency('libjpeg')
math_dep = c_compiler.find_library('m')
threads_dep = dependency('threads')

subdir('include/AR')
subdir('lib')
subdir('examples')
