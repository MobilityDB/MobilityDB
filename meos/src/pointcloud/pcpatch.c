/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Opaque byte-level helpers for the pgpointcloud `pcpatch` base type.
 * Mirrors pcpoint.c — see that file for the design rationale.
 */

#include "pointcloud/pcpatch.h"

/* C */
#include <assert.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include "common/hashfn.h"
#include "utils/builtins.h"
#include <stddef.h>          /* offsetof */
/* MEOS */
#include <meos.h>

/*****************************************************************************
 * Struct-tail padding
 *
 * Same phenomenon as pcpoint.c — pgpointcloud's @c pc_patch_serialize*
 * allocates @c sizeof(SERIALIZED_PATCH) - 1 + <packed-points size> and
 * leaves the struct-tail padding uninitialized. For SERIALIZED_PATCH:
 *
 *   { uint32_t size; uint32_t pcid; uint32_t compression;
 *     uint32_t npoints; PCBOUNDS bounds; uint8_t data[1]; }
 *
 * PCBOUNDS is 4 doubles (alignment 8), so @c data[1] sits at offset 48
 * and the struct rounds to 56 — 7 bytes of tail padding on x86_64.
 * Truncate @c VARSIZE by that amount for cmp/hash.
 *****************************************************************************/

typedef struct
{
  int32 vl_len_;
  uint32_t pcid;
  uint32_t compression;
  uint32_t npoints;
  double bounds[4];  /* matches upstream PCBOUNDS */
  uint8_t data[1];
} PcpatchLayoutShadow;

#define PCPATCH_TAIL_PADDING \
  (sizeof(PcpatchLayoutShadow) - offsetof(PcpatchLayoutShadow, data) - 1)

static inline size_t
pcpatch_meaningful_size(const Pcpatch *pa)
{
  size_t sz = VARSIZE(pa);
  /* header + pcid + compression + npoints + 4 bounds doubles */
  size_t hdr = VARHDRSZ + 3 * sizeof(uint32_t) + 4 * sizeof(double);
  return (sz > hdr + PCPATCH_TAIL_PADDING) ? (sz - PCPATCH_TAIL_PADDING) : sz;
}

/*****************************************************************************
 * Validity helpers
 *****************************************************************************/

bool
ensure_same_pcid_pcpatch(const Pcpatch *pa1, const Pcpatch *pa2)
{
  assert(pa1); assert(pa2);
  if (pa1->pcid != pa2->pcid)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on pcpatch values with different schemas: %u vs %u",
      pa1->pcid, pa2->pcid);
    return false;
  }
  return true;
}

/*****************************************************************************
 * Input/output
 *****************************************************************************/

Pcpatch *
pcpatch_parse(const char **str, bool end)
{
  const char *type_str = "pcpatch";
  const char *p = *str;
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;

  const char *hex_start = p;
  while ((*p >= '0' && *p <= '9') ||
         (*p >= 'a' && *p <= 'f') ||
         (*p >= 'A' && *p <= 'F'))
    p++;
  size_t hex_len = p - hex_start;
  if (hex_len == 0 || (hex_len % 2) != 0)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: empty or odd-length hex", type_str);
    return NULL;
  }
  size_t byte_len = hex_len / 2;
  /* Header minimum: varlena + pcid + compression + npoints + 4 bounds doubles */
  size_t min_hdr = VARHDRSZ + 3 * sizeof(uint32_t) + 4 * sizeof(double);
  if (byte_len < min_hdr)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: too short", type_str);
    return NULL;
  }

  Pcpatch *result = palloc(byte_len);
  if (hex_decode(hex_start, hex_len, (char *) result) != byte_len)
  {
    pfree(result);
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: hex decode failed", type_str);
    return NULL;
  }
  SET_VARSIZE(result, byte_len);

  *str = p;
  if (end)
  {
    while (**str == ' ' || **str == '\t' || **str == '\n' || **str == '\r')
      (*str)++;
    if (**str != '\0')
    {
      pfree(result);
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "Could not parse %s value: trailing data", type_str);
      return NULL;
    }
  }
  return result;
}

Pcpatch *
pcpatch_hex_in(const char *str)
{
  if (! str)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Null input string");
    return NULL;
  }
  return pcpatch_parse(&str, true);
}

char *
pcpatch_hex_out(const Pcpatch *pa, int maxdd)
{
  (void) maxdd;
  assert(pa);
  size_t byte_len = VARSIZE(pa);
  size_t hex_len = byte_len * 2;
  char *result = palloc(hex_len + 1);
  hex_encode((const char *) pa, byte_len, result);
  result[hex_len] = '\0';
  return result;
}

Pcpatch *
pcpatch_from_hexwkb(const char *hexwkb)
{
  return pcpatch_hex_in(hexwkb);
}

char *
pcpatch_as_hexwkb(const Pcpatch *pa)
{
  return pcpatch_hex_out(pa, 0);
}

/*****************************************************************************
 * Constructor + accessors
 *****************************************************************************/

Pcpatch *
pcpatch_copy(const Pcpatch *pa)
{
  assert(pa);
  size_t sz = VARSIZE(pa);
  Pcpatch *result = palloc(sz);
  memcpy(result, pa, sz);
  return result;
}

uint32_t pcpatch_get_pcid(const Pcpatch *pa)    { assert(pa); return pa->pcid; }
uint32_t pcpatch_npoints(const Pcpatch *pa) { assert(pa); return pa->npoints; }

uint32
pcpatch_hash(const Pcpatch *pa)
{
  assert(pa);
  return hash_any((const unsigned char *) pa,
    (int) pcpatch_meaningful_size(pa));
}

uint64
pcpatch_hash_extended(const Pcpatch *pa, uint64 seed)
{
  assert(pa);
  return hash_any_extended((const unsigned char *) pa,
    (int) pcpatch_meaningful_size(pa), seed);
}

/*****************************************************************************
 * Comparison
 *****************************************************************************/

int
pcpatch_cmp(const Pcpatch *pa1, const Pcpatch *pa2)
{
  assert(pa1); assert(pa2);
  size_t sz1 = pcpatch_meaningful_size(pa1);
  size_t sz2 = pcpatch_meaningful_size(pa2);
  size_t minsz = (sz1 < sz2) ? sz1 : sz2;
  int c = memcmp(pa1, pa2, minsz);
  if (c != 0) return (c < 0) ? -1 : 1;
  if (sz1 == sz2) return 0;
  return (sz1 < sz2) ? -1 : 1;
}

bool pcpatch_eq(const Pcpatch *pa1, const Pcpatch *pa2)
{ return pcpatch_cmp(pa1, pa2) == 0; }
bool pcpatch_ne(const Pcpatch *pa1, const Pcpatch *pa2)
{ return pcpatch_cmp(pa1, pa2) != 0; }
bool pcpatch_lt(const Pcpatch *pa1, const Pcpatch *pa2)
{ return pcpatch_cmp(pa1, pa2) <  0; }
bool pcpatch_le(const Pcpatch *pa1, const Pcpatch *pa2)
{ return pcpatch_cmp(pa1, pa2) <= 0; }
bool pcpatch_gt(const Pcpatch *pa1, const Pcpatch *pa2)
{ return pcpatch_cmp(pa1, pa2) >  0; }
bool pcpatch_ge(const Pcpatch *pa1, const Pcpatch *pa2)
{ return pcpatch_cmp(pa1, pa2) >= 0; }

/*****************************************************************************/
