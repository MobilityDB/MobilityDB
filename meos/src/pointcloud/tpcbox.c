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
 * @brief TPCBox — bounding-box type for pgpointcloud temporal values.
 *
 * Mirrors STBox but carries an additional @c pcid (pgpointcloud schema id)
 * because values from different schemas have incompatible dimensions and
 * cannot share a bbox. Phase 8F lands the type plus the base set of
 * operators used by downstream temporal types (Phase 8G tpcpoint,
 * Phase 8H tpcpatch).
 *
 * Scope notes:
 *   * @c pcpatch → @c TPCBox is implemented here and is a free conversion
 *     (pgpointcloud's @c SERIALIZED_PATCH already carries a 2D @c PCBOUNDS
 *     header — no schema lookup required; no Z).
 *   * @c pcpoint → @c TPCBox is deliberately out of scope: extracting X/Y/Z
 *     from a pcpoint byte blob requires the schema XML (loaded from
 *     @c pointcloud_formats by pcid). That plumbing lands with Phase 8G.
 *   * Position operators (left/right/above/below/before/after) and the
 *     GiST / SP-GiST operator classes are also deferred — Phase 8H wires
 *     them up together with the indexed temporal types.
 */

#include "pointcloud/tpcbox.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include "utils/timestamp.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/postgres_types.h"
#include "temporal/span.h"
#include "temporal/type_util.h"
#include "pointcloud/pcpatch.h"

/* Buffer size for input/output of TPCBox text form */
#define TPCBOX_MAXLEN  512

/*****************************************************************************
 * Validity
 *****************************************************************************/

/**
 * @brief Ensure two TPCBoxes share the same schema (pcid).
 * @note pcid 0 is treated as "unknown" — sets/unions involving it propagate
 * the non-zero pcid from the other operand. Rejecting 0-vs-non-0 would
 * block natural workflows like @c tpcbox() aggregation from raw literals.
 */
bool
ensure_same_pcid_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  if (box1->pcid != 0 && box2->pcid != 0 && box1->pcid != box2->pcid)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on TPCBox values with different schemas: %u vs %u",
      box1->pcid, box2->pcid);
    return false;
  }
  return true;
}

/**
 * @brief Ensure two TPCBoxes share the same SRID.
 * @note SRID 0 is treated as "unknown" — same relaxation as pcid.
 */
static bool
ensure_same_srid_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  if (box1->srid != 0 && box2->srid != 0 && box1->srid != box2->srid)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on TPCBox values with different SRIDs: %d vs %d",
      box1->srid, box2->srid);
    return false;
  }
  return true;
}

/*****************************************************************************
 * Input / output
 *
 * Textual format — minimal and deterministic. Structured to roundtrip
 * through lexical scan; the PG recv/send path uses the same byte layout
 * as the in-memory struct (fixed size = @c sizeof(TPCBox)).
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_box_inout
 * @brief Return the text representation of a TPCBox.
 * @details Examples (commas optional inside the parenthesised groups):
 * @code
 * TPCBOX X((10, 20), (30, 40)) PCID=1
 * TPCBOX Z((10, 20, 30), (40, 50, 60)) PCID=1 SRID=4326
 * TPCBOX XT(((10, 20), (30, 40)), [2024-01-01, 2024-01-02]) PCID=1
 * TPCBOX T([2024-01-01, 2024-01-02]) PCID=1
 * GEODTPCBOX ZT(((10, 20, 30), (40, 50, 60)), [2024-01-01, 2024-01-02]) PCID=1
 * @endcode
 */
