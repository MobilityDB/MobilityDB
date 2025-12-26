/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Create a cache of PostgreSQL type and operator Oids in global
 * variable arrays to avoid (slow) lookups
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

#include "pg_temporal/meos_catalog.h"

/* PostgreSQL */
#include <postgres.h>
#include <miscadmin.h> /* For CHECK_FOR_INTERRUPTS */
#include <access/genam.h>
#include <access/heapam.h>
#include <access/htup_details.h>
#include <access/tableam.h>
#if POSTGRESQL_VERSION_NUMBER < 140000
#include <catalog/indexing.h>
#endif
#include <catalog/namespace.h>
#include <catalog/pg_extension.h>
#include <commands/extension.h>
#include <common/hashfn.h>
#include <utils/fmgroids.h>
#include <utils/memutils.h>
#include <utils/syscache.h>
#include <utils/rel.h>
/* MEOS */
#include <meos.h>
#include "temporal/meos_catalog.h"

/* To avoid include builtins.h */
extern int namestrcmp(Name name, const char *str);

/*****************************************************************************/

/**
 * @brief Structure to represent the operator cache hash table
 */
typedef struct
{
  Oid typid;         /**< Oid of the type (hashtable key) */
  meosType type;     /**< MEOS type */
  char status;       /* hash status */
} oid_meostype_entry;

#define SH_PREFIX oid_meostype
#define SH_ELEMENT_TYPE oid_meostype_entry
#define SH_KEY_TYPE Oid
#define SH_KEY typid
#define SH_HASH_KEY(tb, key) murmurhash32(key)
#define SH_EQUAL(tb, a, b) ((a) == (b))
#define SH_SCOPE static inline
#define SH_DEFINE
#define SH_DECLARE
#include "lib/simplehash.h"

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
} oid_meosoper_entry;

/**
 * @brief Define a hashtable mapping operator Oids to a structure containing
 * operator and type numbers
 */
#define SH_PREFIX opertable
#define SH_ELEMENT_TYPE oid_meosoper_entry
#define SH_KEY_TYPE Oid
#define SH_KEY oproid
#define SH_HASH_KEY(tb, key) murmurhash32(key)
#define SH_EQUAL(tb, a, b) ((a) == (b))
#define SH_SCOPE static inline
#define SH_DEFINE
#define SH_DECLARE
#include "lib/simplehash.h"

/*****************************************************************************
 * Global variables defined in the file globals.c
 *****************************************************************************/

/**
 * @brief Global variable that states whether the type and operator Oid caches
 * have been initialized
 * @details
 * - Global array that enables the mapping meosType -> Oid
 * - Global hash table that enables the mapping Oid -> meosType
 * - Global hash table that enables the mapping meosOper -> Oid
 * - Global 3-dimensional array that enables the mapping meosOper -> Oid
 *   in MobilityDB.
 *   The first dimension corresponds to the operator class (e.g., <=), the
 *   second and third dimensions correspond, respectively, to the left and
 *   right arguments of the operator. A value 0 is stored in the cell of the
 *   array if the operator class is not defined for the left and right types.
 */

typedef struct
{
  Oid meostype_oid[NO_MEOS_TYPES];         /* meosType -> Oid */
  struct oid_meostype_hash *oid_meostype;
  struct opertable_hash *meosoper_oid;
  Oid oper_oid_args[NO_MEOS_TYPES][NO_MEOS_TYPES][NO_MEOS_TYPES];
} mobilitydb_constants;

/* Global to hold all the run-time constants */
static mobilitydb_constants *MOBILITYDB_CONSTANTS = NULL;

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

/**
 * @brief Determine whether the type is an internal MobilityDB type
 */
static bool
internal_type(const char *typname)
{
  if (strncmp(typname, "unknown", 7) == 0 ||
      strncmp(typname, "double", 6) == 0 ||
      strncmp(typname, "tdouble", 7) == 0)
    return true;
  return false;
}

/**
 * @brief Utility call to lookup type oid given name and nspoid
 */
static Oid
TypenameNspGetTypid(const char *typname, Oid nsp_oid)
{
  return GetSysCacheOid2(TYPENAMENSP, Anum_pg_type_oid,
    PointerGetDatum(typname), ObjectIdGetDatum(nsp_oid));
}

/**
 * @brief Utility call to lookup relation oid given name and nspoid
 */
static Oid
RelnameNspGetRelid(const char *relname, Oid nsp_oid)
{
  return GetSysCacheOid2(RELNAMENSP, Anum_pg_class_oid,
    PointerGetDatum(relname), ObjectIdGetDatum(nsp_oid));
}

#if POSTGRESQL_VERSION_NUMBER < 160000
/*
 * get_extension_schema - given an extension OID, fetch its extnamespace
 *
 * Returns InvalidOid if no such extension.
 */
