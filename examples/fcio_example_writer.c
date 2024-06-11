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

int main(int argc, char **argv)
{
  if (argc < 2)
    return usage();

  const char *outputfile = NULL;
  int debug = 0;
  int verbosity = 0;
  int write_timeout = 0;
  int write_buffer_size = 0;
  double sleep = 0;
  int max_loops = 0;

  static struct option long_options[] =
  {
    {"help", no_argument, 0, 'h'},
    {"debug", required_argument, 0, 'd'},
    {"verbosity", no_argument, 0, 'v'},
    {"timeout", required_argument, 0, 't'},
    {"buffersize", required_argument, 0, 'b'},
    {"sleep", required_argument, 0, 's'},
    {"max", required_argument, 0, 'm'},
    {0, 0, 0, 0}
  };
  int option_index = 0;
  int c;
  while((c = getopt_long(argc, argv, "hd:t:b:d:vm:s:", long_options, &option_index)) != -1) {
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
      write_timeout = atoi(optarg);
      break;
    case 'b':
      write_buffer_size = atoi(optarg);
      break;
    case 's':
      sleep = atof(optarg);
      break;
    case 'm':
      max_loops = atoi(optarg);
      break;
    }
  }

  for (int i = optind; i < argc; i++) {
    if (i == optind) {
      outputfile = argv[i];
      if (verbosity)
        fprintf(stderr, "Outputfile: %s\n", outputfile);
    }
  }

  if (outputfile == NULL) {
    if (verbosity)
      fprintf(stderr,"Missing output file?\n");
    exit(1);
  }

  FCIODebug(debug);
  FCIOData *data = (FCIOData*)calloc( 1, sizeof(FCIOData) );

  double since = elapsed_time(0);
  if (verbosity)
    fprintf(stderr, "%f %s: connecting to output\n", elapsed_time(since), argv[0]);
  FCIOStream output = FCIOConnect(outputfile, 'w', write_timeout, write_buffer_size);

  if (output == NULL) {
    if (verbosity)
      fprintf(stderr,"Output not connected.\n");
    exit(1);
  }
  int loop = 0;
  FCIOPutRecord(output, data, FCIOConfig);
  if (verbosity)
    fprintf(stderr, "%f %s: loop %d/%d sleep %f FCIOConfig\n", elapsed_time(since), argv[0], loop, max_loops, 0.0);

  do {
    /* send both, only zeros */
    loop++;

    if (loop%2) {
      FCIOPutRecord(output, data, FCIOEvent);
      if (verbosity)
        fprintf(stderr, "%f %s: loop %d/%d sleep %f FCIOEvent\n", elapsed_time(since), argv[0], loop, max_loops, sleep);
    }
    else {
      FCIOPutRecord(output, data, FCIOSparseEvent);
      if (verbosity)
        fprintf(stderr, "%f %s: loop %d/%d sleep %f FCIOSparseEvent\n", elapsed_time(since), argv[0], loop, max_loops, sleep);
    }
    
    nsleep(sleep);

  /* If max_loops is set, we run until loop == max_loops */
  } while ( (max_loops<1) || (loop < max_loops));

  FCIOPutRecord(output, data, FCIOStatus);
  if (verbosity)
    fprintf(stderr, "%f %s: loop %d/%d sleep %f FCIOStatus\n", elapsed_time(since), argv[0], loop, max_loops, 0.0);

  FCIODisconnect(output);

  return 0;
}
