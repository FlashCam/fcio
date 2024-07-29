fcio_test_unknown_tags = executable('fcio_test_unknown_tags', 'fcio_test_unknown_tags.c', dependencies : [fcio_dep], install : false)
fcio_test_mem_interface = executable('fcio_test_mem_interface', 'fcio_test_mem_interface.c', dependencies : [fcio_dep], install : false)

test('fcio_test_unknown_tags', fcio_test_unknown_tags, is_parallel : true, args : ['fcio_test_unknown_tags.dat'])
test('fcio_test_mem_interface', fcio_test_mem_interface, is_parallel : true)
