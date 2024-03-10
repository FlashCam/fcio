project('fcio', 'c',
  version:'1.0.0',
  license:'MPL-2.0',
  license_files: 'LICENSE',
   default_options : [
    'warning_level=3',
    'c_std=c99',
    'buildtype=release',
    'default_library=both',
    'prefer_static=true',
   ]
)

tmio_dep = dependency('tmio', fallback : ['tmio', 'tmio_dep'])

fcio_inc= include_directories('src')

subdir('src')

fcio_dep = declare_dependency(include_directories : fcio_inc, link_with : fcio_lib, sources : fcio_sources, dependencies : [tmio_dep])

pkg_mod = import('pkgconfig')
pkg_mod.generate(fcio_lib)
