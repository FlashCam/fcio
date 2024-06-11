#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <fcio.h>
#include <time_utils.h>

int usage()
{
  const char* usage_message =
  " usage: FCIOSelect [options/flags] <inputfilename> <outputfilename>\n\n"
  " FCIOSelect reads fcio streams and selects on the given list of adc-channels, and dumps the output into another fcio stream.\n"
  " options                      parameters              description\n"
  "   --help, -h                                         show this help\n"
  "   --debug, -d                [debuglevel]            set debuglevel of output, default 2 (WARNINGS)\n\n"
  ;

  fprintf(stderr, "%s\n", usage_message);

  return 2;
}

const char* tag_to_string(int tag) {
  switch(tag) {
    case FCIOConfig: return "FCIOConfig";
    case FCIOSparseEvent: return "FCIOSparseEvent";
    case FCIOEvent: return "FCIOEvent";
    case FCIOStatus: return "FCIOStatus";
    case 0: return "FCIO-EOF/Timeout";
    case -1: return "FCIO-StreamError";
    default: return "Unknown";
  }
}

int main(int argc, char *argv[])
{

  if (argc < 2)
    return usage();

  const char *inputfile = NULL;
  int debug = 0;
  int verbosity = 0;
  int read_timeout = 0;
  int read_buffer_size = 0;
  int stop_tag = 0;
  int max_events = 0;


  static struct option long_options[] =
  {
    {"help", no_argument, 0, 'h'},
    {"debug", required_argument, 0, 'd'},
    {"verbosity", no_argument, 0, 'v'},
    {"timeout", required_argument, 0, 't'},
    {"buffersize", required_argument, 0, 'b'},
    {"stop-tag", required_argument, 0, 's'},
    {"max", required_argument, 0, 'm'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;
  while((c = getopt_long(argc, argv, "hd:t:b:s:vm:", long_options, &option_index)) != -1) {
    switch(c) {
    case '?':
    case 'h':
      exit(usage());
    case 'd':
      debug = atoi(optarg);
      break;
    case 'v':
      verbosity++;
      break;
    case 't':
      read_timeout = atoi(optarg);
      break;
    case 'b':
      read_buffer_size = atoi(optarg);
      break;
    case 's':
      stop_tag = atoi(optarg);
      break;
    case 'm':
      max_events = atoi(optarg);
      break;
    }
  }

  for (int i = optind; i < argc; i++) {
    if (i == optind) {
      inputfile = argv[i];
      if (verbosity)
        fprintf(stderr, "Inputfile: %s\n", inputfile);
    }
  }

  if (inputfile == NULL) {
    if (verbosity)
      fprintf(stderr, "Missing input file?\n");
    exit(1);
  }


  FCIODebug(debug);

  double since = elapsed_time(0);
  if (verbosity)
    fprintf(stderr, "%f %s: opening input\n", elapsed_time(since), argv[0]);
//  FCIOData *input = FCIOOpen(inputfile, read_timeout, read_buffer_size);
  FCIOStateReader* reader = FCIOCreateStateReader(inputfile, read_timeout, read_buffer_size, 10);
  /*--- Description ------------------------------------------------//

  256kb bufsize is default

  Connects to a file, server or client for FCIO read data transfer.

  name is the connection endpoint of the underlying TMIO/BUFIO
  library. Please refer to the documentation of TMIO/BUFIO for
  more information.

  name can be:

  tcp://listen/port           to listen to port at all interfaces
  tcp://listen/port/nodename  to listen to port at nodename interface
  tcp://connect/port/nodename to connect to port and nodename

  Any other name not starting with tcp: is treated as a file name.

  timeout specifies the time to wait for a connection in milliseconds.
  Specify 0 to return immediately (within the typical delays imposed by the
  connection and OS) or -1 to block indefinitely.

  buffer may be used to initialize the size (in kB) of the protocol buffers. If 0
  is specified a default value will be used.

  Returns a FCIOData structure or 0 on error.

  //----------------------------------------------------------------*/

//  if (input==NULL) {
  if (reader == NULL) {
    if(verbosity)
      fprintf(stderr, "Input not connected.\n");
    exit(1);
  }
  int read_counter = 0;
  int expected_events = 0;
  int tag;
  /*
  This is how a standard loop is written:

  while ((tag = FCIOGetRecord(input)) && tag > 0) {

  We break only on a specific tag, to demonstrate what happens
  depending on endpoint and timeout settings
  */
  while (1) {
    //tag = FCIOGetRecord(input);
    FCIOState* state = FCIOGetNextState(reader, NULL);
    if (state)
        tag = state->last_tag;
    else
        tag = stop_tag;
    read_counter++;


    switch (tag) {
      case FCIOSparseEvent:
      case FCIOEvent: {
        expected_events++;
        break;
      }
      case FCIOConfig: {
        break;
      }
      case FCIOStatus: {
        break;
      }
      default: {
        /* we expect to read either Event or SparseEvent*/
        if (tag != stop_tag)
          expected_events++;
        break;
      }
    }

    if (verbosity)
      fprintf(stderr, "%f %s: read_counter %d try reading event %d/%d tag %s \n", elapsed_time(since), argv[0], read_counter, expected_events, max_events, tag_to_string(tag));

    if (tag == stop_tag) {
      if (verbosity)
        fprintf(stderr, "%f %s: got stop_tag %d\n", elapsed_time(since), argv[0], tag);
      break;
    }

    if ( (max_events > 0) && (expected_events >= max_events)) {
      if (verbosity)
        fprintf(stderr, "%f %s: reached max_events %d\n", elapsed_time(since), argv[0], expected_events);
      break;
    }
  }

//  FCIOClose(input);
  FCIODestroyStateReader(reader);

  return 0;
}
