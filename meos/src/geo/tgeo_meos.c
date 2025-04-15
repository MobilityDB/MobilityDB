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
 * @brief Functions for temporal geometries/geographies
 */

#include "geo/tgeo_aggfuncs.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/doublen.h"
#include "general/lifting.h"
#include "general/set.h"
#include "general/skiplist.h"
#include "general/span.h"
#include "general/spanset.h"
#include "general/temporal_aggfuncs.h"
#include "general/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"

/*****************************************************************************
 * Input/output
 *****************************************************************************/

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry point instant from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 */
TInstant *
tgeompointinst_in(const char *str)
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal instant geography point from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 */
TInstant *
tgeogpointinst_in(const char *str)
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tpoint_parse(&str, T_TGEOGPOINT);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
tgeometryinst_in(const char *str)
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tspatial_parse(&str, T_TGEOMETRY);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geography instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
tgeographyinst_in(const char *str)
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tspatial_parse(&str, T_TGEOGRAPHY);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry point sequence from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tgeompointseq_in(const char *str, interpType interp __attribute__((unused)))
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geography point sequence from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tgeogpointseq_in(const char *str, interpType interp __attribute__((unused)))
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tpoint_parse(&str, T_TGEOGPOINT);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry sequence from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tgeometryseq_in(const char *str, interpType interp __attribute__((unused)))
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tspatial_parse(&str, T_TGEOMETRY);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geography sequence from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tgeographyseq_in(const char *str, interpType interp __attribute__((unused)))
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tspatial_parse(&str, T_TGEOGRAPHY);
  if (! temp)
    return NULL;
  assert (temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry point sequence set from its Well-Known
 * Text (WKT) representation
 * @param[in] str String
 */
TSequenceSet *
tgeompointseqset_in(const char *str)
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geography point sequence set from its Well-Known
 * Text (WKT) representation
 * @param[in] str String
 */
TSequenceSet *
tgeogpointseqset_in(const char *str)
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tpoint_parse(&str, T_TGEOGPOINT);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry sequence set from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 */
TSequenceSet *
tgeometryseqset_in(const char *str)
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tspatial_parse(&str, T_TGEOMETRY);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geography sequence set from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 */
TSequenceSet *
tgeographyseqset_in(const char *str)
{
  assert(str);
  /* Call the superclass function */
  Temporal *temp = tpoint_parse(&str, T_TGEOGRAPHY);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}

/*****************************************************************************
 * Intput in MF-JSON format
 *****************************************************************************/

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry point instant from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @csqlfn #Temporal_from_mfjson()
 */
inline TInstant *
tgeompointinst_from_mfjson(json_object *mfjson, int srid)
{
  return tinstant_from_mfjson(mfjson, true, srid, T_TGEOMPOINT);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geography point instant from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @csqlfn #Temporal_from_mfjson()
 */
inline TInstant *
tgeogpointinst_from_mfjson(json_object *mfjson, int srid)
{
  return tinstant_from_mfjson(mfjson, true, srid, T_TGEOGPOINT);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry instant from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @csqlfn #Temporal_from_mfjson()
 */
inline TInstant *
tgeometryinst_from_mfjson(json_object *mfjson, int srid)
{
  return tinstant_from_mfjson(mfjson, true, srid, T_TGEOMETRY);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geography instant from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @csqlfn #Temporal_from_mfjson()
 */
inline TInstant *
tgeographyinst_from_mfjson(json_object *mfjson, int srid)
{
  return tinstant_from_mfjson(mfjson, true, srid, T_TGEOGRAPHY);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry point sequence from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequence *
tgeompointseq_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequence_from_mfjson(mfjson, true, srid, T_TGEOMPOINT, interp);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geography point sequence from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequence *
tgeogpointseq_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequence_from_mfjson(mfjson, true, srid, T_TGEOGPOINT, interp);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry sequence from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequence *
tgeometryseq_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequence_from_mfjson(mfjson, true, srid, T_TGEOMETRY, interp);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geography sequence from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequence *
tgeographyseq_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequence_from_mfjson(mfjson, true, srid, T_TGEOGRAPHY, interp);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry point sequence set from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequenceSet *
tgeompointseqset_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, true, srid, T_TGEOMPOINT, interp);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geography point sequence set from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequenceSet *
tgeogpointseqset_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, true, srid, T_TGEOGPOINT, interp);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geometry sequence set from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequenceSet *
tgeometryseqset_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, true, srid, T_TGEOMETRY, interp);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return a temporal geography sequence set from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequenceSet *
tgeographyseqset_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, true, srid, T_TGEOGRAPHY, interp);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_inout
 * @brief Return a temporal geometry point from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
inline Temporal *
tgeompoint_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TGEOMPOINT);
}

