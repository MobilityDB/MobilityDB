/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Functions for building a cache of temporal types and operators.
 */

#ifndef __MEOS_CATALOG_H__
#define __MEOS_CATALOG_H__

/* C */
#include <stdbool.h>
/* PostgreSQL */
#ifndef int16
typedef signed short int16;
#endif

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * @brief Enumeration that defines the built-in and temporal types used in
 * MobilityDB.
 */
typedef enum
{
  T_UNKNOWN        = 0,   /**< unknown type */
  T_BOOL           = 1,   /**< boolean type */
  T_DOUBLE2        = 2,   /**< double2 type */
  T_DOUBLE3        = 3,   /**< double3 type */
  T_DOUBLE4        = 4,   /**< double4 type */
  T_FLOAT8         = 5,   /**< float8 type */
  T_FLOATSET       = 6,   /**< float8 set type */
  T_FLOATSPAN      = 7,   /**< float8 span type */
  T_FLOATSPANSET   = 8,   /**< float8 span set type */
  T_INT4           = 9,   /**< int4 type */
  T_INT4RANGE      = 10,  /**< PostgreSQL int4 range type */
  T_INT4MULTIRANGE = 11,  /**< PostgreSQL int4 multirange type */
  T_INTSET         = 12,  /**< int4 set type */
  T_INTSPAN        = 13,  /**< int4 span type */
  T_INTSPANSET     = 14,  /**< int4 span set type */
  T_INT8           = 15,  /**< int8 type */
  T_BIGINTSET      = 16,  /**< int8 set type */
  T_BIGINTSPAN     = 17,  /**< int8 span type */
  T_BIGINTSPANSET  = 18,  /**< int8 span set type */
  T_STBOX          = 19,  /**< spatiotemporal box type */
  T_TBOOL          = 20,  /**< temporal boolean type */
  T_TBOX           = 21,  /**< temporal box type */
  T_TDOUBLE2       = 22,  /**< temporal double2 type */
  T_TDOUBLE3       = 23,  /**< temporal double3 type */
  T_TDOUBLE4       = 24,  /**< temporal double4 type */
  T_TEXT           = 25,  /**< text type */
  T_TEXTSET        = 26,  /**< text type */
  T_TFLOAT         = 27,  /**< temporal float type */
  T_TIMESTAMPTZ    = 28,  /**< timestamp with time zone type */
  T_TINT           = 29,  /**< temporal integer type */
  T_TSTZMULTIRANGE = 30,  /**< PostgreSQL timestamp with time zone multirange type */
  T_TSTZRANGE      = 31,  /**< PostgreSQL timestamp with time zone range type */
  T_TSTZSET        = 32,  /**< timestamptz set type */
  T_TSTZSPAN       = 33,  /**< timestamptz span type */
  T_TSTZSPANSET    = 34,  /**< timestamptz span set type */
  T_TTEXT          = 35,  /**< temporal text type */
  T_GEOMETRY       = 36,  /**< geometry type */
  T_GEOMSET        = 37,  /**< geometry set type */
  T_GEOGRAPHY      = 38,  /**< geography type */
  T_GEOGSET        = 39,  /**< geography set type */
  T_TGEOMPOINT     = 40,  /**< temporal geometry point type */
  T_TGEOGPOINT     = 41,  /**< temporal geography point type */
  T_NPOINT         = 42,  /**< network point type */
  T_NPOINTSET      = 43,  /**< network point set type */
  T_NSEGMENT       = 44,  /**< network segment type */
  T_TNPOINT        = 45,  /**< temporal network point type */
} meosType;

/**
 * Enumeration that defines the classes of Boolean operators used in
 * MobilityDB.
 */
