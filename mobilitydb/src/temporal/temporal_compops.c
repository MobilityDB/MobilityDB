/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @file
 * @brief Ever, always and temporal comparison operators
 * @details Ever comparison operators (?=, ?<>, ?<, ?>, ?<=, ?>=),
 * always comparison operators (%=, %<>, %<, %>, %<=, %>=), and
 * temporal comparison operators (#=, #<>, #<, #>, #<=, #>=)
 */

#include "temporal/temporal_compops.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal.h"
#include "temporal/type_util.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Generic function for the ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
EAcomp_base_temporal(FunctionCallInfo fcinfo,
  int (*func)(Datum, const Temporal *))
{
  Datum value = PG_GETARG_ANYDATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  MeosType basetype = temptype_basetype(temp->temptype);
  int result = func(value, temp);
  DATUM_FREE_IF_COPY(value, basetype, 0);
  PG_FREE_IF_COPY(temp, 1);
  /* Function func never return -1 since the calling SQL functions are STRICT */
  PG_RETURN_BOOL(result);}

/**
 * @brief Generic function for the ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
EAcomp_temporal_base(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, Datum))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  MeosType basetype = temptype_basetype(temp->temptype);
  int result = func(temp, value);
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, basetype, 1);
  /* Function func never return -1 since the calling SQL functions are STRICT */
  PG_RETURN_BOOL(result);}

/**
 * @brief Generic function for the ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
Datum
EAcomp_temporal_temporal(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Temporal *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = func(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is ever equal to a temporal value
 * @sqlfn eEqual()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &ever_eq_base_temporal);
}

PGDLLEXPORT Datum Always_eq_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is always equal to a temporal value
 * @sqlfn aEqual()
 * @sqlop @p %=
 */
inline Datum
Always_eq_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &always_eq_base_temporal);
}

PGDLLEXPORT Datum Ever_ne_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is ever different from a temporal value
 * @sqlfn eNotEqual()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &ever_ne_base_temporal);
}

PGDLLEXPORT Datum Always_ne_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is always different from a temporal value
 * @sqlfn aNotEqual()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &always_ne_base_temporal);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is ever equal to a base value
 * @sqlfn eEqual()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &ever_eq_temporal_base);
}

PGDLLEXPORT Datum Always_eq_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is always equal to a base value
 * @sqlfn aEqual()
 * @sqlop @p %=
 */
inline Datum
Always_eq_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &always_eq_temporal_base);
}

PGDLLEXPORT Datum Ever_ne_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is ever different from a base value
 * @sqlfn eNotEqual()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &ever_ne_temporal_base);
}

PGDLLEXPORT Datum Always_ne_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is always different from a base value
 * @sqlfn aNotEqual()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &always_ne_temporal_base);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_lt_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_lt_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is ever less than a temporal value
 * @sqlfn eLessThan()
 * @sqlop @p ?<
 */
inline Datum
Ever_lt_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &ever_lt_base_temporal);
}

PGDLLEXPORT Datum Always_lt_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_lt_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is always less than a temporal value
 * @sqlfn aLessThan()
 * @sqlop @p %<
 */
inline Datum
Always_lt_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &always_lt_base_temporal);
}

PGDLLEXPORT Datum Ever_le_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_le_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is ever less than or equal to a temporal
 * value
 * @sqlfn eLessEqual()
 * @sqlop @p ?<=
 */
inline Datum
Ever_le_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &ever_le_base_temporal);
}

PGDLLEXPORT Datum Always_le_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_le_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is always less than or equal to a
 * temporal value
 * @sqlfn aLessEqual()
 * @sqlop @p %<=
 */
inline Datum
Always_le_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &always_le_base_temporal);
}

PGDLLEXPORT Datum Ever_gt_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_gt_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is ever greater than a temporal value
 * @sqlfn eGreaterThan()
 * @sqlop @p ?>
 */
inline Datum
Ever_gt_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &ever_gt_base_temporal);
}

PGDLLEXPORT Datum Always_gt_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_gt_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is always greater than a temporal value
 * @sqlfn aGreaterThan()
 * @sqlop @p %>
 */
inline Datum
Always_gt_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &always_gt_base_temporal);
}

PGDLLEXPORT Datum Ever_ge_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ge_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is ever greater than or equal to a
 * temporal value
 * @sqlfn eGreaterEqual()
 * @sqlop @p ?>=
 */
inline Datum
Ever_ge_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &ever_ge_base_temporal);
}

PGDLLEXPORT Datum Always_ge_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ge_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a base value is always greater than or equal to a
 * temporal value
 * @sqlfn aGreaterEqual()
 * @sqlop @p %>=
 */
inline Datum
Always_ge_base_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_base_temporal(fcinfo, &always_ge_base_temporal);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_lt_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_lt_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is ever less than a base value
 * @sqlfn eLessThan()
 * @sqlop @p ?<
 */
inline Datum
Ever_lt_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &ever_lt_temporal_base);
}

PGDLLEXPORT Datum Always_lt_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_lt_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is always less than a base value
 * @sqlfn aLessThan()
 * @sqlop @p %<
 */