/**
 * @ingroup meos_geo_inout
 * @brief Return a temporal geography point from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
inline Temporal *
tgeogpoint_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TGEOGPOINT);
}
/**
 * @ingroup meos_geo_inout
 * @brief Return a temporal geometry from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
inline Temporal *
tgeometry_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TGEOMETRY);
}

/**
 * @ingroup meos_geo_inout
 * @brief Return a temporal geography from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
inline Temporal *
tgeography_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TGEOGRAPHY);
}

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * @ingroup meos_geo_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal geo
 * @param[in] temp Temporal geo
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tgeo_out(const Temporal *temp, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL);
  if (! ensure_positive(maxdd))
    return NULL;
  return temporal_out(temp, maxdd);
}

/*****************************************************************************
 * Constructors
 *****************************************************************************/

/**
 * @ingroup meos_geo_constructor
 * @brief Return a temporal geometry point discrete sequence from a point
 * and a timestamptz set
 * @param[in] gs Value
 * @param[in] s Set
 */
TSequence *
tpointseq_from_base_tstzset(const GSERIALIZED *gs, const Set *s)
{
  /* Ensure the validity of the arguments */
 VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TSTZSET(s, NULL);
  if (! ensure_not_empty(gs) || ! ensure_point_type(gs))
    return NULL;
  meosType temptype = FLAGS_GET_GEODETIC(gs->gflags) ?
    T_TGEOGPOINT : T_TGEOMPOINT;
  return tsequence_from_base_tstzset(PointerGetDatum(gs), temptype, s);
}

/**
 * @ingroup meos_geo_constructor
 * @brief Return a temporal geo discrete sequence from a geometry/geography
 * and a timestamptz set
 * @param[in] gs Value
 * @param[in] s Set
 */
