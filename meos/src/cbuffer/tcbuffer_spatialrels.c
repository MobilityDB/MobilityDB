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
 * @brief Ever/always spatial relationships for temporal circular buffers
 *
 * These relationships compute the ever/always spatial relationship between the
 * arguments and return a Boolean. These functions may be used for filtering
 * purposes before applying the corresponding temporal spatial relationship.
 *
 * The following relationships are supported: contains, disjoint, intersects, 
 * touches, and dwithin.
 */

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/lifting.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_spatialrels.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer_spatialfuncs.h"

/*****************************************************************************
 * Generic ever/always spatial relationship functions
 *****************************************************************************/

/**
 * @brief Generic spatial relationship for the traversed area of a temporal
 * circular buffer and a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the functions
 * @param[in] invert True if the arguments should be inverted
 * @return On error return -1
 * @note GEOS cannot handle collection of trapezoids when computing spatial
 * relationships. For example, the calls to the eIntersects function below
 * translate to calls to the GEOSIntersects function with the traversed areas.
 * @code
  SELECT eIntersects(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer
    '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02,
      Cbuffer(Point(1 1),0.5)@2000-01-03]');
  ERROR:  GEOS returned error
  SELECT eIntersects(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer
    '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02]');
  -- t
  SELECT eIntersects(cbuffer 'Cbuffer(Point(1 1),0.5)', tcbuffer 
    '[Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
  -- t
 * @endcode
 * Therefore, after computing the traversed area of the sequence we need to 
 * iterate for each trapezoid in the collection.
 */
int
spatialrel_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum param, varfunc func, int numparam, bool invert)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  assert(numparam == 2 || numparam == 3);
  Datum geo = PointerGetDatum(gs);
  GSERIALIZED *trav = tcbuffer_traversed_area(temp);
  Datum dtrav, result;
  
  /* Call the GEOS function if the traversed area is not a collection */
  if (gserialized_get_type(trav) != COLLECTIONTYPE)
  {
    dtrav = PointerGetDatum(trav);
    if (numparam == 2)
    {
      datum_func2 func2 = (datum_func2) func;
      result = invert ? func2(geo, dtrav) : func2(dtrav, geo);
    }
    else /* numparam == 3 */
    {
      datum_func3 func3 = (datum_func3) func;
      result = invert ? func3(geo, dtrav, param) : func3(dtrav, geo, param);
    }
    pfree(DatumGetPointer(dtrav));
    return result ? 1 : 0;
  }

  /* Call the GEOS function for each trapezoid in the collection */
  LWCOLLECTION *coll = lwgeom_as_lwcollection(lwgeom_from_gserialized(trav));
  for (int i = 0; i < coll->ngeoms; i++)
  {
    const LWGEOM *elem = lwcollection_getsubgeom((LWCOLLECTION *) coll, i);
    dtrav = PointerGetDatum(geo_serialize(elem));
    if (numparam == 2)
    {
      datum_func2 func2 = (datum_func2) func;
      result = invert ? func2(geo, dtrav) : func2(dtrav, geo);
    }
    else /* numparam == 3 */
    {
      datum_func3 func3 = (datum_func3) func;
      result = invert ? func3(geo, dtrav, param) : func3(dtrav, geo, param);
    }
    /* We cannot lwgeom_free((LWGEOM *) coll); */
    if (result)
      return result ? 1 : 0;
  }
  pfree(trav);
  return result ? 1 : 0;
}

/**
 * @brief Generic spatial relationship for the traversed area of a temporal
 * circular buffer and a circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the functions
 * @param[in] invert True if the arguments should be inverted
 * @return On error return -1
 */
static int
spatialrel_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  Datum param, varfunc func, int numparam, bool invert)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return -1;

  GSERIALIZED *trav = cbuffer_geom(cb);
  int result = spatialrel_tcbuffer_geo(temp, trav, param, func, numparam,
    invert);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever/always contains
 *****************************************************************************/