inline Datum
Always_lt_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &always_lt_temporal_base);
}

PGDLLEXPORT Datum Ever_le_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_le_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is ever less than or equal to a base
 * value
 * @sqlfn eLessEqual()
 * @sqlop @p ?<=
 */
inline Datum
Ever_le_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &ever_le_temporal_base);
}

PGDLLEXPORT Datum Always_le_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_le_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is always less than or equal to a
 * base value
 * @sqlfn aLessEqual()
 * @sqlop @p %<=
 */
inline Datum
Always_le_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &always_le_temporal_base);
}

PGDLLEXPORT Datum Ever_gt_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_gt_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is ever greater than a base value
 * @sqlfn eGreaterThan()
 * @sqlop @p ?>
 */
inline Datum
Ever_gt_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &ever_gt_temporal_base);
}

PGDLLEXPORT Datum Always_gt_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_gt_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is always greater than a base value
 * @sqlfn aGreaterThan()
 * @sqlop @p %>
 */
inline Datum
Always_gt_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &always_gt_temporal_base);
}

PGDLLEXPORT Datum Ever_ge_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ge_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is ever greater than or equal to a
 * base value
 * @sqlfn eGreaterEqual()
 * @sqlop @p ?>=
 */
inline Datum
Ever_ge_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &ever_ge_temporal_base);
}

PGDLLEXPORT Datum Always_ge_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ge_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal value is always greater than or equal to a
 * base value
 * @sqlfn aGreaterEqual()
 * @sqlop @p %>=
 */
inline Datum
Always_ge_temporal_base(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_base(fcinfo, &always_ge_temporal_base);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the temporal values are ever equal
 * @sqlfn eEqual()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_eq_temporal_temporal);
}

PGDLLEXPORT Datum Always_eq_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the temporal values are always equal
 * @sqlfn aEqual()
 * @sqlop @p %=
 */
inline Datum
Always_eq_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_eq_temporal_temporal);
}

PGDLLEXPORT Datum Ever_ne_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the temporal values are ever different
 * @sqlfn eNotEqual()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_ne_temporal_temporal);
}

PGDLLEXPORT Datum Always_ne_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the temporal values are always different
 * @sqlfn aNotEqual()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_ne_temporal_temporal);
}

PGDLLEXPORT Datum Ever_lt_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_lt_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the first temporal value is ever less than the second
 * one
 * @sqlfn eLessThan()
 * @sqlop @p ?<
 */
inline Datum
Ever_lt_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_lt_temporal_temporal);
}

PGDLLEXPORT Datum Always_lt_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_lt_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the first temporal value is always less than the
 * second one
 * @sqlfn aLessThan()
 * @sqlop @p %<
 */
inline Datum
Always_lt_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_lt_temporal_temporal);
}

PGDLLEXPORT Datum Ever_le_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_le_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the first temporal value is ever less than or equal to
 * the second one
 * @sqlfn eLessEqual()
 * @sqlop @p ?<=
 */
inline Datum
Ever_le_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_le_temporal_temporal);
}

PGDLLEXPORT Datum Always_le_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_le_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the first temporal value is always less than or equal
 * to the second one
 * @sqlfn aLessEqual()
 * @sqlop @p %<=
 */
inline Datum
Always_le_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_le_temporal_temporal);
}

PGDLLEXPORT Datum Ever_gt_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_gt_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the first temporal value is ever greater than the
 * second one
 * @sqlfn eGreaterThan()
 * @sqlop @p ?>
 */
inline Datum
Ever_gt_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_gt_temporal_temporal);
}

PGDLLEXPORT Datum Always_gt_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_gt_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the first temporal value is always greater than the
 * second one
 * @sqlfn aGreaterThan()
 * @sqlop @p %>
 */
inline Datum
Always_gt_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_gt_temporal_temporal);
}

PGDLLEXPORT Datum Ever_ge_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ge_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the first temporal value is ever greater than or equal
 * to the second one
 * @sqlfn eGreaterEqual()
 * @sqlop @p ?>=
 */
inline Datum
Ever_ge_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_ge_temporal_temporal);
}

PGDLLEXPORT Datum Always_ge_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ge_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if the first temporal value is always greater than or
 * equal to the second one
 * @sqlfn aGreaterEqual()
 * @sqlop @p %>=
 */
inline Datum
Always_ge_temporal_temporal(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_ge_temporal_temporal);
}

/*****************************************************************************
 * Generic functions for temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a base value and a temporal value
 */
