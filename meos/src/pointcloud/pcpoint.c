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
#include <float.h>           /* FLT_MAX */
#include <limits.h>
#include <math.h>            /* fabs, isfinite */
#include <stddef.h>          /* offsetof */
#include <stdint.h>          /* INT8_MIN and the other bounds */
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
#include <common/hashfn.h>
/* PostGIS */
#include <liblwgeom.h>       /* parse_hex, deparse_hex (PostGIS) */
/* pgPointCloud */
#include "pc_api.h"
#include "pc_api_internal.h"   /* machine_endian, int32_flip_endian */
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "temporal/type_inout.h"
#include "pointcloud/meos_schema_hook.h"
#include "pointcloud/pgsql_compat.h"
#include "pointcloud/tpcbox.h"
#include "geo/geo_funcs.h"          /* ensure_srid_known */
#include "geo/meos_transform.h"     /* lwproj_lookup */
#include "geo/tspatial.h"           /* point4d_transf_pj */

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
 * @note Exported (not `static`) so that pcpatch.c zeroes the same padding
 *   of the points it builds.
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
 * @brief Return the endian flag of pgPointCloud, 1 for little endian and 0
 * for big endian, that a Well-Known Binary (WKB) output variant states
 * @details A variant stating neither byte order or both states the one of
 * the machine, as #datum_as_wkb reads it
 */
uint8_t
pc_wkb_endian(uint8_t variant)
{
  bool ndr = (variant & WKB_NDR) != 0, xdr = (variant & WKB_XDR) != 0;
  if (ndr && ! xdr)
    return 1;
  if (xdr && ! ndr)
    return 0;
  return (uint8_t) machine_endian();
}

/**
 * @brief Flip the int32 at the address
 */
void
pc_wkb_int32_flip(uint8_t *ptr)
{
  int32_t val;
  memcpy(&val, ptr, sizeof(int32_t));
  val = int32_flip_endian(val);
  memcpy(ptr, &val, sizeof(int32_t));
}

/**
 * @brief Return the pgPointCloud Well-Known Binary (WKB) representation of a
 * pcpoint in the byte order the variant states
 * @details The WKB is the endian flag, the pcid and the data of the point
 * under the schema of the pcid, as pgPointCloud's @c pc_point_to_wkb writes
 * it in the byte order of the machine; the other byte order flips the pcid
 * and each value of the data
 * @param[in] pt Point
 * @param[in] variant Output variant, of which only the byte order is read
 * @param[out] size_out Size of the result
 */
uint8_t *
pcpoint_to_wkb_bytes(const Pcpoint *pt, uint8_t variant, size_t *size_out)
{
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
  uint8_t *result = palloc(size);
  memcpy(result, wkb, size);
  pcfree(wkb);
  uint8_t endian = pc_wkb_endian(variant);
  if (result[0] != endian)
  {
    /* Endian flag, pcid and data */
    result[0] = endian;
    pc_wkb_int32_flip(result + 1);
    uint8_t *data = uncompressed_bytes_flip_endian(result + 1 +
      sizeof(uint32_t), schema, 1);
    memcpy(result + 1 + sizeof(uint32_t), data, schema->size);
    pcfree(data);
  }
  *size_out = size;
  return result;
}

/**
 * @ingroup meos_pointcloud_base_inout
 * @brief Return the textual (hex-WKB) representation of a pcpoint
 * @details The text is the one the type output function of pgPointCloud
 * writes, in the byte order of the machine
 * @param[in] pt Point
 * @param[in] maxdd Unused (kept for API uniformity with set_out)
 * @param[out] size_out Size of the result, the terminating null character
 * included
 */
