/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Create a cache of metadata information about temporal types and
 * span types in global arrays.
 */

#include "general/meos_catalog.h"

/* PostgreSQL */
/* MEOS */
#include <meos.h>
#include "general/temporaltypes.h"
#if NPOINT
  #include "npoint/tnpoint_static.h"
#endif

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/**
 * Global array that keeps type information for the temporal types defined
 * in MobilityDB.
 */
temptype_cache_struct _temptype_cache[] =
{
  /* temptype    basetype */
  {T_TDOUBLE2,   T_DOUBLE2},
  {T_TDOUBLE3,   T_DOUBLE3},
  {T_TDOUBLE4,   T_DOUBLE4},
  {T_TBOOL,      T_BOOL},
  {T_TINT,       T_INT4},
  {T_TFLOAT,     T_FLOAT8},
  {T_TTEXT,      T_TEXT},
  {T_TGEOMPOINT, T_GEOMETRY},
  {T_TGEOGPOINT, T_GEOGRAPHY},
#if NPOINT
  {T_TNPOINT,    T_NPOINT},
#endif
};

/**
 * Global array that keeps type information for the set types defined
 * in MobilityDB.
 */
settype_cache_struct _settype_cache[] =
{
  /* settype        basetype */
  {T_INTSET,        T_INT4},
  {T_BIGINTSET,     T_INT8},
  {T_FLOATSET,      T_FLOAT8},
  {T_TSTZSET,       T_TIMESTAMPTZ},
  {T_TEXTSET,       T_TEXT},
  {T_GEOMSET,       T_GEOMETRY},
  {T_GEOGSET,       T_GEOGRAPHY},
  {T_NPOINTSET,     T_NPOINT},
};

/**
 * Global array that keeps type information for the span types defined
 * in MobilityDB.
 */
spantype_cache_struct _spantype_cache[] =
{
  /* spantype       basetype */
  {T_INTSPAN,       T_INT4},
  {T_BIGINTSPAN,    T_INT8},
  {T_FLOATSPAN,     T_FLOAT8},
  {T_TSTZSPAN,      T_TIMESTAMPTZ},
};

/**
 * Global array that keeps type information for the span set types defined
 * in MobilityDB.
 */
spansettype_cache_struct _spansettype_cache[] =
{
  /* spansettype    spantype */
  {T_INTSPANSET,    T_INTSPAN},
  {T_BIGINTSPANSET, T_BIGINTSPAN},
  {T_FLOATSPANSET,  T_FLOATSPAN},
  {T_TSTZSPANSET,   T_TSTZSPAN},
};

/*****************************************************************************
 * Cache functions
 *****************************************************************************/

/**
 * Return the base type from the temporal type
 * @note this function is defined again for MobilityDB below
 */
