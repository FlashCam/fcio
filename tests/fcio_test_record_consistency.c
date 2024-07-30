#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <fcio.h>
#include <string.h>

static void fill_default_config(FCIOData* io, int adcbits)
{
  io->config.telid = 0;
  io->config.gps = 0;
  if (adcbits == 12) {
    io->config.adcs = 12*8*24;
    io->config.triggers = 12*8;
    io->config.eventsamples = 8192;
    io->config.adcbits = 12;
    io->config.sumlength = 128;
    io->config.blprecision = 1;
    io->config.mastercards = 1;
    io->config.triggercards = 12*8;
    io->config.adccards = 12*8;
  } else {
    return;
  }
  for (int i = 0; i < io->config.adcs; i++) {
    int card = 1 + (i / 24);
    io->config.tracemap[i] = ((0x1 * card) << 16) + (i % 24);
  }
  for (int i = io->config.adcs; i < io->config.triggers; i++) {
    io->config.tracemap[i] = ((0x10 * i) << 16);
  }
}

static void fill_default_event(FCIOData* io)
{
  io->event.type = 0;
  io->event.pulser = 0;
  io->event.timestamp_size = 5;
  io->event.timeoffset_size = 5;
  io->event.deadregion_size = 5;

  // start fcio_get_event fills these fields on FCIOEvent
  io->event.deadregion[5] = 0;
  io->event.deadregion[6] = io->config.adcs + io->config.triggers;
  io->event.num_traces = io->event.deadregion[6];
  for (int i = 0; i < FCIOMaxChannels; i++)
    io->event.trace_list[i] = i;
  // end


  io->event.timestamp[0] = 1;
  io->event.timestamp[1] = 2;
  io->event.timestamp[2] = 123456789;
  io->event.timestamp[3] = 249999999;

  int counter = 0;
  for (int trace_idx = 0; trace_idx < io->config.adcs; trace_idx++) {
    for (int sample_idx = 0; sample_idx < io->config.eventsamples; sample_idx++) {
      io->event.traces[trace_idx * (io->config.eventsamples + 2) + sample_idx] = counter++;
    }
  }
}

static void fill_default_sparseevent(FCIOData* io)
{
  io->event.type = 0;
  io->event.pulser = 0;
  io->event.timestamp_size = 5;
  io->event.timeoffset_size = 5;
  io->event.deadregion_size = 5;

  io->event.timestamp[0] = 1;
  io->event.timestamp[1] = 2;
  io->event.timestamp[2] = 123456789;
  io->event.timestamp[3] = 249999999;

  int counter = 0;
  for (int trace_idx = 0; trace_idx < io->config.adcs; trace_idx++) {
    for (int sample_idx = 0; sample_idx < io->config.eventsamples; sample_idx++) {
      io->event.traces[trace_idx * (io->config.eventsamples + 2) + sample_idx] = counter++;
    }
  }
}

static void fill_default_eventheader(FCIOData* io)
{
  io->event.type = 0;
  io->event.pulser = 0;
  io->event.timestamp_size = 5;
  io->event.timeoffset_size = 5;
  io->event.deadregion_size = 5;

  io->event.timestamp[0] = 1;
  io->event.timestamp[1] = 2;
  io->event.timestamp[2] = 123456789;
  io->event.timestamp[3] = 249999999;

  int counter = 0;
  for (int trace_idx = 0; trace_idx < io->config.adcs; trace_idx++) {
    for (int sample_idx = 0; sample_idx < io->config.eventsamples; sample_idx++) {
      io->event.traces[trace_idx * (io->config.eventsamples + 2) + sample_idx] = counter++;
    }
  }
}

static void fill_default_status(FCIOData* io)
{
  return;
}

static void fill_default_recevent(FCIOData* io)
{
  return;
}

static inline void cmp_config(fcio_config *left, fcio_config *right)
{
  assert(0 == memcmp(left, right, sizeof(fcio_config)));
}

static inline void cmp_event(fcio_event *left, fcio_event *right)
{
  assert(left->type == right->type);
  assert(left->pulser == right->pulser);
  assert(left->timestamp_size == right->timestamp_size);
  assert(left->deadregion_size == right->deadregion_size);
  assert(left->timeoffset_size == right->timeoffset_size);
  assert(left->num_traces == right->num_traces);
  assert(0 == memcmp(left->timeoffset, right->timeoffset, sizeof(int) * 10 ));
  assert(0 == memcmp(left->timestamp, right->timestamp, sizeof(int) * 10 ));
  assert(0 == memcmp(left->deadregion, right->deadregion, sizeof(int) * 10 ));
  assert(0 == memcmp(left->trace_list, right->trace_list, sizeof(unsigned short) * FCIOMaxChannels ));
  assert(0 == memcmp(left->traces, right->traces, sizeof(unsigned short) * FCIOTraceBufferLength ));
}