/**
 * @brief Return 1 if a geometry ever/always contains a temporal circular
 * buffer, 0 if not, and -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note The function tests whether the traversed area intersects the interior
 * of the geometry. Please refer to the documentation of the ST_Contains and
 * ST_Relate functions
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Econtains_geo_tcbuffer(), #Acontains_geo_tcbuffer()
 */
int
ea_contains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp,
  bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;
  char p[10] = "T********";
  int result = ever ? 
    spatialrel_tcbuffer_geo(temp, gs, PointerGetDatum(&p),
      (varfunc) &datum_geom_relate_pattern, 3, INVERT) :
    spatialrel_tcbuffer_geo(temp, gs, (Datum) NULL, 
      (varfunc) &datum_geom_contains, 2, INVERT);
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a geometry ever contains a temporal circular buffer,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal circular buffer
 * @note The function tests whether the traversed area is contained in the 
 * geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Econtains_geo_tcbuffer()
 */
inline int
econtains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_contains_geo_tcbuffer(gs, temp, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a geometry always contains a temporal circular buffer,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal circular buffer
 * @note The function tests whether the traversed area is contained in the 
 * geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acontains_geo_tcbuffer()
 */
inline int
acontains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_contains_geo_tcbuffer(gs, temp, ALWAYS);
}

/*****************************************************************************/

/**
 * @brief Return 1 if a temporal circular buffer ever/always contains a
 * geometry, 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note The function tests whether the traversed area intersects the interior
 * of the geometry. Please refer to the documentation of the ST_Contains and
 * ST_Relate functions
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Econtains_tcbuffer_geo(), #Acontains_tcbuffer_geo()
 */
int
ea_contains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;
  char *p  = "T********";
  int result = ever ? 
    spatialrel_tcbuffer_geo(temp, gs, PointerGetDatum(p),
      (varfunc) &datum_geom_relate_pattern, 3, INVERT_NO) :
    spatialrel_tcbuffer_geo(temp, gs, (Datum) NULL, 
      (varfunc) &datum_geom_contains, 2, INVERT_NO);
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever contains a geometry,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @note The function tests whether the traversed area is contained in the 
 * geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Econtains_tcbuffer_geo()
 */
inline int
econtains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_contains_tcbuffer_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer always contains a geometry,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @note The function tests whether the traversed area is contained in the 
 * geometry
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acontains_tcbuffer_geo()
 */
inline int
acontains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_contains_tcbuffer_geo(temp, gs, ALWAYS);
}

/*****************************************************************************/

/**
 * @brief Return 1 if a circular buffer ever contains a temporal circular
 * buffer, 0 if not, and -1 on error
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_contains_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp,
  bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return -1;
  GSERIALIZED *gs = cbuffer_geom(cb);
  int result = ea_contains_geo_tcbuffer(gs, temp, ever);
  pfree(gs);
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a circular buffer ever contains a temporal circular
 * buffer, 0 if not, and -1 on error
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Econtains_cbuffer_tcbuffer()
 */
inline int
econtains_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return ea_contains_cbuffer_tcbuffer(cb, temp, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a circular buffer always contains a temporal circular
 * buffer, 0 if not, and -1 on error
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Acontains_cbuffer_tcbuffer()
 */
inline int
acontains_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return ea_contains_cbuffer_tcbuffer(cb, temp, ALWAYS);
}

/*****************************************************************************/

/**
 * @brief Return 1 if a temporal circular buffer ever/always contains a
 * circular buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_contains_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return -1;
  GSERIALIZED *gs = cbuffer_geom(cb);
  int result = ea_contains_tcbuffer_geo(temp, gs, ever);
  pfree(gs);
  return result;
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever contains a circular
 * buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Econtains_tcbuffer_cbuffer()
 */
