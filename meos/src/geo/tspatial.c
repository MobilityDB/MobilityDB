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
 * @brief General functions for temporal points
 */

#include "geo/tspatial.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/set.h"
#include "temporal/lifting.h"
#include "temporal/temporal.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#if CBUFFER
  #include "cbuffer/cbuffer.h"
  #include "cbuffer/tcbuffer_boxops.h"
#endif 
#if NPOINT
  #include "npoint/tnpoint_boxops.h"
#endif 
#if POSE
  #include "pose/pose.h"
  #include "pose/tpose_boxops.h"
#endif 
#if RGEO
  #include "pose/pose.h"
  #include "rgeo/trgeo.h"
  #include "rgeo/trgeo_inst.h"
  #include "rgeo/trgeo_boxops.h"
#endif 

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @brief Return the string representation of a base value
 * @return On error return @p NULL
 */
char *
spatialbase_as_text(Datum value, meosType type, int maxdd)
{
  assert(spatial_basetype(type)); assert(maxdd >= 0);

  switch (type)
  {
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return geo_as_text(DatumGetGserializedP(value), maxdd);
#if CBUFFER
    case T_CBUFFER:
      return cbuffer_as_text(DatumGetCbufferP(value), maxdd);
#endif
#if NPOINT
    case T_NPOINT:
      return npoint_as_text(DatumGetNpointP(value), maxdd);
#endif
#if POSE
    case T_POSE:
      return pose_as_text(DatumGetPoseP(value), maxdd);
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown output function in WKT format for type: %s",
        meostype_name(type));
      return NULL;
  }
}

/**
 * @brief Return the string representation of a base value
 * @return On error return @p NULL
 */
