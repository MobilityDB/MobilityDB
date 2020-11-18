/*****************************************************************************
 *
 * oidcache.h
 *    Functions for building a cache of type and operator Oids.
 *
 * MobilityDB builds a cache of OIDs in global arrays in order to avoid (slow)
 * lookups. The global arrays are initialized at the loading of the extension.
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

#ifndef OIDCACHE_H
#define OIDCACHE_H

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/**
 * Enumeration that defines the built-in and temporal types used in
 * MobilityDB. The Oids of these types are cached in a global array and
 * the enum values are used in the global array for the operator cache.
 */
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
  T_TTEXT,
  T_GEOMETRY,
  T_GEOGRAPHY,
  T_TGEOMPOINT,
  T_TGEOGPOINT,
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
/**
 * Enumeration that defines the classes of Boolean operators used in MobilityDB.
 * The OIDs of the operators corresponding to these classes are cached in
 * a global array.
 */
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
extern Datum fill_opcache(PG_FUNCTION_ARGS);

#endif /* OIDCACHE_H */

/*****************************************************************************/
