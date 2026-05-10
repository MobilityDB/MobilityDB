/***********************************************************************
 * pc_sort.c
 *
 *  Pointclound patch sorting.
 *
 *  Copyright (c) 2016 IGN
 *
 *  Author: M. Brédif
 *
 ***********************************************************************/
#include "pc_api_internal.h"
#include "sort_r/sort_r.h"
#include <assert.h>

// NULL terminated array of PCDIMENSION pointers
typedef PCDIMENSION **PCDIMENSION_LIST;

/**
 * Comparators
 */

int pc_compare_dim(const void *a, const void *b, void *arg)
{
  PCDIMENSION_LIST dim = (PCDIMENSION_LIST)arg;
  uint32_t byteoffset = dim[0]->byteoffset;
  uint32_t interpretation = dim[0]->interpretation;
  double da = pc_double_from_ptr(a + byteoffset, interpretation);
  double db = pc_double_from_ptr(b + byteoffset, interpretation);
  int cmp = ((da > db) - (da < db));
  return (cmp == 0 && dim[1]) ? pc_compare_dim(a, b, dim + 1) : cmp;
}

int pc_compare_pcb(const void *a, const void *b, const void *arg)
{
  PCBYTES *pcb = (PCBYTES *)arg;
  double da = pc_double_from_ptr(a, pcb->interpretation);
  double db = pc_double_from_ptr(b, pcb->interpretation);
  return ((da > db) - (da < db));
}

/**
 * Sort
 */

PCPATCH_UNCOMPRESSED *pc_patch_uncompressed_sort(const PCPATCH_UNCOMPRESSED *pu,
                                                 PCDIMENSION_LIST dim)
{
  PCPATCH_UNCOMPRESSED *spu =
      pc_patch_uncompressed_make(pu->schema, pu->npoints);

  memcpy(spu->data, pu->data, pu->datasize);
  spu->npoints = pu->npoints;
  spu->bounds = pu->bounds;
  spu->stats = pc_stats_clone(pu->stats);

  sort_r(spu->data, spu->npoints, pu->schema->size, pc_compare_dim, dim);

  return spu;
}

PCDIMENSION_LIST pc_schema_get_dimensions_by_name(const PCSCHEMA *schema,
                                                  const char **name, int ndims)
{
  PCDIMENSION_LIST dim = pcalloc((ndims + 1) * sizeof(PCDIMENSION *));
  int i;
  for (i = 0; i < ndims; ++i)
  {
    dim[i] = pc_schema_get_dimension_by_name(schema, name[i]);
    if (!dim[i])
    {
      pcerror("dimension \"%s\" does not exist", name[i]);
      return NULL;
    }
    assert(dim[i]->scale > 0);
  }
  dim[ndims] = NULL;
  return dim;
}

PCPATCH *pc_patch_sort(const PCPATCH *pa, const char **name, int ndims)
{
  PCDIMENSION_LIST dim =
      pc_schema_get_dimensions_by_name(pa->schema, name, ndims);
  PCPATCH *pu = pc_patch_uncompress(pa);
  if (!pu)
  {
    pcfree(dim);
    pcerror("Patch uncompression failed");
    return NULL;
  }
  PCPATCH_UNCOMPRESSED *ps =
      pc_patch_uncompressed_sort((PCPATCH_UNCOMPRESSED *)pu, dim);

  pcfree(dim);
  if (pu != pa)
    pc_patch_free(pu);
  return (PCPATCH *)ps;
}

/**
 * IsSorted
 */

uint32_t pc_patch_uncompressed_is_sorted(const PCPATCH_UNCOMPRESSED *pu,
                                         PCDIMENSION_LIST dim, char strict)
{
  size_t size = pu->schema->size;
  uint8_t *buf = pu->data, *last = pu->data + pu->datasize - size;
  while (buf < last)
  {
    if (pc_compare_dim(buf, buf + size, dim) >= strict)
      return PC_FALSE;
    buf += size;
  }
  return PC_TRUE;
}

uint32_t pc_bytes_uncompressed_is_sorted(const PCBYTES *pcb, char strict)
{
  assert(pcb->compression == PC_DIM_NONE);
  size_t size = pc_interpretation_size(pcb->interpretation);
  uint8_t *buf = pcb->bytes;
  uint8_t *last = buf + pcb->size - size;
  while (buf < last)
  {
    if (pc_compare_pcb(buf, buf + size, pcb) >= strict)
      return PC_FALSE;
    buf += size;
  }
  return PC_TRUE;
}

uint32_t pc_bytes_sigbits_is_sorted(const PCBYTES *pcb, char strict)
{
  assert(pcb->compression == PC_DIM_SIGBITS);
  pcinfo("%s not implemented, decoding", __func__);
  PCBYTES dpcb = pc_bytes_decode(*pcb);
  uint32_t is_sorted = pc_bytes_uncompressed_is_sorted(&dpcb, strict);
  pc_bytes_free(dpcb);
  return is_sorted;
}

