/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Conversion between temporal values and the Arrow C Data Interface
 * @details The Arrow encoding is a length-one `Struct` carrying the temporal
 * value verbatim:
 * @code
 * Struct {
 *   subtype: int8,            -- temporal subtype
 *   interp:  int8,            -- interpolation
 *   flags:   int16,           -- MEOS flags, carried unaltered
 *   srid:    int32,           -- spatial reference, 0 when not spatial
 *   seqs: List< Struct {      -- one element per component sequence
 *     lower_inc: bool,
 *     upper_inc: bool,
 *     insts: List< Struct { t: timestamp[us,UTC], v: <base> } >
 *   } >
 * }
 * @endcode
 * The structural skeleton is the published contract and does not change as
 * base types are added: a new base type only swaps the `v` leaf. Any temporal
 * type, subtype, or interpolation not yet wired raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED rather than producing an approximate or
 * silent result. All temporal float subtypes (instant, sequence, sequence
 * set) and interpolations (discrete, step, linear) are wired; other base
 * types are not yet. The producer owns all memory through arenas freed by
 * the top-level release
 * callbacks (one arena per Arrow object, since the consumer releases the
 * schema and the array independently); child release callbacks are no-ops.
 */

#include "temporal/temporal.h"

/* C */
#include <assert.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/date.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <arrow_c_data_interface.h>
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "temporal/tinstant.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"
#if CBUFFER
  #include <meos_cbuffer.h>
  #include "cbuffer/cbuffer.h"
#endif
#if POSE
  #include <meos_pose.h>
  #include "pose/pose.h"
#endif
#if NPOINT
  #include <meos_npoint.h>
  #include "npoint/tnpoint.h"
#endif
#if RGEO
  #include <meos_rgeo.h>
  #include "rgeo/trgeo_all.h"
#endif
#if POINTCLOUD
  #include <meos_pointcloud.h>
  #include "pointcloud/pcpoint.h"
  #include "pointcloud/pcpatch.h"
#endif

/*****************************************************************************
 * Arena ownership: every allocation for one converted value lives in a
 * singly-linked list of blocks freed once by the top-level release.
 *****************************************************************************/

typedef struct ArrowArena
{
  void **blocks;
  int count;
  int capacity;
} ArrowArena;

static void *
arena_alloc(ArrowArena *arena, size_t size)
{
  void *p = palloc0(size);
  if (arena->count == arena->capacity)
  {
    arena->capacity = arena->capacity ? arena->capacity * 2 : 16;
    arena->blocks = arena->blocks ?
      repalloc(arena->blocks, sizeof(void *) * arena->capacity) :
      palloc(sizeof(void *) * arena->capacity);
  }
  arena->blocks[arena->count++] = p;
  return p;
}

static void
arena_free(ArrowArena *arena)
{
  for (int i = 0; i < arena->count; i++)
    pfree(arena->blocks[i]);
  if (arena->blocks)
    pfree(arena->blocks);
  pfree(arena);
}

/* Child nodes are arena-owned; their release is a no-op. */
static void
noop_schema_release(struct ArrowSchema *s) { s->release = NULL; }
static void
noop_array_release(struct ArrowArray *a) { a->release = NULL; }

static void
root_schema_release(struct ArrowSchema *s)
{
  if (s->private_data)
    arena_free((ArrowArena *) s->private_data);
  s->private_data = NULL;
  s->release = NULL;
}

static void
root_array_release(struct ArrowArray *a)
{
  if (a->private_data)
    arena_free((ArrowArena *) a->private_data);
  a->private_data = NULL;
  a->release = NULL;
}

/*****************************************************************************
 * Schema node builders (arena-allocated). The root struct is the
 * caller-allocated out_schema; every descendant lives in the arena.
 *****************************************************************************/

static struct ArrowSchema *
aw_schema_leaf(ArrowArena *arena, const char *format, const char *name)
{
  struct ArrowSchema *s = arena_alloc(arena, sizeof(struct ArrowSchema));
  s->format = format;
  s->name = name;
  s->metadata = NULL;
  s->flags = 0;
  s->n_children = 0;
  s->children = NULL;
  s->dictionary = NULL;
  s->release = noop_schema_release;
  s->private_data = NULL;
  return s;
}

static struct ArrowSchema *
aw_schema_struct(ArrowArena *arena, const char *name, struct ArrowSchema **kids,
  int64_t n)
{
  struct ArrowSchema *s = aw_schema_leaf(arena, "+s", name);
  s->n_children = n;
  s->children = kids;
  return s;
}

static struct ArrowSchema *
aw_schema_list(ArrowArena *arena, const char *name, struct ArrowSchema *item)
{
  struct ArrowSchema *s = aw_schema_leaf(arena, "+l", name);
  s->n_children = 1;
  s->children = arena_alloc(arena, sizeof(struct ArrowSchema *));
  s->children[0] = item;
  return s;
}

/*****************************************************************************
 * Array node builders (arena-allocated). Validity buffers are NULL because
 * a converted temporal value has no Arrow-level nulls.
 *****************************************************************************/

static struct ArrowArray *
aw_array_node(ArrowArena *arena, int64_t length, int64_t n_buffers,
  int64_t n_children)
{
  struct ArrowArray *a = arena_alloc(arena, sizeof(struct ArrowArray));
  a->length = length;
  a->null_count = 0;
  a->offset = 0;
  a->n_buffers = n_buffers;
  a->n_children = n_children;
  a->buffers = n_buffers ?
    arena_alloc(arena, sizeof(void *) * n_buffers) : NULL;
  a->children = n_children ?
    arena_alloc(arena, sizeof(struct ArrowArray *) * n_children) : NULL;
  a->dictionary = NULL;
  a->release = noop_array_release;
  a->private_data = NULL;
  return a;
}

static struct ArrowArray *
aw_array_prim(ArrowArena *arena, int64_t length, const void *data)
{
  struct ArrowArray *a = aw_array_node(arena, length, 2, 0);
  a->buffers[0] = NULL;
  a->buffers[1] = data;
  return a;
}

/* Arrow utf8 leaf: validity, int32 offsets (length + 1), and the byte data */
static struct ArrowArray *
aw_array_utf8(ArrowArena *arena, int64_t length, const int32_t *offsets,
  const void *data)
{
  struct ArrowArray *a = aw_array_node(arena, length, 3, 0);
  a->buffers[0] = NULL;
  a->buffers[1] = offsets;
  a->buffers[2] = data;
  return a;
}

/* Arrow large binary leaf: validity, int64 offsets (length + 1), byte data */
static struct ArrowArray *
aw_array_largebin(ArrowArena *arena, int64_t length, const int64_t *offsets,
  const void *data)
{
  struct ArrowArray *a = aw_array_node(arena, length, 3, 0);
  a->buffers[0] = NULL;
  a->buffers[1] = offsets;
  a->buffers[2] = data;
  return a;
}

static struct ArrowArray *
aw_array_list(ArrowArena *arena, int64_t length, const int32_t *offsets,
  struct ArrowArray *item)
{
  struct ArrowArray *a = aw_array_node(arena, length, 2, 1);
  a->buffers[0] = NULL;
  a->buffers[1] = offsets;
  a->children[0] = item;
  return a;
}

static struct ArrowArray *
aw_array_struct(ArrowArena *arena, int64_t length, struct ArrowArray **kids,
  int64_t n)
{
  struct ArrowArray *a = aw_array_node(arena, length, 1, n);
  a->buffers[0] = NULL;
  a->children = kids;
  return a;
}

/*****************************************************************************
 * Value-leaf dispatch (the only part of the schema that varies by base type:
 * temporal float decomposes to Float64 "g", temporal integer to Int32 "i",
 * temporal big integer to Int64 "l", temporal H3 index to UInt64 "L",
 * temporal boolean to a bit-packed Boolean "b")
 *****************************************************************************/

/**
 * @brief Return the @p idx-th decomposed scalar value from an Arrow value
 * buffer as a Datum, dispatching on the value-leaf format
 */
static Datum
aw_leaf_datum(const char *vfmt, const void *vvals, int idx)
{
  if (vfmt[0] == 'b')
    return BoolGetDatum((((const uint8_t *) vvals)[idx >> 3] >> (idx & 7)) & 1);
  if (vfmt[0] == 'i')
    return Int32GetDatum(((const int32_t *) vvals)[idx]);
  /* Int64 "l" (big integer) and UInt64 "L" (H3 cell index, binary-identical
   * to int8 in a Datum) share the 64-bit value buffer; only the carried
   * format and the reconstructed temporal type differ. */
  if (vfmt[0] == 'l' || vfmt[0] == 'L')
    return Int64GetDatum(((const int64_t *) vvals)[idx]);
  return Float8GetDatum(((const double *) vvals)[idx]);
}

/**
 * @brief Return the extended WKB of a standalone geometry or geography value
 * @details Mirrors @p geo_as_ewkb (extended NDR form, carrying the SRID and
 * dimensionality losslessly). The public wrapper is not linked into the
 * MobilityDB extension build, so the body is reproduced here over the
 * liblwgeom primitives available in both builds.
 */
static uint8_t *
aw_geo_to_wkb(Datum value, size_t *size)
{
  const GSERIALIZED *gs = (const GSERIALIZED *) DatumGetPointer(value);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  lwvarlena_t *wkb =
    lwgeom_to_wkb_varlena(geom, (uint8_t) (WKB_NDR | WKB_EXTENDED));
  size_t data_size = VARSIZE(wkb) - LWVARHDRSZ;
  uint8_t *result = palloc(data_size);
  memcpy(result, wkb->data, data_size);
  lwgeom_free(geom);
  pfree(wkb);
  *size = data_size;
  return result;
}

/**
 * @brief Reconstruct a geometry or geography from its extended WKB
 * @details Mirrors @p geo_from_wkb_state: the geometry/geography distinction
 * is not encoded in the WKB, so the carried temporal type drives the choice
 * between @p geom_serialize and @p geog_serialize (the latter sets the
 * geodetic flag, which a plain serialization would not).
 */
static GSERIALIZED *
aw_geo_from_wkb(const uint8_t *wkb, size_t wkb_size, bool geodetic)
{
  LWGEOM *geom = lwgeom_from_wkb(wkb, wkb_size, LW_PARSER_CHECK_ALL);
  if (! geom)
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT, "Unable to parse WKB string");
    return NULL;
  }
  if (lwgeom_needs_bbox(geom))
    lwgeom_add_bbox(geom);
  GSERIALIZED *result = geodetic ? geog_serialize(geom) : geom_serialize(geom);
  lwgeom_free(geom);
  return result;
}

#if POINTCLOUD
/**
 * @brief Return the serialized body of a pgPointCloud @p pcpoint or
 * @p pcpatch value
 * @details Mirrors @p pcpoint_to_wkb_buf / @p pcpatch_to_wkb_buf: the
 * canonical encoding is the raw varlena body carried verbatim. The schema
 * id (@c pcid) is resolved out-of-band against pgPointCloud's
 * @c pointcloud_formats catalog, so it is not embedded here. Build-portable:
 * the body is reached with the @c VARSIZE / @c VARDATA varlena macros
 * available in both the MEOS and the MobilityDB extension builds, with no
 * libpc dependency in the serialization itself.
 */
static uint8_t *
aw_pc_to_bytes(Datum value, size_t *size)
{
  const void *vl = DatumGetPointer(value);
  size_t body_len = VARSIZE(vl) - VARHDRSZ;
  uint8_t *result = palloc(body_len ? body_len : 1);
  if (body_len)
    memcpy(result, VARDATA(vl), body_len);
  *size = body_len;
  return result;
}

/**
 * @brief Reconstruct a pgPointCloud @p pcpoint or @p pcpatch varlena from
 * its serialized body
 * @details Mirrors @p pcvarlena_from_wkb_state: wrap the carried body bytes
 * in a fresh varlena header. The receiver resolves @c pcid against the same
 * @c pointcloud_formats catalog.
 */
static void *
aw_pc_from_bytes(const uint8_t *body, size_t body_len)
{
  size_t total = VARHDRSZ + body_len;
  void *vl = palloc(total);
  SET_VARSIZE(vl, total);
  if (body_len)
    memcpy(VARDATA(vl), body, body_len);
  return vl;
}
#endif /* POINTCLOUD */

/**
 * @brief Build the @p idx-th temporal instant from the decomposed Arrow
 * value column, dispatching the point / circular buffer / pose Struct leaf
 * ("+s"), the variable-length utf8 leaf ("u", offsets + bytes) and the
 * opaque large binary geometry leaf ("Z", offsets + EWKB) from the
 * fixed-width scalar leaves
 */
static TInstant *
aw_make_instant(const char *vfmt, MeosType vt, const void *vvals,
  const int32_t *soff, const char *sdata, const int64_t *goff,
  const char *gdata, const double *px, const double *py,
  const double *pz, const double *pr, const double *pth, const double *pqw,
  const double *pqx, const double *pqy, const double *pqz,
  const int64_t *prid, const double *ppos, int32_t srid, bool geodetic,
  int idx, TimestampTz t)
{
#if ! CBUFFER
  (void) pr;   /* the circular-buffer branch is compiled out */
#endif
#if ! POSE
  (void) pth; (void) pqw; (void) pqx; (void) pqy; (void) pqz;
#endif
#if ! NPOINT
  (void) prid; (void) ppos;
#endif
  if (vfmt[0] == 'Z')
  {
    size_t len = (size_t) (goff[idx + 1] - goff[idx]);
#if POINTCLOUD
    if (vt == T_TPCPOINT || vt == T_TPCPATCH)
    {
      void *vl = aw_pc_from_bytes((const uint8_t *) (gdata + goff[idx]), len);
      TInstant *r = tinstant_make(PointerGetDatum(vl), vt, t);
      pfree(vl);
      return r;
    }
#endif
    GSERIALIZED *gs = aw_geo_from_wkb((const uint8_t *) (gdata + goff[idx]),
      len, vt == T_TGEOGRAPHY);
    TInstant *r = tinstant_make(PointerGetDatum(gs), vt, t);
    pfree(gs);
    return r;
  }
  if (vfmt[0] == '+')
  {
#if NPOINT
    if (prid)
    {
      Npoint *np = npoint_make(prid[idx], ppos[idx]);
      TInstant *r = tinstant_make(NpointPGetDatum(np), T_TNPOINT, t);
      pfree(np);
      return r;
    }
#endif
#if CBUFFER
    if (pr)
    {
      GSERIALIZED *cgs =
        geopoint_make(px[idx], py[idx], 0.0, false, false, srid);
      Cbuffer *cb = cbuffer_make(cgs, pr[idx]);
      TInstant *r = tinstant_make(CbufferPGetDatum(cb), T_TCBUFFER, t);
      pfree(cgs);
      pfree(cb);
      return r;
    }
#endif
#if POSE
    if (pth || pqw)
    {
      Pose *po = pqw ?
        pose_make_3d(px[idx], py[idx], pz[idx], pqw[idx], pqx[idx],
          pqy[idx], pqz[idx], srid) :
        pose_make_2d(px[idx], py[idx], pth[idx], srid);
      TInstant *r = tinstant_make(PosePGetDatum(po), T_TPOSE, t);
      pfree(po);
      return r;
    }
#endif
    bool hz = (pz != NULL);
    GSERIALIZED *gs = geopoint_make(px[idx], py[idx], hz ? pz[idx] : 0.0,
      hz, geodetic, srid);
    TInstant *r = tinstant_make(PointerGetDatum(gs),
      geodetic ? T_TGEOGPOINT : T_TGEOMPOINT, t);
    pfree(gs);
    return r;
  }
  if (vfmt[0] == 'u')
  {
    int32_t len = soff[idx + 1] - soff[idx];
    char *cs = palloc((size_t) len + 1);
    if (len)
      memcpy(cs, sdata + soff[idx], (size_t) len);
    cs[len] = '\0';
    text *txt = cstring2text(cs);
    TInstant *r = tinstant_make(PointerGetDatum(txt), T_TTEXT, t);
    pfree(cs);
    pfree(txt);
    return r;
  }
  return tinstant_make(aw_leaf_datum(vfmt, vvals, idx), vt, t);
}

