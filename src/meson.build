install_headers('fcio.h')
fcio_sources =files('fcio.c', 'time_utils.c')

fcio_lib = library('fcio',
  fcio_sources,
  include_directories : fcio_inc,
  dependencies : [ tmio_dep ],
  install : true
)