char *
pcpoint_hex_out(const Pcpoint *pt, int maxdd, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(size_out, NULL);
  VALIDATE_NOT_NULL(pt, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  size_t size;
  uint8_t *wkb = pcpoint_to_wkb_bytes(pt, 0, &size);
  if (! wkb)
    return NULL;
  char *result = palloc(2 * size + 1);
  for (size_t i = 0; i < size; i++)
    deparse_hex(wkb[i], result + 2 * i);
  result[2 * size] = '\0';
  *size_out = 2 * size + 1;
  pfree(wkb);
  return result;
}

/**
 * @ingroup meos_pointcloud_base_inout
 * @brief Return a pcpoint from its Well-Known Binary (WKB) representation
 * @details The WKB is the one of pgPointCloud, in either byte order
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #Pcpoint_from_wkb()
 */
Pcpoint *
pcpoint_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  return pcpoint_from_wkb_bytes(wkb, size);
}

/**
 * @ingroup meos_pointcloud_base_inout
 * @brief Return a pcpoint from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Pcpoint_from_hexwkb()
 */
Pcpoint *
pcpoint_from_hexwkb(const char *hexwkb)
{
  return pcpoint_hex_in(hexwkb);
}

/**
 * @ingroup meos_pointcloud_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a pcpoint
 * @details The WKB is the one of pgPointCloud, in the byte order the variant
 * states
 * @param[in] pt Point
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Pcpoint_as_wkb()
 */
uint8_t *
pcpoint_as_wkb(const Pcpoint *pt, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(PointerGetDatum(pt), T_PCPOINT, variant, size_out);
}

/**
 * @ingroup meos_pointcloud_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a pcpoint
 * @details The WKB is the one of pgPointCloud, in the byte order the variant
 * states
 * @param[in] pt Point
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output, the terminating null character
 * included
 * @csqlfn #Pcpoint_as_hexwkb()
 */
