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
 * @details These relationships compute the ever/always spatial relationship
 * between the arguments and return a Boolean. These functions may be used for
 * filtering purposes before applying the corresponding spatiotemporal
 * relationship.
 *
 * The following relationships are supported: `econtains`, `acontains`, 
 * `ecovers`, `acovers`, `edisjoint`, `adisjoint`, `eintersects`, 
 * `aintersects`, `etouches`, atouches`,  `edwithin`, and `adwithin`.
 */

#include "cbuffer/tcbuffer_spatialrels.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_spatialrels.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_spatialfuncs.h"

/*****************************************************************************
 * Some GEOS versions cannot handle spatial relationships where one of the
 * arguments is a collection. For example, the calls to the `eIntersects`
 * function below translate to calls to the GEOSIntersects function with the
 * traversed areas and GEOS returns an error when the traversed area is a
 * collection.
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
 * Therefore, we need to iterate and apply the spatial relationship to each
 * element of the collection(s)
 *****************************************************************************/

/**
 * @brief Return 1 if two geometries satisfy a spatial relationship
 * @param[in] gs1,gs2 Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the function
 * @param[in] invert True if the arguments should be inverted
 * @return On error return -1
 * @pre None of the two geometries is a geometry collection
 */
static int
spatialrel_geo_geo_simple(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  Datum param, varfunc func, int numparam, bool invert)
{  /* Call the GEOS function when the geometries are not collections */
  assert(geo_is_unitary(gs1)); assert(geo_is_unitary(gs2));
  Datum geo1 = PointerGetDatum(gs1);
  Datum geo2 = PointerGetDatum(gs2);
  bool res;
  if (numparam == 2)
  {
    datum_func2 func2 = (datum_func2) func;
    res = invert ? func2(geo2, geo1) : func2(geo1, geo2);
  }
  else /* numparam == 3 */
  {
    datum_func3 func3 = (datum_func3) func;
    res = invert ? func3(geo2, geo1, param) : func3(geo1, geo2, param);
  }
  return res ? 1 : 0;
}

/**
 * @brief Return 1 if two geometries satisfy a spatial relationship
 * @details The function iterates for every member of a collection if one or
 * the two geometries are collections
 * @param[in] gs1,gs2 Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the functions
 * @param[in] invert True if the arguments should be inverted
 * @return On error return -1
 */
int
spatialrel_geo_geo(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  Datum param, varfunc func, int numparam, bool invert)
{
  /* Extract the elements of the arguments, if they are collections */
  int count1, count2;
  GSERIALIZED **elems1 = geo_extract_elements(gs1, &count1);
  GSERIALIZED **elems2 = geo_extract_elements(gs2, &count2);
  /* Perform the iterations for the elements in the collections if any */
  int result = 0;
  for (int i = 0; i < count1; i++)
  {
    for (int j = 0; j < count2; j++)
    {
      result = spatialrel_geo_geo_simple(elems1[i], elems2[j], param, func,
        numparam, invert);
      if (result)
        break;
    }
    if (result)
      break;
  }
  /* Clean up and return */
  if (count1 == 1)
    pfree(elems1);
  else
    pfree_array((void *) elems1, count1);
  if (count2 == 1)
    pfree(elems2);
  else
    pfree_array((void *) elems2, count2);
  return result;
}

/*****************************************************************************
 * Generic ever/always spatial relationship functions
 * Functions that verify the relationship with the traversed area of EACH
 * SEGMENT of the temporal circular buffer
 *****************************************************************************/

/**
 * @brief Return 1 if a temporal circular buffer instant and a geometry 
 * ever/always satisfy a spatial relationship
 * @param[in] inst Temporal instant
 * @param[in] gs Geometry
 * @param[in] param Optional parameter
 * @param[in] func Spatial relationship function to be applied
 * @param[in] numparam Number of parameters of the function
 * @param[in] invert True when the arguments of the function must be inverted
 * @note The `ever` parameter is not used since the result is the same for the
 * `ever` and the `always` semantics
 */
int
ea_spatialrel_tcbufferinst_geo(const TInstant *inst, const GSERIALIZED *gs,
  Datum param, varfunc func, int numparam, bool invert)
{
  assert(inst); assert(gs); assert(inst->temptype == T_TCBUFFER);
  const Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(inst));
  GSERIALIZED *trav = cbuffer_to_geom(cb);
  int result = spatialrel_geo_geo(trav, gs, param, func, numparam, invert);
  pfree(trav);
  return result ? 1 : 0;
}

/**
 * @brief Return 1 if a temporal circular buffer sequence with discrete or 
 * step interpolation and a geometry ever/always satisfy a spatial relationship
 * @param[in] seq Temporal sequence
 * @param[in] gs Geometry
 * @param[in] param Optional parameter
 * @param[in] func Spatial relationship function to be applied
 * @param[in] numparam Number of parameters of the function
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] invert True when the arguments of the function must be inverted
 */
int
ea_spatialrel_tcbufferseq_discstep_geo(const TSequence *seq,
  const GSERIALIZED *gs, Datum param, varfunc func, int numparam,
  bool ever, bool invert)
{
  assert(seq); assert(gs); assert(seq->temptype == T_TCBUFFER);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  assert(interp == DISCRETE || interp == STEP);
  bool result;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    const Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(inst));
    GSERIALIZED *trav = cbuffer_to_geom(cb);
    result = spatialrel_geo_geo(trav, gs, param, func, numparam, invert);
    pfree(trav);
    if (result && ever)
      return 1;
    else if (! result && ! ever)
      return 0;
  }
  return ever ? 0 : 1;
}

/**
 * @brief Return 1 if a temporal circular buffer sequence with linear
 * interpolation and a geometry ever/always satisfy a spatial relationship
 * @param[in] seq Temporal sequence
 * @param[in] gs Geometry
 * @param[in] param Optional parameter
 * @param[in] func Spatial relationship function to be applied
 * @param[in] numparam Number of parameters of the function
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] invert True when the arguments of the function must be inverted
 */
int
ea_spatialrel_tcbufferseq_linear_geo(const TSequence *seq,
  const GSERIALIZED *gs, Datum param, varfunc func, int numparam,
  bool ever, bool invert)
{
  assert(seq); assert(gs); assert(seq->temptype == T_TCBUFFER);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
    return ea_spatialrel_tcbufferinst_geo(TSEQUENCE_INST_N(seq, 0), gs,
      param, func, numparam, invert);

  /* General case */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  int result;
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    GSERIALIZED *trav = tcbuffersegm_trav_area(inst1, inst2);
    result = spatialrel_geo_geo(trav, gs, param, func, numparam, invert);
    pfree(trav);
    if (result == 1 && ever)
      return 1;
    else if (result != 1 && ! ever)
      return 0;
  }
  return ever ? 0 : 1;
}

/**
 * @brief Return 1 if a temporal circular buffer sequence and a geometry
 * ever/always satisfy a spatial relationship (dispatch function)
 * @param[in] seq Temporal sequence
 * @param[in] gs Geometry
 * @param[in] param Optional parameter
 * @param[in] func Spatial relationship function to be applied
 * @param[in] numparam Number of parameters of the function
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] invert True when the arguments of the function must be inverted
 */
int
ea_spatialrel_tcbufferseq_geo(const TSequence *seq, const GSERIALIZED *gs,
  Datum param, varfunc func, int numparam, bool ever, bool invert)
{
  assert(seq); assert(gs); assert(seq->temptype == T_TCBUFFER);
  return MEOS_FLAGS_LINEAR_INTERP(seq->flags) ?
    ea_spatialrel_tcbufferseq_linear_geo(seq, gs, param, func, numparam,
      ever, invert) :
    ea_spatialrel_tcbufferseq_discstep_geo(seq, gs, param, func, numparam,
      ever, invert);
}

/**
 * @brief Return 1 if a temporal circular buffer sequence set and a geometry
 * ever/always satisfy a spatial relationship
 * @param[in] ss Temporal sequence set
 * @param[in] gs Geometry
 * @param[in] param Optional parameter
 * @param[in] func Spatial relationship function to be applied
 * @param[in] numparam Number of parameters of the function
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] invert True when the arguments of the function must be inverted
 */
int
ea_spatialrel_tcbufferseqset_geo(const TSequenceSet *ss, const GSERIALIZED *gs,
  Datum param, varfunc func, int numparam, bool ever, bool invert)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return ea_spatialrel_tcbufferseq_geo(TSEQUENCESET_SEQ_N(ss, 0), gs, 
      param, func, numparam, ever, invert);

  int result;
  for (int i = 0; i < ss->count; i++)
  {
    result = ea_spatialrel_tcbufferseq_geo(TSEQUENCESET_SEQ_N(ss, i), gs,
      param, func, numparam, ever, invert);
    if (result == 1 && ever)
      return 1;
    else if (result != 1 && ! ever)
      return 0;
}
  return ever ? 0 : 1;
}

/**
 * @brief Return true if a temporal circular buffer and a geometry ever/always
 * satisfy a spatial relationship
 * @details The function computes the traversed area of each segment and
 * verifies that the traversed area and the geometry satisfy the relationship
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] param Optional parameter
 * @param[in] func Spatial relationship function to be applied
 * @param[in] numparam Number of parameters of the function
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] invert True when the arguments of the function must be inverted
 */
int
ea_spatialrel_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum param, varfunc func, int numparam, bool ever, bool invert)
{
  VALIDATE_TCBUFFER(temp, -1); VALIDATE_NOT_NULL(gs, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  /* Bounding box test */ 
  if (func != (varfunc) &datum_geom_disjoint2d)
  {
    STBox box1, box2;
    tspatial_set_stbox(temp, &box1);
    /* Non-empty geometries have a bounding box */
    geo_set_stbox(gs, &box2);
    if (! overlaps_stbox_stbox(&box1, &box2))
      return 0;
  }

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      /* The result is the same for the `ever` and the `always` semantics */
      return ea_spatialrel_tcbufferinst_geo((TInstant *) temp, gs, param,
        func, numparam, invert);
    case TSEQUENCE:
      return ea_spatialrel_tcbufferseq_geo((TSequence *) temp, gs, param,
        func, numparam, ever, invert);
    default: /* TSEQUENCESET */
      return ea_spatialrel_tcbufferseqset_geo((TSequenceSet *) temp, gs,
        param, func, numparam, ever, invert);
  }
}

/*****************************************************************************/

/**
 * @brief Return true if a temporal circular buffer and a circular buffer
 * ever/always satisfy a spatial relationship
 * @param[in] temp Temporal geo
 * @param[in] cb Circular buffer
 * @param[in] param Optional parameter
 * @param[in] func Spatial relationship function to be applied
 * @param[in] numparam Number of parameters of the function
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] invert True when the arguments of the function must be inverted
 */
int
ea_spatialrel_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  Datum param, varfunc func, int numparam, bool ever, bool invert)
{
  VALIDATE_TCBUFFER(temp, -1); VALIDATE_NOT_NULL(cb, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return -1;
  GSERIALIZED *gs = cbuffer_to_geom(cb);
  int result = ea_spatialrel_tcbuffer_geo(temp, gs, param, func, numparam,
    ever, invert);
  pfree(gs);
  return result;
}
  
/*****************************************************************************
 * Generic spatial relationship
 *****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_rel_ever
 * @brief Return 1 if two temporal circular buffers ever/always satisfy a
 * spatial relationship, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] func Spatial relationship function to be called
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] bbox_test True if a bounding text can be used for filtering
 * @csqlfn #Aintersects_tcbuffer_tcbuffer(), #Ecovers_tcbuffer_tcbuffer(), ...
 */
int
ea_spatialrel_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  datum_func2 func, bool ever, bool bbox_test)
{
  VALIDATE_TCBUFFER(temp1, -1); VALIDATE_TCBUFFER(temp2, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return -1;

  /* Bounding box test */
  if (bbox_test)
  {
    STBox *box1 = tspatial_to_stbox(temp1);
    STBox *box2 = tspatial_to_stbox(temp2);
    if (! overlaps_stbox_stbox(box1, box2))
      return 0;
  }

  return ea_spatialrel_tspatial_tspatial(temp1, temp2, func, ever);
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
 * @csqlfn #Acontains_geo_tcbuffer()
 * @note The function is not supported for the `ever` semantics
 */
int
ea_contains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp,
  bool ever)
{
  /* This function is not provided for the ever semantics */
  assert(! ever);
  return ea_spatialrel_tcbuffer_geo(temp, gs, (Datum) NULL,
      (varfunc) &datum_geom_contains, 2, ever, INVERT);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a geometry always contains a temporal circular buffer,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal circular buffer
 * @note The function tests whether the traversed area is contained in the
 * geometry
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
 * @csqlfn #Econtains_tcbuffer_geo(), #Acontains_tcbuffer_geo()
 */
int
ea_contains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever)
{
  const char p[] = "T********";
  int result = ever ?
    ea_spatialrel_tcbuffer_geo(temp, gs, PointerGetDatum(p),
      (varfunc) &datum_geom_relate_pattern, 3, ever, INVERT_NO) :
    ea_spatialrel_tcbuffer_geo(temp, gs, (Datum) NULL,
      (varfunc) &datum_geom_contains, 2, ever, INVERT_NO);
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
  const char p[] = "T********";
  int result = ever ?
    ea_spatialrel_tcbuffer_cbuffer(temp, cb, PointerGetDatum(p),
      (varfunc) &datum_geom_relate_pattern, 3, ever, INVERT) :
    ea_spatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
      (varfunc) &datum_geom_contains, 2, ever, INVERT);
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
  const char p[] = "T********";
  int result = ever ?
    ea_spatialrel_tcbuffer_cbuffer(temp, cb, PointerGetDatum(p),
      (varfunc) &datum_geom_relate_pattern, 3, ever, INVERT_NO) :
    ea_spatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
      (varfunc) &datum_geom_contains, 2, ever, INVERT_NO);
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

/*****************************************************************************
 * Ever/always covers
 *****************************************************************************/

/**
 * @brief Return 1 if a geometry ever/always covers a temporal circular buffer
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note The function is not supported for the `ever` semantics
 * @csqlfn #Acovers_geo_tcbuffer()
 */
int
ea_covers_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  /* This function is not provided for the ever semantics */
  assert(! ever);
  return ea_spatialrel_tcbuffer_geo(temp, gs, (Datum) NULL,
      (varfunc) &datum_geom_covers, 2, ever, INVERT);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a geometry always covers a temporal circular buffer,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal circular buffer
 * @note The function tests whether the traversed area is covered in the
 * geometry
 * @csqlfn #Acovers_geo_tcbuffer()
 */
inline int
acovers_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_covers_geo_tcbuffer(gs, temp, ALWAYS);
}

/*****************************************************************************/

/**
 * @brief Return 1 if a temporal circular buffer ever/always covers a
 * geometry, 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note The function tests whether the traversed area intersects the interior
 * of the geometry.
 * @csqlfn #Ecovers_tcbuffer_geo(), #Acovers_tcbuffer_geo()
 */
int
ea_covers_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  return ea_spatialrel_tcbuffer_geo(temp, gs, (Datum) NULL, 
    (varfunc) &datum_geom_covers, 2, ever, INVERT_NO);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever covers a geometry,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @note The function tests whether the traversed area is covered in the
 * geometry
 * @csqlfn #Ecovers_tcbuffer_geo()
 */
inline int
ecovers_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_covers_tcbuffer_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer always covers a geometry,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @note The function tests whether the traversed area is covered in the
 * geometry
 * @csqlfn #Acovers_tcbuffer_geo()
 */
inline int
acovers_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_covers_tcbuffer_geo(temp, gs, ALWAYS);
}

/*****************************************************************************/

/**
 * @brief Return 1 if a circular buffer ever covers a temporal circular
 * buffer, 0 if not, and -1 on error
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_covers_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp,
  bool ever)
{
  return ea_spatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) &datum_geom_covers, 2, ever, INVERT);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a circular buffer ever covers a temporal circular
 * buffer, 0 if not, and -1 on error
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Ecovers_cbuffer_tcbuffer()
 */
inline int
ecovers_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return ea_covers_cbuffer_tcbuffer(cb, temp, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a circular buffer always covers a temporal circular
 * buffer, 0 if not, and -1 on error
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Acovers_cbuffer_tcbuffer()
 */
inline int
acovers_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return ea_covers_cbuffer_tcbuffer(cb, temp, ALWAYS);
}

/*****************************************************************************/

/**
 * @brief Return 1 if a temporal circular buffer ever/always covers a
 * circular buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_covers_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  bool ever)
{
  return ea_spatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) &datum_geom_covers, 2, ever, INVERT_NO);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever covers a circular
 * buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Ecovers_tcbuffer_cbuffer()
 */
inline int
ecovers_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return ea_covers_tcbuffer_cbuffer(temp, cb, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer always covers a circular
 * buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Acovers_tcbuffer_cbuffer()
 */
inline int
acovers_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return ea_covers_tcbuffer_cbuffer(temp, cb, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever/always covers another
 * one, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Ecovers_tcbuffer_tcbuffer(), #Acovers_tcbuffer_tcbuffer()
 */
int
ea_covers_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool ever)
{
  return ea_spatialrel_tcbuffer_tcbuffer(temp1, temp2, &datum_cbuffer_covers,
    ever, true);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal circular buffer ever covers another one, 0 if not,
 * and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Ecovers_tcbuffer_tcbuffer()
 */
int
ecovers_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return ea_covers_tcbuffer_tcbuffer(temp1, temp2, EVER);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal circular buffer always covers another one, 0 if not,
 * and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Acovers_tcbuffer_tcbuffer()
 */
int
acovers_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return ea_covers_tcbuffer_tcbuffer(temp1, temp2, EVER);
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
  return ea_spatialrel_tcbuffer_geo(temp, gs, (Datum) NULL, 
    (varfunc) &datum_geom_disjoint2d, 2, ever, INVERT_NO);
}

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
ea_disjoint_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp,
  bool ever)
{
  return ea_disjoint_tcbuffer_geo(temp, gs, ever);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a geometry are ever
 * disjoint, 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Edisjoint_tcbuffer_geo()
 */
int
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
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edisjoint_tcbuffer_cbuffer() #Adisjoint_tcbuffer_cbuffer()
 */
int
ea_disjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  bool ever)
{
  return ea_spatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) &datum_geom_disjoint2d, 2, ever, INVERT_NO);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a circular buffer are ever
 * disjoint, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edisjoint_tcbuffer_cbuffer() #Adisjoint_tcbuffer_cbuffer()
 */
int
ea_disjoint_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp,
  bool ever)
{
  return ea_disjoint_tcbuffer_cbuffer(temp, cb, ever);
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
  return ea_disjoint_tcbuffer_cbuffer(temp, cb, EVER);
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
  return ea_disjoint_tcbuffer_cbuffer(temp, cb, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if the temporal circular buffers are ever/always disjoint,
 * 0 if not, and -1 on error or if the temporal circular buffers do not
 * intersect in time
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edisjoint_tcbuffer_tcbuffer()
 */
int
ea_disjoint_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool ever)
{
  return ea_spatialrel_tcbuffer_tcbuffer(temp1, temp2,
    &datum_cbuffer_disjoint, ever, false);
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
  return ea_disjoint_tcbuffer_tcbuffer(temp1, temp2, EVER);
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
  return ea_disjoint_tcbuffer_tcbuffer(temp1, temp2, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always intersects
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever/always intersects a
 * geometry, 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Eintersects_tcbuffer_geo()
 */
int
ea_intersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever)
{
  return ea_spatialrel_tcbuffer_geo(temp, gs, (Datum) NULL, 
    (varfunc) &datum_geom_intersects2d, 2, ever, INVERT_NO);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a geometry intersect
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Eintersects_tcbuffer_geo()
 */
int
ea_intersects_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp,
  bool ever)
{
  return ea_disjoint_tcbuffer_geo(temp, gs, ever);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a geometry and a temporal circular buffer ever intersect,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Eintersects_tcbuffer_geo()
 */
int
eintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_intersects_tcbuffer_geo(temp, gs, EVER);
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
  return ea_intersects_tcbuffer_geo(temp, gs, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever/always intersects a
 * circular buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Eintersects_tcbuffer_cbuffer()
 */
int
ea_intersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  bool ever)
{
  return ea_spatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) &datum_geom_intersects2d, 2, ever, INVERT_NO);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a circular buffer ever/always intersects a temporal
 * circular buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Eintersects_tcbuffer_cbuffer()
 */
int
ea_intersects_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp,
  bool ever)
{
  return ea_intersects_tcbuffer_cbuffer(temp, cb, ever);
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
  return ea_intersects_tcbuffer_cbuffer(temp, cb, EVER);
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
  return ea_intersects_tcbuffer_cbuffer(temp, cb, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever/always intersects another
 * one, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Eintersects_tcbuffer_tcbuffer()
 */
int
ea_intersects_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool ever)
{
  return ea_spatialrel_tcbuffer_tcbuffer(temp1, temp2,
    &datum_cbuffer_intersects, ever, true);

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
  return ea_intersects_tcbuffer_tcbuffer(temp1, temp2, EVER);
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
  return ea_intersects_tcbuffer_tcbuffer(temp1, temp2, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always touches
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a geometry ever touch,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Etouches_tcbuffer_geo(), #Atouches_tcbuffer_geo()
 */
int
ea_touches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  return ea_spatialrel_tcbuffer_geo(temp, gs, (Datum) NULL, 
    (varfunc) &datum_geom_touches, 2, ever, INVERT_NO);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer and a geometry ever touch, 0
 * if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Etouches_tcbuffer_geo()
 */
int
ea_touches_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_touches_tcbuffer_geo(temp, gs, ever);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever touches a geometry,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Atouches_tcbuffer_geo()
 */
int
etouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_touches_tcbuffer_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer always touches a geometry,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Atouches_tcbuffer_geo()
 */
int
atouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_touches_tcbuffer_geo(temp, gs, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever/always touches a circular
 * buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Etouches_tcbuffer_cbuffer(), #Atouches_tcbuffer_cbuffer()
 */
int
ea_touches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  bool ever)
{
  return ea_spatialrel_tcbuffer_cbuffer(temp, cb, (Datum) NULL,
    (varfunc) &datum_geom_touches, 2, ever, INVERT_NO);
}

/**
 * @ingroup meos_internal_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever/always touches a circular
 * buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Etouches_tcbuffer_cbuffer() #Atouches_tcbuffer_cbuffer()
 */
int
ea_touches_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp, bool ever)
{
  return ea_touches_tcbuffer_cbuffer(temp, cb, ever);
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer always touches a circular
 * buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Atouches_tcbuffer_cbuffer()
 */
inline int
etouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return ea_touches_tcbuffer_cbuffer(temp, cb, EVER); 
}

/**
 * @ingroup meos_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer always touches a circular
 * buffer, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Atouches_tcbuffer_cbuffer()
 */
inline int
atouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return ea_touches_tcbuffer_cbuffer(temp, cb, ALWAYS); 
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_rel_ever
 * @brief Return 1 if a temporal circular buffer ever/always touches another
 * one, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Etouches_tcbuffer_tcbuffer()
 */
int
ea_touches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool ever)
{
  return ea_spatialrel_tcbuffer_tcbuffer(temp1, temp2, &datum_cbuffer_touches,
    ever, true);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal circular buffer ever touches another one, 0 if not,
 * and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Etouches_tcbuffer_tcbuffer()
 */
int
etouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return ea_touches_tcbuffer_tcbuffer(temp1, temp2, EVER);
}


/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal circular buffer always touches another one, 0 if not,
 * and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Atouches_tcbuffer_tcbuffer()
 */
int
atouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return ea_touches_tcbuffer_tcbuffer(temp1, temp2, EVER);
}

/*****************************************************************************
 * Ever/always dwithin
 * The function only accepts points and not arbitrary geometries
 *****************************************************************************/

/**
 * @brief Return 1 if a temporal circular buffer and a geometry are ever/always
 * within the given distance, 0 if not, -1 on error or if the geometry is empty
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] invert True if the arguments should be inverted
 * @csqlfn #Edwithin_tcbuffer_geo(), #Adwithin_tcbuffer_geo()
 */
int
ea_dwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs,
  double dist, bool ever, bool invert)
{
  VALIDATE_TCBUFFER(temp, -1); VALIDATE_NOT_NULL(gs, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;
  return ea_spatialrel_tcbuffer_geo(temp, gs, Float8GetDatum(dist),
    (varfunc) &datum_geom_dwithin2d, 3, ever, invert);
}

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
  return ea_dwithin_tcbuffer_geo(temp, gs, dist, EVER, INVERT_NO);
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
  return ea_dwithin_tcbuffer_geo(temp, gs, dist, ALWAYS, INVERT_NO);
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
  VALIDATE_TCBUFFER(temp, -1); VALIDATE_NOT_NULL(cb, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;
  GSERIALIZED *trav = cbuffer_to_geom(cb);
  int result = ea_spatialrel_tcbuffer_geo(temp, trav, Float8GetDatum(dist),
    (varfunc) &datum_geom_dwithin2d, 3, EVER, INVERT_NO);
  pfree(trav);
  return result;
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
  VALIDATE_TCBUFFER(temp, -1); VALIDATE_NOT_NULL(cb, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;
  GSERIALIZED *trav = cbuffer_to_geom(cb);
  GSERIALIZED *buffer = geom_buffer(trav, dist, "");
  int result = ea_spatialrel_tcbuffer_geo(temp, buffer, (Datum) NULL,
    (varfunc) &datum_geom_covers, 2, ALWAYS, INVERT_NO);
  pfree(trav); pfree(buffer);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_spatial_rel_ever
 * @brief Return 1 if two temporal circular buffers are ever/always within a
 * distance, 0 if not, -1 on error or if the temporal circular buffers do not
 * intersect on time
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edwithin_tcbuffer_tcbuffer(), #Adwithin_tcbuffer_tcbuffer()
 */
int 
ea_dwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  double dist, bool ever)
{
  VALIDATE_TCBUFFER(temp1, -1); VALIDATE_TCBUFFER(temp2, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_cbuffer_dwithin;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Float8GetDatum(dist);
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.ever = ever;
  lfinfo.tpfn_temp = &tcbuffersegm_dwithin_turnpt;
  return eafunc_temporal_temporal(temp1, temp2, &lfinfo);
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
