/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Functions for rounding the float components of types
 */

#include "general/type_round.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h> /* For get_float8_infinity() */
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/set.h"
#include "general/span.h"
#include "general/spanset.h"
#include "general/tbox.h"
#include "point/stbox.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Round functions called by the other functions
 *****************************************************************************/

/**
 * @brief Return a float number rounded to a given number of decimal places
 */
double
float_round(double d, int maxdd)
{
  assert(maxdd >= 0);
  double inf = get_float8_infinity();
  double result = d;
  if (d != -1 * inf && d != inf)
  {
    if (maxdd == 0)
      result = round(d);
    else
    {
      double power10 = pow(10.0, maxdd);
      result = round(d * power10) / power10;
    }
  }
  return result;
}

/**
 * @brief Return a float number rounded to a given number of decimal places
 */
Datum
datum_round_float(Datum value, Datum size)
{
  return Float8GetDatum(float_round(DatumGetFloat8(value),
    DatumGetInt32(size)));
}

/*****************************************************************************
 * Set
 *****************************************************************************/

/**
 * @ingroup meos_setspan_transf
 * @brief Return a set with the precision of the values set to a number of
 * decimal places
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @param[in] func Function applied for rounding the elements of the set
 */
static Set *
set_round(const Set *s, int maxdd, datum_func2 func)
{
  assert(s); assert(maxdd >= 0);
  Datum *values = palloc(sizeof(Datum) * s->count);
  Datum size = Int32GetDatum(maxdd);
  for (int i = 0; i < s->count; i++)
    values[i] = func(SET_VAL_N(s, i), size);
  return set_make_free(values, s->count, s->basetype, ORDERED_NO);
}

/**
 * @ingroup meos_internal_setspan_transf
 * @brief Return a float set with the precision of the values set to a number
 * of decimal places
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Floatset_round()
 */
Set *
floatset_rnd(const Set *s, int maxdd)
{
  assert(s); assert(maxdd >= 0); assert(s->settype == T_FLOATSET);
  return set_round(s, maxdd, &datum_round_float);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float set with the precision of the values set to a number
 * of decimal places
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Floatset_round()
 */
Set *
floatset_round(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_negative(maxdd) ||
      ! ensure_set_isof_type(s, T_FLOATSET))
    return NULL;
  return set_round(s, maxdd, &datum_round_float);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a geo set with the precision of the coordinates set to a
 * number of decimal places
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Geoset_round()
 */
Set *
geoset_round(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_negative(maxdd) ||
      ! ensure_geoset_type(s->settype))
    return NULL;
  return set_round(s, maxdd, &datum_round_geo);
}

#if NPOINT
/**
 * @brief Return a network point set with the precision of the positions set
 * to a number of decimal places
 * @csqlfn #Npointset_round()
 */
Set *
npointset_round(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_negative(maxdd) ||
      ! ensure_set_isof_type(s, T_NPOINTSET))
    return NULL;
  return set_round(s, maxdd, &datum_npoint_round);
}
#endif /* NPOINT */

/*****************************************************************************
 * Span
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_transf
 * @brief Return the last argument initialized to a float span with the
 * precision set to a number of decimal places
 * @param[in] s Span
 * @param[in] maxdd Maximum number of decimal digits
 * @param[out] result Result span
 */
void
floatspan_rnd_set(const Span *s, int maxdd, Span *result)
{
  assert(s); assert(s->spantype == T_FLOATSPAN); assert(result);
  /* Set precision of bounds */
  double lower = float_round(DatumGetFloat8(s->lower), maxdd);
  double upper = float_round(DatumGetFloat8(s->upper), maxdd);
  /* Fix the bounds */
  bool lower_inc, upper_inc;
  if (float8_eq(lower, upper))
  {
    lower_inc = upper_inc = true;
  }
  else
  {
    lower_inc = s->lower_inc; upper_inc = s->upper_inc;
  }
  /* Set resulting span */
  span_set(Float8GetDatum(lower), Float8GetDatum(upper), lower_inc, upper_inc,
    s->basetype, s->spantype, result);
  return;
}

