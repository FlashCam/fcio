project('fcio', 'c', version:'1.0.0',
   license:'MPL-2.0',
   default_options : [
    'warning_level=3',
    'c_std=c99',
    'buildtype=release',
    'default_library=static',
    'prefer_static=true',
   ]
)

bufio_dep = dependency('bufio', fallback : ['bufio', 'bufio_dep'])
tmio_dep = dependency('tmio', fallback : ['tmio', 'tmio_dep'])

fcio_inc = include_directories('src')

subdir('src')

fcio_dep = declare_dependency(include_directories : fcio_inc, link_with : fcio_lib)

pkg_mod = import('pkgconfig')
pkg_mod.generate(fcio_lib,
    version : '1.0',
    name : 'libfcio',
    filebase : 'fcio',
    description : 'The io library for all fc250b generated data files.'
)
