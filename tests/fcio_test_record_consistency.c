#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <fcio.h>

#include "fcio_test_utils.h"
#include "test.h"

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

  fill_default_config(output, 12, 2304, 96, 8192);
  FCIOPutRecord(stream,output, FCIOConfig);
  tag = FCIOGetRecord(input);
  assert(tag == FCIOConfig);
  assert(is_same_config(&output->config, &input->config));

  fill_default_event(output);
  FCIOPutRecord(stream,output, FCIOEvent);
  tag = FCIOGetRecord(input);
  assert(tag == FCIOEvent);
  assert(is_same_event(&output->event, &input->event));

  fill_default_sparseevent(output);
  FCIOPutRecord(stream,output, FCIOSparseEvent);
  tag = FCIOGetRecord(input);
  assert(tag == FCIOSparseEvent);
  assert(is_same_sparseevent(&output->event, &input->event));

  fill_default_eventheader(output);
  FCIOPutRecord(stream,output, FCIOEventHeader);
  tag = FCIOGetRecord(input);
  assert(tag == FCIOEventHeader);
  assert(is_same_eventheader(&output->event, &input->event));

  fill_default_status(output);
  FCIOPutRecord(stream,output, FCIOStatus);
  tag = FCIOGetRecord(input);
  assert(tag == FCIOStatus);
  assert(is_same_status(&output->status, &input->status));

  fill_default_recevent(output);
  FCIOPutRecord(stream,output, FCIORecEvent);
  tag = FCIOGetRecord(input);
  assert(tag == FCIORecEvent);
  assert(is_same_recevent(&output->recevent, &input->recevent));

  FCIODisconnect(stream);


  FCIOClose(input);

  return 0;

}
