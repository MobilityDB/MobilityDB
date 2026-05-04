/***********************************************************************
 * pc_patch_lazperf.c
 *
 *	PgSQL Pointcloud is free and open source software provided
 *	by the Government of Canada
 *
 *	Copyright (c) 2016 Paul Blottiere, Oslandia
 *
 ***********************************************************************/

#include <assert.h>

#include "lazperf_adapter.h"
#include "pc_api_internal.h"

void pc_patch_lazperf_free(PCPATCH_LAZPERF *pal)
{
  assert(pal);
  assert(pal->schema);
  pc_patch_free_stats((PCPATCH *)pal);
  pcfree(pal->lazperf);
  pcfree(pal);
}

PCPATCH_LAZPERF *pc_patch_lazperf_from_pointlist(const PCPOINTLIST *pdl)
{
  PCPATCH_UNCOMPRESSED *patch = pc_patch_uncompressed_from_pointlist(pdl);
  PCPATCH_LAZPERF *lazperfpatch = pc_patch_lazperf_from_uncompressed(patch);
  pc_patch_free((PCPATCH *)patch);

  return lazperfpatch;
}

PCPATCH_LAZPERF *
pc_patch_lazperf_from_uncompressed(const PCPATCH_UNCOMPRESSED *pa)
{
#ifndef HAVE_LAZPERF
  pcerror("%s: lazperf support is not enabled", __func__);
  return NULL;
#endif

  PCPATCH_LAZPERF *palaz = NULL;
  uint8_t *compressed;

  // cpp call to get compressed data from pcpatch
  size_t compressSize = lazperf_compress_from_uncompressed(pa, &compressed);

  if (compressSize != -1)
  {
    palaz = pcalloc(sizeof(PCPATCH_LAZPERF));
    palaz->type = PC_LAZPERF;
    palaz->readonly = PC_FALSE;
    palaz->schema = pa->schema;

    // not optimal but we have to pass by the context manager otherwise
    // a segfault happenned (sometimes) during a pcfree of lazperf field
    palaz->lazperf = (uint8_t *)pcalloc(compressSize);
    memcpy(palaz->lazperf, compressed, compressSize);
    free(compressed);

    palaz->npoints = pa->npoints;
    palaz->bounds = pa->bounds;
    palaz->stats = pc_stats_clone(pa->stats);
    palaz->lazperfsize = compressSize;
  }
  else
    pcerror("%s: LAZ compressionf failed", __func__);

  return palaz;
}

PCPOINTLIST *pc_pointlist_from_lazperf(const PCPATCH_LAZPERF *palaz)
{
  PCPATCH_UNCOMPRESSED *pu = NULL;
  pu = pc_patch_uncompressed_from_lazperf(palaz);
  PCPOINTLIST *pl = pc_pointlist_from_uncompressed(pu);
  pl->mem = pc_patch_uncompressed_readonly(pu);
  pc_patch_free((PCPATCH *)pu);
  return pl;
}

PCPATCH_UNCOMPRESSED *
pc_patch_uncompressed_from_lazperf(const PCPATCH_LAZPERF *palaz)
{
#ifndef HAVE_LAZPERF
  pcerror("%s: lazperf support is not enabled", __func__);
  return NULL;
#endif

  PCPATCH_UNCOMPRESSED *pcu = NULL;
  uint8_t *decompressed;

  // cpp call to uncompressed data
  size_t size = lazperf_uncompress_from_compressed(palaz, &decompressed);

  if (size != -1)
  {
    size_t datasize;
    pcu = pcalloc(sizeof(PCPATCH_UNCOMPRESSED));
    pcu->type = PC_NONE;
    pcu->readonly = PC_FALSE;
    pcu->schema = palaz->schema;
    pcu->npoints = palaz->npoints;
    pcu->bounds = palaz->bounds;
    pcu->stats = pc_stats_clone(palaz->stats);

    // not optimal but we have to pass by the context manager otherwise
    // a segfault happenned (sometimes) during a pcfree of lazperf field
    datasize = palaz->schema->size * palaz->npoints;
    pcu->data = (uint8_t *)pcalloc(datasize);
    memcpy(pcu->data, decompressed, datasize);
    free(decompressed);

    pcu->datasize = datasize;
    pcu->maxpoints = palaz->npoints;
  }
  else
    pcerror("%s: lazperf uncompression failed", __func__);

  return pcu;
}

char *pc_patch_lazperf_to_string(const PCPATCH_LAZPERF *pa)
{
#ifndef HAVE_LAZPERF
  pcerror("%s: lazperf support is not enabled", __func__);
  return NULL;
#endif
  PCPATCH_UNCOMPRESSED *patch = pc_patch_uncompressed_from_lazperf(pa);
  char *str = pc_patch_uncompressed_to_string(patch);
  pc_patch_free((PCPATCH *)patch);
  return str;
}

