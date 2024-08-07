#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <fcio.h>

#include "fcio_test_utils.h"
#include "test.h"

#define FCIODEBUG 10
int main(int argc, char* argv[])
{
  assert(argc == 2);

  const char* peer = argv[1];

  FCIODebug(FCIODEBUG);

  /* write test file*/
  FCIOData* output_data = calloc(1, sizeof(FCIOData));
  FCIOStream ostream = FCIOConnect(peer, 'w', 0, 0);

  FCIOData* input_data = FCIOOpen(peer, 0, 0);
  FCIOStream istream = FCIOStreamHandle(input_data);

  fill_default_config(output_data, 12, 181, 0, 8192);
  fill_default_event(output_data);

  FCIOWriteMessage(ostream, FCIOConfig);
  FCIORWConfig(ostream, FCIOWrite2, &output_data->config);

  FCIOReadMessage(input_data->ptmio);
  FCIORWConfig(input_data->ptmio, FCIORead2, &input_data->config);
  assert(is_same_config(&output_data->config, &input_data->config));


  FCIOWriteMessage(ostream, FCIOEvent);
  FCIORWEvent(ostream, FCIOWrite2, &output_data->config, &output_data->event);

  FCIOReadMessage(input_data->ptmio);
  FCIORWEvent(input_data->ptmio, FCIORead2, &input_data->config, &input_data->event);
  assert(is_same_event(&output_data->event, &input_data->event));

  // fprintf(stderr, "adcs %d %d\n", output_data->config.adcs, input->config.adcs);
  // fprintf(stderr, "triggers %d %d\n", output->config.triggers, input->config.triggers);
  // fprintf(stderr, "eventsamples %d %d\n", output->config.eventsamples, input->config.eventsamples);


  FCIODisconnect(ostream);
  FCIOClose(input_data);
  free(output_data);

  return 0;

}
