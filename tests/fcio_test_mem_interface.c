#ifdef __linux__
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#else
#undef _POSIX_C_SOURCE
#endif

#include <bufio.h>
#include <errno.h>
#include <fcio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tmio.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <assert.h>
#include <time.h>

static inline int update_mem_field(FCIOStream stream, char *mem_addr,
                                   size_t mem_size) {
  return bufio_set_mem_field(tmio_stream_handle(stream), mem_addr, mem_size);
}

static int simulate_data_stream(FCIOData *input, size_t max_records) {
  static size_t record_counter = 0;
  static int event_counter = 0;

  if (record_counter == max_records) {
    return 0;
  }

  if (record_counter == 0) {
    srand(0);

    input->config.telid = 0;
    input->config.adcs = 288;
    input->config.triggers = 16;
    input->config.eventsamples = 2048;
    input->config.adcbits = 12;
    input->config.sumlength = 128;
    input->config.blprecision = 1;
    input->config.mastercards = 1;
    input->config.triggercards = 2;
    input->config.adccards = 12;
    input->config.gps = 0;
    for (int i = 0; i < input->config.adcs; i++) {
      int card = 1 + (i / 24);
      input->config.tracemap[i] = ((0x10 * card) << 16) + (i % 24);
    }
    record_counter++;
    return FCIOConfig;

  } else {
    input->event.type = 0;
    input->event.pulser = 0;
    input->event.timestamp_size = 5;
    input->event.timeoffset_size = 5;
    input->event.deadregion_size = 5;

    input->event.timestamp[0] = event_counter++;
    input->event.timestamp[1] = event_counter / 10;
    input->event.timestamp[2] = (249999999 / 10) * (input->event.timestamp[1] % 10) + 100;
    input->event.timestamp[3] = 249999999;

    for (int trace_idx = 0; trace_idx < input->config.adcs; trace_idx++) {
      for (int sample_idx = 0; sample_idx < input->config.eventsamples; sample_idx++) {
        input->event.traces[trace_idx * (input->config.eventsamples + 2) + sample_idx] = rand();
      }
    }

    record_counter++;
    return FCIOEvent;
  }
  return FCIOStatus;
}

#define NRECORDS 3

int main(void) {

  const size_t buffer_size = sizeof(FCIOData);
  char *buffer = malloc(buffer_size);

  char* mem_string;
  size_t mem_string_size = asprintf(&mem_string, "mem://%p/%zu", (void *)buffer, buffer_size);

  fprintf(stderr, "opening %s len %zu\n", mem_string, mem_string_size);

  FCIOStream writer = FCIOConnect(mem_string, 'w', 0, 0);
  FCIOData* reader = FCIOOpen(mem_string, 0, 0);

  FCIOData* input = malloc(sizeof(FCIOData));
  memcpy(input, reader, sizeof(FCIOData));

  int tag;
  while (( tag = simulate_data_stream(input, NRECORDS)) && tag > 0) {

    update_mem_field(writer, buffer, buffer_size);
    update_mem_field(FCIOStreamHandle(reader), buffer, buffer_size);

    FCIOPutRecord(writer, input, tag);
    int local_tag = FCIOGetRecord(reader);

    assert(tag == local_tag);
    assert(memcmp(input, reader, sizeof(FCIOData)));
  }
  free(mem_string);
  free(input);
  FCIOClose(reader);
  FCIODisconnect(writer);

  return 0;
}
