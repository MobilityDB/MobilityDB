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
 * @brief Functions for selectivity estimation of operators on temporal number
 * types.
 */

#include "pg_general/tnumber_selfuncs.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <access/htup_details.h>
#include <catalog/pg_collation_d.h>
#include <utils/float.h>
#include <utils/selfuncs.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/timestampset.h"
#include "general/periodset.h"
/* MobilityDB */
#include "pg_general/span_selfuncs.h"
#include "pg_general/temporal_analyze.h"
#include "pg_general/temporal_catalog.h"
#include "pg_general/temporal_selfuncs.h"

/*****************************************************************************
 * Internal functions computing selectivity
 * The functions assume that the value and time dimensions of temporal values
 * are independent and thus the selectivity values obtained by analyzing the
 * histograms for each dimension can be multiplied.
 *****************************************************************************/

/**
 * Return the enum value associated to the operator
 */
bool
tnumber_cachedop(Oid operid, CachedOp *cachedOp)
{
  for (int i = LT_OP; i <= OVERAFTER_OP; i++)
  {
    if (/* Time types */
        operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TFLOAT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TFLOAT) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_TFLOAT) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_TFLOAT) ||
        /* Span types */
        operid == oper_oid((CachedOp) i, T_INTSPAN, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_FLOATSPAN, T_TFLOAT) ||
        /* Tbox type */
        operid == oper_oid((CachedOp) i, T_TBOX, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_TBOX, T_TFLOAT) ||
        /* Tint type */
        operid == oper_oid((CachedOp) i, T_TINT, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_INT4) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_FLOAT8) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_INTSPAN) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_TBOX) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_TFLOAT) ||
        /* Tfloat type */
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_INT4) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_FLOAT8) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_FLOATSPAN) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_TBOX) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_TFLOAT))
      {
        *cachedOp = (CachedOp) i;
        return true;
      }
  }
  return false;
}

/**
 * Transform the constant into a temporal box
 */
bool
tnumber_const_to_span_period(const Node *other, Span **s, Period **p,
  mobdbType basetype)
{
  Oid consttypid = ((Const *) other)->consttype;
  mobdbType type = oid_type(consttypid);
  if (tnumber_basetype(type))
  {
    Datum value = ((Const *) other)->constvalue;
    *s = elem_to_span(value, type);
  }
  else if (type == T_INTSPAN || type == T_FLOATSPAN)
  {
    Span *span = DatumGetSpanP(((Const *) other)->constvalue);
    *s = span_copy(span);
  }
  else if (type == T_TIMESTAMPTZ)
  {
    TimestampTz t = DatumGetTimestampTz(((Const *) other)->constvalue);
    *p = timestamp_to_period(t);
  }
  else if (type == T_TIMESTAMPSET)
  {
    *p = palloc(sizeof(Period));
    timestampset_period_slice(((Const *) other)->constvalue, *p);
  }
  else if (type == T_PERIOD)
  {
    Period *period = DatumGetSpanP(((Const *) other)->constvalue);
    *p = span_copy(period);
  }
  else if (type == T_PERIODSET)
  {
    *p = palloc(sizeof(Period));
    periodset_period_slice(((Const *) other)->constvalue, *p);
  }
  else if (type == T_TBOX)
  {
    const TBOX *box = DatumGetTboxP(((Const *) other)->constvalue);
    ensure_span_basetype(basetype);
    *s = tbox_to_floatspan(box);
    *p = tbox_to_period(box);
  }
  else if (tnumber_type(type))
  {
    const Temporal *temp = DatumGetTemporalP(((Const *) other)->constvalue);
    TBOX box;
    temporal_set_bbox(temp, &box);
    *s = tbox_to_floatspan(&box);
    *p = tbox_to_period(&box);
  }
  else
    return false;
  return true;
}

/**
 * Return a default selectivity estimate for the operator when we don't
 * have statistics or cannot use them for some reason.
 */
float8
tnumber_sel_default(CachedOp operator)
{
  switch (operator)
  {
    case OVERLAPS_OP:
      return 0.005;

    case CONTAINS_OP:
    case CONTAINED_OP:
      return 0.002;

    case SAME_OP:
      return 0.001;

    case LEFT_OP:
    case RIGHT_OP:
    case OVERLEFT_OP:
    case OVERRIGHT_OP:
    case ABOVE_OP:
    case BELOW_OP:
    case OVERABOVE_OP:
    case OVERBELOW_OP:
    case AFTER_OP:
    case BEFORE_OP:
    case OVERAFTER_OP:
    case OVERBEFORE_OP:

      /* these are similar to regular scalar inequalities */
      return DEFAULT_INEQ_SEL;

    default:
      /* all operators should be handled above, but just in case */
      return 0.001;
  }
}

/**
 * Return an estimate of the selectivity of the temporal search box and the
 * operator for columns of temporal numbers. For the traditional comparison
 * operators (<, <=, ...) we follow the approach for span types in
 * PostgreSQL, this function computes the selectivity for <, <=, >, and >=,
 * while the selectivity functions for = and <> are eqsel and neqsel,
 * respectively.
 */
