#!/bin/bash

function benchmark {
    BINARY=examples/tmio_benchmark
    OPTS="-t 5000 $1"

    $BINARY -n 3000000  -s 80 $OPTS/tmio_disk_benchmark_80.dat
    $BINARY -n 2000000 -s 800 $OPTS/tmio_disk_benchmark_800.dat
    $BINARY -n 200000 -s 8000 $OPTS/tmio_disk_benchmark_8000.dat
    $BINARY -n 20000 -s 80000 $OPTS/tmio_disk_benchmark_80000.dat
    $BINARY -n 2000 -s 800000 $OPTS/tmio_disk_benchmark_800000.dat
}

PATH="$1"
if [ ! -d "$PATH" ]; then
    echo "Please specify target directoy."
    exit
fi

echo "Disk read to cache performance:"
benchmark "-r $PATH"