char *
pcpoint_as_hexwkb(const Pcpoint *pt, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return (char *) datum_as_wkb(PointerGetDatum(pt), T_PCPOINT,
    variant | (uint8_t) WKB_HEX, size_out);
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
 * @csqlfn #Pcpoint_make(), #Pcpoint_make_coords()
 */
Pcpoint *
pcpoint_make(uint32_t pcid, const double *values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;
  for (int i = 0; i < count; i++)
    if (! ensure_finite(values[i]))
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

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the size in bytes of a pcpoint
 * @details The size is the number of bytes of the flat memory image of the
 * value, which #pcpoint_copy duplicates
 * @param[in] pt Pcpoint
 * @errval INT_MAX
 */
int
pcpoint_mem_size(const Pcpoint *pt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, INT_MAX);
  return (int) VARSIZE(pt);
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
 * @errval UINT64_MAX
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
 * Comparison and sort key
 *
 * pgPointCloud defines no order on a point or a patch, and the one order it
 * states, along the decoded dimensions (PC_Sort), needs the schema of the
 * pcid, which a comparison does not read: a value is read and compared with
 * no catalog, only a question that decodes a coordinate reports a missing
 * schema. A point cloud value is therefore ordered first by its pcid as a
 * number, then by the bytes stored after the pcid as unsigned bytes, a value
 * whose bytes prefix those of another sorting first, so that two values are
 * equal exactly when their meaningful bytes are, which their hash reads.
 *****************************************************************************/

/**
 * @brief Compare two point cloud values given their pcid and the bytes
 * stored after it
 * @details Ordered by the pcid as a number, then by the bytes as unsigned
 * bytes, a prefix sorting first
 * @param[in] pcid1,pcid2 Pcids of the values
 * @param[in] b1,b2 Bytes stored after the pcid
 * @param[in] n1,n2 Number of meaningful bytes after the pcid
 * @return -1 / 0 / 1
 */
int
pointcloud_bytes_cmp(uint32_t pcid1, const uint8_t *b1, size_t n1,
  uint32_t pcid2, const uint8_t *b2, size_t n2)
{
  assert(b1); assert(b2);
  if (pcid1 != pcid2)
    return (pcid1 < pcid2) ? -1 : 1;
  int c = memcmp(b1, b2, Min(n1, n2));
  if (c != 0)
    return (c < 0) ? -1 : 1;
  return (n1 > n2) - (n1 < n2);
}

/**
 * @brief Return the number of bytes of the sort key of a point cloud value
 * given the bytes stored after its pcid
 * @param[in] b Bytes stored after the pcid
 * @param[in] n Number of meaningful bytes after the pcid
 */
size_t
pointcloud_sortkey_size(const uint8_t *b, size_t n)
{
  assert(b);
  size_t result = sizeof(uint32_t) + n + 2;
  for (size_t i = 0; i < n; i++)
    if (b[i] == 0)
      result++;
  return result;
}

/**
 * @brief Write the sort key of a point cloud value given its pcid and the
 * bytes stored after it
 * @details The key holds the pcid as a 4-byte integer, most significant byte
 * first, then the bytes as #datum_sortkey_put writes the bytes of a text: a
 * zero byte as a zero byte followed by 0xFF, ended by two zero bytes, which
 * sort below every byte that continues a longer value, so that comparing two
 * keys byte by byte as unsigned values orders the values as
 * #pointcloud_bytes_cmp does, also when other keys follow. Its length is
 * #pointcloud_sortkey_size
 * @param[in] pcid Pcid of the value
 * @param[in] b Bytes stored after the pcid
 * @param[in] n Number of meaningful bytes after the pcid
 * @param[out] out Buffer of #pointcloud_sortkey_size bytes
 * @return The first byte after the key
 */
uint8_t *
pointcloud_sortkey_put(uint32_t pcid, const uint8_t *b, size_t n,
  uint8_t *out)
{
  assert(b); assert(out);
  for (int i = 3; i >= 0; i--)
  {
    out[i] = pcid & 0xFF;
    pcid >>= 8;
  }
  out += sizeof(uint32_t);
  for (size_t i = 0; i < n; i++)
  {
    *out++ = b[i];
    if (b[i] == 0)
      *out++ = 0xFF;
  }
  *out++ = 0;
  *out++ = 0;
  return out;
}

/**
 * @brief Return the number of bytes of the sort key of a pcpoint
 * @param[in] pt Point
 */
size_t
pcpoint_sortkey_size(const Pcpoint *pt)
{
  assert(pt);
  return pointcloud_sortkey_size((const uint8_t *) pt + PC_BYTES_OFFSET,
    pcpoint_meaningful_size(pt) - PC_BYTES_OFFSET);
}

/**
 * @brief Write the sort key of a pcpoint, which #pointcloud_sortkey_put
 * states
 * @param[in] pt Point
 * @param[out] out Buffer of #pcpoint_sortkey_size bytes
 * @return The first byte after the key
 */
uint8_t *
pcpoint_sortkey_put(const Pcpoint *pt, uint8_t *out)
{
  assert(pt); assert(out);
  return pointcloud_sortkey_put(pcpoint_get_pcid(pt),
    (const uint8_t *) pt + PC_BYTES_OFFSET,
    pcpoint_meaningful_size(pt) - PC_BYTES_OFFSET, out);
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Compare two pcpoints, first by their pcid as a number, then by the
 * bytes stored after it
 * @return -1 / 0 / 1
 * @errval INT_MAX
 * @note Reads only the meaningful bytes, pgPointCloud's struct-tail padding
 * being skipped (see the padding comment above), so that Set dedup and B-tree
 * equality are deterministic
 * @csqlfn #Pcpoint_cmp()
 */
int
pcpoint_cmp(const Pcpoint *pt1, const Pcpoint *pt2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt1, INT_MAX); VALIDATE_NOT_NULL(pt2, INT_MAX);
  return pointcloud_bytes_cmp(pcpoint_get_pcid(pt1),
    (const uint8_t *) pt1 + PC_BYTES_OFFSET,
    pcpoint_meaningful_size(pt1) - PC_BYTES_OFFSET, pcpoint_get_pcid(pt2),
    (const uint8_t *) pt2 + PC_BYTES_OFFSET,
    pcpoint_meaningful_size(pt2) - PC_BYTES_OFFSET);
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if two pcpoints are equal
 * @csqlfn #Pcpoint_eq()
 */
bool
pcpoint_eq(const Pcpoint *pt1, const Pcpoint *pt2)
{
  return pcpoint_cmp(pt1, pt2) == 0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if two pcpoints differ
 * @csqlfn #Pcpoint_ne()
 */
bool
pcpoint_ne(const Pcpoint *pt1, const Pcpoint *pt2)
{
  return pcpoint_cmp(pt1, pt2) != 0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpoint precedes the second one
 * @csqlfn #Pcpoint_lt()
 */
bool
pcpoint_lt(const Pcpoint *pt1, const Pcpoint *pt2)
{
  return pcpoint_cmp(pt1, pt2) <  0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpoint precedes or equals the second one
 * @csqlfn #Pcpoint_le()
 */
bool
pcpoint_le(const Pcpoint *pt1, const Pcpoint *pt2)
{
  return pcpoint_cmp(pt1, pt2) <= 0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpoint follows the second in total order
 * @csqlfn #Pcpoint_gt()
 */
bool
pcpoint_gt(const Pcpoint *pt1, const Pcpoint *pt2)
{
  return pcpoint_cmp(pt1, pt2) >  0;
}

/**
 * @ingroup meos_pointcloud_base_comp
 * @brief Return true if the first pcpoint follows or equals the second one
 * @csqlfn #Pcpoint_ge()
 */
bool
pcpoint_ge(const Pcpoint *pt1, const Pcpoint *pt2)
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
pcpoint_as_pcpt(const Pcpoint *pt, const PCSCHEMA *schema, PCPOINT *out)
{
  out->readonly = 1;
  out->schema = schema;
  /* Cast away const — libpc.a uses a non-const pointer but never writes
   * to the buffer when readonly=1. */
  out->data = (uint8_t *) ((const Pcpoint *) pt)->data;
}

/**
 * @ingroup meos_internal_pointcloud_base_accessor
 * @brief Return the X coordinate of a pcpoint read in a schema
 * @param[in] pt Point
 * @param[in] schema Point cloud schema
 * @param[out] out Result
 * @return @p true on success; @p false if the schema lacks an X dimension
 *   or the byte read fails
 */
bool
pcpoint_schema_get_x(const Pcpoint *pt, PCSCHEMA *schema, double *out)
{
  assert(pt); assert(schema); assert(out);
  if (! schema->xdim)
    return false;
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, schema, &pcpt);
  return pc_point_get_x(&pcpt, out);
}

/**
 * @ingroup meos_internal_pointcloud_base_accessor
 * @brief Return the Y coordinate of a pcpoint read in a schema
 * @param[in] pt Point
 * @param[in] schema Point cloud schema
 * @param[out] out Result
 */
bool
pcpoint_schema_get_y(const Pcpoint *pt, PCSCHEMA *schema, double *out)
{
  assert(pt); assert(schema); assert(out);
  if (! schema->ydim)
    return false;
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, schema, &pcpt);
  return pc_point_get_y(&pcpt, out);
}

/**
 * @ingroup meos_internal_pointcloud_base_accessor
 * @brief Return the Z coordinate of a pcpoint read in a schema
 * @param[in] pt Point
 * @param[in] schema Point cloud schema
 * @param[out] out Result
 */
bool
pcpoint_schema_get_z(const Pcpoint *pt, PCSCHEMA *schema, double *out)
{
  assert(pt); assert(schema); assert(out);
  if (! schema->zdim)
    return false;
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, schema, &pcpt);
  return pc_point_get_z(&pcpt, out);
}

/**
 * @ingroup meos_internal_pointcloud_base_accessor
 * @brief Return any named dimension of a pcpoint read in a schema
 * @param[in] pt Point
 * @param[in] schema Point cloud schema
 * @param[in] name Dimension name
 * @param[out] out Result
 */
bool
pcpoint_schema_get_dim(const Pcpoint *pt, PCSCHEMA *schema,
  const char *name, double *out)
{
  assert(pt); assert(schema); assert(name); assert(out);
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, schema, &pcpt);
  return pc_point_get_double_by_name(&pcpt, name, out) != 0;
}

/**
 * @brief Return the schema the pcid of a pcpoint names
 * @details A point stores its dimensions as the bytes of a layout its schema
 * states, so reading any of them needs that schema; a pcid naming no
 * registered schema is an error, as pgpointcloud answers it
 * @param[in] pt Point
 * @errval NULL
 */
PCSCHEMA *
pcpoint_schema(const Pcpoint *pt)
{
  assert(pt);
  PCSCHEMA *result = meos_pc_schema_lookup(pt->pcid);
  if (! result)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "No schema registered for pcid %u", pt->pcid);
  return result;
}

/**
 * @brief Return every dimension of a pcpoint, in the order its schema states
 * them, the array #pcpoint_make takes
 * @details The values are read as #pcpoint_schema_get_dim reads one, through
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
  PCSCHEMA *schema = pcpoint_schema(pt);
  if (! schema)
    return NULL;
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
 * @ingroup meos_internal_pointcloud_box_conversion
 * @brief Convert a pcpoint read in a schema to a degenerate single-point
 * TPCBox
 * @param[in] pt Point
 * @param[in] schema Point cloud schema
 * @return Newly-palloc'd TPCBox, or @p NULL if the schema lacks the
 *   required X/Y dimensions.
 */
TPCBox *
pcpoint_schema_to_tpcbox(const Pcpoint *pt, PCSCHEMA *schema)
{
  assert(pt); assert(schema);
  if (! schema->xdim || ! schema->ydim)
    return NULL;
  PCPOINT pcpt;
  pcpoint_as_pcpt(pt, schema, &pcpt);
  double x, y, z = 0.0;
  bool has_z = (schema->zdim != NULL);
  if (! pc_point_get_x(&pcpt, &x) || ! pc_point_get_y(&pcpt, &y) ||
      (has_z && ! pc_point_get_z(&pcpt, &z)))
    return NULL;
  return tpcbox_srid_make(/* hasx */ true, /* hasz */ has_z,
    /* hast */ false, /* geodetic */ false,
    (int32_t) schema->srid, pt->pcid, x, x, y, y, z, z, NULL);
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the X coordinate of a pcpoint
 * @param[in] pt Point
 * @param[out] out Result
 * @return @p true on success; @p false if the schema lacks an X dimension
 *   or the byte read fails
 * @csqlfn #Pcpoint_get_x()
 */
bool
pcpoint_get_x(const Pcpoint *pt, double *out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, false); VALIDATE_NOT_NULL(out, false);
  PCSCHEMA *schema = pcpoint_schema(pt);
  if (! schema)
    return false;
  return pcpoint_schema_get_x(pt, schema, out);
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the Y coordinate of a pcpoint
 * @param[in] pt Point
 * @param[out] out Result
 * @return @p true on success; @p false if the schema lacks a Y dimension
 *   or the byte read fails
 * @csqlfn #Pcpoint_get_y()
 */
bool
pcpoint_get_y(const Pcpoint *pt, double *out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, false); VALIDATE_NOT_NULL(out, false);
  PCSCHEMA *schema = pcpoint_schema(pt);
  if (! schema)
    return false;
  return pcpoint_schema_get_y(pt, schema, out);
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return the Z coordinate of a pcpoint
 * @param[in] pt Point
 * @param[out] out Result
 * @return @p true on success; @p false if the schema lacks a Z dimension
 *   or the byte read fails
 * @csqlfn #Pcpoint_get_z()
 */
bool
pcpoint_get_z(const Pcpoint *pt, double *out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, false); VALIDATE_NOT_NULL(out, false);
  PCSCHEMA *schema = pcpoint_schema(pt);
  if (! schema)
    return false;
  return pcpoint_schema_get_z(pt, schema, out);
}

/**
 * @ingroup meos_pointcloud_base_accessor
 * @brief Return any named dimension of a pcpoint
 * @param[in] pt Point
 * @param[in] name Dimension name
 * @param[out] out Result
 * @return @p true on success; @p false if the byte read fails
 * @note A name the schema does not state is an error, as pgPointCloud's
 * `PC_Get` raises it
 * @csqlfn #Pcpoint_get_dim()
 */
bool
pcpoint_get_dim(const Pcpoint *pt, const char *name, double *out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, false); VALIDATE_NOT_NULL(name, false);
  VALIDATE_NOT_NULL(out, false);
  PCSCHEMA *schema = pcpoint_schema(pt);
  if (! schema)
    return false;
  if (! pc_schema_get_dimension_by_name(schema, name))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "dimension \"%s\" does not exist in schema", name);
    return false;
  }
  return pcpoint_schema_get_dim(pt, schema, name, out);
}

