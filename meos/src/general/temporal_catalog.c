/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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

#include "general/temporal_catalog.h"

/* PostgreSQL */
/* MobilityDB */
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
 * Global array that keeps type information for the span types defined
 * in MobilityDB.
 */
spantype_cache_struct _spantype_cache[] =
{
  /* spantype       basetype */
  {T_INTSPAN,       T_INT4},
  {T_FLOATSPAN,     T_FLOAT8},
  {T_PERIOD,        T_TIMESTAMPTZ},
};

/*****************************************************************************
 * Cache functions
 *****************************************************************************/

/**
 * Return the base type from the temporal type
 * @note this function is defined again for MobilityDB below
 */
mobdbType
temptype_basetype(mobdbType temptype)
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
mobdbType
spantype_basetype(mobdbType spantype)
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
 * Return the base type from the span type
 */
mobdbType
basetype_spantype(mobdbType basetype)
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

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

/**
 * Return true if the type is a time type
 */
bool
time_type(mobdbType timetype)
{
  if (timetype == T_TIMESTAMPTZ || timetype == T_TIMESTAMPSET ||
    timetype == T_PERIOD || timetype == T_PERIODSET)
    return true;
  return false;
}

/**
 * Ensure that the type corresponds to a time type
 */
void
ensure_time_type(mobdbType timetype)
{
  if (! time_type(timetype))
    elog(ERROR, "unknown time type: %d", timetype);
  return;
}

/*****************************************************************************/

/**
 * Return true if the type is a time type
 */
bool
span_type(mobdbType spantype)
{
  if (spantype == T_PERIOD || spantype == T_INTSPAN || spantype == T_FLOATSPAN)
    return true;
  return false;
}

/**
 * Ensure that the type corresponds to a span type
 */
void
ensure_span_type(mobdbType spantype)
{
  if (! span_type(spantype))
    elog(ERROR, "unknown span type: %d", spantype);
  return;
}

/**
 * Return true if the type is a time type
 */
bool
span_basetype(mobdbType basetype)
{
  if (basetype == T_TIMESTAMPTZ || basetype == T_INT4 || basetype == T_FLOAT8)
    return true;
  return false;
}

/**
 * Ensure that the span base type is supported by MobilityDB
 */
void
ensure_span_basetype(mobdbType basetype)
{
  if (! span_basetype(basetype))
    elog(ERROR, "unknown span base type: %d", basetype);
  return;
}

/*****************************************************************************/

/**
 * @brief Return true if the temporal type is an EXTERNAL temporal type
 *
 * @note Function used in particular in the indexes
 */
bool
temporal_type(mobdbType temptype)
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
 * Ensure that the base type is supported by MobilityDB
 */
void
ensure_temporal_type(mobdbType temptype)
{
  if (! temporal_type(temptype))
    elog(ERROR, "unknown temporal type: %d", temptype);
  return;
}

/**
 * Ensure that the base type is supported by MobilityDB
 * @note The TimestampTz type is added to cope with base types for spans.
 * Also, the int8 type is added to cope with the rid in network points.
 */
void
ensure_temporal_basetype(mobdbType basetype)
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
temptype_continuous(mobdbType temptype)
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
ensure_temptype_continuous(mobdbType temptype)
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
basetype_byvalue(mobdbType basetype)
{
  ensure_temporal_basetype(basetype);
  if (basetype == T_BOOL || basetype == T_INT4 || basetype == T_FLOAT8)
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
basetype_length(mobdbType basetype)
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
talpha_type(mobdbType temptype)
{
  if (temptype == T_TBOOL || temptype == T_TTEXT)
    return true;
  return false;
}

/**
 * Return true if the type is a temporal number type
 */
bool
tnumber_type(mobdbType temptype)
{
  if (temptype == T_TINT || temptype == T_TFLOAT)
    return true;
  return false;
}

/**
 * Return true if the type is a number base type supported by MobilityDB
 */
void
ensure_tnumber_type(mobdbType temptype)
{
  if (! tnumber_type(temptype))
    elog(ERROR, "unknown temporal number type: %d", temptype);
  return;
}

/**
 * Test whether the type is a number base type supported by MobilityDB
 */
bool
tnumber_basetype(mobdbType basetype)
{
  if (basetype == T_INT4 || basetype == T_FLOAT8)
    return true;
  return false;
}

/**
 * Return true if the type is a number base type supported by MobilityDB
 */
void
ensure_tnumber_basetype(mobdbType basetype)
{
  if (! tnumber_basetype(basetype))
    elog(ERROR, "unknown number base type: %d", basetype);
  return;
}

/**
 * Return true if the type is a span number type
 *
 * @note Function used in particular in the indexes
 */
bool
tnumber_spantype(mobdbType spantype)
{
  if (spantype == T_INTSPAN || spantype == T_FLOATSPAN)
    return true;
  return false;
}

/**
 * Ensure that the type is a span type
 */
void
ensure_tnumber_spantype(mobdbType spantype)
{
  if (! tnumber_spantype(spantype))
    elog(ERROR, "unknown number span type: %d", spantype);
  return;
}

/**
 * Return true if the type is a spatiotemporal type
 *
 * @note This function is used for features common to all spatiotemporal types,
 * in particular, all of them use the same bounding box STBOX. Therefore it is
 * used for the indexes and selectivity functions
 */
bool
tspatial_type(mobdbType temptype)
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
 * in particular, all of them use the same bounding box STBOX
 */
bool
tspatial_basetype(mobdbType basetype)
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
 * Return true if the type is a point base type supported by MobilityDB
 */
bool
tgeo_basetype(mobdbType basetype)
{
  if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY)
    return true;
  return false;
}

/**
 * Return true if the type is a temporal point type supported by MobilityDB
 */
bool
tgeo_type(mobdbType temptype)
{
  if (temptype == T_TGEOMPOINT || temptype == T_TGEOGPOINT)
    return true;
  return false;
}

/**
 * Ensure that the type is a point base type supported by MobilityDB
 */
void
ensure_tgeo_type(mobdbType temptype)
{
  if (! tgeo_type(temptype))
    elog(ERROR, "unknown geospatial temporal type: %d", temptype);
  return;
}

/*****************************************************************************/
