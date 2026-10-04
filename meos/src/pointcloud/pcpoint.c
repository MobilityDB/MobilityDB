/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Opaque byte-level helpers for the pgpointcloud `pcpoint` base type
 * @details MEOS treats `pcpoint` values as opaque varlena byte blobs that
 * share a
 * fixed layout (`SERIALIZED_POINT`) with pgpointcloud. The only field
 * interpreted here is `pcid` (schema id) at its fixed offset — enough for
 * same-schema equality checks. Dimension-level extraction (X, Y, intensity,
 * …) requires loading the XML schema keyed by `pcid` from the PG
 * `pointcloud_formats` table and is handled by the PG wrapper layer.
 */

#include "pointcloud/pcpoint.h"

/* C */
#include <assert.h>
#include <limits.h>
#include <stddef.h>          /* offsetof */
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
#include <common/hashfn.h>
/* PostGIS */
#include <liblwgeom.h>       /* parse_hex, deparse_hex (PostGIS) */
/* pgPointCloud */
#include "pc_api.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "pointcloud/meos_schema_hook.h"
#include "pointcloud/pgsql_compat.h"

/*****************************************************************************
 * Struct-tail padding
 *
 * pgpointcloud's `pc_point_serialize` in pointcloud-pg/pgsql/pc_pgsql.c
 * allocates the on-disk SERIALIZED_POINT using the formula
 *
 *   palloc(sizeof(SERIALIZED_POINT) - 1 + schema->size)
 *
 * and sets VARSIZE to the same value. Because the C struct
 *
 *   typedef struct { uint32_t size; uint32_t pcid; uint8_t data[1]; }
 *     SERIALIZED_POINT;
 *
 * rounds up to @c sizeof = 12 on typical 64-bit targets (data[1] at
 * offset 8, struct padded to 4-byte alignment), the formula over-
 * allocates by @c sizeof(SERIALIZED_POINT) - offsetof(..., data) - 1
 * bytes past the last meaningful byte. Those bytes come from @c palloc
 * (not @c palloc0) and are therefore uninitialized heap memory — two
 * otherwise-identical @c PC_MakePoint calls yield pcpoints that
 * disagree on the trailing padding.
 *
 * That non-determinism is only a problem for code that interprets
 * pcpoints at the byte level: @c pcpoint_cmp, @c pcpoint_hash, the
 * derived Set dedup / B-tree / hash operator-class paths. We fix it
 * here by truncating @c VARSIZE down to the meaningful prefix via a
 * compile-time-constant padding measurement. The shadow-struct idiom
 * below mirrors pgpointcloud's struct layout exactly so that our
 * compiler's padding calculation agrees with pgpointcloud's (both
 * builds target the same ABI — libpc.a is linked in, so any ABI skew
 * would already fail at link time).
 *****************************************************************************/

/**
 * @brief Serialized point cloud point, laid out as the upstream library writes it
 */
typedef struct
{
  int32 vl_len_;
  uint32_t pcid;
  uint8_t data[1];  /* matches upstream SERIALIZED_POINT; flex-array
                     * sugar `data[FLEXIBLE_ARRAY_MEMBER]` would give
                     * a different sizeof and defeat the measurement. */
} PcpointLayoutShadow;

#define PCPOINT_TAIL_PADDING \
  (sizeof(PcpointLayoutShadow) - offsetof(PcpointLayoutShadow, data) - 1)

/**
 * @brief Return the meaningful byte length of a pcpoint
 * @details The length is VARSIZE minus pgpointcloud's struct-tail padding.
 * @note Guards against degenerate tiny values: if the computed length
 *   would dip below the header + pcid (8 bytes), fall back to VARSIZE.
 *   In practice any well-formed pcpoint has VARSIZE >= 12 (header +
 *   pcid + at least 1 data byte from the data[1] placeholder) so the
 *   guard only protects against malformed input, not valid values.
 * @note Exported (not `static`) so that the generic WKB path in
 *   temporal/type_out.c (`pcpoint_to_wkb_buf`) can zero the same padding
 *   bytes and stay byte-consistent with the hex-WKB path below.
 */
