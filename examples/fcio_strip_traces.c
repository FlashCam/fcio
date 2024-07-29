#include <stdio.h>
#include <stdlib.h>

#include <fcio.h>

int usage(const char* name)
{
  fprintf(stderr, "\n%s: <input> <output>", name);
  fprintf(stderr, "\n\n"
    "Removes the trace memory fields from FCIOEvent and FCIOSparseEvent records.\n"
    "Replaces them with FCIOEventHeader records in <output>.\n"
    );
  return 1;
}

int main(int argc, const char* argv[])
{
  if (argc < 3)
    return usage(argv[0]);

  FCIOData* io = FCIOOpen(argv[1],0,0);
  FCIOStream out = FCIOConnect(argv[2],'w',0,0);

  int tag;
  while ((tag = FCIOGetRecord(io)) && tag > 0) {
    switch(tag) {
      case FCIOEvent:
      case FCIOSparseEvent: {
        FCIOPutEventHeader(out,io);
        break;
      }
      default: {
        FCIOPutRecord(out,io,tag);
        break;
      }
    }
  }
  FCIOClose(io);
  FCIODisconnect(out);
  return 0;
}