/*****************************************************************************
 * Public conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_conversion
 * @brief Convert a temporal value into Arrow C Data Interface structures
 * @param[in] temp Temporal value
 * @param[out] out_schema,out_array Caller-allocated Arrow structures filled
 * by the producer; release with their @p release callbacks
 * @return True on success; on an unsupported input raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED and returns false
 * @note All temporal float, integer, big integer, boolean, text, point
 * (geometry or geography), circular buffer, pose, network point, general
 * geometry or geography, rigid geometry, H3 index and point cloud subtypes
 * and interpolations are wired; other base types are not yet. The Arrow
 * schema is the full contract.
 */
bool
meos_temporal_to_arrow(const Temporal *temp, struct ArrowSchema *out_schema,
  struct ArrowArray *out_array)
{
  if (! temp || ! out_schema || ! out_array)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG, "Null argument to to_arrow");
    return false;
  }
  bool is_cbuffer = false;
#if CBUFFER
  is_cbuffer = (temp->temptype == T_TCBUFFER);
#endif
  bool is_pose = false;
#if POSE
  is_pose = (temp->temptype == T_TPOSE);
#endif
  bool is_npoint = false;
#if NPOINT
  is_npoint = (temp->temptype == T_TNPOINT);
#endif
  bool is_geo = (temp->temptype == T_TGEOMETRY ||
    temp->temptype == T_TGEOGRAPHY);
  bool is_trgeo = false;
#if RGEO
  is_trgeo = (temp->temptype == T_TRGEOMETRY);
#endif
  bool is_th3index = false;
#if H3
  is_th3index = (temp->temptype == T_TH3INDEX);
#endif
  bool is_pcpoint = false, is_pcpatch = false;
#if POINTCLOUD
  is_pcpoint = (temp->temptype == T_TPCPOINT);
  is_pcpatch = (temp->temptype == T_TPCPATCH);
#endif
  bool is_pc = is_pcpoint || is_pcpatch;
  if (temp->temptype != T_TFLOAT && temp->temptype != T_TINT &&
      temp->temptype != T_TBIGINT &&
      temp->temptype != T_TBOOL && temp->temptype != T_TTEXT &&
      temp->temptype != T_TGEOMPOINT && temp->temptype != T_TGEOGPOINT &&
      ! is_cbuffer && ! is_pose && ! is_npoint && ! is_geo && ! is_trgeo &&
      ! is_th3index && ! is_pc)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_to_arrow: only temporal float, integer, big integer, "
      "boolean, text, point, circular buffer, pose, network point, general "
      "geometry or geography, rigid geometry, H3 index and point cloud are "
      "wired");
    return false;
  }
  bool is_tint = (temp->temptype == T_TINT);
  bool is_tbigint = (temp->temptype == T_TBIGINT);
  bool is_tbool = (temp->temptype == T_TBOOL);
  bool is_ttext = (temp->temptype == T_TTEXT);
  bool is_point = (temp->temptype == T_TGEOMPOINT ||
    temp->temptype == T_TGEOGPOINT);
  /* Z applies to a point or a 3D pose/rigid geometry (3D carries a
   * quaternion). A rigid geometry's per-instant value is a pose. */
  bool has_z = (is_point || is_pose || is_trgeo) &&
    MEOS_FLAGS_GET_Z(temp->flags);
  bool is_pose3d = is_pose && has_z;
  bool is_trgeo3d = is_trgeo && has_z;
  /* Point → Struct{x,y,z?}, circular buffer → Struct{x,y,r}, pose →
   * Struct{x,y,theta} (2D) or Struct{x,y,z,W,X,Y,Z} (3D), network point →
   * Struct{rid,pos}, rigid geometry → Struct{ref (LargeBinary EWKB of the
   * shared reference geometry), then the pose fields}, all the "+s" value
   * leaf; a general geometry or geography → an opaque LargeBinary "Z" leaf
   * of its EWKB; a point cloud point or patch → an opaque LargeBinary "Z"
   * leaf of its serialized varlena body (named "pcpoint"/"pcpatch" to
   * discriminate it from the geometry "Z" leaf); the scalar tier uses a
   * single primitive leaf. */
  const char *vfmt = (is_point || is_cbuffer || is_pose || is_npoint ||
    is_trgeo) ? "+s" : ((is_geo || is_pc) ? "Z" : (is_ttext ? "u" :
    (is_tbool ? "b" : (is_tint ? "i" : (is_tbigint ? "l" :
    (is_th3index ? "L" : "g"))))));

  /* Separate arenas: the schema and the array are independently released
   * by the consumer, so each must own (and free) its own allocations. */
  ArrowArena *arena_s = palloc0(sizeof(ArrowArena));
  ArrowArena *arena_a = palloc0(sizeof(ArrowArena));

  /* Schema: the full nested contract. The value leaf "g" is the only part
   * that varies by base type; everything else is fixed structure. */
  struct ArrowSchema *inst_kids[2];
  inst_kids[0] = aw_schema_leaf(arena_s, "tsu:UTC", "t");
  if (is_point)
  {
    int nxyz = has_z ? 3 : 2;
    struct ArrowSchema **xyz =
      arena_alloc(arena_s, sizeof(struct ArrowSchema *) * nxyz);
    xyz[0] = aw_schema_leaf(arena_s, "g", "x");
    xyz[1] = aw_schema_leaf(arena_s, "g", "y");
    if (has_z)
      xyz[2] = aw_schema_leaf(arena_s, "g", "z");
    inst_kids[1] = aw_schema_struct(arena_s, "v", xyz, nxyz);
  }
  else if (is_cbuffer)
  {
    struct ArrowSchema **xyr =
      arena_alloc(arena_s, sizeof(struct ArrowSchema *) * 3);
    xyr[0] = aw_schema_leaf(arena_s, "g", "x");
    xyr[1] = aw_schema_leaf(arena_s, "g", "y");
    xyr[2] = aw_schema_leaf(arena_s, "g", "r");
    inst_kids[1] = aw_schema_struct(arena_s, "v", xyr, 3);
  }
  else if (is_pose)
  {
    int npose = is_pose3d ? 7 : 3;
    struct ArrowSchema **pp =
      arena_alloc(arena_s, sizeof(struct ArrowSchema *) * npose);
    pp[0] = aw_schema_leaf(arena_s, "g", "x");
    pp[1] = aw_schema_leaf(arena_s, "g", "y");
    if (is_pose3d)
    {
      pp[2] = aw_schema_leaf(arena_s, "g", "z");
      pp[3] = aw_schema_leaf(arena_s, "g", "W");
      pp[4] = aw_schema_leaf(arena_s, "g", "X");
      pp[5] = aw_schema_leaf(arena_s, "g", "Y");
      pp[6] = aw_schema_leaf(arena_s, "g", "Z");
    }
    else
      pp[2] = aw_schema_leaf(arena_s, "g", "theta");
    inst_kids[1] = aw_schema_struct(arena_s, "v", pp, npose);
  }
  else if (is_npoint)
  {
    struct ArrowSchema **rp =
      arena_alloc(arena_s, sizeof(struct ArrowSchema *) * 2);
    rp[0] = aw_schema_leaf(arena_s, "l", "rid");
    rp[1] = aw_schema_leaf(arena_s, "g", "pos");
    inst_kids[1] = aw_schema_struct(arena_s, "v", rp, 2);
  }
  else if (is_trgeo)
  {
    /* Rigid geometry → Struct{ref:LargeBinary, then the pose fields}; the
     * leading child name "ref" discriminates it on read. */
    int nrg = is_trgeo3d ? 8 : 4;
    struct ArrowSchema **rg =
      arena_alloc(arena_s, sizeof(struct ArrowSchema *) * nrg);
    rg[0] = aw_schema_leaf(arena_s, "Z", "ref");
    rg[1] = aw_schema_leaf(arena_s, "g", "x");
    rg[2] = aw_schema_leaf(arena_s, "g", "y");
    if (is_trgeo3d)
    {
      rg[3] = aw_schema_leaf(arena_s, "g", "z");
      rg[4] = aw_schema_leaf(arena_s, "g", "W");
      rg[5] = aw_schema_leaf(arena_s, "g", "X");
      rg[6] = aw_schema_leaf(arena_s, "g", "Y");
      rg[7] = aw_schema_leaf(arena_s, "g", "Z");
    }
    else
      rg[3] = aw_schema_leaf(arena_s, "g", "theta");
    inst_kids[1] = aw_schema_struct(arena_s, "v", rg, nrg);
  }
  else
    inst_kids[1] = aw_schema_leaf(arena_s, vfmt,
      is_pcpoint ? "pcpoint" : (is_pcpatch ? "pcpatch" : "v"));
  struct ArrowSchema *inst_st =
    aw_schema_struct(arena_s, "item", inst_kids, 2);
  struct ArrowSchema *insts_list = aw_schema_list(arena_s, "insts", inst_st);

  struct ArrowSchema *seq_kids[3];
  seq_kids[0] = aw_schema_leaf(arena_s, "b", "lower_inc");
  seq_kids[1] = aw_schema_leaf(arena_s, "b", "upper_inc");
  seq_kids[2] = insts_list;
  struct ArrowSchema *seq_st = aw_schema_struct(arena_s, "item", seq_kids, 3);
  struct ArrowSchema *seqs_list = aw_schema_list(arena_s, "seqs", seq_st);

  struct ArrowSchema **top_sc =
    arena_alloc(arena_s, sizeof(struct ArrowSchema *) * 5);
  top_sc[0] = aw_schema_leaf(arena_s, "c", "subtype");
  top_sc[1] = aw_schema_leaf(arena_s, "c", "interp");
  top_sc[2] = aw_schema_leaf(arena_s, "s", "flags");
  top_sc[3] = aw_schema_leaf(arena_s, "i", "srid");
  top_sc[4] = seqs_list;

  out_schema->format = "+s";
  out_schema->name = NULL;
  out_schema->metadata = NULL;
  out_schema->flags = 0;
  out_schema->n_children = 5;
  out_schema->children = top_sc;
  out_schema->dictionary = NULL;
  out_schema->private_data = arena_s;
  out_schema->release = root_schema_release;

  /* Scalar header buffers (length 1: one temporal value) */
  int8_t *subtype_b = arena_alloc(arena_a, sizeof(int8_t));
  int8_t *interp_b = arena_alloc(arena_a, sizeof(int8_t));
  int16_t *flags_b = arena_alloc(arena_a, sizeof(int16_t));
  int32_t *srid_b = arena_alloc(arena_a, sizeof(int32_t));
  subtype_b[0] = (int8_t) temp->subtype;
  interp_b[0] = (int8_t) MEOS_FLAGS_GET_INTERP(temp->flags);
  flags_b[0] = (int16_t) temp->flags;
  srid_b[0] = (is_point || is_cbuffer || is_pose || is_trgeo) ?
    (int32_t) tspatial_srid(temp) : 0;

  /* Decompose into component sequences. The skeleton is uniform: an instant
   * is one pseudo-sequence of one instant; a sequence is one sequence; a
   * sequence set is its component sequences. */
  int nseqs, total;
  const TInstant *single = NULL;
  const TSequence **comp = NULL;
  if (temp->subtype == TINSTANT)
  {
    single = (const TInstant *) temp;
    nseqs = 1;
    total = 1;
  }
  else if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    nseqs = 1;
    total = seq->count;
    comp = arena_alloc(arena_a, sizeof(TSequence *));
    comp[0] = seq;
  }
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    nseqs = ss->count;
    total = ss->totalcount;
    comp = arena_alloc(arena_a, sizeof(TSequence *) * nseqs);
    for (int j = 0; j < nseqs; j++)
      comp[j] = TSEQUENCESET_SEQ_N(ss, j);
  }

  /* Per-sequence bound bitmaps (Arrow boolean is one bit per element),
   * the two levels of list offsets, and the flattened instant columns. */
  uint8_t *lower_bm = arena_alloc(arena_a, (nseqs + 7) / 8);
  uint8_t *upper_bm = arena_alloc(arena_a, (nseqs + 7) / 8);
  int32_t *seqs_off = arena_alloc(arena_a, sizeof(int32_t) * 2);
  seqs_off[0] = 0;
  seqs_off[1] = nseqs;
  int32_t *insts_off = arena_alloc(arena_a, sizeof(int32_t) * (nseqs + 1));
  insts_off[0] = 0;
  int64_t *tvals = arena_alloc(arena_a, sizeof(int64_t) * (total ? total : 1));
  int vn = total ? total : 1;
  size_t vsz = (is_ttext || is_point || is_cbuffer || is_pose || is_geo ||
    is_trgeo || is_pc) ? 1 : (is_tbool ? (size_t) ((vn + 7) / 8) :
    (is_tint ? sizeof(int32_t) : ((is_tbigint || is_th3index) ?
    sizeof(int64_t) : sizeof(double))) * (size_t) vn);
  void *vvals = arena_alloc(arena_a, vsz);
  /* Temporal text decomposes to a variable-length utf8 column: collect the
   * instant strings during the walk, assemble offsets and bytes afterwards. */
  char **svals = is_ttext ? palloc(sizeof(char *) * vn) : NULL;
  /* Point → x,y,z? columns; circular buffer → x,y (2D centre) and r;
   * pose → x,y and theta (2D), or x,y,z and quaternion W,X,Y,Z (3D). */
  bool xy = (is_point || is_cbuffer || is_pose || is_trgeo);
  bool any3d = is_pose3d || is_trgeo3d;
  bool any2dpose = (is_pose && ! is_pose3d) || (is_trgeo && ! is_trgeo3d);
  double *px = xy ? arena_alloc(arena_a, sizeof(double) * vn) : NULL;
  double *py = xy ? arena_alloc(arena_a, sizeof(double) * vn) : NULL;
  double *pz = ((is_point && has_z) || any3d) ?
    arena_alloc(arena_a, sizeof(double) * vn) : NULL;
  double *pr = is_cbuffer ?
    arena_alloc(arena_a, sizeof(double) * vn) : NULL;
  double *pth = any2dpose ?
    arena_alloc(arena_a, sizeof(double) * vn) : NULL;
  double *pqw = any3d ? arena_alloc(arena_a, sizeof(double) * vn) : NULL;
  double *pqx = any3d ? arena_alloc(arena_a, sizeof(double) * vn) : NULL;
  double *pqy = any3d ? arena_alloc(arena_a, sizeof(double) * vn) : NULL;
  double *pqz = any3d ? arena_alloc(arena_a, sizeof(double) * vn) : NULL;
  /* Network point → parallel Int64 rid and Float64 pos columns. */
  int64_t *prid = is_npoint ?
    arena_alloc(arena_a, sizeof(int64_t) * vn) : NULL;
  double *ppos = is_npoint ?
    arena_alloc(arena_a, sizeof(double) * vn) : NULL;
  /* General geometry or geography → per-instant EWKB; point cloud point or
   * patch → the per-instant serialized varlena body; rigid geometry → the
   * shared reference geometry's EWKB replicated per instant (the leaf is
   * per-instant uniform). Collected during the walk, then assembled into an
   * int64-offset LargeBinary column. */
  uint8_t **gbufs = (is_geo || is_trgeo || is_pc) ?
    palloc(sizeof(uint8_t *) * vn) : NULL;
  size_t *glens = (is_geo || is_trgeo || is_pc) ?
    palloc(sizeof(size_t) * vn) : NULL;
  /* The rigid-geometry reference is constant across the value: serialize it
   * once and replicate the bytes per instant. */
  uint8_t *refwkb = NULL;
  size_t reflen = 0;
