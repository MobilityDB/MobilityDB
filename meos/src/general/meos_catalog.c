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
 * @brief Create a cache of metadata information about temporal types and
 * span types in global arrays
 */

#include "general/meos_catalog.h"

/* C */
#include <assert.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
/* MEOS */
#include <meos.h>
#include "general/doublen.h"
#if NPOINT
  #include "npoint/tnpoint.h"
#endif

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/**
 * @brief Global array containing the type names corresponding to the
 * enumeration meosType defined in file `meos_catalog.h`
 */
const char *_MEOSTYPE_NAMES[] =
{
  [T_UNKNOWN] = "",
  [T_BOOL] = "bool",
  [T_DATE] = "date",
  [T_DATEMULTIRANGE] = "datemultirange",
  [T_DATERANGE] = "daterange",
  [T_DATESET] = "dateset",
  [T_DATESPAN] = "datespan",
  [T_DATESPANSET] = "datespanset",
  [T_DOUBLE2] = "double2",
  [T_DOUBLE3] = "double3",
  [T_DOUBLE4] = "double4",
  [T_FLOAT8] = "float8",
  [T_FLOATSET] = "floatset",
  [T_FLOATSPAN] = "floatspan",
  [T_FLOATSPANSET] = "floatspanset",
  [T_INT4] = "int4",
  [T_INT4MULTIRANGE] = "int4multirange",
  [T_INT4RANGE] = "int4range",
  [T_INTSET] = "intset",
  [T_INTSPAN] = "intspan",
  [T_INTSPANSET] = "intspanset",
  [T_INT8] = "int8",
  [T_BIGINTSET] = "bigintset",
  [T_BIGINTSPAN] = "bigintspan",
  [T_BIGINTSPANSET] = "bigintspanset",
  [T_STBOX] = "stbox",
  [T_TBOOL] = "tbool",
  [T_TBOX] = "tbox",
  [T_TDOUBLE2] = "tdouble2",
  [T_TDOUBLE3] = "tdouble3",
  [T_TDOUBLE4] = "tdouble4",
  [T_TEXT] = "text",
  [T_TEXTSET] = "textset",
  [T_TFLOAT] = "tfloat",
  [T_TIMESTAMPTZ] = "timestamptz",
  [T_TINT] = "tint",
  [T_TSTZMULTIRANGE] = "tstzmultirange",
  [T_TSTZRANGE] = "tstzrange",
  [T_TSTZSET] = "tstzset",
  [T_TSTZSPAN] = "tstzspan",
  [T_TSTZSPANSET] = "tstzspanset",
  [T_TTEXT] = "ttext",
  [T_GEOMETRY] = "geometry",
  [T_GEOMSET] = "geomset",
  [T_GEOGRAPHY] = "geography",
  [T_GEOGSET] = "geogset",
  [T_TGEOMPOINT] = "tgeompoint",
  [T_TGEOGPOINT] = "tgeogpoint",
  [T_NPOINT] = "npoint",
  [T_NPOINTSET] = "npointset",
  [T_NSEGMENT] = "nsegment",
  [T_TNPOINT] = "tnpoint",
};

/**
 * @brief Global array containing the operator names corresponding to the
 * enumeration meosOper defined in file `meos_catalog.h`
 */