char *
tpcbox_out(const TPCBox *box, int maxdd)
{
  VALIDATE_TPCBOX(box, NULL);
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(box->flags);

  char *buf = palloc(TPCBOX_MAXLEN);
  char *p = buf;
  p += snprintf(p, TPCBOX_MAXLEN - (p - buf), "%sTPCBOX ",
    geodetic ? "GEOD" : "");
  if (hasx && hast)
    p += snprintf(p, TPCBOX_MAXLEN - (p - buf), "%sT", hasz ? "Z" : "X");
  else if (hasx)
    p += snprintf(p, TPCBOX_MAXLEN - (p - buf), "%s", hasz ? "Z" : "X");
  else
    p += snprintf(p, TPCBOX_MAXLEN - (p - buf), "T");

  if (hasx)
  {
    if (hast) *p++ = '(';
    *p++ = '(';
    if (hasz)
      p += snprintf(p, TPCBOX_MAXLEN - (p - buf), "%.*g, %.*g, %.*g",
        maxdd, box->xmin, maxdd, box->ymin, maxdd, box->zmin);
    else
      p += snprintf(p, TPCBOX_MAXLEN - (p - buf), "%.*g, %.*g",
        maxdd, box->xmin, maxdd, box->ymin);
    p += snprintf(p, TPCBOX_MAXLEN - (p - buf), "), (");
    if (hasz)
      p += snprintf(p, TPCBOX_MAXLEN - (p - buf), "%.*g, %.*g, %.*g",
        maxdd, box->xmax, maxdd, box->ymax, maxdd, box->zmax);
    else
      p += snprintf(p, TPCBOX_MAXLEN - (p - buf), "%.*g, %.*g",
        maxdd, box->xmax, maxdd, box->ymax);
    *p++ = ')';
    if (hast) *p++ = ',';
  }
  if (hast)
  {
    char *period_str = span_out(&box->period, maxdd);
    p += snprintf(p, TPCBOX_MAXLEN - (p - buf), "%s", period_str);
    pfree(period_str);
    if (hasx) *p++ = ')';
  }
  p += snprintf(p, TPCBOX_MAXLEN - (p - buf), " PCID=%u", box->pcid);
  if (box->srid != 0)
    p += snprintf(p, TPCBOX_MAXLEN - (p - buf), " SRID=%d", box->srid);
  *p = '\0';
  return buf;
}

/**
 * @ingroup meos_pointcloud_box_inout
 * @brief Return a TPCBox from its text representation.
 * @note Full WKT-style parsing is deferred — this accepts only the hex
 *   form produced by send(): the raw byte-image of a TPCBox struct
 *   encoded as ASCII hex. Adequate for round-tripping through PG
 *   @c recv / @c send, which is what SQL @c CREATE TYPE exercises.
 *   Rich WKT input like @c tpcbox_in() lands in Phase 8F.1 once the
 *   indexing paths need it.
 */
TPCBox *
tpcbox_in(const char *str)
{
  if (! str)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Null input string");
    return NULL;
  }
  size_t len = strlen(str);
  /* Skip leading whitespace */
  const char *p = str;
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
  len = strlen(p);
  if (len != 2 * sizeof(TPCBox))
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "TPCBox hex input must be %zu chars (got %zu); rich text form "
      "arrives in a follow-up phase",
      2 * sizeof(TPCBox), len);
    return NULL;
  }
  TPCBox *result = palloc0(sizeof(TPCBox));
  for (size_t i = 0; i < sizeof(TPCBox); i++)
  {
    char hi = p[i * 2], lo = p[i * 2 + 1];
    int hv = (hi >= '0' && hi <= '9') ? hi - '0' :
             (hi >= 'a' && hi <= 'f') ? hi - 'a' + 10 :
             (hi >= 'A' && hi <= 'F') ? hi - 'A' + 10 : -1;
    int lv = (lo >= '0' && lo <= '9') ? lo - '0' :
             (lo >= 'a' && lo <= 'f') ? lo - 'a' + 10 :
             (lo >= 'A' && lo <= 'F') ? lo - 'A' + 10 : -1;
    if (hv < 0 || lv < 0)
    {
      pfree(result);
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "TPCBox input contains non-hex character");
      return NULL;
    }
    ((uint8_t *) result)[i] = (uint8_t) ((hv << 4) | lv);
  }
  return result;
}

/*****************************************************************************
 * Constructors
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_box_constructor
 * @brief Return a fresh TPCBox from component values.
 * @param[in] hasx,hasz,hast,geodetic Dimensionality flags
 * @param[in] srid Spatial reference system (0 = unknown)
 * @param[in] pcid pgpointcloud schema id (0 = unknown)
 * @param[in] xmin,xmax,ymin,ymax,zmin,zmax Spatial bounds (ignored unless
 *   @p hasx; @p zmin/@p zmax ignored unless @p hasz)
 * @param[in] period Time bounds (ignored unless @p hast; may be NULL)
 */