/**
 * @ingroup meos_pointcloud_box_constructor
 * @brief Convert a pcpoint to a degenerate single-point TPCBox
 * @details The SRID of the box is the one the schema of the pcid of the
 * point states
 * @param[in] pt Point
 * @return Newly-palloc'd TPCBox, or @p NULL if the schema lacks the
 *   required X/Y dimensions
 * @csqlfn #Pcpoint_to_tpcbox()
 */
TPCBox *
pcpoint_to_tpcbox(const Pcpoint *pt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL);
  PCSCHEMA *schema = pcpoint_schema(pt);
  if (! schema)
    return NULL;
  return pcpoint_schema_to_tpcbox(pt, schema);
}

/*****************************************************************************/

/*****************************************************************************
 * Transformation into the schema of another pcid
 *****************************************************************************/

/**
 * @brief Return true if a coordinate fits a dimension of a schema
 * @details A dimension stores a coordinate as `(value - offset) / scale` in
 * its interpretation, which pgpointcloud's @c pc_double_to_ptr clamps into the
 * range of that interpretation, so a coordinate beyond it would be stored as
 * another one; the transformation refuses it instead
 * @param[in] dim Dimension
 * @param[in] value Coordinate
 */
static bool
pc_dimension_holds(const PCDIMENSION *dim, double value)
{
  double raw = (value - dim->offset) / dim->scale;
  switch (dim->interpretation)
  {
    case PC_INT8:   return raw >= INT8_MIN && raw <= INT8_MAX;
    case PC_UINT8:  return raw >= 0 && raw <= UINT8_MAX;
    case PC_INT16:  return raw >= INT16_MIN && raw <= INT16_MAX;
    case PC_UINT16: return raw >= 0 && raw <= UINT16_MAX;
    case PC_INT32:  return raw >= INT32_MIN && raw <= INT32_MAX;
    case PC_UINT32: return raw >= 0 && raw <= UINT32_MAX;
    case PC_INT64:  return raw >= (double) INT64_MIN &&
                      raw <= (double) INT64_MAX;
    case PC_UINT64: return raw >= 0 && raw <= (double) UINT64_MAX;
    case PC_FLOAT:  return fabs(raw) <= FLT_MAX;
    case PC_DOUBLE: return isfinite(raw);
    default:        return false;
  }
}