const char *_MEOSOPER_NAMES[] =
{
  [UNKNOWN_OP] = "",
  [EQ_OP] = "=",
  [NE_OP] = "<>",
  [LT_OP] = "<",
  [LE_OP] = "<=",
  [GT_OP] = ">",
  [GE_OP] = ">=",
  [ADJACENT_OP] = "-|-",
  [UNION_OP] = "+",
  [MINUS_OP] = "-",
  [INTERSECT_OP] = "*",
  [OVERLAPS_OP] = "&&",
  [CONTAINS_OP] = "@>",
  [CONTAINED_OP] = "<@",
  [SAME_OP] = "~=",
  [LEFT_OP] = "<<",
  [OVERLEFT_OP] = "&<",
  [RIGHT_OP] = ">>",
  [OVERRIGHT_OP] = "&>",
  [BELOW_OP] = "<<|",
  [OVERBELOW_OP] = "&<|",
  [ABOVE_OP] = "|>>",
  [OVERABOVE_OP] = "|&>",
  [FRONT_OP] = "<</",
  [OVERFRONT_OP] = "&</",
  [BACK_OP] = "/>>",
  [OVERBACK_OP] = "/&>",
  [BEFORE_OP] = "<<#",
  [OVERBEFORE_OP] = "&<#",
  [AFTER_OP] = "#>>",
  [OVERAFTER_OP] = "#&>",
  [EVEREQ_OP] = "?=",
  [EVERNE_OP] = "?<>",
  [EVERLT_OP] = "?<",
  [EVERLE_OP] = "?<=",
  [EVERGT_OP] = "?>",
  [EVERGE_OP] = "?>=",
  [ALWAYSEQ_OP] = "%=",
  [ALWAYSNE_OP] = "%<>",
  [ALWAYSLT_OP] = "%<",
  [ALWAYSLE_OP] = "%<=",
  [ALWAYSGT_OP] = "%>",
  [ALWAYSGE_OP] = "%>=",
};

#define TEMPSUBTYPE_STR_MAX_LEN 12

/**
 * @brief Global array storing the string representation of the concrete
 * subtypes of temporal types
 */
static char *_TEMPSUBTYPE_NAMES[] =
{
  [ANYTEMPSUBTYPE] = "Any subtype",
  [TINSTANT] = "Instant",
  [TSEQUENCE] = "Sequence",
  [TSEQUENCESET] = "SequenceSet"
};

#define INTERP_STR_MAX_LEN 8

/**

 * @brief Global array containing the interpolation names corresponding to the
 * enumeration interpType defined in file `meos_catalog.h`
 * @note The names are in lowercase since they are used in error messages
 */
char * _INTERPTYPE_NAMES[] =
{
  [INTERP_NONE] = "None",
  [DISCRETE] = "Discrete",
  [STEP] = "Step",
  [LINEAR] = "Linear"
};

/*****************************************************************************/

/**
 * @brief Global array that keeps type information for the defined set types
 */
settype_catalog_struct _SETTYPE_CATALOG[] =
{
  /* settype        basetype */
  {T_INTSET,        T_INT4},
  {T_BIGINTSET,     T_INT8},
  {T_FLOATSET,      T_FLOAT8},
  {T_DATESET,       T_DATE},
  {T_TSTZSET,       T_TIMESTAMPTZ},
  {T_TEXTSET,       T_TEXT},
  {T_GEOMSET,       T_GEOMETRY},
  {T_GEOGSET,       T_GEOGRAPHY},
  {T_NPOINTSET,     T_NPOINT},
};

/**
 * @brief Global array that keeps type information for the defined span types
 */
spantype_catalog_struct _SPANTYPE_CATALOG[] =
{
  /* spantype       basetype */
  {T_INTSPAN,       T_INT4},
  {T_BIGINTSPAN,    T_INT8},
  {T_FLOATSPAN,     T_FLOAT8},
  {T_DATESPAN,      T_DATE},
  {T_TSTZSPAN,      T_TIMESTAMPTZ},
};

/**
 * @brief Global array that keeps type information for the defined span set types
 */
spansettype_catalog_struct _SPANSETTYPE_CATALOG[] =
{
  /* spansettype    spantype */
  {T_INTSPANSET,    T_INTSPAN},
  {T_BIGINTSPANSET, T_BIGINTSPAN},
  {T_FLOATSPANSET,  T_FLOATSPAN},
  {T_DATESPANSET,   T_DATESPAN},
  {T_TSTZSPANSET,   T_TSTZSPAN},
};

/**
 * @brief Global array that keeps type information for the defined temporal types
 */
temptype_catalog_struct _TEMPTYPE_CATALOG[] =
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

/*****************************************************************************/

/**
 * @brief Return the string representation of the MEOS type
 */
const char *
meostype_name(meosType type)
{
  return _MEOSTYPE_NAMES[type];
}

/*****************************************************************************/

/**
 * @brief Return the string representation of the subtype of the temporal type
 * corresponding to the enum value
 */
const char *
tempsubtype_name(tempSubtype subtype)
{
  return _TEMPSUBTYPE_NAMES[subtype];
}

