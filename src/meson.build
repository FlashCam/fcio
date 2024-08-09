fcio_inc = include_directories('.')

install_headers('fcio.h')
fcio_sources = files('fcio.c', 'time_utils.c')
fcio_lib = library('fcio',
  fcio_sources,
  include_directories : fcio_inc,
  dependencies : [ tmio_dep ],
  install : true
)
fcio_dep = declare_dependency(include_directories : fcio_inc, link_with : fcio_lib, sources : fcio_sources, dependencies : [tmio_dep])

install_headers('fcio_utils.h')
fcio_utils_sources = files('fcio_utils.c')
fcio_utils_lib = library('fcio_utils',
  fcio_utils_sources,
  include_directories : fcio_inc,
  dependencies : [ fcio_dep ],
  install : true
)
fcio_utils_dep = declare_dependency(include_directories : fcio_inc, link_with : fcio_utils_lib, sources : fcio_utils_sources, dependencies : [fcio_dep])
