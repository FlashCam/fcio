#pragma once

#include <string.h>

#include <fcio.h>

void fill_default_config(FCIOData* io, int adcbits, int nadcs, int ntriggers, int eventsamples)
{
  io->config.streamid= 0;
  io->config.gps = 0;
  if (adcbits == 12) {
    io->config.adcs = nadcs;
    io->config.triggers = ntriggers;
    io->config.eventsamples = eventsamples;
    io->config.adcbits = 12;
    io->config.sumlength = 128;
    io->config.blprecision = 1;
    io->config.mastercards = 1;
    io->config.triggercards = 12*8;
    io->config.adccards = 12*8;
  } else {
    return;
  }
  for (int i = 0; i < io->config.adcs + io->config.triggers; i++) {
    int card = 1 + (i / 24);
    io->config.tracemap[i] = ((0x1 * card) << 16) + (i % 24);
  }
  for (int i = io->config.adcs; i < io->config.triggers; i++) {
    io->config.tracemap[i] = ((0x10 * i) << 16);
  }
}

void fill_default_event(FCIOData* io)
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


  io->event.timestamp[0] = 0;
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

void fill_default_sparseevent(FCIOData* io)
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

void fill_default_eventheader(FCIOData* io)
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

void fill_default_status(__attribute__((unused))FCIOData* io)
{
  return;
}

void fill_default_recevent(__attribute__((unused))FCIOData* io)
{
  return;
}

int is_same_config(fcio_config *left, fcio_config *right)
{
  return 0 == memcmp(left, right, sizeof(fcio_config));
}

int is_same_event(fcio_event *left, fcio_event *right)
{
  return left->type == right->type
  && left->pulser == right->pulser
  && left->timestamp_size == right->timestamp_size
  && left->deadregion_size == right->deadregion_size
  && left->timeoffset_size == right->timeoffset_size
  && left->num_traces == right->num_traces
  && 0 == memcmp(left->timeoffset, right->timeoffset, sizeof(int) * 10 )
  && 0 == memcmp(left->timestamp, right->timestamp, sizeof(int) * 10 )
  && 0 == memcmp(left->deadregion, right->deadregion, sizeof(int) * 10 )
  && 0 == memcmp(left->trace_list, right->trace_list, sizeof(unsigned short) * FCIOMaxChannels )
  && 0 == memcmp(left->traces, right->traces, sizeof(unsigned short) * FCIOTraceBufferLength )
  ;
}

int is_same_sparseevent(fcio_event *left, fcio_event *right)
{
  return left->type == right->type
  && left->pulser == right->pulser
  && left->timestamp_size == right->timestamp_size
  && left->deadregion_size == right->deadregion_size
  && left->timeoffset_size == right->timeoffset_size
  && left->num_traces == right->num_traces
  && 0 == memcmp(left->timeoffset, right->timeoffset, sizeof(int) * 10 )
  && 0 == memcmp(left->timestamp, right->timestamp, sizeof(int) * 10 )
  && 0 == memcmp(left->deadregion, right->deadregion, sizeof(int) * 10 )
  && 0 == memcmp(left->trace_list, right->trace_list, sizeof(unsigned short) * FCIOMaxChannels )
  && 0 == memcmp(left->traces, right->traces, sizeof(unsigned short) * FCIOTraceBufferLength )
  ;
}

int is_same_eventheader(fcio_event *left, fcio_event *right)
{
  return left->type == right->type
  && left->pulser == right->pulser
  && left->timestamp_size == right->timestamp_size
  && left->deadregion_size == right->deadregion_size
  && left->timeoffset_size == right->timeoffset_size
  && left->num_traces == right->num_traces
  && 0 == memcmp(left->timeoffset, right->timeoffset, sizeof(int) * 10 )
  && 0 == memcmp(left->timestamp, right->timestamp, sizeof(int) * 10 )
  && 0 == memcmp(left->deadregion, right->deadregion, sizeof(int) * 10 )
  && 0 == memcmp(left->trace_list, right->trace_list, sizeof(unsigned short) * FCIOMaxChannels )
  && 0 == memcmp(left->traces, right->traces, sizeof(unsigned short) * FCIOTraceBufferLength )
  ;
}

int is_same_status(fcio_status *left, fcio_status *right)
{
  return 0 == memcmp(left, right, sizeof(fcio_status));
}

int is_same_recevent(fcio_recevent *left, fcio_recevent *right)
{
  return 0 == memcmp(left, right, sizeof(fcio_recevent));
}
