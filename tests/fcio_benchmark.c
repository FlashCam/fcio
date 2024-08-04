/*
  tmio_benchmark

  General purpose benchmarking utility for bi- and unidirectional tmio
  streams. Transfer structure tries to imitate a camera server, sending
  a run header followed by many events (20 Byte header, configurable trace
  data) and a final end-of-run.
*/

#define _DEFAULT_SOURCE

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcio.h>
#include "fcio_test_utils.h"
#include "test.h"
#include "timer.h"


int verbosity = 0;

int main_writer(const char *peer,
                int events,
                int bufsize,
                int connect_timeout,
                int verbosity,
                int nadcs,
                int ntriggers,
                int eventsamples
                )
{
  FCIOData* payload = calloc(1, sizeof(FCIOData));
  fill_default_config(payload, 12, nadcs, ntriggers, eventsamples);
  fill_default_event(payload);
  int msgcounter = 0;

  init_benchmark_statistics();

  FCIOStream stream = FCIOConnect(peer, 'w', connect_timeout, bufsize);
  if ( !FCIOPutConfig(stream, payload) ) {
    return msgcounter;
  }
  msgcounter++;
  for (int i = 0; i < events; i++) {
    if( !FCIOPutRecord(stream, payload, FCIOEvent) ) {
      break;
    }
    msgcounter++;
  }
  FCIODisconnect(stream);

  print_benchmark_statistics("writer", msgcounter, sizeof(fcio_event), events * sizeof(fcio_event));

  return msgcounter;
}


int main_reader(const char *peer,
                int bufsize,
                int connect_timeout,
                int verbosity)
{
  int tag;
  int msgcounter = 0;

  init_benchmark_statistics();

  FCIOData* io = FCIOOpen(peer, connect_timeout, bufsize);
  while ( (tag = FCIOGetRecord(io)) && tag > 0)
    msgcounter++;
  FCIOClose(io);

  print_benchmark_statistics("reader", msgcounter, sizeof(fcio_event), msgcounter * sizeof(fcio_event));

  return msgcounter;
}


void usage(void)
{
  fprintf(stderr, "usage: fcio_benchmark [-n events] [-c nchannels] [-s eventsamples] [-v verbositylevel] [-w write_peer] [-r read_peer]\n"
                  "  -n events: number of events to write; an event consists of a header and a payload (default: 10000)\n"
                  "  -c nchannels: number of channels\n"
                  "  -s eventsamples: number of samples per channel\n"
                  "  -b: tmio buffer size in kiB (default: 256 kiB)\n"
                  "  -t: timeout for I/O and poll operations in ms (default: 3000 ms)\n"
                  "  -v: set verbosity level\n"
                  "  -r: set reader peer\n"
                  "  -w: set writer peer\n"
                  );
}


int main(int argc, char **argv)
{
  // Default values for command-line parameters
  int events = 10000;
  int timeout = 3000;
  int verbosity = 2;
  int bufsize = 0;
  int eventsamples = 4096;
  int ntriggers = 0;
  int nadcs = 1;
  int no_fork = 0;

  const char* write_peer = NULL;
  const char* read_peer = NULL;

  int write_delay = 0;
  int read_delay = 0;

  int i = 0;
  while (++i < argc) {
    char *opt = argv[i];

    if (strcmp(opt, "-n") == 0)
      sscanf(argv[++i], "%d", &events);
    else if (strcmp(opt, "-s") == 0)
      sscanf(argv[++i], "%d", &eventsamples);
    else if (strcmp(opt, "-t") == 0)
      sscanf(argv[++i], "%d", &timeout);
    else if (strcmp(opt, "-b") == 0)
      sscanf(argv[++i], "%d", &bufsize);
    else if (strcmp(opt, "-v") == 0)
      sscanf(argv[++i], "%d", &verbosity);
    else if (strcmp(opt, "-c") == 0)
      sscanf(argv[++i], "%d", &nadcs);
    else if (strcmp(opt, "-w") == 0)
      write_peer = argv[++i];
    else if (strcmp(opt, "-r") == 0)
      read_peer = argv[++i];
    else if (strcmp(opt, "--no-fork") == 0)
      no_fork = 1;
    else if (strcmp(opt, "--delay") == 0) {
      switch (*argv[++i]) {
        case 'w': write_delay = atoi(argv[i]+2); break;
        case 'r': read_delay = atoi(argv[i]+2); break;
        default: {
          fprintf(stderr, "--delay requires <r|w>,<delay_in_usec>\n");
          usage();
          return 1;
        }
      }
    }
    else {
      fprintf(stderr, "%s: illegal option -- %s\n", argv[0], opt);
      usage();
      return 1;
    }
  }

  if (!write_peer && !read_peer) {
    fprintf(stderr, "At least one peer required. Use -w|-r to specify.\n");
    usage();
    return 1;
  }

  if (!write_peer || !read_peer) {
    no_fork = 1;
  }

  FCIODebug(verbosity);

  int n_expected_records = events + 1;
  if (verbosity) {
    if (write_peer) fprintf(stderr, "%s writer with %d us delay.\n", no_fork ? "Starting" : "Forking",  write_delay);
    if (read_peer) fprintf(stderr, "Starting reader with %d us delay.\n", read_delay);
  }

  if (no_fork) {
    if (write_peer) {
      usleep(write_delay);
      assert(main_writer(write_peer, events, bufsize, timeout, verbosity, nadcs, ntriggers, eventsamples) == n_expected_records);
    }
    if (read_peer) {
      usleep(read_delay);
      assert(main_reader(read_peer, bufsize, timeout, verbosity) == n_expected_records);
    }

  } else {
    FORK_CHILD
    usleep(write_delay);
    assert(main_writer(write_peer, events, bufsize, timeout, verbosity, nadcs, ntriggers, eventsamples) == n_expected_records);
    FORK_PARENT
    usleep(read_delay);
    assert(main_reader(read_peer, bufsize, timeout, verbosity) == n_expected_records);
    FORK_JOIN
  }
  return 0;
}