/**
 * @ingroup meos_internal_setspan_transf
 * @brief Return a float span with the precision of the bounds set to a
 * number of decimal places
 * @param[in] s Span
 * @param[in] maxdd Maximum number of decimal digits
 * @return On error return @p NULL
 */
Span *
floatspan_rnd(const Span *s, int maxdd)
{
  assert(s); assert(maxdd >=0); assert(s->spantype == T_FLOATSPAN);
  Span *result = palloc(sizeof(Span));
  floatspan_rnd_set(s, maxdd, result);
  return result;
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span with the precision of the bounds set to a
 * number of decimal places
 * @param[in] s Span
 * @param[in] maxdd Maximum number of decimal digits
 * @return On error return @p NULL
 */
Span *
floatspan_round(const Span *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_negative(maxdd) ||
      ! ensure_span_isof_type(s, T_FLOATSPAN))
    return NULL;
  return floatspan_rnd(s, maxdd);
}

/*****************************************************************************
 * SpanSet
 *****************************************************************************/

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span set with the precision of the spans set to a
 * number of decimal places
 * @param[in] ss Span set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Floatspanset_round()
 */
SpanSet *
floatspanset_rnd(const SpanSet *ss, int maxdd)
{
  assert(ss); assert(maxdd >= 0); assert(ss->spansettype == T_FLOATSPANSET);
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    floatspan_rnd_set(SPANSET_SP_N(ss, i), maxdd, &spans[i]);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDERED);
}


/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span set with the precision of the spans set to a
 * number of decimal places
 * @param[in] ss Span set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Floatspanset_round()
 */
SpanSet *
floatspanset_round(const SpanSet *ss, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_negative(maxdd) ||
      ! ensure_spanset_isof_type(ss, T_FLOATSPANSET))
    return NULL;
  return floatspanset_rnd(ss, maxdd);
}

/*****************************************************************************
 * Tbox
 *****************************************************************************/

/**
 * @ingroup meos_box_transf
 * @brief Return a temporal box with the precision of the value span set to a
 * number of decimal places
 * @param[in] box Temporal box
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tbox_round()
 */
TBox *
tbox_round(const TBox *box, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_tbox(box) ||
      ! ensure_span_isof_basetype(&box->span, T_FLOAT8) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  TBox *result = tbox_cp(box);
  floatspan_rnd_set(&box->span, maxdd, &result->span);
  return result;
}

/*****************************************************************************
 * STbox
 *****************************************************************************/

/**
 * @ingroup meos_box_transf
 * @brief Return a spatiotemporal box with the precision of the coordinates set
 * to a number of decimal places
 * @param[in] box Spatiotemporal box
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Stbox_round()
 */
STBox *
stbox_round(const STBox *box, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_stbox(box) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  STBox *result = stbox_cp(box);
  result->xmin = float_round(box->xmin, maxdd);
  result->xmax = float_round(box->xmax, maxdd);
  result->ymin = float_round(box->ymin, maxdd);
  result->ymax = float_round(box->ymax, maxdd);
  if (MEOS_FLAGS_GET_Z(box->flags) || MEOS_FLAGS_GET_GEODETIC(box->flags))
  {
    result->zmin = float_round(box->zmin, maxdd);
    result->zmax = float_round(box->zmax, maxdd);
  }
  return result;
}

/*****************************************************************************
 * TFloat
 *****************************************************************************/

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal float with the precision of the values set to a
 * number of decimal places
 * @param[in] temp Temporal float
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tfloat_round()
 */
Temporal *
tfloat_round(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_round_float;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(maxdd);
  lfinfo.args = true;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.argtype[1] = T_INT4;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return an array of temporal floats with the precision of the
 * coordinates set to a number of decimal places
 * @param[in] temparr Array of temporal values
 * @param[in] count Number of values in the input array
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tfloatarr_round()
 */
Temporal **
tfloatarr_round(const Temporal **temparr, int count, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temparr) ||
      /* Ensure that the FIRST element is a temporal float */
      ! ensure_temporal_isof_type(temparr[0], T_TFLOAT) ||
      ! ensure_positive(count) || ! ensure_not_negative(maxdd))
    return NULL;

  Temporal **result = palloc(sizeof(Temporal *) * count);
  for (int i = 0; i < count; i++)
    result[i] = tfloat_round(temparr[i], maxdd);
  return result;
}