/**
 * @brief Return the enum value corresponding to the string representation
 * of the concrete subtype of a temporal type
 */
bool
tempsubtype_from_string(const char *str, int16 *subtype)
{
  char *tmpstr;
  size_t tmpstartpos, tmpendpos;
  size_t i;

  /* Initialize */
  *subtype = 0;
  /* Locate any leading/trailing spaces */
  tmpstartpos = 0;
  for (i = 0; i < strlen(str); i++)
  {
    if (str[i] != ' ')
    {
      tmpstartpos = i;
      break;
    }
  }
  tmpendpos = strlen(str) - 1;
  for (i = strlen(str) - 1; i != 0; i--)
  {
    if (str[i] != ' ')
    {
      tmpendpos = i;
      break;
    }
  }
  tmpstr = palloc(tmpendpos - tmpstartpos + 2);
  for (i = tmpstartpos; i <= tmpendpos; i++)
    tmpstr[i - tmpstartpos] = str[i];
  /* Add NULL to terminate */
  tmpstr[i - tmpstartpos] = '\0';
  size_t len = strlen(tmpstr);
  /* Now check for the type */
  size_t n = sizeof(_TEMPSUBTYPE_NAMES) / sizeof(char *);
  for (i = 0; i < n; i++)
  {
    if (len == strnlen(_TEMPSUBTYPE_NAMES[i], TEMPSUBTYPE_STR_MAX_LEN) &&
      ! pg_strncasecmp(tmpstr, _TEMPSUBTYPE_NAMES[i], TEMPSUBTYPE_STR_MAX_LEN))
    {
      *subtype = i;
      pfree(tmpstr);
      return true;
    }
  }
  pfree(tmpstr);
  return false;
}

#if DEBUG_BUILD
/**
 * @brief Ensure that the subtype of a temporal value is valid
 * @note The function is used for the dispatch functions for temporal types
 */
bool
temptype_subtype(tempSubtype subtype)
{
  if (subtype == TINSTANT || subtype == TSEQUENCE || subtype == TSEQUENCESET)
    return true;
  return false;
}

/**
 * @brief Ensure that the subtype of a temporal value is valid
 * @note The function is used for the the analyze and selectivity functions
 */
bool
temptype_subtype_all(tempSubtype subtype)
{
  if (subtype == ANYTEMPSUBTYPE ||
    subtype == TINSTANT || subtype == TSEQUENCE || subtype == TSEQUENCESET)
    return true;
  return false;
}
#endif /* DEBUG_BUILD */



/*****************************************************************************/

/**
 * @brief Return the string name from a MEOS operator number
 */
const char *
meosoper_name(meosOper oper)
{
  return _MEOSOPER_NAMES[oper];
}

/**
 * @brief Fetch the operator number from its name
 * @arg[in] str Name of the type
 */
meosOper
meosoper_from_string(const char *str)
{
  int n = sizeof(_MEOSOPER_NAMES) / sizeof(char *);
  for (int i = 0; i < n; i++)
  {
    if (strcmp(_MEOSOPER_NAMES[i], str) == 0)
      return i;
  }
  return UNKNOWN_OP;
}

/*****************************************************************************/

/**
 * @brief Return the string representation of the subtype of the temporal type
 * corresponding to the enum value
 */
const char *
interptype_name(interpType interp)
{
  return _INTERPTYPE_NAMES[interp];
}

/**
 * @brief Get the interpolation type from the interpolation string
 */
interpType
interptype_from_string(const char *str)
{
  int n = sizeof(_INTERPTYPE_NAMES) / sizeof(char *);
  for (int i = 0; i < n; i++)
  {
    if (pg_strncasecmp(str, _INTERPTYPE_NAMES[i],
      INTERP_STR_MAX_LEN) == 0)
      return i;
  }
  /* Error */
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Unknown interpolation type: %s", str);
  return INTERP_NONE; /* make compiler quiet */
}

/*****************************************************************************
 * Cache functions
 *****************************************************************************/

/**
 * @brief Return the base type from the temporal type
 */