inline int
econtains_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return ea_contains_tcbuffer_cbuffer(temp, cb, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer always contains a circular
 * buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Acontains_tcbuffer_cbuffer()
 */
inline int
acontains_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return ea_contains_tcbuffer_cbuffer(temp, cb, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if the first temporal circular buffer ever contains the
 * second one, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Econtains_tcbuffer_tcbuffer()
 */
int
econtains_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return -1;

  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Function %s not implemented", __func__);
  return -1;
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if the first temporal circular buffer always contains the
 * second one, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Acontains_tcbuffer_tcbuffer()
 */
int
acontains_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return -1;

  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Function %s not implemented", __func__);
  return -1;
}

/*****************************************************************************
 * Ever/always disjoint
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a geometry are ever
 * disjoint,0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edisjoint_tcbuffer_geo()
 */
int
ea_disjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;
  int result = ever ?
    spatialrel_tcbuffer_geo(temp, gs, (Datum) NULL,
      (varfunc) &datum_geom_covers, 2, INVERT) :
    eintersects_tcbuffer_geo(temp, gs);
  return INVERT_RESULT(result);
}
/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a geometry are ever 
 * disjoint, 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Edisjoint_tcbuffer_geo()
 */
inline int
edisjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_disjoint_tcbuffer_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a geometry are always 
 * disjoint,0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @note aDisjoint(a, b) is equivalent to NOT eIntersects(a, b)
 * @csqlfn #Adisjoint_tcbuffer_geo()
 */
inline int
adisjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_disjoint_tcbuffer_geo(temp, gs, ALWAYS);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a circular buffer are ever
 * disjoint, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Edisjoint_tcbuffer_cbuffer()
 */
int
edisjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return -1;
  datum_func2 func = &datum_geom_covers;
  int result = spatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) func, 2, INVERT);
  return INVERT_RESULT(result);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a geometry are always 
 * disjoint, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @note aDisjoint(a, b) is equivalent to NOT eIntersects(a, b)
 * @csqlfn #Adisjoint_tcbuffer_geo()
 */
inline int
adisjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return INVERT_RESULT(eintersects_tcbuffer_cbuffer(temp, cb));
}

#if MEOS
/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if the temporal circular buffers are ever disjoint, 0 if not,
 * and -1 on error or if the temporal circular buffers do not intersect in time
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Edisjoint_tcbuffer_tcbuffer()
 */
inline int
edisjoint_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_ne, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if the temporal circular buffers are always disjoint, 0 if
 * not, and -1 on error or if the temporal circular buffers do not intersect
 * in time
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Adisjoint_tcbuffer_tcbuffer()
 */
inline int
adisjoint_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_ne,
    ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always intersects
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a geometry and a temporal circular buffer ever intersect,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Eintersects_tcbuffer_geo()
 */
inline int
eintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return spatialrel_tcbuffer_geo(temp, gs, (Datum) NULL, 
    (varfunc) &datum_geom_intersects2d, 2, INVERT_NO);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a geometry and a temporal circular buffer always 
 * intersect, 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @note aIntersects(tcbuffer, geo) is equivalent to NOT eDisjoint(tcbuffer, geo)
 * @csqlfn #Aintersects_tcbuffer_geo()
 */
inline int
aintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return INVERT_RESULT(edisjoint_tcbuffer_geo(temp, gs));
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a circular buffer and a temporal circular buffer ever 
 * intersect, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Eintersects_tcbuffer_cbuffer()
 */
inline int
eintersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return spatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL, 
    (varfunc) datum_geom_intersects2d, 2,  INVERT_NO);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a circular buffer and a temporal circular buffer always 
 * intersect, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @note aIntersects(tcbuffer, cbuffer) is equivalent to 
 * NOT eDisjoint(tcbuffer, cbuffer)
 * @csqlfn #Aintersects_tcbuffer_cbuffer()
 */
inline int
aintersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return INVERT_RESULT(edisjoint_tcbuffer_cbuffer(temp, cb));
}

#if MEOS
/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if the temporal circular buffers ever intersect, 0 if not,
 * and -1 on error or if the temporal circular buffers do not intersect in time
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Eintersects_tcbuffer_tcbuffer()
 */
inline int
eintersects_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return MEOS_FLAGS_GET_GEODETIC(temp1->flags) ?
    ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_same, EVER) :
    ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_eq, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if the temporal circular buffers always intersect, 0 if not,
 * and -1 on error or if the temporal circular buffers do not intersect in time
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Aintersects_tcbuffer_tcbuffer()
 */