/*****************************************************************************
 * Geometry/Geography
 *****************************************************************************/

/**
 * @brief Set the precision of the coordinates of the n-th point in a point
 * array to a number of decimal places
 */
static void
round_point_n(POINTARRAY *points, uint32_t n, int maxdd, bool hasz, bool hasm)
{
  /* N.B. lwpoint->point can be of 2, 3, or 4 dimensions depending on
   * the values of the arguments hasz and hasm !!! */
  POINT4D *pt = (POINT4D *) getPoint_internal(points, n);
  pt->x = float_round(pt->x, maxdd);
  pt->y = float_round(pt->y, maxdd);
  if (hasz && hasm)
  {
    pt->z = float_round(pt->z, maxdd);
    pt->m = float_round(pt->m, maxdd);
  }
  else if (hasz)
    pt->z = float_round(pt->z, maxdd);
  else if (hasm)
    /* The m co ordinate is located at the third double of the point */
    pt->z = float_round(pt->z, maxdd);
  return;
}

/**
 * @brief Return a point with the coordinates set to a number of decimal places
 */
static Datum
round_point(GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == POINTTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWPOINT *point = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
  round_point_n(point->point, 0, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) point);
  pfree(point);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates of a line to a number of
 * decimal places
 */
static void
round_lwline(LWLINE *line, int maxdd, bool hasz, bool hasm)
{
  int npoints = line->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point_n(line->points, i, maxdd, hasz, hasm);
  return;
}

/**
 * @brief Return a line with the coordinates set to a number of decimal places
 */
static Datum
round_linestring(GSERIALIZED *gs,int maxdd)
{
  assert(gserialized_get_type(gs) == LINETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWLINE *line = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
  round_lwline(line, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) line);
  lwfree(line);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates of a triangle to a number of
 * decimal places
 */
static void
round_lwtriangle(LWTRIANGLE *triangle, int maxdd, bool hasz, bool hasm)
{
  int npoints = triangle->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point_n(triangle->points, i, maxdd, hasz, hasm);
  return;
}

/**
 * @brief Return a triangle with the precision of the coordinates set to a
 * number of decimal places
 */
static Datum
round_triangle(GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == TRIANGLETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWTRIANGLE *triangle = lwgeom_as_lwtriangle(lwgeom_from_gserialized(gs));
  round_lwtriangle(triangle, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) triangle);
  lwfree(triangle);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates of a circular string to a number
 * of decimal places
 */
static void
round_lwcircstring(LWCIRCSTRING *circstring, int maxdd, bool hasz,
  bool hasm)
{
  int npoints = circstring->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point_n(circstring->points, i, maxdd, hasz, hasm);
  return;
}

/**
 * @brief Return a circular string with the precision of the coordinates set to
 * a number of decimal places
 */
static Datum
round_circularstring(GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == CIRCSTRINGTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWCIRCSTRING *circstring = lwgeom_as_lwcircstring(lwgeom_from_gserialized(gs));
  round_lwcircstring(circstring, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) circstring);
  lwfree(circstring);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates of a polygon to a number of
 * decimal places
 */
static void
round_lwpoly(LWPOLY *poly, int maxdd, bool hasz, bool hasm)
{
  int nrings = poly->nrings;
  for (int i = 0; i < nrings; i++)
  {
    POINTARRAY *points = poly->rings[i];
    int npoints = points->npoints;
    for (int j = 0; j < npoints; j++)
      round_point_n(points, j, maxdd, hasz, hasm);
  }
  return;
}

/**
 * @brief Reuturn a polygon with the precision of the coordinates set to a
 * number of decimal places
 */
static Datum
round_polygon(GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == POLYGONTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWPOLY *poly = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs));
  round_lwpoly(poly, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) poly);
  lwfree(poly);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates of a multipoint to a number of
 * decimal places
 */
