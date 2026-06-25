/***********************************************************************
 * pc_patch.c
 *
 *  Pointclound patch handling. Create, get and set values from the
 *  basic PCPATCH structure.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include "pc_api_internal.h"
#include <assert.h>
#include <math.h>

int pc_patch_compute_extent(PCPATCH *pa)
{
  if (!pa)
    return PC_FAILURE;
  switch (pa->type)
  {
  case PC_NONE:
    return pc_patch_uncompressed_compute_extent((PCPATCH_UNCOMPRESSED *)pa);
  case PC_DIMENSIONAL:
    return pc_patch_dimensional_compute_extent((PCPATCH_DIMENSIONAL *)pa);
  case PC_LAZPERF:
    return pc_patch_lazperf_compute_extent((PCPATCH_LAZPERF *)pa);
  }
  return PC_FAILURE;
}

/**
 * Calculate or re-calculate statistics for a patch.
 */
int pc_patch_compute_stats(PCPATCH *pa)
{
  if (!pa)
    return PC_FAILURE;

  switch (pa->type)
  {
  case PC_NONE:
    return pc_patch_uncompressed_compute_stats((PCPATCH_UNCOMPRESSED *)pa);

  case PC_DIMENSIONAL:
  {
    PCPATCH_UNCOMPRESSED *pu =
        pc_patch_uncompressed_from_dimensional((PCPATCH_DIMENSIONAL *)pa);
    pc_patch_uncompressed_compute_stats(pu);
    pa->stats = pu->stats;
    pu->stats = NULL;
    pc_patch_uncompressed_free(pu);
    return PC_SUCCESS;
  }
  case PC_LAZPERF:
  {
    PCPATCH_UNCOMPRESSED *pu =
        pc_patch_uncompressed_from_lazperf((PCPATCH_LAZPERF *)pa);
    pc_patch_uncompressed_compute_stats(pu);
    pa->stats = pc_stats_clone(pu->stats);
    pc_patch_uncompressed_free(pu);
    return PC_SUCCESS;
  }
  default:
  {
    pcerror("%s: unknown compression type", __func__, pa->type);
    return PC_FAILURE;
  }
  }

  return PC_FAILURE;
}

void pc_patch_free_stats(PCPATCH *patch)
{
  if (patch->stats)
  {
    pc_stats_free(patch->stats);
    patch->stats = NULL;
  }
}

void pc_patch_free(PCPATCH *patch)
{
  switch (patch->type)
  {
  case PC_NONE:
  {
    pc_patch_uncompressed_free((PCPATCH_UNCOMPRESSED *)patch);
    break;
  }
  case PC_DIMENSIONAL:
  {
    pc_patch_dimensional_free((PCPATCH_DIMENSIONAL *)patch);
    break;
  }
  case PC_LAZPERF:
  {
    pc_patch_lazperf_free((PCPATCH_LAZPERF *)patch);
    break;
  }
  default:
  {
    pcerror("%s: unknown compression type %d", __func__, patch->type);
    break;
  }
  }
}

PCPATCH *pc_patch_from_pointlist(const PCPOINTLIST *ptl)
{
  return (PCPATCH *)pc_patch_uncompressed_from_pointlist(ptl);
}