static Oid
get_extension_schema(Oid ext_oid)
{
  Relation rel = table_open(ExtensionRelationId, AccessShareLock);

  ScanKeyData entry[1];
  ScanKeyInit(&entry[0], Anum_pg_extension_oid, BTEqualStrategyNumber, F_OIDEQ,
  ObjectIdGetDatum(ext_oid));

  SysScanDesc scandesc = systable_beginscan(rel, ExtensionOidIndexId, true,
    NULL, 1, entry);

  HeapTuple tuple = systable_getnext(scandesc);

  /* We assume that there can be at most one matching tuple */
  Oid result;
  if (HeapTupleIsValid(tuple))
    result = ((Form_pg_extension) GETSTRUCT(tuple))->extnamespace;
  else
    result = InvalidOid;

  systable_endscan(scandesc);
  table_close(rel, AccessShareLock);
  return result;
}
#endif

/**
 * @brief Return namespace Oid for the extension
 */
static Oid
mobilitydb_namespace_oid()
{
  Oid nsp_oid = InvalidOid;
  Oid ext_oid = get_extension_oid("mobilitydb", true);
  if (ext_oid != InvalidOid)
    nsp_oid = get_extension_schema(ext_oid);

  /* early exit if we cannot lookup nsp_name */
  if (nsp_oid == InvalidOid)
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Unable to determine 'mobilitydb' install schema");

  return nsp_oid;
}

/* Cache type lookups in per-session location */
static mobilitydb_constants *
get_mobilitydb_constants()
{
  /* Put constants cache in a child of the CacheContext */
  MemoryContext context = AllocSetContextCreate(CacheMemoryContext,
    "MobilityDB Constants Context", ALLOCSET_DEFAULT_SIZES);

  /* Allocate in the CacheContext so it is kept at the end of the statement 
     Initialize it to zero for the meostype_oid and oper_oid_args arrays */
  mobilitydb_constants* constants =
    MemoryContextAllocZero(context, sizeof(mobilitydb_constants));

  /* Populate the meosType -> Oid array */
  Oid nsp_oid = mobilitydb_namespace_oid();
  for (int i = 0; i < NO_MEOS_TYPES; i++)
  {
    /* Depending on compilation constraints (e.g., -DCBUFFER=1) some types are
       not defined and in this case MEOS_TYPE_NAMES[i] will be equal to 0 */
    const char *name = meostype_name(i);
    if (name && ! internal_type(name))
    {
      /* Search for type oid in extension namespace */
      constants->meostype_oid[i] = TypenameNspGetTypid(name, nsp_oid);
      /* If not found, search default namespace */
      if (constants->meostype_oid[i] == InvalidOid)
        constants->meostype_oid[i] = TypenameGetTypid(name);
    }
  }

  /* Create the Oid -> meosType hash table and populate it */
  constants->oid_meostype = oid_meostype_create(CacheMemoryContext, 64, NULL);
  /* Initialize the oid -> meosType array */
  for (int i = 0; i < NO_MEOS_TYPES; i++)
  {
    Oid oid = constants->meostype_oid[i];
    if (! OidIsValid(oid))
      continue;
    oid_meostype_entry *entry;
    bool found;
    entry = oid_meostype_insert(constants->oid_meostype, oid, &found);
    if (found)
      ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
        errmsg("Duplicate Oid %u in MobilityDB type catalog", oid)));
    entry->type = (meosType) i;
  }

  /* Create the operator hash table and populate the operator array and the
     operator hash table */
  constants->meosoper_oid = opertable_create(CacheMemoryContext, 2048, NULL);
  /* Fetch the rows of the table containing the MobilityDB operator cache */
  Oid catalog = RelnameNspGetRelid("mobilitydb_opcache", nsp_oid);
  Relation rel = table_open(catalog, AccessShareLock);
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
    oid_meosoper_entry *entry =
      opertable_insert(constants->meosoper_oid, oproid, &found);
    if (! found)
    {
      entry->oproid = oproid;
      entry->oper = i;
      entry->ltype = j;
      entry->rtype = k;
    }
    /* Fill the operator Oid array */
    constants->oper_oid_args[i][j][k] = oproid;
    /* Read next tuple from table */
    tuple = heap_getnext(scan, ForwardScanDirection);
  }
  heap_endscan(scan);
  table_close(rel, AccessShareLock);

  return constants;
}

/**
 * @brief Initialize Oid cache
 */
static inline void
mobilitydb_initialize_cache()
{
  /* Return if cache has been initialized */
  if (MOBILITYDB_CONSTANTS)
    return;
  /* Cache the info if we don't already have it */
  MOBILITYDB_CONSTANTS = get_mobilitydb_constants();
}

/*****************************************************************************/

/**
 * @brief Fetch from the cache the Oid of a type
 * @arg[in] type Type number
 */
