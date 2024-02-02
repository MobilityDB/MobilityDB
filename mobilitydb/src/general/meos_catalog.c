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
 * @brief Create a cache of PostgreSQL type and operator Oids in global arrays
 * to avoid (slow) lookups
 *
 * The arrays are initialized when the extension is loaded.
 *
 * Estimating the selectivity of Boolean operators is essential for defining
 * efficient queries execution plans. The extension defines several classes
 * of Boolean operators (equal, less than, overlaps, ...), each of which has
 * as left or right arguments a built-in type (such as integer, timestamptz,
 * geometry, ...) or a type defined by the extension (such as tstzspan, tint,
 * ...).
 *
 * As of January 2023 there are 1470 operators defined in MobilityDB.
 * We need to translate between operator Oid <-> MEOS operator info both ways.
 * For Oid -> MEOS operator we use a hash table with Oid as key.
 * For MEOS operator info -> Oid we use a three-dimensional array containing
 * all possible combinations of operator/left argument/right argument.
 * The invalid combinations are initialized to 0.
 */

#include "pg_general/meos_catalog.h"

/* PostgreSQL */
#include <postgres.h>
#include <miscadmin.h> /* For CHECK_FOR_INTERRUPTS */
#include <access/heapam.h>
#include <access/htup_details.h>
#include <access/tableam.h>
#include <catalog/namespace.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
#include <utils/rel.h>
/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h"

/* To avoid include builtins.h */
extern int namestrcmp(Name name, const char *str);

/*****************************************************************************/

/**
 * @brief Structure to represent the operator cache hash table
 */
typedef struct
{
  Oid oproid;        /**< Oid of the operator (hashtable key) */
  meosOper oper;     /**< Operator type number */
  meosType ltype;    /**< Type number of the left argument */
  meosType rtype;    /**< Type number of the right argument */
  char status;       /* hash status */
} oid_oper_entry;

/**
 * @brief Define a hashtable mapping operator Oids to a structure containing 
 * operator and type numbers
 */
#define SH_PREFIX opertable
#define SH_ELEMENT_TYPE oid_oper_entry
#define SH_KEY_TYPE Oid
#define SH_KEY oproid
#define SH_HASH_KEY(tb, key) hash_bytes_uint32(key)
#define SH_EQUAL(tb, a, b) a == b
#define SH_SCOPE static inline
#define SH_DEFINE
#define SH_DECLARE
#include "lib/simplehash.h"

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/**
 * @brief Global variable that states whether the type and operator Oid caches
 * have been initialized
 */
bool _OID_CACHE_READY = false;

/**
 * @brief Global array that keeps the type Oids used in MobilityDB
 */
Oid _TYPE_OIDS[NO_MEOS_TYPES];

/**
 * @brief Global hash table that keeps the operator Oids used in MobilityDB
 */
struct opertable_hash *_OID_OPER = NULL;

/**
 * @brief Global 3-dimensional array that keeps the operator Oids used in
 * MobilityDB
 * 
 * The first dimension corresponds to the operator class (e.g., <=), the second
 * and third dimensions correspond, respectively, to the left and right
 * arguments of the operator. A value 0 is stored in the cell of the array if
 * the operator class is not defined for the left and right types.
 */
Oid _OPER_OID[NO_MEOS_TYPES][NO_MEOS_TYPES][NO_MEOS_TYPES];

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

/**
 * @brief Determine whether the type is an internal MobilityDB type
 */
static bool
internal_type(const char *typname)
{
  if (strncmp(typname, "double", 6) == 0 || strncmp(typname, "tdouble", 7) == 0)
    return true;
  return false;
}

/**
 * @brief Populate the type Oid cache
 */
static void
populate_typeoid_cache()
{
  /* Return if the cache has been already filled */
  if (_OID_CACHE_READY)
    return;
  /* Fill the cache */
  for (int i = 0; i < NO_MEOS_TYPES; i++)
  {
    /* Depending on the PG version some types may not exist (e.g.,
     * multirangetype) and in this case _MEOSTYPE_NAMES[i] will be equal to 0 */
    const char *name = meostype_name(i);
    if (name && ! internal_type(name))
      _TYPE_OIDS[i] = TypenameGetTypid(name);
  }
  return;
}

