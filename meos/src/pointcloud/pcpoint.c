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
 * @brief Opaque byte-level helpers for the pgpointcloud `pcpoint` base type.
 *
 * MEOS treats `pcpoint` values as opaque varlena byte blobs that share a
 * fixed layout (`SERIALIZED_POINT`) with pgpointcloud. The only field
 * interpreted here is `pcid` (schema id) at its fixed offset — enough for
 * same-schema equality checks. Dimension-level extraction (X, Y, intensity,
 * …) requires loading the XML schema keyed by `pcid` from the PG
 * `pointcloud_formats` table and is deferred to Phase 8G (tpcpoint).
 */

#include "pointcloud/pcpoint.h"

/* C */
#include <assert.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include "common/hashfn.h"
#include "utils/builtins.h"  /* hex_encode, hex_decode */
/* MEOS */
#include <meos.h>

/*****************************************************************************
 * Validity helpers
 *****************************************************************************/

/**
 * @brief Return true if two pcpoints share the same schema (pcid)
 */
bool
ensure_same_pcid_pcpoint(const Pcpoint *pt1, const Pcpoint *pt2)
{
  assert(pt1); assert(pt2);
  if (pt1->pcid != pt2->pcid)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on pcpoint values with different schemas: %u vs %u",
      pt1->pcid, pt2->pcid);
    return false;
  }
  return true;
}

/*****************************************************************************
 * Input/output functions
 *
 * Text format at the MEOS layer is the ASCII hex encoding of the raw
 * SERIALIZED_POINT varlena bytes. This mirrors how PostGIS geometry is
 * text-represented in MEOS (HexWKB). Structured WKT parsing requires
 * schema resolution and is out of scope for Phase 8D.
 *****************************************************************************/

/**
 * @brief Parse a pcpoint from its hex-encoded representation in a cursor
 */
Pcpoint *
pcpoint_parse(const char **str, bool end)
{
  const char *type_str = "pcpoint";
  const char *p = *str;
  /* Skip leading whitespace */
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;

  /* Scan contiguous hex chars */
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
  /* Minimum viable: varlena header (4 B) + pcid (4 B) = 8 bytes of raw
   * bytes AFTER the varlena header. Since SET_VARSIZE stores total length
   * including the 4-byte header, the decoded byte stream is what goes
   * INTO the pcpoint minus the vl_len_ field. The pgpointcloud on-wire
   * layout stores the 4-byte size at the start of the blob, so we accept
   * the decoded bytes directly as the full varlena. */
  if (byte_len < VARHDRSZ + sizeof(uint32_t))
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: too short", type_str);
    return NULL;
  }

  Pcpoint *result = palloc(byte_len);
  if (hex_decode(hex_start, hex_len, (char *) result) != byte_len)
  {
    pfree(result);
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: hex decode failed", type_str);
    return NULL;
  }
  /* Overwrite the varlena header to match the decoded byte length (the
   * on-wire first 4 bytes may use pgpointcloud's own size convention). */
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

/**
 * @brief Return a pcpoint from its hex-encoded representation
 */
Pcpoint *
pcpoint_in(const char *str)
{
  if (! str)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Null input string");
    return NULL;
  }
  return pcpoint_parse(&str, true);
}

/**
 * @brief Return the hex-encoded representation of a pcpoint
 * @note `maxdd` is ignored — pcpoint has no floating-point text form at
 *   the MEOS layer.
 */
char *
pcpoint_out(const Pcpoint *pt, int maxdd)
{
  (void) maxdd;
  assert(pt);
  size_t byte_len = VARSIZE(pt);
  size_t hex_len = byte_len * 2;
  char *result = palloc(hex_len + 1);
  hex_encode((const char *) pt, byte_len, result);
  result[hex_len] = '\0';
  return result;
}

/**
 * @brief Return a pcpoint from its hex-encoded representation
 */
Pcpoint *
pcpoint_from_hexwkb(const char *hexwkb)
{
  return pcpoint_in(hexwkb);
}

/**
 * @brief Return the hex-encoded representation of a pcpoint
 */
char *
pcpoint_as_hexwkb(const Pcpoint *pt)
{
  return pcpoint_out(pt, 0);
}

/*****************************************************************************
 * Constructor
 *****************************************************************************/

/**
 * @brief Return a palloc'd copy of a pcpoint
 */
Pcpoint *
pcpoint_copy(const Pcpoint *pt)
{
  assert(pt);
  size_t sz = VARSIZE(pt);
  Pcpoint *result = palloc(sz);
  memcpy(result, pt, sz);
  return result;
}

/*****************************************************************************
 * Accessors
 *****************************************************************************/

/**
 * @brief Return the pcid (schema id) of a pcpoint
 */
uint32_t
pcpoint_pcid(const Pcpoint *pt)
{
  assert(pt);
  return pt->pcid;
}

/**
 * @brief Return the 32-bit hash of a pcpoint
 */
uint32
pcpoint_hash(const Pcpoint *pt)
{
  assert(pt);
  return hash_any((const unsigned char *) pt, (int) VARSIZE(pt));
}

/**
 * @brief Return the 64-bit seeded hash of a pcpoint
 */
uint64
pcpoint_hash_extended(const Pcpoint *pt, uint64 seed)
{
  assert(pt);
  return hash_any_extended((const unsigned char *) pt, (int) VARSIZE(pt),
    seed);
}

/*****************************************************************************
 * Comparison
 *
 * Byte-wise on the full varlena, with length as the primary key. This gives
 * a stable total order usable by MEOS Set dedup/sort machinery without
 * making any claim about meaningful ordering on pcpoint values.
 *****************************************************************************/

/**
 * @brief Compare two pcpoints byte-wise
 * @return -1 / 0 / 1
 */
int
pcpoint_cmp(const Pcpoint *pt1, const Pcpoint *pt2)
{
  assert(pt1); assert(pt2);
  size_t sz1 = VARSIZE(pt1);
  size_t sz2 = VARSIZE(pt2);
  size_t minsz = (sz1 < sz2) ? sz1 : sz2;
  int c = memcmp(pt1, pt2, minsz);
  if (c != 0) return (c < 0) ? -1 : 1;
  if (sz1 == sz2) return 0;
  return (sz1 < sz2) ? -1 : 1;
}

bool pcpoint_eq(const Pcpoint *pt1, const Pcpoint *pt2)
{ return pcpoint_cmp(pt1, pt2) == 0; }
bool pcpoint_ne(const Pcpoint *pt1, const Pcpoint *pt2)
{ return pcpoint_cmp(pt1, pt2) != 0; }
bool pcpoint_lt(const Pcpoint *pt1, const Pcpoint *pt2)
{ return pcpoint_cmp(pt1, pt2) <  0; }
bool pcpoint_le(const Pcpoint *pt1, const Pcpoint *pt2)
{ return pcpoint_cmp(pt1, pt2) <= 0; }
bool pcpoint_gt(const Pcpoint *pt1, const Pcpoint *pt2)
{ return pcpoint_cmp(pt1, pt2) >  0; }
bool pcpoint_ge(const Pcpoint *pt1, const Pcpoint *pt2)
{ return pcpoint_cmp(pt1, pt2) >= 0; }

/*****************************************************************************/