static Datum
Tcomp_base_temporal(FunctionCallInfo fcinfo,
  Temporal * (*func)(Datum, const Temporal *))
{
  Datum value = PG_GETARG_ANYDATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  MeosType basetype = oid_meostype(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Temporal *result = func(value, temp);
  assert(result);
  DATUM_FREE_IF_COPY(value, basetype, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return the temporal comparison of a temporal value and a base value
 */
Datum
Tcomp_temporal_base(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, Datum))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  MeosType basetype = oid_meostype(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Temporal *result = func(temp, value);
  assert(result);
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, basetype, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return the temporal comparison of two temporal values
 */
Datum
Tcomp_temporal_temporal(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Temporal *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a base value is equal
 * to a temporal value
 * @sqlfn tEqual()
 * @sqlop @p #=
 */
inline Datum
Teq_base_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_base_temporal(fcinfo, &teq_base_temporal);
}

PGDLLEXPORT Datum Tne_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a base value is
 * different from a temporal value
 * @sqlfn tNotEqual()
 * @sqlop @p #<>
 */
inline Datum
Tne_base_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_base_temporal(fcinfo, &tne_base_temporal);
}

PGDLLEXPORT Datum Tlt_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tlt_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a base value is less
 * than a temporal value
 * @sqlfn tLessThan()
 * @sqlop @p #<
 */
inline Datum
Tlt_base_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_base_temporal(fcinfo, &tlt_base_temporal);
}

PGDLLEXPORT Datum Tle_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tle_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a base value is less
 * than or equal to a temporal value
 * @sqlfn tLessEqual()
 * @sqlop @p #<=
 */
inline Datum
Tle_base_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_base_temporal(fcinfo, &tle_base_temporal);
}

PGDLLEXPORT Datum Tgt_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgt_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a base value is greater
 * than a temporal value
 * @sqlfn tGreaterThan()
 * @sqlop @p #>
 */
inline Datum
Tgt_base_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_base_temporal(fcinfo, &tgt_base_temporal);
}

PGDLLEXPORT Datum Tge_base_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tge_base_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a base value is greater
 * than or equal to a temporal value
 * @sqlfn tGreaterEqual()
 * @sqlop @p #>=
 */
inline Datum
Tge_base_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_base_temporal(fcinfo, &tge_base_temporal);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a temporal value is
 * equal to a base value
 * @sqlfn tEqual()
 * @sqlop @p #=
 */
inline Datum
Teq_temporal_base(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_base(fcinfo, &teq_temporal_base);
}

PGDLLEXPORT Datum Tne_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a temporal value is
 * different from a base value
 * @sqlfn tNotEqual()
 * @sqlop @p #<>
 */
inline Datum
Tne_temporal_base(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_base(fcinfo, &tne_temporal_base);
}

PGDLLEXPORT Datum Tlt_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tlt_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a temporal value is
 * less than a base value
 * @sqlfn tLessThan()
 * @sqlop @p #<
 */
inline Datum
Tlt_temporal_base(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_base(fcinfo, &tlt_temporal_base);
}

PGDLLEXPORT Datum Tle_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tle_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a temporal value is
 * less than or equal to a base value
 * @sqlfn tLessEqual()
 * @sqlop @p #<=
 */
inline Datum
Tle_temporal_base(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_base(fcinfo, &tle_temporal_base);
}

PGDLLEXPORT Datum Tgt_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgt_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a temporal value is
 * greater than a base value
 * @sqlfn tGreaterThan()
 * @sqlop @p #>
 */
inline Datum
Tgt_temporal_base(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_base(fcinfo, &tgt_temporal_base);
}

PGDLLEXPORT Datum Tge_temporal_base(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tge_temporal_base);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a temporal value is
 * greater than or equal to a base value
 * @sqlfn tGreaterEqual()
 * @sqlop @p #>=
 */
inline Datum
Tge_temporal_base(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_base(fcinfo, &tge_temporal_base);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether the first temporal
 * value is equal to the second one
 * @sqlfn tEqual()
 * @sqlop @p #=
 */
inline Datum
Teq_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_temporal(fcinfo, &teq_temporal_temporal);
}

PGDLLEXPORT Datum Tne_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether the first temporal
 * value is different from the second one
 * @sqlfn tNotEqual()
 * @sqlop @p #<>
 */
inline Datum
Tne_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_temporal(fcinfo, &tne_temporal_temporal);
}

PGDLLEXPORT Datum Tlt_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tlt_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether the first temporal
 * value is less than the second one
 * @sqlfn tLessThan()
 * @sqlop @p #<
 */
inline Datum
Tlt_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_temporal(fcinfo, &tlt_temporal_temporal);
}

PGDLLEXPORT Datum Tle_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tle_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether the first temporal
 * value is less than or equal to second one
 * @sqlfn tLessEqual()
 * @sqlop @p #<=
 */
inline Datum
Tle_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_temporal(fcinfo, &tle_temporal_temporal);
}

PGDLLEXPORT Datum Tgt_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgt_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether a temporal value is
 * greater than the second one
 * @sqlfn tGreaterThan()
 * @sqlop @p #>
 */
inline Datum
Tgt_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_temporal(fcinfo, &tgt_temporal_temporal);
}
PGDLLEXPORT Datum Tge_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tge_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return a temporal boolean that states whether the first temporal
 * value is greater than or equal to the second one
 * @sqlfn tGreaterEqual()
 * @sqlop @p #>=
 */
inline Datum
Tge_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_temporal(fcinfo, &tge_temporal_temporal);
}

/*****************************************************************************/