inline int
aintersects_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return MEOS_FLAGS_GET_GEODETIC(temp1->flags) ?
    ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_same, ALWAYS) :
    ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum2_point_eq, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always touches
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a geometry ever touch, 0 
 * if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Etouches_tcbuffer_geo()
 */
int
etouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  /* Bounding box test */
  STBox *box1 = tspatial_stbox(temp);
  STBox *box2 = geo_stbox(gs);
  if (! overlaps_stbox_stbox(box1, box2))
    return 0;

  datum_func2 func = get_intersects_fn_geo(temp->flags, gs->gflags);
  GSERIALIZED *trav = tcbuffer_traversed_area(temp);
  GSERIALIZED *gsbound = geom_boundary(gs);
  bool result = false;
  if (gsbound && ! gserialized_is_empty(gsbound))
    result = func(GserializedPGetDatum(gsbound), GserializedPGetDatum(trav));
  /* TODO */
  // else if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
  // {
    // /* The geometry is a point or a multipoint -> the boundary is empty */
    // GSERIALIZED *tempbound = geom_boundary(trav);
    // if (tempbound)
    // {
      // result = func(GserializedPGetDatum(tempbound), GserializedPGetDatum(gs));
      // pfree(tempbound);
    // }
  // }
  pfree(trav); pfree(gsbound);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a geometry always touch, 
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Atouches_tcbuffer_geo()
 */
int
atouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  /* Bounding box test */
  STBox *box1 = tspatial_stbox(temp);
  STBox *box2 = geo_stbox(gs);
  if (! overlaps_stbox_stbox(box1, box2))
    return 0;

  GSERIALIZED *gsbound = geom_boundary(gs);
  bool result = false;
  if (gsbound && ! gserialized_is_empty(gsbound))
  {
    Temporal *temp1 = (Temporal *) temp; // TODO tcbuffer_minus_geom(temp, gsbound, NULL);
    result = (temp1 == NULL);
    if (temp1)
      pfree(temp1);
  }
  pfree(gsbound);
  return result ? 1 : 0;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a circular buffer ever 
 * touch, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Etouches_tcbuffer_cbuffer()
 */
int
etouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return -1;

  /* Bounding box test */
  STBox *box1 = tspatial_stbox(temp);
  STBox *box2 = cbuffer_stbox(cb);
  if (! overlaps_stbox_stbox(box1, box2))
    return 0;

  GSERIALIZED *trav2 = cbuffer_geom(cb);
  GSERIALIZED *cbbound = geom_boundary(trav2);
  bool result = false;
  if (cbbound && ! gserialized_is_empty(cbbound))
    result = spatialrel_tcbuffer_geo(temp, cbbound, (Datum) NULL, 
      (varfunc) &datum_geom_intersects2d, 2, INVERT_NO);
  /* TODO */
  // else if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
  // {
    // GSERIALIZED *trav1 = tcbuffer_traversed_area(temp);
    // /* The geometry is a point or a multipoint -> the boundary is empty */
    // GSERIALIZED *tempbound = geom_boundary(trav1);
    // if (tempbound)
    // {
      // result = func(GserializedPGetDatum(tempbound), GserializedPGetDatum(cb));
      // pfree(tempbound);
    // }
    // pfree(trav1); 
  // }
  pfree(trav2); pfree(cbbound);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a geometry always touch, 
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Atouches_tcbuffer_cbuffer()
 */
int
atouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return -1;

  /* Bounding box test */
  STBox *box1 = tspatial_stbox(temp);
  STBox *box2 = cbuffer_stbox(cb);
  if (! overlaps_stbox_stbox(box1, box2))
    return 0;

  GSERIALIZED *trav = cbuffer_geom(cb);
  GSERIALIZED *bound = geom_boundary(trav);
  bool result = false;
  if (bound && ! gserialized_is_empty(bound))
  {
    // TODO tcbuffer_minus_geom(temp, bound, NULL);
    Temporal *temp1 = (Temporal *) temp; 
    result = (temp1 == NULL);
    if (temp1)
      pfree(temp1);
  }
  pfree(trav); pfree(bound);
  return result ? 1 : 0;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if two temporal circular buffers ever touch each other, 
 * 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Etouches_tcbuffer_tcbuffer()
 */