PCPATCH *pc_patch_compress(const PCPATCH *patch, void *userdata)
{
  uint32_t schema_compression = patch->schema->compression;
  uint32_t patch_compression = patch->type;

  switch (schema_compression)
  {
  case PC_DIMENSIONAL:
  {
    if (patch_compression == PC_NONE)
    {
      /* Dimensionalize, dimensionally compress, return */
      PCPATCH_DIMENSIONAL *pcdu =
          pc_patch_dimensional_from_uncompressed((PCPATCH_UNCOMPRESSED *)patch);
      if (!pcdu)
      {
        pcerror("%s: dimensional compression failed", __func__);
      }

      PCPATCH_DIMENSIONAL *pcdd =
          pc_patch_dimensional_compress(pcdu, (PCDIMSTATS *)userdata);
      pc_patch_dimensional_free(pcdu);
      return (PCPATCH *)pcdd;
    }
    else if (patch_compression == PC_DIMENSIONAL)
    {
      /* Make sure it's compressed, return */
      return (PCPATCH *)pc_patch_dimensional_compress(
          (PCPATCH_DIMENSIONAL *)patch, (PCDIMSTATS *)userdata);
    }
    else if (patch_compression == PC_LAZPERF)
    {
      PCPATCH_UNCOMPRESSED *pcu =
          pc_patch_uncompressed_from_lazperf((PCPATCH_LAZPERF *)patch);
      PCPATCH_DIMENSIONAL *pal = pc_patch_dimensional_from_uncompressed(pcu);
      PCPATCH_DIMENSIONAL *palc = pc_patch_dimensional_compress(pal, NULL);
      pc_patch_dimensional_free(pal);
      return (PCPATCH *)palc;
    }
    else
    {
      pcerror("%s: unknown patch compression type %d", __func__,
              patch_compression);
    }
  }
  case PC_NONE:
  {
    if (patch_compression == PC_NONE)
    {
      return (PCPATCH *)patch;
    }
    else if (patch_compression == PC_DIMENSIONAL)
    {
      PCPATCH_UNCOMPRESSED *pcu =
          pc_patch_uncompressed_from_dimensional((PCPATCH_DIMENSIONAL *)patch);
      return (PCPATCH *)pcu;
    }
    else if (patch_compression == PC_LAZPERF)
    {
      PCPATCH_UNCOMPRESSED *pcu =
          pc_patch_uncompressed_from_lazperf((PCPATCH_LAZPERF *)patch);
      return (PCPATCH *)pcu;
    }
    else
    {
      pcerror("%s: unknown patch compression type %d", __func__,
              patch_compression);
    }
  }
  case PC_LAZPERF:
  {
    if (patch_compression == PC_NONE)
    {
      PCPATCH_LAZPERF *pgc =
          pc_patch_lazperf_from_uncompressed((PCPATCH_UNCOMPRESSED *)patch);
      if (!pgc)
        pcerror("%s: lazperf compression failed", __func__);
      return (PCPATCH *)pgc;
    }
    else if (patch_compression == PC_DIMENSIONAL)
    {
      PCPATCH_UNCOMPRESSED *pad =
          pc_patch_uncompressed_from_dimensional((PCPATCH_DIMENSIONAL *)patch);
      PCPATCH_LAZPERF *pal =
          pc_patch_lazperf_from_uncompressed((PCPATCH_UNCOMPRESSED *)pad);
      pc_patch_uncompressed_free(pad);
      return (PCPATCH *)pal;
    }
    else if (patch_compression == PC_LAZPERF)
    {
      return (PCPATCH *)patch;
    }
    else
    {
      pcerror("%s: unknown patch compression type %d", __func__,
              patch_compression);
    }
  }
  default:
  {
    pcerror("%s: unknown schema compression type %d", __func__,
            schema_compression);
  }
  }

  pcerror("%s: fatal error", __func__);
  return NULL;
}

PCPATCH *pc_patch_uncompress(const PCPATCH *patch)
{
  uint32_t patch_compression = patch->type;

  if (patch_compression == PC_DIMENSIONAL)
  {
    PCPATCH_UNCOMPRESSED *pu =
        pc_patch_uncompressed_from_dimensional((PCPATCH_DIMENSIONAL *)patch);
    return (PCPATCH *)pu;
  }

  if (patch_compression == PC_NONE)
  {
    return (PCPATCH *)patch;
  }

  if (patch_compression == PC_LAZPERF)
  {
    PCPATCH_UNCOMPRESSED *pu =
        pc_patch_uncompressed_from_lazperf((PCPATCH_LAZPERF *)patch);
    return (PCPATCH *)pu;
  }

  return NULL;
}

