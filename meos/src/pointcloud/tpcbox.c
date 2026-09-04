/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * cannot share a bbox. Provides the base set of operators used by
 * downstream temporal types (tpcpoint, tpcpatch).
 *
 * Scope notes:
 *   * @c pcpatch → @c TPCBox is implemented here and is a free conversion
 *     (pgpointcloud's @c SERIALIZED_PATCH already carries a 2D @c PCBOUNDS
 *     header — no schema lookup required; no Z).
 *   * @c pcpoint → @c TPCBox lives in the PG wrapper layer: extracting
 *     X/Y/Z from a pcpoint byte blob requires the schema XML (loaded
 *     from @c pointcloud_formats by pcid), which only the PG layer can
 *     do.
 */

#include "pointcloud/tpcbox.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <meos_pointcloud.h>
#include <pgtypes.h>
#include "temporal/span.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "geo/tspatial_parser.h"
#include "pointcloud/pcpatch.h"
#include "pointcloud/pgsql_compat.h"
#include "pointcloud/meos_schema_hook.h"

/* Buffer size for input/output of TPCBox text form */
#define TPCBOX_MAXLEN  512

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Ensure two TPCBoxes share the same schema (pcid).
 * @note The schemas are compared as they are stated: a pcid of 0 names no
 * schema, and a box carrying coordinates under it is not comparable with a box
 * carrying them under a schema that does. The X guard in
 * #ensure_valid_tpcbox_tpcbox is what lets a box holding no coordinates meet a
 * box of any schema.
 */
static bool
ensure_same_pcid_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  if (box1->pcid != box2->pcid)
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
 * @note The reference systems are compared as they are stated, which is the
 * same strict equality #ensure_same_srid applies to the spatiotemporal box.
 */
static bool
ensure_same_srid_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  if (box1->srid != box2->srid)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on TPCBox values with different SRIDs: %d vs %d",
      box1->srid, box2->srid);
    return false;
  }
  return true;
}

/**
 * @brief Return true if two temporal pointcloud boxes are valid for operations
 * @details The boxes are comparable when they name the same schema, which is
 * what gives their coordinates a meaning: a schema states the dimensions a
 * value holds and what a stored number reads as, and it states the SRID every
 * value carrying that pcid inherits
 * @param[in] box1,box2 Temporal pointcloud boxes
 */
bool
ensure_valid_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box1, false); VALIDATE_TPCBOX(box2, false);
  /* Both boxes carry coordinates here, so both name the schema those
   * coordinates are read in. A box carrying none has nothing to interpret,
   * so it meets a box of any schema */
  if (MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags) &&
      (! ensure_same_pcid_tpcbox(box1, box2) ||
       ! ensure_same_srid_tpcbox(box1, box2)))
    return false;
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
 * @brief Parse a TPCBox from its Well-Known Text (WKT) representation
 * @details Mirrors #stbox_parse: an optional `SRID=` prefix, then the
 * `(GEOD)TPCBOX` keyword wrapping — in parentheses — the box body shared with
 * STBox (parsed by #stbox_parse_dims) and the `pcid` as a final component.
 * Round-trips with #tpcbox_out.
 */
TPCBox *
tpcbox_parse(const char **str)
{
  assert(str);
  const char *type_str = meostype_name(T_TPCBOX);
  /* Get the SRID if it is given */
  int32_t srid;
  bool hassrid = srid_parse(str, &srid);
  bool geodetic = false;
  /* Determine whether the box is geodetic or not */
  if (pg_strncasecmp(*str, "TPCBOX", 6) == 0)
  {
    *str += 6;
    p_whitespace(str);
  }
  else if (pg_strncasecmp(*str, "GEODTPCBOX", 10) == 0)
  {
    *str += 10;
    geodetic = true;
    p_whitespace(str);
    if (! hassrid)
      srid = WGS84_SRID;
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing prefix (GEOD)TPCBOX", type_str);
    return NULL;
  }

  /* Opening parenthesis of the composite value */
  p_whitespace(str);
  if (! ensure_oparen(str, type_str))
    return NULL;

  /* Parse the box body shared with STBox */
  p_whitespace(str);
  STBox *box = stbox_parse_dims(str, geodetic, srid, type_str);
  if (! box)
    return NULL;

  /* Comma before the PCID component */
  p_whitespace(str);
  if (! p_comma(str))
  {
    pfree(box);
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing comma before PCID", type_str);
    return NULL;
  }
  /* Parse the PCID */
  p_whitespace(str);
  if (! (**str >= '0' && **str <= '9'))
  {
    pfree(box);
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Invalid PCID", type_str);
    return NULL;
  }
  uint32_t pcid = 0;
  while (**str >= '0' && **str <= '9')
  {
    pcid = pcid * 10 + (uint32_t) (**str - '0');
    *str += 1;
  }

  /* Closing parenthesis of the composite value */
  p_whitespace(str);
  if (! ensure_cparen(str, type_str))
  {
    pfree(box);
    return NULL;
  }

  /* Ensure there is no more input */
  if (! ensure_end_input(str, type_str))
  {
    pfree(box);
    return NULL;
  }

  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);

  /* Reconcile the two levels a TPCBox states its reference system at: the
   * `SRID=` prefix the value carries and the schema its pcid names. This is
   * the one site where both are genuinely written, so it is the one site that
   * owes the reconciliation. A level reading `SRID_UNKNOWN` states nothing, so
   * either level alone carries the answer and the schema is preferred where it
   * speaks; two levels stating different systems is a value contradicting
   * itself, which is an error rather than a choice between them. Where no
   * schema resolves — a pcid of 0, or a MEOS program with no catalog behind
   * it — the prefix is the only level there is, so reading a value never
   * requires a catalog to be reachable */
  srid = box->srid;
  if (pcid != 0)
  {
    int32_t schema_srid = meos_pc_schema_srid(pcid);
    if (schema_srid != SRID_INVALID && schema_srid != SRID_UNKNOWN)
    {
      if (srid != SRID_UNKNOWN && srid != schema_srid)
      {
        pfree(box);
        meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
          "Could not parse %s value: The value states SRID %d and the schema "
          "of pcid %u states SRID %d", type_str, srid, pcid, schema_srid);
        return NULL;
      }
      srid = schema_srid;
    }
  }

  TPCBox *result = tpcbox_make(hasx, hasz, hast, geodetic, srid, pcid,
    box->xmin, box->xmax, box->ymin, box->ymax, box->zmin, box->zmax,
    hast ? &box->period : NULL);
  pfree(box);
  return result;
}

