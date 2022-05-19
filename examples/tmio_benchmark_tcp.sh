#!/bin/bash

function benchmark {
    BINARY=examples/tmio_benchmark
    OPTS="-t 5000 $1"
    READ_DELAY=1.0

    [[ $OPTS == *"-r"* ]] && sleep $READ_DELAY
    $BINARY -n 3000000 -s 80 $OPTS

    [[ $OPTS == *"-r"* ]] && sleep $READ_DELAY
    $BINARY -n 2000000 -s 800 $OPTS

    [[ $OPTS == *"-r"* ]] && sleep $READ_DELAY
    $BINARY -n 200000 -s 8000 $OPTS

    [[ $OPTS == *"-r"* ]] && sleep $READ_DELAY
    $BINARY -n 20000 -s 80000 $OPTS

    [[ $OPTS == *"-r"* ]] && sleep $READ_DELAY
    $BINARY -n 2000 -s 800000 $OPTS
}

TCP_PORT=3001
TCP_HOST="$1"

echo "TCP cache to cache performance:"
[ "$TCP_HOST" == "" ] && benchmark "-w tcp://listen/$TCP_PORT"
[ "$TCP_HOST" != "" ] && benchmark "-r tcp://connect/$TCP_PORT/$TCP_HOST"
echo
echo "TCP memory to memory performance:"
[ "$TCP_HOST" == "" ] && benchmark "-m -w tcp://listen/$TCP_PORT"
[ "$TCP_HOST" != "" ] && benchmark "-m -r tcp://connect/$TCP_PORT/$TCP_HOST"
