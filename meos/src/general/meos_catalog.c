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
 * @file
 * @brief Create a cache of metadata information about temporal types and
 * span types in global arrays.
 */

#include "general/meos_catalog.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
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
temptype_catalog_struct _temptype_catalog[] =
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
  {T_TNPOINT,    T_NPOINT},
};

/**
 * @brief Global array that keeps type information for the set types defined
 */
settype_catalog_struct _settype_catalog[] =
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
 * @brief Global array that keeps type information for the span types defined
 */
spantype_catalog_struct _spantype_catalog[] =
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
spansettype_catalog_struct _spansettype_catalog[] =
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
  int n = sizeof(_temptype_catalog) / sizeof(temptype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_temptype_catalog[i].temptype == temptype)
      return _temptype_catalog[i].basetype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a temporal type", temptype);
  return T_UNKNOWN; /* make compiler quiet */
}

/**
 * @brief Return the base type from the span type
 */
meosType
spantype_basetype(meosType spantype)
{
  int n = sizeof(_spantype_catalog) / sizeof(spantype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_spantype_catalog[i].spantype == spantype)
      return _spantype_catalog[i].basetype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a span type", spantype);
  return T_UNKNOWN; /* make compiler quiet */
}

/**
 * @brief Return the span type from the span set type
 */
meosType
spansettype_spantype(meosType spansettype)
{
  int n = sizeof(_spansettype_catalog) / sizeof(spansettype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_spansettype_catalog[i].spansettype == spansettype)
      return _spansettype_catalog[i].spantype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a span set type", spansettype);
  return T_UNKNOWN; /* make compiler quiet */
}

/**
 * @brief Return the base type from the span type
 */
meosType
basetype_spantype(meosType basetype)
{
  int n = sizeof(_spantype_catalog) / sizeof(spantype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_spantype_catalog[i].basetype == basetype)
      return _spantype_catalog[i].spantype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a span type", basetype);
  return T_UNKNOWN; /* make compiler quiet */
}

/**
 * @brief Return the span type from the span set type
 */
meosType
spantype_spansettype(meosType spantype)
{
  int n = sizeof(_spansettype_catalog) / sizeof(spansettype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_spansettype_catalog[i].spantype == spantype)
      return _spansettype_catalog[i].spansettype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a span type", spantype);
  return T_UNKNOWN; /* make compiler quiet */
}

/**
 * @brief Return the base type from a set type
 */
meosType
settype_basetype(meosType settype)
{
  int n = sizeof(_settype_catalog) / sizeof(settype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_settype_catalog[i].settype == settype)
      return _settype_catalog[i].basetype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a set type", settype);
  return T_UNKNOWN; /* make compiler quiet */
}

/**
 * @brief Return the base type from the set type
 */
meosType
basetype_settype(meosType basetype)
{
  int n = sizeof(_settype_catalog) / sizeof(settype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_settype_catalog[i].basetype == basetype)
      return _settype_catalog[i].settype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a set type", basetype);
  return T_UNKNOWN; /* make compiler quiet */
}

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

#ifdef DEBUG_BUILD
/**
 * @brief Return true if the type is a base type of one of the template types,
 * that is, Set, Span, SpanSet, and Temporal
 * @note This function is only used in the asserts
 */
bool
meos_basetype(meosType type)
{
  if (type == T_BOOL || type == T_TEXT || type == T_INT4 ||
    type == T_INT8 || type == T_FLOAT8 || type == T_TIMESTAMPTZ ||
    /* The doubleX are internal types used for temporal aggregation */
    type == T_DOUBLE2 || type == T_DOUBLE3 || type == T_DOUBLE4 ||
    type == T_GEOMETRY || type == T_GEOGRAPHY || type == T_NPOINT
    )
    return true;
  return false;
}
#endif

/**
 * @brief Return true if the values of the base type are passed by value.
 */
bool
basetype_byvalue(meosType type)
{
  assert(meos_basetype(type));
  if (type == T_BOOL || type == T_INT4 || type == T_INT8 || type == T_FLOAT8 ||
      type == T_TIMESTAMPTZ)
    return true;
  return false;
}

/**
 * @brief Return true if the values of the base type are of variable length.
 */
bool
basetype_varlength(meosType type)
{
  assert(meos_basetype(type));
  if (type == T_TEXT || type == T_GEOMETRY || type == T_GEOGRAPHY)
    return true;
  return false;
}

/**
 * @brief Return the length of a base type
 */
