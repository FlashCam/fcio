#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <bufio.h>


double timer(double offset)
{
  static long day0 = 0;
  static long sec0 = 0;
  static long usec0 = 0;

  struct timeval tv;
  gettimeofday(&tv, NULL);
  long day = tv.tv_sec / 86400;
  long sec = tv.tv_sec % 86400;
  long usec = tv.tv_usec;

  if (day0 == 0) {
    day0 = day;
    sec0 = sec;
    usec0 = usec;
  }

  return (86400.0 * (day - day0) + (sec - sec0) + 1.0e-6 * (usec - usec0)) -
         offset;
}


int main(int argc, char *argv[])
{
  if (argc != 5 || strlen(argv[1]) != 1 || (argv[1][0] != 'w' && argv[1][0] != 'r')) {
    printf("usage: bufio_benchmark [w|r] address message_size message_count\n");
    return 1;
  }

  char mode = argv[1][0];
  char *address = argv[2];
  unsigned long message_size = atoi(argv[3]);
  unsigned long message_count = atoi(argv[4]);

  char *buf = (char *) malloc(message_size);
  if (buf == NULL) {
    fprintf(stderr, "failed to allocate buffer\n");
    return 1;
  }

  bufio_stream *s = bufio_open(address, argv[1], 10000, 4096, NULL);
  if (s == NULL) {
    fprintf(stderr, "failed to connect\n");
    return 1;
  }

  bufio_timeout(s, 1000);

  double t = timer(0);

  if (mode == 'w') {
    for (unsigned long i = 0; i < message_count; i++) {
      if (bufio_write(s, buf, message_size) != message_size) {
        fprintf(stderr, "write failed: %s\n", bufio_status_str(s));
        return 1;
      }
    }
  } else {
    for (unsigned long i = 0; i < message_count; i++) {
      if (bufio_read(s, buf, message_size) != message_size) {
        fprintf(stderr, "read failed: %s\n", bufio_status_str(s));
        return 1;
      }
    }
  }

  if (bufio_flush(s) != 0) {
    fprintf(stderr, "flush failed: %s\n", bufio_status_str(s));
    return 1;
  }

  double elapsed = timer(t);
  double throughput = (double) message_count / (double) elapsed;
  double megabits = (double) (throughput * message_size * 8) / 1000000;

  fprintf(stderr, "message size: %d [B]\n", (int) message_size);
  fprintf(stderr, "message count: %d\n", (int) message_count);
  fprintf(stderr, "mean throughput: %.3f [msg/s]\n", throughput);
  fprintf(stderr, "mean throughput: %.3f [Mb/s]\n", megabits);

  usleep(50000);

  return bufio_close(s);
}