#if RGEO
  if (is_trgeo)
    refwkb = aw_geo_to_wkb(PointerGetDatum(trgeo_geom_p(temp)), &reflen);
#else
  (void) refwkb; (void) reflen;
#endif
  int k = 0;
  for (int j = 0; j < nseqs; j++)
  {
    bool lo, up;
    if (single)
    {
      lo = up = true;
      tvals[k] = (int64_t) single->t;
      if (is_point)
      {
        Datum pd = tinstant_value_p(single);
        if (has_z)
        {
          const POINT3DZ *p = DATUM_POINT3DZ_P(pd);
          px[k] = p->x; py[k] = p->y; pz[k] = p->z;
        }
        else
        {
          const POINT2D *p = DATUM_POINT2D_P(pd);
          px[k] = p->x; py[k] = p->y;
        }
      }
      else if (is_cbuffer)
      {
#if CBUFFER
        Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(single));
        const POINT2D *cp =
          (const POINT2D *) GS_POINT_PTR(cbuffer_point_p(cb));
        px[k] = cp->x; py[k] = cp->y; pr[k] = cbuffer_radius(cb);
#endif
      }
      else if (is_pose)
      {
#if POSE
        const Pose *po = DatumGetPoseP(tinstant_value_p(single));
        px[k] = po->data[0]; py[k] = po->data[1];
        if (is_pose3d)
        {
          pz[k] = po->data[2]; pqw[k] = po->data[3];
          pqx[k] = po->data[4]; pqy[k] = po->data[5]; pqz[k] = po->data[6];
        }
        else
          pth[k] = po->data[2];
#endif
      }
      else if (is_npoint)
      {
#if NPOINT
        const Npoint *np = DatumGetNpointP(tinstant_value_p(single));
        prid[k] = npoint_route(np); ppos[k] = npoint_position(np);
#endif
      }
      else if (is_geo)
        gbufs[k] = aw_geo_to_wkb(tinstant_value_p(single), &glens[k]);
      else if (is_pc)
      {
#if POINTCLOUD
        gbufs[k] = aw_pc_to_bytes(tinstant_value_p(single), &glens[k]);
#endif
      }
      else if (is_trgeo)
      {
#if RGEO
        const Pose *po = DatumGetPoseP(tinstant_value_p(single));
        px[k] = po->data[0]; py[k] = po->data[1];
        if (is_trgeo3d)
        {
          pz[k] = po->data[2]; pqw[k] = po->data[3];
          pqx[k] = po->data[4]; pqy[k] = po->data[5]; pqz[k] = po->data[6];
        }
        else
          pth[k] = po->data[2];
        gbufs[k] = palloc(reflen ? reflen : 1);
        if (reflen)
          memcpy(gbufs[k], refwkb, reflen);
        glens[k] = reflen;
#endif
      }
      else if (is_ttext)
        svals[k] = text2cstring(DatumGetTextP(tinstant_value_p(single)));
      else if (is_tbool)
      {
        if (DatumGetBool(tinstant_value_p(single)))
          ((uint8_t *) vvals)[k >> 3] |= (uint8_t) (1 << (k & 7));
      }
      else if (is_tint)
        ((int32_t *) vvals)[k] = DatumGetInt32(tinstant_value_p(single));
      else if (is_tbigint || is_th3index)
        /* The H3 cell index is a uint64 binary-identical to int8, so the
         * raw 64 bits move through the same path as a big integer. */
        ((int64_t *) vvals)[k] = DatumGetInt64(tinstant_value_p(single));
      else
        ((double *) vvals)[k] = DatumGetFloat8(tinstant_value_p(single));
      k++;
    }
    else
    {
      const TSequence *seq = comp[j];
      lo = seq->period.lower_inc;
      up = seq->period.upper_inc;
      for (int i = 0; i < seq->count; i++)
      {
        const TInstant *inst = TSEQUENCE_INST_N(seq, i);
        tvals[k] = (int64_t) inst->t;
        if (is_point)
        {
          Datum pd = tinstant_value_p(inst);
          if (has_z)
          {
            const POINT3DZ *p = DATUM_POINT3DZ_P(pd);
            px[k] = p->x; py[k] = p->y; pz[k] = p->z;
          }
          else
          {
            const POINT2D *p = DATUM_POINT2D_P(pd);
            px[k] = p->x; py[k] = p->y;
          }
        }
        else if (is_cbuffer)
        {
#if CBUFFER
          Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(inst));
          const POINT2D *cp =
            (const POINT2D *) GS_POINT_PTR(cbuffer_point_p(cb));
          px[k] = cp->x; py[k] = cp->y; pr[k] = cbuffer_radius(cb);
#endif
        }
        else if (is_pose)
        {
#if POSE
          const Pose *po = DatumGetPoseP(tinstant_value_p(inst));
          px[k] = po->data[0]; py[k] = po->data[1];
          if (is_pose3d)
          {
            pz[k] = po->data[2]; pqw[k] = po->data[3];
            pqx[k] = po->data[4]; pqy[k] = po->data[5];
            pqz[k] = po->data[6];
          }
          else
            pth[k] = po->data[2];
#endif
        }
        else if (is_npoint)
        {
#if NPOINT
          const Npoint *np = DatumGetNpointP(tinstant_value_p(inst));
          prid[k] = npoint_route(np); ppos[k] = npoint_position(np);
#endif
        }
        else if (is_geo)
          gbufs[k] = aw_geo_to_wkb(tinstant_value_p(inst), &glens[k]);
        else if (is_pc)
        {
#if POINTCLOUD
          gbufs[k] = aw_pc_to_bytes(tinstant_value_p(inst), &glens[k]);
#endif
        }
        else if (is_trgeo)
        {
#if RGEO
          const Pose *po = DatumGetPoseP(tinstant_value_p(inst));
          px[k] = po->data[0]; py[k] = po->data[1];
          if (is_trgeo3d)
          {
            pz[k] = po->data[2]; pqw[k] = po->data[3];
            pqx[k] = po->data[4]; pqy[k] = po->data[5];
            pqz[k] = po->data[6];
          }
          else
            pth[k] = po->data[2];
          gbufs[k] = palloc(reflen ? reflen : 1);
          if (reflen)
            memcpy(gbufs[k], refwkb, reflen);
          glens[k] = reflen;
#endif
        }
        else if (is_ttext)
          svals[k] = text2cstring(DatumGetTextP(tinstant_value_p(inst)));
        else if (is_tbool)
        {
          if (DatumGetBool(tinstant_value_p(inst)))
            ((uint8_t *) vvals)[k >> 3] |= (uint8_t) (1 << (k & 7));
        }
        else if (is_tint)
          ((int32_t *) vvals)[k] = DatumGetInt32(tinstant_value_p(inst));
        else if (is_tbigint || is_th3index)
          ((int64_t *) vvals)[k] = DatumGetInt64(tinstant_value_p(inst));
        else
          ((double *) vvals)[k] = DatumGetFloat8(tinstant_value_p(inst));
        k++;
      }
    }
    if (lo) lower_bm[j >> 3] |= (uint8_t) (1 << (j & 7));
    if (up) upper_bm[j >> 3] |= (uint8_t) (1 << (j & 7));
    insts_off[j + 1] = k;
  }

  /* Assemble the utf8 column: int32 offsets (total + 1) then the byte data */
  int32_t *str_off = NULL;
  char *str_data = NULL;
  if (is_ttext)
  {
    str_off = arena_alloc(arena_a, sizeof(int32_t) * (total + 1));
    int32_t acc = 0;
    for (int i = 0; i < total; i++)
    {
      str_off[i] = acc;
      acc += (int32_t) strlen(svals[i]);
    }
    str_off[total] = acc;
    str_data = arena_alloc(arena_a, acc ? (size_t) acc : 1);
    for (int i = 0; i < total; i++)
    {
      int32_t len = str_off[i + 1] - str_off[i];
      if (len)
        memcpy(str_data + str_off[i], svals[i], (size_t) len);
      pfree(svals[i]);
    }
    pfree(svals);
  }

  /* Assemble the LargeBinary column: int64 offsets (total + 1) then the
   * concatenated per-instant bytes (EWKB for a geometry, the serialized
   * varlena body for a point cloud value). For a rigid geometry this is the
   * reference-geometry "ref" child of the value struct. */
  int64_t *geo_off = NULL;
  char *geo_data = NULL;
  if (is_geo || is_trgeo || is_pc)
  {
    geo_off = arena_alloc(arena_a, sizeof(int64_t) * (total + 1));
    int64_t acc = 0;
    for (int i = 0; i < total; i++)
    {
      geo_off[i] = acc;
      acc += (int64_t) glens[i];
    }
    geo_off[total] = acc;
    geo_data = arena_alloc(arena_a, acc ? (size_t) acc : 1);
    for (int i = 0; i < total; i++)
    {
      if (glens[i])
        memcpy(geo_data + geo_off[i], gbufs[i], glens[i]);
      pfree(gbufs[i]);
    }
    pfree(gbufs);
    pfree(glens);
#if RGEO
    if (refwkb)
      pfree(refwkb);
#endif
  }

  /* Array tree mirroring the schema */
  struct ArrowArray **inst_a_kids =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 2);
  inst_a_kids[0] = aw_array_prim(arena_a, total, tvals);
  if (is_point)
  {
    int nxyz = has_z ? 3 : 2;
    struct ArrowArray **xyz =
      arena_alloc(arena_a, sizeof(struct ArrowArray *) * nxyz);
    xyz[0] = aw_array_prim(arena_a, total, px);
    xyz[1] = aw_array_prim(arena_a, total, py);
    if (has_z)
      xyz[2] = aw_array_prim(arena_a, total, pz);
    inst_a_kids[1] = aw_array_struct(arena_a, total, xyz, nxyz);
  }
  else if (is_cbuffer)
  {
    struct ArrowArray **xyr =
      arena_alloc(arena_a, sizeof(struct ArrowArray *) * 3);
    xyr[0] = aw_array_prim(arena_a, total, px);
    xyr[1] = aw_array_prim(arena_a, total, py);
    xyr[2] = aw_array_prim(arena_a, total, pr);
    inst_a_kids[1] = aw_array_struct(arena_a, total, xyr, 3);
  }
  else if (is_pose)
  {
    int npose = is_pose3d ? 7 : 3;
    struct ArrowArray **pp =
      arena_alloc(arena_a, sizeof(struct ArrowArray *) * npose);
    pp[0] = aw_array_prim(arena_a, total, px);
    pp[1] = aw_array_prim(arena_a, total, py);
    if (is_pose3d)
    {
      pp[2] = aw_array_prim(arena_a, total, pz);
      pp[3] = aw_array_prim(arena_a, total, pqw);
      pp[4] = aw_array_prim(arena_a, total, pqx);
      pp[5] = aw_array_prim(arena_a, total, pqy);
      pp[6] = aw_array_prim(arena_a, total, pqz);
    }
    else
      pp[2] = aw_array_prim(arena_a, total, pth);
    inst_a_kids[1] = aw_array_struct(arena_a, total, pp, npose);
  }
  else if (is_npoint)
  {
    struct ArrowArray **rp =
      arena_alloc(arena_a, sizeof(struct ArrowArray *) * 2);
    rp[0] = aw_array_prim(arena_a, total, prid);
    rp[1] = aw_array_prim(arena_a, total, ppos);
    inst_a_kids[1] = aw_array_struct(arena_a, total, rp, 2);
  }
  else if (is_trgeo)
  {
    int nrg = is_trgeo3d ? 8 : 4;
    struct ArrowArray **rg =
      arena_alloc(arena_a, sizeof(struct ArrowArray *) * nrg);
    rg[0] = aw_array_largebin(arena_a, total, geo_off, geo_data);
    rg[1] = aw_array_prim(arena_a, total, px);
    rg[2] = aw_array_prim(arena_a, total, py);
    if (is_trgeo3d)
    {
      rg[3] = aw_array_prim(arena_a, total, pz);
      rg[4] = aw_array_prim(arena_a, total, pqw);
      rg[5] = aw_array_prim(arena_a, total, pqx);
      rg[6] = aw_array_prim(arena_a, total, pqy);
      rg[7] = aw_array_prim(arena_a, total, pqz);
    }
    else
      rg[3] = aw_array_prim(arena_a, total, pth);
    inst_a_kids[1] = aw_array_struct(arena_a, total, rg, nrg);
  }
  else
    inst_a_kids[1] = (is_geo || is_pc) ?
      aw_array_largebin(arena_a, total, geo_off, geo_data) :
      (is_ttext ? aw_array_utf8(arena_a, total, str_off, str_data) :
      aw_array_prim(arena_a, total, vvals));
  struct ArrowArray *inst_st_a =
    aw_array_struct(arena_a, total, inst_a_kids, 2);
  struct ArrowArray *insts_list_a =
    aw_array_list(arena_a, nseqs, insts_off, inst_st_a);

  struct ArrowArray **seq_a_kids =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 3);
  seq_a_kids[0] = aw_array_prim(arena_a, nseqs, lower_bm);
  seq_a_kids[1] = aw_array_prim(arena_a, nseqs, upper_bm);
  seq_a_kids[2] = insts_list_a;
  struct ArrowArray *seq_st_a = aw_array_struct(arena_a, nseqs, seq_a_kids, 3);
  struct ArrowArray *seqs_list_a =
    aw_array_list(arena_a, 1, seqs_off, seq_st_a);

  struct ArrowArray **top_a =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 5);
  top_a[0] = aw_array_prim(arena_a, 1, subtype_b);
  top_a[1] = aw_array_prim(arena_a, 1, interp_b);
  top_a[2] = aw_array_prim(arena_a, 1, flags_b);
  top_a[3] = aw_array_prim(arena_a, 1, srid_b);
  top_a[4] = seqs_list_a;

  out_array->length = 1;
  out_array->null_count = 0;
  out_array->offset = 0;
  out_array->n_buffers = 1;
  out_array->n_children = 5;
  out_array->buffers = arena_alloc(arena_a, sizeof(void *) * 1);
  out_array->buffers[0] = NULL;
  out_array->children = top_a;
  out_array->dictionary = NULL;
  out_array->private_data = arena_a;
  out_array->release = root_array_release;
  return true;
}