TPCBox *
tpcbox_make(bool hasx, bool hasz, bool hast, bool geodetic,
  int32_t srid, uint32_t pcid, double xmin, double xmax, double ymin,
  double ymax, double zmin, double zmax, const Span *period)
{
  if (! hasx && ! hast)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "TPCBox must have at least one of spatial (X) or temporal (T) "
      "dimension");
    return NULL;
  }
  TPCBox *result = palloc0(sizeof(TPCBox));
  result->srid = srid;
  result->pcid = pcid;
  MEOS_FLAGS_SET_X(result->flags, hasx);
  MEOS_FLAGS_SET_Z(result->flags, hasz);
  MEOS_FLAGS_SET_T(result->flags, hast);
  MEOS_FLAGS_SET_GEODETIC(result->flags, geodetic);
  if (hasx)
  {
    result->xmin = xmin; result->xmax = xmax;
    result->ymin = ymin; result->ymax = ymax;
    if (hasz)
    {
      result->zmin = zmin; result->zmax = zmax;
    }
  }
  if (hast && period)
    memcpy(&result->period, period, sizeof(Span));
  return result;
}

/**
 * @ingroup meos_pointcloud_box_constructor
 * @brief Return a palloc'd copy of a TPCBox.
 */
TPCBox *
tpcbox_copy(const TPCBox *box)
{
  VALIDATE_TPCBOX(box, NULL);
  TPCBox *result = palloc(sizeof(TPCBox));
  memcpy(result, box, sizeof(TPCBox));
  return result;
}

/*****************************************************************************
 * Conversion
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_box_conversion
 * @brief Return the 2D spatial bounding box of a pcpatch as a TPCBox.
 * @note Z is not populated — pgpointcloud's SERIALIZED_PATCH carries only
 *   2D @c PCBOUNDS in its header; the per-point Z values are packed
 *   inside the compressed data block and cannot be summarised without
 *   schema access. Time dimension is likewise absent (pcpatch is static).
 *   Callers that need Z get it once Phase 8G's schema layer lands.
 */
TPCBox *
pcpatch_to_tpcbox(const Pcpatch *pa, int32_t srid)
{
  VALIDATE_NOT_NULL(pa, NULL);
  Span empty_period;
  memset(&empty_period, 0, sizeof(Span));
  return tpcbox_make(
    /* hasx */ true, /* hasz */ false, /* hast */ false,
    /* geodetic */ false, srid, pa->pcid,
    pa->bounds[0], pa->bounds[2],  /* xmin, xmax */
    pa->bounds[1], pa->bounds[3],  /* ymin, ymax */
    0.0, 0.0,                      /* zmin, zmax (unused) */
    &empty_period);
}

/*****************************************************************************
 * Accessors
 *
 * Flag-peek predicates return bool directly; coordinate accessors return
 * true / write into the out-pointer if the corresponding dimension is
 * set, mirroring how @c stbox_xmin and friends behave.
 *****************************************************************************/

bool tpcbox_hasx(const TPCBox *box)
{ VALIDATE_TPCBOX(box, false); return MEOS_FLAGS_GET_X(box->flags); }
bool tpcbox_hasz(const TPCBox *box)
{ VALIDATE_TPCBOX(box, false); return MEOS_FLAGS_GET_Z(box->flags); }
bool tpcbox_hast(const TPCBox *box)
{ VALIDATE_TPCBOX(box, false); return MEOS_FLAGS_GET_T(box->flags); }
bool tpcbox_geodetic(const TPCBox *box)
{ VALIDATE_TPCBOX(box, false); return MEOS_FLAGS_GET_GEODETIC(box->flags); }

