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
 * @brief Functions for building a cache of type and operator Oids.
 */

#ifndef __PG_TEMPORAL_CATALOG_H__
#define __PG_TEMPORAL_CATALOG_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "meos.h"
#include "general/temporal_catalog.h"
/* MobilityDB */
#include "pg_general/temporal_catalog.h"

/*****************************************************************************/

/**
 * Enumeration that defines the classes of Boolean operators used in
 * MobilityDB.
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

/*****************************************************************************/

/* MobilityDB functions */

extern Oid type_oid(mobdbType t);
extern Oid oper_oid(CachedOp op, mobdbType lt, mobdbType rt);
extern mobdbType oid_type(Oid typid);

/*****************************************************************************/

#endif /* __PG_TEMPORAL_CATALOG_H__ */