/**
 * @ingroup meos_temporal_conversion
 * @brief Reconstruct a temporal value from Arrow C Data Interface structures
 * @param[in] schema,array Arrow structures produced by
 * #meos_temporal_to_arrow
 * @return New temporal value, or NULL on an unsupported layout
 * (raises @p MEOS_ERR_FEATURE_NOT_SUPPORTED)
 */
Temporal *
meos_temporal_from_arrow(const struct ArrowSchema *schema,
  const struct ArrowArray *array)
{
  if (! schema || ! array || ! schema->format ||
      strcmp(schema->format, "+s") != 0 || schema->n_children != 5 ||
      array->n_children != 5 || array->length != 1)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: unsupported Arrow layout");
    return NULL;
  }

  /* The base-type leaf gates this: an unrecognised value column routes to
   * NOT_SUPPORTED rather than being misread. Float64 "g" is temporal float,
   * Int32 "i" is temporal integer, Int64 "l" is temporal big integer,
   * UInt64 "L" is temporal H3 index, Boolean "b" is temporal boolean, Utf8
   * "u" is temporal text, Struct "+s" is a temporal point ({x,y,z?}, geom
   * or geog from the carried flags word), a temporal circular buffer
   * ({x,y,r}), a temporal pose ({x,y,theta} 2D or {x,y,z,W,X,Y,Z} 3D) or a
   * temporal network point ({rid,pos}) or a temporal rigid geometry
   * ({ref,...pose}), LargeBinary "Z" is a general temporal geometry or
   * geography carrying each instant's EWKB verbatim (geometry or geography
   * from the carried flags word); the first child name ("rid"/"ref") and
   * the third child name ("r"/"theta"/"z") with the child count
   * disambiguate; the srid slot is the spatial SRID, non-zero allowed only
   * for a struct leaf (network point and the LargeBinary geometry leaf keep
   * it zero, the SRID travelling inside the EWKB). */
  const struct ArrowSchema *v_sc = schema->children[4]    /* seqs list */
    ->children[0]                                          /* seq struct */
    ->children[2]                                          /* insts list */
    ->children[0]                                          /* inst struct */
    ->children[1];                                         /* v leaf */
  int8_t subtype = ((const int8_t *) array->children[0]->buffers[1])[0];
  int8_t interp = ((const int8_t *) array->children[1]->buffers[1])[0];
  int16_t flags = ((const int16_t *) array->children[2]->buffers[1])[0];
  int32_t srid = ((const int32_t *) array->children[3]->buffers[1])[0];
  bool v_is_struct = v_sc->format && strcmp(v_sc->format, "+s") == 0;
  bool v_is_largebin = v_sc->format && strcmp(v_sc->format, "Z") == 0;
  bool v_is_th3index = v_sc->format && strcmp(v_sc->format, "L") == 0;
  /* A "Z" LargeBinary leaf is a point cloud value when its leaf name is
   * "pcpoint"/"pcpatch" (the discriminator, symmetric with the struct
   * child-name discriminators), otherwise a general geometry or geography. */
  const char *vname = (v_is_largebin && v_sc->name) ? v_sc->name : "";
  bool v_is_pcpoint = v_is_largebin && strcmp(vname, "pcpoint") == 0;
  bool v_is_pcpatch = v_is_largebin && strcmp(vname, "pcpatch") == 0;
  bool v_is_pc = v_is_pcpoint || v_is_pcpatch;
  bool v_is_geo = v_is_largebin && ! v_is_pc;
  /* Among struct value leaves: first child "rid" → network point; third
   * child "r" → circular buffer, "theta" → 2D pose, "z" → 3D point; a
   * 7-child struct → 3D pose; otherwise a point. */
  const char *c0 = (v_is_struct && v_sc->n_children >= 1 &&
    v_sc->children[0]->name) ? v_sc->children[0]->name : "";
  const char *c2 = (v_is_struct && v_sc->n_children >= 3 &&
    v_sc->children[2]->name) ? v_sc->children[2]->name : "";
  bool v_is_npoint = v_is_struct && v_sc->n_children == 2 &&
    strcmp(c0, "rid") == 0;
  bool v_is_cbuffer = v_is_struct && v_sc->n_children == 3 &&
    strcmp(c2, "r") == 0;
  bool v_is_pose = v_is_struct &&
    ((v_sc->n_children == 3 && strcmp(c2, "theta") == 0) ||
     v_sc->n_children == 7);
  bool v_is_pose3d = v_is_pose && v_sc->n_children == 7;
  /* Rigid geometry: a struct whose leading child is the "ref" LargeBinary
   * of the shared reference geometry, followed by the pose fields (4
   * children = 2D, 8 = 3D). */
  bool v_is_trgeo = v_is_struct && v_sc->n_children >= 4 &&
    strcmp(c0, "ref") == 0;
  bool v_is_trgeo3d = v_is_trgeo && v_sc->n_children == 8;
  bool v_is_point = v_is_struct && ! v_is_cbuffer && ! v_is_pose &&
    ! v_is_npoint && ! v_is_trgeo;
  if (! v_sc->format ||
      (! v_is_struct && strcmp(v_sc->format, "g") != 0 &&
       strcmp(v_sc->format, "i") != 0 && strcmp(v_sc->format, "l") != 0 &&
       strcmp(v_sc->format, "L") != 0 && strcmp(v_sc->format, "b") != 0 &&
       strcmp(v_sc->format, "u") != 0 && strcmp(v_sc->format, "Z") != 0) ||
      (! v_is_struct && ! v_is_largebin && srid != 0) ||
      (subtype != TINSTANT && subtype != TSEQUENCE &&
       subtype != TSEQUENCESET))
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: only temporal float, integer, big integer, "
      "boolean, text, point, circular buffer, pose, network point, general "
      "geometry or geography, rigid geometry, H3 index and point cloud are "
      "wired");
    return NULL;
  }
#if ! POINTCLOUD
  if (v_is_pc)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: point cloud support is not built");
    return NULL;
  }
#endif
#if ! CBUFFER
  if (v_is_cbuffer)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: circular buffer support is not built");
    return NULL;
  }
#endif
#if ! POSE
  if (v_is_pose)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: pose support is not built");
    return NULL;
  }
#endif
#if ! NPOINT
  if (v_is_npoint)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: network point support is not built");
    return NULL;
  }
#endif
#if ! RGEO
  if (v_is_trgeo)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: rigid geometry support is not built");
    return NULL;
  }
#endif
#if ! H3
  if (v_is_th3index)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: H3 index support is not built");
    return NULL;
  }
#endif
  /* A rigid geometry reconstructs as a temporal pose, then wraps it with
   * the shared reference geometry; the inner build uses the pose path. A
   * point cloud value reconstructs directly from its serialized varlena
   * body, the leaf name selecting the temporal point cloud type. */
  MeosType vt = v_is_trgeo ? T_TPOSE :
    (v_is_pcpoint ? T_TPCPOINT : (v_is_pcpatch ? T_TPCPATCH :
    (v_is_geo ?
    (MEOS_FLAGS_GET_GEODETIC(flags) ? T_TGEOGRAPHY : T_TGEOMETRY) :
    ((v_sc->format[0] == 'u') ? T_TTEXT :
    ((v_sc->format[0] == 'b') ? T_TBOOL :
    ((v_sc->format[0] == 'i') ? T_TINT :
    ((v_sc->format[0] == 'l') ? T_TBIGINT :
    ((v_sc->format[0] == 'L') ? T_TH3INDEX : T_TFLOAT))))))));
  bool geodetic = v_is_point && MEOS_FLAGS_GET_GEODETIC(flags);

  const struct ArrowArray *seq_st_a = array->children[4]->children[0];
  const uint8_t *lower_bm =
    (const uint8_t *) seq_st_a->children[0]->buffers[1];
  const uint8_t *upper_bm =
    (const uint8_t *) seq_st_a->children[1]->buffers[1];
  const struct ArrowArray *insts_a = seq_st_a->children[2];
  const int32_t *insts_off = (const int32_t *) insts_a->buffers[1];
  const struct ArrowArray *inst_st_a = insts_a->children[0];
  const int64_t *tvals = (const int64_t *) inst_st_a->children[0]->buffers[1];
  const void *vvals = inst_st_a->children[1]->buffers[1];
  const int32_t *soff = (v_sc->format[0] == 'u') ?
    (const int32_t *) inst_st_a->children[1]->buffers[1] : NULL;
  const char *sdata = (v_sc->format[0] == 'u') ?
    (const char *) inst_st_a->children[1]->buffers[2] : NULL;
  const int64_t *goff = v_is_largebin ?
    (const int64_t *) inst_st_a->children[1]->buffers[1] : NULL;
  const char *gdata = v_is_largebin ?
    (const char *) inst_st_a->children[1]->buffers[2] : NULL;
  const struct ArrowArray *vstruct = inst_st_a->children[1];
  bool v_xy = v_is_struct && ! v_is_npoint && ! v_is_trgeo;
  const double *px = v_xy ?
    (const double *) vstruct->children[0]->buffers[1] : NULL;
  const double *py = v_xy ?
    (const double *) vstruct->children[1]->buffers[1] : NULL;
  const int64_t *prid = v_is_npoint ?
    (const int64_t *) vstruct->children[0]->buffers[1] : NULL;
  const double *ppos = v_is_npoint ?
    (const double *) vstruct->children[1]->buffers[1] : NULL;
  const double *pz = ((v_is_point && vstruct->n_children == 3) ||
    v_is_pose3d) ?
    (const double *) vstruct->children[2]->buffers[1] : NULL;
  const double *pr = v_is_cbuffer ?
    (const double *) vstruct->children[2]->buffers[1] : NULL;
  const double *pth = (v_is_pose && ! v_is_pose3d) ?
    (const double *) vstruct->children[2]->buffers[1] : NULL;
  const double *pqw = v_is_pose3d ?
    (const double *) vstruct->children[3]->buffers[1] : NULL;
  const double *pqx = v_is_pose3d ?
    (const double *) vstruct->children[4]->buffers[1] : NULL;
  const double *pqy = v_is_pose3d ?
    (const double *) vstruct->children[5]->buffers[1] : NULL;
  const double *pqz = v_is_pose3d ?
    (const double *) vstruct->children[6]->buffers[1] : NULL;
  /* Rigid geometry: child[0] is the "ref" LargeBinary (constant across the
   * value); the pose fields follow at children[1..], so the pose pointers
   * are shifted by one. The inner build is a temporal pose; the reference
   * geometry wraps it afterwards. */
  const int64_t *roff = v_is_trgeo ?
    (const int64_t *) vstruct->children[0]->buffers[1] : NULL;
  const char *rdata = v_is_trgeo ?
    (const char *) vstruct->children[0]->buffers[2] : NULL;
  if (v_is_trgeo)
  {
    px = (const double *) vstruct->children[1]->buffers[1];
    py = (const double *) vstruct->children[2]->buffers[1];
    if (v_is_trgeo3d)
    {
      pz = (const double *) vstruct->children[3]->buffers[1];
      pqw = (const double *) vstruct->children[4]->buffers[1];
      pqx = (const double *) vstruct->children[5]->buffers[1];
      pqy = (const double *) vstruct->children[6]->buffers[1];
      pqz = (const double *) vstruct->children[7]->buffers[1];
    }
    else
      pth = (const double *) vstruct->children[3]->buffers[1];
  }
  int nseqs = (int) seq_st_a->length;
  int total = nseqs > 0 ? insts_off[nseqs] - insts_off[0] : 0;
  if (nseqs <= 0 || total <= 0)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: empty value unsupported");
    return NULL;
  }

  /* The rigid-geometry reference is constant; rebuild it once from the
   * first instant's "ref" child and wrap the reconstructed temporal pose. */
  GSERIALIZED *trgeo_ref = NULL;
#if RGEO
  if (v_is_trgeo)
    trgeo_ref = aw_geo_from_wkb((const uint8_t *) (rdata + roff[0]),
      (size_t) (roff[1] - roff[0]), false);
#else
  (void) roff; (void) rdata; (void) trgeo_ref; (void) v_is_trgeo3d;
