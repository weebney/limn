project(
  'limn',
  'c',
  version: '0.1',
  default_options: [
    'warning_level=3',
    'c_std=c11',
  ],
)

cc = meson.get_compiler('c')

# Dependencies
raylib_dep = dependency('raylib')
tinyfiledialogs = subproject('tinyfiledialogs')
tinyfiledialogs_dep = tinyfiledialogs.get_variable('tinyfiledialogs_dep')

# Source files
src_files = ['limn.c']

# Compiler flags
c_args = []

# Linker flags
link_args = []

executable(
  'limn',
  src_files,
  c_args: c_args,
  link_args: link_args,
  dependencies: [
    raylib_dep,
    tinyfiledialogs_dep,
  ],
  install: true,
)
