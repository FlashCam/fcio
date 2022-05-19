/*
  tmio_sink

  Copies messages from one stream to another.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tmio.h"


void usage(void)
{
  fprintf(stderr, "usage: tmio_sink [-p protocol] [-t timeout] [-b bufsize] [-s max_msg_size] [-f flush_interval] [-v] input_name output_name\n"
                "  -p protocol: protocol to accept on input (default: all)\n"
                "  -t timeout: timeout for I/O and poll operations in ms (default: 5000 ms)\n"
                "  -b bufsize: tmio buffer size in kiB (default: 256 kiB)\n"
                "  -s max_msg_size: buffer size for incoming payloads in byte; larger payloads are truncated (default: 2 MiB)\n"
                "  -f flush_interval: number of items (tags, payloads) after which the output buffer is flushed (default: 10)\n"
                "  -v: increase verbosity; may occur more than once\n");
}


int main(int argc, char *argv[])
{
  char *protocol = (char *) calloc(1, 64);  // Zero-length string: accept any protocol
  int timeout = 3000;  // 3 seconds
  int connect_timeout = 5000;  // 5 seconds
  int bufsize = 0;  // Default
  int max_msg_size = 2 * 1024 * 1024;  // 2 MiB
  unsigned long flush_interval = 10;
  int verbosity = 0;

  // Parse command-line parameters
  if (argc < 3) {
    usage();
    return 1;
  }

  int i = 0;
  while (++i < argc - 2) {
    char *opt = argv[i];
    if (strcmp(opt, "-p") == 0)
      sscanf(argv[++i], "%64s", protocol);
    else if (strcmp(opt, "-t") == 0)
      sscanf(argv[++i], "%d", &timeout);
    else if(strcmp(opt, "-b") == 0)
      sscanf(argv[++i], "%d", &bufsize);
    else if (strcmp(opt, "-s") == 0)
      sscanf(argv[++i], "%d", &max_msg_size);
    else if (strcmp(opt, "-f") == 0)
      sscanf(argv[++i], "%lu", &flush_interval);
    else if (strcmp(opt, "-v") == 0)
      verbosity++;
    else {
      fprintf(stderr, "%s: illegal option -- %s\n", argv[0], opt);
      usage();
      return 1;
    }
  }

  char *input_name = argv[argc - 2];
  char *output_name = argv[argc - 1];

  // Print configuration
  fprintf(stderr, "Protocol: %64s\n", protocol);
  fprintf(stderr, "Timeout: %d ms\n", timeout);
  fprintf(stderr, "Connect timeout: %d ms\n", timeout);
  fprintf(stderr, "Buffer size: %d Byte\n", bufsize);
  fprintf(stderr, "Verbosity: %d\n", verbosity);
  fprintf(stderr, "Input: %s\n", input_name);
  fprintf(stderr, "Output: %s\n", output_name);
  fprintf(stderr, "\n");

  // Create buffer
  char *buffer = (char *) calloc(1, max_msg_size);
  if (buffer == NULL) {
    fprintf(stderr, "Failed to allocate buffer for %d bytes.\n", max_msg_size);
    return 1;
  }

  // Connect input stream
  tmio_stream *input_stream = tmio_init(protocol, timeout, bufsize, verbosity);
  if (tmio_open(input_stream, input_name, connect_timeout) == -1) {
    fprintf(stderr, "Failed to open input stream\n");
    return 1;
  }

  // Connect output stream: use input protocol for output
  strncpy(protocol, tmio_protocol(input_stream), strlen(tmio_protocol(input_stream)));
  tmio_stream *output_stream = tmio_init(protocol, timeout, bufsize, verbosity);
  if (tmio_create(output_stream, output_name, connect_timeout) == -1) {
    fprintf(stderr, "Failed to open output stream\n");
    return 1;
  }

  // Copy messages from input stream to output stream
  int tag;
  unsigned long nparts = 0;
  while (tmio_status(input_stream) == 0 && tmio_status(output_stream) == 0 &&
         (tag = tmio_read_tag(input_stream)) >= 0) {
    if (tag == 0)  // Timed out; retry
      continue;

    tmio_write_tag(output_stream, tag);
    if (flush_interval > 0 && ++nparts % flush_interval == 0)
      tmio_flush(output_stream);

    // Read and write payloads
    int nbytes;
    while ((nbytes = tmio_read_data(input_stream, buffer, max_msg_size)) >= 0) {
      if (nbytes > max_msg_size) {
        // Do not write truncated data, skip rest of record
        fprintf(stderr, "Truncated read: received %d bytes\n", nbytes);
        break;
      }

      tmio_write_data(output_stream, buffer, nbytes);
      if (flush_interval > 0 && ++nparts % flush_interval == 0)
        tmio_flush(output_stream);
    }

    if (flush_interval > 0 && ++nparts % flush_interval == 0)
      tmio_flush(output_stream);
  }

  // Check for errors
  int status = 0;
  if (tmio_status(input_stream) != 0) {
    status = 1;
    fprintf(stderr, "Input stream error: %s\n", tmio_status_str(input_stream));
  }

  if (tmio_status(output_stream) != 0) {
    status = 1;
    fprintf(stderr, "Output stream error: %s\n", tmio_status_str(output_stream));
  }

  // Clean up
  tmio_close(input_stream);
  tmio_delete(input_stream);

  tmio_close(output_stream);
  tmio_delete(output_stream);

  return status;
}
