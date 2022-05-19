#include "tmio.h"
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

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
#define MAX_TRACEDATASIZE (2 * 1024 * 1024)
const int protocol_timeout = 3000;  // ms
const int connect_timeout = 0;  // immediate
const int wait_timeout = 100;  // ms
const int verbosity = 3;  // 0...3 (silent...very verbose)
const int buffersize = 0;  // 0: default size, >0: kByte

int main(int argc, char **argv)
{
  RunHeader runheader = {0, 0};
  EventHeader eventheader = {0};
  char tracedata[MAX_TRACEDATASIZE];
  const char *peer = argc > 1 ? argv[1] : "tcp://connect/3000/localhost";

  // Connect or open input file
  tmio_stream *stream = tmio_init("CTACamera", protocol_timeout, buffersize, verbosity);
  if (tmio_open(stream, peer, connect_timeout) == -1)
    return 1;

  int tag = 0;
  int status = 0;
  while ((status = tmio_wait(stream, wait_timeout)) != -1) {
    if (status == 0)  // Wait timed out; could do some other work at this point
      break;

    switch (tag = tmio_read_tag(stream)) {
      case StartRunTag:
        tmio_read_data(stream, &runheader, sizeof(runheader));
        break;

      case EventTag:
        tmio_read_data(stream, &eventheader, sizeof(eventheader));
        tmio_read_data(stream, tracedata, MIN(runheader.tracedatasize, MAX_TRACEDATASIZE));
        break;

      default:
        break;
    }
  }

  tmio_monitor(stream);  // Print statistics
  tmio_delete(stream);

  return 0;
}