uint32_t pc_bytes_zlib_is_sorted(const PCBYTES *pcb, char strict)
{
  assert(pcb->compression == PC_DIM_ZLIB);
  pcinfo("%s not implemented, decoding", __func__);
  PCBYTES dpcb = pc_bytes_decode(*pcb);
  uint32_t is_sorted = pc_bytes_uncompressed_is_sorted(&dpcb, strict);
  pc_bytes_free(dpcb);
  return is_sorted;
}

uint32_t pc_bytes_run_length_is_sorted(const PCBYTES *pcb, char strict)
{
  assert(pcb->compression == PC_DIM_RLE);
  uint8_t run;
  size_t size = pc_interpretation_size(pcb->interpretation);
  const uint8_t *bytes_rle_curr_val = pcb->bytes + 1;
  const uint8_t *bytes_rle_next_val = pcb->bytes + 2 + size;
  const uint8_t *bytes_rle_end_val = pcb->bytes + pcb->size - size;
  while (bytes_rle_next_val < bytes_rle_end_val)
  {
    run = bytes_rle_curr_val[-1];
    assert(run > 0);
    if (pc_compare_pcb(bytes_rle_curr_val, bytes_rle_next_val, pcb) >=
            strict              // value comparison
        || (strict && run > 1)) // run_length should be 1 if strict
      return PC_FALSE;
    bytes_rle_curr_val = bytes_rle_next_val;
    bytes_rle_next_val += 1 + size;
  }
  return PC_TRUE;
}

uint32_t pc_patch_dimensional_is_sorted(const PCPATCH_DIMENSIONAL *pdl,
                                        PCDIMENSION_LIST dim, char strict)
{
  assert(pdl);
  assert(pdl->schema);

  // uncompress when checking multiple dimensions
  if (dim[1])
  {
    PCPATCH_UNCOMPRESSED *pu = pc_patch_uncompressed_from_dimensional(pdl);
    if (!pu)
    {
      pcerror("Patch uncompression failed");
      return PC_FAILURE - 1; // aliasing issue : PC_FALSE == PC_FAILURE...
    }
    uint32_t is_sorted = pc_patch_uncompressed_is_sorted(pu, dim, strict);
    pc_patch_free((PCPATCH *)pu);
    return is_sorted;
  }

  PCBYTES *pcb = pdl->bytes + dim[0]->position;
  switch (pcb->compression)
  {
  case PC_DIM_RLE:
  {
    return pc_bytes_run_length_is_sorted(pcb, strict);
  }
  case PC_DIM_SIGBITS:
  {
    return pc_bytes_sigbits_is_sorted(pcb, strict);
  }
  case PC_DIM_ZLIB:
  {
    return pc_bytes_zlib_is_sorted(pcb, strict);
  }
  case PC_DIM_NONE:
  {
    return pc_bytes_uncompressed_is_sorted(pcb, strict);
  }
  default:
  {
    pcerror("%s: Uh oh", __func__);
  }
  }
  return PC_FAILURE - 1; // aliasing issue : PC_FALSE == PC_FAILURE...
}

uint32_t pc_patch_lazperf_is_sorted(const PCPATCH_LAZPERF *pa,
                                    PCDIMENSION_LIST dim, char strict)
{
  PCPATCH_UNCOMPRESSED *pu = pc_patch_uncompressed_from_lazperf(pa);
  if (!pu)
  {
    pcerror("Patch uncompression failed");
    return PC_FAILURE - 1; // aliasing issue : PC_FALSE == PC_FAILURE...
  }
  uint32_t is_sorted = pc_patch_uncompressed_is_sorted(pu, dim, strict);
  pc_patch_free((PCPATCH *)pu);
  return is_sorted;
}

uint32_t pc_patch_is_sorted(const PCPATCH *pa, const char **name, int ndims,
                            char strict)
{
  int is_sorted = PC_FAILURE - 1; // aliasing issue : PC_FALSE == PC_FAILURE...
  PCDIMENSION_LIST dim =
      pc_schema_get_dimensions_by_name(pa->schema, name, ndims);
  if (!dim)
    return is_sorted;
  strict = (strict > 0); // ensure 0-1 value

  switch (pa->type)
  {
  case PC_NONE:
    is_sorted = pc_patch_uncompressed_is_sorted((PCPATCH_UNCOMPRESSED *)pa, dim,
                                                strict);
    break;
  case PC_DIMENSIONAL:
    is_sorted =
        pc_patch_dimensional_is_sorted((PCPATCH_DIMENSIONAL *)pa, dim, strict);
    break;
  case PC_LAZPERF:
    is_sorted = pc_patch_lazperf_is_sorted((PCPATCH_LAZPERF *)pa, dim, strict);
    break;
  default:
    pcerror("%s: unsupported compression %d requested", __func__, pa->type);
  }
  pcfree(dim);
  return is_sorted;
}