#endif

  if (subtype == TINSTANT)
  {
    TInstant *ti = aw_make_instant(v_sc->format, vt, vvals, soff, sdata,
      goff, gdata, px, py, pz, pr, pth, pqw, pqx, pqy, pqz, prid, ppos, srid,
      geodetic, 0, (TimestampTz) tvals[0]);
#if RGEO
    if (v_is_trgeo)
    {
      Temporal *r = (Temporal *) geo_tposeinst_to_trgeo(trgeo_ref, ti);
      pfree(ti);
      pfree(trgeo_ref);
      return r;
    }
#endif
    return (Temporal *) ti;
  }

  interpType ip = (interpType) interp;
  if (subtype == TSEQUENCE)
  {
    int n = insts_off[1] - insts_off[0];
    bool lo = (lower_bm[0] & 0x01) != 0;
    bool up = (upper_bm[0] & 0x01) != 0;
    TInstant **instants = palloc(sizeof(TInstant *) * n);
    for (int i = 0; i < n; i++)
      instants[i] = aw_make_instant(v_sc->format, vt, vvals, soff, sdata,
        goff, gdata, px, py, pz, pr, pth, pqw, pqx, pqy, pqz, prid, ppos, srid,
        geodetic, i, (TimestampTz) tvals[i]);
    TSequence *result = tsequence_make(instants, n, lo, up, ip, true);
    for (int i = 0; i < n; i++)
      pfree(instants[i]);
    pfree(instants);
#if RGEO
    if (v_is_trgeo)
    {
      Temporal *r = (Temporal *) geo_tposeseq_to_trgeo(trgeo_ref, result);
      pfree(result);
      pfree(trgeo_ref);
      return r;
    }
#endif
    return (Temporal *) result;
  }

  /* TSEQUENCESET: rebuild each component sequence, then assemble */
  TSequence **seqs = palloc(sizeof(TSequence *) * nseqs);
  for (int j = 0; j < nseqs; j++)
  {
    int lo_off = insts_off[j], cnt = insts_off[j + 1] - lo_off;
    bool lo = ((lower_bm[j >> 3] >> (j & 7)) & 1) != 0;
    bool up = ((upper_bm[j >> 3] >> (j & 7)) & 1) != 0;
    TInstant **instants = palloc(sizeof(TInstant *) * cnt);
    for (int i = 0; i < cnt; i++)
      instants[i] = aw_make_instant(v_sc->format, vt, vvals, soff, sdata,
        goff, gdata, px, py, pz, pr, pth, pqw, pqx, pqy, pqz, prid, ppos, srid,
        geodetic, lo_off + i,
        (TimestampTz) tvals[lo_off + i]);
    seqs[j] = tsequence_make(instants, cnt, lo, up, ip, true);
    for (int i = 0; i < cnt; i++)
      pfree(instants[i]);
    pfree(instants);
  }
  TSequenceSet *result = tsequenceset_make(seqs, nseqs, true);
  for (int j = 0; j < nseqs; j++)
    pfree(seqs[j]);
  pfree(seqs);
#if RGEO
  if (v_is_trgeo)
  {
    Temporal *r = (Temporal *) geo_tposeseqset_to_trgeo(trgeo_ref, result);
    pfree(result);
    pfree(trgeo_ref);
    return r;
  }
#endif
  return (Temporal *) result;
}

/**
 * @ingroup meos_temporal_conversion
 * @brief Round-trip a temporal value through the Arrow C Data Interface
 * @details Convert @p temp to the Arrow C Data Interface and reconstruct it,
 * returning the new value. The result is equal to @p temp; an unsupported
 * base type raises @p MEOS_ERR_FEATURE_NOT_SUPPORTED. Validates the lossless
 * correspondence and is the interop self-check reused by language bindings.
 * @param[in] temp Temporal value
 * @return New temporal value equal to @p temp, or @p NULL on an unsupported
 * base type
 */
Temporal *
meos_temporal_arrow_roundtrip(const Temporal *temp)
{
  struct ArrowSchema schema = {0};
  struct ArrowArray array = {0};
  if (! meos_temporal_to_arrow(temp, &schema, &array))
    return NULL;
  Temporal *result = meos_temporal_from_arrow(&schema, &array);
  if (schema.release)
    schema.release(&schema);
  if (array.release)
    array.release(&array);
  return result;
}

/*****************************************************************************
 * Conversion of the finite-subset closure types (set, span, spanset) into
 * the Arrow C Data Interface
 *
 * @details set, span and spanset are the MEOS types that represent finite
 * subsets of an (otherwise infinite) base domain; they close the MEOS type
 * algebra. The Arrow encoding mirrors the temporal VALUE_SCHEMA above
 * verbatim: every per-base-type value leaf reuses the exact same machinery
 * (the @p aw_* node builders, the @p aw_leaf_datum / @p aw_geo_to_wkb /
 * @p aw_geo_from_wkb seam, the @c "i"/"l"/"g"/"u"/"Z" leaf formats, the
 * @c "tsu:UTC" raw-value timestamp convention, the int16 raw-MEOS-flags
 * word, the int32 SRID slot non-zero only for a geometry value, and the
 * Boolean @c "lower_inc"/"upper_inc" convention). The skeletons are:
 * @code
 * set     Struct{ settype:int16, flags:int16, srid:int32,
 *                 elems: List< VALUE_LEAF(B) > }       -- sorted, as stored
 * span    Struct{ spantype:int16,
 *                 lower: VALUE_LEAF(B), upper: VALUE_LEAF(B),
 *                 lower_inc:bool, upper_inc:bool }
 * spanset Struct{ spansettype:int16,
 *                 spans: List< span-Struct(B) > }
 * @endcode
 * The base type drives the value leaf exactly as for the temporal types:
 * int4 -> Int32 "i", int8 -> Int64 "l", float8 -> Float64 "g", text ->
 * Utf8 "u", timestamptz -> "tsu:UTC" (the raw MEOS value carried verbatim,
 * as the temporal @c t leaf does), date -> "tdD" (the raw DateADT int32
 * carried verbatim under the Arrow date32 logical type, the same
 * raw-value-under-an-Arrow-logical-format convention as the timestamp
 * leaf, lossless by round-trip), geometry or geography -> the opaque
 * LargeBinary "Z" leaf of the value's extended WKB. Every struct children
 * array is arena-allocated so the schema outlives the call (the canonical
 * pattern). Any base type not in this surface raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED rather than producing a silent or
 * approximate result.
 *****************************************************************************/

/**
 * @brief Classify a set/span/spanset base type into its Arrow value-leaf
 * format string, mirroring the temporal VALUE_SCHEMA leaf mapping
 * @return The Arrow format string, or NULL when the base type has no wired
 * value leaf
 */
static const char *
aw_base_vfmt(MeosType basetype)
{
  switch (basetype)
  {
    case T_INT4:        return "i";
    case T_INT8:        return "l";
    case T_FLOAT8:      return "g";
    case T_TEXT:        return "u";
    case T_DATE:        return "tdD";
    case T_TIMESTAMPTZ: return "tsu:UTC";
    case T_GEOMETRY:
    case T_GEOGRAPHY:   return "Z";
    default:            return NULL;
  }
}

/**
 * @brief Append the @p idx-th element to the per-base-type value column
 * being assembled, dispatching on the value-leaf format. Fixed-width
 * leaves write straight into @p vvals; the variable-length text and
 * geometry leaves collect into the scratch arrays assembled afterwards.
 */
static void
aw_put_value(const char *vfmt, MeosType basetype, Datum value, int idx,
  void *vvals, char **svals, uint8_t **gbufs, size_t *glens)
{
  /* The value-leaf format alone drives the write; basetype is kept in the
   * signature for symmetry with aw_get_value (which needs it for the
   * geometry/geography distinction). */
  (void) basetype;
  if (vfmt[0] == 'i')
    ((int32_t *) vvals)[idx] = DatumGetInt32(value);
  else if (vfmt[0] == 'l')
    ((int64_t *) vvals)[idx] = (int64_t) DatumGetInt64(value);
  else if (vfmt[0] == 'g')
    ((double *) vvals)[idx] = DatumGetFloat8(value);
  else if (vfmt[0] == 't' && vfmt[1] == 'd')
    /* date32 leaf: the raw DateADT int32, carried verbatim (the PostgreSQL
     * epoch travels with it, recovered losslessly on read, mirroring the
     * timestamp leaf's raw-value convention) */
    ((int32_t *) vvals)[idx] = (int32_t) DatumGetDateADT(value);
  else if (vfmt[0] == 't' && vfmt[1] == 's')
    /* timestamp[us,UTC] leaf: the raw TimestampTz int64, carried verbatim
     * exactly as the temporal t leaf does */
    ((int64_t *) vvals)[idx] = (int64_t) DatumGetTimestampTz(value);
  else if (vfmt[0] == 'u')
    svals[idx] = text2cstring(DatumGetTextP(value));
  else /* 'Z' : a geometry or geography, carried as extended WKB */
    gbufs[idx] = aw_geo_to_wkb(value, &glens[idx]);
}

/**
 * @brief Reconstruct the @p idx-th base value as a Datum from the decoded
 * Arrow value column, dispatching on the value-leaf format. The caller
 * owns the returned Datum for the by-reference (text, geometry) cases and
 * frees it after the value is copied into the constructed set/span.
 */
static Datum
aw_get_value(const char *vfmt, MeosType basetype, const void *vvals,
  const int32_t *soff, const char *sdata, const int64_t *goff,
  const char *gdata, int idx)
{
  if (vfmt[0] == 'i')
    return Int32GetDatum(((const int32_t *) vvals)[idx]);
  if (vfmt[0] == 'l')
    return Int64GetDatum(((const int64_t *) vvals)[idx]);
  if (vfmt[0] == 'g')
    return Float8GetDatum(((const double *) vvals)[idx]);
  if (vfmt[0] == 't' && vfmt[1] == 'd')
    return DateADTGetDatum((DateADT) ((const int32_t *) vvals)[idx]);
  if (vfmt[0] == 't' && vfmt[1] == 's')
    return TimestampTzGetDatum((TimestampTz) ((const int64_t *) vvals)[idx]);
  if (vfmt[0] == 'u')
  {
    int32_t len = soff[idx + 1] - soff[idx];
    char *cs = palloc((size_t) len + 1);
    if (len)
      memcpy(cs, sdata + soff[idx], (size_t) len);
    cs[len] = '\0';
    text *txt = cstring2text(cs);
    pfree(cs);
    return PointerGetDatum(txt);
  }
  /* 'Z' : a geometry or geography reconstructed from its extended WKB */
  size_t len = (size_t) (goff[idx + 1] - goff[idx]);
  GSERIALIZED *gs = aw_geo_from_wkb((const uint8_t *) (gdata + goff[idx]),
    len, basetype == T_GEOGRAPHY);
  return PointerGetDatum(gs);
}

/**
 * @brief Build the Arrow value-leaf schema node for a base type, reusing
 * the temporal seam (a fixed-width or variable-length leaf, named @p name)
 */
static struct ArrowSchema *
aw_value_schema(ArrowArena *arena, const char *vfmt, const char *name)
{
  return aw_schema_leaf(arena, vfmt, name);
}

/**
 * @brief Assemble the Arrow value-leaf array node for a fully-walked
 * per-base-type column, reusing the temporal array builders
 */
static struct ArrowArray *
aw_value_array(ArrowArena *arena, const char *vfmt, int total,
  const void *vvals, const int32_t *str_off, const char *str_data,
  const int64_t *geo_off, const char *geo_data)
{
  if (vfmt[0] == 'u')
    return aw_array_utf8(arena, total, str_off, str_data);
  if (vfmt[0] == 'Z')
    return aw_array_largebin(arena, total, geo_off, geo_data);
  return aw_array_prim(arena, total, vvals);
}

/**
 * @brief Size in bytes of the fixed-width value buffer for @p n elements
 * of the given value-leaf format (0 for the variable-length leaves, which
 * use the scratch-then-assemble path)
 */
static size_t
aw_value_bufsize(const char *vfmt, int n)
{
  if (vfmt[0] == 'u' || vfmt[0] == 'Z')
    return 1;
  if (vfmt[0] == 'i' || (vfmt[0] == 't' && vfmt[1] == 'd'))
    return sizeof(int32_t) * (size_t) n;
  if (vfmt[0] == 'l' || (vfmt[0] == 't' && vfmt[1] == 's'))
    return sizeof(int64_t) * (size_t) n;
  return sizeof(double) * (size_t) n;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a set into Arrow C Data Interface structures
 * @param[in] s Set
 * @param[out] out_schema,out_array Caller-allocated Arrow structures filled
 * by the producer; release with their @p release callbacks
 * @return True on success; on an unsupported base type raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED and returns false
 */
bool
meos_set_to_arrow(const Set *s, struct ArrowSchema *out_schema,
  struct ArrowArray *out_array)
{
  if (! s || ! out_schema || ! out_array)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG, "Null argument to set_to_arrow");
    return false;
  }
  MeosType basetype = (MeosType) s->basetype;
  const char *vfmt = aw_base_vfmt(basetype);
  if (! vfmt)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_set_to_arrow: unsupported set base type");
    return false;
  }
  bool is_geo = (vfmt[0] == 'Z');
  ArrowArena *arena_s = palloc0(sizeof(ArrowArena));
  ArrowArena *arena_a = palloc0(sizeof(ArrowArena));

  /* Schema: Struct{settype, flags, srid, elems:List<value-leaf>} */
  struct ArrowSchema *elem_leaf = aw_value_schema(arena_s, vfmt, "item");
  struct ArrowSchema *elems_list =
    aw_schema_list(arena_s, "elems", elem_leaf);
  struct ArrowSchema **top_sc =
    arena_alloc(arena_s, sizeof(struct ArrowSchema *) * 4);
  top_sc[0] = aw_schema_leaf(arena_s, "s", "settype");
  top_sc[1] = aw_schema_leaf(arena_s, "s", "flags");
  top_sc[2] = aw_schema_leaf(arena_s, "i", "srid");
  top_sc[3] = elems_list;
  out_schema->format = "+s";
  out_schema->name = NULL;
  out_schema->metadata = NULL;
  out_schema->flags = 0;
  out_schema->n_children = 4;
  out_schema->children = top_sc;
  out_schema->dictionary = NULL;
  out_schema->private_data = arena_s;
  out_schema->release = root_schema_release;

  int16_t *settype_b = arena_alloc(arena_a, sizeof(int16_t));
  int16_t *flags_b = arena_alloc(arena_a, sizeof(int16_t));
  int32_t *srid_b = arena_alloc(arena_a, sizeof(int32_t));
  settype_b[0] = (int16_t) s->settype;
  flags_b[0] = (int16_t) s->flags;
  srid_b[0] = is_geo ? (int32_t) spatialset_srid(s) : 0;

  int total = s->count;
  int vn = total ? total : 1;
  void *vvals = arena_alloc(arena_a, aw_value_bufsize(vfmt, vn));
  char **svals = (vfmt[0] == 'u') ? palloc(sizeof(char *) * vn) : NULL;
  uint8_t **gbufs = is_geo ? palloc(sizeof(uint8_t *) * vn) : NULL;
  size_t *glens = is_geo ? palloc(sizeof(size_t) * vn) : NULL;
  for (int i = 0; i < total; i++)
    aw_put_value(vfmt, basetype, SET_VAL_N(s, i), i, vvals, svals, gbufs,
      glens);

  int32_t *str_off = NULL;
  char *str_data = NULL;
  if (vfmt[0] == 'u')
  {
    str_off = arena_alloc(arena_a, sizeof(int32_t) * (total + 1));
    int32_t acc = 0;
    for (int i = 0; i < total; i++)
    {
      str_off[i] = acc;
      acc += (int32_t) strlen(svals[i]);
    }
    str_off[total] = acc;
    str_data = arena_alloc(arena_a, acc ? (size_t) acc : 1);
    for (int i = 0; i < total; i++)
    {
      int32_t len = str_off[i + 1] - str_off[i];
      if (len)
        memcpy(str_data + str_off[i], svals[i], (size_t) len);
      pfree(svals[i]);
    }
    pfree(svals);
  }
  int64_t *geo_off = NULL;
  char *geo_data = NULL;
  if (is_geo)
  {
    geo_off = arena_alloc(arena_a, sizeof(int64_t) * (total + 1));
    int64_t acc = 0;
    for (int i = 0; i < total; i++)
    {
      geo_off[i] = acc;
      acc += (int64_t) glens[i];
    }
    geo_off[total] = acc;
    geo_data = arena_alloc(arena_a, acc ? (size_t) acc : 1);
    for (int i = 0; i < total; i++)
    {
      if (glens[i])
        memcpy(geo_data + geo_off[i], gbufs[i], glens[i]);
      pfree(gbufs[i]);
    }
    pfree(gbufs);
    pfree(glens);
  }

  struct ArrowArray *elem_a = aw_value_array(arena_a, vfmt, total, vvals,
    str_off, str_data, geo_off, geo_data);
  int32_t *elems_off = arena_alloc(arena_a, sizeof(int32_t) * 2);
  elems_off[0] = 0;
  elems_off[1] = total;
  struct ArrowArray *elems_list_a =
    aw_array_list(arena_a, 1, elems_off, elem_a);
  struct ArrowArray **top_a =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 4);
  top_a[0] = aw_array_prim(arena_a, 1, settype_b);
  top_a[1] = aw_array_prim(arena_a, 1, flags_b);
  top_a[2] = aw_array_prim(arena_a, 1, srid_b);
  top_a[3] = elems_list_a;
  out_array->length = 1;
  out_array->null_count = 0;
  out_array->offset = 0;
  out_array->n_buffers = 1;
  out_array->n_children = 4;
  out_array->buffers = arena_alloc(arena_a, sizeof(void *) * 1);
  out_array->buffers[0] = NULL;
  out_array->children = top_a;
  out_array->dictionary = NULL;
  out_array->private_data = arena_a;
  out_array->release = root_array_release;
  return true;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Reconstruct a set from Arrow C Data Interface structures
 * @param[in] schema,array Arrow structures produced by #meos_set_to_arrow
 * @return New set, or NULL on an unsupported layout (raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED)
 */
