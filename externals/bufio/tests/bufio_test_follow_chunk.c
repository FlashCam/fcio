#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/time.h>
#include <unistd.h>

#include "bufio.h"


#define BUFSIZE (4096)
char buf[BUFSIZE];

int usage(void)
{
  fprintf(stderr,
          "bufio_test_follow_chunk -- usage: bufio_test_follow_chunk [r|w]\n");
  return 1;
}


int reader(void)
{
  bufio_stream *s = bufio_open("lockedfile://bufio_test_follow_chunk.dat", "r", 0, 0,
                               "bufio_test_follow_chunk");
  if (s == NULL) {
    fprintf(stderr, "failed to open file\n");
    return 1;
  }

  while (bufio_wait(s, 6000) == 1) {  // use large timeout (>3 s) for network filesystems
    int nbytes = bufio_read(s, buf, BUFSIZE / 2 + BUFSIZE / 4);

    fprintf(stderr, "found new data: %d bytes\n", nbytes);
  }

  bufio_close(s);
  return 0;
}


int writer(void)
{
  bufio_stream *s = bufio_open("lockedfile://bufio_test_follow_chunk.dat", "w", 0, BUFSIZE / 4,
                               "bufio_test_follow_chunk");
  if (s == NULL) {
    fprintf(stderr, "failed to open file\n");
    return 1;
  }

  bufio_timeout(s, 1000);  // timeout for acquisition of lock
  while (usleep(100000) == 0) {
    int nbytes = bufio_write(s, buf, BUFSIZE / 2);
    fsync(bufio_fileno(s));
    usleep(5000000);
    nbytes += bufio_write(s, buf, BUFSIZE / 4);
    bufio_sync(s);  // use sync instead of flush for network filesystems

    fprintf(stderr, "wrote new data: %d bytes\n", nbytes);
  }

  bufio_close(s);
  return 0;
}


int main(int argc, char **argv)
{
  if (argc != 2)
    return usage();

  switch (argv[1][0]) {
    case 'r':
      return reader();

    case 'w':
      return writer();

    default:
      return usage();
  }
}