static inline void cmp_sparseevent(fcio_event *left, fcio_event *right)
{
  assert(left->type == right->type);
  assert(left->pulser == right->pulser);
  assert(left->timestamp_size == right->timestamp_size);
  assert(left->deadregion_size == right->deadregion_size);
  assert(left->timeoffset_size == right->timeoffset_size);
  assert(left->num_traces == right->num_traces);
  assert(0 == memcmp(left->timeoffset, right->timeoffset, sizeof(int) * 10 ));
  assert(0 == memcmp(left->timestamp, right->timestamp, sizeof(int) * 10 ));
  assert(0 == memcmp(left->deadregion, right->deadregion, sizeof(int) * 10 ));
  assert(0 == memcmp(left->trace_list, right->trace_list, sizeof(unsigned short) * FCIOMaxChannels ));
  assert(0 == memcmp(left->traces, right->traces, sizeof(unsigned short) * FCIOTraceBufferLength ));
}

static inline void cmp_eventheader(fcio_event *left, fcio_event *right)
{
  assert(left->type == right->type);
  assert(left->pulser == right->pulser);
  assert(left->timestamp_size == right->timestamp_size);
  assert(left->deadregion_size == right->deadregion_size);
  assert(left->timeoffset_size == right->timeoffset_size);
  assert(left->num_traces == right->num_traces);
  assert(0 == memcmp(left->timeoffset, right->timeoffset, sizeof(int) * 10 ));
  assert(0 == memcmp(left->timestamp, right->timestamp, sizeof(int) * 10 ));
  assert(0 == memcmp(left->deadregion, right->deadregion, sizeof(int) * 10 ));
  assert(0 == memcmp(left->trace_list, right->trace_list, sizeof(unsigned short) * FCIOMaxChannels ));
  assert(0 == memcmp(left->traces, right->traces, sizeof(unsigned short) * FCIOTraceBufferLength ));
}

static inline void cmp_status(fcio_status *left, fcio_status *right)
{
  assert(0 == memcmp(left, right, sizeof(fcio_status)));
}

static inline void cmp_recevent(fcio_recevent *left, fcio_recevent *right)
{
  assert(0 == memcmp(left, right, sizeof(fcio_recevent)));
}

#define FCIODEBUG 0
int main(int argc, char* argv[])
{
  assert(argc == 2);

  const char* peer = argv[1];

  FCIODebug(FCIODEBUG);
  int tag = 0;

  /* write test file*/
  FCIOStream stream = FCIOConnect(peer, 'w', 0, 0);
  FCIOData* input = FCIOOpen(peer, 0, 0);
  FCIOData* output = calloc(1, sizeof(FCIOData));
  memcpy(&output->config, &input->config, sizeof(fcio_config));
  memcpy(&output->event, &input->event, sizeof(fcio_event));
  memcpy(&output->status, &input->status, sizeof(fcio_status));
  memcpy(&output->recevent, &input->recevent, sizeof(fcio_recevent));

  fill_default_config(output, 12);
  FCIOPutRecord(stream,output, FCIOConfig);
  tag = FCIOGetRecord(input);
  assert(tag == FCIOConfig);
  cmp_config(&output->config, &input->config);

  fill_default_event(output);
  FCIOPutRecord(stream,output, FCIOEvent);
  tag = FCIOGetRecord(input);
  assert(tag == FCIOEvent);
  cmp_event(&output->event, &input->event);

  fill_default_sparseevent(output);
  FCIOPutRecord(stream,output, FCIOSparseEvent);
  tag = FCIOGetRecord(input);
  assert(tag == FCIOSparseEvent);
  cmp_sparseevent(&output->event, &input->event);

  fill_default_eventheader(output);
  FCIOPutRecord(stream,output, FCIOEventHeader);
  tag = FCIOGetRecord(input);
  assert(tag == FCIOEventHeader);
  cmp_eventheader(&output->event, &input->event);

  fill_default_status(output);
  FCIOPutRecord(stream,output, FCIOStatus);
  tag = FCIOGetRecord(input);
  assert(tag == FCIOStatus);
  cmp_status(&output->status, &input->status);

  fill_default_recevent(output);
  FCIOPutRecord(stream,output, FCIORecEvent);
  tag = FCIOGetRecord(input);
  assert(tag == FCIORecEvent);
  cmp_recevent(&output->recevent, &input->recevent);

  FCIODisconnect(stream);








  FCIOClose(input);

  return 0;


  // /* read using StateReader */
  // FCIOStateReader* reader = FCIOCreateStateReader(argv[1], 0, 0, 0);
  // int timedout = -1;
  // FCIOState* state = FCIOGetNextState(reader, &timedout);
  // FCIODestroyStateReader(reader);
  // assert(state->last_tag == TESTTAG);
  // assert(timedout == 0);

  // /* read using StateReader, but deselected */
  // reader = FCIOCreateStateReader(argv[1], 0, 0, 0);
  // FCIODeselectStateTag(reader, TESTTAG);
  // state = FCIOGetNextState(reader, &timedout);
  // FCIODestroyStateReader(reader);
  // assert(state == NULL);
  // assert(timedout == 2);

  // return 0;

}