/**
 * @ingroup meos_pointcloud_box_inout
 * @brief Return a TPCBox from its Well-Known Text (WKT) representation
 * @details Round-trips with #tpcbox_out, matching the `(GEOD)STBOX` text form
 * of the sibling STBox plus a trailing `PCID`. Binary interchange goes through
 * @c recv / @c send.
 * @csqlfn #Tpcbox_in()
 */
TPCBox *
tpcbox_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return tpcbox_parse(&str);
}

/**
 * @ingroup meos_pointcloud_box_inout
 * @brief Return the text representation of a TPCBox.
 * @details Examples (commas optional inside the parenthesised groups):
 * @code
 * TPCBOX(X((10, 20), (30, 40)), 1)
 * SRID=4326;TPCBOX(Z((10, 20, 30), (40, 50, 60)), 1)
 * TPCBOX(XT(((10, 20), (30, 40)), [2024-01-01, 2024-01-02]), 1)
 * TPCBOX(T([2024-01-01, 2024-01-02]), 1)
 * GEODTPCBOX(ZT(((10, 20, 30), (40, 50, 60)), [2024-01-01, 2024-01-02]), 1)
 * @endcode
 * @csqlfn #Tpcbox_out()
 */
char *
tpcbox_out(const TPCBox *box, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  char *xmin = NULL, *xmax = NULL, *ymin = NULL, *ymax = NULL, *zmin = NULL,
    *zmax = NULL, *period = NULL;
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(box->flags);

  char *str = palloc(TPCBOX_MAXLEN);
  /* SRID is emitted as a prefix, matching the sibling (GEOD)STBOX text form */
  char srid[18];
  if (hasx && box->srid > 0)
    snprintf(srid, sizeof(srid), "SRID=%d;", box->srid);
  else
    srid[0] = '\0';
  const char *boxtype = geodetic ? "GEODTPCBOX" : "TPCBOX";
  if (hast)
    period = span_out(&box->period, maxdd);

  if (hasx && hast)
  {
    xmin = float8_out(box->xmin, maxdd);
    xmax = float8_out(box->xmax, maxdd);
    ymin = float8_out(box->ymin, maxdd);
    ymax = float8_out(box->ymax, maxdd);
    if (hasz)
    {
      zmin = float8_out(box->zmin, maxdd);
      zmax = float8_out(box->zmax, maxdd);
      snprintf(str, TPCBOX_MAXLEN,
        "%s%s(ZT(((%s,%s,%s),(%s,%s,%s)),%s), %u)",
        srid, boxtype, xmin, ymin, zmin, xmax, ymax, zmax, period, box->pcid);
    }
    else
      snprintf(str, TPCBOX_MAXLEN, "%s%s(XT(((%s,%s),(%s,%s)),%s), %u)",
        srid, boxtype, xmin, ymin, xmax, ymax, period, box->pcid);
  }
  else if (hasx)
  {
    xmin = float8_out(box->xmin, maxdd);
    xmax = float8_out(box->xmax, maxdd);
    ymin = float8_out(box->ymin, maxdd);
    ymax = float8_out(box->ymax, maxdd);
    if (hasz)
    {
      zmin = float8_out(box->zmin, maxdd);
      zmax = float8_out(box->zmax, maxdd);
      snprintf(str, TPCBOX_MAXLEN, "%s%s(Z((%s,%s,%s),(%s,%s,%s)), %u)",
        srid, boxtype, xmin, ymin, zmin, xmax, ymax, zmax, box->pcid);
    }
    else
      snprintf(str, TPCBOX_MAXLEN, "%s%s(X((%s,%s),(%s,%s)), %u)",
        srid, boxtype, xmin, ymin, xmax, ymax, box->pcid);
  }
  else /* hast */
    snprintf(str, TPCBOX_MAXLEN, "%s%s(T(%s), %u)", srid, boxtype, period,
      box->pcid);

  if (hasx)
  {
    pfree(xmin); pfree(xmax); pfree(ymin); pfree(ymax);
    if (hasz)
    {
      pfree(zmin); pfree(zmax);
    }
  }
  if (hast)
    pfree(period);
  return str;
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
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, NULL);
  TPCBox *result = palloc(sizeof(TPCBox));
  memcpy(result, box, sizeof(TPCBox));
  return result;
}