meosType
temptype_basetype(meosType type)
{
  int n = sizeof(_TEMPTYPE_CATALOG) / sizeof(temptype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_TEMPTYPE_CATALOG[i].temptype == type)
      return _TEMPTYPE_CATALOG[i].basetype;
  }
  /* We only arrive here on error */
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "type %s is not a temporal type", meostype_name(type));
  return T_UNKNOWN;
}

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

/**
 * @brief Return the base type from a set type
 */
meosType
settype_basetype(meosType type)
{
  int n = sizeof(_SETTYPE_CATALOG) / sizeof(settype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_SETTYPE_CATALOG[i].settype == type)
      return _SETTYPE_CATALOG[i].basetype;
  }
  /* We only arrive here on error */
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "type %s is not a set type", meostype_name(type));
  return T_UNKNOWN;
}

/**
 * @brief Return the base type from the set type
 */
meosType
basetype_settype(meosType type)
{
  int n = sizeof(_SETTYPE_CATALOG) / sizeof(settype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_SETTYPE_CATALOG[i].basetype == type)
      return _SETTYPE_CATALOG[i].settype;
  }
  /* We only arrive here on error */
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "type %s is not a set type", meostype_name(type));
  return T_UNKNOWN;
}

/**
 * @brief Return the base type from the span type
 */
meosType
spantype_basetype(meosType type)
{
  int n = sizeof(_SPANTYPE_CATALOG) / sizeof(spantype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_SPANTYPE_CATALOG[i].spantype == type)
      return _SPANTYPE_CATALOG[i].basetype;
  }
  /* We only arrive here on error */
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "type %s is not a span type", meostype_name(type));
  return T_UNKNOWN;
}

/**
 * @brief Return the span type from the span set type
 */
meosType
spansettype_spantype(meosType type)
{
  int n = sizeof(_SPANSETTYPE_CATALOG) / sizeof(spansettype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_SPANSETTYPE_CATALOG[i].spansettype == type)
      return _SPANSETTYPE_CATALOG[i].spantype;
  }
  /* We only arrive here on error */
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "type %s is not a span set type", meostype_name(type));
  return T_UNKNOWN;
}

/**
 * @brief Return the span type of a base type
 */
meosType
basetype_spantype(meosType type)
{
  int n = sizeof(_SPANTYPE_CATALOG) / sizeof(spantype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_SPANTYPE_CATALOG[i].basetype == type)
      return _SPANTYPE_CATALOG[i].spantype;
  }
  /* We only arrive here on error */
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "type %s is not a span type", meostype_name(type));
  return T_UNKNOWN;
}

/**
 * @brief Return the span type from the span set type
 */
meosType
spantype_spansettype(meosType type)
{
  int n = sizeof(_SPANSETTYPE_CATALOG) / sizeof(spansettype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (_SPANSETTYPE_CATALOG[i].spantype == type)
      return _SPANSETTYPE_CATALOG[i].spansettype;
  }
  /* We only arrive here on error */
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "type %s is not a span type", meostype_name(type));
  return T_UNKNOWN;
}

/*****************************************************************************/

#if 0 /* not used */
/**
 * @brief Determine whether the type is an internal MobilityDB type
 */
bool
meostype_internal(meosType type)
{
  if (type == T_DOUBLE2 || type == T_DOUBLE3 || type == T_DOUBLE4 ||
      type == T_TDOUBLE2 || type == T_TDOUBLE3 || type == T_TDOUBLE4)
    return true;
  return false;
}
#endif /* not used */

#ifdef DEBUG_BUILD
/**
 * @brief Return true if the type is a base type of one of the template types,
 * that is, @p Set, @p Span, @p SpanSet, and @p Temporal
 * @note This function is only used in the asserts
 */
bool
meos_basetype(meosType type)
{
  if (type == T_BOOL || type == T_INT4 || type == T_INT8 || type == T_FLOAT8 ||
    type == T_TEXT || type == T_DATE || type == T_TIMESTAMPTZ ||
    /* The doubleX are internal types used for temporal aggregation */
    type == T_DOUBLE2 || type == T_DOUBLE3 || type == T_DOUBLE4 ||
    type == T_GEOMETRY || type == T_GEOGRAPHY || type == T_NPOINT
    )
    return true;
  return false;
}
#endif

/**
 * @brief Return true if the values of the base type are passed by value
 */
