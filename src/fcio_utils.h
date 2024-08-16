#pragma once

#include <stddef.h>

#include "fcio.h"

typedef struct {
  size_t protocol;
  size_t config;
  size_t event;
  size_t sparseevent;
  size_t eventheader;
  size_t status;
  size_t fspconfig;
  size_t fspevent;
  size_t fspstatus;

} FCIORecordSizes;

void FCIOMeasureRecordSizes(FCIOData* data, FCIORecordSizes* sizes);
void FCIOCalculateRecordSizes(FCIOData* data, FCIORecordSizes* sizes);
size_t FCIOWrittenBytes(FCIOStream stream);
int FCIOSetMemField(FCIOStream stream, char *mem_addr, size_t mem_size);
const char* FCIOTagStr(int tag);
