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
 * @file oidcache.c
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
 
#include "oidcache.h"

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
  "tgeogpoint"
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
 * Functions for the Oid cache
 *****************************************************************************/

/**
 * Global variable that states whether the type and operator cache
 * has been initialized.
 */
bool _ready = false;

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

/**
 * Populate the Oid cache for types
 */
static void
populate_types()
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
populate_operators()
{
  Oid namespaceId = LookupNamespaceNoError("public") ;
  OverrideSearchPath* overridePath = GetOverrideSearchPath(CurrentMemoryContext);
  overridePath->schemas = lcons_oid(namespaceId, overridePath->schemas);
  PushOverrideSearchPath(overridePath);

  PG_TRY();
  {

    populate_types();
    bzero(_op_oids, sizeof(_op_oids));

    /*
     * This fetches the pre-computed operator cache from the catalog where
     * it is stored in a table. See the fill_opcache function below.
     */
    Oid catalog = RelnameGetRelid("pg_temporal_opcache");
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
#if MOBDB_PGSQL_VERSION < 130000
    heap_close(rel, AccessShareLock);
#else
    table_close(rel, AccessShareLock);
#endif
    _ready = true;

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
 * Fetch from the cache the Oid of a type
 *
 * @arg[in] t Enum value for the type
 */
Oid
type_oid(CachedType type)
{
  if (!_ready)
    populate_operators();
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


PG_FUNCTION_INFO_V1(fill_opcache);
/**
 * Function executed during the `CREATE EXTENSION` to precompute the
 * operator cache and store it as a table in the catalog
 */
PGDLLEXPORT Datum
fill_opcache(PG_FUNCTION_ARGS)
{
  Oid catalog = RelnameGetRelid("pg_temporal_opcache");
#if MOBDB_PGSQL_VERSION < 130000
  Relation rel = heap_open(catalog, AccessExclusiveLock);
#else
  Relation rel = table_open(catalog, AccessExclusiveLock);
#endif
  TupleDesc tupDesc = rel->rd_att;

  bool isnull[] = {false, false, false, false};
  Datum data[] = {0, 0, 0, 0};

  populate_types();
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
#if MOBDB_PGSQL_VERSION < 130000
  heap_close(rel, AccessExclusiveLock);
#else
  table_close(rel, AccessExclusiveLock);
#endif
  PG_RETURN_VOID();
}

/*****************************************************************************/