int
etouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return -1;

  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Function %s not implemented", __func__);
  return -1;
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if two temporal circular buffers always touch each other, 
 * 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Atouches_tcbuffer_tcbuffer()
 */
int
atouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return -1;

  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Function %s not implemented", __func__);
  return -1;
}

/*****************************************************************************
 * Ever/always dwithin
 * The function only accepts points and not arbitrary geometries
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a geometry and a temporal circular buffer are ever within
 * the given distance, 0 if not, -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Edwithin_tcbuffer_geo()
 */
int
edwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;
  return spatialrel_tcbuffer_geo(temp, gs, Float8GetDatum(dist),
    (varfunc) &datum_geom_dwithin2d, 3, INVERT_NO);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a geometry and a temporal circular buffer are always 
 * within a distance, 0 if not, -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Adwithin_tcbuffer_geo()
 */
int
adwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  GSERIALIZED *buffer = geom_buffer(gs, dist, "");
  datum_func2 func = &datum_geom_covers;
  int result = spatialrel_tcbuffer_geo(temp, buffer, (Datum) NULL,
    (varfunc) func, 2, INVERT);
  pfree(buffer);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a geometry and a temporal circular buffer are ever within
 * the given distance, 0 if not, -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] dist Distance
 * @csqlfn #Edwithin_tcbuffer_cbuffer()
 */
int
edwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  double dist)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;
  return spatialrel_tcbuffer_cbuffer(temp, cb, Float8GetDatum(dist),
    (varfunc) &datum_geom_dwithin2d, 3, INVERT_NO);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a geometry and a temporal circular buffer are always
 * within a distance, 0 if not, -1 on error 
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] dist Distance
 * @csqlfn #Adwithin_tcbuffer_cbuffer()
 */
int
adwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  double dist)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  GSERIALIZED *cbufgeo = cbuffer_geom(cb);
  GSERIALIZED *buffer = geom_buffer(cbufgeo, dist, "");
  datum_func2 func = &datum_geom_covers;
  int result = spatialrel_tcbuffer_geo(temp, buffer, (Datum) NULL,
    (varfunc) func, 2, INVERT);
  pfree(buffer);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return true if the temporal circular buffers are ever within a
 * distance
 * @param[in] inst1,inst2 Temporal circular buffers
 * @param[in] dist Distance
 * @pre The temporal circular buffers are synchronized
 */
static bool
ea_dwithin_tcbufferinst_tcbufferinst(const TInstant *inst1, 
  const TInstant *inst2, double dist)
{
  assert(inst1); assert(inst2);
  /* Result is the same for both EVER and ALWAYS */
  const Cbuffer *cb1 = DatumGetCbufferP(tinstant_value_p(inst1));
  const GSERIALIZED *gs1 = cbuffer_geom(cb1);
  const Cbuffer *cb2 = DatumGetCbufferP(tinstant_value_p(inst2));
  const GSERIALIZED *gs2 = cbuffer_geom(cb2);
  return DatumGetBool(datum_geom_dwithin2d(PointerGetDatum(gs1),
    PointerGetDatum(gs2), Float8GetDatum(dist)));
}

/**
 * @brief Return true if two temporal circular buffers are ever within a
 * distance
 * @param[in] seq1,seq2 Temporal circular buffers
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal circular buffers are synchronized
 */
static bool
ea_dwithin_tcbufferseq_tcbufferseq_discstep(const TSequence *seq1,
  const TSequence *seq2, double dist, bool ever)
{
  assert(seq1); assert(seq2);
  bool ret_loop = ever ? true : false;
  for (int i = 0; i < seq1->count; i++)
  {
    bool res = ea_dwithin_tcbufferinst_tcbufferinst(TSEQUENCE_INST_N(seq1, i),
      TSEQUENCE_INST_N(seq2, i), dist);
    if ((ever && res) || (! ever && ! res))
      return ret_loop;
  }
  return ! ret_loop;
}