PCPATCH *pc_patch_from_wkb(const PCSCHEMA *s, uint8_t *wkb, size_t wkbsize)
{
  /*
  byte:	  endianness (1 = NDR, 0 = XDR)
  uint32:   pcid (key to POINTCLOUD_SCHEMAS)
  uint32:   compression (0 = no compression, 1 = dimensional, 2 = lazperf)
  uchar[]:  data (interpret relative to pcid and compression)
  */
  uint32_t compression, pcid;
  PCPATCH *patch;

  if (!wkbsize)
  {
    pcerror("%s: zero length wkb", __func__);
  }

  /*
   * It is possible for the WKB compression to be different from the
   * schema compression at this point. The schema compression is only
   * forced at serialization time.
   */
  pcid = pc_wkb_get_pcid(wkb);
  compression = wkb_get_compression(wkb);

  if (pcid != s->pcid)
  {
    pcerror("%s: wkb pcid (%d) not consistent with schema pcid (%d)", __func__,
            pcid, s->pcid);
  }

  switch (compression)
  {
  case PC_NONE:
  {
    patch = pc_patch_uncompressed_from_wkb(s, wkb, wkbsize);
    break;
  }
  case PC_DIMENSIONAL:
  {
    patch = pc_patch_dimensional_from_wkb(s, wkb, wkbsize);
    break;
  }
  case PC_LAZPERF:
  {
    patch = pc_patch_lazperf_from_wkb(s, wkb, wkbsize);
    break;
  }
  default:
  {
    /* Don't get here */
    pcerror("%s: unknown compression '%d' requested", __func__, compression);
    return NULL;
  }
  }

  if (PC_FAILURE == pc_patch_compute_extent(patch))
    pcerror("%s: pc_patch_compute_extent failed", __func__);

  if (PC_FAILURE == pc_patch_compute_stats(patch))
    pcerror("%s: pc_patch_compute_stats failed", __func__);

  return patch;
}

uint8_t *pc_patch_to_wkb(const PCPATCH *patch, size_t *wkbsize)
{
  /*
  byte:	  endianness (1 = NDR, 0 = XDR)
  uint32:   pcid (key to POINTCLOUD_SCHEMAS)
  uint32:   compression (0 = no compression, 1 = dimensional, 2 = lazperf)
  uchar[]:  data (interpret relative to pcid and compression)
  */
  switch (patch->type)
  {
  case PC_NONE:
  {
    return pc_patch_uncompressed_to_wkb((PCPATCH_UNCOMPRESSED *)patch, wkbsize);
  }
  case PC_DIMENSIONAL:
  {
    return pc_patch_dimensional_to_wkb((PCPATCH_DIMENSIONAL *)patch, wkbsize);
  }
  case PC_LAZPERF:
  {
    return pc_patch_lazperf_to_wkb((PCPATCH_LAZPERF *)patch, wkbsize);
  }
  }
  pcerror("%s: unknown compression requested '%d'", __func__,
          patch->schema->compression);
  return NULL;
}

char *pc_patch_to_string(const PCPATCH *patch)
{
  switch (patch->type)
  {
  case PC_NONE:
    return pc_patch_uncompressed_to_string((PCPATCH_UNCOMPRESSED *)patch);
  case PC_DIMENSIONAL:
    return pc_patch_dimensional_to_string((PCPATCH_DIMENSIONAL *)patch);
  case PC_LAZPERF:
    return pc_patch_lazperf_to_string((PCPATCH_LAZPERF *)patch);
  }
  pcerror("%s: unsupported compression %d requested", __func__, patch->type);
  return NULL;
}

