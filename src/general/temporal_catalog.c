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
 * @file temporal_catalog.c
 * @brief Create a cache of information about temporal types and PostgreSQL
 * OIDs in global arrays in order to avoid (slow) lookups. These arrays are
 * initialized at the loading of the extension.
 *
 * The selectivity of Boolean operators is essential to determine efficient
 * execution plans for queries. The temporal extension defines several classes
 * of Boolean operators (equal, less than, overlaps, ...), currently 30, each
 * of which can have as left or right arguments a built-in type (such as
 * integer, timestamptz, box, geometry, ...) or a newly defined type (such as
 * period, tint, ...), currently 33.
 *
 * There are currently 3,392 operators, each of which is identified by an Oid.
 * To avoid enumerating all of these operators in the Oid cache, we use a
 * two-dimensional array containing all possible combinations of
 * operator/left argument/right argument (currently 23 * 40 * 40 = 36,800 cells).
 * The invalid combinations will be initialized to 0.
 */

#include "general/temporal_catalog.h"

/* PostgreSQL */
#include <postgres.h>
/* MobilityDB */
#include "general/temporaltypes.h"
#if ! MEOS
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
#if ! MEOS
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
CachedType
temptype_basetype(CachedType temptype)
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
CachedType
spantype_basetype(CachedType spantype)
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
CachedType
basetype_spantype(CachedType basetype)
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
time_type(CachedType timetype)
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
ensure_time_type(CachedType timetype)
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
span_type(CachedType spantype)
{
  if (spantype == T_PERIOD || spantype == T_INTSPAN || spantype == T_FLOATSPAN)
    return true;
  return false;
}

/**
 * Ensure that the type corresponds to a time type
 */
void
ensure_span_type(CachedType spantype)
{
  if (! span_type(spantype))
    elog(ERROR, "unknown span type: %d", spantype);
  return;
}

/**
 * Ensures that the base type is supported by MobilityDB
 */
void
ensure_span_basetype(CachedType basetype)
{
  if (basetype != T_TIMESTAMPTZ && basetype != T_INT4 && basetype != T_FLOAT8)
    elog(ERROR, "unknown span base type: %d", basetype);
  return;
}

/*****************************************************************************/

/**
 * Return true if the temporal type is an EXTERNAL temporal type
 *
 * @note Function used in particular in the indexes
 */
