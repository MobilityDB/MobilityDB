/*****************************************************************************
 *
 * OidCache.c
 *	  Functions for building a cache of Oids.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include <postgres.h>
#include <access/heapam.h>
#include <access/htup_details.h>
#include <catalog/namespace.h>
#include <catalog/pg_type.h>
#include <utils/rangetypes.h>
#include <utils/rel.h>

#include "TemporalTypes.h"
#include "OidCache.h"

/*****************************************************************************
 * Global arrays for caching the OIDs in order to avoid (slow) lookups.
 * These arrays are initialized at the loading of the extension
 *****************************************************************************/

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
	"ttext"
#ifdef WITH_POSTGIS
	,"geometry",
	"geography",
	"tgeompoint",
	"tgeogpoint",
#endif
};

const char *_op_names[] = 
{
	"=",	/* EQ_OP */
	"<>",	/* NE_OP */
	"<",	/* LT_OP */
	"<=",	/* LE_OP */
	">",	/* GT_OP */
	">=",	/* GE_OP */
	"-|-",	/* ADJACENT_OP */		
	"+",	/* UNION_OP */
	"-",	/* MINUS_OP */
	"*",	/* INTERSECT_OP */
	"&&",	/* OVERLAPS_OP */
	"@>",	/* CONTAINS_OP */
	"<@",	/* CONTAINED_OP */
	"~=",	/* SAME_OP */
	"<<",	/* LEFT_OP */
	"&<",	/* OVERLEFT_OP */
	">>",	/* RIGHT_OP */
	"&>",	/* OVERRIGHT_OP */
	"<<|",	/* BELOW_OP */
	"&<|",	/* OVERBELOW_OP */
	"|>>",	/* ABOVE_OP */
	"|&>",	/* OVERABOVE_OP */
	"<</",	/* FRONT_OP */
	"&</",	/* OVERFRONT_OP */
	"/>>",	/* BACK_OP */
	"/&>",	/* OVERBACK_OP */
	"<<#",	/* BEFORE_OP */
	"&<#",	/* OVERBEFORE_OP */
	"#>>",	/* AFTER_OP */
	"#&>"	/* OVERAFTER_OP */
};

/*****************************************************************************
 * Functions for the Oid cache
 *****************************************************************************/

/* Global variables */

bool _ready = false;
Oid _type_oids[sizeof(_type_names) / sizeof(char *)];
Oid _op_oids[sizeof(_op_names) / sizeof(char *)]
	[sizeof(_type_names) / sizeof(char *)]
	[sizeof(_type_names) / sizeof(char *)];

/* Fetch in the cache the oid of a type */

Oid 
type_oid(CachedType t) 
{
	if (!_ready)
		populate_oidcache();
	return _type_oids[t];
}

/* Fetch in the cache the oid of an operator */

Oid 
oper_oid(CachedOp op, CachedType lt, CachedType rt)
{
	if (!_ready)
		populate_oidcache();
	return _op_oids[op][lt][rt];
}

/* Populate the oid cache */

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
}

void 
populate_oidcache() 
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
		Relation rel = heap_open(catalog, AccessShareLock);
		TupleDesc tupDesc = rel->rd_att;
		ScanKeyData scandata;
		HeapScanDesc scan = heap_beginscan_catalog(rel, 0, &scandata);
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
		heap_close(rel, AccessShareLock);
		_ready = true;

		PopOverrideSearchPath() ;
	}
	PG_CATCH();
	{
		PopOverrideSearchPath() ;
		PG_RE_THROW();
	}
	PG_END_TRY();
}

/*
 * This function is run during the CREATE EXTENSION to pre-compute the 
 * opcache and store it as a table in the catalog.
 */
PG_FUNCTION_INFO_V1(fill_opcache);

PGDLLEXPORT Datum 
fill_opcache(PG_FUNCTION_ARGS) 
{
	Oid catalog = RelnameGetRelid("pg_temporal_opcache");
	Relation rel = heap_open(catalog, AccessExclusiveLock);
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
	heap_close(rel, AccessExclusiveLock);
	PG_RETURN_VOID();
}

/*****************************************************************************/
