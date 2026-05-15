/***********************************************************************
 * lazperf_adapter.h
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *
 *  Copyright (c) 2013 Natural Resources Canada
 *  Copyright (c) 2013 OpenGeo
 *  Copyright (c) 2017 Oslandia
 *
 ***********************************************************************/

#ifndef _LAZPERF_ADAPTER_H
#define _LAZPERF_ADAPTER_H

#include <stddef.h>
#include <stdint.h>

#include "pc_api_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif
  size_t lazperf_compress_from_uncompressed(const PCPATCH_UNCOMPRESSED *pa,
                                            uint8_t **compressed);
  size_t lazperf_uncompress_from_compressed(const PCPATCH_LAZPERF *pa,
                                            uint8_t **decompressed);
#ifdef __cplusplus
}
#endif

#endif /* _LAZPERF_ADAPTER_H */
