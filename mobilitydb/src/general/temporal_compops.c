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
 * @brief Temporal comparison operators: #=, #<>, #<, #>, #<=, #>=.
 */

#include "general/temporal_compops.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/temporal_catalog.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of the base value and the temporal
 * value.
 */
static Datum
tcomp_base_temporal_ext(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, mobdbType, mobdbType))
{
  Datum value = PG_GETARG_ANYDATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = tcomp_temporal_base(temp, value, basetype, func, INVERT);
  /* Restrict the result to the Boolean value in the fourth argument if any */
  if (result != NULL && restr)
  {
    Temporal *at_result = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = at_result;
  }
  DATUM_FREE_IF_COPY(value, basetype, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * @brief Return the temporal comparison of the temporal value and the base
 * value.
 */
Datum
tcomp_temporal_base_ext(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, mobdbType, mobdbType))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = tcomp_temporal_base(temp, value, basetype,
    func, INVERT_NO);
  /* Restrict the result to the Boolean value in the third argument if any */
  if (result != NULL && restr)
  {
    Temporal *at_result = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = at_result;
  }
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, basetype, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * @brief Return the temporal comparison of the temporal values.
 */
Datum
tcomp_temporal_temporal_ext(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, mobdbType, mobdbType))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = tcomp_temporal_temporal(temp1, temp2, func);
  /* Restrict the result to the Boolean value in the fourth argument if any */
  if (result != NULL && restr)
  {
    Temporal *at_result = temporal_restrict_value(result, atvalue, REST_AT);
    pfree(result);
    result = at_result;
  }
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal eq
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Teq_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the base value and the temporal value.
 * @sqlfunc temporal_teq()
 * @sqlop @p #=
 */
PGDLLEXPORT Datum
Teq_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal_ext(fcinfo, &datum2_eq2);
}

PG_FUNCTION_INFO_V1(Teq_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal value and the base value.
 * @sqlfunc temporal_teq()
 * @sqlop @p #=
 */
PGDLLEXPORT Datum
Teq_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base_ext(fcinfo, &datum2_eq2);
}

PG_FUNCTION_INFO_V1(Teq_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal values.
 * @sqlfunc temporal_teq()
 * @sqlop @p #=
 */
PGDLLEXPORT Datum
Teq_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal_ext(fcinfo, &datum2_eq2);
}

/*****************************************************************************
 * Temporal ne
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tne_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the base value and the temporal value.
 * @sqlfunc temporal_tne()
 * @sqlop @p #<>
 */
PGDLLEXPORT Datum
Tne_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal_ext(fcinfo, &datum2_ne2);
}

PG_FUNCTION_INFO_V1(Tne_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal value and the base value.
 * @sqlfunc temporal_tne()
 * @sqlop @p #<>
 */
PGDLLEXPORT Datum
Tne_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base_ext(fcinfo, &datum2_ne2);
}

PG_FUNCTION_INFO_V1(Tne_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal values.
 * @sqlfunc temporal_tne()
 * @sqlop @p #<>
 */
PGDLLEXPORT Datum
Tne_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal_ext(fcinfo, &datum2_ne2);
}

/*****************************************************************************
 * Temporal lt
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tlt_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the base value and the temporal value.
 * @sqlfunc temporal_tlt()
 * @sqlop @p #<
 */
PGDLLEXPORT Datum
Tlt_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal_ext(fcinfo, &datum2_lt2);
}

PG_FUNCTION_INFO_V1(Tlt_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal value and the base value.
 * @sqlfunc temporal_tlt()
 * @sqlop @p #<
 */
PGDLLEXPORT Datum
Tlt_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base_ext(fcinfo, &datum2_lt2);
}

PG_FUNCTION_INFO_V1(Tlt_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal values.
 * @sqlfunc temporal_tlt()
 * @sqlop @p #<
 */
PGDLLEXPORT Datum
Tlt_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal_ext(fcinfo, &datum2_lt2);
}

/*****************************************************************************
 * Temporal le
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tle_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the base value and the temporal value.
 * @sqlfunc temporal_tle()
 * @sqlop @p #<=
 */
PGDLLEXPORT Datum
Tle_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal_ext(fcinfo, &datum2_le2);
}

PG_FUNCTION_INFO_V1(Tle_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal value and the base value.
 * @sqlfunc temporal_tle()
 * @sqlop @p #<=
 */
PGDLLEXPORT Datum
Tle_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base_ext(fcinfo, &datum2_le2);
}

PG_FUNCTION_INFO_V1(Tle_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal values.
 * @sqlfunc temporal_tle()
 * @sqlop @p #<=
 */
PGDLLEXPORT Datum
Tle_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal_ext(fcinfo, &datum2_le2);
}

/*****************************************************************************
 * Temporal gt
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tgt_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the base value and the temporal value.
 * @sqlfunc temporal_tgt()
 * @sqlop @p #>
 */
PGDLLEXPORT Datum
Tgt_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal_ext(fcinfo, &datum2_gt2);
}

PG_FUNCTION_INFO_V1(Tgt_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal value and the base value.
 * @sqlfunc temporal_tgt()
 * @sqlop @p #>
 */
PGDLLEXPORT Datum
Tgt_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base_ext(fcinfo, &datum2_gt2);
}

PG_FUNCTION_INFO_V1(Tgt_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal values.
 * @sqlfunc temporal_tgt()
 * @sqlop @p #>
 */
PGDLLEXPORT Datum
Tgt_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal_ext(fcinfo, &datum2_gt2);
}

/*****************************************************************************
 * Temporal ge
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tge_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the base value and the temporal value.
 * @sqlfunc temporal_tge()
 * @sqlop @p #>=
 */
PGDLLEXPORT Datum
Tge_base_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_base_temporal_ext(fcinfo, &datum2_ge2);
}

PG_FUNCTION_INFO_V1(Tge_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal value and the base value.
 * @sqlfunc temporal_tge()
 * @sqlop @p #>=
 */
PGDLLEXPORT Datum
Tge_temporal_base(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_base_ext(fcinfo, &datum2_ge2);
}

PG_FUNCTION_INFO_V1(Tge_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return the temporal comparison of the temporal values.
 * @sqlfunc temporal_tge()
 * @sqlop @p #>=
 */
PGDLLEXPORT Datum
Tge_temporal_temporal(PG_FUNCTION_ARGS)
{
  return tcomp_temporal_temporal_ext(fcinfo, &datum2_ge2);
}

/*****************************************************************************/
