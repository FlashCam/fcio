#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <tmio.h>
#include "timer.h"



int main(int argc, char *argv[])
{
  if (argc != 5) {
    printf("usage: tmio_benchmark_simple [w|r] address message_size message_count\n");
    return 1;
  }

  char mode = argv[1][0] == 'w' ? 'w' : 'r';
  char *address = argv[2];
  unsigned long message_size = (atoi(argv[3]) > (int) sizeof(int)) ? (unsigned long)atoi(argv[3]) : sizeof(int);
  unsigned long message_count = atoi(argv[4]);
  unsigned long payload_size = message_size > sizeof(int) ? message_size - sizeof(int) : 0;

  int tag = 1;
  char *buf = (char *) malloc(payload_size);
  if (buf == NULL) {
    fprintf(stderr, "failed to allocate buffer\n");
    return 1;
  }

  tmio_stream *s = tmio_init("tmio_benchmark_simple", 1000, 0, 0);
  if (s == NULL) {
    fprintf(stderr, "init failed\n");
    return 1;
  }

  if (mode == 'w') {
    if (tmio_create(s, address, 10000) == -1) {
      fprintf(stderr, "listen failed\n");
      return 1;
    }
  } else {
    if (tmio_open(s, address, 10000) == -1) {
      fprintf(stderr, "connect failed\n");
      return 1;
    }
  }

  double t = timer(0);

  if (mode == 'w') {
    for (unsigned long i = 0; i < message_count; i++) {
      if (tmio_write_tag(s, tag) == -1 ||
          (payload_size > 0 && tmio_write_data(s, buf, payload_size) != (long) payload_size)) {
        fprintf(stderr, "write failed: %s\n", tmio_status_str(s));
        return 1;
      }
    }
  } else {
    for (unsigned long i = 0; i < message_count; i++) {
      if (tmio_read_tag(s) != 1 ||
          (payload_size > 0 && tmio_read_data(s, buf, payload_size) != (long) payload_size)) {
        fprintf(stderr, "read failed: %s\n", tmio_status_str(s));
        return 1;
      }
    }
  }

  if (tmio_flush(s) != 0) {
    fprintf(stderr, "flush failed: %s\n", tmio_status_str(s));
    return 1;
  }

  double elapsed = timer(t);
  double throughput = (double) message_count / (double) elapsed;
  double megabits = (double) (throughput * message_size * 8) / 1000000;

  printf ("message size: %d [B]\n", (int) message_size);
  printf ("message count: %d\n", (int) message_count);
  printf ("mean throughput: %.3f [msg/s]\n", throughput);
  printf ("mean throughput: %.3f [Mb/s]\n", megabits);

  usleep(50000);

  return tmio_close(s);
}
