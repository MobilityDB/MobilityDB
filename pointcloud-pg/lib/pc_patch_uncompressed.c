/***********************************************************************
 * pc_patch_uncompressed.c
 *
 *  Pointclound patch handling. Create, get and set values from the
 *  uncompressed PCPATCH structure.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include "pc_api_internal.h"
#include "stringbuffer.h"
#include <assert.h>

/* TODO: expose to API ? Would require also exposing stringbuffer
 *  See https://github.com/pgpointcloud/pointcloud/issues/74
 */
static int
pc_patch_uncompressed_to_stringbuffer(const PCPATCH_UNCOMPRESSED *patch,
                                      stringbuffer_t *sb)
{
  PCPOINTLIST *pl;
  int i, j;

  /* { "pcid":1, "points":[[<dim1>, <dim2>, <dim3>, <dim4>],[<dim1>, <dim2>,
   * <dim3>, <dim4>]] }*/

  /* TODO: reserve space in buffer ? */

  pl = pc_pointlist_from_uncompressed(patch);
  stringbuffer_aprintf(sb, "{\"pcid\":%d,\"pts\":[", patch->schema->pcid);
  for (i = 0; i < pl->npoints; i++)
  {
    PCPOINT *pt = pc_pointlist_get_point(pl, i);
    if (i)
      stringbuffer_append(sb, ",[");
    else
      stringbuffer_append(sb, "[");
    for (j = 0; j < pt->schema->ndims; j++)
    {
      double d;
      if (!pc_point_get_double_by_index(pt, j, &d))
      {
        pcerror("%s: unable to read double at index %d", __func__, j);
        return PC_FAILURE;
      }
      if (j)
        stringbuffer_aprintf(sb, ",%g", d);
      else
        stringbuffer_aprintf(sb, "%g", d);
    }
    stringbuffer_append(sb, "]");
  }
  stringbuffer_append(sb, "]}");

  /* All done, copy and clean up */
  pc_pointlist_free(pl);

  return PC_SUCCESS;
}

char *pc_patch_uncompressed_to_string(const PCPATCH_UNCOMPRESSED *patch)
{
  stringbuffer_t *sb = stringbuffer_create();
  char *str;
  if (PC_FAILURE == pc_patch_uncompressed_to_stringbuffer(patch, sb))
    return NULL;
  str = stringbuffer_release_string(sb);
  stringbuffer_destroy(sb);
  return str;
}

uint8_t *pc_patch_uncompressed_to_wkb(const PCPATCH_UNCOMPRESSED *patch,
                                      size_t *wkbsize)
{
  /*
  byte:     endianness (1 = NDR, 0 = XDR)
  uint32:   pcid (key to POINTCLOUD_SCHEMAS)
  uint32:   compression (0 = no compression, 1 = dimensional, 2 = lazperf)
  uint32:   npoints
  uchar[]:  data (interpret relative to pcid)
  */
  char endian = machine_endian();
  /* endian + pcid + compression + npoints + datasize */
  size_t size = 1 + 4 + 4 + 4 + patch->datasize;
  uint8_t *wkb = pcalloc(size);
  uint32_t compression = patch->type;
  uint32_t npoints = patch->npoints;
  uint32_t pcid = patch->schema->pcid;
  wkb[0] = endian;                                /* Write endian flag */
  memcpy(wkb + 1, &pcid, 4);                      /* Write PCID */
  memcpy(wkb + 5, &compression, 4);               /* Write compression */
  memcpy(wkb + 9, &npoints, 4);                   /* Write npoints */
  memcpy(wkb + 13, patch->data, patch->datasize); /* Write data */
  if (wkbsize)
    *wkbsize = size;
  return wkb;
}

PCPATCH *pc_patch_uncompressed_from_wkb(const PCSCHEMA *s, const uint8_t *wkb,
                                        size_t wkbsize)
{
  /*
  byte:     endianness (1 = NDR, 0 = XDR)
  uint32:   pcid (key to POINTCLOUD_SCHEMAS)
  uint32:   compression (0 = no compression, 1 = dimensional, 2 = lazperf)
  uint32:   npoints
  pcpoint[]:  data (interpret relative to pcid)
  */
  static size_t hdrsz =
      1 + 4 + 4 + 4; /* endian + pcid + compression + npoints */
  PCPATCH_UNCOMPRESSED *patch;
  uint8_t *data;
  uint8_t swap_endian = (wkb[0] != machine_endian());
  uint32_t npoints;

  if (wkb_get_compression(wkb) != PC_NONE)
  {
    pcerror("%s: call with wkb that is not uncompressed", __func__);
    return NULL;
  }

  npoints = wkb_get_npoints(wkb);
  if ((wkbsize - hdrsz) != (s->size * npoints))
  {
    pcerror("%s: wkb size and expected data size do not match", __func__);
    return NULL;
  }

  if (swap_endian)
  {
    data = uncompressed_bytes_flip_endian(wkb + hdrsz, s, npoints);
  }
  else
  {
    data = pcalloc(npoints * s->size);
    memcpy(data, wkb + hdrsz, npoints * s->size);
  }

  patch = pcalloc(sizeof(PCPATCH_UNCOMPRESSED));
  patch->type = PC_NONE;
  patch->readonly = PC_FALSE;
  patch->schema = s;
  patch->npoints = npoints;
  patch->maxpoints = npoints;
  patch->datasize = (wkbsize - hdrsz);
  patch->data = data;
  patch->stats = NULL;

  return (PCPATCH *)patch;
}