bool
tpcbox_xmin(const TPCBox *box, double *result)
{
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags)) return false;
  *result = box->xmin; return true;
}
bool
tpcbox_xmax(const TPCBox *box, double *result)
{
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags)) return false;
  *result = box->xmax; return true;
}
bool
tpcbox_ymin(const TPCBox *box, double *result)
{
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags)) return false;
  *result = box->ymin; return true;
}
bool
tpcbox_ymax(const TPCBox *box, double *result)
{
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags)) return false;
  *result = box->ymax; return true;
}
bool
tpcbox_zmin(const TPCBox *box, double *result)
{
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_Z(box->flags)) return false;
  *result = box->zmin; return true;
}
bool
tpcbox_zmax(const TPCBox *box, double *result)
{
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_Z(box->flags)) return false;
  *result = box->zmax; return true;
}
bool
tpcbox_tmin(const TPCBox *box, TimestampTz *result)
{
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_T(box->flags)) return false;
  *result = DatumGetTimestampTz(box->period.lower); return true;
}
bool
tpcbox_tmax(const TPCBox *box, TimestampTz *result)
{
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_T(box->flags)) return false;
  *result = DatumGetTimestampTz(box->period.upper); return true;
}

int32_t
tpcbox_srid(const TPCBox *box)
{ VALIDATE_TPCBOX(box, 0); return box->srid; }

uint32_t
tpcbox_pcid(const TPCBox *box)
{ VALIDATE_TPCBOX(box, 0); return box->pcid; }

/*****************************************************************************
 * Transformation — expand
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_box_transf
 * @brief Expand box2 in place to include box1.
 * @pre Callers must ensure pcid/SRID compatibility first.
 */
void
tpcbox_expand(const TPCBox *box1, TPCBox *box2)
{
  assert(box1); assert(box2);
  if (MEOS_FLAGS_GET_X(box2->flags) && MEOS_FLAGS_GET_X(box1->flags))
  {
    box2->xmin = Min(box1->xmin, box2->xmin);
    box2->xmax = Max(box1->xmax, box2->xmax);
    box2->ymin = Min(box1->ymin, box2->ymin);
    box2->ymax = Max(box1->ymax, box2->ymax);
    if (MEOS_FLAGS_GET_Z(box2->flags) && MEOS_FLAGS_GET_Z(box1->flags))
    {
      box2->zmin = Min(box1->zmin, box2->zmin);
      box2->zmax = Max(box1->zmax, box2->zmax);
    }
  }
  if (MEOS_FLAGS_GET_T(box2->flags) && MEOS_FLAGS_GET_T(box1->flags))
    span_expand(&box1->period, &box2->period);
  /* Propagate pcid / srid from the enriching box if box2's was unset */
  if (box2->pcid == 0) box2->pcid = box1->pcid;
  if (box2->srid == 0) box2->srid = box1->srid;
}

/*****************************************************************************
 * Set operations
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_box_setops
 * @brief Return the union of two TPCBoxes.
 * @param[in] box1,box2 Input
 * @param[in] strict If true, require the boxes to overlap (strict union);
 *   if false, return the smallest box enclosing both even when disjoint.
 */