/**
 * @brief Populate the operator Oid cache from the precomputed operator cache
 * stored in table `mobilitydb_opcache`
 *
 * This table is filled by function #fill_oid_cache when the extension is created.
 * @note Due to some memory context issues, the _OPER_OID array should be
 * filled again even if it is already filled during the extension creation.
 */
static void
populate_operoid_cache()
{
  Oid namespaceId = LookupNamespaceNoError("public");
  OverrideSearchPath* overridePath = GetOverrideSearchPath(CurrentMemoryContext);
  overridePath->schemas = lcons_oid(namespaceId, overridePath->schemas);
  PushOverrideSearchPath(overridePath);

  /* Create the operator hash table */
  _OID_OPER = opertable_create(CacheMemoryContext, 2048, NULL);

  PG_TRY();
  {
    populate_typeoid_cache();
    /* Initialize the operator array */
    memset(_OPER_OID, 0, sizeof(_OPER_OID));
    /* Fetch the rows of the table containing the MobilityDB operator cache */
    Oid catalog = RelnameGetRelid("mobilitydb_opcache");
#if POSTGRESQL_VERSION_NUMBER < 130000
    Relation rel = heap_open(catalog, AccessShareLock);
#else
    Relation rel = table_open(catalog, AccessShareLock);
#endif
    TupleDesc tupDesc = rel->rd_att;
    ScanKeyData scandata;
    TableScanDesc scan = table_beginscan_catalog(rel, 0, &scandata);
    HeapTuple tuple = heap_getnext(scan, ForwardScanDirection);
    while (HeapTupleIsValid(tuple))
    {
      bool isnull = false;
      int32 i = DatumGetInt32(heap_getattr(tuple, 1, tupDesc, &isnull));
      int32 j = DatumGetInt32(heap_getattr(tuple, 2, tupDesc, &isnull));
      int32 k = DatumGetInt32(heap_getattr(tuple, 3, tupDesc, &isnull));
      Oid oproid = DatumGetObjectId(heap_getattr(tuple, 4, tupDesc, &isnull));
      /* Fill the struct to be added to the hash table */
      bool found;
      oid_oper_entry *entry = opertable_insert(_OID_OPER, oproid, &found);
      if (! found)
      {
        entry->oproid = oproid;
        entry->oper = i;
        entry->ltype = j;
        entry->rtype = k;
      }
      /* Fill the operator Oid array */
      _OPER_OID[i][j][k] = oproid;
      /* Read next tuple from table */
      tuple = heap_getnext(scan, ForwardScanDirection);
    }
    heap_endscan(scan);
#if POSTGRESQL_VERSION_NUMBER < 130000
    heap_close(rel, AccessShareLock);
#else
    table_close(rel, AccessShareLock);
#endif
    PopOverrideSearchPath();
    /* Mark that the cache has been initialized */
    _OID_CACHE_READY = true;
  }
  PG_CATCH();
  {
    PopOverrideSearchPath();
    PG_RE_THROW();
  }
  PG_END_TRY();
}

/*****************************************************************************/

/**
 * @brief Fetch from the cache the Oid of a type
 * @arg[in] type Type number
 */
Oid
type_oid(meosType type)
{
  if (!_OID_CACHE_READY)
    populate_operoid_cache();
  Oid result = _TYPE_OIDS[type];
  if (! result)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Unknown MEOS type; %d", type)));
  return result;
}

/**
 * @brief Fetch from the cache the type number
 * @arg[in] type Type Oid
 * @note This function cannot send an error when the type is not found since
 * it is used for all types that appear in the `pg_operator` table when the
 * extension is created
 */
meosType
oid_type(Oid typid)
{
  if (!_OID_CACHE_READY)
    populate_operoid_cache();
  for (int i = 0; i < NO_MEOS_TYPES; i++)
  {
    if (_TYPE_OIDS[i] == typid)
      return i;
  }
  return T_UNKNOWN;
}

