fcio_test_unknown_tags = executable('fcio_test_unknown_tags', 'fcio_test_unknown_tags.c', dependencies : [fcio_dep])
fcio_benchmark = executable('fcio_benchmark', ['fcio_benchmark.c', 'timer.c'], dependencies: [fcio_utils_dep])
fcio_test_record_consistency = executable('fcio_test_record_consistency', 'fcio_test_record_consistency.c', dependencies : [fcio_dep])

test('fcio_test_unknown_tags', fcio_test_unknown_tags, is_parallel : true, args : ['fcio_test_unknown_tags.dat'])
test('fcio_test_record_consistency', fcio_test_record_consistency, is_parallel : true, args : ['fcio_test_record_consistency.dat'])

test('fcio_benchmark_camera_tcp_loopback', fcio_benchmark, is_parallel : false, args : ['-n','10000','-s','128','-c','1764', '-w', 'tcp://listen/3001', '-r', 'tcp://connect/3001/localhost'], suite : ['benchmark'])
test('fcio_benchmark_camera_file', fcio_benchmark, is_parallel : false, args : ['-n','10000','-s','128','-c','1764', '-w', 'file://fcio_benchmark.dat', '-r', 'file://fcio_benchmark.dat', '--no-fork'], suite : ['benchmark'])

test('fcio_benchmark_germanium_tcp_loopback', fcio_benchmark, is_parallel : false, args : ['-n','10000','-s','8192','-c','180', '-w', 'tcp://listen/3001', '-r', 'tcp://connect/3001/localhost'], suite : ['benchmark'])
test('fcio_benchmark_germanium_file', fcio_benchmark, is_parallel : false, args : ['-n','1000','-s','8192','-c','180', '-w', 'file://fcio_benchmark.dat', '-r', 'file://fcio_benchmark.dat', '--no-fork'], suite : ['benchmark'])

fcio_test_record_sizes = executable('fcio_test_record_sizes', 'fcio_test_record_sizes.c', dependencies : [fcio_utils_dep])
test('fcio_test_record_sizes', fcio_test_record_sizes, is_parallel : true, args : ['0'])
