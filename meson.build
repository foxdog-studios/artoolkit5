project('ARToolkit5', 'c')

c_compiler = meson.get_compiler('c')

ar_includes = include_directories('include')

gl_dep = dependency('GL')
glut_dep = dependency('freeglut')
math_dep = c_compiler.find_library('m')

subdir('include/AR')
subdir('lib')
subdir('examples')
