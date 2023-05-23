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
 * @brief Basic functions for temporal types of any subtype.
 */

#ifndef __PG_TEMPORAL_H__
#define __PG_TEMPORAL_H__

/* PostgreSQL */
#include <postgres.h>
#include <lib/stringinfo.h>
/* MEOS */
#include "general/meos_catalog.h"
#include "general/span.h"
#include "general/set.h"
#include "general/tbox.h"
#include "point/stbox.h"

/*****************************************************************************/

#include <c.h>
#include <utils/palloc.h>
#include <utils/elog.h>
#include <utils/array.h>
#include <utils/lsyscache.h>
#include <catalog/pg_type_d.h> /* for TIMESTAMPTZOID and similar */
#include "pg_point/postgis.h"

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
 * Additional operator strategy numbers used in the GiST and SP-GiST temporal
 * opclasses with respect to those defined in the file stratnum.h
 *****************************************************************************/

#define RTOverBeforeStrategyNumber    28    /* for &<# */
#define RTBeforeStrategyNumber        29    /* for <<# */
#define RTAfterStrategyNumber         30    /* for #>> */
#define RTOverAfterStrategyNumber     31    /* for #&> */
#define RTOverFrontStrategyNumber     32    /* for &</ */
#define RTFrontStrategyNumber         33    /* for <</ */
#define RTBackStrategyNumber          34    /* for />> */
#define RTOverBackStrategyNumber      35    /* for /&> */

/*****************************************************************************
 * Generic oGIN perator strategy numbers indepenedent of the argument types
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

#define TYPMOD_GET_SUBTYPE(typmod) ((int16) ((typmod == -1) ? (0) : (typmod & 0x0000000F)))

/**
 * Structure to represent the temporal subtype array
 */
struct tempsubtype_struct
{
  char *subtypeName;   /**< string representing the temporal type */
  int16 subtype;       /**< subtype */
};

#define TEMPSUBTYPE_STRUCT_ARRAY_LEN \
  (sizeof tempsubtype_struct_array/sizeof(struct tempsubtype_struct))
#define TEMPSUBTYPE_MAX_LEN   13

/* Initialization function */

extern void _PG_init(void);

/* Miscellaneous */

extern uint32_t time_max_header_size(void);

/* PostgreSQL cache functions */

extern FunctionCallInfo fetch_fcinfo(void);
extern void store_fcinfo(FunctionCallInfo fcinfo);

/* Typmod functions */

extern const char *tempsubtype_name(int16 subtype);
extern bool tempsubtype_from_string(const char *str, int16 *subtype);

/* Send/receive functions */

extern Temporal *temporal_recv(StringInfo buf);
extern void temporal_write(const Temporal *temp, StringInfo buf);

/* Parameter tests */

extern void ensure_non_empty_array(ArrayType *array);

/*****************************************************************************/

#endif /* __PG_TEMPORAL_H__ */