/**
 * @brief Return the timestamps at which EITHER the segments of the two
 * temporal points OR a segment of a temporal point and a point are within a
 * distance
 * @param[in] sv1,ev1 Points defining the first segment
 * @param[in] sv2,ev2 Points defining the second segment
 * @param[in] lower,upper Timestamps associated to the segments
 * @param[in] dist Distance
 * @param[out] t1,t2 Resulting timestamps
 * @return Number of timestamps in the result, between 0 and 2. In the case
 * of a single result both t1 and t2 are set to the unique timestamp
 */
int
tdwithin_tcbuffersegm_tcbuffersegm(Datum sv1 __attribute__((unused)), 
  Datum ev1 __attribute__((unused)), Datum sv2 __attribute__((unused)), 
  Datum ev2 __attribute__((unused)), TimestampTz lower __attribute__((unused)), 
  TimestampTz upper __attribute__((unused)), 
  double dist __attribute__((unused)), TimestampTz *t1 __attribute__((unused)), 
  TimestampTz *t2 __attribute__((unused)))
{
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Function %s not implemented", __func__);
  return 0;
}

/**
 * @brief Return true if two temporal circular buffers are ever within a
 * distance
 * @param[in] seq1,seq2 Temporal circular buffers
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal circular buffers are synchronized
 */
static bool
ea_dwithin_tcbufferseq_tcbufferseq_cont(const TSequence *seq1,
  const TSequence *seq2, double dist, bool ever)
{
  assert(seq1); assert(seq2);

  const TInstant *start1, *start2;
  if (seq1->count == 1)
  {
    start1 = TSEQUENCE_INST_N(seq1, 0);
    start2 = TSEQUENCE_INST_N(seq2, 0);
    return ea_dwithin_tcbufferinst_tcbufferinst(start1, start2, dist);
  }

  start1 = TSEQUENCE_INST_N(seq1, 0);
  start2 = TSEQUENCE_INST_N(seq2, 0);
  const Cbuffer *scb1 = DatumGetCbufferP(tinstant_value_p(start1));
  const Cbuffer *scb2 = DatumGetCbufferP(tinstant_value_p(start2));
  GSERIALIZED *sgs1 = cbuffer_geom(scb1);
  GSERIALIZED *sgs2 = cbuffer_geom(scb2);
  Datum sv1 = PointerGetDatum(sgs1);
  Datum sv2 = PointerGetDatum(sgs2);

  bool linear1 = MEOS_FLAGS_LINEAR_INTERP(seq1->flags);
  bool linear2 = MEOS_FLAGS_LINEAR_INTERP(seq2->flags);
  TimestampTz lower = start1->t;
  bool lower_inc = seq1->period.lower_inc;
  bool ret_loop = ever ? true : false;
  for (int i = 1; i < seq1->count; i++)
  {
    const TInstant *end1 = TSEQUENCE_INST_N(seq1, i);
    const TInstant *end2 = TSEQUENCE_INST_N(seq2, i);
    const Cbuffer *ecb1 = DatumGetCbufferP(tinstant_value_p(end1));
    const Cbuffer *ecb2 = DatumGetCbufferP(tinstant_value_p(end2));
    GSERIALIZED *egs1 = cbuffer_geom(ecb1);
    GSERIALIZED *egs2 = cbuffer_geom(ecb2);
    Datum ev1 = PointerGetDatum(egs1);
    Datum ev2 = PointerGetDatum(egs2);
    TimestampTz upper = end1->t;
    bool upper_inc = (i == seq1->count - 1) ? seq1->period.upper_inc : false;

    /* Both segments are constant */
    if (geo_equals(sgs1, egs1) && geo_equals(sgs2, egs2))
    {
      bool res = geom_dwithin2d(sgs1, sgs2, dist);
      if ((ever && res) || (! ever && ! res))
        return ret_loop;
    }

    /* General case */
    TimestampTz t1, t2;
    Datum sev1 = linear1 ? ev1 : sv1;
    Datum sev2 = linear2 ? ev2 : sv2;
    /* Find the instants t1 and t2 (if any) during which the dwithin function
     * is true */
    int solutions = tdwithin_tcbuffersegm_tcbuffersegm(sv1, sev1, sv2, sev2,
      lower, upper, dist, &t1, &t2);
    bool res = (solutions == 2 ||
      (solutions == 1 && ((t1 != lower || lower_inc) &&
        (t1 != upper || upper_inc))));
    if ((ever && res) || (! ever && ! res))
      return ret_loop;

    sv1 = ev1;
    sv2 = ev2;
    lower = upper;
    lower_inc = true;
  }
  return ! ret_loop;
}