Set *
meos_set_from_arrow(const struct ArrowSchema *schema,
  const struct ArrowArray *array)
{
  if (! schema || ! array || ! schema->format ||
      strcmp(schema->format, "+s") != 0 || schema->n_children != 4 ||
      array->n_children != 4 || array->length != 1)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_set_from_arrow: unsupported Arrow layout");
    return NULL;
  }
  int16_t settype = ((const int16_t *) array->children[0]->buffers[1])[0];
  MeosType basetype = settype_basetype((MeosType) settype);
  const char *vfmt = aw_base_vfmt(basetype);
  const struct ArrowSchema *v_sc = schema->children[3]->children[0];
  if (! vfmt || ! v_sc->format || ! aw_base_vfmt(basetype) ||
      strcmp(v_sc->format, vfmt) != 0)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_set_from_arrow: unsupported set base type");
    return NULL;
  }
  const struct ArrowArray *elems_a = array->children[3];
  const int32_t *elems_off = (const int32_t *) elems_a->buffers[1];
  const struct ArrowArray *vleaf = elems_a->children[0];
  int total = elems_off[1] - elems_off[0];
  if (total <= 0)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_set_from_arrow: empty set unsupported");
    return NULL;
  }
  const void *vvals = vleaf->buffers[1];
  const int32_t *soff = (vfmt[0] == 'u') ?
    (const int32_t *) vleaf->buffers[1] : NULL;
  const char *sdata = (vfmt[0] == 'u') ?
    (const char *) vleaf->buffers[2] : NULL;
  const int64_t *goff = (vfmt[0] == 'Z') ?
    (const int64_t *) vleaf->buffers[1] : NULL;
  const char *gdata = (vfmt[0] == 'Z') ?
    (const char *) vleaf->buffers[2] : NULL;
  Datum *values = palloc(sizeof(Datum) * total);
  bool byref = (vfmt[0] == 'u' || vfmt[0] == 'Z');
  for (int i = 0; i < total; i++)
    values[i] = aw_get_value(vfmt, basetype, vvals, soff, sdata, goff,
      gdata, i);
  /* The Arrow elements are stored in the set's own sorted order; pass
   * order = false so the canonical order is preserved exactly. */
  Set *result = set_make(values, total, basetype, false);
  if (byref)
    for (int i = 0; i < total; i++)
      pfree(DatumGetPointer(values[i]));
  pfree(values);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Round-trip a set through the Arrow C Data Interface
 * @param[in] s Set
 * @return New set equal to @p s, or @p NULL on an unsupported base type
 */
Set *
meos_set_arrow_roundtrip(const Set *s)
{
  struct ArrowSchema schema = {0};
  struct ArrowArray array = {0};
  if (! meos_set_to_arrow(s, &schema, &array))
    return NULL;
  Set *result = meos_set_from_arrow(&schema, &array);
  if (schema.release)
    schema.release(&schema);
  if (array.release)
    array.release(&array);
  return result;
}

/*****************************************************************************/

/**
 * @brief Fill an arena-allocated value-leaf array node for a single
 * scalar/text/geometry base value (used for the span lower and upper
 * bounds, which are single elements rather than a column)
 */
static struct ArrowArray *
aw_one_value_array(ArrowArena *arena, const char *vfmt, MeosType basetype,
  Datum value)
{
  if (vfmt[0] == 'u')
  {
    char *cs = text2cstring(DatumGetTextP(value));
    int32_t n = (int32_t) strlen(cs);
    int32_t *off = arena_alloc(arena, sizeof(int32_t) * 2);
    off[0] = 0; off[1] = n;
    char *data = arena_alloc(arena, n ? (size_t) n : 1);
    if (n)
      memcpy(data, cs, (size_t) n);
    pfree(cs);
    return aw_array_utf8(arena, 1, off, data);
  }
  if (vfmt[0] == 'Z')
  {
    size_t len = 0;
    uint8_t *wkb = aw_geo_to_wkb(value, &len);
    int64_t *off = arena_alloc(arena, sizeof(int64_t) * 2);
    off[0] = 0; off[1] = (int64_t) len;
    char *data = arena_alloc(arena, len ? len : 1);
    if (len)
      memcpy(data, wkb, len);
    pfree(wkb);
    return aw_array_largebin(arena, 1, off, data);
  }
  void *buf = arena_alloc(arena, aw_value_bufsize(vfmt, 1));
  aw_put_value(vfmt, basetype, value, 0, buf, NULL, NULL, NULL);
  return aw_array_prim(arena, 1, buf);
}

/**
 * @brief Read back the single base value of a span bound from its decoded
 * Arrow leaf array (the inverse of #aw_one_value_array)
 */
static Datum
aw_one_value_read(const char *vfmt, MeosType basetype,
  const struct ArrowArray *leaf)
{
  const void *vvals = leaf->buffers[1];
  const int32_t *soff = (vfmt[0] == 'u') ?
    (const int32_t *) leaf->buffers[1] : NULL;
  const char *sdata = (vfmt[0] == 'u') ?
    (const char *) leaf->buffers[2] : NULL;
  const int64_t *goff = (vfmt[0] == 'Z') ?
    (const int64_t *) leaf->buffers[1] : NULL;
  const char *gdata = (vfmt[0] == 'Z') ?
    (const char *) leaf->buffers[2] : NULL;
  return aw_get_value(vfmt, basetype, vvals, soff, sdata, goff, gdata, 0);
}

/**
 * @brief Build the Arrow schema and array for one span as the children of
 * a caller-supplied struct level (shared by #meos_span_to_arrow and the
 * spanset element list)
 */
static void
aw_span_struct(ArrowArena *arena_s, ArrowArena *arena_a, const Span *sp,
  const char *vfmt, struct ArrowSchema ***sc_out, int64_t *nsc_out,
  struct ArrowArray ***a_out, int64_t *na_out)
{
  struct ArrowSchema **sc =
    arena_alloc(arena_s, sizeof(struct ArrowSchema *) * 5);
  sc[0] = aw_schema_leaf(arena_s, "s", "spantype");
  sc[1] = aw_value_schema(arena_s, vfmt, "lower");
  sc[2] = aw_value_schema(arena_s, vfmt, "upper");
  sc[3] = aw_schema_leaf(arena_s, "b", "lower_inc");
  sc[4] = aw_schema_leaf(arena_s, "b", "upper_inc");
  MeosType basetype = spantype_basetype((MeosType) sp->spantype);
  int16_t *spantype_b = arena_alloc(arena_a, sizeof(int16_t));
  spantype_b[0] = (int16_t) sp->spantype;
  uint8_t *lo_bm = arena_alloc(arena_a, 1);
  uint8_t *up_bm = arena_alloc(arena_a, 1);
  if (sp->lower_inc) lo_bm[0] |= 1;
  if (sp->upper_inc) up_bm[0] |= 1;
  struct ArrowArray **a =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 5);
  a[0] = aw_array_prim(arena_a, 1, spantype_b);
  a[1] = aw_one_value_array(arena_a, vfmt, basetype, sp->lower);
  a[2] = aw_one_value_array(arena_a, vfmt, basetype, sp->upper);
  a[3] = aw_array_prim(arena_a, 1, lo_bm);
  a[4] = aw_array_prim(arena_a, 1, up_bm);
  *sc_out = sc; *nsc_out = 5;
  *a_out = a; *na_out = 5;
}

/**
 * @brief Reconstruct a span from a decoded span-struct schema/array pair
 */
static Span *
aw_span_from_struct(const struct ArrowSchema *sc,
  const struct ArrowArray *a)
{
  if (sc->n_children != 5 || a->n_children != 5)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_span_from_arrow: unsupported span layout");
    return NULL;
  }
  int16_t spantype = ((const int16_t *) a->children[0]->buffers[1])[0];
  MeosType basetype = spantype_basetype((MeosType) spantype);
  const char *vfmt = aw_base_vfmt(basetype);
  if (! vfmt || ! sc->children[1]->format ||
      strcmp(sc->children[1]->format, vfmt) != 0)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_span_from_arrow: unsupported span base type");
    return NULL;
  }
  Datum lower = aw_one_value_read(vfmt, basetype, a->children[1]);
  Datum upper = aw_one_value_read(vfmt, basetype, a->children[2]);
  bool lo = (((const uint8_t *) a->children[3]->buffers[1])[0] & 1) != 0;
  bool up = (((const uint8_t *) a->children[4]->buffers[1])[0] & 1) != 0;
  Span *result = span_make(lower, upper, lo, up, basetype);
  if (vfmt[0] == 'u' || vfmt[0] == 'Z')
  {
    pfree(DatumGetPointer(lower));
    pfree(DatumGetPointer(upper));
  }
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a span into Arrow C Data Interface structures
 * @param[in] s Span
 * @param[out] out_schema,out_array Caller-allocated Arrow structures filled
 * by the producer; release with their @p release callbacks
 * @return True on success; on an unsupported base type raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED and returns false
 */
bool
meos_span_to_arrow(const Span *s, struct ArrowSchema *out_schema,
  struct ArrowArray *out_array)
{
  if (! s || ! out_schema || ! out_array)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG, "Null argument to span_to_arrow");
    return false;
  }
  MeosType basetype = spantype_basetype((MeosType) s->spantype);
  const char *vfmt = aw_base_vfmt(basetype);
  if (! vfmt)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_span_to_arrow: unsupported span base type");
    return false;
  }
  ArrowArena *arena_s = palloc0(sizeof(ArrowArena));
  ArrowArena *arena_a = palloc0(sizeof(ArrowArena));
  struct ArrowSchema **sc;
  struct ArrowArray **a;
  int64_t nsc, na;
  aw_span_struct(arena_s, arena_a, s, vfmt, &sc, &nsc, &a, &na);
  out_schema->format = "+s";
  out_schema->name = NULL;
  out_schema->metadata = NULL;
  out_schema->flags = 0;
  out_schema->n_children = nsc;
  out_schema->children = sc;
  out_schema->dictionary = NULL;
  out_schema->private_data = arena_s;
  out_schema->release = root_schema_release;
  out_array->length = 1;
  out_array->null_count = 0;
  out_array->offset = 0;
  out_array->n_buffers = 1;
  out_array->n_children = na;
  out_array->buffers = arena_alloc(arena_a, sizeof(void *) * 1);
  out_array->buffers[0] = NULL;
  out_array->children = a;
  out_array->dictionary = NULL;
  out_array->private_data = arena_a;
  out_array->release = root_array_release;
  return true;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Reconstruct a span from Arrow C Data Interface structures
 * @param[in] schema,array Arrow structures produced by #meos_span_to_arrow
 * @return New span, or NULL on an unsupported layout (raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED)
 */
Span *
meos_span_from_arrow(const struct ArrowSchema *schema,
  const struct ArrowArray *array)
{
  if (! schema || ! array || ! schema->format ||
      strcmp(schema->format, "+s") != 0 || schema->n_children != 5 ||
      array->n_children != 5 || array->length != 1)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_span_from_arrow: unsupported Arrow layout");
    return NULL;
  }
  return aw_span_from_struct(schema, array);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Round-trip a span through the Arrow C Data Interface
 * @param[in] s Span
 * @return New span equal to @p s, or @p NULL on an unsupported base type
 */
