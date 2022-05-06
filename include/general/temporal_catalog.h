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
 * @file temporal_catalog.h
 * Functions for building a cache of type and operator Oids.
 */

#ifndef TEMP_CATALOG_H
#define TEMP_CATALOG_H

/* PostgreSQL */
#include <postgres.h>

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * @brief Enumeration that defines the built-in and temporal types used in
 * MobilityDB.
 *
 * The Oids of these types are cached in a global array and
 * the enum values are used in the global array for the operator cache.
 */
typedef enum
{
  // T_UNKNOWN = 0,
  T_BOOL,          /**< boolean type */
  T_DOUBLE2,       /**< double2 type */
  T_DOUBLE3,       /**< double3 type */
  T_DOUBLE4,       /**< double4 type */
  T_FLOAT8,        /**< float8 type */
  T_FLOATSPAN,     /**< float8 span type */
  T_INT4,          /**< int4 type */
  T_INT4RANGE,     /**< int4 range type */
  T_INTSPAN,       /**< int4 span type */
  T_INT8,          /**< int8 type */
  T_PERIOD,        /**< period type */
  T_PERIODSET,     /**< period set type */
  T_STBOX,         /**< spatiotemporal box type */
  T_TBOOL,         /**< temporal boolean type */
  T_TBOX,          /**< temporal box type */
  T_TDOUBLE2,      /**< temporal double2 type */
  T_TDOUBLE3,      /**< temporal double3 type */
  T_TDOUBLE4,      /**< temporal double4 type */
  T_TEXT,          /**< text type */
  T_TFLOAT,        /**< temporal float type */
  T_TIMESTAMPSET,  /**< timestamp set type */
  T_TIMESTAMPTZ,   /**< timestamp with time zone type */
  T_TINT,          /**< temporal integer type */
  T_TSTZRANGE,     /**< timestamp with time zone rabge type */
  T_TTEXT,         /**< temporal text type */
  T_GEOMETRY,      /**< geometry type */
  T_GEOGRAPHY,     /**< geography type */
  T_TGEOMPOINT,    /**< temporal geometry point type */
  T_TGEOGPOINT,    /**< temporal geography point type */
  T_NPOINT,        /**< network point type */
  T_NSEGMENT,      /**< network segment type */
  T_TNPOINT,       /**< temporal network point type */
} CachedType;

/**
 * Enumeration that defines the classes of Boolean operators used in MobilityDB.
 * The OIDs of the operators corresponding to these classes are cached in
 * a global array.
 */
typedef enum
{
  // UNKNOWN_OP = 0,
  EQ_OP,           /**< Equality `=` operator */
  NE_OP,           /**< Distinct `!=` operator */
  LT_OP,           /**< Less than `<` operator */
  LE_OP,           /**< Less than or equal to `<=` operator */
  GT_OP,           /**< Greater than `<` operator */
  GE_OP,           /**< Greater than or equal to `>=` operator */
  ADJACENT_OP,     /**< Adjacent `-|-` operator */
  UNION_OP,        /**< Union `+` operator */
  MINUS_OP,        /**< Minus `-` operator */
  INTERSECT_OP,    /**< Intersection `*` operator */
  OVERLAPS_OP,     /**< Overlaps `&&` operator */
  CONTAINS_OP,     /**< Contains `@>` operator */
  CONTAINED_OP,    /**< Contained `<@` operator */
  SAME_OP,         /**< Same `~=` operator */
  LEFT_OP,         /**< Left `<<` operator */
  OVERLEFT_OP,     /**< Overleft `&<` operator */
  RIGHT_OP,        /**< Right `>>` operator */
  OVERRIGHT_OP,    /**< Overright `&>` operator */
  BELOW_OP,        /**< Below `<<|` operator */
  OVERBELOW_OP,    /**< Overbelow `&<|` operator */
  ABOVE_OP,        /**< Above `|>>` operator */
  OVERABOVE_OP,    /**< Overbove `|&>` operator */
  FRONT_OP,        /**< Front `<</` operator */
  OVERFRONT_OP,    /**< Overfront `&</` operator */
  BACK_OP,         /**< Back `/>>` operator */
  OVERBACK_OP,     /**< Overback `/&>` operator */
  BEFORE_OP,       /**< Before `<<#` operator */
  OVERBEFORE_OP,   /**< Overbefore `&<#` operator */
  AFTER_OP,        /**< After `#>>` operator */
  OVERAFTER_OP,    /**< Overafter `#&>` operator */
} CachedOp;

/**
 * Structure to represent the temporal type cache array.
 */
typedef struct
{
  CachedType temptype;    /**< Enum value of the temporal type */
  CachedType basetype;    /**< Enum value of the base type */
} temptype_cache_struct;

/**
 * Structure to represent the span type cache array.
 */
typedef struct
{
  CachedType spantype;    /**< Enum value of the span type */
  CachedType basetype;    /**< Enum value of the base type */
} spantype_cache_struct;

/*****************************************************************************/

/* Cache functions */

extern CachedType temptype_basetype(CachedType temptype);
extern CachedType spantype_basetype(CachedType spantype);
extern CachedType basetype_spantype(CachedType basetype);

/* Catalog functions */

extern bool time_type(CachedType timetype);
extern void ensure_time_type(CachedType timetype);
extern bool span_type(CachedType spantype);
extern void ensure_span_type(CachedType spantype);
extern void ensure_span_basetype(CachedType basetype);
extern bool temporal_type(CachedType temptype);
extern void ensure_temporal_type(CachedType temptype);
extern void ensure_temporal_basetype(CachedType basetype);
extern bool temptype_continuous(CachedType temptype);
extern void ensure_temptype_continuous(CachedType temptype);
extern bool basetype_byvalue(CachedType basetype);
extern int16 basetype_length(CachedType basetype);
extern bool talpha_type(CachedType temptype);
extern bool tnumber_type(CachedType temptype);
extern void ensure_tnumber_type(CachedType temptype);
extern bool tnumber_basetype(CachedType basetype);
extern void ensure_tnumber_basetype(CachedType basetype);
extern bool tnumber_spantype(CachedType spantype);
extern void ensure_tnumber_spantype(CachedType spantype);
extern bool tspatial_type(CachedType temptype);
extern bool tspatial_basetype(CachedType basetype);
extern bool tgeo_basetype(CachedType basetype);
extern bool tgeo_type(CachedType basetype);
extern void ensure_tgeo_type(CachedType basetype);

/* MobilityDB functions */

extern Oid type_oid(CachedType t);
extern Oid oper_oid(CachedOp op, CachedType lt, CachedType rt);
extern CachedType oid_type(Oid typid);

#endif /* TEMPCACHE_H */

/*****************************************************************************/