TPCBox *
union_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2, bool strict)
{
  VALIDATE_TPCBOX(box1, NULL); VALIDATE_TPCBOX(box2, NULL);
  if (! ensure_same_pcid_tpcbox(box1, box2) ||
      ! ensure_same_srid_tpcbox(box1, box2))
    return NULL;
  if (MEOS_FLAGS_GET_X(box1->flags) != MEOS_FLAGS_GET_X(box2->flags) ||
      MEOS_FLAGS_GET_Z(box1->flags) != MEOS_FLAGS_GET_Z(box2->flags) ||
      MEOS_FLAGS_GET_T(box1->flags) != MEOS_FLAGS_GET_T(box2->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "TPCBox union requires matching dimensionality flags");
    return NULL;
  }
  if (strict && ! overlaps_tpcbox_tpcbox(box1, box2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Strict union requires overlapping TPCBox values");
    return NULL;
  }
  TPCBox *result = tpcbox_copy(box1);
  tpcbox_expand(box2, result);
  return result;
}

/**
 * @ingroup meos_internal_pointcloud_box_setops
 * @brief Write the intersection of two TPCBoxes into @p result.
 * @return true if the boxes intersect (result is valid); false otherwise.
 */
bool
inter_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2, TPCBox *result)
{
  assert(box1); assert(box2); assert(result);
  if (! ensure_same_pcid_tpcbox(box1, box2) ||
      ! ensure_same_srid_tpcbox(box1, box2))
    return false;
  bool hasx = MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags);
  bool hasz = MEOS_FLAGS_GET_Z(box1->flags) && MEOS_FLAGS_GET_Z(box2->flags);
  bool hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  if (! hasx && ! hast)
    return false;  /* no shared dimensions → no intersection */

  memset(result, 0, sizeof(TPCBox));
  result->srid = (box1->srid != 0) ? box1->srid : box2->srid;
  result->pcid = (box1->pcid != 0) ? box1->pcid : box2->pcid;
  MEOS_FLAGS_SET_X(result->flags, hasx);
  MEOS_FLAGS_SET_Z(result->flags, hasz);
  MEOS_FLAGS_SET_T(result->flags, hast);
  MEOS_FLAGS_SET_GEODETIC(result->flags,
    MEOS_FLAGS_GET_GEODETIC(box1->flags));

  if (hasx)
  {
    result->xmin = Max(box1->xmin, box2->xmin);
    result->xmax = Min(box1->xmax, box2->xmax);
    if (result->xmin > result->xmax) return false;
    result->ymin = Max(box1->ymin, box2->ymin);
    result->ymax = Min(box1->ymax, box2->ymax);
    if (result->ymin > result->ymax) return false;
    if (hasz)
    {
      result->zmin = Max(box1->zmin, box2->zmin);
      result->zmax = Min(box1->zmax, box2->zmax);
      if (result->zmin > result->zmax) return false;
    }
  }
  if (hast)
  {
    if (! inter_span_span(&box1->period, &box2->period, &result->period))
      return false;
  }
  return true;
}

/**
 * @ingroup meos_pointcloud_box_setops
 * @brief Return the intersection of two TPCBoxes, or NULL if disjoint.
 */
TPCBox *
intersection_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  VALIDATE_TPCBOX(box1, NULL); VALIDATE_TPCBOX(box2, NULL);
  TPCBox tmp;
  if (! inter_tpcbox_tpcbox(box1, box2, &tmp))
    return NULL;
  TPCBox *result = palloc(sizeof(TPCBox));
  memcpy(result, &tmp, sizeof(TPCBox));
  return result;
}

/*****************************************************************************
 * Topological predicates
 *****************************************************************************/

/**
 * @brief Shared dimension check: returns the effective (hasx, hasz, hast)
 * triple for comparing @p box1 vs @p box2 — predicates only make sense on
 * dimensions both boxes carry.
 */
static inline void
tpcbox_shared_dims(const TPCBox *box1, const TPCBox *box2,
  bool *hasx, bool *hasz, bool *hast)
{
  *hasx = MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags);
  *hasz = MEOS_FLAGS_GET_Z(box1->flags) && MEOS_FLAGS_GET_Z(box2->flags);
  *hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
}

bool
contains_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  VALIDATE_TPCBOX(box1, false); VALIDATE_TPCBOX(box2, false);
  if (! ensure_same_pcid_tpcbox(box1, box2)) return false;
  bool hasx, hasz, hast;
  tpcbox_shared_dims(box1, box2, &hasx, &hasz, &hast);
  /* box2 must have at least the dimensions box1 requires */
  if (MEOS_FLAGS_GET_X(box1->flags) && ! MEOS_FLAGS_GET_X(box2->flags))
    return false;
  if (MEOS_FLAGS_GET_T(box1->flags) && ! MEOS_FLAGS_GET_T(box2->flags))
    return false;
  if (hasx)
  {
    if (box2->xmin < box1->xmin || box2->xmax > box1->xmax) return false;
    if (box2->ymin < box1->ymin || box2->ymax > box1->ymax) return false;
    if (hasz && (box2->zmin < box1->zmin || box2->zmax > box1->zmax))
      return false;
  }
  if (hast && ! contains_span_span(&box1->period, &box2->period))
    return false;
  return true;
}

bool
contained_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  return contains_tpcbox_tpcbox(box2, box1);
}

