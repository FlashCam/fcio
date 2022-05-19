#include "tmio.h"

// Define tags and template data structures
#define StartRunTag 1
#define EventTag    2

typedef struct {
  int runid;
  int tracedatasize;
} RunHeader;

typedef struct {
  int evtid;
} EventHeader;

// Configuration
#define TRACEDATASIZE (8000)
const int nevents = 10000;
const int protocol_timeout = 3000;  // ms
const int connect_timeout = -1;  // indefinite
const int wait_timeout = 0;  // immediate
const int verbosity = 3;  // 0...3 (silent...very verbose)
const int buffersize = 0;  // 0: default size, >0: kByte

/* Sets event header and trace buffer. Returns 0 after nevents have been
   requested. */
int get_dummy_event(EventHeader *evtheader, char **tracedata)
{
  static char traces[TRACEDATASIZE];
  static int evtid = 0;

  evtheader->evtid = evtid;
  *tracedata = traces;
  return evtid++ < nevents;
}


int main(int argc, char **argv)
{
  RunHeader runheader = {0, TRACEDATASIZE};
  EventHeader eventheader = {0};
  char *tracedata;
  const char *peer = argc > 1 ? argv[1] : "tcp://listen/3000";

  // Listen for connection or create output file
  tmio_stream *stream = tmio_init("CTACamera", protocol_timeout, buffersize, verbosity);
  if (tmio_create(stream, peer, connect_timeout) == -1)
    return 1;

  // Write run header
  tmio_write_tag(stream, StartRunTag);
  tmio_write_data(stream, &runheader, sizeof(runheader));

  while (tmio_status(stream) == 0 && get_dummy_event(&eventheader, &tracedata)) {
    // Write event
    tmio_write_tag(stream, EventTag);
    tmio_write_data(stream, &eventheader, sizeof(eventheader));
    tmio_write_data(stream, tracedata, runheader.tracedatasize);

    // Quickly poll for an incoming message
    if (tmio_wait(stream, wait_timeout) == 1)
      break;
  }

  tmio_monitor(stream);  // Print statistics
  tmio_delete(stream);

  return 0;
}