size_t
pcpoint_meaningful_size(const Pcpoint *pt)
{
  size_t sz = VARSIZE(pt);
  size_t hdr = VARHDRSZ + sizeof(uint32_t); /* varlena + pcid */
  return (sz > hdr + PCPOINT_TAIL_PADDING) ? (sz - PCPOINT_TAIL_PADDING) : sz;
}

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
 * The text of a pcpoint is the hex encoding of its pgPointCloud Well-Known
 * Binary (WKB), the text the type input and output functions of pgPointCloud
 * read and write: the endian flag, the pcid and the data of the point under
 * the schema of the pcid. Reading and writing it resolve that schema.
 *****************************************************************************/

/**
 * @brief Return a pcpoint from its pgPointCloud Well-Known Binary (WKB)
 * representation, as pgPointCloud's @c pc_point_from_hexwkb reads it
 * @details The header and the size are tested here, since the library
 * reading the data ends a standalone process on an error
 * @param[in] wkb WKB bytes
 * @param[in] size Number of bytes
 */
static Pcpoint *
pcpoint_from_wkb_bytes(const uint8_t *wkb, size_t size)
{
  /* Endian flag and pcid */
  if (size < 1 + sizeof(uint32_t))
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse pcpoint value: too short");
    return NULL;
  }
  if (wkb[0] > 1)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse pcpoint value: invalid endian flag %d", wkb[0]);
    return NULL;
  }
  uint32_t pcid = pc_wkb_get_pcid(wkb);
  const PCSCHEMA *schema = meos_pc_schema(pcid);
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "No schema registered for pcid %u", pcid);
    return NULL;
  }
  if (size - 1 - sizeof(uint32_t) != schema->size)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse pcpoint value: %zu bytes of data where the schema %u "
      "states %zu", size - 1 - sizeof(uint32_t), pcid, schema->size);
    return NULL;
  }
  PCPOINT *pcpt = pc_point_from_wkb(schema, (uint8_t *) wkb, size);
  if (! pcpt)
    return NULL;
  Pcpoint *result = (Pcpoint *) meos_pc_point_serialize(pcpt);
  pc_point_free(pcpt);
  /* Zero the struct-tail padding, as #pcpoint_make does */
  size_t meaningful = pcpoint_meaningful_size(result);
  memset(((uint8_t *) result) + meaningful, 0, VARSIZE(result) - meaningful);
  return result;
}

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
  uint8_t *wkb = palloc(byte_len);
  /* The scan above admits hexadecimal digits alone, so each pair decodes */
  for (size_t i = 0; i < byte_len; i++)
    wkb[i] = parse_hex((char *) hex_start + 2 * i);
  Pcpoint *result = pcpoint_from_wkb_bytes(wkb, byte_len);
  pfree(wkb);
  if (! result)
    return NULL;

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
 * @brief Return a pcpoint from its textual (hex-WKB) representation
 * @details The text is the one the type input function of pgPointCloud reads
 * @param[in] str String
 */
Pcpoint *
pcpoint_hex_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return pcpoint_parse(&str, true);
}

/**
 * @ingroup meos_pointcloud_base_inout
 * @brief Return the textual (hex-WKB) representation of a pcpoint
 * @details The text is the one the type output function of pgPointCloud
 * writes, in the byte order of the machine
 * @param[in] pt Point
 * @param[in] maxdd Unused (kept for API uniformity with set_out)
 */
char *
pcpoint_hex_out(const Pcpoint *pt, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  uint32_t pcid = pcpoint_get_pcid(pt);
  const PCSCHEMA *schema = meos_pc_schema(pcid);
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "No schema registered for pcid %u", pcid);
    return NULL;
  }
  PCPOINT *pcpt = meos_pc_point_deserialize((const SERIALIZED_POINT *) pt,
    schema);
  if (! pcpt)
    return NULL;
  size_t size;
  uint8_t *wkb = pc_point_to_wkb(pcpt, &size);
  pc_point_free(pcpt);
  char *result = palloc(2 * size + 1);
  for (size_t i = 0; i < size; i++)
    deparse_hex(wkb[i], result + 2 * i);
  result[2 * size] = '\0';
  pcfree(wkb);
  return result;
}