/*****************************************************************************
 * Conversion
 *****************************************************************************/

/**
 * @brief Return the Z extent of the points of a patch
 * @details The @c PCBOUNDS header of a patch states its X and Y extent only,
 * so the Z extent is the one its points state and is reached by walking them,
 * the way @ref pcpatch_to_geom reaches their Z coordinate.
 * @param[in] pa Patch
 * @param[in] schema Schema the pcid of the patch names
 * @param[out] zmin,zmax Z extent of the points stating a Z coordinate
 * @return @p true if some point states a Z coordinate, so that the extent is
 *   set, and @p false otherwise
 */
static bool
pcpatch_z_extent(const Pcpatch *pa, const PCSCHEMA *schema, double *zmin,
  double *zmax)
{
  /* Pcpatch is byte-compatible with SERIALIZED_PATCH (see pcpatch.h) */
  PCPATCH *patch = MEOS_PC_PATCH_DESERIALIZE(
    (const SERIALIZED_PATCH *) pa, schema);
  if (! patch)
    return false;
  PCPOINTLIST *pl = pc_pointlist_from_patch(patch);
  if (! pl)
  {
    pc_patch_free(patch);
    return false;
  }

  bool found = false;
  for (uint32_t i = 0; i < pl->npoints; i++)
  {
    double z;
    if (! pc_point_get_z(pc_pointlist_get_point(pl, i), &z))
      continue;
    if (! found)
    {
      *zmin = *zmax = z;
      found = true;
    }
    else if (z < *zmin)
      *zmin = z;
    else if (z > *zmax)
      *zmax = z;
  }

  pc_pointlist_free(pl);
  pc_patch_free(patch);
  return found;
}

/**
 * @ingroup meos_pointcloud_box_conversion
 * @brief Return the spatial bounding box of a pcpatch as a TPCBox.
 * @details The schema the pcid of the patch names decides whether the box
 * carries a Z dimension, in the same way it decides whether the box of a
 * pcpoint carries one. The X and Y extent is the one the @c PCBOUNDS header
 * of the patch states, which is why a patch of a pcid no schema states still
 * answers the extent it holds; the Z extent is the one its points state, so a
 * patch of a schema holding Z is walked once. The time dimension is absent, a
 * patch being static.
 * @param[in] pa Patch
 * @param[in] srid Spatial reference system of the result, which the caller
 *   states so that it can override the one of the schema
 * @csqlfn #Pcpatch_to_tpcbox()
 */
