/*
  tmio_benchmark

  General purpose benchmarking utility for bi- and unidirectional tmio
  streams. Transfer structure tries to imitate a camera server, sending
  a run header followed by many events (20 Byte header, configurable trace
  data) and a final end-of-run.
*/

#define _DEFAULT_SOURCE

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <tmio.h>
#include "timer.h"

// Define tags and template data structures
#define FCStartRunTag 100
#define FCEventTag    200
#define FCEndRunTag   300
#define FCTrigger     400

typedef struct {
  int runid;
  int startime;
  int stoptime;
  int pixels;
  int samples;
  int tracedatasize;
} FCRunHeader;

typedef struct {
  int evtno;
  int pps;
  int ticks;
  int maxticks;
  int dummy;
} FCEventHeader;

typedef unsigned short FCTraceData;


/*
  Allocates a chunk of memory for trace data (a multiple of
  tracesize). When use_large_buffer is true, the minimum buffer
  size is 256 MiB to enforce frequent cache misses. Returns
  the size of the buffer in byte.
*/
#define LARGE_TRACE_POOL_SIZE (256 * 1024 * 1024)
char *tracepool = NULL;  // allocated by init_trace_pool
long tracepoolsize = 0;
long tracesize = 8000;  // set by command-line

long init_trace_pool(bool use_large_buffer)
{
  if (tracepool != NULL) {
    free(tracepool);
    tracepoolsize = 0;
  }

  if (use_large_buffer && tracesize < LARGE_TRACE_POOL_SIZE)
    tracepoolsize = tracesize * (long) ceil((double) LARGE_TRACE_POOL_SIZE / tracesize);
  else
    tracepoolsize = tracesize;

  tracepool = (char *) calloc(1, tracepoolsize);
  if (tracepool == NULL) {
    fprintf(stderr, "Failed to allocate trace pool.");
    exit(1);
  }

  return tracepoolsize;
}


/*
  Returns the location of the next chunk of memory from the trace pool.
*/
FCTraceData * get_next_trace_data(void)
{
  static int evtno = 0;

  assert(tracepool != NULL && tracepoolsize > 0);
  assert(tracesize > 0 && tracesize <= tracepoolsize);

  if (tracepoolsize < LARGE_TRACE_POOL_SIZE || tracepoolsize == tracesize)
    return (FCTraceData *) tracepool;  // alway use first chunk of memory
  else
    return (FCTraceData *) &tracepool[(evtno++ * tracesize) % (tracepoolsize - tracesize)];
}


/*
  Fills the event header and sets data to the location of the next
  chunk of memory from the trace pool.
*/
int get_dummy_event(FCEventHeader * header, FCTraceData **data)
{
  static int evtno = 0;

  header->evtno = evtno++;
  *data = get_next_trace_data();

  return *data == NULL;
}


