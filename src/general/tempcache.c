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
 * @file tempcache.c
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

#include "general/tempcache.h"

/* PostgreSQL */
#include <access/heapam.h>
#include <access/htup_details.h>
#if POSTGRESQL_VERSION_NUMBER >= 120000
#include <access/tableam.h>
#endif
#include <catalog/namespace.h>
#include <utils/builtins.h>
#include <utils/rel.h>
/* MobilityDB */
#include "general/temporaltypes.h"

/*****************************************************************************/

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
  [T_FLOATRANGE] = "floatrange",
  [T_INT4] = "int4",
  [T_INTRANGE] = "intrange",
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

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/**
 * Global variable that states whether the temporal type cache has been
 * initialized.
 */
bool _temptype_cache_ready = false;

/**
 * Global array that keeps type information for the temporal types defined
 * in MobilityDB.
 */
static temptype_cache_struct _temptype_cache[TEMPTYPE_CACHE_MAX_LEN];

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
 * Functions populating the Oid cache
 *****************************************************************************/

/**
 * Fetch from the cache the enum value of a type name
 *
 * @arg[in] typname Type name
 * @note This function is used for populating the temptype cache
 */
static CachedType
typname_type(char *typname)
{
  int n = sizeof(_type_names) / sizeof(char *);
  for (int i = 0; i < n; i++)
  {
    if (strcmp(_type_names[i], typname) == 0)
      return i;
  }
  ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
    errmsg("Unknown type name %s", typname)));
}

/**
 * Populate the Oid cache for temporal types
 */
static void
populate_temptype_cache()
{
  // elog(NOTICE, "populate temptype cache");
  Oid namespaceId = LookupNamespaceNoError("public") ;
  OverrideSearchPath* overridePath = GetOverrideSearchPath(CurrentMemoryContext);
  overridePath->schemas = lcons_oid(namespaceId, overridePath->schemas);
  PushOverrideSearchPath(overridePath);

  PG_TRY();
  {
    memset(_temptype_cache, 0, sizeof(_temptype_cache));
    /*
     * This fetches the pre-computed temporal type cache from the catalog
     * where it is stored in a table.
     */
    Oid catalog = RelnameGetRelid("mobdb_temptype");
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
    int i = 0;
    while (HeapTupleIsValid(tuple))
    {
      bool isnull = false;
      /* All the following attributes are declared as NOT NULL in the table */
      _temptype_cache[i].temptypid = DatumGetObjectId(heap_getattr(tuple, 1, tupDesc, &isnull));
      _temptype_cache[i].temptypname = text_to_cstring(DatumGetTextP(heap_getattr(tuple, 2, tupDesc, &isnull)));
      _temptype_cache[i].temptype = typname_type(_temptype_cache[i].temptypname);
      _temptype_cache[i].basetypid = DatumGetObjectId(heap_getattr(tuple, 3, tupDesc, &isnull));
      _temptype_cache[i].basetypname = text_to_cstring(DatumGetTextP(heap_getattr(tuple, 4, tupDesc, &isnull)));
      _temptype_cache[i].basetype = typname_type(_temptype_cache[i].basetypname);
      _temptype_cache[i].basetyplen = DatumGetInt16(heap_getattr(tuple, 5, tupDesc, &isnull));
      _temptype_cache[i].basebyval = DatumGetObjectId(heap_getattr(tuple, 6, tupDesc, &isnull));
      _temptype_cache[i].basecont = DatumGetObjectId(heap_getattr(tuple, 7, tupDesc, &isnull));
      /* The box type attributes may be null or zero for internal types such as doubleN */
      _temptype_cache[i].boxtypid = InvalidOid;
      _temptype_cache[i].boxtypname = "";
      _temptype_cache[i].boxtyplen = 0;
      _temptype_cache[i].boxtypid = DatumGetObjectId(heap_getattr(tuple, 8, tupDesc, &isnull));
      if (! isnull)
      {
        _temptype_cache[i].boxtypname = text_to_cstring(DatumGetTextP(heap_getattr(tuple, 9, tupDesc, &isnull)));
        _temptype_cache[i].boxtyplen = DatumGetInt16(heap_getattr(tuple, 10, tupDesc, &isnull));
      }
      i++;
      if (i == TEMPTYPE_CACHE_MAX_LEN)
        elog(ERROR, "Cache for temporal types consumed, consider increasing TEMPORAL_TYPE_CACHE_MAX_LEN");
      tuple = heap_getnext(scan, ForwardScanDirection);
    }
    heap_endscan(scan);
#if POSTGRESQL_VERSION_NUMBER < 130000
    heap_close(rel, AccessShareLock);
#else
    table_close(rel, AccessShareLock);
#endif
    PopOverrideSearchPath();
    _temptype_cache_ready = true;
  }
  PG_CATCH();
  {
    PopOverrideSearchPath() ;
    PG_RE_THROW();
  }
  PG_END_TRY();
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
    _type_oids[i] = TypenameGetTypid(_type_names[i]);
    if (!_type_oids[i])
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("No Oid for type %s", _type_names[i])));
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

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

/**
 * Return the Oid of the base type from the temporal type
 */
Oid
temptype_basetypid(CachedType temptype)
{
  if (!_temptype_cache_ready)
    populate_temptype_cache();
  for (int i = 0; i < TEMPTYPE_CACHE_MAX_LEN; i++)
  {
    if (_temptype_cache[i].temptype == temptype)
      return _temptype_cache[i].basetypid;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a temporal type", temptype);
}

/**
 * Return the Oid of the base type from the Oid of the temporal type
 */
Oid
temptypid_basetypid(Oid temptypid)
{
  if (!_temptype_cache_ready)
    populate_temptype_cache();
  for (int i = 0; i < TEMPTYPE_CACHE_MAX_LEN; i++)
  {
    if (_temptype_cache[i].temptypid == temptypid)
      return _temptype_cache[i].basetypid;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a temporal type", temptypid);
}

/**
 * Return the temporal type from the base type
 */
CachedType
temptype_basetype(CachedType temptype)
{
  if (!_temptype_cache_ready)
    populate_temptype_cache();
  for (int i = 0; i < TEMPTYPE_CACHE_MAX_LEN; i++)
  {
    if (_temptype_cache[i].temptype == temptype)
      return _temptype_cache[i].basetype;
  }
  /* We only arrive here on error */
  elog(ERROR, "type %u is not a temporal type", temptype);
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

/*****************************************************************************/

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

/*****************************************************************************/