Span *
meos_span_arrow_roundtrip(const Span *s)
{
  struct ArrowSchema schema = {0};
  struct ArrowArray array = {0};
  if (! meos_span_to_arrow(s, &schema, &array))
    return NULL;
  Span *result = meos_span_from_arrow(&schema, &array);
  if (schema.release)
    schema.release(&schema);
  if (array.release)
    array.release(&array);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a span set into Arrow C Data Interface structures
 * @param[in] ss Span set
 * @param[out] out_schema,out_array Caller-allocated Arrow structures filled
 * by the producer; release with their @p release callbacks
 * @return True on success; on an unsupported base type raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED and returns false
 */
bool
meos_spanset_to_arrow(const SpanSet *ss, struct ArrowSchema *out_schema,
  struct ArrowArray *out_array)
{
  if (! ss || ! out_schema || ! out_array)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG,
      "Null argument to spanset_to_arrow");
    return false;
  }
  MeosType basetype = spantype_basetype((MeosType) ss->spantype);
  const char *vfmt = aw_base_vfmt(basetype);
  if (! vfmt)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_spanset_to_arrow: unsupported span set base type");
    return false;
  }
  int n = ss->count;
  ArrowArena *arena_s = palloc0(sizeof(ArrowArena));
  ArrowArena *arena_a = palloc0(sizeof(ArrowArena));

  /* Schema: Struct{spansettype, spans:List<span-struct>}. The span-struct
   * children schema is the same for every element, so it is built once
   * from the first span. */
  struct ArrowSchema **sp_sc;
  struct ArrowArray **dummy_a;
  int64_t nsp_sc, ndummy;
  /* Build the per-element schema once (array side rebuilt per element). */
  {
    struct ArrowSchema **s0sc;
    int64_t ns0;
    struct ArrowArray **a0;
    int64_t na0;
    aw_span_struct(arena_s, arena_a, SPANSET_SP_N(ss, 0), vfmt, &s0sc, &ns0,
      &a0, &na0);
    sp_sc = s0sc; nsp_sc = ns0; dummy_a = a0; ndummy = na0;
    (void) dummy_a; (void) ndummy;
  }
  struct ArrowSchema *sp_st =
    aw_schema_struct(arena_s, "item", sp_sc, nsp_sc);
  struct ArrowSchema *spans_list =
    aw_schema_list(arena_s, "spans", sp_st);
  struct ArrowSchema **top_sc =
    arena_alloc(arena_s, sizeof(struct ArrowSchema *) * 2);
  top_sc[0] = aw_schema_leaf(arena_s, "s", "spansettype");
  top_sc[1] = spans_list;
  out_schema->format = "+s";
  out_schema->name = NULL;
  out_schema->metadata = NULL;
  out_schema->flags = 0;
  out_schema->n_children = 2;
  out_schema->children = top_sc;
  out_schema->dictionary = NULL;
  out_schema->private_data = arena_s;
  out_schema->release = root_schema_release;

  int16_t *sstype_b = arena_alloc(arena_a, sizeof(int16_t));
  sstype_b[0] = (int16_t) ss->spansettype;

  /* The span-struct is a struct of 5 parallel one-element columns; across
   * the n spans each column has n entries. Build the n columns directly so
   * the list child is a single struct array of length n (the canonical
   * List<Struct> shape, matching the temporal seqs/insts encoding). */
  int16_t *col_spantype = arena_alloc(arena_a, sizeof(int16_t) * (n ? n : 1));
  uint8_t *col_lo = arena_alloc(arena_a, (n + 7) / 8 ? (n + 7) / 8 : 1);
  uint8_t *col_up = arena_alloc(arena_a, (n + 7) / 8 ? (n + 7) / 8 : 1);
  void *col_lower = NULL;
  void *col_upper = NULL;
  char **lo_sv = NULL, **up_sv = NULL;
  uint8_t **lo_gb = NULL, **up_gb = NULL;
  size_t *lo_gl = NULL, *up_gl = NULL;
  int vn = n ? n : 1;
  if (vfmt[0] == 'u')
  {
    lo_sv = palloc(sizeof(char *) * vn);
    up_sv = palloc(sizeof(char *) * vn);
  }
  else if (vfmt[0] == 'Z')
  {
    lo_gb = palloc(sizeof(uint8_t *) * vn);
    up_gb = palloc(sizeof(uint8_t *) * vn);
    lo_gl = palloc(sizeof(size_t) * vn);
    up_gl = palloc(sizeof(size_t) * vn);
  }
  else
  {
    col_lower = arena_alloc(arena_a, aw_value_bufsize(vfmt, vn));
    col_upper = arena_alloc(arena_a, aw_value_bufsize(vfmt, vn));
  }
  for (int i = 0; i < n; i++)
  {
    const Span *sp = SPANSET_SP_N(ss, i);
    col_spantype[i] = (int16_t) sp->spantype;
    if (sp->lower_inc) col_lo[i >> 3] |= (uint8_t) (1 << (i & 7));
    if (sp->upper_inc) col_up[i >> 3] |= (uint8_t) (1 << (i & 7));
    aw_put_value(vfmt, basetype, sp->lower, i, col_lower, lo_sv, lo_gb,
      lo_gl);
    aw_put_value(vfmt, basetype, sp->upper, i, col_upper, up_sv, up_gb,
      up_gl);
  }

  /* Assemble the variable-length lower/upper columns when needed. */
  int32_t *lo_soff = NULL, *up_soff = NULL;
  char *lo_sdata = NULL, *up_sdata = NULL;
  int64_t *lo_goff = NULL, *up_goff = NULL;
  char *lo_gdata = NULL, *up_gdata = NULL;
  if (vfmt[0] == 'u')
  {
    lo_soff = arena_alloc(arena_a, sizeof(int32_t) * (n + 1));
    up_soff = arena_alloc(arena_a, sizeof(int32_t) * (n + 1));
    int32_t la = 0, ua = 0;
    for (int i = 0; i < n; i++)
    {
      lo_soff[i] = la; la += (int32_t) strlen(lo_sv[i]);
      up_soff[i] = ua; ua += (int32_t) strlen(up_sv[i]);
    }
    lo_soff[n] = la; up_soff[n] = ua;
    lo_sdata = arena_alloc(arena_a, la ? (size_t) la : 1);
    up_sdata = arena_alloc(arena_a, ua ? (size_t) ua : 1);
    for (int i = 0; i < n; i++)
    {
      int32_t l = lo_soff[i + 1] - lo_soff[i];
      int32_t u = up_soff[i + 1] - up_soff[i];
      if (l) memcpy(lo_sdata + lo_soff[i], lo_sv[i], (size_t) l);
      if (u) memcpy(up_sdata + up_soff[i], up_sv[i], (size_t) u);
      pfree(lo_sv[i]); pfree(up_sv[i]);
    }
    pfree(lo_sv); pfree(up_sv);
  }
  else if (vfmt[0] == 'Z')
  {
    lo_goff = arena_alloc(arena_a, sizeof(int64_t) * (n + 1));
    up_goff = arena_alloc(arena_a, sizeof(int64_t) * (n + 1));
    int64_t la = 0, ua = 0;
    for (int i = 0; i < n; i++)
    {
      lo_goff[i] = la; la += (int64_t) lo_gl[i];
      up_goff[i] = ua; ua += (int64_t) up_gl[i];
    }
    lo_goff[n] = la; up_goff[n] = ua;
    lo_gdata = arena_alloc(arena_a, la ? (size_t) la : 1);
    up_gdata = arena_alloc(arena_a, ua ? (size_t) ua : 1);
    for (int i = 0; i < n; i++)
    {
      if (lo_gl[i]) memcpy(lo_gdata + lo_goff[i], lo_gb[i], lo_gl[i]);
      if (up_gl[i]) memcpy(up_gdata + up_goff[i], up_gb[i], up_gl[i]);
      pfree(lo_gb[i]); pfree(up_gb[i]);
    }
    pfree(lo_gb); pfree(up_gb); pfree(lo_gl); pfree(up_gl);
  }

  struct ArrowArray **sp_a_kids =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 5);
  sp_a_kids[0] = aw_array_prim(arena_a, n, col_spantype);
  sp_a_kids[1] = aw_value_array(arena_a, vfmt, n, col_lower, lo_soff,
    lo_sdata, lo_goff, lo_gdata);
  sp_a_kids[2] = aw_value_array(arena_a, vfmt, n, col_upper, up_soff,
    up_sdata, up_goff, up_gdata);
  sp_a_kids[3] = aw_array_prim(arena_a, n, col_lo);
  sp_a_kids[4] = aw_array_prim(arena_a, n, col_up);
  struct ArrowArray *sp_st_a = aw_array_struct(arena_a, n, sp_a_kids, 5);
  int32_t *spans_off = arena_alloc(arena_a, sizeof(int32_t) * 2);
  spans_off[0] = 0;
  spans_off[1] = n;
  struct ArrowArray *spans_list_a =
    aw_array_list(arena_a, 1, spans_off, sp_st_a);
  struct ArrowArray **top_a =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 2);
  top_a[0] = aw_array_prim(arena_a, 1, sstype_b);
  top_a[1] = spans_list_a;
  out_array->length = 1;
  out_array->null_count = 0;
  out_array->offset = 0;
  out_array->n_buffers = 1;
  out_array->n_children = 2;
  out_array->buffers = arena_alloc(arena_a, sizeof(void *) * 1);
  out_array->buffers[0] = NULL;
  out_array->children = top_a;
  out_array->dictionary = NULL;
  out_array->private_data = arena_a;
  out_array->release = root_array_release;
  return true;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Reconstruct a span set from Arrow C Data Interface structures
 * @param[in] schema,array Arrow structures produced by
 * #meos_spanset_to_arrow
 * @return New span set, or NULL on an unsupported layout (raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED)
 */
SpanSet *
meos_spanset_from_arrow(const struct ArrowSchema *schema,
  const struct ArrowArray *array)
{
  if (! schema || ! array || ! schema->format ||
      strcmp(schema->format, "+s") != 0 || schema->n_children != 2 ||
      array->n_children != 2 || array->length != 1)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_spanset_from_arrow: unsupported Arrow layout");
    return NULL;
  }
  int16_t sstype = ((const int16_t *) array->children[0]->buffers[1])[0];
  MeosType spantype = spansettype_spantype((MeosType) sstype);
  MeosType basetype = spantype_basetype(spantype);
  const char *vfmt = aw_base_vfmt(basetype);
  const struct ArrowSchema *sp_st_sc = schema->children[1]->children[0];
  if (! vfmt || sp_st_sc->n_children != 5 ||
      ! sp_st_sc->children[1]->format ||
      strcmp(sp_st_sc->children[1]->format, vfmt) != 0)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_spanset_from_arrow: unsupported span set base type");
    return NULL;
  }
  const struct ArrowArray *spans_a = array->children[1];
  const int32_t *spans_off = (const int32_t *) spans_a->buffers[1];
  const struct ArrowArray *sp_st_a = spans_a->children[0];
  int n = spans_off[1] - spans_off[0];
  if (n <= 0)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_spanset_from_arrow: empty span set unsupported");
    return NULL;
  }
  const void *lo_v = sp_st_a->children[1]->buffers[1];
  const void *up_v = sp_st_a->children[2]->buffers[1];
  const int32_t *lo_soff = (vfmt[0] == 'u') ?
    (const int32_t *) sp_st_a->children[1]->buffers[1] : NULL;
  const char *lo_sdata = (vfmt[0] == 'u') ?
    (const char *) sp_st_a->children[1]->buffers[2] : NULL;
  const int32_t *up_soff = (vfmt[0] == 'u') ?
    (const int32_t *) sp_st_a->children[2]->buffers[1] : NULL;
  const char *up_sdata = (vfmt[0] == 'u') ?
    (const char *) sp_st_a->children[2]->buffers[2] : NULL;
  const int64_t *lo_goff = (vfmt[0] == 'Z') ?
    (const int64_t *) sp_st_a->children[1]->buffers[1] : NULL;
  const char *lo_gdata = (vfmt[0] == 'Z') ?
    (const char *) sp_st_a->children[1]->buffers[2] : NULL;
  const int64_t *up_goff = (vfmt[0] == 'Z') ?
    (const int64_t *) sp_st_a->children[2]->buffers[1] : NULL;
  const char *up_gdata = (vfmt[0] == 'Z') ?
    (const char *) sp_st_a->children[2]->buffers[2] : NULL;
  const uint8_t *lo_bm =
    (const uint8_t *) sp_st_a->children[3]->buffers[1];
  const uint8_t *up_bm =
    (const uint8_t *) sp_st_a->children[4]->buffers[1];
  bool byref = (vfmt[0] == 'u' || vfmt[0] == 'Z');
  Span *spans = palloc(sizeof(Span) * n);
  for (int i = 0; i < n; i++)
  {
    Datum lower = aw_get_value(vfmt, basetype, lo_v, lo_soff, lo_sdata,
      lo_goff, lo_gdata, i);
    Datum upper = aw_get_value(vfmt, basetype, up_v, up_soff, up_sdata,
      up_goff, up_gdata, i);
    bool lo = ((lo_bm[i >> 3] >> (i & 7)) & 1) != 0;
    bool up = ((up_bm[i >> 3] >> (i & 7)) & 1) != 0;
    Span *one = span_make(lower, upper, lo, up, basetype);
    spans[i] = *one;
    pfree(one);
    if (byref)
    {
      pfree(DatumGetPointer(lower));
      pfree(DatumGetPointer(upper));
    }
  }
  /* The Arrow spans are in the span set's own normalized order; pass
   * normalize = false and order = false to preserve it exactly. */
  SpanSet *result = spanset_make_exp(spans, n, n, false, false);
  pfree(spans);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Round-trip a span set through the Arrow C Data Interface
 * @param[in] ss Span set
 * @return New span set equal to @p ss, or @p NULL on an unsupported base
 * type
 */
SpanSet *
meos_spanset_arrow_roundtrip(const SpanSet *ss)
{
  struct ArrowSchema schema = {0};
  struct ArrowArray array = {0};
  if (! meos_spanset_to_arrow(ss, &schema, &array))
    return NULL;
  SpanSet *result = meos_spanset_from_arrow(&schema, &array);
  if (schema.release)
    schema.release(&schema);
  if (array.release)
    array.release(&array);
  return result;
}

/*****************************************************************************
 * Conversion of the product-domain finite-subset closure types (tbox, stbox)
 * into the Arrow C Data Interface
 *
 * @details A box is a finite rectangular subset of a product (multi
 * dimensional) domain: a temporal box of the time x value domain, a
 * spatiotemporal box of the space x time domain. A box is literally a
 * product of spans, so the Arrow encoding composes the SAME span Struct used
 * by #meos_span_to_arrow and #meos_spanset_to_arrow (the @p aw_span_struct /
 * @p aw_span_from_struct pair): there is exactly one span schema, shared
 * between a span as a value and a span as a box component. The skeletons
 * mirror the canonical @c TBox / @c STBox C structs verbatim:
 * @code
 * tbox  Struct{ flags:int16,
 *               period: span-Struct(tstz)   -- present iff the T flag,
 *               span:   span-Struct(value)  -- present iff the X flag }
 * stbox Struct{ flags:int16, srid:int32,
 *               period: span-Struct(tstz)              -- iff the T flag,
 *               x: Struct{ xmin:float64, xmax:float64 } -- iff the X flag,
 *               y: Struct{ ymin:float64, ymax:float64 } -- iff the X flag,
 *               z: Struct{ zmin:float64, zmax:float64 } -- iff the Z flag }
 * @endcode
 * The int16 MEOS flags word is carried verbatim, exactly as the temporal and
 * the set encodings do; it is the authoritative discriminator on read (the
 * geometry-vs-geography / geodetic distinction of an stbox is recovered from
 * it). Dimension presence is flag driven: an absent flag-gated dimension is
 * an OMITTED child (the @p n_children varies), the same canonical optionality
 * pattern the temporal point leaf uses to encode 2D versus 3D by child count
 * rather than an Arrow-level null. The spatial extent of an stbox is the raw
 * minimum and maximum doubles of the canonical struct with no per-axis
 * inclusivity, so it is mirrored as a plain @c Struct{min,max} and is NOT
 * forced through the span Struct (an stbox carries no spatial inclusivity).
 * Every children array is arena allocated so the schema outlives the call.
 *****************************************************************************/