static void
round_lwmpoint(LWMPOINT *mpoint, int maxdd, bool hasz, bool hasm)
{
  int ngeoms = mpoint->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWPOINT *point = mpoint->geoms[i];
    round_point_n(point->point, 0, maxdd, hasz, hasm);
  }
  return;
}

/**
 * @brief Return a multipoint with the precision of the coordinates set to a
 * number of decimal places
 */
static Datum
round_multipoint(GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == MULTIPOINTTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMPOINT *mpoint =  lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  round_lwmpoint(mpoint, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) mpoint);
  lwfree(mpoint);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates of a multilinestring to a
 * number of decimal places
 */
static void
round_lwmline(LWMLINE *mline, int maxdd, bool hasz, bool hasm)
{
  int ngeoms = mline->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWLINE *line = mline->geoms[i];
    int npoints = line->points->npoints;
    for (int j = 0; j < npoints; j++)
      round_point_n(line->points, j, maxdd, hasz, hasm);
  }
  return;
}

/**
 * @brief Return a multilinestring with the precision of the coordinates set to
 * a number of decimal places
 */
static Datum
round_multilinestring(GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == MULTILINETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMLINE *mline = lwgeom_as_lwmline(lwgeom_from_gserialized(gs));
  round_lwmline(mline, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) mline);
  lwfree(mline);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates of a multipolygon to a number of
 * decimal places
 */
static void
round_lwmpoly(LWMPOLY *mpoly, int maxdd, bool hasz, bool hasm)
{
  int ngeoms = mpoly->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWPOLY *poly = mpoly->geoms[i];
    round_lwpoly(poly, maxdd, hasz, hasm);
  }
  return;
}

/**
 * @brief Return a multipolygon with the precision of the coordinates set to a
 * number of decimal places
 */
static Datum
round_multipolygon(GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == MULTIPOLYGONTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMPOLY *mpoly = lwgeom_as_lwmpoly(lwgeom_from_gserialized(gs));
  round_lwmpoly(mpoly, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) mpoly);
  lwfree(mpoly);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates of a geometry collection to a
 * number of decimal places
 */
static Datum
round_geometrycollection(GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == COLLECTIONTYPE);
  LWCOLLECTION *coll = lwgeom_as_lwcollection(lwgeom_from_gserialized(gs));
  int ngeoms = coll->ngeoms;
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *geom = coll->geoms[i];
    if (geom->type == POINTTYPE)
      round_point_n((lwgeom_as_lwpoint(geom))->point, 0, maxdd, hasz, hasm);
    else if (geom->type == LINETYPE)
      round_lwline(lwgeom_as_lwline(geom), maxdd, hasz, hasm);
    else if (geom->type == TRIANGLETYPE)
      round_lwtriangle(lwgeom_as_lwtriangle(geom), maxdd, hasz, hasm);
    else if (geom->type == CIRCSTRINGTYPE)
      round_lwcircstring(lwgeom_as_lwcircstring(geom), maxdd, hasz, hasm);
    else if (geom->type == POLYGONTYPE)
      round_lwpoly(lwgeom_as_lwpoly(geom), maxdd, hasz, hasm);
    else if (geom->type == MULTIPOINTTYPE)
      round_lwmpoint(lwgeom_as_lwmpoint(geom), maxdd, hasz, hasm);
    else if (geom->type == MULTILINETYPE)
      round_lwmline(lwgeom_as_lwmline(geom), maxdd, hasz, hasm);
    else if (geom->type == MULTIPOLYGONTYPE)
      round_lwmpoly(lwgeom_as_lwmpoly(geom), maxdd, hasz, hasm);
    else
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Unsupported geometry type");
      return PointerGetDatum(NULL);
    }
  }
  GSERIALIZED *result = geo_serialize((LWGEOM *) coll);
  lwfree(coll);
  return PointerGetDatum(result);
}

