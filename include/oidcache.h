/*****************************************************************************
 *
 * oidcache.h
 *	  Functions for building a cache of Oids.
 *
 * The temporal extension builds a cache of OIDs in global arrays in order to 
 * avoid (slow) lookups. The global arrays are initialized at the loading 
 * of the extension.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef TEMPORAL_OIDCACHE_H
#define TEMPORAL_OIDCACHE_H

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>

/*
 * The list of built-in and temporal types that must be cached. 
 */

/* Currently 33 types */ 
typedef enum 
{
	T_BOOL,
	T_DOUBLE2,
	T_DOUBLE3,
	T_DOUBLE4,
	T_FLOAT8,
	T_FLOATRANGE,
	T_INT4,
	T_INTRANGE,
	T_PERIOD,
	T_PERIODSET,
	T_STBOX,
	T_TBOOL,
	T_TBOX,
	T_TDOUBLE2,
	T_TDOUBLE3,
	T_TDOUBLE4,
	T_TEXT,
	T_TFLOAT,
	T_TIMESTAMPSET,
	T_TIMESTAMPTZ,
	T_TINT,
	T_TSTZRANGE,
	T_TTEXT
#ifdef WITH_POSTGIS
	,T_GEOMETRY,
	T_GEOGRAPHY,
	T_TGEOMPOINT,
	T_TGEOGPOINT,
#endif
} CachedType;

/*
 * The selectivity of Boolean operators is essential in order to determine 
 * efficient execution plans for queries. The temporal extension defines 
 * several classes of Boolean operators (equal, less than, overlaps, ...),
 * currently 30, each of which can have as left or right arguments a built-in
 * type (such as integer, timestamptz, box, geometry, ...) or a newly defined 
 * type (such as period, tint, ...), currently 33. 
 *
 * There are currently 3,392 operators, each of which is identified by an Oid. 
 * To avoid enumerating all of these operators in the Oid cache, we use a 
 * two-dimensional array containing all possible combinations of 
 * operator/left argument/right argument (currently 23 * 40 * 40 = 36,800 cells). 
 * The invalid combinations will be initialized to 0.
 */

/* Currently 30 operators */
typedef enum 
{
	EQ_OP,
	NE_OP,
	LT_OP,
	LE_OP,
	GT_OP,
	GE_OP,
	ADJACENT_OP,		
	UNION_OP,		
	MINUS_OP,		
	INTERSECT_OP,		
	OVERLAPS_OP,
	CONTAINS_OP,
	CONTAINED_OP,
	SAME_OP,
	LEFT_OP,
	OVERLEFT_OP,
	RIGHT_OP,
	OVERRIGHT_OP,
	BELOW_OP,
	OVERBELOW_OP,
	ABOVE_OP,
	OVERABOVE_OP,
	FRONT_OP,
	OVERFRONT_OP,
	BACK_OP,
	OVERBACK_OP,
	BEFORE_OP,
	OVERBEFORE_OP,
	AFTER_OP,
	OVERAFTER_OP,
} CachedOp;


extern Oid type_oid(CachedType t);
extern Oid oper_oid(CachedOp op, CachedType lt, CachedType rt);
extern void populate_oidcache();

extern Datum fill_opcache(PG_FUNCTION_ARGS);

#endif /* TEMPORAL_OIDCACHE_H */

/*****************************************************************************/