int main_writer(char *peer,
                int events,
                int flush_interval,
                int bufsize,
                int timeout,
                int connect_timeout,
                int verbosity)
{
  FCRunHeader   runheader = {1, 0, 0, 0, 0, tracesize};
  FCEventHeader evtheader = {1, 1, 0, 250000000, 0};
  FCTraceData   *traces = NULL;

  if (verbosity)
    fprintf(stderr, "Camera writer started\n");

  // Listen for connections or open output file
  tmio_stream *stream = tmio_init("FCcamera V1.0", timeout, bufsize, verbosity);
  if (tmio_create(stream, peer, connect_timeout) == -1)
    exit(1);

  if (verbosity)
    fprintf(stderr, "Starting transfer...\n");

  // Write run header
  tmio_write_tag(stream, FCStartRunTag);
  tmio_write_data(stream, &runheader, sizeof(runheader));

  // Init statistics
  double t = timer(0);
  double tcpu = cputime(0);
  double sum = 0;

  // Write loop
  int i = 0;
  while (tmio_status(stream) == 0 &&
         i++ < events &&
         get_dummy_event(&evtheader, &traces) == 0) {
    tmio_write_tag(stream, FCEventTag);
    sum += tmio_write_data(stream, &evtheader, sizeof(evtheader));
    sum += tmio_write_data(stream, traces, runheader.tracedatasize);

    if (flush_interval > 0 && ++i % flush_interval == 0)
      tmio_flush(stream);
  }

  tmio_sync(stream);  // also flushes kernel buffers; required to time file writes

  // Print statistics
  t = timer(t);
  tcpu = cputime(tcpu);
  int payload = runheader.tracedatasize + sizeof(evtheader);
  fprintf(stderr,
          "%6d Byte/msg, %3.0f%% CPU, %7.0f msgs/s, %4.0f MByte/s\n",
          payload, 100.0 * tcpu / t, evtheader.evtno / t, (double) sum / t / 1.0e6);

  // Send run footer
  tmio_write_tag(stream, FCEndRunTag);
  tmio_write_data(stream, &runheader, sizeof(runheader));
  tmio_flush(stream);

  // Wait for acknowledge
  if (tmio_type(stream) == TMIO_SOCKET)
    tmio_read_tag(stream);

  // Print status information if required
  if (tmio_status(stream) != 0)
    fprintf(stderr, "tmio error: %s\n", tmio_status_str(stream));

  if (verbosity) {
    fprintf(stderr, "Sent %.0f byte, last status %d\n", sum, tmio_status(stream));
    tmio_monitor(stream);
  }

  // Close stream
  tmio_delete(stream);

  return 0;
}


int main_reader(char *peer,
                int bufsize,
                int timeout,
                int connect_timeout,
                int verbosity)
{
  FCRunHeader runheader = {0, 0, 0, 0, 0, 0};
  FCEventHeader evtheader = {0, 0, 0, 0, 0};
  FCTraceData *traces = NULL;

  if (verbosity)
    printf("Camera reader started\n");

  tmio_stream *stream = tmio_init("FCcamera V1.0", timeout, bufsize, verbosity);
  if (tmio_open(stream, peer, connect_timeout) == -1)
    exit(0);

  // Init statistics
  long sum = 0;
  double t = 0.0;  // timers will be started after reception of run header
  double tcpu = 0.0;

  if (verbosity)
    printf("Starting transfer...\n");

  int tag;
  int is_reading = 1;
  while (is_reading && (tag = tmio_read_tag(stream)) > 0) {
    switch (tag) {
      case FCStartRunTag:
        tmio_read_data(stream, &runheader, sizeof(runheader));

        // Note: we use a static buffer size to avoid a realloc at this
        // point to ensure similar performance measurements to the
        // sender's end
        tracesize = runheader.tracedatasize;
        if (tracepoolsize < tracesize)
          fprintf(stderr, "Warning: Traces will be truncated.\n");

        break;

      case FCEndRunTag:
        tmio_read_data(stream, &runheader, sizeof(runheader));
        is_reading = 0;
        break;

      case FCEventTag:
        if (sum == 0) {
          // Init statistics
          t = timer(0);
          tcpu = cputime(0);
        }

        traces = get_next_trace_data();
        sum += tmio_read_data(stream, &evtheader, sizeof(evtheader));
        sum += tmio_read_data(stream, traces, runheader.tracedatasize);

        break;

      default:
        fprintf(stderr, "Unknown tag %d\n", tag);
    }
  }

  if (tmio_status(stream) != 0)
    fprintf(stderr, "tmio status: %s\n", tmio_status_str(stream));

  // Print statistics
  t = timer(t);
  tcpu = cputime(tcpu);
  int payload = runheader.tracedatasize + sizeof(evtheader);
  fprintf(stderr,
          "%6d Byte/msg, %3.0f%% CPU, %7.0f msgs/s, %4.0f MByte/s\n",
          payload, 100.0 * tcpu / t, evtheader.evtno / t, (double) sum / t / 1.0e6);

  // Send acknowledge
  if (tmio_type(stream) == TMIO_SOCKET) {
    tmio_write_tag(stream, FCTrigger);
    tmio_flush(stream);
  }

  // Print status information if required
  if (verbosity) {
    fprintf(stderr, "Received %ld byte, last status %d\n", sum, tmio_status(stream));
    tmio_monitor(stream);
  }

  // Close stream
  usleep(100);  // Only required when benchmarking localhost connections
  tmio_delete(stream);

  return 0;
}


