/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
#include <limits.h>
#include <string.h>
#include <stddef.h>          /* offsetof */
/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
#include <common/hashfn.h>
/* PostGIS */
#include <liblwgeom.h>       /* parse_hex, deparse_hex */
/* pgPointCloud */
#include "pc_api.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "pointcloud/pcpoint.h"
#include "pointcloud/meos_schema_hook.h"
#include "pointcloud/pgsql_compat.h"

/*****************************************************************************
 * Reserved tail
 *
 * Same phenomenon as pcpoint.c, but the amount reserved is NOT the tail
 * padding of the structure. A serialized patch is sized as
 *
 *   BUFFERALIGN(sizeof(SERIALIZED_PATCH)) - 1 + pc_stats_size + <data size>
 *
 * (upstream @c pc_patch_serialize, mirrored by @c meos_pc_patch_serialize in
 * pgsql_compat.c), and the statistics and packed points are written from
 * @c offsetof(SERIALIZED_PATCH, data) onwards. For SERIALIZED_PATCH:
 *
 *   { uint32_t size; uint32_t pcid; uint32_t compression;
 *     uint32_t npoints; PCBOUNDS bounds; uint8_t data[1]; }
 *
 * PCBOUNDS is 4 doubles (alignment 8), so @c data[1] sits at offset 48 and
 * the structure rounds to 56. BUFFERALIGN rounds that up to 64, so the bytes
 * that are reserved but never written are
 *
 *   BUFFERALIGN(sizeof) - 1 - offsetof(data)
 *
 * which is 15, not the 7 the structure alone accounts for. The buffer comes
 * from @c palloc, so those bytes hold whatever the heap held before; deriving
 * the amount from the structure leaves 8 of them inside the compared prefix
 * and makes @c pcpatch_cmp and @c pcpatch_hash depend on uninitialized
 * memory. Truncate @c VARSIZE by the reserved amount for cmp/hash.
 *
 * The point counterpart needs no such rounding: a serialized point is sized
 * as @c sizeof(SERIALIZED_POINT) - 1 + schema->size, with no BUFFERALIGN, so
 * pcpoint.c measures its reserve from the structure alone.
 *****************************************************************************/

/**
 * @brief 
 */
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
  (BUFFERALIGN(sizeof(PcpatchLayoutShadow)) - 1 - \
   offsetof(PcpatchLayoutShadow, data))

/**
 * @brief Return the comparable byte length of a pcpatch.
 * @details Strips the trailing bytes that pgpointcloud's varlena layout
 * reserves past the statistics and packed points, so that
 * @c pcpatch_cmp / @c pcpatch_hash do not depend on them.
 */
static inline size_t
pcpatch_meaningful_size(const Pcpatch *pa)
{
  size_t sz = VARSIZE(pa);
  /* header + pcid + compression + npoints + 4 bounds doubles */
  size_t hdr = VARHDRSZ + 3 * sizeof(uint32_t) + 4 * sizeof(double);
  return (sz > hdr + PCPATCH_TAIL_PADDING) ? (sz - PCPATCH_TAIL_PADDING) : sz;
}

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Return true if two pcpatch values share the same schema (pcid)
 */
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

/**
 * @brief Parse a pcpatch from its hex-encoded representation in a cursor
 */
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
  for (size_t i = 0; i < byte_len; i++)
    ((char *) result)[i] = (char) parse_hex((char *) hex_start + 2 * i);
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
 * @ingroup meos_pointcloud_base_inout
 * @brief Return a pcpatch from its textual (hex-WKB) representation
 * @param[in] str String
 * @note PG-side input is provided by pgpointcloud's own
 *   @c PC_AsBinary / @c pcpatch_in, which we do not redefine.
 */
Pcpatch *
pcpatch_hex_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return pcpatch_parse(&str, true);
}

/**
 * @ingroup meos_pointcloud_base_inout
 * @brief Return the textual (hex-WKB) representation of a pcpatch
 * @param[in] pa Patch
 * @param[in] maxdd Unused (kept for API uniformity with set_out)
 * @note The bytes past the meaningful prefix are the reserved tail described
 * at the top of this file, which the allocation leaves uninitialized. They
 * are emitted as zeros so that two patches holding the same points always
 * print the same string, keeping the full @c VARSIZE length that the
 * deserializer's size check expects.
 */
