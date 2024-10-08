project(
  'fcio',
  'c',
  version: '1.0.0',
  license: 'MPL-2.0',
  license_files: 'LICENSE',
  default_options: [
    'warning_level=3',
    'c_std=c99',
    'buildtype=release',
    'default_library=static',
    'prefer_static=true',
  ],
)

tmio_dep = dependency('tmio', fallback: ['tmio', 'tmio_dep'])

subdir('src')
subdir('tests')
subdir('examples')

pkg_mod = import('pkgconfig')
pkg_mod.generate(fcio_lib)
