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
 * @brief Input/output of temporal geos in WKT, EWKT, and MF-JSON format
 */

#include "geo/tgeo_out.h"

/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"

/*****************************************************************************
 * Intput in MF-JSON format
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geometry point instant from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @csqlfn #Temporal_from_mfjson()
 */
TInstant *
tgeompointinst_from_mfjson(json_object *mfjson, int srid)
{
  return tinstant_from_mfjson(mfjson, true, srid, T_TGEOMPOINT);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geography point instant from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @csqlfn #Temporal_from_mfjson()
 */
TInstant *
tgeogpointinst_from_mfjson(json_object *mfjson, int srid)
{
  return tinstant_from_mfjson(mfjson, true, srid, T_TGEOGPOINT);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geometry instant from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @csqlfn #Temporal_from_mfjson()
 */
TInstant *
tgeometryinst_from_mfjson(json_object *mfjson, int srid)
{
  return tinstant_from_mfjson(mfjson, true, srid, T_TGEOMETRY);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geography instant from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @csqlfn #Temporal_from_mfjson()
 */
TInstant *
tgeographyinst_from_mfjson(json_object *mfjson, int srid)
{
  return tinstant_from_mfjson(mfjson, true, srid, T_TGEOGRAPHY);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geometry point sequence from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
TSequence *
tgeompointseq_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequence_from_mfjson(mfjson, true, srid, T_TGEOMPOINT, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geography point sequence from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
TSequence *
tgeogpointseq_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequence_from_mfjson(mfjson, true, srid, T_TGEOGPOINT, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geometry sequence from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
TSequence *
tgeometryseq_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequence_from_mfjson(mfjson, true, srid, T_TGEOMETRY, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geography sequence from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
TSequence *
tgeographyseq_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequence_from_mfjson(mfjson, true, srid, T_TGEOGRAPHY, interp);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geometry point sequence set from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
TSequenceSet *
tgeompointseqset_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, true, srid, T_TGEOMPOINT, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geography point sequence set from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
TSequenceSet *
tgeogpointseqset_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, true, srid, T_TGEOGPOINT, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geometry sequence set from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
TSequenceSet *
tgeometryseqset_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, true, srid, T_TGEOMETRY, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geography sequence set from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @param[in] srid SRID
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
TSequenceSet *
tgeographyseqset_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, true, srid, T_TGEOGRAPHY, interp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal geometry point from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
Temporal *
tgeompoint_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TGEOMPOINT);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal geography point from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
Temporal *
tgeogpoint_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TGEOGPOINT);
}
/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal geometry from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
Temporal *
tgeometry_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TGEOMETRY);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal geography from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
Temporal *
tgeography_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TGEOGRAPHY);
}

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * @brief Output a geometry/geography in the Well-Known Text (WKT)
 * representation (internal function)
 * @note The parameter @p type is not needed for geometries/geographies
 */
char *
geo_wkt_out(Datum value, int maxdd, bool extended)
{
  GSERIALIZED *gs = DatumGetGserializedP(value);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  char *result = lwgeom_to_wkt(geom, extended ? WKT_EXTENDED : WKT_ISO, maxdd, 
    NULL);
  lwgeom_free(geom);
  return result;
}

/*****************************************************************************/

#if MEOS
/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal geo
 * @param[in] temp Temporal geo
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tgeo_out(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || 
      ! ensure_tgeo_type_all(temp->temptype))
    return NULL;
  return temporal_out(temp, maxdd);
}
#endif /* MEOS */

/*****************************************************************************/