/**
 * @brief Return a geometry with the precision of the coordinates set to a
 * number of decimal places
 * @note Currently not all geometry types are allowed
 */
Datum
datum_round_geo(Datum value, Datum size)
{
  GSERIALIZED *gs = DatumGetGserializedP(value);
  int maxdd = DatumGetInt32(size);
  if (gserialized_is_empty(gs))
    return PointerGetDatum(geo_copy(gs));

  uint32_t type = gserialized_get_type(gs);
  if (type == POINTTYPE)
    return round_point(gs, maxdd);
  if (type == LINETYPE)
    return round_linestring(gs, maxdd);
  if (type == TRIANGLETYPE)
    return round_triangle(gs, maxdd);
  if (type == CIRCSTRINGTYPE)
    return round_circularstring(gs, maxdd);
  if (type == POLYGONTYPE)
    return round_polygon(gs, maxdd);
  if (type == MULTIPOINTTYPE)
    return round_multipoint(gs, maxdd);
  if (type == MULTILINETYPE)
    return round_multilinestring(gs, maxdd);
  if (type == MULTIPOLYGONTYPE)
    return round_multipolygon(gs, maxdd);
  if (type == COLLECTIONTYPE)
    return round_geometrycollection(gs, maxdd);
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Unsupported geometry type");
  return PointerGetDatum(NULL);
}

/*****************************************************************************
 * Temporal Point
 *****************************************************************************/

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal point with the precision of the coordinates set to
 * a number of decimal places
 * @param[in] temp Temporal point
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tpoint_round()
 */
Temporal *
tpoint_round(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_round_geo;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(maxdd);
  lfinfo.restype = temp->temptype;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return an array of temporal points with the precision of the
 * coordinates set to a number of decimal places
 * @param[in] temparr Array of temporal points
 * @param[in] count Number of elements in the array
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tpointarr_round()
 */
Temporal **
tpointarr_round(const Temporal **temparr, int count, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temparr) ||
      /* Ensure that the FIRST element is a temporal point */
      ! ensure_tgeo_type(temparr[0]->temptype) ||
      ! ensure_positive(count) || ! ensure_not_negative(maxdd))
    return NULL;

  Temporal **result = palloc(sizeof(Temporal *) * count);
  for (int i = 0; i < count; i++)
    result[i] = tpoint_round(temparr[i], maxdd);
  return result;
}

/*****************************************************************************
 * Network Point
 *****************************************************************************/

#if NPOINT
/**
 * @brief Return a network point with the precision of the position set to a
 * number of decimal places
 */
Npoint *
npoint_round(const Npoint *np, int maxdd)
{
  /* Set precision of position */
  double pos = float_round(np->pos, maxdd);
  return npoint_make(np->rid, pos);
}

/**
 * @brief Return a network point with the precision of the position set to a
 * number of decimal places
 * @note Funcion used by the lifting infrastructure
 */
Datum
datum_npoint_round(Datum npoint, Datum size)
{
  /* Set precision of position */
  return PointerGetDatum(npoint_round(DatumGetNpointP(npoint),
    DatumGetInt32(size)));
}

/**
 * @brief Return a network segment with the precision of the positions set to a
 * number of decimal places
 */
Nsegment *
nsegment_round(const Nsegment *ns, int maxdd)
{
  /* Set precision of positions */
  double pos1 = float_round(ns->pos1, maxdd);
  double pos2 = float_round(ns->pos2, maxdd);
  return nsegment_make(ns->rid, pos1, pos2);
}
#endif /* NPOINT */

/*****************************************************************************
 * Temporal Network Point
 *****************************************************************************/

#if NPOINT
/**
 * @brief Return a temporal network point with the precision of the fractions
 * set to a number of decimal places
 */
Temporal *
tnpoint_round(const Temporal *temp, Datum size)
{
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_npoint_round;
  lfinfo.numparam = 1;
  lfinfo.param[0] = size;
  lfinfo.restype = temp->temptype;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}
#endif /* NPOINT */

/*****************************************************************************/
