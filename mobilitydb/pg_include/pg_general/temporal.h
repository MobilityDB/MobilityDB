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
#include <general/temporal.h>
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

#define TYPMOD_GET_SUBTYPE(typmod) \
  ((int16) ((typmod == -1) ? (0) : (typmod & 0x0000000F)))

/* Initialization function */

extern void _PG_init(void);

/* Miscellaneous */

extern uint32_t time_max_header_size(void);

/* PostgreSQL cache functions */

extern FunctionCallInfo fetch_fcinfo(void);
extern void store_fcinfo(FunctionCallInfo fcinfo);

/* Send/receive functions */

extern Temporal *temporal_recv(StringInfo buf);
extern void temporal_write(const Temporal *temp, StringInfo buf);

/* Parameter tests */

extern bool ensure_not_empty_array(ArrayType *array);

/* Indexing functions */

extern Temporal *temporal_slice(Datum tempdatum);

/*****************************************************************************/

#endif /* __PG_TEMPORAL_H__ */