uint8_t *pc_patch_lazperf_to_wkb(const PCPATCH_LAZPERF *patch, size_t *wkbsize)
{
#ifndef HAVE_LAZPERF
  pcerror("%s: lazperf support is not enabled", __func__);
  return NULL;
#else
  /*
  byte:		 endianness (1 = NDR, 0 = XDR)
  uint32:	 pcid (key to POINTCLOUD_SCHEMAS)
  uint32:	 compression
  uint32:	 npoints
  uint32:	 lazperfsize
  uint8[]:	lazperfbuffer
  */

  uint8_t *buf;
  char endian = machine_endian();
  /* endian + pcid + compression + npoints + lazperfsize + lazperf */
  size_t size = 1 + 4 + 4 + 4 + 4 + patch->lazperfsize;

  uint8_t *wkb = pcalloc(size);
  uint32_t compression = patch->type;
  uint32_t npoints = patch->npoints;
  uint32_t pcid = patch->schema->pcid;
  uint32_t lazperfsize = patch->lazperfsize;
  wkb[0] = endian;                   /* Write endian flag */
  memcpy(wkb + 1, &pcid, 4);         /* Write PCID */
  memcpy(wkb + 5, &compression, 4);  /* Write compression */
  memcpy(wkb + 9, &npoints, 4);      /* Write npoints */
  memcpy(wkb + 13, &lazperfsize, 4); /* Write lazperf buffer size */

  buf = wkb + 17;
  memcpy(buf, patch->lazperf, patch->lazperfsize);
  if (wkbsize)
    *wkbsize = size;

  return wkb;
#endif
}

PCPATCH *pc_patch_lazperf_from_wkb(const PCSCHEMA *schema, const uint8_t *wkb,
                                   size_t wkbsize)
{
#ifndef HAVE_LAZPERF
  pcerror("%s: lazperf support is not enabled", __func__);
  return NULL;
#else
  /*
  byte:		 endianness (1 = NDR, 0 = XDR)
  uint32:	 pcid (key to POINTCLOUD_SCHEMAS)
  uint32:	 compression (0 = no compression, 1 = dimensional, 2 = lazperf)
  uint32:	 npoints
  uint32:	 lazperfsize
  uint8[]:	lazerperfbuffer
  */
  static size_t hdrsz =
      1 + 4 + 4 + 4; /* endian + pcid + compression + npoints */
  PCPATCH_LAZPERF *patch;
  uint8_t swap_endian = (wkb[0] != machine_endian());
  uint32_t npoints;
  size_t lazperfsize;
  const uint8_t *buf;

  if (wkb_get_compression(wkb) != PC_LAZPERF)
  {
    pcerror("%s: call with wkb that is not LAZPERF compressed", __func__);
    return NULL;
  }

  npoints = wkb_get_npoints(wkb);

  patch = pcalloc(sizeof(PCPATCH_LAZPERF));
  patch->type = PC_LAZPERF;
  patch->readonly = PC_FALSE;
  patch->schema = schema;
  patch->npoints = npoints;
  patch->stats = NULL;

  /* Start on the LAZPERF */
  buf = wkb + hdrsz;
  lazperfsize = wkb_get_int32(buf, swap_endian);
  buf += 4;

  /* Copy in the tree buffer */
  patch->lazperfsize = lazperfsize;
  patch->lazperf = pcalloc(lazperfsize);
  memcpy(patch->lazperf, buf, lazperfsize);

  return (PCPATCH *)patch;
#endif
}

int pc_patch_lazperf_compute_extent(PCPATCH_LAZPERF *patch)
{
#ifndef HAVE_LAZPERF
  pcerror("%s: lazperf support is not enabled", __func__);
  return PC_FAILURE;
#endif

  PCPATCH_UNCOMPRESSED *pau = pc_patch_uncompressed_from_lazperf(patch);
  return pc_patch_uncompressed_compute_extent(pau);
}

PCPOINT *pc_patch_lazperf_pointn(const PCPATCH_LAZPERF *patch, int n)
{
#ifndef HAVE_LAZPERF
  pcerror("%s: lazperf support is not enabled", __func__);
  return NULL;
#endif

  PCPOINT *pt = pc_point_make(patch->schema);
  PCPATCH_UNCOMPRESSED *pau = pc_patch_uncompressed_from_lazperf(patch);
  size_t size = patch->schema->size;
  memcpy(pt->data, pau->data + n * size, size);
  pc_patch_free((PCPATCH *)pau);
  return pt;
}