char *
spatialbase_as_ewkt(Datum value, meosType type, int maxdd)
{
  assert(spatial_basetype(type)); assert(maxdd >= 0);


  /* Get the text representation of the value */
  char *base_str = spatialbase_as_text(value, type, maxdd);
  /* Get the SRID */
  char srid_str[18];
  srid_str[0] = '\0';
  int32 srid = spatial_srid(value, type);
  if (srid <= 0)
    return base_str;

  /* SRID_MAXIMUM is defined by PostGIS as 999999 */
  snprintf(srid_str, sizeof(srid_str), "SRID=%d;", srid);
  char *result = palloc(strlen(srid_str) + strlen(base_str) + 1);
  strcpy(result, srid_str);
  strcat(result, base_str);
  pfree(base_str);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the output representation of a spatial set given by a function
 * @param[in] s Spatial set
 * @param[in] maxdd Maximum number of decimal digits
 * @param[in] wkt_out Well-Known Text (WKT) function to print the set elements
 * without leading SRID string
 * @param[in] extended True when the leading SRID string is output
 */
char *
spatialset_out_fn(const Set *s, int maxdd, outfunc wkt_out, bool extended)
{
  /* Ensure the validity of the arguments */
  VALIDATE_SPATIALSET(s, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;
  
  char *set_str = set_out_fn(s, maxdd, wkt_out);
  if (! extended)
    return set_str;

  /* Get the SRID if extended */
  char srid_str[18];
  srid_str[0] = '\0';
  int32 srid = spatialset_srid(s);
  if (srid <= 0)
    return set_str;

  /* SRID_MAXIMUM is defined by PostGIS as 999999 */
  snprintf(srid_str, sizeof(srid_str), "SRID=%d;", srid);
  char *result = palloc(strlen(srid_str) + strlen(set_str) + 1);
  strcpy(result, srid_str);
  strcat(result, set_str);
  pfree(set_str);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_set_inout
 * @brief Return the Well-Known Text (WKT) representation of a spatial set
 * @csqlfn #Spatialset_as_text()
 */
inline char *
spatialset_as_text(const Set *s, int maxdd)
{
  return spatialset_out_fn(s, maxdd, &spatialbase_as_text, false);
}

/**
 * @ingroup meos_geo_set_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a geo set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Spatialset_as_ewkt()
 */
inline char *
spatialset_as_ewkt(const Set *s, int maxdd)
{
  /* The SRID will be output as prefix, the elements will output the SRID*/
  return spatialset_out_fn(s, maxdd, &spatialbase_as_text, true);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return the Well-Known Text (WKT) representation of a spatiotemporal
 * instant
 * @param[in] inst Temporal instant
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tspatialinst_as_text(const TInstant *inst, int maxdd)
{
  return tinstant_to_string(inst, maxdd, &spatialbase_as_text);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal
 * spatial sequence
 * @param[in] seq Temporal sequence
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tspatialseq_as_text(const TSequence *seq, int maxdd)
{
  assert(seq);
  return tsequence_to_string(seq, maxdd, false, &spatialbase_as_text);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal
 * spatial sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tspatialseqset_as_text(const TSequenceSet *ss, int maxdd)
{
  assert(ss);
  return tsequenceset_to_string(ss, maxdd, &spatialbase_as_text);
}

/**
 * @ingroup meos_geo_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal
 * spatial value
 * @param[in] temp Spatiotemporal value
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tspatial_as_text()
 */
char *
tspatial_as_text(const Temporal *temp, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSPATIAL(temp, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tspatialinst_as_text((TInstant *) temp, maxdd);
    case TSEQUENCE:
      return tspatialseq_as_text((TSequence *) temp, maxdd);
    default: /* TSEQUENCESET */
      return tspatialseqset_as_text((TSequenceSet *) temp, maxdd);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * spatiotemporal value
 * @param[in] temp Spatiotemporal value
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tspatial_as_ewkt()
 */
char *
tspatial_as_ewkt(const Temporal *temp, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSPATIAL(temp, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  int32_t srid = tspatial_srid(temp);
  char str1[18];
  /* Determine whether an interpolation prefix must follow the SRID prefix */
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  bool interp_prefix = temptype_continuous(temp->temptype) && interp == STEP;
  if (srid > 0)
    /* SRID_MAXIMUM is defined by PostGIS as 999999 */
    snprintf(str1, sizeof(str1), "SRID=%d%c", srid, interp_prefix ? ',' : ';');
  else
    str1[0] = '\0';
  char *str2 = tspatial_as_text(temp, maxdd);
  char *result = palloc(strlen(str1) + strlen(str2) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  pfree(str2);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation
 * of an array of spatial values
 * @param[in] spatialarr Array of spatial values
 * @param[in] elemtype Type of the elements in the input array
 * @param[in] count Number of elements in the input array
 * @param[in] maxdd Maximum number of decimal digits to output
 * @param[in] extended True if the output is in EWKT
 */
char **
spatialarr_wkt_out(const Datum *spatialarr, meosType elemtype, int count,
  int maxdd, bool extended)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(spatialarr, NULL);
  if (! ensure_positive(count) || ! ensure_not_negative(maxdd))
    return NULL;

  char **result = palloc(sizeof(char *) * count);
  for (int i = 0; i < count; i++)
  {
    if (temporal_type(elemtype))
      result[i] = extended ?
        tspatial_as_ewkt(DatumGetTemporalP(spatialarr[i]), maxdd) :
        tspatial_as_text(DatumGetTemporalP(spatialarr[i]), maxdd);
    else
      result[i] = extended ?
        spatialbase_as_ewkt(spatialarr[i], elemtype, maxdd) :
        spatialbase_as_text(spatialarr[i], elemtype, maxdd);
  }
  return result;
}

#if MEOS
/**
 * @ingroup meos_internal_geo_inout
 * @brief Return the Well-Known Text (WKT) representation of an array of
 * spatial values
 * @param[in] spatialarr Array of spatial values
 * @param[in] elemtype Type of the elements in the input array
 * @param[in] count Number of elements in the input array
 * @param[in] maxdd Maximum number of decimal digits to output
 */
inline char **
spatialarr_as_text(const Datum *spatialarr, meosType elemtype, int count, 
  int maxdd)
{
  return spatialarr_wkt_out(spatialarr, elemtype, count, maxdd, false);
}

/**
 * @ingroup meos_internal_geo_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of an array
 * of spatial values
 * @param[in] spatialarr Array of spatial values
 * @param[in] elemtype Type of the elements in the input array
 * @param[in] count Number of elements in the input array
 * @param[in] maxdd Maximum number of decimal digits to output
 */
inline char **
spatialarr_as_ewkt(const Datum *spatialarr, meosType elemtype, int count, 
  int maxdd)
{
  return spatialarr_wkt_out(spatialarr, elemtype, count, maxdd, true);
}
#endif /* MEOS */

/*****************************************************************************
 * Box functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument the bounding box of a spatial set
 * @param[in] s Set
 * @param[out] box Spatiotemporal box
 */
void
spatialset_set_stbox(const Set *s, STBox *box)
{
  assert(s); assert(box);
  memset(box, 0, sizeof(STBox));
  memcpy(box, SET_BBOX_PTR(s), sizeof(STBox));
  return;
}

/**
 * @ingroup meos_geo_box_conversion
 * @brief Convert a spatiotemporal set into a spatiotemporal box
 * @param[in] s Set
 * @csqlfn #Spatialset_to_stbox()
 */
STBox *
spatialset_to_stbox(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_SPATIALSET(s, NULL);
  STBox *result = palloc(sizeof(STBox));
  spatialset_set_stbox(s, result);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_box
 * @brief Return in the last argument the spatiotemporal box of a temporal
 * spatial value
 * @param[in] temp Spatiotemporal value
 * @param[out] box Spatiotemporal box
 */
void
tspatial_set_stbox(const Temporal *temp, STBox *box)
{
  assert(temp); assert(box); assert(tspatial_type(temp->temptype));
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      if (tgeo_type_all(temp->temptype))
        tgeoinst_set_stbox((TInstant *) temp, box);
#if CBUFFER
      else if (temp->temptype == T_TCBUFFER)
        tcbufferinst_set_stbox((TInstant *) temp, box);
#endif
#if NPOINT
      else if (temp->temptype == T_TNPOINT)
        tnpointinst_set_stbox((TInstant *) temp, box);
#endif
#if POSE
      else if (temp->temptype == T_TPOSE)
        tposeinst_set_stbox((TInstant *) temp, box);
#endif
#if RGEO
      else if (temp->temptype == T_TRGEOMETRY)
        trgeoinst_set_stbox(trgeoinst_geom_p((TInstant *) temp),
          (TInstant *) temp, box);
#endif
      else
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "Unknown spatiotemporal type: %s", meostype_name(temp->temptype));
      break;
    case TSEQUENCE:
      tspatialseq_set_stbox((TSequence *) temp, box);
      break;
    default: /* TSEQUENCESET */
      tspatialseqset_set_stbox((TSequenceSet *) temp, box);
  }
  return;
}

/**
 * @ingroup meos_geo_conversion
 * @brief Convert a spatiotemporal value into a spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Tspatial_to_stbox()
 */
STBox *
tspatial_to_stbox(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSPATIAL(temp, NULL);
  STBox *result = palloc(sizeof(STBox));
  tspatial_set_stbox(temp, result);
  return result;
}

/*****************************************************************************/
