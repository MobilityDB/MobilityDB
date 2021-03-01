/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file temporal_selfuncs.c
 * Functions for selectivity estimation of operators on temporal types whose
 * bounding box is a `Period`, that is, `tbool` and `ttext`.
 *
 * The operators currently supported are as follows
 * - B-tree comparison operators: `<`, `<=`, `>`, `>=`
 * - Bounding box operators: `&&`, `@>`, `<@`, `~=`
 * - Relative position operators: `<<#`, `&<#`, `#>>`, `#>>`
 * - Ever/always comparison operators: `?=`, `%=`, `?<>`, `%<>`, `?<, `%<`, 
 * ... These still need to be defined. TODO
 *
 * Due to implicit casting, a condition such as `tbool <<# timestamptz` will be
 * transformed into `tbool <<# period`. This allows to reduce the number of
 * cases for the operator definitions, indexes, selectivity, etc.
 */

#include "temporal_selfuncs.h"

#include <assert.h>
#include <access/amapi.h>
#include <access/heapam.h>
#include <access/htup_details.h>
#include <access/itup.h>
#include <access/relscan.h>
#include <access/visibilitymap.h>
#include <access/skey.h>
#if MOBDB_PGSQL_VERSION < 110000
#include <catalog/pg_collation.h>
#else
#include <catalog/pg_collation_d.h>
#endif
#include <executor/tuptable.h>
#include <optimizer/paths.h>
#include <storage/bufmgr.h>
#include <utils/builtins.h>
#include <utils/date.h>
#include <utils/datum.h>
#include <utils/memutils.h>
#include <utils/rel.h>
#include <utils/syscache.h>
#include <temporal_boxops.h>
#include <timetypes.h>

#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "time_selfuncs.h"
#include "rangetypes_ext.h"
#include "temporal_analyze.h"
#include "tpoint.h"

/*****************************************************************************
 * This function is exported only after PostgreSQL version 12
 *****************************************************************************/

#if MOBDB_PGSQL_VERSION < 120000
/**
 * Equal selectivity for var = const case
 *
 * @note Function copied from selfuncs.c since it is not exported.
 */
double
var_eq_const(VariableStatData *vardata, Oid operator,
       Datum constval, bool constisnull,
       bool varonleft, bool negate)
{
  double    selec;
  double    nullfrac = 0.0;
  bool    isdefault;
  Oid      opfuncoid;

  /*
   * If the constant is NULL, assume operator is strict and return zero, ie,
   * operator will never return TRUE.  (It's zero even for a negator op.)
   */
  if (constisnull)
    return 0.0;

  /*
   * Grab the nullfrac for use below.  Note we allow use of nullfrac
   * regardless of security check.
   */
  if (HeapTupleIsValid(vardata->statsTuple))
  {
    Form_pg_statistic stats;

    stats = (Form_pg_statistic) GETSTRUCT(vardata->statsTuple);
    nullfrac = stats->stanullfrac;
  }

  /*
   * If we matched the var to a unique index or DISTINCT clause, assume
   * there is exactly one match regardless of anything else.  (This is
   * slightly bogus, since the index or clause's equality operator might be
   * different from ours, but it's much more likely to be right than
   * ignoring the information.)
   */
  if (vardata->isunique && vardata->rel && vardata->rel->tuples >= 1.0)
  {
    selec = 1.0 / vardata->rel->tuples;
  }
  else if (HeapTupleIsValid(vardata->statsTuple) &&
       statistic_proc_security_check(vardata,
                       (opfuncoid = get_opcode(operator))))
  {
    AttStatsSlot sslot;
    bool    match = false;
    int      i;

    /*
     * Is the constant "=" to any of the column's most common values?
     * (Although the given operator may not really be "=", we will assume
     * that seeing whether it returns TRUE is an appropriate test.  If you
     * don't like this, maybe you shouldn't be using eqsel for your
     * operator...)
     */
    if (get_attstatsslot(&sslot, vardata->statsTuple,
               // EZ replaced InvalidOid by operator
               STATISTIC_KIND_MCV, operator,
               ATTSTATSSLOT_VALUES | ATTSTATSSLOT_NUMBERS))
    {
      FmgrInfo  eqproc;

      fmgr_info(opfuncoid, &eqproc);

      for (i = 0; i < sslot.nvalues; i++)
      {
        /* be careful to apply operator right way 'round */
        if (varonleft)
          match = DatumGetBool(FunctionCall2Coll(&eqproc,
                               DEFAULT_COLLATION_OID,
                               sslot.values[i],
                               constval));
        else
          match = DatumGetBool(FunctionCall2Coll(&eqproc,
                               DEFAULT_COLLATION_OID,
                               constval,
                               sslot.values[i]));
        if (match)
          break;
      }
    }
    else
    {
      /* no most-common-value info available */
      i = 0;        /* keep compiler quiet */
    }

    if (match)
    {
      /*
       * Constant is "=" to this common value.  We know selectivity
       * exactly (or as exactly as ANALYZE could calculate it, anyway).
       */
      selec = sslot.numbers[i];
    }
    else
    {
      /*
       * Comparison is against a constant that is neither NULL nor any
       * of the common values.  Its selectivity cannot be more than
       * this:
       */
      double    sumcommon = 0.0;
      double    otherdistinct;

      for (i = 0; i < sslot.nnumbers; i++)
        sumcommon += sslot.numbers[i];
      selec = 1.0 - sumcommon - nullfrac;
      CLAMP_PROBABILITY(selec);

      /*
       * and in fact it's probably a good deal less. We approximate that
       * all the not-common values share this remaining fraction
       * equally, so we divide by the number of other distinct values.
       */
      otherdistinct = get_variable_numdistinct(vardata, &isdefault) -
        sslot.nnumbers;
      if (otherdistinct > 1)
        selec /= otherdistinct;

      /*
       * Another cross-check: selectivity shouldn't be estimated as more
       * than the least common "most common value".
       */
      if (sslot.nnumbers > 0 && selec > sslot.numbers[sslot.nnumbers - 1])
        selec = sslot.numbers[sslot.nnumbers - 1];
    }

    free_attstatsslot(&sslot);
  }
  else
  {
    /*
     * No ANALYZE stats available, so make a guess using estimated number
     * of distinct values and assuming they are equally common. (The guess
     * is unlikely to be very good, but we do know a few special cases.)
     */
    selec = 1.0 / get_variable_numdistinct(vardata, &isdefault);
  }

  /* now adjust if we wanted <> rather than = */
  if (negate)
    selec = 1.0 - selec - nullfrac;

  /* result should be in range, but make sure... */
  CLAMP_PROBABILITY(selec);

  return selec;
}
#endif

