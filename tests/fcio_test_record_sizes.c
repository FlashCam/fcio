#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "fcio.h"
#include "fcio_utils.h"

#include "test.h"

void print_sizes(FCIORecordSizes sizes)
{
  fprintf(stderr, "..protocol    %zu bytes\n", sizes.protocol);
  fprintf(stderr, "..config      %zu bytes\n", sizes.config);
  fprintf(stderr, "..event       %zu bytes\n", sizes.event);
  fprintf(stderr, "..sparseevent %zu bytes\n", sizes.sparseevent);
  fprintf(stderr, "..eventheader %zu bytes\n", sizes.eventheader);
  fprintf(stderr, "..status      %zu bytes\n", sizes.status);
  fprintf(stderr, "..fspconfig   %zu bytes\n", sizes.fspconfig);
  fprintf(stderr, "..fspevent    %zu bytes\n", sizes.fspevent);
  fprintf(stderr, "..fspstatus   %zu bytes\n", sizes.fspstatus);
}


void set_parameters(FCIOData* data, unsigned int nchannels, unsigned int nsamples, int verbose)
{
  data->config.streamid = 100;
  data->config.adcs = nchannels;
  data->config.adccards = data->config.adcs / 24 + ((data->config.adcs % 24) == 0 ? 0 : 1);
  data->config.triggercards = data->config.adccards / 8 + ((data->config.adccards % 8) == 0 ? 0 : 1);
  data->config.triggers = data->config.triggercards * 8;
  data->config.mastercards = 1;
  data->config.eventsamples = nsamples;

  data->event.timestamp_size = 10;
  data->event.timeoffset_size = 10;
  data->event.deadregion_size = 10;
  data->event.num_traces = data->config.adcs + data->config.triggers;

  data->status.cards = data->config.adccards + data->config.triggercards + data->config.mastercards;
  data->status.size = sizeof(card_status);
  if (verbose) {
    fprintf(stderr,
      "set_parameters:\n"
      "config: eventsamples %d adcs %d triggers %d\n"
      "config: adccards %d triggercards %d mastercard %d\n"
      "event:  num_traces %d\n"
      "status: cards %d size %d\n"
      ,data->config.eventsamples, data->config.adcs, data->config.triggers,
      data->config.adccards, data->config.triggercards, data->config.mastercards,
      data->event.num_traces, data->status.cards, data->status.size
    );
  }
}

void check(FCIOData* data, unsigned int nchannels, unsigned int nsamples, int verbose) {

  set_parameters(data, nchannels, nsamples, verbose);
  FCIORecordSizes measured_sizes = {0};
  FCIORecordSizes calculated_sizes = {0};
  FCIOMeasureRecordSizes(data, &measured_sizes);
  FCIOCalculateRecordSizes(data, &calculated_sizes);

  if (verbose) {
    fprintf(stderr, "measured:\n");
    print_sizes(measured_sizes);
    fprintf(stderr, "calculated:\n");
    print_sizes(calculated_sizes);
    fprintf(stderr, "\n");
  }

  assert(measured_sizes.protocol == calculated_sizes.protocol);
  assert(measured_sizes.config == calculated_sizes.config);
  assert(measured_sizes.event == calculated_sizes.event);
  assert(measured_sizes.sparseevent == calculated_sizes.sparseevent);
  assert(measured_sizes.eventheader == calculated_sizes.eventheader);
  assert(measured_sizes.status == calculated_sizes.status);
}

int main(int argc, char* argv[])
{
  int verbose = 0;
  if (argc >= 2)
    verbose = atoi(argv[1]);

  FCIOData* data = calloc(1, sizeof(FCIOData));

  check(data, 1, 2, verbose); // min

  check(data, 1764, 128, verbose); // fc camera default
  check(data, 1764, 4096, verbose); // fc camera max
  check(data, 2304, 8192, verbose); // max 12-bit

  check(data, 181, 8192, verbose); // lgnd
  check(data, 181, 6144, verbose); // lgnd better
  check(data, 181, 32768, verbose); // lgnd fft
  check(data, 576, FCIOMaxSamples, verbose); // max 16-bit

  free(data);

  return 0;
}