/**
 * @ingroup meos_pointcloud_base_inout
 * @brief Return a pcpoint from its hex-WKB representation
 */
Pcpoint *
pcpoint_from_hexwkb(const char *hexwkb)
{
  return pcpoint_hex_in(hexwkb);
}

/**
 * @ingroup meos_pointcloud_base_inout
 * @brief Return the hex-WKB representation of a pcpoint
 */
char *
pcpoint_as_hexwkb(const Pcpoint *pt)
{
  return pcpoint_hex_out(pt, 0);
}

/*****************************************************************************
 * Constructor
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_base_constructor
 * @brief Return a pcpoint from the coordinates of the dimensions of the
 *   schema a point cloud identifier names
 * @param[in] pcid Point cloud identifier naming the schema
 * @param[in] values Coordinate of each dimension, in the order the schema
 *   states the dimensions
 * @param[in] count Number of coordinates
 * @errval NULL
 * @note The schema is resolved through the MEOS cache, so a schema stated in
 *   SQL and one parsed from an XML document build a value alike.
 * @csqlfn #Pcpoint_make()
 */
Pcpoint *
pcpoint_make(uint32_t pcid, const double *values, int count)
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
  if (count != (int) schema->ndims)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The number of coordinates must be the number of dimensions of the "
      "schema %u: %d vs %u", pcid, count, schema->ndims);
    return NULL;
  }

  PCPOINT *pcpt = pc_point_from_double_array(schema, (double *) values, 0,
    (uint32_t) count);
  if (! pcpt)
    return NULL;
  Pcpoint *result = (Pcpoint *) meos_pc_point_serialize(pcpt);
  pc_point_free(pcpt);
  /* The bytes past the meaningful prefix are pgpointcloud's struct-tail
   * padding, which the serialization leaves uninitialized. They are zeroed
   * so that two pcpoints holding the same point hold the same bytes, which
   * the comparison and the hash read (see the file header) */
  size_t meaningful = pcpoint_meaningful_size(result);
  memset(((uint8_t *) result) + meaningful, 0, VARSIZE(result) - meaningful);
  return result;
}

/**
 * @ingroup meos_pointcloud_base_constructor
 * @brief Return a palloc'd copy of a pcpoint
 */
Pcpoint *
pcpoint_copy(const Pcpoint *pt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL);
  size_t sz = VARSIZE(pt);
  Pcpoint *result = palloc(sz);
  memcpy(result, pt, sz);
  return result;
}

/*****************************************************************************
 * Accessors
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the pcid (schema id) of a pcpoint
 * @csqlfn #Pcpoint_pcid()
 */
uint32_t
pcpoint_get_pcid(const Pcpoint *pt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, INT_MAX);
  return pt->pcid;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return the 32-bit hash of a pcpoint
 * @note Hashes only the meaningful-prefix bytes — pgpointcloud's
 *   struct-tail padding is skipped because it holds uninitialized heap
 *   bytes that differ between otherwise-identical values.
 * @errval UINT32_MAX
 * @csqlfn #Pcpoint_hash()
 */
uint32
pcpoint_hash(const Pcpoint *pt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, UINT32_MAX);
  return hash_any((const unsigned char *) pt,
    (int) pcpoint_meaningful_size(pt));
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return the 64-bit seeded hash of a pcpoint
 * @csqlfn #Pcpoint_hash_extended()
 */
uint64
pcpoint_hash_extended(const Pcpoint *pt, uint64 seed)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, UINT64_MAX);
  return hash_any_extended((const unsigned char *) pt,
    (int) pcpoint_meaningful_size(pt), seed);
}