typedef enum
{
  UNKNOWN_OP      = 0,
  EQ_OP           = 1,  /**< Equality `=` operator */
  NE_OP           = 2,  /**< Distinct `!=` operator */
  LT_OP           = 3,  /**< Less than `<` operator */
  LE_OP           = 4,  /**< Less than or equal to `<=` operator */
  GT_OP           = 5,  /**< Greater than `<` operator */
  GE_OP           = 6,  /**< Greater than or equal to `>=` operator */
  ADJACENT_OP     = 7,  /**< Adjacent `-|-` operator */
  UNION_OP        = 8,  /**< Union `+` operator */
  MINUS_OP        = 9,  /**< Minus `-` operator */
  INTERSECT_OP    = 10, /**< Intersection `*` operator */
  OVERLAPS_OP     = 11, /**< Overlaps `&&` operator */
  CONTAINS_OP     = 12, /**< Contains `@>` operator */
  CONTAINED_OP    = 13, /**< Contained `<@` operator */
  SAME_OP         = 14, /**< Same `~=` operator */
  LEFT_OP         = 15, /**< Left `<<` operator */
  OVERLEFT_OP     = 16, /**< Overleft `&<` operator */
  RIGHT_OP        = 17, /**< Right `>>` operator */
  OVERRIGHT_OP    = 18, /**< Overright `&>` operator */
  BELOW_OP        = 19, /**< Below `<<|` operator */
  OVERBELOW_OP    = 20, /**< Overbelow `&<|` operator */
  ABOVE_OP        = 21, /**< Above `|>>` operator */
  OVERABOVE_OP    = 22, /**< Overbove `|&>` operator */
  FRONT_OP        = 23, /**< Front `<</` operator */
  OVERFRONT_OP    = 24, /**< Overfront `&</` operator */
  BACK_OP         = 25, /**< Back `/>>` operator */
  OVERBACK_OP     = 26, /**< Overback `/&>` operator */
  BEFORE_OP       = 27, /**< Before `<<#` operator */
  OVERBEFORE_OP   = 28, /**< Overbefore `&<#` operator */
  AFTER_OP        = 29, /**< After `#>>` operator */
  OVERAFTER_OP    = 30, /**< Overafter `#&>` operator */
  EVEREQ_OP       = 31, /**< Evereq `?=` operator */
  EVERNE_OP       = 32, /**< Everne `?<>` operator */
  EVERLT_OP       = 33, /**< Everlt `?<` operator */
  EVERLE_OP       = 34, /**< Everle `?<=` operator */
  EVERGT_OP       = 35, /**< Evergt `?>` operator */
  EVERGE_OP       = 36, /**< Everge `?>=` operator */
  ALWAYSEQ_OP     = 37, /**< Alwayseq `%=` operator */
  ALWAYSNE_OP     = 38, /**< Alwaysne `%<>` operator */
  ALWAYSLT_OP     = 39, /**< Alwayslt `%<` operator */
  ALWAYSLE_OP     = 40, /**< Alwaysle `%<=` operator */
  ALWAYSGT_OP     = 41, /**< Alwaysgt `%>` operator */
  ALWAYSGE_OP     = 42, /**< Alwaysge `%>=` operator */
} meosOper;

/**
 * Structure to represent the temporal type cache array.
 */
typedef struct
{
  meosType temptype;    /**< Enum value of the temporal type */
  meosType basetype;    /**< Enum value of the base type */
} temptype_catalog_struct;

/**
 * Structure to represent the span type cache array.
 */
typedef struct
{
  meosType settype;     /**< Enum value of the set type */
  meosType basetype;    /**< Enum value of the base type */
} settype_catalog_struct;

/**
 * Structure to represent the span type cache array.
 */
typedef struct
{
  meosType spantype;    /**< Enum value of the span type */
  meosType basetype;    /**< Enum value of the base type */
} spantype_catalog_struct;

/**
 * Structure to represent the spanset type cache array.
 */
typedef struct
{
  meosType spansettype;    /**< Enum value of the span type */
  meosType spantype;       /**< Enum value of the base type */
} spansettype_catalog_struct;

/*****************************************************************************/

/* Type conversion functions */

extern meosType temptype_basetype(meosType temptype);
extern meosType settype_basetype(meosType settype);
extern meosType spantype_basetype(meosType spantype);
extern meosType spantype_spansettype(meosType spantype);
extern meosType spansettype_spantype(meosType spansettype);
extern meosType basetype_spantype(meosType basetype);
extern meosType basetype_settype(meosType basetype);

/* Catalog functions */

extern bool meos_basetype(meosType type);
extern bool alpha_basetype(meosType basetype);
extern bool number_basetype(meosType basetype);
extern bool alphanum_basetype(meosType basetype);
extern bool geo_basetype(meosType basetype);
extern bool spatial_basetype(meosType basetype);

extern bool time_type(meosType timetype);
extern bool set_basetype(meosType basetype);

extern bool set_type(meosType type);
extern bool numset_type(meosType type);
extern bool timeset_type(meosType type);
extern bool set_span_type(meosType type);
extern bool alphanumset_type(meosType settype);
extern bool geoset_type(meosType type);
extern bool spatialset_type(meosType type);

extern bool span_basetype(meosType type);
extern bool span_canon_basetype(meosType type);
extern bool span_type(meosType type);
extern bool span_bbox_type(meosType type);
extern bool numspan_basetype(meosType type);
extern bool numspan_type(meosType type);
extern bool timespan_basetype(meosType type);
extern bool timespan_type(meosType type);

extern bool spanset_type(meosType type);
extern bool numspanset_type(meosType type);
extern bool timespanset_type(meosType type);

extern bool temporal_type(meosType temptype);
extern bool temporal_basetype(meosType basetype);
extern bool temptype_continuous(meosType temptype);
extern bool basetype_byvalue(meosType type);
extern bool basetype_varlength(meosType type);
extern int16 basetype_length(meosType basetype);
extern bool talphanum_type(meosType type);
extern bool talpha_type(meosType temptype);
extern bool tnumber_type(meosType temptype);
extern bool tnumber_basetype(meosType basetype);
extern bool tnumber_settype(meosType settype);
extern bool tnumber_spantype(meosType settype);
extern bool tnumber_spansettype(meosType spansettype);
extern bool tspatial_type(meosType temptype);
extern bool tspatial_basetype(meosType basetype);
extern bool tgeo_type(meosType basetype);

/*****************************************************************************/

#endif /* __MEOS_CATALOG_H__ */