meosType
temptype_basetype(meosType temptype)
{
  int n = sizeof(_temptype_cache) / sizeof(temptype_cache_struct);
  for (int i = 0; i < n; i++)
  {
    if (_temptype_cache[i].temptype == temptype)
      return _temptype_cache[i].basetype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a temporal type", temptype);
}

/**
 * Return the base type from the span type
 */
meosType
spantype_basetype(meosType spantype)
{
  int n = sizeof(_spantype_cache) / sizeof(spantype_cache_struct);
  for (int i = 0; i < n; i++)
  {
    if (_spantype_cache[i].spantype == spantype)
      return _spantype_cache[i].basetype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a span type", spantype);
}

/**
 * Return the span type from the span set type
 */
meosType
spansettype_spantype(meosType spansettype)
{
  int n = sizeof(_spansettype_cache) / sizeof(spansettype_cache_struct);
  for (int i = 0; i < n; i++)
  {
    if (_spansettype_cache[i].spansettype == spansettype)
      return _spansettype_cache[i].spantype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a span set type", spansettype);
}

/**
 * Return the base type from the span type
 */
meosType
basetype_spantype(meosType basetype)
{
  int n = sizeof(_spantype_cache) / sizeof(spantype_cache_struct);
  for (int i = 0; i < n; i++)
  {
    if (_spantype_cache[i].basetype == basetype)
      return _spantype_cache[i].spantype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a span type", basetype);
}

/**
 * Return the span type from the span set type
 */
meosType
spantype_spansettype(meosType spantype)
{
  int n = sizeof(_spansettype_cache) / sizeof(spansettype_cache_struct);
  for (int i = 0; i < n; i++)
  {
    if (_spansettype_cache[i].spantype == spantype)
      return _spansettype_cache[i].spansettype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a span type", spantype);
}

/**
 * Return the base type from a set type
 */
meosType
settype_basetype(meosType settype)
{
  int n = sizeof(_settype_cache) / sizeof(settype_cache_struct);
  for (int i = 0; i < n; i++)
  {
    if (_settype_cache[i].settype == settype)
      return _settype_cache[i].basetype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a set type", settype);
}

/**
 * Return the base type from the set type
 */
meosType
basetype_settype(meosType basetype)
{
  int n = sizeof(_settype_cache) / sizeof(settype_cache_struct);
  for (int i = 0; i < n; i++)
  {
    if (_settype_cache[i].basetype == basetype)
      return _settype_cache[i].settype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a set type", basetype);
}

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

/**
 * Ensure that a type is a base type
 */
void
ensure_basetype(meosType basetype)
{
  if (basetype != T_BOOL && basetype != T_TEXT && basetype != T_INT4 &&
    basetype != T_INT8 && basetype != T_FLOAT8 && basetype != T_TIMESTAMPTZ &&
    basetype != T_GEOMETRY && basetype != T_GEOGRAPHY && basetype != T_NPOINT
    )
    elog(ERROR, "unknown base type: %d", basetype);
  return;
}

/**
 * Return true if the type is a set type
 */
bool
alpha_basetype(meosType basetype)
{
  if (basetype == T_BOOL || basetype == T_TEXT)
    return true;
  return false;
}

/**
 * Return true if the type is a set type
 */
bool
number_basetype(meosType basetype)
{
  if (basetype == T_INT4 || basetype == T_INT8 || basetype == T_FLOAT8)
    return true;
  return false;
}

/**
 * Return true if the type is a set type
 */
bool
alphanum_basetype(meosType basetype)
{
  if (basetype == T_BOOL || basetype == T_TEXT || basetype == T_INT4 ||
      basetype == T_INT8 || basetype == T_FLOAT8 || basetype == T_TIMESTAMPTZ)
    return true;
  return false;
}

/**
 * Return true if the type is a geo base type
 */
bool
geo_basetype(meosType basetype)
{
  if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY)
    return true;
  return false;
}

/**
 * Return true if the type is a spatial base type
 */
bool
spatial_basetype(meosType basetype)
{
  if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY)
    return true;
  return false;
}

/*****************************************************************************/

/**
 * Return true if the type is a time type
 */
bool
time_type(meosType timetype)
{
  if (timetype == T_TIMESTAMPTZ || timetype == T_TSTZSET ||
    timetype == T_TSTZSPAN || timetype == T_TSTZSPANSET)
    return true;
  return false;
}

/**
 * Ensure that the type corresponds to a time type
 */
void
ensure_time_type(meosType timetype)
{
  if (! time_type(timetype))
    elog(ERROR, "unknown time type: %d", timetype);
  return;
}

/*****************************************************************************/

/**
 * Return true if the type is a base type of a set type
 */
bool
set_basetype(meosType basetype)
{
  if (basetype == T_TIMESTAMPTZ || basetype == T_INT4 || basetype == T_INT8 ||
      basetype == T_FLOAT8 || basetype == T_TEXT || basetype == T_GEOMETRY ||
      basetype == T_GEOGRAPHY || basetype == T_NPOINT)
    return true;
  return false;
}

/**
 * Ensure that the span base type is
 */
void
ensure_set_basetype(meosType basetype)
{
  if (! set_basetype(basetype))
    elog(ERROR, "unknown set base type: %d", basetype);
  return;
}

/**
 * Return true if the type is a set type
 */
bool
set_type(meosType settype)
{
  if (settype == T_TSTZSET || settype == T_INTSET || settype == T_BIGINTSET ||
      settype == T_FLOATSET || settype == T_TEXTSET || settype == T_GEOMSET ||
      settype == T_GEOGSET || settype == T_NPOINTSET)
    return true;
  return false;
}

/**
 * Ensure that the type is a set type
 */
void
ensure_set_type(meosType settype)
{
  if (! set_type(settype))
    elog(ERROR, "unknown set type: %d", settype);
  return;
}

/**
 * Return true if the type is a set type
 */
bool
alphanumset_type(meosType settype)
{
  if (settype == T_TSTZSET || settype == T_INTSET || settype == T_BIGINTSET ||
      settype == T_FLOATSET || settype == T_TEXTSET)
    return true;
  return false;
}

/**
 * Return true if the type is a set type
 */
bool
numset_type(meosType settype)
{
  if (settype == T_INTSET || settype == T_BIGINTSET || settype == T_FLOATSET)
    return true;
  return false;
}

#if 0 /* not used */
/**
 * Ensure that the type is a set type
 */
void
ensure_numset_type(meosType settype)
{
  if (! numset_type(settype))
    elog(ERROR, "unknown numeric set type: %d", settype);
  return;
}

/**
 * Return true if the type is a base type of a numeric set type
 */
bool
numset_basetype(meosType basetype)
{
  if (basetype == T_INT4 || basetype == T_INT8 || basetype == T_FLOAT8 ||
      basetype == T_TEXT)
    return true;
  return false;
}

/**
 * Ensure that the type is a base type of a numeric set type
 */
void
ensure_numset_basetype(meosType basetype)
{
  if (! numspan_basetype(basetype))
    elog(ERROR, "unknown numeric set base type: %d", basetype);
  return;
}
#endif /* not used */

/**
 * Return true if the type is a geo set type
 */
bool
geoset_type(meosType settype)
{
  if (settype == T_GEOMSET || settype == T_GEOGSET || settype == T_NPOINTSET)
    return true;
  return false;
}

/*****************************************************************************/

/**
 * Return true if the type is a span type
 */
bool
span_type(meosType spantype)
{
  if (spantype == T_TSTZSPAN || spantype == T_INTSPAN ||
      spantype == T_BIGINTSPAN || spantype == T_FLOATSPAN)
    return true;
  return false;
}

/**
 * Ensure that the type is a span type
 */
void
ensure_span_type(meosType spantype)
{
  if (! span_type(spantype))
    elog(ERROR, "unknown span type: %d", spantype);
  return;
}

/**
 * Return true if the type is a numeric span type
 */
bool
numspan_type(meosType spantype)
{
  if (spantype == T_INTSPAN || spantype == T_BIGINTSPAN ||
      spantype == T_FLOATSPAN)
    return true;
  return false;
}

#if 0 /* not used */
/**
 * Ensure that the type is a span type
 */
void
ensure_numspan_type(meosType spantype)
{
  if (! numspan_type(spantype))
    elog(ERROR, "unknown numeric span type: %d", spantype);
  return;
}
#endif /* not used */

/**
 * Return true if the type is a set base type
 */
bool
span_basetype(meosType basetype)
{
  if (basetype == T_TIMESTAMPTZ || basetype == T_INT4 || basetype == T_INT8 ||
      basetype == T_FLOAT8)
    return true;
  return false;
}

/**
 * Ensure that the type is a span base type
 */
void
ensure_span_basetype(meosType basetype)
{
  if (! span_basetype(basetype))
    elog(ERROR, "unknown span base type: %d", basetype);
  return;
}

/**
 * Return true if the type is a base type of a numeric span type
 */
bool
numspan_basetype(meosType basetype)
{
  if (basetype == T_INT4 || basetype == T_INT8 || basetype == T_FLOAT8)
    return true;
  return false;
}

#if 0 /* not used */
/**
 * Ensure that the type is a base type of a numeric span type
 */
void
ensure_numspan_basetype(meosType basetype)
{
  if (! numspan_basetype(basetype))
    elog(ERROR, "unknown numeric span base type: %d", basetype);
  return;
}
#endif /* not used */

/*****************************************************************************/

/**
 * Return true if the type is a span set type
 */
bool
spanset_type(meosType spansettype)
{
  if (spansettype == T_TSTZSPANSET || spansettype == T_INTSPANSET ||
      spansettype == T_BIGINTSPANSET || spansettype == T_FLOATSPANSET)
    return true;
  return false;
}

#if 0 /* not used */
/**
 * Ensure that the type is a span type
 */
void
ensure_spanset_type(meosType spansettype)
{
  if (! spanset_type(spansettype))
    elog(ERROR, "unknown span set type: %d", spansettype);
  return;
}
#endif /* not used */

/**
 * Return true if the type is a numeric span type
 */
bool
numspanset_type(meosType spansettype)
{
  if (spansettype == T_INTSPANSET || spansettype == T_BIGINTSPANSET ||
      spansettype == T_FLOATSPANSET)
    return true;
  return false;
}

#if 0 /* not used */
/**
 * Ensure that the type is a span type
 */
void
ensure_numspanset_type(meosType spansettype)
{
  if (! numspanset_type(spansettype))
    elog(ERROR, "unknown numeric span set type: %d", spansettype);
  return;
}

/**
 * Return true if the type is a set base type
 */
bool
spanset_basetype(meosType basetype)
{
  if (basetype == T_TIMESTAMPTZ || basetype == T_INT4 || basetype == T_INT8 ||
      basetype == T_FLOAT8)
    return true;
  return false;
}

/**
 * Ensure that the type is a span set base type
 */
void
ensure_spanset_basetype(meosType basetype)
{
  if (! spanset_basetype(basetype))
    elog(ERROR, "unknown span set base type: %d", basetype);
  return;
}

/**
 * Return true if the type is a base type of a numeric span set type
 */
bool
numspanset_basetype(meosType basetype)
{
  if (basetype == T_INT4 || basetype == T_INT8 || basetype == T_FLOAT8)
    return true;
  return false;
}

/**
 * Ensure that the type is a base type of a numeric span set type
 */
void
ensure_numspanset_basetype(meosType basetype)
{
  if (! numspanset_basetype(basetype))
    elog(ERROR, "unknown numeric span set base type: %d", basetype);
  return;
}
#endif /* not used */

/*****************************************************************************/

/**
 * @brief Return true if the temporal type is an EXTERNAL temporal type
 *
 * @note Function used in particular in the indexes
 */
bool
temporal_type(meosType temptype)
{
  if (temptype == T_TBOOL || temptype == T_TINT || temptype == T_TFLOAT ||
    temptype == T_TTEXT || temptype == T_TGEOMPOINT || temptype == T_TGEOGPOINT
#if NPOINT
    || temptype == T_TNPOINT
#endif
    )
    return true;
  return false;
}

/**
 * Ensure that a type is a temporal type
 */
void
ensure_temporal_type(meosType temptype)
{
  if (! temporal_type(temptype))
    elog(ERROR, "unknown temporal type: %d", temptype);
  return;
}

/**
 * Ensure that a type is a temporal base type
 * @note The TimestampTz type is added to cope with base types for spans.
 * Also, the int8 type is added to cope with the rid in network points.
 */
void
ensure_temporal_basetype(meosType basetype)
{
  if (basetype != T_TIMESTAMPTZ &&
    basetype != T_BOOL && basetype != T_INT4 && basetype != T_INT8 &&
    basetype != T_FLOAT8 && basetype != T_TEXT &&
    basetype != T_DOUBLE2 && basetype != T_DOUBLE3 && basetype != T_DOUBLE4 &&
    basetype != T_GEOMETRY && basetype != T_GEOGRAPHY
#if NPOINT
    && basetype != T_NPOINT
#endif
    )
    elog(ERROR, "unknown temporal base type: %d", basetype);
  return;
}

/**
 * Return true if the temporal type is continuous
 */
bool
temptype_continuous(meosType temptype)
{
  if (temptype == T_TFLOAT || temptype == T_TDOUBLE2 ||
    temptype == T_TDOUBLE3 || temptype == T_TDOUBLE4 ||
    temptype == T_TGEOMPOINT || temptype == T_TGEOGPOINT
#if NPOINT
    || temptype == T_TNPOINT
#endif
    )
    return true;
  return false;
}

/**
 * Ensure that the temporal type is continuous
 */
void
ensure_temptype_continuous(meosType temptype)
{
  if (! temptype_continuous(temptype))
    elog(ERROR, "unknown continuous temporal type: %d", temptype);
  return;
}

/**
 * Return true if the values of the type are passed by value.
 *
 * This function is called only for the base types of the temporal types
 * To avoid a call of the slow function get_typbyval (which makes a lookup
 * call), the known base types are explicitly enumerated.
 */
bool
basetype_byvalue(meosType basetype)
{
  ensure_temporal_basetype(basetype);
  if (basetype == T_BOOL || basetype == T_INT4 || basetype == T_INT8 ||
      basetype == T_FLOAT8 || basetype == T_TIMESTAMPTZ)
    return true;
  return false;
}

/**
 * Return the length of type
 *
 * This function is called only for the base types of the temporal types
 * passed by reference. To avoid a call of the slow function get_typlen
 * (which makes a lookup call), the known base types are explicitly enumerated.
 */
int16
basetype_length(meosType basetype)
{
  ensure_temporal_basetype(basetype);
  if (basetype == T_DOUBLE2)
    return sizeof(double2);
  if (basetype == T_DOUBLE3)
    return sizeof(double3);
  if (basetype == T_DOUBLE4)
    return sizeof(double4);
  if (basetype == T_TEXT)
    return -1;
  if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY)
    return -1;
#if NPOINT
  if (basetype == T_NPOINT)
    return sizeof(Npoint);
#endif
  elog(ERROR, "unknown basetype_length function for base type: %d", basetype);
}

/**
 * Return true if the type is a temporal alpha type (i.e., those whose
 * bounding box is a period) supported by MobilityDB
 */
bool
talpha_type(meosType temptype)
{
  if (temptype == T_TBOOL || temptype == T_TTEXT)
    return true;
  return false;
}

/**
 * Return true if the type is a temporal number type
 */
bool
tnumber_type(meosType temptype)
{
  if (temptype == T_TINT || temptype == T_TFLOAT)
    return true;
  return false;
}

/**
 * Return true if the type is a number base type supported by MobilityDB
 */
void
ensure_tnumber_type(meosType temptype)
{
  if (! tnumber_type(temptype))
    elog(ERROR, "unknown temporal number type: %d", temptype);
  return;
}

/**
 * Test whether the type is a number base type supported by MobilityDB
 */
bool
tnumber_basetype(meosType basetype)
{
  if (basetype == T_INT4 || basetype == T_FLOAT8)
    return true;
  return false;
}

/**
 * Return true if the type is a number base type supported by MobilityDB
 */
void
ensure_tnumber_basetype(meosType basetype)
{
  if (! tnumber_basetype(basetype))
    elog(ERROR, "unknown number base type: %d", basetype);
  return;
}

/**
 * Return true if the type is a set number type
 *
 * @note Function used in particular in the indexes
 */
bool
tnumber_settype(meosType settype)
{
  if (settype == T_INTSET || settype == T_FLOATSET)
    return true;
  return false;
}

/**
 * Ensure that the type is a span type
 */
void
ensure_tnumber_settype(meosType settype)
{
  if (! tnumber_settype(settype))
    elog(ERROR, "unknown number set type: %d", settype);
  return;
}

/**
 * Return true if the type is a span number type
 *
 * @note Function used in particular in the indexes
 */
bool
tnumber_spantype(meosType spantype)
{
  if (spantype == T_INTSPAN || spantype == T_FLOATSPAN)
    return true;
  return false;
}

/**
 * Ensure that the type is a span type
 */
void
ensure_tnumber_spantype(meosType spantype)
{
  if (! tnumber_spantype(spantype))
    elog(ERROR, "unknown number span type: %d", spantype);
  return;
}

/**
 * Return true if the type is a span number type
 *
 * @note Function used in particular in the indexes
 */
bool
tnumber_spansettype(meosType spansettype)
{
  if (spansettype == T_INTSPANSET || spansettype == T_FLOATSPANSET)
    return true;
  return false;
}

/**
 * Ensure that the type is a span type
 */
void
ensure_tnumber_spansettype(meosType spansettype)
{
  if (! tnumber_spansettype(spansettype))
    elog(ERROR, "unknown number span set type: %d", spansettype);
  return;
}

/**
 * Return true if the type is a spatiotemporal type
 *
 * @note This function is used for features common to all spatiotemporal types,
 * in particular, all of them use the same bounding box STBox. Therefore it is
 * used for the indexes and selectivity functions
 */
bool
tspatial_type(meosType temptype)
{
  if (temptype == T_TGEOMPOINT || temptype == T_TGEOGPOINT
#if NPOINT
      || temptype == T_TNPOINT
#endif
      )
    return true;
  return false;
}

/**
 * Return true if the type is a spatiotemporal type
 *
 * @note This function is used for features common to all spatiotemporal types,
 * in particular, all of them use the same bounding box STBox
 */
bool
tspatial_basetype(meosType basetype)
{
  if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY
#if NPOINT
    || basetype == T_NPOINT
#endif
    )
    return true;
  return false;
}

/**
 * Return true if the type is a temporal point type supported by MobilityDB
 */
bool
tgeo_type(meosType temptype)
{
  if (temptype == T_TGEOMPOINT || temptype == T_TGEOGPOINT)
    return true;
  return false;
}

/**
 * Ensure that the type is a point base type supported by MobilityDB
 */
void
ensure_tgeo_type(meosType temptype)
{
  if (! tgeo_type(temptype))
    elog(ERROR, "unknown geospatial temporal type: %d", temptype);
  return;
}

/*****************************************************************************/