/**
 * @brief Return in the last arguments the schema of the target pcid of a
 * point cloud transformation and the transformation between the SRIDs of the
 * two schemas, @p NULL when they state the same SRID
 * @details A point cloud value takes its SRID from the schema its pcid names,
 * so it is transformed into another schema, which states the target SRID and
 * how the transformed coordinates are stored. Both schemas must be registered
 * and state an SRID, and the target one must have an X and a Y dimension
 * @param[in] pcid_from pcid of the value
 * @param[in] pcid_to Target pcid
 * @param[out] schema_to Schema of the target pcid
 * @param[out] pj Transformation, @p NULL when the SRIDs are equal
 */
bool
pc_transform_lookup(uint32_t pcid_from, uint32_t pcid_to,
  const PCSCHEMA **schema_to, LWPROJ **pj)
{
  *pj = NULL;
  const PCSCHEMA *from = meos_pc_schema(pcid_from);
  const PCSCHEMA *to = meos_pc_schema(pcid_to);
  if (! from || ! to)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "No schema registered for pcid %u", from ? pcid_to : pcid_from);
    return false;
  }
  if (! ensure_srid_known((int32_t) from->srid) ||
      ! ensure_srid_known((int32_t) to->srid))
    return false;
  if (! to->xdim || ! to->ydim)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The schema of pcid %u has no X and Y dimensions", pcid_to);
    return false;
  }
  *schema_to = to;
  if (from->srid == to->srid)
    return true;
  return lwproj_lookup((int32_t) from->srid, (int32_t) to->srid, pj) != 0;
}

