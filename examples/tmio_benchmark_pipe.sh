#!/bin/bash

function benchmark {
    BINARY=examples/tmio_benchmark
    OPTS="-t 5000 $1"

    $BINARY -n 3000000  -s 80 $OPTS -w - | $BINARY $OPTS -r -
    $BINARY -n 2000000 -s 800 $OPTS -w - | $BINARY $OPTS -r -
    $BINARY -n 200000 -s 8000 $OPTS -w - | $BINARY $OPTS -r -
    $BINARY -n 20000 -s 80000 $OPTS -w - | $BINARY $OPTS -r -
    $BINARY -n 2000 -s 800000 $OPTS -w - | $BINARY $OPTS -r -
}

echo "Standard stream cache to cache performance:"
benchmark
echo
echo "Standard stream memory to memory performance:"
benchmark "-m"