TPCBox *
pcpatch_to_tpcbox(const Pcpatch *pa, int32_t srid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, NULL);
  const PCSCHEMA *schema = meos_pc_schema_lookup(pa->pcid);
  double zmin = 0.0, zmax = 0.0;
  bool hasz = schema && schema->zdim &&
    pcpatch_z_extent(pa, schema, &zmin, &zmax);

  Span empty_period;
  memset(&empty_period, 0, sizeof(Span));
  return tpcbox_make(
    /* hasx */ true, hasz, /* hast */ false,
    /* geodetic */ false, srid, pa->pcid,
    /* PCBOUNDS is {xmin, xmax, ymin, ymax}, the order the bounds field of
     * struct Pcpatch states and pointcloud-pg/lib/pc_api.h defines */
    pa->bounds[0], pa->bounds[1],  /* xmin, xmax */
    pa->bounds[2], pa->bounds[3],  /* ymin, ymax */
    zmin, zmax,
    &empty_period);
}

/*****************************************************************************
 * Accessors
 *
 * Flag-peek predicates return bool directly; coordinate accessors return
 * true / write into the out-pointer if the corresponding dimension is
 * set, mirroring how @c stbox_xmin and friends behave.
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return @p true if a TPCBox has its X dimension set
 * @csqlfn #Tpcbox_hasx()
 */
bool
tpcbox_hasx(const TPCBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false);
  return stbox_hasx((const STBox *) box);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return @p true if a TPCBox has its Z dimension set
 * @csqlfn #Tpcbox_hasz()
 */
bool
tpcbox_hasz(const TPCBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false);
  return stbox_hasz((const STBox *) box);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return @p true if a TPCBox has its T (time) dimension set
 * @csqlfn #Tpcbox_hast()
 */
bool
tpcbox_hast(const TPCBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false);
  return stbox_hast((const STBox *) box);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return @p true if a TPCBox is in a geographic SRID
 */
bool
tpcbox_geodetic(const TPCBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false);
  return stbox_isgeodetic((const STBox *) box);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return in the last argument the minimum X value of a TPCBox
 * @param[in] box Box
 * @param[out] result Result
 * @return @p true on success, @p false if the box has no X dimension
 * @csqlfn #Tpcbox_xmin()
 */
