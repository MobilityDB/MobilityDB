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
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "general/pg_types.h"
#include "general/temporaltypes.h"
#if NPOINT
  #include "npoint/tnpoint_static.h"
#endif

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/**
 * @brief Global array that keeps type information for the temporal types
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
 * @brief Global array that keeps type information for the set types defined
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
#if NPOINT
  {T_NPOINTSET,     T_NPOINT},
#endif
};

/**
 * @brief Global array that keeps type information for the span types defined
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
 * @brief Global array that keeps type information for the span set types defined
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
 * @brief Return the base type from the temporal type
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
 * @brief Return the base type from the span type
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
 * @brief Return the span type from the span set type
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
 * @brief Return the base type from the span type
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
 * @brief Return the span type from the span set type
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
 * @brief Return the base type from a set type
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
 * @brief Return the base type from the set type
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
 * @brief Ensure that a type is a base type of one of the template types, that is,
 * Set, Span, SpanSet, and Temporal
 */
void
ensure_basetype(meosType type)
{
  if (type != T_BOOL && type != T_TEXT && type != T_INT4 &&
    type != T_INT8 && type != T_FLOAT8 && type != T_TIMESTAMPTZ &&
    /* The doubleX are internal types used for temporal aggregation */
    type != T_DOUBLE2 && type != T_DOUBLE3 && type != T_DOUBLE4 &&
    type != T_GEOMETRY && type != T_GEOGRAPHY && type != T_NPOINT
    )
    elog(ERROR, "unknown base type: %d", type);
  return;
}

/**
 * @brief Return true if the values of the type are passed by value.
 */
bool
basetype_byvalue(meosType type)
{
  ensure_basetype(type);
  if (type == T_BOOL || type == T_INT4 || type == T_INT8 || type == T_FLOAT8 ||
      type == T_TIMESTAMPTZ)
    return true;
  return false;
}

/**
 * @brief Return the length of a base type
 */
int16
basetype_length(meosType type)
{
  ensure_basetype(type);
  if (type == T_DOUBLE2)
    return sizeof(double2);
  if (type == T_DOUBLE3)
    return sizeof(double3);
  if (type == T_DOUBLE4)
    return sizeof(double4);
  if (type == T_TEXT)
    return -1;
  if (type == T_GEOMETRY || type == T_GEOGRAPHY)
    return -1;
#if NPOINT
  if (type == T_NPOINT)
    return sizeof(Npoint);
#endif
  elog(ERROR, "unknown basetype_length function for base type: %d", type);
}

/**
 * @brief Return true if the type is an alpha base type
 */
