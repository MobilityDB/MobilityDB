/***********************************************************************
 * pc_pointlist.c
 *
 *  Point list handling. Create, get and set values from the
 *  basic PCPOINTLIST structure.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include "pc_api_internal.h"
#include <assert.h>

PCPOINTLIST *pc_pointlist_make(uint32_t npoints)
{
  PCPOINTLIST *pl = pcalloc(sizeof(PCPOINTLIST));
  pl->points = pcalloc(sizeof(PCPOINT *) * npoints);
  pl->maxpoints = npoints;
  pl->npoints = 0;
  pl->mem = NULL;
  return pl;
}

void pc_pointlist_free(PCPOINTLIST *pl)
{
  int i;
  for (i = 0; i < pl->npoints; i++)
  {
    pc_point_free(pl->points[i]);
  }
  if (pl->mem)
    pcfree(pl->mem);
  pcfree(pl->points);
  pcfree(pl);
  return;
}

void pc_pointlist_add_point(PCPOINTLIST *pl, PCPOINT *pt)
{
  if (pl->npoints >= pl->maxpoints)
  {
    if (pl->maxpoints < 1)
      pl->maxpoints = 1;
    pl->maxpoints *= 2;
    pl->points = pcrealloc(pl->points, pl->maxpoints * sizeof(PCPOINT *));
  }

  pl->points[pl->npoints] = pt;
  pl->npoints += 1;
  return;
}

PCPOINT *pc_pointlist_get_point(const PCPOINTLIST *pl, int i)
{
  return pl->points[i];
}

PCPOINTLIST *pc_pointlist_from_dimensional(const PCPATCH_DIMENSIONAL *pdl)
{
  PCPOINTLIST *pl;
  PCPATCH_DIMENSIONAL *pdl_uncompressed;
  const PCSCHEMA *schema = pdl->schema;
  int i, j, ndims, npoints;
  uint8_t *data;
  assert(pdl);

  pdl_uncompressed = pc_patch_dimensional_decompress(pdl);

  ndims = schema->ndims;
  npoints = pdl->npoints;
  pl = pc_pointlist_make(npoints);
  pl->mem = data = pcalloc(npoints * schema->size);

  for (i = 0; i < npoints; i++)
  {
    PCPOINT *pt = pc_point_from_data(schema, data);
    for (j = 0; j < ndims; j++)
    {
      PCDIMENSION *dim = pc_schema_get_dimension(schema, j);

      uint8_t *in = pdl_uncompressed->bytes[j].bytes + dim->size * i;
      uint8_t *out = data + dim->byteoffset;
      memcpy(out, in, dim->size);
    }
    pc_pointlist_add_point(pl, pt);
    data += schema->size;
  }
  pc_patch_dimensional_free(pdl_uncompressed);

  return pl;
}

PCPOINTLIST *pc_pointlist_from_uncompressed(const PCPATCH_UNCOMPRESSED *patch)
{
  int i;
  PCPOINTLIST *pl;
  size_t pt_size = patch->schema->size;
  uint32_t npoints = patch->npoints;

  pl = pc_pointlist_make(npoints);
  for (i = 0; i < npoints; i++)
  {
    pc_pointlist_add_point(
        pl, pc_point_from_data(patch->schema, patch->data + i * pt_size));
  }
  return pl;
}

PCPOINTLIST *pc_pointlist_from_patch(const PCPATCH *patch)
{
  switch (patch->type)
  {
  case PC_NONE:
  {
    return pc_pointlist_from_uncompressed((PCPATCH_UNCOMPRESSED *)patch);
  }
  case PC_DIMENSIONAL:
  {
    return pc_pointlist_from_dimensional((PCPATCH_DIMENSIONAL *)patch);
  }
  case PC_LAZPERF:
  {
    return pc_pointlist_from_lazperf((PCPATCH_LAZPERF *)patch);
  }
  }

  /* Don't get here */
  pcerror("pc_pointlist_from_patch: unsupported compression type %d",
          patch->type);
  return NULL;
}
