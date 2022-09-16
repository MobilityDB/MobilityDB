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

#ifndef __TEMPORAL_CATALOG_H__
#define __TEMPORAL_CATALOG_H__

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
  // T_UNKNOWN = 0,
  T_BOOL,          /**< boolean type */
  T_DOUBLE2,       /**< double2 type */
  T_DOUBLE3,       /**< double3 type */
  T_DOUBLE4,       /**< double4 type */
  T_FLOAT8,        /**< float8 type */
  T_FLOATSPAN,     /**< float8 span type */
  T_INT4,          /**< int4 type */
#if ! MEOS
  T_INT4RANGE,     /**< PostgreSQL int4 range type */
#endif
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
#if ! MEOS
  T_TSTZRANGE,     /**< PostgreSQL timestamp with time zone range type */
#endif
  T_TTEXT,         /**< temporal text type */
  T_GEOMETRY,      /**< geometry type */
  T_GEOGRAPHY,     /**< geography type */
  T_TGEOMPOINT,    /**< temporal geometry point type */
  T_TGEOGPOINT,    /**< temporal geography point type */
#if NPOINT
  T_NPOINT,        /**< network point type */
  T_NSEGMENT,      /**< network segment type */
  T_TNPOINT,       /**< temporal network point type */
#endif
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
  mobdbType spantype;    /**< Enum value of the span type */
  mobdbType basetype;    /**< Enum value of the base type */
} spantype_cache_struct;

/*****************************************************************************/

/* Cache functions */

extern mobdbType temptype_basetype(mobdbType temptype);
extern mobdbType spantype_basetype(mobdbType spantype);
extern mobdbType basetype_spantype(mobdbType basetype);

/* Catalog functions */

extern bool time_type(mobdbType timetype);
extern void ensure_time_type(mobdbType timetype);
extern bool span_type(mobdbType spantype);
extern void ensure_span_type(mobdbType spantype);
extern bool span_basetype(mobdbType basetype);
extern void ensure_span_basetype(mobdbType basetype);
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
extern bool tnumber_spantype(mobdbType spantype);
extern void ensure_tnumber_spantype(mobdbType spantype);
extern bool tspatial_type(mobdbType temptype);
extern bool tspatial_basetype(mobdbType basetype);
extern bool tgeo_basetype(mobdbType basetype);
extern bool tgeo_type(mobdbType basetype);
extern void ensure_tgeo_type(mobdbType basetype);

/*****************************************************************************/

#endif /* __TEMPORAL_CATALOG_H__ */

