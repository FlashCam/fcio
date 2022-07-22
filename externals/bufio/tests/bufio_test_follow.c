#include <assert.h>
#include <stdio.h>

#include "bufio.h"


int main(void)
{
  bufio_stream *output = bufio_open("bufio_test_follow.dat", "w", 0, 0, "bufio_test_follow");
  assert(output != NULL);

  bufio_stream *input = bufio_open("bufio_test_follow.dat", "r", 0, 0, "bufio_test_follow");
  assert(input != NULL);

  char buf[255];

  // Read/wait should fail
  assert(bufio_read(input, buf, 1) == 0);
  assert(bufio_status(input) == BUFIO_EOF);
  bufio_clear_status(input);
  assert(bufio_wait(input, 100) == 0);
  assert(bufio_status(input) == BUFIO_EOF);

  // Write 1 byte
  assert(bufio_write(output, buf, 1) == 1);
  assert(bufio_flush(output) == 0);

  // Read one byte
  bufio_clear_status(input);
  assert(bufio_wait(input, 100) == 1);
  assert(bufio_status(input) == 0);
  assert(bufio_read(input, buf, 1) == 1);
  assert(bufio_status(input) == 0);

  // Read/wait should fail
  assert(bufio_wait(input, 100) == 0);
  assert(bufio_status(input) == BUFIO_EOF);
  bufio_clear_status(input);
  assert(bufio_read(input, buf, 1) == 0);
  assert(bufio_status(input) == BUFIO_EOF);

  // Clean up
  assert(bufio_close(output) == 0);
  assert(bufio_close(input) == 0);

  return 0;
}
