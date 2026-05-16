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
 * @note All temporal float, integer, boolean, text, point (geometry or
 * geography), circular buffer, pose, network point, general geometry or
 * geography and rigid geometry subtypes and interpolations are wired; other
 * base types are not yet. The Arrow schema is the full contract.
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
  if (temp->temptype != T_TFLOAT && temp->temptype != T_TINT &&
      temp->temptype != T_TBOOL && temp->temptype != T_TTEXT &&
      temp->temptype != T_TGEOMPOINT && temp->temptype != T_TGEOGPOINT &&
      ! is_cbuffer && ! is_pose && ! is_npoint && ! is_geo && ! is_trgeo)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_to_arrow: only temporal float, integer, boolean, text, "
      "point, circular buffer, pose, network point, general geometry or "
      "geography and rigid geometry are wired");
    return false;
  }
  bool is_tint = (temp->temptype == T_TINT);
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
   * of its EWKB; the scalar tier uses a single primitive leaf. */
  const char *vfmt = (is_point || is_cbuffer || is_pose || is_npoint ||
    is_trgeo) ? "+s" : (is_geo ? "Z" : (is_ttext ? "u" :
    (is_tbool ? "b" : (is_tint ? "i" : "g"))));

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
    inst_kids[1] = aw_schema_leaf(arena_s, vfmt, "v");
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
    is_trgeo) ? 1 : (is_tbool ? (size_t) ((vn + 7) / 8) :
    (is_tint ? sizeof(int32_t) : sizeof(double)) * (size_t) vn);
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
  /* General geometry or geography → per-instant EWKB; rigid geometry → the
   * shared reference geometry's EWKB replicated per instant (the leaf is
   * per-instant uniform). Collected during the walk, then assembled into an
   * int64-offset LargeBinary column. */
  uint8_t **gbufs = (is_geo || is_trgeo) ?
    palloc(sizeof(uint8_t *) * vn) : NULL;
  size_t *glens = (is_geo || is_trgeo) ?
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
   * concatenated per-instant EWKB bytes. For a rigid geometry this is the
   * reference-geometry "ref" child of the value struct. */
  int64_t *geo_off = NULL;
  char *geo_data = NULL;
  if (is_geo || is_trgeo)
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
    inst_a_kids[1] = is_geo ?
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
   * Int32 "i" is temporal integer, Boolean "b" is temporal boolean, Utf8
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
  bool v_is_geo = v_sc->format && strcmp(v_sc->format, "Z") == 0;
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
       strcmp(v_sc->format, "i") != 0 && strcmp(v_sc->format, "b") != 0 &&
       strcmp(v_sc->format, "u") != 0 && strcmp(v_sc->format, "Z") != 0) ||
      (! v_is_struct && ! v_is_geo && srid != 0) ||
      (subtype != TINSTANT && subtype != TSEQUENCE &&
       subtype != TSEQUENCESET))
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: only temporal float, integer, boolean, text, "
      "point, circular buffer, pose, network point, general geometry or "
      "geography and rigid geometry are wired");
    return NULL;
  }
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
  /* A rigid geometry reconstructs as a temporal pose, then wraps it with
   * the shared reference geometry; the inner build uses the pose path. */
  MeosType vt = v_is_trgeo ? T_TPOSE : (v_is_geo ?
    (MEOS_FLAGS_GET_GEODETIC(flags) ? T_TGEOGRAPHY : T_TGEOMETRY) :
    ((v_sc->format[0] == 'u') ? T_TTEXT :
    ((v_sc->format[0] == 'b') ? T_TBOOL :
    ((v_sc->format[0] == 'i') ? T_TINT : T_TFLOAT))));
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
  const int64_t *goff = v_is_geo ?
    (const int64_t *) inst_st_a->children[1]->buffers[1] : NULL;
  const char *gdata = v_is_geo ?
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

/*****************************************************************************/
