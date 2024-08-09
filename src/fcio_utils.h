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

FCIORecordSizes FCIOMeasureRecordSizes(FCIOData* data, FCIORecordSizes sizes);
FCIORecordSizes FCIOCalculateRecordSizes(FCIOData* data, FCIORecordSizes sizes);