/*****************************************************************************
 * Comparison
 *
 * Byte-wise on the full varlena, with length as the primary key. This gives
 * a stable total order usable by MEOS Set dedup/sort machinery without
 * making any claim about meaningful ordering on pcpoint values.
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Compare two pcpoints byte-wise
 * @return -1 / 0 / 1
 * @errval INT_MAX
 * @note Compares only the meaningful-prefix bytes — pgpointcloud's
 * struct-tail padding is skipped (see the padding comment above).
 * Two pcpoints that disagree only on those padding bytes now compare
 * equal, making Set dedup and B-tree equality deterministic.
 * @csqlfn #Pcpoint_cmp()
 */
int
pcpoint_cmp(const Pcpoint *pt1, const Pcpoint *pt2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt1, INT_MAX); VALIDATE_NOT_NULL(pt2, INT_MAX);
  size_t sz1 = pcpoint_meaningful_size(pt1);
  size_t sz2 = pcpoint_meaningful_size(pt2);
  size_t minsz = (sz1 < sz2) ? sz1 : sz2;
  int c = memcmp(pt1, pt2, minsz);
  if (c != 0) return (c < 0) ? -1 : 1;
  if (sz1 == sz2) return 0;
  return (sz1 < sz2) ? -1 : 1;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if two pcpoints are equal
 * @csqlfn #Pcpoint_eq()
 */
