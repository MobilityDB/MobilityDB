/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 *
 * MobilityDB builds a cache of OIDs in global arrays in order to avoid (slow)
 * lookups. The global arrays are initialized at the loading of the extension.
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

#include "tempcache.h"

#if MOBDB_PGSQL_VERSION >= 120000
#include <access/tableam.h>
#endif
#include <access/heapam.h>
#include <access/htup_details.h>
#include <catalog/namespace.h>
#include <utils/rel.h>

#include "temporaltypes.h"

/*****************************************************************************/

/**
 * Global array for caching the names of the types used in MobilityDB
 * to avoid (slow) lookups. The array is initialized at the loading of
 * the extension.
 */
const char *_type_names[] =
{
  "bool",
  "double2",
  "double3",
  "double4",
  "float8",
  "floatrange",
  "int4",
  "intrange",
  "period",
  "periodset",
  "stbox",
  "tbool",
  "tbox",
  "tdouble2",
  "tdouble3",
  "tdouble4",
  "text",
  "tfloat",
  "timestampset",
  "timestamptz",
  "tint",
  "tstzrange",
  "ttext",
  "geometry",
  "geography",
  "tgeompoint",
  "tgeogpoint",
  "npoint",
  "nsegment",
  "tnpoint"
};

/**
 * Global array for caching the names of the operators used in MobilityDB
 * to avoid (slow) lookups. The array is initialized at the loading of
 * the extension.
 */
const char *_op_names[] =
{
  "=",  /* EQ_OP */
  "<>",  /* NE_OP */
  "<",  /* LT_OP */
  "<=",  /* LE_OP */
  ">",  /* GT_OP */
  ">=",  /* GE_OP */
  "-|-",  /* ADJACENT_OP */
  "+",  /* UNION_OP */
  "-",  /* MINUS_OP */
  "*",  /* INTERSECT_OP */
  "&&",  /* OVERLAPS_OP */
  "@>",  /* CONTAINS_OP */
  "<@",  /* CONTAINED_OP */
  "~=",  /* SAME_OP */
  "<<",  /* LEFT_OP */
  "&<",  /* OVERLEFT_OP */
  ">>",  /* RIGHT_OP */
  "&>",  /* OVERRIGHT_OP */
  "<<|",  /* BELOW_OP */
  "&<|",  /* OVERBELOW_OP */
  "|>>",  /* ABOVE_OP */
  "|&>",  /* OVERABOVE_OP */
  "<</",  /* FRONT_OP */
  "&</",  /* OVERFRONT_OP */
  "/>>",  /* BACK_OP */
  "/&>",  /* OVERBACK_OP */
  "<<#",  /* BEFORE_OP */
  "&<#",  /* OVERBEFORE_OP */
  "#>>",  /* AFTER_OP */
  "#&>"  /* OVERAFTER_OP */
};

/*****************************************************************************
 * Functions for the temporal cache
 *****************************************************************************/

/**
 * Global variable that states whether the temporal cache has been initialized
 */
static bool _ready = false;

/**
 * Global array that keeps type information for the temporal types defined
 * in MobilityDB.
 */
static temptypecache_struct _temptype_cache[TEMPTYPECACHE_MAX_LEN];

/**
 * Global array that keeps the Oids of the types used in MobilityDB.
 */
static Oid _type_oids[sizeof(_type_names) / sizeof(char *)];

/**
 * Global 3-dimensional array that keeps the Oids of the operators
 * used in MobilityDB. The first dimension corresponds to the operator
 * class (e.g., <=), the second and third dimensions correspond,
 * respectively, to the left and right arguments of the operator.
 * A value 0 is stored in the cell of the array if the operator class
 * is not defined for the left and right types.
 */
static Oid _op_oids[sizeof(_op_names) / sizeof(char *)]
  [sizeof(_type_names) / sizeof(char *)]
  [sizeof(_type_names) / sizeof(char *)];

/*****************************************************************************/

