#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <fcio.h>

#define TESTTAG 31
#define TESTCONTENT 10
#define FCIODEBUG 2

/*
  This test checks if unkown tags are passed to the user by GetRecord and GetNextState without reading any data.
*/

int main(int argc, char* argv[])
{
  if (argc != 2)
    return 1;

  FCIODebug(FCIODEBUG);

  /* write test file*/
  FCIOStream stream = FCIOConnect(argv[1], 'w', 0, 0);
  FCIOWriteMessage(stream, TESTTAG);
  FCIOWriteInt(stream, TESTCONTENT);
  FCIODisconnect(stream);

  /* read using GetRecord */
  FCIOData* input = FCIOOpen(argv[1], 0, 0);
  int tag = FCIOGetRecord(input);
  assert(tag == TESTTAG);
  int content = 0;
  FCIOReadInt(input->ptmio, content);
  FCIOClose(input);
  assert(content == TESTCONTENT);

  /* read using StateReader */
  FCIOStateReader* reader = FCIOCreateStateReader(argv[1], 0, 0, 0);
  int timedout = -1;
  FCIOState* state = FCIOGetNextState(reader, &timedout);
  FCIODestroyStateReader(reader);
  assert(state->last_tag == TESTTAG);
  assert(timedout == 0);

  /* read using StateReader, but deselected */
  reader = FCIOCreateStateReader(argv[1], 0, 0, 0);
  FCIODeselectStateTag(reader, TESTTAG);
  state = FCIOGetNextState(reader, &timedout);
  FCIODestroyStateReader(reader);
  assert(state == NULL);
  assert(timedout == 2);

  return 0;

}