bool
tpcbox_xmin(const TPCBox *box, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  return stbox_xmin((const STBox *) box, result);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return in the last argument the maximum X value of a TPCBox
 * @param[in] box Box
 * @param[out] result Result
 * @return @p true on success, @p false if the box has no X dimension
 * @csqlfn #Tpcbox_xmax()
 */
bool
tpcbox_xmax(const TPCBox *box, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  return stbox_xmax((const STBox *) box, result);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return in the last argument the minimum Y value of a TPCBox
 * @param[in] box Box
 * @param[out] result Result
 * @return @p true on success, @p false if the box has no XY dimensions
 * @csqlfn #Tpcbox_ymin()
 */
bool
tpcbox_ymin(const TPCBox *box, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  return stbox_ymin((const STBox *) box, result);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return in the last argument the maximum Y value of a TPCBox
 * @param[in] box Box
 * @param[out] result Result
 * @return @p true on success, @p false if the box has no XY dimensions
 * @csqlfn #Tpcbox_ymax()
 */
bool
tpcbox_ymax(const TPCBox *box, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  return stbox_ymax((const STBox *) box, result);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return in the last argument the minimum Z value of a TPCBox
 * @param[in] box Box
 * @param[out] result Result
 * @return @p true on success, @p false if the box has no Z dimension
 * @csqlfn #Tpcbox_zmin()
 */
bool
tpcbox_zmin(const TPCBox *box, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  return stbox_zmin((const STBox *) box, result);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return in the last argument the maximum Z value of a TPCBox
 * @param[in] box Box
 * @param[out] result Result
 * @return @p true on success, @p false if the box has no Z dimension
 * @csqlfn #Tpcbox_zmax()
 */
bool
tpcbox_zmax(const TPCBox *box, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  return stbox_zmax((const STBox *) box, result);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return in the last argument the minimum T value of a TPCBox
 * @param[in] box Box
 * @param[out] result Result
 * @return @p true on success, @p false if the box has no T dimension
 * @csqlfn #Tpcbox_tmin()
 */
bool
tpcbox_tmin(const TPCBox *box, TimestampTz *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  return stbox_tmin((const STBox *) box, result);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return in the last argument whether the minimum T value of a TPCBox
 * is inclusive
 * @param[in] box Box
 * @param[out] result Result
 * @return @p true on success, @p false if the box has no T dimension
 * @csqlfn #Tpcbox_tmin_inc()
 */
bool
tpcbox_tmin_inc(const TPCBox *box, bool *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  return stbox_tmin_inc((const STBox *) box, result);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return in the last argument the maximum T value of a TPCBox
 * @param[in] box Box
 * @param[out] result Result
 * @return @p true on success, @p false if the box has no T dimension
 * @csqlfn #Tpcbox_tmax()
 */
bool
tpcbox_tmax(const TPCBox *box, TimestampTz *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  return stbox_tmax((const STBox *) box, result);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return in the last argument whether the maximum T value of a TPCBox
 * is inclusive
 * @param[in] box Box
 * @param[out] result Result
 * @return @p true on success, @p false if the box has no T dimension
 * @csqlfn #Tpcbox_tmax_inc()
 */
bool
tpcbox_tmax_inc(const TPCBox *box, bool *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, false); VALIDATE_NOT_NULL(result, false);
  return stbox_tmax_inc((const STBox *) box, result);
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return the SRID of a TPCBox
 * @csqlfn #Tpcbox_srid()
 */
int32_t
tpcbox_srid(const TPCBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, 0);
  return box->srid;
}

/**
 * @ingroup meos_pointcloud_box_accessor
 * @brief Return the pgPointCloud schema id (pcid) of a TPCBox
 * @csqlfn #Tpcbox_pcid()
 */
uint32_t
tpcbox_pcid(const TPCBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, 0);
  return box->pcid;
}

/**
 * @ingroup meos_pointcloud_box_conversion
 * @brief Project a TPCBox to a STBox by dropping the pcid.
 * @details Lets users compose tpcbox values into stbox-only operators
 *   (extent aggregation, etc.) without a manual constructor. The
 *   spatial / temporal extent and SRID transfer unchanged.
 * @csqlfn #Tpcbox_to_stbox()
 */
STBox *
tpcbox_to_stbox(const TPCBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, NULL);
  STBox *result = palloc(sizeof(STBox));
  result->period = box->period;
  result->xmin = box->xmin;
  result->ymin = box->ymin;
  result->zmin = box->zmin;
  result->xmax = box->xmax;
  result->ymax = box->ymax;
  result->zmax = box->zmax;
  result->srid = box->srid;
  result->flags = box->flags;
  return result;
}

/*****************************************************************************
 * Transformation
 *****************************************************************************/

/**
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
  /* The schema and its SRID are not extent: a combine grows what the boxes
   * measure and leaves what they are measured in alone, the comparability
   * gate having already established the two agree */
}

/**
 * @ingroup meos_pointcloud_box_transf
 * @brief Return a tpcbox with coordinate bounds rounded to a given
 *   number of decimal digits.
 * @param[in] box Bounding box
 * @param[in] maxdd Maximum number of decimal digits (must be >= 0)
 * @return Newly-palloc'd TPCBox, or @p NULL on invalid argument.
 * @csqlfn #Tpcbox_round()
 */
TPCBox *
tpcbox_round(const TPCBox *box, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  TPCBox *result = tpcbox_copy(box);
  if (MEOS_FLAGS_GET_X(box->flags))
  {
    result->xmin = float8_round(box->xmin, maxdd);
    result->xmax = float8_round(box->xmax, maxdd);
    result->ymin = float8_round(box->ymin, maxdd);
    result->ymax = float8_round(box->ymax, maxdd);
    if (MEOS_FLAGS_GET_Z(box->flags))
    {
      result->zmin = float8_round(box->zmin, maxdd);
      result->zmax = float8_round(box->zmax, maxdd);
    }
  }
  return result;
}

/**
 * @ingroup meos_pointcloud_box_transf
 * @brief Return a tpcbox stating a reference system its schema does not state
 * @details The schema a pcid names is what holds the reference system, so
 * where that schema states one it is not the caller's to set and the box is
 * refused. A pcid of 0 names no schema and a schema may state none, and there
 * the box itself is the only level there is. Coordinates are not transformed
 * either way; to reproject, project the underlying tpcpoint and take its box.
 * @csqlfn #Tpcbox_set_srid()
 */
TPCBox *
tpcbox_set_srid(const TPCBox *box, int32_t srid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCBOX(box, NULL);
  if (box->pcid != 0)
  {
    int32_t schema_srid = meos_pc_schema_srid(box->pcid);
    if (schema_srid != SRID_INVALID && schema_srid != SRID_UNKNOWN)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The SRID of a TPCBox is the one its schema states: pcid %u states "
        "SRID %d", box->pcid, schema_srid);
      return NULL;
    }
  }
  TPCBox *result = tpcbox_copy(box);
  result->srid = srid;
  return result;
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
 * @csqlfn #Union_tpcbox_tpcbox()
 */
TPCBox *
union_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2, bool strict)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2))
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
 * @csqlfn #Intersection_tpcbox_tpcbox()
 */
bool
inter_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2, TPCBox *result)
{
  assert(box1); assert(box2); assert(result);
  bool hasx = MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags);
  bool hasz = MEOS_FLAGS_GET_Z(box1->flags) && MEOS_FLAGS_GET_Z(box2->flags);
  bool hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  if (! hasx && ! hast)
    return false;  /* no shared dimensions → no intersection */

  memset(result, 0, sizeof(TPCBox));
  result->srid = box1->srid;
  result->pcid = box1->pcid;
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
 * @csqlfn #Intersection_tpcbox_tpcbox()
 */