bool pcpoint_eq(const Pcpoint *pt1, const Pcpoint *pt2)
{
  return pcpoint_cmp(pt1, pt2) == 0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if two pcpoints differ
 * @csqlfn #Pcpoint_ne()
 */
bool pcpoint_ne(const Pcpoint *pt1, const Pcpoint *pt2)
{
  return pcpoint_cmp(pt1, pt2) != 0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpoint precedes the second one
 * @csqlfn #Pcpoint_lt()
 */
bool pcpoint_lt(const Pcpoint *pt1, const Pcpoint *pt2)
{
  return pcpoint_cmp(pt1, pt2) <  0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpoint precedes or equals the second one
 * @csqlfn #Pcpoint_le()
 */
bool pcpoint_le(const Pcpoint *pt1, const Pcpoint *pt2)
{
  return pcpoint_cmp(pt1, pt2) <= 0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpoint follows the second in total order
 * @csqlfn #Pcpoint_gt()
 */
bool pcpoint_gt(const Pcpoint *pt1, const Pcpoint *pt2)
{
  return pcpoint_cmp(pt1, pt2) >  0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpoint follows or equals the second one
 * @csqlfn #Pcpoint_ge()
 */
bool pcpoint_ge(const Pcpoint *pt1, const Pcpoint *pt2)
{
  return pcpoint_cmp(pt1, pt2) >= 0;
}

/*****************************************************************************
 * Schema-aware accessors
 *
 * Wrap a varlena Pcpoint as a transient PCPOINT (libpc.a's
 * uncompressed struct) so we can reuse pgpointcloud's own dimension
 * readers — pc_point_get_x/y/z, pc_point_get_double_by_name. That keeps
 * scale/offset/interpretation handling in one place (pgpointcloud).
 *****************************************************************************/

/**
 * @brief Shim a varlena Pcpoint into a read-only libpc.a PCPOINT
 * @note Shares the underlying dimension-byte pointer; the caller's
 *   Pcpoint is NOT copied.
 */
static inline void
pcpoint_as_pcpt(const Pcpoint *pt, PCSCHEMA *schema, PCPOINT *out)
{
  out->readonly = 1;
  out->schema = schema;
  /* Cast away const — libpc.a uses a non-const pointer but never writes
   * to the buffer when readonly=1. */
  out->data = (uint8_t *) ((const Pcpoint *) pt)->data;
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the X coordinate of a pcpoint via the given schema
 * @param[in] pt Point
 * @param[in] schema Point cloud schema
 * @param[out] out Result
 * @return @p true on success; @p false if the schema lacks an X dimension
 *   or the byte read fails
 * @csqlfn #Pcpoint_get_x()
 */
bool
pcpoint_get_x(const Pcpoint *pt, PCSCHEMA *schema, double *out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, false); VALIDATE_NOT_NULL(schema, false);
  VALIDATE_NOT_NULL(out, false);
  if (! schema->xdim)
    return false;
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, schema, &pcpt);
  return pc_point_get_x(&pcpt, out);
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the Y coordinate of a pcpoint via the given schema
 * @param[in] pt Point
 * @param[in] schema Point cloud schema
 * @param[out] out Result
 * @csqlfn #Pcpoint_get_y()
 */
bool
pcpoint_get_y(const Pcpoint *pt, PCSCHEMA *schema, double *out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, false); VALIDATE_NOT_NULL(schema, false);
  VALIDATE_NOT_NULL(out, false);
  if (! schema->ydim)
    return false;
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, schema, &pcpt);
  return pc_point_get_y(&pcpt, out);
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the Z coordinate of a pcpoint via the given schema
 * @param[in] pt Point
 * @param[in] schema Point cloud schema
 * @param[out] out Result
 * @csqlfn #Pcpoint_get_z()
 */
bool
pcpoint_get_z(const Pcpoint *pt, PCSCHEMA *schema, double *out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, false); VALIDATE_NOT_NULL(schema, false);
  VALIDATE_NOT_NULL(out, false);
  if (! schema->zdim)
    return false;
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, schema, &pcpt);
  return pc_point_get_z(&pcpt, out);
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return any named dimension of a pcpoint via the given schema
 * @param[in] pt Point
 * @param[in] schema Point cloud schema
 * @param[in] name Dimension name
 * @param[out] out Result
 * @csqlfn #Pcpoint_get_dim()
 */
bool
pcpoint_get_dim(const Pcpoint *pt, PCSCHEMA *schema,
  const char *name, double *out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, false); VALIDATE_NOT_NULL(schema, false);
  VALIDATE_NOT_NULL(name, false); VALIDATE_NOT_NULL(out, false);
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, schema, &pcpt);
  return pc_point_get_double_by_name(&pcpt, name, out) != 0;
}

/**
 * @brief Return every dimension of a pcpoint, in the order its schema states
 * them, the array #pcpoint_make takes
 * @details The values are read as #pcpoint_get_dim reads one, through
 * pgpointcloud's own reader, so each carries its scale and offset, and setting
 * them back recovers the stored bytes of the point. Every dimension of the
 * layout of the schema is read, which is what pgpointcloud's text form writes
 * @param[in] pt Point
 * @param[out] count Number of dimensions
 * @errval NULL
 */
double *
pcpoint_dims(const Pcpoint *pt, int *count)
{
  assert(pt); assert(count);
  PCSCHEMA *schema = meos_pc_schema_lookup(pt->pcid);
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "No schema registered for pcid %u", pt->pcid);
    return NULL;
  }
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, schema, &pcpt);
  double *result = palloc(sizeof(double) * (schema->ndims ? schema->ndims : 1));
  for (uint32_t i = 0; i < schema->ndims; i++)
  {
    if (! pc_point_get_double_by_index(&pcpt, i, &result[i]))
    {
      pfree(result);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Could not read the dimensions of a point of pcid %u", pt->pcid);
      return NULL;
    }
  }
  *count = (int) schema->ndims;
  return result;
}

/**
 * @ingroup meos_pointcloud_box_constructor
 * @brief Convert a pcpoint to a degenerate single-point TPCBox
 * @return Newly-palloc'd TPCBox, or @p NULL if the schema lacks the
 *   required X/Y dimensions.
 * @csqlfn #Pcpoint_to_tpcbox()
 */
TPCBox *
pcpoint_to_tpcbox(const Pcpoint *pt, PCSCHEMA *schema)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL); VALIDATE_NOT_NULL(schema, NULL);
  if (! schema->xdim || ! schema->ydim)
    return NULL;
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, schema, &pcpt);
  double x, y, z = 0.0;
  bool has_z = (schema->zdim != NULL);
  if (! pc_point_get_x(&pcpt, &x) || ! pc_point_get_y(&pcpt, &y) ||
      (has_z && ! pc_point_get_z(&pcpt, &z)))
    return NULL;
  return tpcbox_make(/* hasx */ true, /* hasz */ has_z,
    /* hast */ false, /* geodetic */ false,
    (int32_t) schema->srid, pt->pcid, x, x, y, y, z, z, NULL);
}

/*****************************************************************************/