int16
basetype_length(meosType type)
{
  assert(meos_basetype(type));
  if (basetype_byvalue(type))
    return sizeof(Datum);
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
  elog(ERROR, "unknown base type: %d", type);
  return 0; /* make compiler quiet */
}

#ifdef DEBUG_BUILD
/**
 * @brief Return true if the type is an alphanumeric base type
 * @note This function is only used in the asserts
 */
bool
alphanum_basetype(meosType type)
{
  if (type == T_BOOL || type == T_TEXT || type == T_INT4 ||
      type == T_INT8 || type == T_FLOAT8 || type == T_TIMESTAMPTZ)
    return true;
  return false;
}
#endif

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
  if (type == T_GEOMETRY || type == T_GEOGRAPHY || type == T_NPOINT)
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

/*****************************************************************************/

#ifdef DEBUG_BUILD
/**
 * @brief Return true if the type is a base type of a set type
 * @note This function is only used in the asserts
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
#endif

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
 * @brief Return true if the type is a number set type
 */
bool
numset_type(meosType type)
{
  if (type == T_INTSET || type == T_BIGINTSET || type == T_FLOATSET)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a time set type
 */
bool
timeset_type(meosType type)
{
  if (type == T_TSTZSET)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a set type with a span as a bounding box
 */
bool
set_span_type(meosType type)
{
  if (type == T_INTSET || type == T_BIGINTSET || type == T_FLOATSET ||
    type == T_TSTZSET)
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
  if (type == T_GEOMSET || type == T_GEOGSET)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a geo set type
 */
bool
spatialset_type(meosType type)
{
  if (type == T_GEOMSET || type == T_GEOGSET || type == T_NPOINTSET)
    return true;
  return false;
}

/*****************************************************************************/

/**
 * @brief Return true if the type is a span base type
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

#ifdef DEBUG_BUILD
/**
 * @brief Return true if the type is a span type
 * @note This function is only used in the asserts
 */
bool
span_bbox_type(meosType type)
{
  if (set_span_type(type) || span_type(type) || spanset_type(type) ||
    talpha_type(type))
    return true;
  return false;
}
#endif

/**
 * @brief Return true if the type is a numeric span type
 */
bool
numspan_basetype(meosType type)
{
  if (type == T_INT4 || type == T_INT8 || type == T_FLOAT8)
    return true;
  return false;
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

/**
 * @brief Return true if the type is a numeric span type
 */
bool
timespan_basetype(meosType type)
{
  if (type == T_TIMESTAMPTZ)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a numeric span type
 */
bool
timespan_type(meosType type)
{
  if (type == T_TSTZSPAN)
    return true;
  return false;
}

/*****************************************************************************/

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

#if 0 /* not used */
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
#endif

/**
 * @brief Return true if the type is a numeric span type
 */
bool
timespanset_type(meosType type)
{
  if (type == T_TSTZSPANSET)
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
      type == T_TTEXT || type == T_TGEOMPOINT || type == T_TGEOGPOINT ||
      /* The doubleX are internal types used for temporal aggregation */
      type == T_TDOUBLE2 || type == T_TDOUBLE3 || type == T_TDOUBLE4
#if NPOINT
    || type == T_TNPOINT
#endif
    )
    return true;
  return false;
}

#ifdef DEBUG_BUILD
/**
 * @brief Return true if the type is a temporal base type
 * @note This function is only used in the asserts
 */
bool
temporal_basetype(meosType type)
{
  if (type == T_BOOL || type == T_INT4 || type == T_FLOAT8 || type == T_TEXT ||
    /* The doubleX are internal types used for temporal aggregation */
    type == T_DOUBLE2 || type == T_DOUBLE3 || type == T_DOUBLE4 ||
    type == T_GEOMETRY || type == T_GEOGRAPHY
#if NPOINT
    || type == T_NPOINT
#endif
    )
    return true;
  return false;
}
#endif

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

#ifdef DEBUG_BUILD
/**
 * @brief Return true if the type is a temporal alphanumeric type
 * @note This function is only used in the asserts
 */
bool
talphanum_type(meosType type)
{
  if (type == T_TBOOL || type == T_TINT || type == T_TFLOAT || type == T_TTEXT)
    return true;
  return false;
}
#endif

/**
 * @brief Return true if the type is a temporal alpha type (i.e., those whose
 * bounding box is a period)
 */
bool
talpha_type(meosType type)
{
  if (type == T_TBOOL || type == T_TTEXT || type == T_TDOUBLE2 ||
      type == T_TDOUBLE3 || type == T_TDOUBLE4)
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

#if MEOS
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
#endif /* MEOS */

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

/*****************************************************************************/
