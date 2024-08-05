#include <stdio.h>
#include <stdlib.h>
#include <fcio.h>

int usage(const char* name)
{
  fprintf(stderr, "\n%s: <input> <debug:int>", name);
  fprintf(stderr, "\n\n"
    "Reads and fcio stream(file) and prints the internal INFO/DEBUG statements.\n"
    );
  return 1;
}

int main(int argc, char* argv[])
{
  if (argc < 3)
    return usage(argv[0]);

  int debug = atoi(argv[2]);

  FCIODebug(debug);

  FCIOData* io = FCIOOpen(argv[1],0,0);

  int tag;
  while ((tag = FCIOGetRecord(io)) && tag > 0)
    ;

  FCIOClose(io);
  return 0;
}