char *
pcpatch_hex_out(const Pcpatch *pa, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, NULL);
  if (! ensure_positive(maxdd))
    return NULL;

  size_t byte_len = VARSIZE(pa);
  size_t meaningful = pcpatch_meaningful_size(pa);
  size_t hex_len = byte_len * 2;
  char *result = palloc(hex_len + 1);
  size_t i = 0;
  for (; i < meaningful; i++)
    deparse_hex(((const uint8_t *) pa)[i], result + 2 * i);
  for (; i < byte_len; i++)
    deparse_hex(0, result + 2 * i);
  result[hex_len] = '\0';
  return result;
}

/**
 * @ingroup meos_pointcloud_base_inout
 * @brief Return a pcpatch from its hex-WKB representation
 */
Pcpatch *
pcpatch_from_hexwkb(const char *hexwkb)
{
  return pcpatch_hex_in(hexwkb);
}

/**
 * @ingroup meos_pointcloud_base_inout
 * @brief Return the hex-WKB representation of a pcpatch
 */
char *
pcpatch_as_hexwkb(const Pcpatch *pa)
{
  return pcpatch_hex_out(pa, 0);
}

/*****************************************************************************
 * Constructor + accessors
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_base_constructor
 * @brief Return a pcpatch from the coordinates of its points
 * @param[in] pcid Point cloud identifier naming the schema
 * @param[in] values Coordinate of each dimension of each point, one point
 *   after another, in the order the schema states the dimensions
 * @param[in] count Number of coordinates, a whole number of points
 * @return On error return @p NULL
 * @note The schema is resolved through the MEOS cache, so a schema stated in
 *   SQL and one parsed from an XML document build a value alike.
 * @csqlfn #Pcpatch_make_coords()
 */
Pcpatch *
pcpatch_make_coords(uint32_t pcid, const double *values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;
  const PCSCHEMA *schema = meos_pc_schema(pcid);
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "No schema registered for pcid %u", pcid);
    return NULL;
  }
  int ndims = (int) schema->ndims;
  /* A schema states at least one dimension, so a caller reaching this with
   * none has built one outside the registration entries, which validate it.
   * The count below divides by this, and a zero divisor is undefined */
  if (ndims < 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The schema %u states no dimensions", pcid);
    return NULL;
  }
  if (count % ndims != 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The number of coordinates must be a whole number of points of the "
      "schema %u: %d is not a multiple of %d", pcid, count, ndims);
    return NULL;
  }

  int npoints = count / ndims;
  PCPOINTLIST *pl = pc_pointlist_make(npoints);
  for (int i = 0; i < npoints; i++)
  {
    PCPOINT *pt = pc_point_from_double_array(schema, (double *) values,
      (uint32_t) (i * ndims), (uint32_t) ndims);
    if (! pt)
    {
      pc_pointlist_free(pl);
      return NULL;
    }
    pc_pointlist_add_point(pl, pt);
  }
  PCPATCH *pa = pc_patch_from_pointlist(pl);
  pc_pointlist_free(pl);
  if (! pa)
    return NULL;
  Pcpatch *result = (Pcpatch *) meos_pc_patch_serialize(pa, NULL);
  pc_patch_free(pa);
  if (! result)
    return NULL;
  /* Zero the reserved tail described at the top of this file so that two
   * patches holding the same points hold the same bytes */
  size_t meaningful = pcpatch_meaningful_size(result);
  memset(((uint8_t *) result) + meaningful, 0, VARSIZE(result) - meaningful);
  return result;
}

/**
 * @ingroup meos_pointcloud_base_constructor
 * @brief Return a pcpatch from an array of pcpoints
 * @param[in] points Array of points, all of the same schema
 * @param[in] count Number of points
 * @return On error return @p NULL
 * @note The schema is resolved through the MEOS cache, so a schema stated in
 *   SQL and one parsed from an XML document build a value alike.
 * @csqlfn #Pcpatch_make()
 */
