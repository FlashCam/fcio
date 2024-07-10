executable('fcio-print-records', 'fcio_print_records.c', dependencies : [ fcio_dep ], install : false)

executable('fcio-strip-traces', 'fcio_strip_traces.c', dependencies : [ fcio_dep ], install : false)

executable('fcio-example-reader', 'fcio_example_reader.c', dependencies : [ fcio_dep ], install : false)

executable('fcio-example-writer', 'fcio_example_writer.c', dependencies : [ fcio_dep ], install : false)

fcio_test_direct_buffer_access = executable('fcio_test_direct_buffer_access', 'fcio_test_direct_buffer_access.c', dependencies : [fcio_dep], install : false)

test('fcio_test_direct_buffer_access', fcio_test_direct_buffer_access, is_parallel : false)
