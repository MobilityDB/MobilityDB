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
 * @file
 * @brief Functions for selectivity estimation of operators on temporal types
 * whose bounding box is a `Period`, that is, `tbool` and `ttext`.
 *
 * The operators currently supported are as follows
 * - B-tree comparison operators: `<`, `<=`, `>`, `>=`
 * - Bounding box operators: `&&`, `@>`, `<@`, `~=`
 * - Relative position operators: `<<#`, `&<#`, `#>>`, `#>>`
 * - Ever/always comparison operators: `?=`, `%=`, `?<>`, `%<>`, `?<, `%<`,
 * ... These still need to be defined. TODO
 *
 */

#include "pg_general/temporal_selfuncs.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <access/amapi.h>
#include <access/heapam.h>
#include <access/htup_details.h>
#include <access/itup.h>
#include <access/relscan.h>
#include <access/visibilitymap.h>
#include <access/skey.h>
#include <catalog/pg_collation_d.h>
#include <parser/parsetree.h>
#include <executor/tuptable.h>
#include <optimizer/paths.h>
#include <storage/bufmgr.h>
#include <utils/date.h>
#include <utils/datum.h>
#include <utils/memutils.h>
#include <utils/rel.h>
#include <utils/syscache.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/spanset.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/span_selfuncs.h"
#include "pg_general/temporal_analyze.h"
#include "pg_general/temporal_selfuncs.h"
#include "pg_point/tpoint_selfuncs.h"

/*****************************************************************************
 * Internal functions computing selectivity
 * The functions assume that the value and time dimensions of temporal values
 * are independent and thus the selectivity values obtained by analyzing the
 * histograms for each dimension can be multiplied.
 *****************************************************************************/

/**
 * @brief Transform the constant into a period
 */
static bool
temporal_const_to_period(Node *other, Span *period)
{
  Oid consttype = ((Const *) other)->consttype;
  Datum constvalue = ((Const *) other)->constvalue;
  meosType type = oid_type(consttype);
  if (time_type(type))
    span_const_to_span(other, period);
  else if (talpha_type(type))
    temporal_set_bbox(DatumGetTemporalP(constvalue), period);
  else
    return false;
  return true;
}

/**
 * @brief Transform the constant into a temporal box
 */
bool
tnumber_const_to_span_period(const Node *other, Span **s, Span **p)
{
  Oid consttypid = ((Const *) other)->consttype;
  meosType type = oid_type(consttypid);
  if (numspan_type(type))
  {
    Span *span = DatumGetSpanP(((Const *) other)->constvalue);
    *s = span_copy(span);
  }
  else if (type == T_TSTZSPAN)
  {
    Span *period = DatumGetSpanP(((Const *) other)->constvalue);
    *p = span_copy(period);
  }
  else if (type == T_TBOX)
  {
    const TBox *box = DatumGetTboxP(((Const *) other)->constvalue);
    *s = tbox_to_floatspan(box);
    *p = tbox_to_period(box);
  }
  else if (tnumber_type(type))
  {
    const Temporal *temp = DatumGetTemporalP(((Const *) other)->constvalue);
    TBox box;
    temporal_set_bbox(temp, &box);
    *s = tbox_to_floatspan(&box);
    *p = tbox_to_period(&box);
  }
  else
    return false;
  return true;
}

/**
 * @brief Transform the constant into an STBox
 * @note This function is also used for temporal network points
 */
static bool
tpoint_const_to_stbox(Node *other, STBox *box)
{
  Oid consttype = ((Const *) other)->consttype;
  Datum constvalue = ((Const *) other)->constvalue;
  meosType type = oid_type(consttype);
  if (geo_basetype(type))
    geo_set_stbox(DatumGetGserializedP(constvalue), box);
  else if (type == T_TSTZSPAN)
    period_set_stbox(DatumGetSpanP(constvalue), box);
  else if (type == T_STBOX)
    memcpy(box, DatumGetSTboxP(constvalue), sizeof(STBox));
  else if (tspatial_type(type))
    temporal_set_bbox(DatumGetTemporalP(constvalue), box);
  else
    return false;
  return true;
}

/*****************************************************************************/

/**
 * @brief Return a default selectivity estimate for the operator when we don't
 * have statistics or cannot use them for some reason.
 */
static double
temporal_sel_default(meosOper oper)
{
  switch (oper)
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
 * @brief Return a default selectivity estimate for the operator when we don't
 * have statistics or cannot use them for some reason.
 */
float8
tnumber_sel_default(meosOper operator)
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
 * @brief Return a default restriction selectivity estimate for a given
 * operator, when we don't have statistics or cannot use them for some reason.
 */
static float8
tpoint_sel_default(meosOper oper)
{
  switch (oper)
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
    case FRONT_OP:
    case BACK_OP:
    case OVERFRONT_OP:
    case OVERBACK_OP:
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

/*****************************************************************************/

/**
 * @brief Return a default join selectivity estimate for given operator, when
 * we don't have statistics or cannot use them for some reason.
 */
float8
temporal_joinsel_default(Oid operid __attribute__((unused)))
{
  // TODO take care of the operator
  return 0.001;
}

/**
 * @brief Return a default join selectivity estimate for given operator, when
 * we don't have statistics or cannot use them for some reason.
 */
float8
tnumber_joinsel_default(meosOper oper __attribute__((unused)))
{
  // TODO take care of the operators
  return 0.001;
}

/**
 * @brief Return a default join selectivity estimate for a given operator,
 * when we don't have statistics or cannot use them for some reason.
 */
static float8
tpoint_joinsel_default(meosOper oper)
{
  switch (oper)
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
    case FRONT_OP:
    case BACK_OP:
    case OVERFRONT_OP:
    case OVERBACK_OP:
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

/*****************************************************************************/

/**
 * @brief Return the enum value associated to the operator
 */
static bool
temporal_oper_sel(meosOper oper __attribute__((unused)), meosType ltype,
  meosType rtype)
{
  if ((timespan_basetype(ltype) || timeset_type(ltype) ||
        timespan_type(ltype) || timespanset_type(ltype) ||
        talpha_type(ltype)) &&
      (timespan_basetype(rtype) || timeset_type(rtype) ||
        timespan_type(rtype) || timespanset_type(rtype) ||
        talpha_type(ltype)))
    return true;
  return false;
}

/**
 * @brief Return the enum value associated to the operator
 */
bool
tnumber_oper_sel(Oid operid __attribute__((unused)), meosType ltype,
  meosType rtype)
{
  if ((timespan_basetype(ltype) || timeset_type(ltype) ||
        timespan_type(ltype) || timespanset_type(ltype) ||
        ltype == T_TBOX || temporal_type(ltype)) &&
      (timespan_basetype(rtype) || timeset_type(rtype) ||
        timespan_type(rtype) || timespanset_type(rtype) ||
        rtype == T_TBOX || temporal_type(rtype)))
    return true;
  return false;
}

/**
 * @brief Get the enum value associated to the operator
 */
static bool
tpoint_oper_sel(Oid operid __attribute__((unused)), meosType ltype,
  meosType rtype)
{
  if ((timespan_basetype(ltype) || timeset_type(ltype) ||
        timespan_type(ltype) || timespanset_type(ltype) ||
        geo_basetype(ltype) || ltype == T_STBOX || tgeo_type(ltype)) &&
      (timespan_basetype(rtype) || timeset_type(rtype) ||
        timespan_type(rtype) || timespanset_type(rtype) ||
        geo_basetype(rtype) || rtype == T_STBOX || tgeo_type(rtype)))
    return true;
  return false;
}

/**
 * @brief Get the enum value associated to the operator
 */
bool
tnpoint_oper_sel(Oid operid __attribute__((unused)), meosType ltype,
  meosType rtype)
{
  if ((timespan_basetype(ltype) || timeset_type(ltype) ||
        timespan_type(ltype) || timespanset_type(ltype) ||
        spatial_basetype(ltype) || ltype == T_STBOX || tspatial_type(ltype)) &&
      (timespan_basetype(rtype) || timeset_type(rtype) ||
        timespan_type(rtype) || timespanset_type(rtype) ||
        spatial_basetype(rtype) || rtype == T_STBOX || tspatial_type(rtype)))
    return true;
  return false;
}

/**
 * @brief Get enumeration value associated to the operator according to the family
 */
static bool
temporal_oper_sel_family(meosOper oper __attribute__((unused)), meosType ltype,
  meosType rtype, TemporalFamily tempfamily)
{
  /* Get enumeration value associated to the operator */
#if NPOINT
  assert(tempfamily == TEMPORALTYPE || tempfamily == TNUMBERTYPE ||
    tempfamily == TPOINTTYPE || tempfamily == TNPOINTTYPE);
#else
  assert(tempfamily == TEMPORALTYPE || tempfamily == TNUMBERTYPE ||
    tempfamily == TPOINTTYPE);
#endif /* NPOINT */

  if (tempfamily == TEMPORALTYPE)
    return temporal_oper_sel(oper, ltype, rtype);
  else if (tempfamily == TNUMBERTYPE)
    return tnumber_oper_sel(oper, ltype, rtype);
  else if (tempfamily == TPOINTTYPE)
    return tpoint_oper_sel(oper, ltype, rtype);
  else /* tempfamily == TNPOINTTYPE */
    return tnpoint_oper_sel(oper, ltype, rtype);
}

/*****************************************************************************/

/**
 * @brief Return an estimate of the selectivity of the search period and the
 * operator for columns of temporal values.
 *
 * For the traditional comparison operators (<, <=, ...), we follow the
 * approach for range types in PostgreSQL, this function computes the
 * selectivity for <, <=, >, and >=, while the selectivity functions for = and
 * <> are eqsel and neqsel, respectively.
 */
Selectivity
temporal_sel_period(VariableStatData *vardata, Span *period, meosOper oper)
{
  float8 selec;

  /*
   * There is no ~= operator for time types and thus it is necessary to
   * take care of this operator here.
   */
  if (oper == SAME_OP)
  {
    Oid operid = oper_oid(EQ_OP, T_TSTZSPAN, T_TSTZSPAN);
#if POSTGRESQL_VERSION_NUMBER < 130000
    selec = var_eq_const(vardata, operid, SpanPGetDatum(period),
      false, false, false);
#else
    selec = var_eq_const(vardata, operid, DEFAULT_COLLATION_OID,
      SpanPGetDatum(period), false, false, false);
#endif
  }
  else if (oper == OVERLAPS_OP || oper == CONTAINS_OP ||
    oper == CONTAINED_OP ||  oper == ADJACENT_OP ||
    oper == BEFORE_OP || oper == OVERBEFORE_OP ||
    oper == AFTER_OP || oper == OVERAFTER_OP ||
    /* For b-tree comparisons, temporal values are first compared wrt
     * their bounding boxes, and if these are equal, other criteria apply.
     * For selectivity estimation we approximate by taking into account
     * only the bounding boxes. In the case here the bounding box is a
     * period and thus we can use the period selectivity estimation */
    oper == LT_OP || oper == LE_OP ||
    oper == GT_OP || oper == GE_OP)
  {
    /* Cast the period as a span to call the span selectivity functions */
    selec = span_sel_hist(vardata, (Span *) period, oper, TIME_SEL);
  }
  else /* Unknown operator */
  {
    selec = temporal_sel_default(oper);
  }
  return selec;
}

/**
 * @brief Return an estimate of the selectivity of the temporal search box and
 * the operator for columns of temporal numbers.
 *
 * For the traditional comparison operators (<, <=, ...) we follow the approach
 * for span types in PostgreSQL, this function computes the selectivity for <,
 * <=, >, and >=, while the selectivity functions for = and <> are eqsel and
 * neqsel, respectively.
 */
Selectivity
tnumber_sel_span_period(VariableStatData *vardata, Span *span, Span *period,
  meosOper oper)
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
  if (oper == SAME_OP)
  {
    /* Selectivity for the value dimension */
    if (span != NULL)
    {
      value_oprid = oper_oid(EQ_OP, span->spantype, span->spantype);
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
      period_oprid = oper_oid(EQ_OP, period->spantype, period->spantype);
#if POSTGRESQL_VERSION_NUMBER < 130000
      selec *= var_eq_const(vardata, period_oprid, SpanPGetDatum(period),
        false, false, false);
#else
      selec *= var_eq_const(vardata, period_oprid, DEFAULT_COLLATION_OID,
        SpanPGetDatum(period), false, false, false);
#endif
    }
  }
  else if (oper == OVERLAPS_OP || oper == CONTAINS_OP ||
    oper == CONTAINED_OP ||
    /* For b-tree comparisons, temporal values are first compared wrt
     * their bounding boxes, and if these are equal, other criteria apply.
     * For selectivity estimation we approximate by taking into account
     * only the bounding boxes. */
    oper == LT_OP || oper == LE_OP ||
    oper == GT_OP || oper == GE_OP)
  {
    /* Selectivity for the value dimension */
    if (span != NULL)
      selec *= span_sel_hist(vardata, span, oper, VALUE_SEL);
    /* Selectivity for the time dimension */
    if (period != NULL)
      /* Cast the period as a span to call the span selectivity functions */
      selec *= span_sel_hist(vardata, (Span *) period, oper, TIME_SEL);
  }
  else if (oper == LEFT_OP || oper == RIGHT_OP ||
    oper == OVERLEFT_OP || oper == OVERRIGHT_OP)
  {
    /* Selectivity for the value dimension */
    if (span != NULL)
      selec *= span_sel_hist(vardata, span, oper, VALUE_SEL);
  }
  else if (oper == BEFORE_OP || oper == AFTER_OP ||
    oper == OVERBEFORE_OP || oper == OVERAFTER_OP)
  {
    /* Selectivity for the value dimension */
    if (period != NULL)
      /* Cast the period as a span to call the span selectivity functions */
      selec *= span_sel_hist(vardata, (Span *) period, oper, TIME_SEL);
  }
  else /* Unknown operator */
  {
    selec = tnumber_sel_default(oper);
  }
  return selec;
}

/*****************************************************************************/

/**
 * @brief Estimate the selectivity value of the operators for temporal types
 * whose bounding box is a period, that is, tbool and ttext
 */
float8
temporal_sel(PlannerInfo *root, Oid operid, List *args, int varRelid,
  TemporalFamily tempfamily)
{
  VariableStatData vardata;
  Node *other;
  bool varonleft;
  Selectivity selec;

  /* Determine whether we can estimate selectivity for the operator */
  meosType ltype, rtype;
  meosOper oper = oid_oper(operid, &ltype, &rtype);
  if (! temporal_oper_sel_family(oper, ltype, rtype, tempfamily))
    /* In the case of unknown operator */
    return DEFAULT_TEMP_SEL;

  /*
   * If expression is not (variable op something) or (something op
   * variable), then punt and return a default estimate.
   */
  if (! get_restriction_variable(root, args, varRelid, &vardata, &other,
      &varonleft))
  {
    if (tempfamily == TEMPORALTYPE)
      return temporal_sel_default(oper);
    else if (tempfamily == TNUMBERTYPE)
      return tnumber_sel_default(oper);
    else if (tempfamily == TPOINTTYPE)
      return tpoint_sel_default(oper);
    else /* tempfamily == TNPOINTTYPE */
      return tpoint_sel_default(oper);
  }

  /*
   * Can't do anything useful if the something is not a constant, either.
   */
  if (!IsA(other, Const))
  {
    ReleaseVariableStats(vardata);
    if (tempfamily == TEMPORALTYPE)
      return temporal_sel_default(oper);
    else if(tempfamily == TNUMBERTYPE)
      return tnumber_sel_default(oper);
    else /* tempfamily == TPOINTYPE */
      return tpoint_sel_default(oper);
  }

  /*
   * All the bounding box operators are strict, so we can cope with a NULL
   * constant right away.
   */
  if (((Const *) other)->constisnull)
  {
    ReleaseVariableStats(vardata);
    return 0.0;
  }

  /*
   * If var is on the right, commute the operator, so that we can assume the
   * var is on the left in what follows.
   */
  if (! varonleft)
  {
    /* we have other Op var, commute to make var Op other */
    operid = get_commutator(operid);
    if (! operid)
    {
      /* Use default selectivity (should we raise an error instead?) */
      ReleaseVariableStats(vardata);
      if (tempfamily == TEMPORALTYPE)
        return temporal_sel_default(oper);
      else if (tempfamily == TNUMBERTYPE)
        return tnumber_sel_default(oper);
      else /* tempfamily == TPOINTTYPE */
        return tpoint_sel_default(oper);
    }
  }

  /* Transform the constant into a bounding box and compute the selectivity */
  if (tempfamily == TEMPORALTYPE)
  {
    Span period;
    if (! temporal_const_to_period(other, &period))
      /* In the case of unknown constant */
      return temporal_sel_default(oper);
    /* Compute the selectivity */
    selec = temporal_sel_period(&vardata, &period, oper);
  }
  else if (tempfamily == TNUMBERTYPE)
  {
#if DEBUG_BUILD
    /* Get the base type of the temporal column */
    meosType basetype = temptype_basetype(oid_type(vardata.atttype));
    assert(span_basetype(basetype));
#endif /* DEBUG_BUILD */
    /* Transform the constant into a span and/or a period */
    Span *s = NULL;
    Span *p = NULL;
    if (! tnumber_const_to_span_period(other, &s, &p))
      /* In the case of unknown constant */
      return tnumber_sel_default(oper);

    /* Compute the selectivity */
    selec = tnumber_sel_span_period(&vardata, s, p, oper);
    /* Free variables */
    if (s) pfree(s);
    if (p) pfree(p);
  }
  else /* tempfamily == TPOINTTYPE */
  {
    /*
     * Transform the constant into an STBox
     */
    STBox box;
    if (! tpoint_const_to_stbox(other, &box))
      /* In the case of unknown constant */
      return tpoint_sel_default(oper);

    assert(MEOS_FLAGS_GET_X(box.flags) || MEOS_FLAGS_GET_T(box.flags));

    /* Enable the multiplication of the selectivity of the spatial and time
     * dimensions since either may be missing */
    selec = 1.0;

    /*
     * Estimate selectivity for the spatial dimension
     */
    if (MEOS_FLAGS_GET_X(box.flags))
    {
      /* PostGIS does not provide selectivity for the traditional
       * comparisons <, <=, >, >= */
      if (oper == LT_OP || oper == LE_OP || oper == GT_OP || oper == GE_OP)
        selec *= tpoint_sel_default(oper);
      else
        selec *= geo_sel(&vardata, &box, oper);
    }
    /*
     * Estimate selectivity for the time dimension
     */
    if (MEOS_FLAGS_GET_T(box.flags))
    {
      /* Transform the STBox into a Period */
      Span period;
      memcpy(&period, &box.period, sizeof(Span));

      /* Compute the selectivity */
      selec *= temporal_sel_period(&vardata, &period, oper);
    }
  }

  ReleaseVariableStats(vardata);
  CLAMP_PROBABILITY(selec);
  return selec;
}

/*
 * Estimate the restriction selectivity value of the operators for the
 * various families of temporal types.
 */
float8
temporal_sel_ext(FunctionCallInfo fcinfo, TemporalFamily tempfamily)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid operid = PG_GETARG_OID(1);
  List *args = (List *) PG_GETARG_POINTER(2);
  int varRelid = PG_GETARG_INT32(3);

  assert(tempfamily == TEMPORALTYPE || tempfamily == TNUMBERTYPE ||
         tempfamily == TPOINTTYPE || tempfamily == TNPOINTTYPE);
  float8 result = temporal_sel(root, operid, args, varRelid, tempfamily);
  return result;
}

PGDLLEXPORT Datum Temporal_sel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_sel);
/**
 * @brief Estimate the selectivity value of the operators for temporal types
 * whose bounding box is a period, that is, tbool and ttext.
 */
Datum
Temporal_sel(PG_FUNCTION_ARGS)
{
  return Float8GetDatum(temporal_sel_ext(fcinfo, TEMPORALTYPE));
}

PGDLLEXPORT Datum Tnumber_sel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_sel);
/**
 * @brief Estimate the selectivity value of the operators for temporal numbers
 */
Datum
Tnumber_sel(PG_FUNCTION_ARGS)
{
  return Float8GetDatum(temporal_sel_ext(fcinfo, TNUMBERTYPE));
}

PGDLLEXPORT Datum Tpoint_sel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_sel);
/**
 * @brief Estimate the restriction selectivity of the operators for temporal
 * points
 */
Datum
Tpoint_sel(PG_FUNCTION_ARGS)
{
  return Float8GetDatum(temporal_sel_ext(fcinfo, TPOINTTYPE));
}

PGDLLEXPORT Datum Tnpoint_sel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_sel);
/**
 * @brief Estimate the restriction selectivity of the operators for temporal
 * network points
 */
Datum
Tnpoint_sel(PG_FUNCTION_ARGS)
{
  return Float8GetDatum(temporal_sel_ext(fcinfo, TNPOINTTYPE));
}

/*****************************************************************************
 * Estimate the join selectivity
 *****************************************************************************/

/**
 * @brief Depending on the operator and the arguments, determine wheter the
 * value, the time, or both components are taken into account for computing
 * the join selectivity
 */
static bool
tnumber_joinsel_components(meosOper oper, meosType oprleft,
  meosType oprright, bool *value, bool *time)
{
  /* Get the argument which may not a temporal number */
  meosType arg = tnumber_type(oprleft) ? oprright : oprleft;

  /* Determine the components */
  if (tnumber_basetype(arg) || tnumber_spantype(arg) ||
    oper == LEFT_OP || oper == OVERLEFT_OP ||
    oper == RIGHT_OP || oper == OVERRIGHT_OP)
  {
    *value = true;
    *time = false;
  }
  else if (time_type(arg) ||
    oper == BEFORE_OP || oper == OVERBEFORE_OP ||
    oper == AFTER_OP || oper == OVERAFTER_OP)
  {
    *value = false;
    *time = true;
  }
  else if (tnumber_type(arg) && (oper == OVERLAPS_OP ||
    oper == CONTAINS_OP || oper == CONTAINED_OP ||
    oper == SAME_OP || oper == ADJACENT_OP))
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

/**
 * @brief Depending on the operator and the arguments, determine wheter the
 * space, the time, or both components are taken into account for computing the
 * join selectivity
 */
static bool
tpoint_joinsel_components(meosOper oper, meosType oprleft,
  meosType oprright, bool *space, bool *time)
{
  /* Get the argument which may not be a temporal point */
  meosType arg = tspatial_type(oprleft) ? oprright : oprleft;

  /* Determine the components */
  if (tspatial_basetype(arg) ||
    oper == LEFT_OP || oper == OVERLEFT_OP ||
    oper == RIGHT_OP || oper == OVERRIGHT_OP ||
    oper == BELOW_OP || oper == OVERBELOW_OP ||
    oper == ABOVE_OP || oper == OVERABOVE_OP ||
    oper == FRONT_OP || oper == OVERFRONT_OP ||
    oper == BACK_OP || oper == OVERBACK_OP ||
    oper == ADJACENT_OP)
  {
    *space = true;
    *time = false;
  }
  else if (time_type(arg) ||
    oper == BEFORE_OP || oper == OVERBEFORE_OP ||
    oper == AFTER_OP || oper == OVERAFTER_OP)
  {
    *space = false;
    *time = true;
  }
  else if (tspatial_type(arg) && (oper == OVERLAPS_OP ||
    oper == CONTAINS_OP || oper == CONTAINED_OP ||
    oper == SAME_OP || oper == ADJACENT_OP))
  {
    *space = true;
    *time = true;
  }
  else
  {
    /* By default only the time component is taken into account */
    *space = false;
    *time = true;
  }
  return true;
}

/**
 * @brief Return an estimate of the join selectivity for columns of temporal
 * values.
 *
 * For the traditional comparison operators (<, <=, ...), we follow the
 * approach for range types in PostgreSQL, this function  computes the
 * selectivity for <, <=, >, and >=, while the selectivity functions for
 * = and <> are eqsel and neqsel, respectively.
 */
float8
temporal_joinsel(PlannerInfo *root, Oid operid, List *args, JoinType jointype,
  SpecialJoinInfo *sjinfo, TemporalFamily tempfamily)
{
  assert(tempfamily == TEMPORALTYPE || tempfamily == TNUMBERTYPE ||
         tempfamily == TPOINTTYPE || tempfamily == TNPOINTTYPE);

  Node *arg1 = (Node *) linitial(args);
  Node *arg2 = (Node *) lsecond(args);
  Var *var1 = (Var *) arg1;
  Var *var2 = (Var *) arg2;

  /* We only do column joins right now, no functional joins */
  /* TODO: handle t1 <op> expandX(t2) */
  if (!IsA(arg1, Var) || !IsA(arg2, Var))
    return DEFAULT_TEMP_JOINSEL;

  /* Determine whether we can estimate selectivity for the operator */
  meosType ltype, rtype;
  meosOper oper = oid_oper(operid, &ltype, &rtype);
  if (! temporal_oper_sel_family(oper, ltype, rtype, tempfamily))
    /* In the case of unknown operator */
    return DEFAULT_TEMP_SEL;

  /*
   * Determine whether the value/space and/or the time components are
   * taken into account for the selectivity estimation
   */
  bool value = false, space = false, time = false;
  if (tempfamily == TEMPORALTYPE)
  {
    value = false;
    time = true;
  }
  else if (tempfamily == TNUMBERTYPE)
  {
    meosType oprleft = oid_type(var1->vartype);
    meosType oprright = oid_type(var2->vartype);
    if (! tnumber_joinsel_components(oper, oprleft, oprright, &value, &time))
      /* In the case of unknown arguments */
      return tnumber_joinsel_default(oper);
  }
  else /* tempfamily == TPOINTTYPE */
  {
    meosType oprleft = oid_type(var1->vartype);
    meosType oprright = oid_type(var2->vartype);
    if (! tpoint_joinsel_components(oper, oprleft, oprright, &space, &time))
      /* In the case of unknown arguments */
      return tpoint_joinsel_default(oper);
  }
  /*
   * Multiply the components of the join selectivity estimation
   */
  float8 selec = 1.0;
  if (value)
  {
    /*
     * There is no ~= operator for time types and thus it is necessary to
     * take care of this operator here.
     */
    if (oper == SAME_OP)
      // TODO
      selec *= span_joinsel_default(oper);
    else
      /* Estimate join selectivity */
      selec *= span_joinsel(root, oper, args, jointype, sjinfo);
  }
  if (space)
  {
    /* What are the Oids of our tables/relations? */
    Oid relid1 = rt_fetch(var1->varno, root->parse->rtable)->relid;
    Oid relid2 = rt_fetch(var2->varno, root->parse->rtable)->relid;

    /* Pull the stats from the stats system. */
    int mode = Int32GetDatum(0) /* ND mode TO GENERALIZE */;
    ND_STATS *stats1 = pg_get_nd_stats(relid1, var1->varattno, mode, false);
    ND_STATS *stats2 = pg_get_nd_stats(relid2, var2->varattno, mode, false);

    /* If we can't get stats, we have to stop here! */
    if (! stats1 || ! stats2)
      selec *= tpoint_joinsel_default(oper);
    else
      selec *= geo_joinsel(stats1, stats2);
    if (stats1)
      pfree(stats1);
    if (stats2)
      pfree(stats2);
  }
  if (time)
  {
    /*
     * Return default selectivity for the time dimension for the following cases
     * - There is no ~= operator for time types
     * - The support functions for the ever spatial relationships add a
     *   bounding box test with the && operator, but we need to exclude
     *   the dwithin operator since it takes 3 arguments and thus the
     *   PostgreSQL function get_join_variables cannot be invoked.
     */
    if (oper == SAME_OP ||
      (oper == OVERLAPS_OP && list_length(args) != 2))
      selec *= span_joinsel_default(oper);
    else
      /* Estimate join selectivity */
      selec *= span_joinsel(root, oper, args, jointype, sjinfo);
  }

  CLAMP_PROBABILITY(selec);
  return (float8) selec;
}

/*
 * @brief Estimate the join selectivity value of the operators for temporal
 * alphanumeric types and temporal number types.
 */
float8
temporal_joinsel_ext(FunctionCallInfo fcinfo, TemporalFamily tempfamily)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid operid = PG_GETARG_OID(1);
  List *args = (List *) PG_GETARG_POINTER(2);
  JoinType jointype = (JoinType) PG_GETARG_INT16(3);
  SpecialJoinInfo *sjinfo = (SpecialJoinInfo *) PG_GETARG_POINTER(4);

  /* Check length of args and punt on > 2 */
  if (list_length(args) != 2)
    return DEFAULT_TEMP_JOINSEL;

  /* Only respond to an inner join/unknown context join */
  if (jointype != JOIN_INNER)
    return DEFAULT_TEMP_JOINSEL;

  float8 result = temporal_joinsel(root, operid, args, jointype, sjinfo,
      tempfamily);
  return result;
}

PGDLLEXPORT Datum Temporal_joinsel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_joinsel);
/*
 * Estimate the join selectivity value of the operators for temporal types
 * whose bounding box is a period, that is, tbool and ttext.
 */
Datum
Temporal_joinsel(PG_FUNCTION_ARGS)
{
  return Float8GetDatum(temporal_joinsel_ext(fcinfo, TEMPORALTYPE));
}

PGDLLEXPORT Datum Tnumber_joinsel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_joinsel);
/**
 * @brief Estimate the join selectivity value of the operators for temporal
 * numbers
 */
Datum
Tnumber_joinsel(PG_FUNCTION_ARGS)
{
  return Float8GetDatum(temporal_joinsel_ext(fcinfo, TNUMBERTYPE));
}

PGDLLEXPORT Datum Tnpoint_joinsel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_joinsel);
/**
 * @brief Estimate the join selectivity of the operators for temporal network
 * points
 */
Datum
Tnpoint_joinsel(PG_FUNCTION_ARGS)
{
  return Float8GetDatum(temporal_joinsel_ext(fcinfo, TNPOINTTYPE));
}

PGDLLEXPORT Datum Tpoint_joinsel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_joinsel);
/**
 * @brief Estimate the join selectivity value of the operators for temporal
 * points.
 */
Datum
Tpoint_joinsel(PG_FUNCTION_ARGS)
{
  return Float8GetDatum(temporal_joinsel_ext(fcinfo, TPOINTTYPE));
}

/*****************************************************************************/