bool
basetype_byvalue(meosType type)
{
  assert(meos_basetype(type));
  if (type == T_BOOL || type == T_INT4 || type == T_INT8 || type == T_FLOAT8 ||
      type == T_DATE || type == T_TIMESTAMPTZ)
    return true;
  return false;
}

/**
 * @brief Return true if the values of the base type are of variable length
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
 * @return On error return SHRT_MAX
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
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "Unknown base type: %s", meostype_name(type));
  return SHRT_MAX;
}

#ifdef DEBUG_BUILD
/**
 * @brief Return true if the type is an alphanumeric base type
 * @note This function is only used in the asserts
 */
bool
alphanum_basetype(meosType type)
{
  if (type == T_BOOL || type == T_INT4 || type == T_INT8 || type == T_FLOAT8 ||
      type == T_TEXT || type == T_DATE || type == T_TIMESTAMPTZ)
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
  if (type == T_DATE || type == T_DATESET || type == T_DATESPAN ||
      type == T_DATESPANSET || type == T_TIMESTAMPTZ || type == T_TSTZSET ||
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
  if (type == T_TIMESTAMPTZ || type == T_DATE || type == T_INT4 ||
      type == T_INT8 || type == T_FLOAT8 || type == T_TEXT ||
      type == T_GEOMETRY || type == T_GEOGRAPHY || type == T_NPOINT)
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
  if (type == T_TSTZSET || type == T_DATESET || type == T_INTSET ||
      type == T_BIGINTSET || type == T_FLOATSET || type == T_TEXTSET ||
      type == T_GEOMSET || type == T_GEOGSET || type == T_NPOINTSET)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a number set type
 */
bool
numset_type(meosType type)
{
  if (type == T_INTSET || type == T_BIGINTSET || type == T_FLOATSET ||
      /* Dates are represented as integers */
      type == T_DATESET)
    return true;
  return false;
}

/**
 * @brief Ensure that the type is a number set type
 */
bool
ensure_numset_type(meosType type)
{
  if (! numset_type(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The set value must be a number or a date set");
    return false;
  }
  return true;
}

/**
 * @brief Return true if the type is a time set type
 */
bool
timeset_type(meosType type)
{
  if (type == T_DATESET || type == T_TSTZSET)
    return true;
  return false;
}

#if 0 /* not used */
/**
 * @brief Ensure that the type is a number set type
 */
bool
ensure_timeset_type(meosType type)
{
  if (! timeset_type(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The set value must be a time set");
    return false;
  }
  return true;
}
#endif /* not used */

/**
 * @brief Return true if the type is a set type with a span as a bounding box
 */
bool
set_spantype(meosType type)
{
  if (type == T_INTSET || type == T_BIGINTSET || type == T_FLOATSET ||
    type == T_DATESET || type == T_TSTZSET)
    return true;
  return false;
}

/**
 * @brief Ensure that a set value is a set type with a span as a bounding box
 */
bool
ensure_set_spantype(meosType type)
{
  if (! set_spantype(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The set value must be a number or timestamp set");
    return false;
  }
  return true;
}

/**
 * @brief Return true if the type is a set type
 */
bool
alphanumset_type(meosType type)
{
  if (type == T_TSTZSET || type == T_DATESET || type == T_INTSET ||
      type == T_BIGINTSET || type == T_FLOATSET || type == T_TEXTSET)
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
 * @brief Ensure that a set value is a geo set
 */
bool
ensure_geoset_type(meosType type)
{
  if (! geoset_type(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The set value must be a geo set");
    return false;
  }
  return true;
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

/**
 * @brief Ensure that a temporal value is a temporal number
 */
bool
ensure_spatialset_type(meosType type)
{
  if (! spatialset_type(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The set value must be a spatial set");
    return false;
  }
  return true;
}

/*****************************************************************************/

/**
 * @brief Return true if the type is a span base type
 */
bool
span_basetype(meosType type)
{
  if (type == T_TIMESTAMPTZ || type == T_DATE || type == T_INT4 ||
      type == T_INT8 || type == T_FLOAT8)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a canonical base type of a span type
 */
bool
span_canon_basetype(meosType type)
{
  if (type == T_DATE || type == T_INT4 || type == T_INT8)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a span type
 */
bool
span_type(meosType type)
{
  if (type == T_TSTZSPAN || type == T_DATESPAN || type == T_INTSPAN ||
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
  if (set_spantype(type) || span_type(type) || spanset_type(type) ||
    talpha_type(type))
    return true;
  return false;
}
#endif

/**
 * @brief Return true if the type is a number span type
 */
bool
numspan_basetype(meosType type)
{
  if (type == T_INT4 || type == T_INT8 || type == T_FLOAT8 ||
      /* Dates are represented as integers */
      type == T_DATE)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a number span type
 */
bool
numspan_type(meosType type)
{
  if (type == T_INTSPAN || type == T_BIGINTSPAN || type == T_FLOATSPAN)
    return true;
  return false;
}

/**
 * @brief Ensure that a span is a number span type
 */
bool
ensure_numspan_type(meosType type)
{
  if (! numspan_type(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The span value must be a number span type");
    return false;
  }
  return true;
}

/**
 * @brief Return true if the type is a time span type
 */
bool
timespan_basetype(meosType type)
{
  if (type == T_TIMESTAMPTZ || type == T_DATE)
    return true;
  return false;
}

/**
 * @brief Return true if the type is a time span type
 */
bool
timespan_type(meosType type)
{
  if (type == T_TSTZSPAN || type == T_DATESPAN)
    return true;
  return false;
}

#if 0 /* not used */
/**
 * @brief Ensure that a span is a time span type
 */
bool
ensure_timespan_type(meosType type)
{
  if (! timespan_type(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The span value must be a time span type");
    return false;
  }
  return true;
}
#endif /* not used */

/*****************************************************************************/

/**
 * @brief Return true if the type is a span set type
 */
bool
spanset_type(meosType type)
{
  if (type == T_TSTZSPANSET || type == T_DATESPANSET || type == T_INTSPANSET ||
      type == T_BIGINTSPANSET || type == T_FLOATSPANSET)
    return true;
  return false;
}

#if 0 /* not used */
/**
 * @brief Return true if the type is a number span type
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
 * @brief Return true if the type is a time span type
 */
bool
timespanset_type(meosType type)
{
  if (type == T_TSTZSPANSET || type == T_DATESPANSET)
    return true;
  return false;
}

/**
 * @brief Ensure that a span is a time span type
 */
bool
ensure_timespanset_type(meosType type)
{
  if (! timespanset_type(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The value must be a time span set type");
    return false;
  }
  return true;
}

/*****************************************************************************/

/**
 * @brief Return true if the type is an @b external temporal type
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
 * bounding box is a timestamptz span)
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
 * @brief Ensure that a type is a temporal number
 */
bool
ensure_tnumber_type(meosType type)
{
  if (! tnumber_type(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The temporal value must be a temporal number");
    return false;
  }
  return true;
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
 * @brief Ensure that a type is a temporal number base type
 */
bool
ensure_tnumber_basetype(meosType type)
{
  if (! tnumber_basetype(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The temporal value must be a base value of temporal number");
    return false;
  }
  return true;
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
 * @details This function is used for features common to all spatiotemporal 
 * types, in particular, all of them use the same bounding box @ STBox.
 * Therefore, it is used for the indexes and selectivity functions.
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
 * @brief Ensure that a temporal value is a temporal point or network point
 */
bool
ensure_tspatial_type(meosType type)
{
  if (! tspatial_type(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The temporal value must be a temporal point type");
    return false;
  }
  return true;
}


/**
 * @brief Return true if the type is a base type of a spatiotemporal type
 * @details This function is used for features common to all spatiotemporal 
 * types, in particular, all of them use the same bounding box @p STBox
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
 * @brief Ensure that a temporal value is a temporal point type
 */
bool
ensure_tgeo_type(meosType type)
{
  if (! tgeo_type(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The temporal value must be a temporal point type");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal value is a temporal number or a temporal point
 * type
 */
bool
ensure_tnumber_tgeo_type(meosType type)
{
  if (! tnumber_type(type) && ! tgeo_type(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The temporal value must be a temporal number or a temporal point type");
    return false;
  }
  return true;
}

/*****************************************************************************/