bool
overlaps_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  VALIDATE_TPCBOX(box1, false); VALIDATE_TPCBOX(box2, false);
  if (! ensure_same_pcid_tpcbox(box1, box2)) return false;
  bool hasx, hasz, hast;
  tpcbox_shared_dims(box1, box2, &hasx, &hasz, &hast);
  if (hasx)
  {
    if (box1->xmax < box2->xmin || box2->xmax < box1->xmin) return false;
    if (box1->ymax < box2->ymin || box2->ymax < box1->ymin) return false;
    if (hasz && (box1->zmax < box2->zmin || box2->zmax < box1->zmin))
      return false;
  }
  if (hast && ! overlaps_span_span(&box1->period, &box2->period))
    return false;
  return true;
}

bool
same_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  return tpcbox_eq(box1, box2);
}

bool
adjacent_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  VALIDATE_TPCBOX(box1, false); VALIDATE_TPCBOX(box2, false);
  if (! ensure_same_pcid_tpcbox(box1, box2)) return false;
  /* Adjacent = their intersection is empty (no interior overlap) AND
   * their expansion-union shares a boundary. Simplest correct
   * implementation: touch on at least one face of a shared dimension
   * and do not overlap on any other. Use the Span adjacency helper for
   * the time dimension; hand-coded for spatial. */
  TPCBox inter;
  if (inter_tpcbox_tpcbox(box1, box2, &inter))
    return false; /* overlaps strictly → not adjacent */
  bool hasx, hasz, hast;
  tpcbox_shared_dims(box1, box2, &hasx, &hasz, &hast);
  if (hasx)
  {
    if (box1->xmax == box2->xmin || box2->xmax == box1->xmin ||
        box1->ymax == box2->ymin || box2->ymax == box1->ymin)
      return true;
    if (hasz &&
        (box1->zmax == box2->zmin || box2->zmax == box1->zmin))
      return true;
  }
  if (hast && adjacent_span_span(&box1->period, &box2->period))
    return true;
  return false;
}

/*****************************************************************************
 * Comparison
 *
 * Total order: pcid, then srid, then flags, then period, then spatial
 * bounds in XYZ-min / XYZ-max order. Deterministic; usable by B-tree.
 *****************************************************************************/

int
tpcbox_cmp(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  if (box1->pcid != box2->pcid)
    return (box1->pcid < box2->pcid) ? -1 : 1;
  if (box1->srid != box2->srid)
    return (box1->srid < box2->srid) ? -1 : 1;
  if (box1->flags != box2->flags)
    return (box1->flags < box2->flags) ? -1 : 1;
  if (MEOS_FLAGS_GET_T(box1->flags))
  {
    int c = span_cmp(&box1->period, &box2->period);
    if (c != 0) return c;
  }
  if (MEOS_FLAGS_GET_X(box1->flags))
  {
    if (box1->xmin != box2->xmin) return (box1->xmin < box2->xmin) ? -1 : 1;
    if (box1->ymin != box2->ymin) return (box1->ymin < box2->ymin) ? -1 : 1;
    if (MEOS_FLAGS_GET_Z(box1->flags) && box1->zmin != box2->zmin)
      return (box1->zmin < box2->zmin) ? -1 : 1;
    if (box1->xmax != box2->xmax) return (box1->xmax < box2->xmax) ? -1 : 1;
    if (box1->ymax != box2->ymax) return (box1->ymax < box2->ymax) ? -1 : 1;
    if (MEOS_FLAGS_GET_Z(box1->flags) && box1->zmax != box2->zmax)
      return (box1->zmax < box2->zmax) ? -1 : 1;
  }
  return 0;
}

bool tpcbox_eq(const TPCBox *box1, const TPCBox *box2)
{ return tpcbox_cmp(box1, box2) == 0; }
bool tpcbox_ne(const TPCBox *box1, const TPCBox *box2)
{ return tpcbox_cmp(box1, box2) != 0; }
bool tpcbox_lt(const TPCBox *box1, const TPCBox *box2)
{ return tpcbox_cmp(box1, box2) <  0; }
bool tpcbox_le(const TPCBox *box1, const TPCBox *box2)
{ return tpcbox_cmp(box1, box2) <= 0; }
bool tpcbox_gt(const TPCBox *box1, const TPCBox *box2)
{ return tpcbox_cmp(box1, box2) >  0; }
bool tpcbox_ge(const TPCBox *box1, const TPCBox *box2)
{ return tpcbox_cmp(box1, box2) >= 0; }

/*****************************************************************************/
