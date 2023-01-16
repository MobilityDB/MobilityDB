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
 * @brief Aggregate functions for set types.
 */

// #include "general/time_aggfuncs.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/memutils.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/skiplist.h"
/* MobilityDB */
#include "pg_general/skiplist.h"
#include "pg_general/temporal.h"
#include "pg_general/meos_catalog.h"

/*****************************************************************************
 * Aggregate transition functions for set types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Set_agg_transfn);
/**
 * Transition function for set aggregation of values
 */
PGDLLEXPORT Datum
Set_agg_transfn(PG_FUNCTION_ARGS)
{
  // MemoryContext ctx = set_aggregation_context(fcinfo);
  Set *state = PG_ARGISNULL(0) ? NULL : PG_GETARG_SET_P(0);
  if (PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  void *value = NULL;
  if (basetype_varlength(basetype) &&
      PG_DATUM_NEEDS_DETOAST((struct varlena *) d))
    value = PG_DETOAST_DATUM(d);
  state = set_agg_transfn(state, value ? PointerGetDatum(value) : d, basetype);
  if (! state)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(state);
}

/*****************************************************************************
 * Aggregate combine functions for set types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Set_agg_combinefn);
/**
 * Combine function for set aggregate of set types
 */
PGDLLEXPORT Datum
Set_agg_combinefn(PG_FUNCTION_ARGS)
{
  MemoryContext ctx = set_aggregation_context(fcinfo);
  Set *state1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_SET_P(0);
  Set *state2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_SET_P(1);
  if (state1 == NULL && state2 == NULL)
    PG_RETURN_NULL();
  unset_aggregation_context(ctx);
  store_fcinfo(fcinfo);
  Set *result = set_agg_combinefn(state1, state2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