/*****************************************************************************
 * Internal functions computing selectivity
 * The functions assume that the value and time dimensions of temporal values
 * are independent and thus the selectivity values obtained by analyzing the
 * histograms for each dimension can be multiplied.
 *****************************************************************************/

/**
 * Transform the constant into a period.
 *
 * @note Due to implicit casting constants of type TimestampTz, TimestampSet,
 * and PeriodSet are transformed into a Period
 */
static bool
temporal_const_to_period(Node *other, Period *period)
{
  Oid consttype = ((Const *) other)->consttype;

  if (consttype == type_oid(T_PERIOD))
    memcpy(period, DatumGetPeriod(((Const *) other)->constvalue), sizeof(Period));
  else if (consttype == type_oid(T_TBOOL) || consttype == type_oid(T_TTEXT))
    temporal_bbox(period, DatumGetTemporal(((Const *) other)->constvalue));
  else
    return false;
  return true;
}

/**
 * Returns the enum value associated to the operator
 */
static bool
temporal_cachedop(Oid operator, CachedOp *cachedOp)
{
  for (int i = LT_OP; i <= OVERAFTER_OP; i++) {
    if (operator == oper_oid((CachedOp) i, T_PERIOD, T_TBOOL) ||
      operator == oper_oid((CachedOp) i, T_TBOOL, T_PERIOD) ||
      operator == oper_oid((CachedOp) i, T_TBOX, T_TBOOL) ||
      operator == oper_oid((CachedOp) i, T_TBOOL, T_TBOX) ||
      operator == oper_oid((CachedOp) i, T_TBOOL, T_TBOOL) ||
      operator == oper_oid((CachedOp) i, T_PERIOD, T_TTEXT) ||
      operator == oper_oid((CachedOp) i, T_TTEXT, T_PERIOD) ||
      operator == oper_oid((CachedOp) i, T_TBOX, T_TTEXT) ||
      operator == oper_oid((CachedOp) i, T_TTEXT, T_TBOX) ||
      operator == oper_oid((CachedOp) i, T_TTEXT, T_TTEXT))
      {
        *cachedOp = (CachedOp) i;
        return true;
      }
  }
  return false;
}

/**
 * Returns a default selectivity estimate for the operator when we don't
 * have statistics or cannot use them for some reason.
 */
static double
default_temporal_selectivity(CachedOp operator)
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
 * Returns an estimate of the selectivity of the search period and the
 * operator for columns of temporal values. For the traditional comparison
 * operators (<, <=, ...), we follow the approach for range types in
 * PostgreSQL, this function computes the selectivity for <, <=, >, and >=,
 * while the selectivity functions for = and <> are eqsel and neqsel,
 * respectively.
 */