PCPATCH_UNCOMPRESSED *pc_patch_uncompressed_make(const PCSCHEMA *s,
                                                 uint32_t maxpoints)
{
  PCPATCH_UNCOMPRESSED *pch;
  size_t datasize;

  if (!s)
  {
    pcerror("%s: null schema passed in", __func__);
    return NULL;
  }

  /* Width of the data area */
  if (!s->size)
  {
    pcerror("%s, invalid size calculation", __func__);
    return NULL;
  }

  /* Set up basic info */
  pch = pcalloc(sizeof(PCPATCH_UNCOMPRESSED));
  pch->type = PC_NONE;
  pch->readonly = PC_FALSE;
  pch->schema = s;
  pch->npoints = 0;
  pch->stats = NULL;
  pch->maxpoints = maxpoints;

  /* Make our own data area */
  datasize = s->size * maxpoints;
  pch->datasize = datasize;
  pch->data = NULL;
  if (datasize)
  {
    pch->data = pcalloc(datasize);
  }
  pc_bounds_init(&(pch->bounds));

  return pch;
}

int pc_patch_uncompressed_compute_extent(PCPATCH_UNCOMPRESSED *patch)
{
  int i;
  PCPOINT *pt = pc_point_from_data(patch->schema, patch->data);
  PCBOUNDS b;
  double x, y;

  /* Calculate bounds */
  pc_bounds_init(&b);
  for (i = 0; i < patch->npoints; i++)
  {
    /* Just push the data buffer forward by one point at a time */
    pt->data = patch->data + i * patch->schema->size;
    pc_point_get_x(pt, &x);
    pc_point_get_y(pt, &y);
    if (b.xmin > x)
      b.xmin = x;
    if (b.ymin > y)
      b.ymin = y;
    if (b.xmax < x)
      b.xmax = x;
    if (b.ymax < y)
      b.ymax = y;
  }

  patch->bounds = b;
  pcfree(pt);
  return PC_SUCCESS;
}

void pc_patch_uncompressed_free(PCPATCH_UNCOMPRESSED *patch)
{
  assert(patch);
  assert(patch->schema);

  pc_patch_free_stats((PCPATCH *)patch);

  if (patch->data && !patch->readonly)
  {
    pcfree(patch->data);
  }
  pcfree(patch);
}

// Make the patch readonly. Return the memory segment
// owned by the patch, if any, to enable transfer of ownership
uint8_t *pc_patch_uncompressed_readonly(PCPATCH_UNCOMPRESSED *patch)
{
  uint8_t *data = patch->readonly ? NULL : patch->data;
  patch->readonly = PC_TRUE;
  return data;
}

PCPATCH_UNCOMPRESSED *
pc_patch_uncompressed_from_pointlist(const PCPOINTLIST *pl)
{
  PCPATCH_UNCOMPRESSED *pch;
  const PCSCHEMA *s;
  PCPOINT *pt;
  uint8_t *ptr;
  int i;
  uint32_t numpts;

  if (!pl)
  {
    pcerror("%s: null PCPOINTLIST passed in", __func__);
    return NULL;
  }

  numpts = pl->npoints;
  if (!numpts)
  {
    pcerror("%s: zero size PCPOINTLIST passed in", __func__);
    return NULL;
  }

  /* Assume the first PCSCHEMA is the same as the rest for now */
  /* We will check this as we go along */
  pt = pc_pointlist_get_point(pl, 0);
  s = pt->schema;

  /* Confirm we have a schema pointer */
  if (!s)
  {
    pcerror("%s: null schema encountered", __func__);
    return NULL;
  }

  /* Confirm width of a point data buffer */
  if (!s->size)
  {
    pcerror("%s: invalid point size", __func__);
    return NULL;
  }

  /* Make our own data area */
  pch = pcalloc(sizeof(PCPATCH_UNCOMPRESSED));
  pch->datasize = s->size * numpts;
  pch->data = pcalloc(pch->datasize);
  pch->stats = NULL;
  ptr = pch->data;

  /* Initialize bounds */
  pc_bounds_init(&(pch->bounds));

  /* Set up basic info */
  pch->readonly = PC_FALSE;
  pch->maxpoints = numpts;
  pch->type = PC_NONE;
  pch->schema = s;
  pch->npoints = 0;

  for (i = 0; i < numpts; i++)
  {
    pt = pc_pointlist_get_point(pl, i);
    if (pt)
    {
      if (pt->schema->pcid != s->pcid)
      {
        pcerror("%s: points do not share a schema", __func__);
        return NULL;
      }
      memcpy(ptr, pt->data, s->size);
      pch->npoints++;
      ptr += s->size;
    }
    else
    {
      pcwarn("%s: encountered null point", __func__);
    }
  }

  if (PC_FAILURE == pc_patch_uncompressed_compute_extent(pch))
  {
    pcerror("%s: failed to compute patch extent", __func__);
    return NULL;
  }

  if (PC_FAILURE == pc_patch_uncompressed_compute_stats(pch))
  {
    pcerror("%s: failed to compute patch stats", __func__);
    return NULL;
  }

  return pch;
}