/*****************************************************************************/

/**
 * @brief Fetch from the cache the Oid of an operator
 * @arg[in] oper Operator number
 * @arg[in] lt Type number for the left argument
 * @arg[in] rt Type number for the right argument
 */
Oid
oper_oid(meosOper oper, meosType lt, meosType rt)
{
  if (!_OID_CACHE_READY)
    populate_operoid_cache();
  Oid result = _OPER_OID[oper][lt][rt];
  if (! result)
  {
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Unknown MEOS operator: %s, ltype; %s, rtype; %s",
        meosoper_name(oper), meostype_name(lt), meostype_name(rt))));
  }
  return _OPER_OID[oper][lt][rt];
}

/**
 * @brief Fetch from the hash table the operator info
 * @arg[in] oproid Operator oid
 * @arg[out] ltype,rtype Type number of the left/right argument
 */
meosOper
oid_oper(Oid oproid, meosType *ltype, meosType *rtype)
{
  if (!_OID_CACHE_READY)
    populate_operoid_cache();
  oid_oper_entry *entry = opertable_lookup(_OID_OPER, oproid);
  if (! entry)
  {
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Unknown operator Oid %d", oproid)));
    return UNKNOWN_OP; /* make compiler quiet */
  }
  else
  {
    if (ltype)
      *ltype = entry->ltype;
    if (rtype)
      *rtype = entry->rtype;
    return entry->oper;
  }
}

/*****************************************************************************/

PGDLLEXPORT Datum fill_oid_cache(PG_FUNCTION_ARGS __attribute__((unused)));
PG_FUNCTION_INFO_V1(fill_oid_cache);
/**
 * @brief Function executed during the `CREATE EXTENSION` to precompute the
 * operator cache and store it in table `mobilitydb_opcache`
 * @see #populate_operoid_cache
 */