void usage(void)
{
  fprintf(stderr, "usage: tmio_benchmark [-n events] [-s tracesize] [-f flush_interval] [-m] [-v] (-r|-w) name\n"
                  "  -n events: number of events to write; an event consists of a header and a payload (default: 10000)\n"
                  "  -s tracesize: size of event payload in byte (for writer, multiple of %ld B, default: 8000 B)\n"
                  "  -f flush_interval: number of events after which buffers are flushed (default: 10)\n"
                  "  -m: read events to or write events from main memory instead of cache\n"
                  "  -b: tmio buffer size in kiB (default: 256 kiB)\n"
                  "  -t: timeout for I/O and poll operations in ms (default: 3000 ms)\n"
                  "  -v: increase verbosity; may occur more than once\n"
                  "  -r: start reader process; mutually exclusive with -w\n"
                  "  -w: start writer process; mutually exclusive with -r\n",
                  sizeof(FCTraceData));
}


int main(int argc, char **argv)
{
  // Default values for command-line parameters
  bool is_reader = false;
  bool is_writer = false;
  int events = 10000;
  int flush_interval = 10;
  int timeout = 3000;
  int connect_timeout = 3000;
  int verbosity = 0;
  bool use_large_buffer = false;
  int bufsize = 0;

  // Parse command-line parameters
  if (argc < 3) {
    usage();
    return 1;
  }

  int i = 0;
  while (++i < argc - 1) {
    char *opt = argv[i];

    if (strcmp(opt, "-m") == 0)
      use_large_buffer = true;
    else if (strcmp(opt, "-w") == 0)
      is_writer = true;
    else if (strcmp(opt, "-r") == 0)
      is_reader = true;
    else if (strcmp(opt, "-n") == 0)
      sscanf(argv[++i], "%d", &events);
    else if (strcmp(opt, "-s") == 0)
      sscanf(argv[++i], "%ld", &tracesize);
    else if (strcmp(opt, "-f") == 0)
      sscanf(argv[++i], "%d", &flush_interval);
    else if (strcmp(opt, "-t") == 0)
      sscanf(argv[++i], "%d", &timeout);
    else if (strcmp(opt, "-b") == 0)
      sscanf(argv[++i], "%d", &bufsize);
    else if (strcmp(opt, "-v") == 0)
      verbosity++;
    else {
      fprintf(stderr, "%s: illegal option -- %s\n", argv[0], opt);
      usage();
      return 1;
    }
  }

  if (!(is_reader ^ is_writer)) {
    usage();
    return 1;
  }

  // Allocate trace pool
  if (is_writer) {
    // Make sure the trace size is a multiple of the sample size
    if (tracesize < (int) sizeof(FCTraceData))
      tracesize = sizeof(FCTraceData);
    else
      tracesize = (tracesize / sizeof(FCTraceData)) * sizeof(FCTraceData);
  } else {
    // For the reader, allocate a constant-size read buffer (in real
    // applications this could grow dynamically, but not for benchmarking)
    tracesize = 1024 * 1024;  // 1 MiB
  }

  init_trace_pool(use_large_buffer);

  // Start reader/writer
  char *peer = argv[argc - 1];
  if (is_reader)
    return main_reader(peer, bufsize, timeout, connect_timeout, verbosity);
  else
    return main_writer(peer, events, flush_interval, bufsize, timeout, connect_timeout, verbosity);
}