PCPATCH *pc_patch_from_patchlist(PCPATCH **palist, int numpatches)
{
  int i;
  uint32_t totalpoints = 0;
  PCPATCH_UNCOMPRESSED *paout;
  const PCSCHEMA *schema = NULL;
  uint8_t *buf;

  assert(palist);
  assert(numpatches);

  /* All schemas better be the same... */
  schema = palist[0]->schema;

  /* How many points will this output have? */
  for (i = 0; i < numpatches; i++)
  {
    if (schema->pcid != palist[i]->schema->pcid)
    {
      pcerror("%s: inconsistent schemas in input", __func__);
      return NULL;
    }
    totalpoints += palist[i]->npoints;
  }

  /* Blank output */
  paout = pc_patch_uncompressed_make(schema, totalpoints);
  buf = paout->data;

  /* Uncompress dimensionals, copy uncompressed */
  for (i = 0; i < numpatches; i++)
  {
    const PCPATCH *pa = palist[i];

    /* Update bounds */
    pc_bounds_merge(&(paout->bounds), &(pa->bounds));

    switch (pa->type)
    {
    case PC_DIMENSIONAL:
    {
      PCPATCH_UNCOMPRESSED *pu = pc_patch_uncompressed_from_dimensional(
          (const PCPATCH_DIMENSIONAL *)pa);
      size_t sz = pu->schema->size * pu->npoints;
      memcpy(buf, pu->data, sz);
      buf += sz;
      pc_patch_free((PCPATCH *)pu);
      break;
    }
    case PC_NONE:
    {
      PCPATCH_UNCOMPRESSED *pu = (PCPATCH_UNCOMPRESSED *)pa;
      size_t sz = pu->schema->size * pu->npoints;
      memcpy(buf, pu->data, sz);
      buf += sz;
      break;
    }
    case PC_LAZPERF:
    {
      PCPATCH_UNCOMPRESSED *pu =
          pc_patch_uncompressed_from_lazperf((const PCPATCH_LAZPERF *)pa);
      size_t sz = pu->schema->size * pu->npoints;
      memcpy(buf, pu->data, sz);
      buf += sz;
      pc_patch_uncompressed_free(pu);
      break;
    }
    default:
    {
      pcerror("%s: unknown compression type (%d)", __func__, pa->type);
      break;
    }
    }
  }

  paout->npoints = totalpoints;

  if (PC_FAILURE == pc_patch_uncompressed_compute_stats(paout))
  {
    pcerror("%s: stats computation failed", __func__);
    return NULL;
  }

  return (PCPATCH *)paout;
}

// first: the first element to select (1-based indexing)
// count: the number of points to select
PCPATCH *pc_patch_range(const PCPATCH *pa, int first, int count)
{
  PCPATCH_UNCOMPRESSED *paout, *pu;
  int countmax;
  uint8_t *buf;
  size_t size;
  size_t start;

  assert(pa);

  first--;
  countmax = pa->npoints - first;

  if (count > countmax)
    count = countmax;

  if (first < 0 || count <= 0)
    return NULL;

  if (count == pa->npoints)
    return (PCPATCH *)pa;

  paout = pc_patch_uncompressed_make(pa->schema, count);
  if (!paout)
    return NULL;
  paout->npoints = count;

  pu = (PCPATCH_UNCOMPRESSED *)pc_patch_uncompress(pa);
  if (!pu)
  {
    pc_patch_free((PCPATCH *)paout);
    return NULL;
  }

  buf = paout->data;
  start = pa->schema->size * first;
  size = pa->schema->size * count;

  memcpy(buf, pu->data + start, size);

  if (((PCPATCH *)pu) != pa)
    pc_patch_free((PCPATCH *)pu);

  if (PC_FAILURE == pc_patch_uncompressed_compute_extent(paout))
  {
    pcerror("%s: extent computation failed", __func__);
    pc_patch_free((PCPATCH *)paout);
    return NULL;
  }
  if (PC_FAILURE == pc_patch_uncompressed_compute_stats(paout))
  {
    pcerror("%s: stats computation failed", __func__);
    pc_patch_free((PCPATCH *)paout);
    return NULL;
  }

  return (PCPATCH *)paout;
}