Pcpatch *
pcpatch_make(const Pcpoint **points, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(points, NULL);
  if (! ensure_positive(count))
    return NULL;
  for (int i = 0; i < count; i++)
    VALIDATE_NOT_NULL(points[i], NULL);
  for (int i = 1; i < count; i++)
    if (! ensure_same_pcid_pcpoint(points[0], points[i]))
      return NULL;
  uint32_t pcid = pcpoint_get_pcid(points[0]);
  const PCSCHEMA *schema = meos_pc_schema(pcid);
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "No schema registered for pcid %u", pcid);
    return NULL;
  }

  PCPOINTLIST *pl = pc_pointlist_make(count);
  for (int i = 0; i < count; i++)
  {
    PCPOINT *pt = meos_pc_point_deserialize(
      (const SERIALIZED_POINT *) points[i], schema);
    if (! pt)
    {
      pc_pointlist_free(pl);
      return NULL;
    }
    pc_pointlist_add_point(pl, pt);
  }
  PCPATCH *pa = pc_patch_from_pointlist(pl);
  pc_pointlist_free(pl);
  if (! pa)
    return NULL;
  Pcpatch *result = (Pcpatch *) meos_pc_patch_serialize(pa, NULL);
  pc_patch_free(pa);
  if (! result)
    return NULL;
  /* Zero the reserved tail described at the top of this file so that two
   * patches holding the same points hold the same bytes, as pcpatch_hex_out
   * prints them */
  size_t meaningful = pcpatch_meaningful_size(result);
  memset(((uint8_t *) result) + meaningful, 0, VARSIZE(result) - meaningful);
  return result;
}

/**
 * @ingroup meos_pointcloud_base_constructor
 * @brief Return a palloc'd copy of a pcpatch
 */
Pcpatch *
pcpatch_copy(const Pcpatch *pa)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, NULL);
  size_t sz = VARSIZE(pa);
  Pcpatch *result = palloc(sz);
  memcpy(result, pa, sz);
  return result;
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the pcid (schema id) of a pcpatch
 * @csqlfn #Pcpatch_pcid()
 */
uint32_t pcpatch_get_pcid(const Pcpatch *pa)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, INT_MAX);
  return pa->pcid;
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the number of points stored in a pcpatch
 */
uint32_t pcpatch_npoints(const Pcpatch *pa)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, INT_MAX);
  return pa->npoints;
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the 32-bit hash of a pcpatch
 * @return On error return @p UINT32_MAX
 * @csqlfn #Pcpatch_hash()
 */
uint32
pcpatch_hash(const Pcpatch *pa)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, UINT32_MAX);
  return hash_any((const unsigned char *) pa,
    (int) pcpatch_meaningful_size(pa));
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the 64-bit hash of a pcpatch with a seed
 * @csqlfn #Pcpatch_hash_extended()
 */
uint64
pcpatch_hash_extended(const Pcpatch *pa, uint64 seed)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, UINT64_MAX);
  return hash_any_extended((const unsigned char *) pa,
    (int) pcpatch_meaningful_size(pa), seed);
}

/*****************************************************************************
 * Comparison
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Compare two pcpatch values byte-wise
 * @return -1 / 0 / 1
 * @return On error return @p INT_MAX
 * @note Compares only the meaningful-prefix bytes — pgpointcloud's
 *   struct-tail padding is skipped so two pcpatches that disagree only
 *   on those padding bytes compare equal.
 * @csqlfn #Pcpatch_cmp()
 */
int
pcpatch_cmp(const Pcpatch *pa1, const Pcpatch *pa2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa1, INT_MAX); VALIDATE_NOT_NULL(pa2, INT_MAX);
  size_t sz1 = pcpatch_meaningful_size(pa1);
  size_t sz2 = pcpatch_meaningful_size(pa2);
  size_t minsz = (sz1 < sz2) ? sz1 : sz2;
  int c = memcmp(pa1, pa2, minsz);
  if (c != 0) return (c < 0) ? -1 : 1;
  if (sz1 == sz2) return 0;
  return (sz1 < sz2) ? -1 : 1;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if two pcpatch values are equal
 */
bool pcpatch_eq(const Pcpatch *pa1, const Pcpatch *pa2)
{
  return pcpatch_cmp(pa1, pa2) == 0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if two pcpatch values differ
 */
bool pcpatch_ne(const Pcpatch *pa1, const Pcpatch *pa2)
{
  return pcpatch_cmp(pa1, pa2) != 0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpatch precedes the second in total order
 */
bool pcpatch_lt(const Pcpatch *pa1, const Pcpatch *pa2)
{
  return pcpatch_cmp(pa1, pa2) <  0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpatch precedes or equals the second
 *   in total order
 */
bool pcpatch_le(const Pcpatch *pa1, const Pcpatch *pa2)
{
  return pcpatch_cmp(pa1, pa2) <= 0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpatch follows the second in total order
 */
bool pcpatch_gt(const Pcpatch *pa1, const Pcpatch *pa2)
{ return pcpatch_cmp(pa1, pa2) >  0; }

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpatch follows or equals the second
 *   in total order
 */
bool pcpatch_ge(const Pcpatch *pa1, const Pcpatch *pa2)
{
  return pcpatch_cmp(pa1, pa2) >= 0;
}

/*****************************************************************************/