TPCBox *
intersection_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2))
    return NULL;
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
 * @ingroup meos_internal_pointcloud_box_topo
 * @brief Return @p true if the first TPCBox contains the second
 */
bool
tpcbox_contains(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  return stbox_contains((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_topo
 * @brief Return @p true if the first TPCBox contains the second
 * @csqlfn #Contains_tpcbox_tpcbox()
 */
bool
contains_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_common_dimension(box1->flags, box2->flags))
    return false;
  return tpcbox_contains(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_topo
 * @brief Return @p true if the first TPCBox is contained in the second
 */
bool
tpcbox_contained(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  return stbox_contained((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_topo
 * @brief Return @p true if the first TPCBox is contained in the second
 * @csqlfn #Contained_tpcbox_tpcbox()
 */
bool
contained_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_common_dimension(box1->flags, box2->flags))
    return false;
  return tpcbox_contained(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_topo
 * @brief Return @p true if two TPCBox values overlap
 */
bool
tpcbox_overlaps(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  return stbox_overlaps((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_topo
 * @brief Return @p true if two TPCBox values overlap
 * @csqlfn #Overlaps_tpcbox_tpcbox()
 */
bool
overlaps_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_common_dimension(box1->flags, box2->flags))
    return false;
  return tpcbox_overlaps(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_topo
 * @brief Return @p true if two TPCBox values are equal in the common
 * dimensions
 */
bool
tpcbox_same(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  return stbox_same((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_topo
 * @brief Return @p true if two TPCBox values are equal in the common
 * dimensions
 * @csqlfn #Same_tpcbox_tpcbox()
 */
bool
same_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_common_dimension(box1->flags, box2->flags))
    return false;
  return tpcbox_same(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_topo
 * @brief Return @p true if two TPCBox values touch but do not overlap
 */
bool
tpcbox_adjacent(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  return stbox_adjacent((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_topo
 * @brief Return @p true if two TPCBox values touch but do not overlap
 * @csqlfn #Adjacent_tpcbox_tpcbox()
 */
bool
adjacent_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_common_dimension(box1->flags, box2->flags))
    return false;
  return tpcbox_adjacent(box1, box2);
}


/*****************************************************************************
 * Comparison
 *
 * Total order: pcid, then srid, then flags, then period, then spatial
 * bounds in XYZ-min / XYZ-max order. Deterministic; usable by B-tree.
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_box_comp
 * @brief Total-order comparator for TPCBox.
 * @details Order: pcid, srid, flags, period, then spatial bounds.
 *   Deterministic; suitable for B-tree.
 * @return -1, 0, or 1
 * @return On error return @p INT_MAX
 * @csqlfn #Tpcbox_cmp()
 */
int
tpcbox_cmp(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, INT_MAX); VALIDATE_NOT_NULL(box2, INT_MAX);

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

/**
 * @ingroup meos_pointcloud_box_comp
 * @brief Return @p true if two TPCBox values are strictly equal.
 * @csqlfn #Tpcbox_eq()
 */
bool tpcbox_eq(const TPCBox *box1, const TPCBox *box2)
{
  return tpcbox_cmp(box1, box2) == 0;
}

/**
 * @ingroup meos_pointcloud_box_comp
 * @brief Return @p true if two TPCBox values differ.
 * @csqlfn #Tpcbox_ne()
 */
bool tpcbox_ne(const TPCBox *box1, const TPCBox *box2)
{
  return tpcbox_cmp(box1, box2) != 0;
}

/**
 * @ingroup meos_pointcloud_box_comp
 * @brief Return @p true if box1 strictly precedes box2 in total order.
 * @csqlfn #Tpcbox_lt()
 */
bool tpcbox_lt(const TPCBox *box1, const TPCBox *box2)
{
  return tpcbox_cmp(box1, box2) <  0;
}

/**
 * @ingroup meos_pointcloud_box_comp
 * @brief Return @p true if box1 precedes or equals box2 in total order.
 * @csqlfn #Tpcbox_le()
 */
bool tpcbox_le(const TPCBox *box1, const TPCBox *box2)
{
  return tpcbox_cmp(box1, box2) <= 0;
}

/**
 * @ingroup meos_pointcloud_box_comp
 * @brief Return @p true if box1 strictly follows box2 in total order.
 * @csqlfn #Tpcbox_gt()
 */
bool tpcbox_gt(const TPCBox *box1, const TPCBox *box2)
{
  return tpcbox_cmp(box1, box2) >  0;
}

/**
 * @ingroup meos_pointcloud_box_comp
 * @brief Return @p true if box1 follows or equals box2 in total order.
 * @csqlfn #Tpcbox_ge()
 */
bool tpcbox_ge(const TPCBox *box1, const TPCBox *box2)
{
  return tpcbox_cmp(box1, box2) >= 0;
}

/*****************************************************************************
 * Position functions
 *****************************************************************************/

/* X axis */

/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly left of box2 (X-axis).
 * @details Returns @p false if either box lacks the X dimension.
 */
bool
tpcbox_left(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_X(box1->flags));
  assert(MEOS_FLAGS_GET_X(box2->flags));
  return stbox_left((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly left of box2 (X-axis).
 * @details Returns @p false if either box lacks the X dimension.
 * @csqlfn #Left_tpcbox_tpcbox()
 */
bool
left_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_X(T_TPCBOX, box1->flags) ||
      ! ensure_has_X(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_left(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend to the right of box2.
 */
bool
tpcbox_overleft(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_X(box1->flags));
  assert(MEOS_FLAGS_GET_X(box2->flags));
  return stbox_overleft((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend to the right of box2.
 * @csqlfn #Overleft_tpcbox_tpcbox()
 */
bool
overleft_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_X(T_TPCBOX, box1->flags) ||
      ! ensure_has_X(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_overleft(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly right of box2 (X-axis).
 */
bool
tpcbox_right(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_X(box1->flags));
  assert(MEOS_FLAGS_GET_X(box2->flags));
  return stbox_right((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly right of box2 (X-axis).
 * @csqlfn #Right_tpcbox_tpcbox()
 */
bool
right_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_X(T_TPCBOX, box1->flags) ||
      ! ensure_has_X(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_right(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend to the left of box2.
 */
bool
tpcbox_overright(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_X(box1->flags));
  assert(MEOS_FLAGS_GET_X(box2->flags));
  return stbox_overright((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend to the left of box2.
 * @csqlfn #Overright_tpcbox_tpcbox()
 */
bool
overright_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_X(T_TPCBOX, box1->flags) ||
      ! ensure_has_X(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_overright(box1, box2);
}


/* Y axis */

/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly below box2 (Y-axis).
 */
bool
tpcbox_below(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_X(box1->flags));
  assert(MEOS_FLAGS_GET_X(box2->flags));
  return stbox_below((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly below box2 (Y-axis).
 * @csqlfn #Below_tpcbox_tpcbox()
 */
bool
below_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_X(T_TPCBOX, box1->flags) ||
      ! ensure_has_X(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_below(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend above box2.
 */
bool
tpcbox_overbelow(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_X(box1->flags));
  assert(MEOS_FLAGS_GET_X(box2->flags));
  return stbox_overbelow((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend above box2.
 * @csqlfn #Overbelow_tpcbox_tpcbox()
 */
bool
overbelow_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_X(T_TPCBOX, box1->flags) ||
      ! ensure_has_X(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_overbelow(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly above box2 (Y-axis).
 */
bool
tpcbox_above(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_X(box1->flags));
  assert(MEOS_FLAGS_GET_X(box2->flags));
  return stbox_above((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly above box2 (Y-axis).
 * @csqlfn #Above_tpcbox_tpcbox()
 */
bool
above_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_X(T_TPCBOX, box1->flags) ||
      ! ensure_has_X(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_above(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend below box2.
 */
bool
tpcbox_overabove(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_X(box1->flags));
  assert(MEOS_FLAGS_GET_X(box2->flags));
  return stbox_overabove((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend below box2.
 * @csqlfn #Overabove_tpcbox_tpcbox()
 */
bool
overabove_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_X(T_TPCBOX, box1->flags) ||
      ! ensure_has_X(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_overabove(box1, box2);
}


/* Z axis — front/back only meaningful when both boxes have Z */

/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly in front of box2 (Z-axis).
 * @details Returns @p false if either box lacks a Z dimension.
 */
bool
tpcbox_front(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_Z(box1->flags));
  assert(MEOS_FLAGS_GET_Z(box2->flags));
  return stbox_front((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly in front of box2 (Z-axis).
 * @details Returns @p false if either box lacks a Z dimension.
 * @csqlfn #Front_tpcbox_tpcbox()
 */
bool
front_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_Z(T_TPCBOX, box1->flags) ||
      ! ensure_has_Z(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_front(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend behind box2.
 */
bool
tpcbox_overfront(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_Z(box1->flags));
  assert(MEOS_FLAGS_GET_Z(box2->flags));
  return stbox_overfront((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend behind box2.
 * @csqlfn #Overfront_tpcbox_tpcbox()
 */
bool
overfront_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_Z(T_TPCBOX, box1->flags) ||
      ! ensure_has_Z(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_overfront(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly behind box2 (Z-axis).
 */
bool
tpcbox_back(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_Z(box1->flags));
  assert(MEOS_FLAGS_GET_Z(box2->flags));
  return stbox_back((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly behind box2 (Z-axis).
 * @csqlfn #Back_tpcbox_tpcbox()
 */
bool
back_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_Z(T_TPCBOX, box1->flags) ||
      ! ensure_has_Z(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_back(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend in front of box2.
 */
bool
tpcbox_overback(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_Z(box1->flags));
  assert(MEOS_FLAGS_GET_Z(box2->flags));
  return stbox_overback((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend in front of box2.
 * @csqlfn #Overback_tpcbox_tpcbox()
 */
bool
overback_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_Z(T_TPCBOX, box1->flags) ||
      ! ensure_has_Z(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_overback(box1, box2);
}


/* Time axis — before/after only meaningful when both boxes have T */

/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly before box2 in time.
 * @details Returns @p false if either box lacks a T dimension.
 */
bool
tpcbox_before(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_T(box1->flags));
  assert(MEOS_FLAGS_GET_T(box2->flags));
  return stbox_before((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly before box2 in time.
 * @details Returns @p false if either box lacks a T dimension.
 * @csqlfn #Before_tpcbox_tpcbox()
 */
bool
before_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_T(T_TPCBOX, box1->flags) ||
      ! ensure_has_T(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_before(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend after box2 in time.
 */
bool
tpcbox_overbefore(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_T(box1->flags));
  assert(MEOS_FLAGS_GET_T(box2->flags));
  return stbox_overbefore((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend after box2 in time.
 * @csqlfn #Overbefore_tpcbox_tpcbox()
 */
bool
overbefore_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_T(T_TPCBOX, box1->flags) ||
      ! ensure_has_T(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_overbefore(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly after box2 in time.
 */
bool
tpcbox_after(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_T(box1->flags));
  assert(MEOS_FLAGS_GET_T(box2->flags));
  return stbox_after((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 is strictly after box2 in time.
 * @csqlfn #After_tpcbox_tpcbox()
 */
bool
after_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_T(T_TPCBOX, box1->flags) ||
      ! ensure_has_T(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_after(box1, box2);
}


/**
 * @ingroup meos_internal_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend before box2 in time.
 */
bool
tpcbox_overafter(const TPCBox *box1, const TPCBox *box2)
{
  assert(box1); assert(box2);
  assert(MEOS_FLAGS_GET_T(box1->flags));
  assert(MEOS_FLAGS_GET_T(box2->flags));
  return stbox_overafter((const STBox *) box1, (const STBox *) box2);
}

/**
 * @ingroup meos_pointcloud_box_pos
 * @brief Return @p true if box1 does not extend before box2 in time.
 * @csqlfn #Overafter_tpcbox_tpcbox()
 */
bool
overafter_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2) ||
      ! ensure_has_T(T_TPCBOX, box1->flags) ||
      ! ensure_has_T(T_TPCBOX, box2->flags))
    return false;
  return tpcbox_overafter(box1, box2);
}


/*****************************************************************************/
