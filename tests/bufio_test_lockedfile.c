#define _BSD_SOURCE

#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

#include "bufio.h"
#include "test.h"


int main(void)
{
  fprintf(stderr, "test_bufio_lockedfile: ");

  char buf[16];

  bufio_stream *so = bufio_open("lockedfile://test_bufio_lockedfile.dat", "w", 0, 8, NULL);
  assert(so != NULL);

  bufio_stream *si = bufio_open("lockedfile://test_bufio_lockedfile.dat", "r", 0, 8, NULL);
  assert(si != NULL);

  bufio_timeout(si, 100);

  // Assert no initial data
  assert(bufio_wait(si, 0) == 0 && bufio_status(si) == BUFIO_EOF);
  assert(bufio_wait(si, 100) == 0 && bufio_status(si) == BUFIO_EOF);

  // Assert no data immediately after buffered write
  assert(bufio_write(so, buf, 4) == 4);
  assert(bufio_read(si, buf, 4) == 0 && bufio_status(si) == BUFIO_EOF);
  assert(bufio_wait(si, 0) == 0 && bufio_status(si) == BUFIO_EOF);
  assert(bufio_wait(si, 100) == 0 && bufio_status(si) == BUFIO_EOF);

  // Assert data after flush
  assert(bufio_flush(so) == 0);
  assert(bufio_read(si, buf, 4) == 4);

  // Assert no data immediately after unbuffered write
  assert(bufio_write(so, buf, 16) == 16);
  FORK_CHILD
    assert(bufio_read(si, buf, 16) == 0 && bufio_status(si) == BUFIO_TIMEDOUT);
    assert(bufio_wait(si, 0) == 0 && bufio_status(si) == BUFIO_TIMEDOUT);
    assert(bufio_wait(si, 100) == 0 && bufio_status(si) == BUFIO_TIMEDOUT);
  FORK_PARENT
  FORK_JOIN

  // Assert data after flush
  assert(bufio_flush(so) == 0);
  FORK_CHILD
    assert(bufio_read(si, buf, 16) == 16);
  FORK_PARENT
  FORK_JOIN

  // Assert wait returns properly
  FORK_CHILD
    assert(bufio_wait(si, 250) == 1);
  FORK_PARENT
    usleep(100000);
    assert(bufio_write(so, buf, 1) == 1);
    assert(bufio_flush(so) == 0);
  FORK_JOIN

  // Clean up
  assert(bufio_close(si) == 0);
  assert(bufio_close(so) == 0);

  fprintf(stderr, "pass\n");
  return 0;
}