static void
populate_temptypcache()
{
  Oid namespaceId = LookupNamespaceNoError("catalog") ;
  OverrideSearchPath* overridePath = GetOverrideSearchPath(CurrentMemoryContext);
  overridePath->schemas = lcons_oid(namespaceId, overridePath->schemas);
  PushOverrideSearchPath(overridePath);

  PG_TRY();
  {

    bzero(_temptype_cache, sizeof(temptypecache_struct) * TEMPTYPECACHE_MAX_LEN);

    /*
     * Fetches the temporal type cache from the catalog where it is stored in
     * a table.
     */
    Oid catalog = RelnameGetRelid("mobilitydb_typcache");
#if MOBDB_PGSQL_VERSION < 130000
    Relation rel = heap_open(catalog, AccessShareLock);
#else
    Relation rel = table_open(catalog, AccessShareLock);
#endif
    TupleDesc tupDesc = rel->rd_att;
    ScanKeyData scandata;
#if MOBDB_PGSQL_VERSION >= 120000
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
      _temptype_cache[i].temptypname = NameStr(*DatumGetName(heap_getattr(tuple, 2, tupDesc, &isnull)));
      _temptype_cache[i].basetypid = DatumGetObjectId(heap_getattr(tuple, 3, tupDesc, &isnull));
      _temptype_cache[i].basetypname = NameStr(*DatumGetName(heap_getattr(tuple, 4, tupDesc, &isnull)));
      _temptype_cache[i].basetyplen = DatumGetInt16(heap_getattr(tuple, 5, tupDesc, &isnull));
      _temptype_cache[i].basebyval = DatumGetObjectId(heap_getattr(tuple, 6, tupDesc, &isnull));
      _temptype_cache[i].basecont = DatumGetObjectId(heap_getattr(tuple, 7, tupDesc, &isnull));
      /* The box type attributes may be null or zero for internal types such as doubleN */
      _temptype_cache[i].boxtypid = InvalidOid;
      _temptype_cache[i].boxtypname = NULL;
      _temptype_cache[i].boxtyplen = 0;
      _temptype_cache[i].boxtypid = DatumGetObjectId(heap_getattr(tuple, 8, tupDesc, &isnull));
      if (! isnull)
      {
        _temptype_cache[i].boxtypname = NameStr(*DatumGetName(heap_getattr(tuple, 9, tupDesc, &isnull)));
        _temptype_cache[i].boxtyplen = DatumGetInt16(heap_getattr(tuple, 10, tupDesc, &isnull));
      }
      i++;
      tuple = heap_getnext(scan, ForwardScanDirection);
    }
    heap_endscan(scan);
#if MOBDB_PGSQL_VERSION < 130000
    heap_close(rel, AccessShareLock);
#else
    table_close(rel, AccessShareLock);
#endif
    PopOverrideSearchPath();
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
populate_types()
{
  bzero(_type_oids, sizeof(_type_oids));
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
populate_operators()
{
  bzero(_op_oids, sizeof(_op_oids));
  int32 m = sizeof(_op_names) / sizeof(char *);
  int32 n = sizeof(_type_names) / sizeof(char *);
  for (int32 i = 0; i < m; i++)
  {
    List *lst = list_make1(makeString((char *) _op_names[i]));
    for (int32 j = 0; j < n; j++)
      for (int32 k = 0; k < n; k++)
      {
        Datum opoid = ObjectIdGetDatum(OpernameGetOprid(lst, _type_oids[j], _type_oids[k]));
        if (opoid != InvalidOid)
          _op_oids[i][j][k] = opoid;
      }
    pfree(lst);
  }
  return;
}

PG_FUNCTION_INFO_V1(fill_tempcache);
/**
 * Function executed during the `CREATE EXTENSION` to precompute the
 * operator cache and store it as a table in the catalog
 */
PGDLLEXPORT Datum
fill_tempcache(PG_FUNCTION_ARGS)
{
  populate_temptypcache();
  populate_types();
  populate_operators();
  _ready = true;
  PG_RETURN_VOID();
}

/*****************************************************************************/

/**
 * Fetch from the cache the Oid of a type
 *
 * @arg[in] t Enum value for the type
 */
Oid
type_oid(CachedType type)
{
  if (!_ready)
    populate_types();
  return _type_oids[type];
}

/**
 * Fetch from the cache the Oid of an operator
 *
 * @arg[in] op Enum value for the operator
 * @arg[in] lt Enum value for the left type
 * @arg[in] rt Enum value for the right type
 */
Oid
oper_oid(CachedOp op, CachedType lt, CachedType rt)
{
  if (!_ready)
    populate_operators();
  return _op_oids[op][lt][rt];
}

// /**
 // * Returns the Oid of the base type from the Oid of the temporal type
 // */
// Oid
// temporal_basetypid(Oid temptypid)
// {
  // if (!_ready)
    // populate_temptypcache();
  // Oid result = InvalidOid;
  // for (int i = 0; i < TEMPTYPECACHE_MAX_LEN; i++)
  // {
    // if (_temptype_cache[i].temptypid == temptypid)
    // {
      // result = _temptype_cache[i].basetypid;
      // break;
    // }
    // /* If there are no more temporal types in the array */
    // if (_temptype_cache[i].temptypid == InvalidOid)
      // break;
  // }
  // if (result == InvalidOid)
    // elog(ERROR, "type %u is not a temporal type", temptypid);
  // return result;
// }

// /**
 // * Returns the Oid of the temporal type corresponding to the Oid of the
 // * base type
 // */
// Oid
// temporal_oid_from_base(Oid basetypid)
// {
  // if (!_ready)
    // populate_temptypcache();
  // Oid result = InvalidOid;
  // for (int i = 0; i < TEMPTYPECACHE_MAX_LEN; i++)
  // {
    // if (_temptype_cache[i].basetypid == basetypid)
    // {
      // result = _temptype_cache[i].temptypid;
      // break;
    // }
    // /* If there are no more temporal types in the array */
    // if (_temptype_cache[i].temptypid == InvalidOid)
      // break;
  // }
  // if (result == InvalidOid)
    // elog(ERROR, "type %u is not a base type", basetypid);
  // return result;
// }


// /**
 // * Returns the Oid of the base type corresponding to the Oid of the
 // * temporal type
 // */
// Oid
// base_oid_from_temporal(Oid temptypid)
// {
  // if (!_ready)
    // populate_temptypcache();
  // Oid result = InvalidOid;
  // for (int i = 0; i < TEMPTYPECACHE_MAX_LEN; i++)
  // {
    // if (_temptype_cache[i].temptypid == temptypid)
    // {
      // result = _temptype_cache[i].basetypid;
      // break;
    // }
    // /* If there are no more temporal types in the array */
    // if (_temptype_cache[i].temptypid == InvalidOid)
      // break;
  // }
  // if (result == InvalidOid)
    // elog(ERROR, "type %u is not a temporal type", temptypid);
  // return result;
// }

// /**
 // * Returns true if the values of the type are passed by value.
 // *
 // * This function is called only for the base types of the temporal types
 // * and for TimestampTz. To avoid a call of the slow function get_typbyval
 // * (which makes a lookup call), the known base types are explicitly enumerated.
 // */
// bool
// get_typbyval_fast(Oid basetypid)
// {
  // if (!_ready)
    // populate_temptypcache();
  // if (basetypid == TIMESTAMPTZOID)
    // return true;
  // for (int i = 0; i < TEMPTYPECACHE_MAX_LEN; i++)
  // {
    // if (_temptype_cache[i].basetypid == basetypid)
      // return _temptype_cache[i].basebyval;
    // /* If there are no more temporal types in the array */
    // if (_temptype_cache[i].temptypid == InvalidOid)
      // break;
  // }
  // elog(ERROR, "type %u is not a base type", basetypid);
// }

// /**
 // * Returns the length of type
 // *
 // * This function is called only for the base types of the temporal types
 // * passed by reference. To avoid a call of the slow function get_typlen
 // * (which makes a lookup call), the known base types are explicitly enumerated.
 // */
// int
// get_typlen_byref(Oid basetypid)
// {
  // if (!_ready)
    // populate_temptypcache();
  // for (int i = 0; i < TEMPTYPECACHE_MAX_LEN; i++)
  // {
    // if (_temptype_cache[i].basetypid == basetypid)
      // return _temptype_cache[i].basetyplen;
    // /* If there are no more temporal types in the array */
    // if (_temptype_cache[i].temptypid == InvalidOid)
      // break;
  // }
  // elog(ERROR, "type %u is not a base type", basetypid);
// }

// /**
 // * Returns true if the Oid is a temporal type
 // *
 // * @note Function used in particular in the indexes
 // */
// bool
// temporal_type(Oid temptypid)
// {
  // if (!_ready)
    // populate_temptypcache();
  // for (int i = 0; i < TEMPTYPECACHE_MAX_LEN; i++)
  // {
    // if (_temptype_cache[i].temptypid == temptypid)
      // return true;
    // /* If there are no more temporal types in the array */
    // if (_temptype_cache[i].temptypid == InvalidOid)
      // break;
  // }
  // return false;
// }

// /**
 // * Ensures that the Oid is an external base type supported by MobilityDB
 // */
// void
// ensure_temporal_base_type(Oid basetypid)
// {
  // if (!_ready)
    // populate_temptypcache();
  // for (int i = 0; i < TEMPTYPECACHE_MAX_LEN; i++)
  // {
    // if (_temptype_cache[i].basetypid == basetypid)
      // return;
    // /* If there are no more temporal types in the array */
    // if (_temptype_cache[i].temptypid == InvalidOid)
      // break;
  // }
  // elog(ERROR, "type %u is not a base type", basetypid);
  // return;
// }

// /**
 // * Ensures that the Oid is an external or an internal base type
 // * supported by MobilityDB
 // */
// void
// ensure_temporal_base_type_all(Oid basetypid)
// {
  // if (!_ready)
    // populate_temptypcache();
  // for (int i = 0; i < TEMPTYPECACHE_MAX_LEN; i++)
  // {
    // if (_temptype_cache[i].basetypid == basetypid)
      // return;
    // /* If there are no more temporal types in the array */
    // if (_temptype_cache[i].temptypid == InvalidOid)
      // break;
  // }
  // elog(ERROR, "type %u is not a base type", basetypid);
  // return;
// }

// /**
 // * Returns true if the Oid corresponds to a continuous base type
 // */
// bool
// continuous_base_type(Oid basetypid)
// {
  // if (!_ready)
    // populate_temptypcache();
  // for (int i = 0; i < TEMPTYPECACHE_MAX_LEN; i++)
  // {
    // if (_temptype_cache[i].basetypid == basetypid)
      // return _temptype_cache[i].basecont;
    // /* If there are no more temporal types in the array */
    // if (_temptype_cache[i].temptypid == InvalidOid)
      // break;
  // }
  // elog(ERROR, "type %u is not a base type", basetypid);
// }

// /**
 // * Ensures that the Oid is an external base type that is continuous
 // */
// void
// ensure_continuous_base_type(Oid basetypid)
// {
  // if (! continuous_base_type(basetypid))
    // elog(ERROR, "The base type is not continuous: %d", basetypid);
  // return;
// }

// /**
 // * Ensures that the Oid is an external base type that is continuous
 // */
// void
// ensure_continuous_base_type_all(Oid basetypid)
// {
  // if (! continuous_base_type(basetypid))
    // elog(ERROR, "The base type is not continuous: %d", basetypid);
  // return;
// }

// /*****************************************************************************/

// /**
 // * Returns true if the Oid is a temporal number type
 // *
 // * @note Function used in particular in the indexes
 // */
// bool
// tnumber_range_type(Oid typid)
// {
  // if (typid == type_oid(T_INTRANGE) || typid == type_oid(T_FLOATRANGE))
    // return true;
  // return false;
// }

// /**
 // * Ensures that the Oid is a range type
 // */
// void
// ensure_tnumber_range_type(Oid typid)
// {
  // if (! tnumber_range_type(typid))
    // elog(ERROR, "unknown number range type: %d", typid);
  // return;
// }

// /**
 // * Returns true if the Oid is a temporal number type
 // *
 // * @note Function used in particular in the indexes
 // */
// bool
// tnumber_type(Oid typid)
// {
  // if (typid == type_oid(T_TINT) || typid == type_oid(T_TFLOAT))
    // return true;
  // return false;
// }

// /**
 // * Test whether the Oid is a number base type supported by MobilityDB
 // */
// bool
// tnumber_base_type(Oid typid)
// {
  // if (typid == INT4OID || typid == FLOAT8OID)
    // return true;
  // return false;
// }

// /**
 // * Ensures that the Oid is a number base type supported by MobilityDB
 // */
// void
// ensure_tnumber_base_type(Oid basetypid)
// {
  // if (! tnumber_base_type(basetypid))
    // elog(ERROR, "unknown number base type: %d", basetypid);
  // return;
// }

// /*****************************************************************************/

// /**
 // * Returns true if the Oid is a temporal point type
 // *
 // * @note Function used in particular in the indexes
 // */
// bool
// tgeo_type(Oid typid)
// {
  // if (typid == type_oid(T_TGEOMPOINT) || typid == type_oid(T_TGEOGPOINT) || 
    // typid == type_oid(T_TNPOINT))
    // return true;
  // return false;
// }

// /**
 // * Ensures that the Oid is a point base type supported by MobilityDB
 // */
// bool
// tgeo_base_type(Oid typid)
// {
  // if (typid == type_oid(T_GEOMETRY) || typid == type_oid(T_GEOGRAPHY))
    // return true;
  // return false;
// }

// /**
 // * Ensures that the Oid is a point base type supported by MobilityDB
 // */
// void
// ensure_tgeo_base_type(Oid basetypid)
// {
  // if (! tgeo_base_type(basetypid))
    // elog(ERROR, "unknown geospatial base type: %d", basetypid);
  // return;
// }

// /**
 // * Returns true if the temporal type corresponding to the Oid of the
 // * base type has its trajectory precomputed
 // */
// bool
// type_has_precomputed_trajectory(Oid basetypid)
// {
  // if (tgeo_base_type(basetypid))
    // return true;
  // return false;
// }

/*****************************************************************************/