Selectivity
temporal_sel_internal(PlannerInfo *root, VariableStatData *vardata,
  Period *period, CachedOp cachedOp)
{
  double selec;

  /*
   * There is no ~= operator for time types and thus it is necessary to
   * take care of this operator here.
   */
  if (cachedOp == SAME_OP)
  {
    Oid operator = oper_oid(EQ_OP, T_PERIOD, T_PERIOD);
#if MOBDB_PGSQL_VERSION < 130000
    selec = var_eq_const(vardata, operator, PeriodGetDatum(period),
      false, false, false);
#else
    selec = var_eq_const(vardata, operator, DEFAULT_COLLATION_OID,
      PeriodGetDatum(period), false, false, false);
#endif
  }
  else if (cachedOp == OVERLAPS_OP || cachedOp == CONTAINS_OP ||
    cachedOp == CONTAINED_OP ||  cachedOp == ADJACENT_OP ||
    cachedOp == BEFORE_OP || cachedOp == OVERBEFORE_OP ||
    cachedOp == AFTER_OP || cachedOp == OVERAFTER_OP ||
    /* For b-tree comparisons, temporal values are first compared wrt
     * their bounding boxes, and if these are equal, other criteria apply.
     * For selectivity estimation we approximate by taking into account
     * only the bounding boxes. In the case here the bounding box is a
     * period and thus we can use the period selectivity estimation */
    cachedOp == LT_OP || cachedOp == LE_OP ||
    cachedOp == GT_OP || cachedOp == GE_OP)
  {
    selec = calc_period_hist_selectivity(vardata, period, cachedOp);
  }
  else /* Unknown operator */
  {
    selec = default_temporal_selectivity(cachedOp);
  }
  return selec;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_sel);
/**
 * Estimate the selectivity value of the operators for temporal types
 * whose bounding box is a period, that is, tbool and ttext.
 */
PGDLLEXPORT Datum
temporal_sel(PG_FUNCTION_ARGS)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid operator = PG_GETARG_OID(1);
  List *args = (List *) PG_GETARG_POINTER(2);
  int varRelid = PG_GETARG_INT32(3);
  VariableStatData vardata;
  Node *other;
  bool varonleft;
  Selectivity selec;
  CachedOp cachedOp;
  Period constperiod;

  /*
   * Get enumeration value associated to the operator
   */
  bool found = temporal_cachedop(operator, &cachedOp);
  /* In the case of unknown operator */
  if (!found)
    PG_RETURN_FLOAT8(DEFAULT_TEMP_SELECTIVITY);

  /*
   * If expression is not (variable op something) or (something op
   * variable), then punt and return a default estimate.
   */
  if (!get_restriction_variable(root, args, varRelid,
    &vardata, &other, &varonleft))
    PG_RETURN_FLOAT8(default_temporal_selectivity(cachedOp));

  /*
   * Can't do anything useful if the something is not a constant, either.
   */
  if (!IsA(other, Const))
  {
    ReleaseVariableStats(vardata);
    PG_RETURN_FLOAT8(default_temporal_selectivity(cachedOp));
  }

  /*
   * All the period operators are strict, so we can cope with a NULL constant
   * right away.
   */
  if (((Const *) other)->constisnull)
  {
    ReleaseVariableStats(vardata);
    PG_RETURN_FLOAT8(0.0);
  }

  /*
   * If var is on the right, commute the operator, so that we can assume the
   * var is on the left in what follows.
   */
  if (!varonleft)
  {
    /* we have other Op var, commute to make var Op other */
    operator = get_commutator(operator);
    if (!operator)
    {
      /* Use default selectivity (should we raise an error instead?) */
      ReleaseVariableStats(vardata);
      PG_RETURN_FLOAT8(default_temporal_selectivity(cachedOp));
    }
  }

  /*
   * Transform the constant into a Period
   */
  found = temporal_const_to_period(other, &constperiod);
  /* In the case of unknown constant */
  if (!found)
    PG_RETURN_FLOAT8(default_temporal_selectivity(cachedOp));

  /* Compute the selectivity of the temporal column */
  selec = temporal_sel_internal(root, &vardata, &constperiod, cachedOp);

  ReleaseVariableStats(vardata);
  CLAMP_PROBABILITY(selec);
  PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(temporal_joinsel);
/*
 * Estimate the join selectivity value of the operators for temporal types
 * whose bounding box is a period, that is, tbool and ttext.
 */
PGDLLEXPORT Datum
temporal_joinsel(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(DEFAULT_TEMP_SELECTIVITY);
}

/*****************************************************************************/