bool
temporal_type(CachedType temptype)
{
  if (temptype == T_TBOOL || temptype == T_TINT || temptype == T_TFLOAT ||
    temptype == T_TTEXT || temptype == T_TGEOMPOINT || temptype == T_TGEOGPOINT
#if ! MEOS
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
ensure_temporal_type(CachedType temptype)
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
ensure_temporal_basetype(CachedType basetype)
{
  if (basetype != T_TIMESTAMPTZ &&
    basetype != T_BOOL && basetype != T_INT4 && basetype != T_INT8 &&
    basetype != T_FLOAT8 && basetype != T_TEXT &&
    basetype != T_DOUBLE2 && basetype != T_DOUBLE3 && basetype != T_DOUBLE4 &&
    basetype != T_GEOMETRY && basetype != T_GEOGRAPHY
#if ! MEOS
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
temptype_continuous(CachedType temptype)
{
  if (temptype == T_TFLOAT || temptype == T_TDOUBLE2 ||
    temptype == T_TDOUBLE3 || temptype == T_TDOUBLE4 ||
    temptype == T_TGEOMPOINT || temptype == T_TGEOGPOINT
#if ! MEOS
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
ensure_temptype_continuous(CachedType temptype)
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
basetype_byvalue(CachedType basetype)
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
basetype_length(CachedType basetype)
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
#if ! MEOS
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
talpha_type(CachedType temptype)
{
  if (temptype == T_TBOOL || temptype == T_TTEXT)
    return true;
  return false;
}

/**
 * Return true if the type is a temporal number type
 */
bool
tnumber_type(CachedType temptype)
{
  if (temptype == T_TINT || temptype == T_TFLOAT)
    return true;
  return false;
}

/**
 * Return true if the type is a number base type supported by MobilityDB
 */
void
ensure_tnumber_type(CachedType temptype)
{
  if (! tnumber_type(temptype))
    elog(ERROR, "unknown temporal number type: %d", temptype);
  return;
}

/**
 * Test whether the type is a number base type supported by MobilityDB
 */
bool
tnumber_basetype(CachedType basetype)
{
  if (basetype == T_INT4 || basetype == T_FLOAT8)
    return true;
  return false;
}

/**
 * Return true if the type is a number base type supported by MobilityDB
 */
void
ensure_tnumber_basetype(CachedType basetype)
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
tnumber_spantype(CachedType spantype)
{
  if (spantype == T_INTSPAN || spantype == T_FLOATSPAN)
    return true;
  return false;
}

/**
 * Ensure that the type is a span type
 */
void
ensure_tnumber_spantype(CachedType spantype)
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
tspatial_type(CachedType temptype)
{
  if (temptype == T_TGEOMPOINT || temptype == T_TGEOGPOINT
#if ! MEOS
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
tspatial_basetype(CachedType basetype)
{
  if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY
#if ! MEOS
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
tgeo_basetype(CachedType basetype)
{
  if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY)
    return true;
  return false;
}

/**
 * Return true if the type is a temporal point type supported by MobilityDB
 */
bool
tgeo_type(CachedType temptype)
{
  if (temptype == T_TGEOMPOINT || temptype == T_TGEOGPOINT)
    return true;
  return false;
}

/**
 * Ensure that the type is a point base type supported by MobilityDB
 */
void
ensure_tgeo_type(CachedType temptype)
{
  if (! tgeo_type(temptype))
    elog(ERROR, "unknown geospatial temporal type: %d", temptype);
  return;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

#include <access/heapam.h>
#include <access/htup_details.h>
#if POSTGRESQL_VERSION_NUMBER >= 120000
  #include <access/tableam.h>
#endif
#include <catalog/namespace.h>
#include <utils/rel.h>

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/**
 * Global array for caching the names of the types used in MobilityDB
 * to avoid (slow) lookups. The array is initialized at the loading of
 * the extension.
 */
const char *_type_names[] =
{
  [T_BOOL] = "bool",
  [T_DOUBLE2] = "double2",
  [T_DOUBLE3] = "double3",
  [T_DOUBLE4] = "double4",
  [T_FLOAT8] = "float8",
  [T_FLOATSPAN] = "floatspan",
  [T_INT4] = "int4",
  [T_INT4RANGE] = "int4range",
  [T_INTSPAN] = "intspan",
  [T_INT8] = "int8",
  [T_PERIOD] = "period",
  [T_PERIODSET] = "periodset",
  [T_STBOX] = "stbox",
  [T_TBOOL] = "tbool",
  [T_TBOX] = "tbox",
  [T_TDOUBLE2] = "tdouble2",
  [T_TDOUBLE3] = "tdouble3",
  [T_TDOUBLE4] = "tdouble4",
  [T_TEXT] = "text",
  [T_TFLOAT] = "tfloat",
  [T_TIMESTAMPSET] = "timestampset",
  [T_TIMESTAMPTZ] = "timestamptz",
  [T_TINT] = "tint",
  [T_TSTZRANGE] = "tstzrange",
  [T_TTEXT] = "ttext",
  [T_GEOMETRY] = "geometry",
  [T_GEOGRAPHY] = "geography",
  [T_TGEOMPOINT] = "tgeompoint",
  [T_TGEOGPOINT] = "tgeogpoint",
  [T_NPOINT] = "npoint",
  [T_NSEGMENT] = "nsegment",
  [T_TNPOINT] = "tnpoint"
};

/**
 * Global array for caching the names of the operators used in MobilityDB
 * to avoid (slow) lookups. The array is initialized at the loading of
 * the extension.
 */
const char *_op_names[] =
{
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
  [OVERAFTER_OP] = "#&>"
};

/**
 * Global variable that states whether the type and operator Oid caches
 * has been initialized.
 */
bool _oid_cache_ready = false;

/**
 * Global array that keeps the Oids of the types used in MobilityDB.
 */
Oid _type_oids[sizeof(_type_names) / sizeof(char *)];

/**
 * Global 3-dimensional array that keeps the Oids of the operators
 * used in MobilityDB. The first dimension corresponds to the operator
 * class (e.g., <=), the second and third dimensions correspond,
 * respectively, to the left and right arguments of the operator.
 * A value 0 is stored in the cell of the array if the operator class
 * is not defined for the left and right types.
 */
Oid _op_oids[sizeof(_op_names) / sizeof(char *)]
  [sizeof(_type_names) / sizeof(char *)]
  [sizeof(_type_names) / sizeof(char *)];

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

static bool
internal_type(const char *typname)
{
#if POSTGRESQL_VERSION_NUMBER >= 140000
  if (strncmp(typname, "double", 6) == 0 || strncmp(typname, "tdouble", 7) == 0)
    return true;
#endif
  return false;
}

/**
 * Populate the Oid cache for types
 */
static void
populate_typeoid_cache()
{
  int n = sizeof(_type_names) / sizeof(char *);
  for (int i = 0; i < n; i++)
  {
    if (! internal_type(_type_names[i]))
    {
      _type_oids[i] = TypenameGetTypid(_type_names[i]);
      if (!_type_oids[i])
        ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
          errmsg("No Oid for type %s", _type_names[i])));
    }
  }
  return;
}

/**
 * Populate the Oid cache for operators
 */
static void
populate_operoid_cache()
{
  // elog(NOTICE, "populate operators");
  Oid namespaceId = LookupNamespaceNoError("public") ;
  OverrideSearchPath* overridePath = GetOverrideSearchPath(CurrentMemoryContext);
  overridePath->schemas = lcons_oid(namespaceId, overridePath->schemas);
  PushOverrideSearchPath(overridePath);

  PG_TRY();
  {
    populate_typeoid_cache();
    memset(_op_oids, 0, sizeof(_op_oids));
    /*
     * This fetches the pre-computed operator cache from the catalog where
     * it is stored in a table. See the fill_opcache function below.
     */
    Oid catalog = RelnameGetRelid("mobilitydb_opcache");
#if POSTGRESQL_VERSION_NUMBER < 130000
    Relation rel = heap_open(catalog, AccessShareLock);
#else
    Relation rel = table_open(catalog, AccessShareLock);
#endif
    TupleDesc tupDesc = rel->rd_att;
    ScanKeyData scandata;
#if POSTGRESQL_VERSION_NUMBER >= 120000
    TableScanDesc scan = table_beginscan_catalog(rel, 0, &scandata);
#else
    HeapScanDesc scan = heap_beginscan_catalog(rel, 0, &scandata);
#endif
    HeapTuple tuple = heap_getnext(scan, ForwardScanDirection);
    while (HeapTupleIsValid(tuple))
    {
      bool isnull = false;
      int32 i = DatumGetInt32(heap_getattr(tuple, 1, tupDesc, &isnull));
      int32 j = DatumGetInt32(heap_getattr(tuple, 2, tupDesc, &isnull));
      int32 k = DatumGetInt32(heap_getattr(tuple, 3, tupDesc, &isnull));
      _op_oids[i][j][k] = DatumGetObjectId(heap_getattr(tuple, 4, tupDesc, &isnull));
      tuple = heap_getnext(scan, ForwardScanDirection);
    }
    heap_endscan(scan);
#if POSTGRESQL_VERSION_NUMBER < 130000
    heap_close(rel, AccessShareLock);
#else
    table_close(rel, AccessShareLock);
#endif
    PopOverrideSearchPath();
    _oid_cache_ready = true;
  }
  PG_CATCH();
  {
    PopOverrideSearchPath() ;
    PG_RE_THROW();
  }
  PG_END_TRY();
}

PG_FUNCTION_INFO_V1(fill_opcache);
/**
 * Function executed during the `CREATE EXTENSION` to precompute the
 * operator cache and store it as a table in the catalog
 */
PGDLLEXPORT Datum
fill_opcache(PG_FUNCTION_ARGS __attribute__((unused)))
{
  Oid catalog = RelnameGetRelid("mobilitydb_opcache");
#if POSTGRESQL_VERSION_NUMBER < 130000
  Relation rel = heap_open(catalog, AccessExclusiveLock);
#else
  Relation rel = table_open(catalog, AccessExclusiveLock);
#endif
  TupleDesc tupDesc = rel->rd_att;

  bool isnull[] = {false, false, false, false};
  Datum data[] = {0, 0, 0, 0};

  populate_typeoid_cache();
  int32 m = sizeof(_op_names) / sizeof(char *);
  int32 n = sizeof(_type_names) / sizeof(char *);
  for (int32 i = 0; i < m; i++)
  {
    List* lst = list_make1(makeString((char *) _op_names[i]));
    for (int32 j = 0; j < n; j++)
      for (int32 k = 0; k < n; k++)
      {
        data[3] = ObjectIdGetDatum(OpernameGetOprid(lst, _type_oids[j], _type_oids[k]));
        if (data[3] != InvalidOid)
        {
          data[0] = Int32GetDatum(i);
          data[1] = Int32GetDatum(j);
          data[2] = Int32GetDatum(k);
          HeapTuple t = heap_form_tuple(tupDesc, data, isnull);
          simple_heap_insert(rel, t);
        }
      }
    pfree(lst);
  }
#if POSTGRESQL_VERSION_NUMBER < 130000
  heap_close(rel, AccessExclusiveLock);
#else
  table_close(rel, AccessExclusiveLock);
#endif
  PG_RETURN_VOID();
}

/**
 * Fetch from the cache the Oid of a type
 *
 * @arg[in] type Enum value for the type
 */
Oid
type_oid(CachedType type)
{
  if (!_oid_cache_ready)
    populate_operoid_cache();
  return _type_oids[type];
}

/**
 * Fetch from the cache the Oid of an operator
 *
 * @arg[in] oper Enum value for the operator
 * @arg[in] lt Enum value for the left type
 * @arg[in] rt Enum value for the right type
 */
Oid
oper_oid(CachedOp oper, CachedType lt, CachedType rt)
{
  if (!_oid_cache_ready)
    populate_operoid_cache();
  return _op_oids[oper][lt][rt];
}

/**
 * Fetch from the cache the Oid of a type
 *
 * @arg[in] type Enum value for the type
 */
CachedType
oid_type(Oid typid)
{
  if (!_oid_cache_ready)
    populate_operoid_cache();
  int n = sizeof(_type_names) / sizeof(char *);
  for (int i = 0; i < n; i++)
  {
    if (_type_oids[i] == typid)
      return i;
  }
  ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
    errmsg("Unknown type Oid %d", typid)));
}

#endif /* #if ! MEOS */

/*****************************************************************************/