bool
alpha_basetype(meosType type)
{
  if (type == T_BOOL || type == T_TEXT)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a number base type
 */
bool
number_basetype(meosType type)
{
  if (type == T_INT4 || type == T_INT8 || type == T_FLOAT8)
    return true;
  return false;
}

/**
 * @brief Return true if the type is an alphanumeric base type
 */
bool
alphanum_basetype(meosType type)
{
  if (type == T_BOOL || type == T_TEXT || type == T_INT4 ||
      type == T_INT8 || type == T_FLOAT8 || type == T_TIMESTAMPTZ)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a geo base type
 */
bool
geo_basetype(meosType type)
{
  if (type == T_GEOMETRY || type == T_GEOGRAPHY)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a spatial base type
 */
bool
spatial_basetype(meosType type)
{
  if (type == T_GEOMETRY || type == T_GEOGRAPHY)
    return true;
  return false;
}

/*****************************************************************************/

/**
 * @brief Return true if the type is a time type
 */
bool
time_type(meosType type)
{
  if (type == T_TIMESTAMPTZ || type == T_TSTZSET ||
    type == T_TSTZSPAN || type == T_TSTZSPANSET)
    return true;
  return false;
}

/**
 * @brief Ensure that the type corresponds to a time type
 */
void
ensure_time_type(meosType type)
{
  if (! time_type(type))
    elog(ERROR, "unknown time type: %d", type);
  return;
}

/*****************************************************************************/

/**
 * @brief Return true if the type is a base type of a set type
 */
bool
set_basetype(meosType type)
{
  if (type == T_TIMESTAMPTZ || type == T_INT4 || type == T_INT8 ||
      type == T_FLOAT8 || type == T_TEXT || type == T_GEOMETRY ||
      type == T_GEOGRAPHY || type == T_NPOINT)
    return true;
  return false;
}

/**
 * @brief Ensure that the type is a set base type
 */
void
ensure_set_basetype(meosType type)
{
  if (! set_basetype(type))
    elog(ERROR, "unknown set base type: %d", type);
  return;
}

/**
 * @brief Return true if the type is a set type
 */
bool
set_type(meosType type)
{
  if (type == T_TSTZSET || type == T_INTSET || type == T_BIGINTSET ||
      type == T_FLOATSET || type == T_TEXTSET || type == T_GEOMSET ||
      type == T_GEOGSET || type == T_NPOINTSET)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a set type
 */
bool
numset_type(meosType type)
{
  if (type == T_INTSET || type == T_BIGINTSET || type == T_FLOATSET)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a set type
 */
bool
alphanumset_type(meosType type)
{
  if (type == T_TSTZSET || type == T_INTSET || type == T_BIGINTSET ||
      type == T_FLOATSET || type == T_TEXTSET)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a geo set type
 */
bool
geoset_type(meosType type)
{
  if (type == T_GEOMSET || type == T_GEOGSET || type == T_NPOINTSET)
    return true;
  return false;
}

/*****************************************************************************/

/**
 * @brief Return true if the type is a set base type
 */
bool
span_basetype(meosType type)
{
  if (type == T_TIMESTAMPTZ || type == T_INT4 || type == T_INT8 ||
      type == T_FLOAT8)
    return true;
  return false;
}

/**
 * @brief Ensure that the type is a span base type
 */
void
ensure_span_basetype(meosType type)
{
  if (! span_basetype(type))
    elog(ERROR, "unknown span base type: %d", type);
  return;
}

/**
 * @brief Return true if the type is a canonical base type of a span type
 */
bool
span_canon_basetype(meosType type)
{
  if (type == T_INT4 || type == T_INT8)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a span type
 */
bool
span_type(meosType type)
{
  if (type == T_TSTZSPAN || type == T_INTSPAN ||
      type == T_BIGINTSPAN || type == T_FLOATSPAN)
    return true;
  return false;
}

/**
 * @brief Ensure that the type is a span type
 */
void
ensure_span_type(meosType type)
{
  if (! span_type(type))
    elog(ERROR, "unknown span type: %d", type);
  return;
}

/**
 * @brief Return true if the type is a numeric span type
 */
bool
numspan_type(meosType type)
{
  if (type == T_INTSPAN || type == T_BIGINTSPAN || type == T_FLOATSPAN)
    return true;
  return false;
}

/*****************************************************************************/

/**
 * @brief Return true if the type is a span set base type
 */
bool
spanset_basetype(meosType type)
{
  if (type == T_TIMESTAMPTZ || type == T_INT4 || type == T_INT8 ||
      type == T_FLOAT8)
    return true;
  return false;
}

/**
 * @brief Ensure that the type is a span set base type
 */
void
ensure_spanset_basetype(meosType type)
{
  if (! spanset_basetype(type))
    elog(ERROR, "unknown span set base type: %d", type);
  return;
}

/**
 * @brief Return true if the type is a span set type
 */
bool
spanset_type(meosType type)
{
  if (type == T_TSTZSPANSET || type == T_INTSPANSET ||
      type == T_BIGINTSPANSET || type == T_FLOATSPANSET)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a numeric span type
 */
bool
numspanset_type(meosType type)
{
  if (type == T_INTSPANSET || type == T_BIGINTSPANSET ||
      type == T_FLOATSPANSET)
    return true;
  return false;
}

/*****************************************************************************/

/**
 * @brief Return true if the type is an EXTERNAL temporal type
 * @note Function used in particular in the indexes
 */
bool
temporal_type(meosType type)
{
  if (type == T_TBOOL || type == T_TINT || type == T_TFLOAT ||
      type == T_TTEXT || type == T_TGEOMPOINT || type == T_TGEOGPOINT
#if NPOINT
    || type == T_TNPOINT
#endif
    )
    return true;
  return false;
}

/**
 * @brief Ensure that a type is a temporal type
 */
void
ensure_temporal_type(meosType type)
{
  if (! temporal_type(type))
    elog(ERROR, "unknown temporal type: %d", type);
  return;
}

/**
 * @brief Ensure that a type is a temporal base type
 */
void
ensure_temporal_basetype(meosType type)
{
  if (type != T_BOOL && type != T_INT4 && type != T_FLOAT8 && type != T_TEXT &&
    /* The doubleX are internal types used for temporal aggregation */
    type != T_DOUBLE2 && type != T_DOUBLE3 && type != T_DOUBLE4 &&
    type != T_GEOMETRY && type != T_GEOGRAPHY
#if NPOINT
    && type != T_NPOINT
#endif
    )
    elog(ERROR, "unknown temporal base type: %d", type);
  return;
}

/**
 * @brief Return true if the type is a temporal continuous type
 */
bool
temptype_continuous(meosType type)
{
  if (type == T_TFLOAT || type == T_TDOUBLE2 || type == T_TDOUBLE3 ||
      type == T_TDOUBLE4 || type == T_TGEOMPOINT || type == T_TGEOGPOINT
#if NPOINT
    || type == T_TNPOINT
#endif
    )
    return true;
  return false;
}

/**
 * @brief Ensure that the temporal type is continuous
 */
void
ensure_temptype_continuous(meosType type)
{
  if (! temptype_continuous(type))
    elog(ERROR, "unknown continuous temporal type: %d", type);
  return;
}

/**
 * @brief Return true if the type is a temporal alpha type (i.e., those whose
 * bounding box is a period)
 */
bool
talpha_type(meosType type)
{
  if (type == T_TBOOL || type == T_TTEXT)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a temporal number type
 */
bool
tnumber_type(meosType type)
{
  if (type == T_TINT || type == T_TFLOAT)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a number base type
 */
void
ensure_tnumber_type(meosType type)
{
  if (! tnumber_type(type))
    elog(ERROR, "unknown temporal number type: %d", type);
  return;
}

/**
 * @brief Return true if the type is a temporal number base type
 */
bool
tnumber_basetype(meosType type)
{
  if (type == T_INT4 || type == T_FLOAT8)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a number base type
 */
void
ensure_tnumber_basetype(meosType type)
{
  if (! tnumber_basetype(type))
    elog(ERROR, "unknown number base type: %d", type);
  return;
}

#if 0 /* not used */
/**
 * @brief Return true if the type is a set number type
 * @note Function used in particular in the indexes
 */
bool
tnumber_settype(meosType type)
{
  if (type == T_INTSET || type == T_FLOATSET)
    return true;
  return false;
}

/**
 * @brief Ensure that the type is a span type
 */
void
ensure_tnumber_settype(meosType type)
{
  if (! tnumber_settype(type))
    elog(ERROR, "unknown number set type: %d", type);
  return;
}
#endif /* not used */

/**
 * @brief Return true if the type is a span number type
 * @note Function used in particular in the indexes
 */
bool
tnumber_spantype(meosType type)
{
  if (type == T_INTSPAN || type == T_FLOATSPAN)
    return true;
  return false;
}

/**
 * @brief Ensure that the type is a span type
 */
void
ensure_tnumber_spantype(meosType type)
{
  if (! tnumber_spantype(type))
    elog(ERROR, "unknown number span type: %d", type);
  return;
}

/**
 * @brief Return true if the type is a span set number type
 */
bool
tnumber_spansettype(meosType type)
{
  if (type == T_INTSPANSET || type == T_FLOATSPANSET)
    return true;
  return false;
}

/**
 * @brief Ensure that the type is a span type
 */
void
ensure_tnumber_spansettype(meosType type)
{
  if (! tnumber_spansettype(type))
    elog(ERROR, "unknown number span set type: %d", type);
  return;
}

/**
 * @brief Return true if the type is a spatiotemporal type
 * @note This function is used for features common to all spatiotemporal types,
 * in particular, all of them use the same bounding box STBox. Therefore it is
 * used for the indexes and selectivity functions
 */
bool
tspatial_type(meosType type)
{
  if (type == T_TGEOMPOINT || type == T_TGEOGPOINT
#if NPOINT
      || type == T_TNPOINT
#endif
      )
    return true;
  return false;
}

/**
 * @brief Return true if the type is a base type of a spatiotemporal type
 * @note This function is used for features common to all spatiotemporal types,
 * in particular, all of them use the same bounding box STBox
 */
bool
tspatial_basetype(meosType type)
{
  if (type == T_GEOMETRY || type == T_GEOGRAPHY
#if NPOINT
    || type == T_NPOINT
#endif
    )
    return true;
  return false;
}

/**
 * @brief Return true if the type is a temporal point type
 */
bool
tgeo_type(meosType type)
{
  if (type == T_TGEOMPOINT || type == T_TGEOGPOINT)
    return true;
  return false;
}

/**
 * @brief Ensure that the type is a point base type
 */
void
ensure_tgeo_type(meosType type)
{
  if (! tgeo_type(type))
    elog(ERROR, "unknown geospatial temporal type: %d", type);
  return;
}

/*****************************************************************************/