/** get point n from patch */
/** positive 1-based:  1=first point,  npoints=last  point */
/** negative 1-based: -1=last  point, -npoints=first point */
PCPOINT *pc_patch_pointn(const PCPATCH *patch, int n)
{
  if (!patch)
    return NULL;
  if (n < 0)
    n = patch->npoints + n; // negative indices count a backward
  else
    --n; // 1-based => 0-based indexing
  if (n < 0 || n >= patch->npoints)
    return NULL;

  switch (patch->type)
  {
  case PC_NONE:
    return pc_patch_uncompressed_pointn((PCPATCH_UNCOMPRESSED *)patch, n);
  case PC_DIMENSIONAL:
    return pc_patch_dimensional_pointn((PCPATCH_DIMENSIONAL *)patch, n);
  case PC_LAZPERF:
    return pc_patch_lazperf_pointn((PCPATCH_LAZPERF *)patch, n);
  }
  pcerror("%s: unsupported compression %d requested", __func__, patch->type);
  return NULL;
}

static void pc_patch_point_set(PCPOINT *p, const uint8_t *data,
                               PCDIMENSION **dims, const uint8_t *def)
{
  size_t i;
  for (i = 0; i < p->schema->ndims; i++)
  {
    const PCDIMENSION *ddim = dims[i];
    const PCDIMENSION *pdim = p->schema->dims[i];
    uint8_t *pdata = p->data + pdim->byteoffset;
    const uint8_t *ddata =
        ddim ? data + ddim->byteoffset : def + pdim->byteoffset;
    memcpy(pdata, ddata, pdim->size);
  }
}

/** set schema for patch */
PCPATCH *pc_patch_set_schema(PCPATCH *patch, const PCSCHEMA *new_schema,
                             double def)
{
  PCDIMENSION **new_dimensions = new_schema->dims;
  PCDIMENSION *old_dimensions[new_schema->ndims];
  const PCSCHEMA *old_schema = patch->schema;
  PCPATCH_UNCOMPRESSED *paout;
  PCPOINT opt, npt;
  PCPATCH *pain;
  PCPOINT *dpt;
  size_t i, j;

  // create a point for storing the default values
  dpt = pc_point_make(new_schema);

  for (j = 0; j < new_schema->ndims; j++)
  {
    PCDIMENSION *ndim = new_dimensions[j];
    PCDIMENSION *odim = pc_schema_get_dimension_by_name(old_schema, ndim->name);
    old_dimensions[j] = odim;
    if (odim)
    {
      if (ndim->interpretation != odim->interpretation)
      {
        pcerror("dimension interpretations are not matching");
        pc_point_free(dpt);
        return NULL;
      }
    }
    else
    {
      pc_point_set_double(dpt, ndim, def);
    }
  }

  pain = pc_patch_uncompress(patch);
  paout = pc_patch_uncompressed_make(new_schema, patch->npoints);
  paout->npoints = pain->npoints;

  opt.schema = old_schema;
  npt.schema = new_schema;
  opt.readonly = PC_TRUE;
  npt.readonly = PC_TRUE;

  opt.data = ((PCPATCH_UNCOMPRESSED *)pain)->data;
  npt.data = paout->data;

  for (i = 0; i < patch->npoints; i++)
  {
    pc_patch_point_set(&npt, opt.data, old_dimensions, dpt->data);
    opt.data += old_schema->size;
    npt.data += new_schema->size;
  }

  if (patch->stats)
  {
    paout->stats = pc_stats_new(new_schema);

    opt.data = patch->stats->min.data;
    npt.data = paout->stats->min.data;
    pc_patch_point_set(&npt, opt.data, old_dimensions, dpt->data);

    opt.data = patch->stats->max.data;
    npt.data = paout->stats->max.data;
    pc_patch_point_set(&npt, opt.data, old_dimensions, dpt->data);

    opt.data = patch->stats->avg.data;
    npt.data = paout->stats->avg.data;
    pc_patch_point_set(&npt, opt.data, old_dimensions, dpt->data);

    pc_point_get_x(&paout->stats->min, &paout->bounds.xmin);
    pc_point_get_y(&paout->stats->min, &paout->bounds.ymin);
    pc_point_get_x(&paout->stats->max, &paout->bounds.xmax);
    pc_point_get_y(&paout->stats->max, &paout->bounds.ymax);
  }
  else
  {
    double xscale = npt.schema->xdim->scale / opt.schema->xdim->scale;
    double yscale = npt.schema->ydim->scale / opt.schema->ydim->scale;
    double xoffset = npt.schema->xdim->offset - opt.schema->xdim->offset;
    double yoffset = npt.schema->ydim->offset - opt.schema->ydim->offset;

    paout->bounds.xmin = patch->bounds.xmin * xscale + xoffset;
    paout->bounds.xmax = patch->bounds.xmax * xscale + xoffset;
    paout->bounds.ymin = patch->bounds.ymin * yscale + yoffset;
    paout->bounds.xmax = patch->bounds.ymax * yscale + yoffset;
  }

  pc_point_free(dpt);

  if (pain != patch)
    pc_patch_free(pain);

  return (PCPATCH *)paout;
}