TSequence *
tgeoseq_from_base_tstzset(const GSERIALIZED *gs, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TSTZSET(s, NULL);
  if (! ensure_not_empty(gs))
    return NULL;
  meosType temptype = FLAGS_GET_GEODETIC(gs->gflags) ?
    T_TGEOGRAPHY : T_TGEOMETRY;
  return tsequence_from_base_tstzset(PointerGetDatum(gs), temptype, s);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_constructor
 * @brief Return a temporal point sequence from a point and a timestamptz span
 * @param[in] gs Value
 * @param[in] s Span
 * @param[in] interp Interpolation
 */
TSequence *
tpointseq_from_base_tstzspan(const GSERIALIZED *gs, const Span *s,
  interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TSTZSPAN(s, NULL);
  if (gserialized_is_empty(gs))
    return NULL;
  meosType temptype = FLAGS_GET_GEODETIC(gs->gflags) ?
    T_TGEOGPOINT : T_TGEOMPOINT;
  return tsequence_from_base_tstzspan(PointerGetDatum(gs), temptype, s,
    interp);
}

/**
 * @ingroup meos_geo_constructor
 * @brief Return a temporal geo sequence from a geometry/geography and a
 * timestamptz span
 * @param[in] gs Value
 * @param[in] s Span
 * @param[in] interp Interpolation
 */
TSequence *
tgeoseq_from_base_tstzspan(const GSERIALIZED *gs, const Span *s,
  interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TSTZSPAN(s, NULL);
  if (gserialized_is_empty(gs))
    return NULL;
  meosType temptype = FLAGS_GET_GEODETIC(gs->gflags) ?
    T_TGEOGRAPHY : T_TGEOMETRY;
  return tsequence_from_base_tstzspan(PointerGetDatum(gs), temptype, s,
    interp);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_constructor
 * @brief Return a temporal point sequence set from a point and a timestamptz
 * span set
 * @param[in] gs Value
 * @param[in] ss Span set
 * @param[in] interp Interpolation
 */
TSequenceSet *
tpointseqset_from_base_tstzspanset(const GSERIALIZED *gs, const SpanSet *ss,
  interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TSTZSPANSET(ss, NULL);
  if (! ensure_not_empty(gs) || ! ensure_point_type(gs))
    return NULL;
  meosType temptype = FLAGS_GET_GEODETIC(gs->gflags) ?
    T_TGEOGPOINT : T_TGEOMPOINT;
  return tsequenceset_from_base_tstzspanset(PointerGetDatum(gs), temptype, ss,
    interp);
}

/**
 * @ingroup meos_geo_constructor
 * @brief Return a temporal geo sequence set from a point and a timestamptz
 * span set
 * @param[in] gs Value
 * @param[in] ss Span set
 * @param[in] interp Interpolation
 */
TSequenceSet *
tgeoseqset_from_base_tstzspanset(const GSERIALIZED *gs, const SpanSet *ss,
  interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TSTZSPANSET(ss, NULL);
  if (! ensure_not_empty(gs))
    return NULL;
  meosType temptype = FLAGS_GET_GEODETIC(gs->gflags) ?
    T_TGEOGRAPHY : T_TGEOMETRY;
  return tsequenceset_from_base_tstzspanset(PointerGetDatum(gs), temptype, ss,
    interp);
}


/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_constructor
 * @brief Return a temporal geometry/point from a geometry and the time frame
 * of another temporal value
 * @param[in] gs Value
 * @param[in] temp Temporal value
 * @param[in] ispoint True for temporal points, false for temporal geos
 */
Temporal *
tgeo_from_base_temp_int(const GSERIALIZED *gs, const Temporal *temp, 
  bool ispoint)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_empty(gs))
    return NULL;
  meosType tgeotype;
  if (ispoint)
    tgeotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_TGEOGPOINT : T_TGEOMPOINT;
  else
    tgeotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_TGEOGRAPHY : T_TGEOMETRY;
  return temporal_from_base_temp(PointerGetDatum(gs), tgeotype, temp);
}

/**
 * @ingroup meos_geo_constructor
 * @brief Return a temporal point from a point and the time frame of another
 * temporal value
 * @param[in] gs Value
 * @param[in] temp Temporal value
 */
inline Temporal *
tpoint_from_base_temp(const GSERIALIZED *gs, const Temporal *temp)
{
  return tgeo_from_base_temp_int(gs, temp, true);
}

/**
 * @ingroup meos_geo_constructor
 * @brief Return a temporal geo from a geometry/geography and the time frame of
 * another temporal value
 * @param[in] gs Value
 * @param[in] temp Temporal value
 */
inline Temporal *
tgeo_from_base_temp(const GSERIALIZED *gs, const Temporal *temp)
{
  return tgeo_from_base_temp_int(gs, temp, false);
}

/*****************************************************************************
 * Accessors
 *****************************************************************************/

/**
 * @ingroup meos_geo_accessor
 * @brief Return a copy of the start value of a temporal geo
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_value()
 */
GSERIALIZED *
tgeo_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL);
  return DatumGetGserializedP(temporal_start_value(temp));
}

/**
 * @ingroup meos_geo_accessor
 * @brief Return a copy of the end value of a temporal geo
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_value()
 */
GSERIALIZED *
tgeo_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL);
  return DatumGetGserializedP(temporal_end_value(temp));
}

/**
 * @ingroup meos_geo_accessor
 * @brief Return a copy of the n-th value of a temporal geo
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @csqlfn #Temporal_value_n()
 */
bool
tgeo_value_n(const Temporal *temp, int n, GSERIALIZED **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, false); VALIDATE_NOT_NULL(result, false);
  Datum dresult;
  if (! temporal_value_n(temp, n, &dresult))
    return false;
  *result = DatumGetGserializedP(dresult);
  return true;
}

/**
 * @ingroup meos_geo_accessor
 * @brief Return the array of copies of base values of a temporal geo
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
GSERIALIZED **
tgeo_values(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  Datum *datumarr = temporal_values_p(temp, count);
  GSERIALIZED **result = palloc(sizeof(GSERIALIZED *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = geo_copy(DatumGetGserializedP(datumarr[i]));
  pfree(datumarr);
  return result;
}

/*****************************************************************************/
