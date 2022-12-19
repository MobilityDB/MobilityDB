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
 * @brief Functions for building a cache of temporal types and operators.
 */

#ifndef __MOBDB_CATALOG_H__
#define __MOBDB_CATALOG_H__

/* PostgreSQL */
#include <postgres.h>

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
  T_PERIOD         = 19,  /**< period type */
  T_PERIODSET      = 20,  /**< period set type */
  T_STBOX          = 21,  /**< spatiotemporal box type */
  T_TBOOL          = 22,  /**< temporal boolean type */
  T_TBOX           = 23,  /**< temporal box type */
  T_TDOUBLE2       = 24,  /**< temporal double2 type */
  T_TDOUBLE3       = 25,  /**< temporal double3 type */
  T_TDOUBLE4       = 26,  /**< temporal double4 type */
  T_TEXT           = 27,  /**< text type */
  T_TFLOAT         = 28,  /**< temporal float type */
  T_TIMESTAMPSET   = 29,  /**< timestamp set type */
  T_TIMESTAMPTZ    = 30,  /**< timestamp with time zone type */
  T_TINT           = 31,  /**< temporal integer type */
  T_TSTZRANGE      = 32,  /**< PostgreSQL timestamp with time zone range type */
  T_TSTZMULTIRANGE = 33,  /**< PostgreSQL timestamp with time zone multirange type */
  T_TTEXT          = 34,  /**< temporal text type */
  T_GEOMETRY       = 35,  /**< geometry type */
  T_GEOGRAPHY      = 36,  /**< geography type */
  T_TGEOMPOINT     = 37,  /**< temporal geometry point type */
  T_TGEOGPOINT     = 38,  /**< temporal geography point type */
  T_NPOINT         = 39,  /**< network point type */
  T_NSEGMENT       = 40,  /**< network segment type */
  T_TNPOINT        = 41,  /**< temporal network point type */
} mobdbType;

/**
 * Structure to represent the temporal type cache array.
 */
typedef struct
{
  mobdbType temptype;    /**< Enum value of the temporal type */
  mobdbType basetype;    /**< Enum value of the base type */
} temptype_cache_struct;

/**
 * Structure to represent the span type cache array.
 */
typedef struct
{
  mobdbType settype;     /**< Enum value of the set type */
  mobdbType basetype;    /**< Enum value of the base type */
} settype_cache_struct;

/**
 * Structure to represent the span type cache array.
 */
typedef struct
{
  mobdbType spantype;    /**< Enum value of the span type */
  mobdbType basetype;    /**< Enum value of the base type */
} spantype_cache_struct;

/**
 * Structure to represent the spanset type cache array.
 */
typedef struct
{
  mobdbType spansettype;    /**< Enum value of the span type */
  mobdbType spantype;       /**< Enum value of the base type */
} spansettype_cache_struct;

/*****************************************************************************/

/* Cache functions */

extern mobdbType temptype_basetype(mobdbType temptype);
extern mobdbType settype_basetype(mobdbType settype);
extern mobdbType spantype_basetype(mobdbType spantype);
extern mobdbType spantype_spansettype(mobdbType spantype);
extern mobdbType spansettype_spantype(mobdbType spansettype);
extern mobdbType basetype_spantype(mobdbType basetype);
extern mobdbType basetype_settype(mobdbType basetype);

/* Catalog functions */

extern bool time_type(mobdbType timetype);
extern void ensure_time_type(mobdbType timetype);
extern bool set_basetype(mobdbType basetype);
extern void ensure_set_basetype(mobdbType basetype);

extern bool set_type(mobdbType settype);
extern void ensure_set_type(mobdbType settype);
extern bool numset_type(mobdbType settype);
extern void ensure_numset_type(mobdbType settype);

extern bool span_type(mobdbType spantype);
extern void ensure_span_type(mobdbType spantype);
extern bool numspan_type(mobdbType spantype);
extern void ensure_numspan_type(mobdbType spantype);
extern bool span_basetype(mobdbType basetype);
extern void ensure_span_basetype(mobdbType basetype);
extern bool numspan_basetype(mobdbType basetype);
extern void ensure_numspan_basetype(mobdbType basetype);
extern bool spanset_type(mobdbType spansettype);
extern void ensure_spanset_type(mobdbType spansettype);
extern bool numspanset_type(mobdbType spansettype);
extern void ensure_numspanset_type(mobdbType spansettype);
extern bool spanset_basetype(mobdbType basetype);
extern void ensure_spanset_basetype(mobdbType basetype);
extern bool numspanset_basetype(mobdbType basetype);
extern void ensure_numspanset_basetype(mobdbType basetype);
extern bool temporal_type(mobdbType temptype);
extern void ensure_temporal_type(mobdbType temptype);
extern void ensure_temporal_basetype(mobdbType basetype);
extern bool temptype_continuous(mobdbType temptype);
extern void ensure_temptype_continuous(mobdbType temptype);
extern bool basetype_byvalue(mobdbType basetype);
extern int16 basetype_length(mobdbType basetype);
extern bool talpha_type(mobdbType temptype);
extern bool tnumber_type(mobdbType temptype);
extern void ensure_tnumber_type(mobdbType temptype);
extern bool tnumber_basetype(mobdbType basetype);
extern void ensure_tnumber_basetype(mobdbType basetype);
extern bool tnumber_settype(mobdbType settype);
extern void ensure_tnumber_settype(mobdbType settype);
extern bool tnumber_spantype(mobdbType settype);
extern void ensure_tnumber_spantype(mobdbType spantype);
extern bool tnumber_spansettype(mobdbType spansettype);
extern void ensure_tnumber_spansettype(mobdbType spansettype);
extern bool tspatial_type(mobdbType temptype);
extern bool tspatial_basetype(mobdbType basetype);
extern bool tgeo_basetype(mobdbType basetype);
extern bool tgeo_type(mobdbType basetype);
extern void ensure_tgeo_type(mobdbType basetype);

/*****************************************************************************/

#endif /* __MOBDB_CATALOG_H__ */