Selectivity
tnumber_sel_span_period(VariableStatData *vardata, Span *span, Period *period,
  CachedOp cachedOp, Oid basetypid)
{
  double selec;
  Oid value_oprid, period_oprid;

  /* Enable the multiplication of the selectivity of the value and time
   * dimensions since either may be missing */
  selec = 1.0;

  /*
   * There is no ~= operator for span/time types and thus it is necessary to
   * take care of this operator here.
   */
  if (cachedOp == SAME_OP)
  {
    /* Selectivity for the value dimension */
    if (span != NULL)
    {
      value_oprid = oper_oid(EQ_OP, basetypid, basetypid);
#if POSTGRESQL_VERSION_NUMBER < 130000
      selec *= var_eq_const(vardata, value_oprid, PointerGetDatum(span),
        false, false, false);
#else
      selec *= var_eq_const(vardata, value_oprid, DEFAULT_COLLATION_OID,
        PointerGetDatum(span), false, false, false);
#endif
    }
    /* Selectivity for the time dimension */
    if (period != NULL)
    {
      period_oprid = oper_oid(EQ_OP, T_PERIOD, T_PERIOD);
#if POSTGRESQL_VERSION_NUMBER < 130000
      selec *= var_eq_const(vardata, period_oprid, SpanPGetDatum(period),
        false, false, false);
#else
      selec *= var_eq_const(vardata, period_oprid, DEFAULT_COLLATION_OID,
        SpanPGetDatum(period), false, false, false);
#endif
    }
  }
  else if (cachedOp == OVERLAPS_OP || cachedOp == CONTAINS_OP ||
    cachedOp == CONTAINED_OP ||
    /* For b-tree comparisons, temporal values are first compared wrt
     * their bounding boxes, and if these are equal, other criteria apply.
     * For selectivity estimation we approximate by taking into account
     * only the bounding boxes. */
    cachedOp == LT_OP || cachedOp == LE_OP ||
    cachedOp == GT_OP || cachedOp == GE_OP)
  {
    /* Selectivity for the value dimension */
    if (span != NULL)
      selec *= span_sel_hist(vardata, span, cachedOp, SPANSEL);
    /* Selectivity for the time dimension */
    if (period != NULL)
      /* Cast the period as a span to call the span selectivity functions */
      selec *= span_sel_hist(vardata, (Span *) period, cachedOp, PERIODSEL);
  }
  else if (cachedOp == LEFT_OP || cachedOp == RIGHT_OP ||
    cachedOp == OVERLEFT_OP || cachedOp == OVERRIGHT_OP)
  {
    /* Selectivity for the value dimension */
    if (span != NULL)
      selec *= span_sel_hist(vardata, span, cachedOp, SPANSEL);
  }
  else if (cachedOp == BEFORE_OP || cachedOp == AFTER_OP ||
    cachedOp == OVERBEFORE_OP || cachedOp == OVERAFTER_OP)
  {
    /* Selectivity for the value dimension */
    if (period != NULL)
      /* Cast the period as a span to call the span selectivity functions */
      selec *= span_sel_hist(vardata, (Span *) period, cachedOp, PERIODSEL);
  }
  else /* Unknown operator */
  {
    selec = tnumber_sel_default(cachedOp);
  }
  return selec;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tnumber_sel);
/**
 * Estimate the selectivity value of the operators for temporal numbers
 */
PGDLLEXPORT Datum
Tnumber_sel(PG_FUNCTION_ARGS)
{
  return temporal_sel_ext(fcinfo, TNUMBERTYPE);
}

/*****************************************************************************/

/**
 * Return a default join selectivity estimate for given operator, when we
 * don't have statistics or cannot use them for some reason.
 */
float8
tnumber_joinsel_default(CachedOp cachedOp __attribute__((unused)))
{
  // TODO take care of the operator
  return 0.001;
}

/**
 * Depending on the operator and the arguments, determine wheter the value,
 * the time, or both components are taken into account for computing the
 * join selectivity
 */
bool
tnumber_joinsel_components(CachedOp cachedOp, mobdbType oprleft,
  mobdbType oprright, bool *value, bool *time)
{
  /* Get the argument which may not a temporal number */
  mobdbType arg = tnumber_type(oprleft) ? oprright : oprleft;

  /* Determine the components */
  if (tnumber_basetype(arg) || tnumber_spantype(arg) ||
    cachedOp == LEFT_OP || cachedOp == OVERLEFT_OP ||
    cachedOp == RIGHT_OP || cachedOp == OVERRIGHT_OP)
  {
    *value = true;
    *time = false;
  }
  else if (time_type(arg) ||
    cachedOp == BEFORE_OP || cachedOp == OVERBEFORE_OP ||
    cachedOp == AFTER_OP || cachedOp == OVERAFTER_OP)
  {
    *value = false;
    *time = true;
  }
  else if (tnumber_type(arg) && (cachedOp == OVERLAPS_OP ||
    cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP ||
    cachedOp == SAME_OP || cachedOp == ADJACENT_OP))
  {
    *value = true;
    *time = true;
  }
  else
  {
    /* By default only the time component is taken into account */
    *value = false;
    *time = true;
  }
  return true;
}

PG_FUNCTION_INFO_V1(Tnumber_joinsel);
/**
 * Estimate the join selectivity value of the operators for temporal numbers
 */
PGDLLEXPORT Datum
Tnumber_joinsel(PG_FUNCTION_ARGS)
{
  return temporal_joinsel_ext(fcinfo, TNUMBERTYPE);
}

/*****************************************************************************/
