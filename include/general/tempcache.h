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
 * @file tempcache.h
 * Functions for building a cache of type and operator Oids.
 */

#ifndef TEMPCACHE_H
#define TEMPCACHE_H

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************/

/**
 * Enumeration that defines the built-in and temporal types used in
 * MobilityDB. The Oids of these types are cached in a global array and
 * the enum values are used in the global array for the operator cache.
 * The temporal types should be first to accelerate the search for them.
 */
typedef enum
{
  // T_UNKNOWN = 0,
  T_BOOL,
  T_DOUBLE2,
  T_DOUBLE3,
  T_DOUBLE4,
  T_FLOAT8,
  T_FLOATRANGE,
  T_FLOATSPAN,
  T_INT4,
  T_INTRANGE,
  T_INTSPAN,
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
  T_TSTZSPAN,
  T_TIMESTAMPTZ,
  T_TINT,
  T_TSTZRANGE,
  T_TTEXT,
  T_GEOMETRY,
  T_GEOGRAPHY,
  T_TGEOMPOINT,
  T_TGEOGPOINT,
  T_NPOINT,
  T_NSEGMENT,
  T_TNPOINT,
} CachedType;

/**
 * Enumeration that defines the classes of Boolean operators used in MobilityDB.
 * The OIDs of the operators corresponding to these classes are cached in
 * a global array.
 */
typedef enum
{
  // UNKNOWN_OP = 0,
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

/**
 * Structure to represent the temporal type cache array.
 */
typedef struct
{
  CachedType temptype;    /**< Enum value of the temporal type */
  CachedType basetype;    /**< Enum value of the base type */
  bool basecont;          /**< True if the base type is continuous */
} temptype_cache_struct;

typedef struct
{
  CachedType spantype;    /**< Enum value of the span type */
  CachedType basetype;    /**< Enum value of the base type */
  bool basecont;          /**< True if the base type is continuous */
} spantype_cache_struct;

#define TEMPTYPE_CACHE_MAX_LEN   16

/*****************************************************************************/

/* Catalog functions */

extern CachedType temptype_basetype(CachedType temptype);
extern CachedType spantype_basetype(CachedType spantype);
extern CachedType basetype_spantype(CachedType basetype);

extern Oid type_oid(CachedType t);
extern Oid oper_oid(CachedOp op, CachedType lt, CachedType rt);
extern CachedType oid_type(Oid typid);

#endif /* TEMPCACHE_H */

/*****************************************************************************/
