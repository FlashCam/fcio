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

bufio_dep = dependency('bufio', fallback : ['bufio', 'bufio_dep'])
tmio_dep = dependency('tmio', fallback : ['tmio', 'tmio_dep'])

fcio_inc= include_directories('src')

subdir('src')

fcio_dep = declare_dependency(include_directories : fcio_inc, link_with : fcio_lib, sources : fcio_sources)

pkg_mod = import('pkgconfig')
pkg_mod.generate(fcio_lib)