/**
 * @brief Return a pcpoint transformed into the schema of another pcid
 * @details Each dimension of the target schema takes the coordinate of the
 * dimension of the same name of the point, or 0 where the point has none, as
 * pgpointcloud's @c PC_Transform fills it. The X, Y and Z coordinates are
 * then transformed between the SRIDs of the two schemas, and a coordinate
 * that does not fit its target dimension is refused
 * @param[in] pt Point
 * @param[in] pcid_to Target pcid
 * @param[in] to Schema of the target pcid
 * @param[in] pj Transformation, @p NULL when the SRIDs are equal
 * @errval NULL
 */
Pcpoint *
pcpoint_transf_pj(const Pcpoint *pt, uint32_t pcid_to, const PCSCHEMA *to,
  const LWPROJ *pj)
{
  assert(pt); assert(to);
  PCSCHEMA *from = pcpoint_schema(pt);
  if (! from)
    return NULL;
  double *values = palloc(sizeof(double) * to->ndims);
  for (uint32_t i = 0; i < to->ndims; i++)
  {
    values[i] = 0.0;
    const char *name = to->dims[i]->name;
    if (pc_schema_get_dimension_by_name(from, name))
      (void) pcpoint_schema_get_dim(pt, from, name, &values[i]);
  }
  if (pj)
  {
    POINT4D p = {0.0, 0.0, 0.0, 0.0};
    bool hasz = from->zdim && to->zdim;
    if (! pcpoint_schema_get_x(pt, from, &p.x) ||
        ! pcpoint_schema_get_y(pt, from, &p.y) ||
        (hasz && ! pcpoint_schema_get_z(pt, from, &p.z)))
    {
      pfree(values);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The schema of pcid %u has no X and Y dimensions",
        pcpoint_get_pcid(pt));
      return NULL;
    }
    if (! point4d_transf_pj(&p, hasz, pj))
    {
      pfree(values);
      return NULL;
    }
    values[to->xdim->position] = p.x;
    values[to->ydim->position] = p.y;
    if (hasz)
      values[to->zdim->position] = p.z;
  }
  for (uint32_t i = 0; i < to->ndims; i++)
  {
    if (! pc_dimension_holds(to->dims[i], values[i]))
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
        "The coordinate %g of dimension %s does not fit the schema of pcid %u",
        values[i], to->dims[i]->name, pcid_to);
      pfree(values);
      return NULL;
    }
  }
  Pcpoint *result = pcpoint_make(pcid_to, values, (int) to->ndims);
  pfree(values);
  return result;
}

/**
 * @ingroup meos_pointcloud_base_srid
 * @brief Return a pcpoint transformed into the schema of another pcid, which
 * states the target SRID
 * @param[in] pt Point
 * @param[in] pcid Target pcid
 * @csqlfn #Pcpoint_transform()
 */
Pcpoint *
pcpoint_transform(const Pcpoint *pt, uint32_t pcid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL);
  const PCSCHEMA *to;
  LWPROJ *pj;
  if (! pc_transform_lookup(pcpoint_get_pcid(pt), pcid, &to, &pj))
    return NULL;
  return pcpoint_transf_pj(pt, pcid, to, pj);
}