Datum
fill_oid_cache(PG_FUNCTION_ARGS __attribute__((unused)))
{
  /* Fill the Oid type cache */
  populate_typeoid_cache();

  /* Get the Oid of the mobilitydb_opcache table */
  Oid cat_mob = RelnameGetRelid("mobilitydb_opcache");
#if POSTGRESQL_VERSION_NUMBER < 130000
  Relation rel_mob = heap_open(cat_mob, AccessExclusiveLock);
#else
  Relation rel_mob = table_open(cat_mob, AccessExclusiveLock);
#endif
  TupleDesc tupDesc_mob = rel_mob->rd_att;
  bool isnullarr[] = {false, false, false, false};
  Datum data[] = {0, 0, 0, 0};

  /* Get the Oid of the pg_operator catalog table */
  Oid cat_pg = RelnameGetRelid("pg_operator");
#if POSTGRESQL_VERSION_NUMBER < 130000
  Relation rel_pg = heap_open(cat_pg, AccessShareLock);
#else
  Relation rel_pg = table_open(cat_pg, AccessShareLock);
#endif
  ScanKeyData scandata;
  TableScanDesc scan = table_beginscan_catalog(rel_pg, 0, &scandata);
  HeapTuple tuple = heap_getnext(scan, ForwardScanDirection);
  while (HeapTupleIsValid(tuple))
  {
    TupleDesc tupDesc_pg = rel_pg->rd_att;
    /* Get the column numbers of the required attributes */
    AttrNumber oproid_n = InvalidAttrNumber;
    AttrNumber oprname_n = InvalidAttrNumber;
    AttrNumber oprleft_n = InvalidAttrNumber;
    AttrNumber oprright_n = InvalidAttrNumber;
    int k = 0;
    for (int i = 0; i < tupDesc_pg->natts; i++)
    {
      Form_pg_attribute att = TupleDescAttr(tupDesc_pg, i);
      if (namestrcmp(&(att->attname), "oid") == 0)
      {
        oproid_n = att->attnum;
        k++;
      }
      else if (namestrcmp(&(att->attname), "oprname") == 0)
      {
        oprname_n = att->attnum;
        k++;
      }
      else if (namestrcmp(&(att->attname), "oprleft") == 0)
      {
        oprleft_n = att->attnum;
        k++;
      }
      else if (namestrcmp(&(att->attname), "oprright") == 0)
      {
        oprright_n = att->attnum;
        k++;
      }
      if (k == 4)
        break;
    }
    /* Get the operator and type Oids and the operator name */
    bool isnull;
    Oid oproid = DatumGetInt32(heap_getattr(tuple, oproid_n, tupDesc_pg,
      &isnull));
    NameData *oprName = DatumGetName(heap_getattr(tuple, oprname_n, tupDesc_pg,
      &isnull));
    char *oprname = (char *) (oprName->data);
    Oid oprleft = DatumGetInt32(heap_getattr(tuple, oprleft_n, tupDesc_pg,
      &isnull));
    Oid oprright = DatumGetInt32(heap_getattr(tuple, oprright_n, tupDesc_pg,
      &isnull));
    /* Get the type and operator numbers */
    meosOper oper = meosoper_from_string(oprname);
    meosType ltype = oid_type(oprleft);
    meosType rtype = oid_type(oprright);
    /* Fill the cache if the operator and all its types are recognized */
    if (oper != UNKNOWN_OP && ltype != T_UNKNOWN && rtype != T_UNKNOWN)
    {
      data[0] = Int32GetDatum(oper);
      data[1] = Int32GetDatum(ltype);
      data[2] = Int32GetDatum(rtype);
      data[3] = ObjectIdGetDatum(oproid);
      HeapTuple t = heap_form_tuple(tupDesc_mob, data, isnullarr);
      simple_heap_insert(rel_mob, t);
    }
    tuple = heap_getnext(scan, ForwardScanDirection);
    /* Give backend a chance of interrupting us */
    CHECK_FOR_INTERRUPTS();
  }
  heap_endscan(scan);
#if POSTGRESQL_VERSION_NUMBER < 130000
  heap_close(rel_pg, AccessShareLock);
  heap_close(rel_mob, AccessExclusiveLock);
#else
  table_close(rel_pg, AccessShareLock);
  table_close(rel_mob, AccessExclusiveLock);
#endif
  PG_RETURN_VOID();
}

/*****************************************************************************
 * Range and multirange catalog functions
 *****************************************************************************/

/**
 * @brief Return true if the type is a base type of a built-in PostgreSQL range
 * type
 */
bool
range_basetype(meosType type)
{
  if (type == T_TIMESTAMPTZ || type == T_DATE || type == T_INT4)
    return true;
  return false;
}

/**
 * @brief Ensure that a type is a built-in PostgreSQL range type
 */
bool
ensure_range_basetype(meosType type)
{
  if (! range_basetype(type))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The value must be of a type compatible with a range type");
    return false;
  }
  return true;
}

/**
 * @brief Return the range type of a base type
 */
meosType
basetype_rangetype(meosType type)
{
  ensure_range_basetype(type);
  if (type == T_INT4)
    return type_oid(T_INT4RANGE);
  else if (type == T_DATE)
    return type_oid(T_DATERANGE);
  else if (type ==  T_TIMESTAMPTZ)
    return type_oid(T_TSTZRANGE);
  else
  {
    /* We only arrive here on error */
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Type %s is not a base type of a range type", meostype_name(type));
    return T_UNKNOWN;
  }
}

/**
 * @brief Return the range type of a base type
 */
meosType
basetype_multirangetype(meosType type)
{
  ensure_range_basetype(type);
  if (type == T_INT4)
    return type_oid(T_INT4MULTIRANGE);
  else if (type == T_DATE)
    return type_oid(T_DATEMULTIRANGE);
  else if (type ==  T_TIMESTAMPTZ)
    return type_oid(T_TSTZMULTIRANGE);
  else
  {
    /* We only arrive here on error */
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "type %s is not a base type of a multirange type", meostype_name(type));
    return T_UNKNOWN;
  }
}

/*****************************************************************************/
