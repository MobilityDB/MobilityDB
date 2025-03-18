/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Basic functions for temporal types of any subtype.
 */

#ifndef __PG_TEMPORAL_H__
#define __PG_TEMPORAL_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type_d.h> /* for TIMESTAMPTZOID and similar */
#include <lib/stringinfo.h>
#include <utils/array.h>
#include <utils/lsyscache.h>
/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h"

/*****************************************************************************/


// #if POSTGRESQL_VERSION_NUMBER < 130000
// #if USE_FLOAT4_BYVAL
// #error Postgres needs to be configured with USE_FLOAT4_BYVAL
// #endif
// #endif

// #if USE_FLOAT8_BYVAL
// #error Postgres needs to be configured with USE_FLOAT8_BYVAL
// #endif

/* To avoid including fmgrprotos.h */
extern Datum numeric_float8(PG_FUNCTION_ARGS);
extern Datum numeric_round(PG_FUNCTION_ARGS);
extern Datum float8_numeric(PG_FUNCTION_ARGS);

#define PG_GETARG_ANYDATUM(X) (get_typlen(get_fn_expr_argtype(fcinfo->flinfo, X)) == -1 ? \
  PointerGetDatum(PG_GETARG_VARLENA_P(X)) : PG_GETARG_DATUM(X))

/*****************************************************************************
 * Generic GIN operator strategy numbers independent of the argument types
 *****************************************************************************/

#define GinOverlapStrategy             1    /* for && @@ */
#define GinContainsStrategy            2    /* for @> @? */
#define GinContainedStrategy           3    /* for <@ ?@ */
#define GinEqualStrategy               4    /* for =  @=*/

/*****************************************************************************
 * Struct definitions for the unnest operation
 *****************************************************************************/

/**
 * Structure to represent the state when unnesting a temporal type.
 */
typedef struct
{
  bool done;
  int i;
  int count;
  Temporal *temp;  /* Temporal value to unnest */
  Datum *values;   /* Values obtained by getValues(temp) */
} TempUnnestState;

/*****************************************************************************
 * Struct definitions for GisT indexes copied from PostgreSQL
 *****************************************************************************/

/**
 * Structure to represent information about an entry that can be placed
 * to either group without affecting overlap over selected axis ("common entry").
 */
typedef struct
{
  /* Index of entry in the initial array */
  int      index;
  /* Delta between penalties of entry insertion into different groups */
  double    delta;
} CommonEntry;

/**
 * Structure to represent a projection of bounding box to an axis.
 */
typedef struct
{
  double    lower,
            upper;
} SplitInterval;

/*****************************************************************************
 * Definitions for GiST indexes
 *****************************************************************************/

/* Minimum accepted ratio of split */
#define LIMIT_RATIO 0.3

/* Convenience macros for NaN-aware comparisons */
#define FLOAT8_EQ(a,b)   (float8_cmp_internal(a, b) == 0)
#define FLOAT8_LT(a,b)   (float8_cmp_internal(a, b) < 0)
#define FLOAT8_LE(a,b)   (float8_cmp_internal(a, b) <= 0)
#define FLOAT8_GT(a,b)   (float8_cmp_internal(a, b) > 0)
#define FLOAT8_GE(a,b)   (float8_cmp_internal(a, b) >= 0)
#define FLOAT8_MAX(a,b)  (FLOAT8_GT(a, b) ? (a) : (b))
#define FLOAT8_MIN(a,b)  (FLOAT8_LT(a, b) ? (a) : (b))

/*****************************************************************************
 * Struct definitions for SP-GiST indexes
 *****************************************************************************/

/** Enumeration for the types of SP-GiST indexes */
typedef enum
{
  SPGIST_QUADTREE,
  SPGIST_KDTREE,
} SPGistIndexType;

/*****************************************************************************
 * Typmod definitions
 *****************************************************************************/

#define TYPMOD_MAXLEN 64

/*
 * MobilityDB reuses the typmod definitions from PostGIS using the two spare
 * bits left for storing the temporal subtype.
 *
 * The 'typmod' is an int32_t used as follows:
 * Plus/minus = Top bit.
 * Spare bits = Next 2 bits -> Used in MobilityDB to store the temporal subtype
 * SRID = Next 21 bits.
 * TYPE = Next 6 bits.
 * ZM Flags = Bottom 2 bits.
 *
#define TYPMOD_GET_SRID(typmod) ((((typmod) & 0x0FFFFF00) - ((typmod) & 0x10000000)) >> 8)
#define TYPMOD_SET_SRID(typmod, srid) ((typmod) = (((typmod) & 0xE00000FF) | ((srid & 0x001FFFFF)<<8)))
#define TYPMOD_GET_TYPE(typmod) ((typmod & 0x000000FC)>>2)
#define TYPMOD_SET_TYPE(typmod, type) ((typmod) = (typmod & 0xFFFFFF03) | ((type & 0x0000003F)<<2))
#define TYPMOD_GET_Z(typmod) ((typmod & 0x00000002)>>1)
#define TYPMOD_SET_Z(typmod) ((typmod) = typmod | 0x00000002)
#define TYPMOD_GET_M(typmod) (typmod & 0x00000001)
#define TYPMOD_SET_M(typmod) ((typmod) = typmod | 0x00000001)
#define TYPMOD_GET_NDIMS(typmod) (2+TYPMOD_GET_Z(typmod)+TYPMOD_GET_M(typmod))
 *
 */

/* Get/set the temporal subtype from the 2 spare bits from PostGIS */
#define TYPMOD_GET_TEMPSUBTYPE(typmod) ((typmod & 0x60000000)>>29)
#define TYPMOD_SET_TEMPSUBTYPE(typmod, tempsubtype) ((typmod) = (((typmod) & 0x9FFFFFFF) | ((tempsubtype & 0x00000003)<<29)))

/**
 * @brief Return the size in bytes to read from toast to get the basic
 * information from a variable-length time type: Time struct (i.e., Set
 * or SpanSet) and bounding box size
*/
#define TIME_MAX_HEADER_SIZE DOUBLE_PAD(Max(sizeof(Set), sizeof(SpanSet)))

/*****************************************************************************
 * Miscellaneous
 *****************************************************************************/

/* Initialization function */

extern void _PG_init(void);

/* Header size in bytes for time types to read from toast */

extern uint32_t time_max_header_size(void);

/* PostgreSQL cache functions */

extern FunctionCallInfo fetch_fcinfo(void);
extern void store_fcinfo(FunctionCallInfo fcinfo);

/* Send/receive functions */

extern Temporal *temporal_recv(StringInfo buf);
extern void temporal_write(const Temporal *temp, StringInfo buf);

extern bytea *Datum_as_wkb(FunctionCallInfo fcinfo, Datum value, meosType type,
  bool extended);
extern text *Datum_as_hexwkb(FunctionCallInfo fcinfo, Datum value,
  meosType type);

/* Parameter tests */

extern bool ensure_not_empty_array(ArrayType *array);

/* Indexing functions */

extern Temporal *temporal_slice(Datum tempdatum);

/*****************************************************************************/

#endif /* __PG_TEMPORAL_H__ */