Oid
meostype_oid(meosType type)
{
  mobilitydb_initialize_cache();
  Oid result = MOBILITYDB_CONSTANTS->meostype_oid[type];
  if (! result)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Unknown MEOS type; %s", meostype_name(type))));
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
oid_meostype(Oid typid)
{
  mobilitydb_initialize_cache();
  oid_meostype_entry *entry = oid_meostype_lookup(
    MOBILITYDB_CONSTANTS->oid_meostype, typid);
  /* Do not throw an error as in function #meostype_oid since when looking up
     an operator id we get PostgreSQL types that do not have a corresponding
     meosType, for example, the char type with Oid 18 */
  if (! entry)
    return T_UNKNOWN;
  else
    return entry->type;
}

/*****************************************************************************/

/**
 * @brief Fetch from the cache the Oid of an operator
 * @arg[in] oper Operator number
 * @arg[in] lt Type number for the left argument
 * @arg[in] rt Type number for the right argument
 */
Oid
meosoper_oid(meosOper oper, meosType lt, meosType rt)
{
  mobilitydb_initialize_cache();

  Oid result = MOBILITYDB_CONSTANTS->oper_oid_args[oper][lt][rt];
  if (! result)
  {
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Unknown MEOS operator: %s, ltype; %s, rtype; %s",
        meosoper_name(oper), meostype_name(lt), meostype_name(rt))));
  }
  return MOBILITYDB_CONSTANTS->oper_oid_args[oper][lt][rt];
}

/**
 * @brief Fetch from the hash table the operator info
 * @arg[in] oproid Operator oid
 * @arg[out] ltype,rtype Type number of the left/right argument
 */
meosOper
oid_meosoper(Oid oproid, meosType *ltype, meosType *rtype)
{
  mobilitydb_initialize_cache();

  oid_meosoper_entry *entry = opertable_lookup(
    MOBILITYDB_CONSTANTS->meosoper_oid, oproid);
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

PGDLLEXPORT Datum fill_oid_cache(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(fill_oid_cache);
/**
 * @brief Function executed during the `CREATE EXTENSION` to precompute the
 * operator cache and store it in table `mobilitydb_opcache`
 */
Datum
fill_oid_cache(PG_FUNCTION_ARGS)
{
  /* Get the Oid of the mobilitydb_opcache table */
  Oid cat_mob = RelnameGetRelid("mobilitydb_opcache");
  Relation rel_mob = table_open(cat_mob, AccessExclusiveLock);
  TupleDesc tupDesc_mob = rel_mob->rd_att;
  bool isnullarr[] = {false, false, false, false};
  Datum data[] = {0, 0, 0, 0};

  /* Get the Oid of the pg_operator catalog table */
  Oid cat_pg = RelnameGetRelid("pg_operator");
  Relation rel_pg = table_open(cat_pg, AccessShareLock);
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
    meosType ltype = oid_meostype(oprleft);
    meosType rtype = oid_meostype(oprright);
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
  table_close(rel_pg, AccessShareLock);
  table_close(rel_mob, AccessExclusiveLock);
  PG_RETURN_VOID();
}

/*****************************************************************************
 * Range and multirange catalog functions
 *****************************************************************************/

/**
 * @brief Return true if the type is a base type of a built-in PostgreSQL range
 * type
 */
inline bool
range_basetype(meosType type)
{
  return (type == T_TIMESTAMPTZ || type == T_DATE || type == T_INT4 ||
    type == T_INT8);
}

/**
 * @brief Ensure that a type is a built-in PostgreSQL range type
 */
bool
ensure_range_basetype(meosType type)
{
  if (range_basetype(type))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "The value must be of a type compatible with a range type");
  return false;
}

/**
 * @brief Return the range type of a base type
 */
meosType
basetype_rangetype(meosType type)
{
  ensure_range_basetype(type);
  if (type == T_INT4)
    return meostype_oid(T_INT4RANGE);
  if (type == T_INT8)
    return meostype_oid(T_INT8RANGE);
  if (type == T_DATE)
    return meostype_oid(T_DATERANGE);
  if (type ==  T_TIMESTAMPTZ)
    return meostype_oid(T_TSTZRANGE);

  /* We only arrive here on error */
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "Type %s is not a base type of a range type", meostype_name(type));
  return T_UNKNOWN;
}

/**
 * @brief Return the range type of a base type
 */
meosType
basetype_multirangetype(meosType type)
{
  ensure_range_basetype(type);
  if (type == T_INT4)
    return meostype_oid(T_INT4MULTIRANGE);
  if (type == T_INT8)
    return meostype_oid(T_INT8MULTIRANGE);
  if (type == T_DATE)
    return meostype_oid(T_DATEMULTIRANGE);
  if (type ==  T_TIMESTAMPTZ)
    return meostype_oid(T_TSTZMULTIRANGE);

  /* We only arrive here on error */
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "type %s is not a base type of a multirange type", meostype_name(type));
  return T_UNKNOWN;
}

/*****************************************************************************/