/**
 * Read all the points from "patch", and transform them based on "new_schema".
 * Return a new patch with the transformed points.
 */
PCPATCH *pc_patch_transform(const PCPATCH *patch, const PCSCHEMA *new_schema,
                            double def)
{
  PCDIMENSION **new_dimensions = new_schema->dims;
  PCDIMENSION *old_dimensions[new_schema->ndims];
  const PCSCHEMA *old_schema = patch->schema;
  PCPATCH_UNCOMPRESSED *paout;
  PCPOINT opt, npt;
  PCPATCH *pain;
  size_t i, j;

  if (old_schema->srid != new_schema->srid)
  {
    pcwarn("old and new schemas have different srids, and data "
           "reprojection is not yet supported");
    return NULL;
  }

  for (j = 0; j < new_schema->ndims; j++)
  {
    PCDIMENSION *ndim = new_dimensions[j];
    PCDIMENSION *odim = pc_schema_get_dimension_by_name(old_schema, ndim->name);
    old_dimensions[j] = odim;
  }

  pain = pc_patch_uncompress(patch);

  paout = pc_patch_uncompressed_make(new_schema, patch->npoints);
  paout->npoints = pain->npoints;

  opt.schema = old_schema;
  npt.schema = new_schema;
  opt.readonly = PC_TRUE;
  npt.readonly = PC_TRUE;

  opt.data = ((PCPATCH_UNCOMPRESSED *)pain)->data;
  npt.data = paout->data;

  // reinterpret the data and fill the output patch
  //
  // TODO: for the case where the old and new dimension sets don't intersect
  // (all the values in old_dimensions are NULL) a faster path could probably
  // be used
  for (i = 0; i < patch->npoints; i++)
  {
    for (j = 0; j < new_schema->ndims; j++)
    {
      // pc_point_get_double returns immediately w/o changing val if the
      // dimension it is passed is NULL
      double val = def;
      pc_point_get_double(&opt, old_dimensions[j], &val);
      pc_point_set_double(&npt, new_dimensions[j], val);
    }

    opt.data += old_schema->size;
    npt.data += new_schema->size;
  }

  if (pain != patch)
    pc_patch_free(pain);

  if (PC_FAILURE == pc_patch_uncompressed_compute_extent(paout))
  {
    pcerror("%s: failed to compute patch extent", __func__);
    pc_patch_free((PCPATCH *)paout);
    return NULL;
  }

  if (PC_FAILURE == pc_patch_uncompressed_compute_stats(paout))
  {
    pcerror("%s: failed to compute patch stats", __func__);
    pc_patch_free((PCPATCH *)paout);
    return NULL;
  }

  return (PCPATCH *)paout;
}