/**
 * @brief Build a plain @c Struct{min:float64, max:float64} schema/array pair
 * for one spatial axis of an stbox (the raw extremal doubles of the
 * canonical @c STBox struct, with no inclusivity)
 */
static void
aw_axis_struct(ArrowArena *arena_s, ArrowArena *arena_a, const char *name,
  double mn, double mx, struct ArrowSchema **sc_out,
  struct ArrowArray **a_out)
{
  struct ArrowSchema **kids =
    arena_alloc(arena_s, sizeof(struct ArrowSchema *) * 2);
  kids[0] = aw_schema_leaf(arena_s, "g", "min");
  kids[1] = aw_schema_leaf(arena_s, "g", "max");
  *sc_out = aw_schema_struct(arena_s, name, kids, 2);
  double *mn_b = arena_alloc(arena_a, sizeof(double));
  double *mx_b = arena_alloc(arena_a, sizeof(double));
  mn_b[0] = mn;
  mx_b[0] = mx;
  struct ArrowArray **akids =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 2);
  akids[0] = aw_array_prim(arena_a, 1, mn_b);
  akids[1] = aw_array_prim(arena_a, 1, mx_b);
  *a_out = aw_array_struct(arena_a, 1, akids, 2);
}

/**
 * @brief Read back the @c min and @c max doubles of one stbox spatial axis
 * (the inverse of #aw_axis_struct)
 */
static void
aw_axis_read(const struct ArrowArray *a, double *mn, double *mx)
{
  *mn = ((const double *) a->children[0]->buffers[1])[0];
  *mx = ((const double *) a->children[1]->buffers[1])[0];
}

/**
 * @brief Assemble one span as a named child Struct, reusing the canonical
 * #aw_span_struct shared builder verbatim (the single span schema used by
 * #meos_span_to_arrow and #meos_spanset_to_arrow)
 */
static void
aw_span_child(ArrowArena *arena_s, ArrowArena *arena_a, const char *name,
  const Span *sp, struct ArrowSchema **sc_out, struct ArrowArray **a_out)
{
  MeosType basetype = spantype_basetype((MeosType) sp->spantype);
  const char *vfmt = aw_base_vfmt(basetype);
  struct ArrowSchema **sp_sc;
  struct ArrowArray **sp_a;
  int64_t nsp, nspa;
  aw_span_struct(arena_s, arena_a, sp, vfmt, &sp_sc, &nsp, &sp_a, &nspa);
  *sc_out = aw_schema_struct(arena_s, name, sp_sc, nsp);
  *a_out = aw_array_struct(arena_a, 1, sp_a, nspa);
}

/**
 * @ingroup meos_box_conversion
 * @brief Convert a temporal box into Arrow C Data Interface structures
 * @param[in] box Temporal box
 * @param[out] out_schema,out_array Caller-allocated Arrow structures filled
 * by the producer; release with their @p release callbacks
 * @return True on success; on a null argument raises
 * @p MEOS_ERR_INVALID_ARG and returns false
 */
bool
meos_tbox_to_arrow(const TBox *box, struct ArrowSchema *out_schema,
  struct ArrowArray *out_array)
{
  if (! box || ! out_schema || ! out_array)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG, "Null argument to tbox_to_arrow");
    return false;
  }
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  ArrowArena *arena_s = palloc0(sizeof(ArrowArena));
  ArrowArena *arena_a = palloc0(sizeof(ArrowArena));

  /* The flags word is carried verbatim and is the authoritative
   * discriminator on read, exactly as the temporal and set encodings do. */
  struct ArrowSchema **top_sc =
    arena_alloc(arena_s, sizeof(struct ArrowSchema *) * 3);
  struct ArrowArray **top_a =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 3);
  int64_t nc = 0;
  top_sc[nc] = aw_schema_leaf(arena_s, "s", "flags");
  int16_t *flags_b = arena_alloc(arena_a, sizeof(int16_t));
  flags_b[0] = (int16_t) box->flags;
  top_a[nc] = aw_array_prim(arena_a, 1, flags_b);
  nc++;
  /* period (a tstz Span) present iff the T flag, span (a number Span)
   * present iff the X flag: the canonical flag-driven optionality. Each
   * reuses the shared span Struct builder verbatim. */
  if (hast)
  {
    aw_span_child(arena_s, arena_a, "period", &box->period,
      &top_sc[nc], &top_a[nc]);
    nc++;
  }
  if (hasx)
  {
    aw_span_child(arena_s, arena_a, "span", &box->span,
      &top_sc[nc], &top_a[nc]);
    nc++;
  }
  out_schema->format = "+s";
  out_schema->name = NULL;
  out_schema->metadata = NULL;
  out_schema->flags = 0;
  out_schema->n_children = nc;
  out_schema->children = top_sc;
  out_schema->dictionary = NULL;
  out_schema->private_data = arena_s;
  out_schema->release = root_schema_release;
  out_array->length = 1;
  out_array->null_count = 0;
  out_array->offset = 0;
  out_array->n_buffers = 1;
  out_array->n_children = nc;
  out_array->buffers = arena_alloc(arena_a, sizeof(void *) * 1);
  out_array->buffers[0] = NULL;
  out_array->children = top_a;
  out_array->dictionary = NULL;
  out_array->private_data = arena_a;
  out_array->release = root_array_release;
  return true;
}

/**
 * @ingroup meos_box_conversion
 * @brief Reconstruct a temporal box from Arrow C Data Interface structures
 * @param[in] schema,array Arrow structures produced by #meos_tbox_to_arrow
 * @return New temporal box, or NULL on an unsupported layout (raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED)
 */
TBox *
meos_tbox_from_arrow(const struct ArrowSchema *schema,
  const struct ArrowArray *array)
{
  if (! schema || ! array || ! schema->format ||
      strcmp(schema->format, "+s") != 0 || schema->n_children < 1 ||
      array->n_children != schema->n_children || array->length != 1 ||
      ! schema->children[0]->name ||
      strcmp(schema->children[0]->name, "flags") != 0)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_tbox_from_arrow: unsupported Arrow layout");
    return NULL;
  }
  int16_t flags = ((const int16_t *) array->children[0]->buffers[1])[0];
  bool hast = MEOS_FLAGS_GET_T(flags);
  bool hasx = MEOS_FLAGS_GET_X(flags);
  Span period, span;
  int idx = 1;
  if (hast)
  {
    Span *p = aw_span_from_struct(schema->children[idx], array->children[idx]);
    if (! p)
      return NULL;
    period = *p;
    pfree(p);
    idx++;
  }
  if (hasx)
  {
    Span *s = aw_span_from_struct(schema->children[idx], array->children[idx]);
    if (! s)
      return NULL;
    span = *s;
    pfree(s);
    idx++;
  }
  /* tbox_make is the canonical constructor; a null component is the
   * canonical way to express an absent dimension. */
  return tbox_make(hasx ? &span : NULL, hast ? &period : NULL);
}

/**
 * @ingroup meos_box_conversion
 * @brief Round-trip a temporal box through the Arrow C Data Interface
 * @param[in] box Temporal box
 * @return New temporal box equal to @p box, or @p NULL on failure
 */
TBox *
meos_tbox_arrow_roundtrip(const TBox *box)
{
  struct ArrowSchema schema = {0};
  struct ArrowArray array = {0};
  if (! meos_tbox_to_arrow(box, &schema, &array))
    return NULL;
  TBox *result = meos_tbox_from_arrow(&schema, &array);
  if (schema.release)
    schema.release(&schema);
  if (array.release)
    array.release(&array);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_box_conversion
 * @brief Convert a spatiotemporal box into Arrow C Data Interface structures
 * @param[in] box Spatiotemporal box
 * @param[out] out_schema,out_array Caller-allocated Arrow structures filled
 * by the producer; release with their @p release callbacks
 * @return True on success; on a null argument raises
 * @p MEOS_ERR_INVALID_ARG and returns false
 */
bool
meos_stbox_to_arrow(const STBox *box, struct ArrowSchema *out_schema,
  struct ArrowArray *out_array)
{
  if (! box || ! out_schema || ! out_array)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG, "Null argument to stbox_to_arrow");
    return false;
  }
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  ArrowArena *arena_s = palloc0(sizeof(ArrowArena));
  ArrowArena *arena_a = palloc0(sizeof(ArrowArena));

  /* flags and srid are always present, mirroring the canonical STBox struct
   * (the int16 flags word carried verbatim, the int32 srid slot exactly as
   * the temporal encoding's srid slot). The geodetic / geometry distinction
   * is recovered from the verbatim flags word on read. */
  struct ArrowSchema **top_sc =
    arena_alloc(arena_s, sizeof(struct ArrowSchema *) * 6);
  struct ArrowArray **top_a =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 6);
  int64_t nc = 0;
  top_sc[nc] = aw_schema_leaf(arena_s, "s", "flags");
  int16_t *flags_b = arena_alloc(arena_a, sizeof(int16_t));
  flags_b[0] = (int16_t) box->flags;
  top_a[nc] = aw_array_prim(arena_a, 1, flags_b);
  nc++;
  top_sc[nc] = aw_schema_leaf(arena_s, "i", "srid");
  int32_t *srid_b = arena_alloc(arena_a, sizeof(int32_t));
  srid_b[0] = (int32_t) box->srid;
  top_a[nc] = aw_array_prim(arena_a, 1, srid_b);
  nc++;
  /* period present iff the T flag; the x and y axes iff the X flag; the z
   * axis iff the Z flag (the canonical STBox flag-driven optionality). The
   * period reuses the shared span Struct builder verbatim; the spatial axes
   * are the raw extremal doubles with no inclusivity. */
  if (hast)
  {
    aw_span_child(arena_s, arena_a, "period", &box->period,
      &top_sc[nc], &top_a[nc]);
    nc++;
  }
  if (hasx)
  {
    aw_axis_struct(arena_s, arena_a, "x", box->xmin, box->xmax,
      &top_sc[nc], &top_a[nc]);
    nc++;
    aw_axis_struct(arena_s, arena_a, "y", box->ymin, box->ymax,
      &top_sc[nc], &top_a[nc]);
    nc++;
  }
  if (hasz)
  {
    aw_axis_struct(arena_s, arena_a, "z", box->zmin, box->zmax,
      &top_sc[nc], &top_a[nc]);
    nc++;
  }
  out_schema->format = "+s";
  out_schema->name = NULL;
  out_schema->metadata = NULL;
  out_schema->flags = 0;
  out_schema->n_children = nc;
  out_schema->children = top_sc;
  out_schema->dictionary = NULL;
  out_schema->private_data = arena_s;
  out_schema->release = root_schema_release;
  out_array->length = 1;
  out_array->null_count = 0;
  out_array->offset = 0;
  out_array->n_buffers = 1;
  out_array->n_children = nc;
  out_array->buffers = arena_alloc(arena_a, sizeof(void *) * 1);
  out_array->buffers[0] = NULL;
  out_array->children = top_a;
  out_array->dictionary = NULL;
  out_array->private_data = arena_a;
  out_array->release = root_array_release;
  return true;
}

/**
 * @ingroup meos_box_conversion
 * @brief Reconstruct a spatiotemporal box from Arrow C Data Interface
 * structures
 * @param[in] schema,array Arrow structures produced by #meos_stbox_to_arrow
 * @return New spatiotemporal box, or NULL on an unsupported layout (raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED)
 */
STBox *
meos_stbox_from_arrow(const struct ArrowSchema *schema,
  const struct ArrowArray *array)
{
  if (! schema || ! array || ! schema->format ||
      strcmp(schema->format, "+s") != 0 || schema->n_children < 2 ||
      array->n_children != schema->n_children || array->length != 1 ||
      ! schema->children[0]->name ||
      strcmp(schema->children[0]->name, "flags") != 0 ||
      ! schema->children[1]->name ||
      strcmp(schema->children[1]->name, "srid") != 0)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_stbox_from_arrow: unsupported Arrow layout");
    return NULL;
  }
  int16_t flags = ((const int16_t *) array->children[0]->buffers[1])[0];
  int32_t srid = ((const int32_t *) array->children[1]->buffers[1])[0];
  bool hast = MEOS_FLAGS_GET_T(flags);
  bool hasx = MEOS_FLAGS_GET_X(flags);
  bool hasz = MEOS_FLAGS_GET_Z(flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(flags);
  Span period;
  bool have_period = false;
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  int idx = 2;
  if (hast)
  {
    Span *p = aw_span_from_struct(schema->children[idx], array->children[idx]);
    if (! p)
      return NULL;
    period = *p;
    pfree(p);
    have_period = true;
    idx++;
  }
  if (hasx)
  {
    aw_axis_read(array->children[idx], &xmin, &xmax);
    idx++;
    aw_axis_read(array->children[idx], &ymin, &ymax);
    idx++;
  }
  if (hasz)
  {
    aw_axis_read(array->children[idx], &zmin, &zmax);
    idx++;
  }
  /* stbox_make is the canonical constructor: the verbatim flags word drives
   * hasx / hasz / geodetic and the raw spatial extent and srid are passed
   * through unchanged. */
  return stbox_make(hasx, hasz, geodetic, srid, xmin, xmax, ymin, ymax,
    zmin, zmax, have_period ? &period : NULL);
}

/**
 * @ingroup meos_box_conversion
 * @brief Round-trip a spatiotemporal box through the Arrow C Data Interface
 * @param[in] box Spatiotemporal box
 * @return New spatiotemporal box equal to @p box, or @p NULL on failure
 */
STBox *
meos_stbox_arrow_roundtrip(const STBox *box)
{
  struct ArrowSchema schema = {0};
  struct ArrowArray array = {0};
  if (! meos_stbox_to_arrow(box, &schema, &array))
    return NULL;
  STBox *result = meos_stbox_from_arrow(&schema, &array);
  if (schema.release)
    schema.release(&schema);
  if (array.release)
    array.release(&array);
  return result;
}

/*****************************************************************************/
