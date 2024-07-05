#!/bin/bash

writer ()
{
  set -x
  build/examples/fcio-example-writer -v --debug ${WRITER_DEBUG} --timeout ${WRITER_TIMEOUT} --max ${WRITER_MAX} --sleep ${WRITER_SLEEP} ${WRITER_ENDPOINT}
  # set +x
}

reader()
{
  set -x
  build/examples/fcio-example-reader -v --debug ${READER_DEBUG} --timeout ${READER_TIMEOUT} --max ${READER_MAX} --stop-tag ${READER_STOPTAG} ${READER_ENDPOINT}
  # set +x
}

run()
{
  echo ""
  (trap 'kill 0' SIGINT; (sleep ${WRITE_DELAY} && writer) & (sleep ${READ_DELAY} && reader) & wait)
  echo ""
}

run_twice()
{
  echo ""
  (trap 'kill 0' SIGINT; writer & (sleep ${READ_DELAY} && reader) & (sleep 2 && writer) & wait)
  echo ""
}

# this case needs other syntax as stdout and stdin are connected directly
# hence unnamed pipe
run_piped()
{
  echo ""
  (trap 'kill 0' SIGINT; (writer | reader) & wait)
  echo ""
}
# test file
# test tcp listen -> connect
# test tcp connect -> listen

# set fcio library debug level
# 0  = logging off
# 1  = errors on
# 2  = warning on
# 3  = info on
# >3 = debugging
WRITER_DEBUG=0
READER_DEBUG=2

# timeout in ms, or -1 for indefinitely
# timeout applies to connection opening step as well as internal polling
WRITER_TIMEOUT=0
READER_TIMEOUT=0

# tags which indicate that no known message was contained
# their exact meaning changes depending on how the connection was made
# -1: Error
# -0: EOF/ Timeout
READER_STOPTAG=-1

# The writer sends all known message types per loop
# we send 2 loops to test different timeout connections between each loop
WRITER_MAX=2
# we read one more to demonstrate what tag is returned, but there is no point
# in running indefinitely if we don't find the right one
READER_MAX=3

# the time to wait between each block of messages (loop)
# number is in seconds
WRITER_SLEEP=0

READ_DELAY=0.5

# # test unnamed pipe
# READER_ENDPOINT=-
# WRITER_ENDPOINT=-
# READER_STOPTAG=0
# run_piped

# READER_STOPTAG=-1
# run_piped

# # tcp connection connect -> listen
# WRITER_SLEEP=0
# WRITER_ENDPOINT=tcp://connect/4000
# READER_ENDPOINT=tcp://listen/4000

# READER_TIMEOUT=-1
# WRITER_TIMEOUT=-1

# READER_STOPTAG=0
# run

# READER_STOPTAG=-1
# run



# file io:
rm test.dat 2>/dev/null

WRITER_SLEEP=0

WRITER_TIMEOUT=-1
READER_TIMEOUT=-1
WRITER_MAX=2
READER_MAX=4

READER_STOPTAG=-2

# file: unlocked
# WRITER_ENDPOINT=test.dat
# READER_ENDPOINT=test.dat
# run

# file: locked
# WRITER_ENDPOINT=lockedfile://test.dat
# READER_ENDPOINT=lockedfile://test.dat

# tcp:
 WRITE_DELAY=1
 READ_DELAY=0
 WRITER_ENDPOINT=tcp://listen/4000
 READER_ENDPOINT=tcp://connect/4000
 run

# pipe:
# WRITER_ENDPOINT=-
# READER_ENDPOINT=-
# run_piped

# fifo:
# WRITE_DELAY=0.5
# READ_DELAY=0
# mkfifo test.dat
# WRITER_ENDPOINT=test.dat
# READER_ENDPOINT=test.dat
# run

# READER_TIMEOUT=-1
# WRITER_TIMEOUT=-1

# READER_STOPTAG=0
# run

# READER_STOPTAG=-1
# run

##
# - files:
#   - does not wait until timeout before sending tag=0 but always returns immediately
#   - does not check for file open count but only end of file
#   - if no locked files are used the reader might read not-yet fully written bytes and return StreamError, which requires use of lockedfile.
#   -> to be consistent with tcp, should check open file count, and return EOF/Timeout on file count == 1 or wait for timeout.
# - tcp:
#   - returns on timeout or on eof, eof being tcp close, no use in continue to keep polling
#   - cannot connect twice to an alreay closed socket
# - pipe:
#   - pipe closes when one of the attached process closes
#   - the return is always StreamError
#   - timeout has no effect (how would that work?)

## Open questions:
# - what is the difference between timeout=0/-1 for files and pipes?
# - does udp actually work?

# wishlist
# -1 should indicate an actual error
# 0 should return EOF/EOS after timeout or on closed file/stream/pipe


#               file/lockedfile     tcp        pipe           fifo
# error tag=-1                                 on close       on close
# eof   tag=0   on eof              on eof

#               file/lockedfile     tcp        pipe           fifo
# on close                                     -1             -1
# on eof         0                   0

#               file/lockedfile     tcp        pipe           fifo
# on close                                     -1             -1
# on sync       -1                  -1         -1             -1
# on eof         0                   0