/**
 * @brief Return true if two temporal circular buffers are ever within a
 * distance
 * @param[in] ss1,ss2 Temporal circular buffers
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal circular buffers are synchronized
 */
static bool
ea_dwithin_tcbufferseqset_tcbufferseqset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, double dist, bool ever)
{
  assert(ss1); assert(ss2);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(ss1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(ss2->flags);
  bool ret_loop = ever ? true : false;
  for (int i = 0; i < ss1->count; i++)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, i);
    const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, i);
    bool res = linear ?
      ea_dwithin_tcbufferseq_tcbufferseq_cont(seq1, seq2, dist, ever) :
      ea_dwithin_tcbufferseq_tcbufferseq_discstep(seq1, seq2, dist, ever);
    if ((ever && res) || (! ever && ! res))
      return ret_loop;
  }
  return ! ret_loop;
}

/*****************************************************************************/

/**
 * @brief Return 1 if two temporal circular buffers are ever within a distance,
 * 0 if not, -1 if the temporal circular buffers do not intersect on time
 * @pre The temporal circular buffers are synchronized
 */
int
ea_dwithin_tcbuffer_tcbuffer_sync(const Temporal *sync1, const Temporal *sync2,
  double dist, bool ever)
{
  assert(sync1); assert(sync2);
  assert(temptype_subtype(sync1->subtype));
  switch (sync1->subtype)
  {
    case TINSTANT:
      return ea_dwithin_tcbufferinst_tcbufferinst((TInstant *) sync1,
        (TInstant *) sync2, dist);
    case TSEQUENCE:
      return MEOS_FLAGS_LINEAR_INTERP(sync1->flags) ||
          MEOS_FLAGS_LINEAR_INTERP(sync2->flags) ?
        ea_dwithin_tcbufferseq_tcbufferseq_cont((TSequence *) sync1,
          (TSequence *) sync2, dist, ever) :
        ea_dwithin_tcbufferseq_tcbufferseq_discstep((TSequence *) sync1,
          (TSequence *) sync2, dist, ever);
    default: /* TSEQUENCESET */
      return ea_dwithin_tcbufferseqset_tcbufferseqset((TSequenceSet *) sync1,
        (TSequenceSet *) sync2, dist, ever);
  }
}

/**
 * @ingroup meos_internal_cbuffer_spatial_rel_ever
 * @brief Return 1 if two temporal circular buffers are ever within a distance,
 * 0 if not, -1 on error or if the temporal circular buffers do not intersect
 * on time
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edwithin_tcbuffer_tcbuffer()
 */
int
ea_dwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  double dist, bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  Temporal *sync1, *sync2;
  /* Return NULL if the temporal circular buffers do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return -1;

  bool result = ea_dwithin_tcbuffer_tcbuffer_sync(sync1, sync2, dist, ever);
  pfree(sync1); pfree(sync2);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if two temporal circular buffers are ever within a distance,
 * 0 if not, -1 on error or if the temporal circular buffers do not intersect
 * on time
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] dist Distance
 * @csqlfn #Edwithin_tcbuffer_tcbuffer()
 */
inline int
edwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  return ea_dwithin_tcbuffer_tcbuffer(temp1, temp2, dist, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if two temporal circular buffers are always within a 
 * distance, 0 if not, -1 on error or if the temporal circular buffers do not
 * intersect on time
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] dist Distance
 * @csqlfn #Adwithin_tcbuffer_tcbuffer()
 */
inline int
adwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  return ea_dwithin_tcbuffer_tcbuffer(temp1, temp2, dist, ALWAYS);
}

/*****************************************************************************/