PCPATCH_UNCOMPRESSED *
pc_patch_uncompressed_from_dimensional(const PCPATCH_DIMENSIONAL *pdl)
{
  int i, j, npoints;
  PCPATCH_UNCOMPRESSED *patch;
  PCPATCH_DIMENSIONAL *pdl_uncompressed;
  const PCSCHEMA *schema;
  uint8_t *buf;

  npoints = pdl->npoints;
  schema = pdl->schema;
  patch = pcalloc(sizeof(PCPATCH_UNCOMPRESSED));
  patch->type = PC_NONE;
  patch->readonly = PC_FALSE;
  patch->schema = schema;
  patch->npoints = npoints;
  patch->maxpoints = npoints;
  patch->bounds = pdl->bounds;
  patch->stats = pc_stats_clone(pdl->stats);
  patch->datasize = schema->size * pdl->npoints;
  patch->data = pcalloc(patch->datasize);
  buf = patch->data;

  /* Can only read from uncompressed dimensions */
  pdl_uncompressed = pc_patch_dimensional_decompress(pdl);

  for (i = 0; i < npoints; i++)
  {
    for (j = 0; j < schema->ndims; j++)
    {
      PCDIMENSION *dim = pc_schema_get_dimension(schema, j);
      uint8_t *in = pdl_uncompressed->bytes[j].bytes + dim->size * i;
      uint8_t *out = buf + dim->byteoffset;
      memcpy(out, in, dim->size);
    }
    buf += schema->size;
  }

  pc_patch_dimensional_free(pdl_uncompressed);

  return patch;
}

int pc_patch_uncompressed_add_point(PCPATCH_UNCOMPRESSED *c, const PCPOINT *p)
{
  size_t sz;
  uint8_t *ptr;
  double x, y;

  if (!(c && p))
  {
    pcerror("%s: null point or patch argument", __func__);
    return PC_FAILURE;
  }

  if (c->schema->pcid != p->schema->pcid)
  {
    pcerror("%s: pcids of point (%d) and patch (%d) not equal", __func__,
            c->schema->pcid, p->schema->pcid);
    return PC_FAILURE;
  }

  if (c->readonly)
  {
    pcerror("%s: cannot add point to readonly patch", __func__);
    return PC_FAILURE;
  }

  if (c->type != PC_NONE)
  {
    pcerror("%s: cannot add point to compressed patch", __func__);
    return PC_FAILURE;
  }

  sz = c->schema->size;

  /* Double the data buffer if it's already full */
  if (c->npoints == c->maxpoints)
  {
    c->maxpoints *= 2;
    c->datasize = c->maxpoints * sz;
    c->data = pcrealloc(c->data, c->datasize);
  }

  /* Copy the data buffer from point to patch */
  ptr = c->data + sz * c->npoints;
  memcpy(ptr, p->data, sz);
  c->npoints += 1;

  /* Update bounding box */
  pc_point_get_x(p, &x);
  pc_point_get_y(p, &y);
  if (c->bounds.xmin > x)
    c->bounds.xmin = x;
  if (c->bounds.ymin > y)
    c->bounds.ymin = y;
  if (c->bounds.xmax < x)
    c->bounds.xmax = x;
  if (c->bounds.ymax < y)
    c->bounds.ymax = y;

  return PC_SUCCESS;
}

/** get point n, 0-based, positive */
PCPOINT *pc_patch_uncompressed_pointn(const PCPATCH_UNCOMPRESSED *patch, int n)
{
  return pc_point_from_data(patch->schema,
                            patch->data + n * patch->schema->size);
}
