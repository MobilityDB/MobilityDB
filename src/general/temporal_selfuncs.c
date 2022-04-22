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
 * @file temporal_selfuncs.c
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

#include "general/temporal_selfuncs.h"

/* PostgreSQL */
#include <assert.h>
#include <access/amapi.h>
#include <access/heapam.h>
#include <access/htup_details.h>
#include <access/itup.h>
#include <access/relscan.h>
#include <access/visibilitymap.h>
#include <access/skey.h>
#include <catalog/pg_collation_d.h>
#include <executor/tuptable.h>
#include <optimizer/paths.h>
#include <storage/bufmgr.h>
#include <utils/builtins.h>
#include <utils/date.h>
#include <utils/datum.h>
#include <utils/memutils.h>
#include <utils/rel.h>
#include <utils/syscache.h>
/* MobilityDB */
#include "general/timetypes.h"
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_selfuncs.h"
#include "general/time_ops.h"
#include "general/rangetypes_ext.h"
#include "general/temporal_util.h"
#include "general/temporal_boxops.h"
#include "general/temporal_analyze.h"
#include "general/tnumber_selfuncs.h"
#include "point/tpoint.h"
#include "point/tpoint_selfuncs.h"

/*****************************************************************************
 * This function is exported only after PostgreSQL version 12
 *****************************************************************************/

#if POSTGRESQL_VERSION_NUMBER < 120000
/**
 * Equal selectivity for var = const case
 *
 * @note Function copied from selfuncs.c since it is not exported.
 */
double
var_eq_const(VariableStatData *vardata, Oid operid,
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
                       (opfuncoid = get_opcode(operid))))
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
               STATISTIC_KIND_MCV, operid,
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
 * Transform the constant into a period
 */
static bool
temporal_const_to_period(Node *other, Period *period)
{
  Oid consttype = ((Const *) other)->consttype;
  CachedType type = oid_type(consttype);
  if (time_type(type))
    time_const_to_period(other, period);
  else if (type == T_TBOOL || type == T_TTEXT)
    temporal_bbox(DatumGetTemporalP(((Const *) other)->constvalue), period);
  else
    return false;
  return true;
}

/**
 * Return the enum value associated to the operator
 */
static bool
temporal_cachedop(Oid operid, CachedOp *cachedOp)
{
  for (int i = LT_OP; i <= OVERAFTER_OP; i++) {
    if (operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TBOOL) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TTEXT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TBOOL) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TTEXT) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_TBOOL) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_TTEXT) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_TBOOL) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_TTEXT) ||
        operid == oper_oid((CachedOp) i, T_TBOOL, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_TBOOL, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_TBOOL, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_TBOOL, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_TBOOL, T_TBOOL) ||
        operid == oper_oid((CachedOp) i, T_TTEXT, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_TTEXT, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_TTEXT, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_TTEXT, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_TTEXT, T_TTEXT))
      {
        *cachedOp = (CachedOp) i;
        return true;
      }
  }
  return false;
}

/**
 * Return a default selectivity estimate for the operator when we don't
 * have statistics or cannot use them for some reason.
 */
static double
temporal_sel_default(CachedOp oper)
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
 * Return a default join selectivity estimate for given operator, when we
 * don't have statistics or cannot use them for some reason.
 */
float8
temporal_joinsel_default(Oid operid __attribute__((unused)))
{
  // TODO take care of the operator
  return 0.001;
}


/**
 * Return an estimate of the selectivity of the search period and the
 * operator for columns of temporal values. For the traditional comparison
 * operators (<, <=, ...), we follow the approach for range types in
 * PostgreSQL, this function computes the selectivity for <, <=, >, and >=,
 * while the selectivity functions for = and <> are eqsel and neqsel,
 * respectively.
 */
Selectivity
temporal_sel_period(VariableStatData *vardata, Period *period,
  CachedOp cachedOp)
{
  float8 selec;

  /*
   * There is no ~= operator for time types and thus it is necessary to
   * take care of this operator here.
   */
  if (cachedOp == SAME_OP)
  {
    Oid operid = oper_oid(EQ_OP, T_PERIOD, T_PERIOD);
#if POSTGRESQL_VERSION_NUMBER < 130000
    selec = var_eq_const(vardata, operid, PeriodPGetDatum(period),
      false, false, false);
#else
    selec = var_eq_const(vardata, operid, DEFAULT_COLLATION_OID,
      PeriodPGetDatum(period), false, false, false);
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
    selec = period_sel_hist(vardata, period, cachedOp);
  }
  else /* Unknown operator */
  {
    selec = temporal_sel_default(cachedOp);
  }
  return selec;
}

/*****************************************************************************/

/**
 * Get enumeration value associated to the operator according to the family
 */
static bool
temporal_cachedop_family(Oid operid, CachedOp *cachedOp, TemporalFamily tempfamily)
{
  /* Get enumeration value associated to the operator */
  assert(tempfamily == TEMPORALTYPE || tempfamily == TNUMBERTYPE);
  if (tempfamily == TEMPORALTYPE)
    return temporal_cachedop(operid, cachedOp);
  else /* tempfamily == TNUMBERTYPE */
    return tnumber_cachedop(operid, cachedOp);
}

/**
 * Estimate the selectivity value of the operators for temporal types whose
 * bounding box is a period, that is, tbool and ttext (internal function)
 */
float8
temporal_sel(PlannerInfo *root, Oid operid, List *args, int varRelid,
  TemporalFamily tempfamily)
{
  VariableStatData vardata;
  Node *other;
  bool varonleft;
  Selectivity selec;

  /* Get enumeration value associated to the operator */
  CachedOp cachedOp;
  if (! temporal_cachedop_family(operid, &cachedOp, tempfamily))
    /* In the case of unknown operator */
    return DEFAULT_TEMP_SEL;

  /*
   * If expression is not (variable op something) or (something op
   * variable), then punt and return a default estimate.
   */
  if (!get_restriction_variable(root, args, varRelid, &vardata, &other,
      &varonleft))
  {
    if (tempfamily == TEMPORALTYPE)
      return temporal_sel_default(cachedOp);
    else /* tempfamily == TNUMBERTYPE */
      return tnumber_sel_default(cachedOp);
  }

  /*
   * Can't do anything useful if the something is not a constant, either.
   */
  if (!IsA(other, Const))
  {
    ReleaseVariableStats(vardata);
    if (tempfamily == TEMPORALTYPE)
      return temporal_sel_default(cachedOp);
    else /* tempfamily == TNUMBERTYPE */
      return tnumber_sel_default(cachedOp);
  }

  /*
   * All the period operators are strict, so we can cope with a NULL constant
   * right away.
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
  if (!varonleft)
  {
    /* we have other Op var, commute to make var Op other */
    operid = get_commutator(operid);
    if (! operid)
    {
      /* Use default selectivity (should we raise an error instead?) */
      ReleaseVariableStats(vardata);
      if (tempfamily == TEMPORALTYPE)
        return temporal_sel_default(cachedOp);
      else /* tempfamily == TNUMBERTYPE */
        return tnumber_sel_default(cachedOp);
    }
  }

  /* Transform the constant into a bounding box and compute the selectivity */
  if (tempfamily == TEMPORALTYPE)
  {
    Period period;
    if (! temporal_const_to_period(other, &period))
      /* In the case of unknown constant */
      return temporal_sel_default(cachedOp);
    /* Compute the selectivity */
    selec = temporal_sel_period(&vardata, &period, cachedOp);
  }
  else /* tempfamily == TNUMBERTYPE */
  {
    TBOX box;
    if (! tnumber_const_to_tbox(other, &box))
      /* In the case of unknown constant */
      return tnumber_sel_default(cachedOp);

    assert(MOBDB_FLAGS_GET_X(box.flags) ||
      MOBDB_FLAGS_GET_T(box.flags));
    /* Get the base type of the temporal column */
    Oid basetypid = temptypid_basetypid(vardata.atttype);
    /* Compute the selectivity */
    selec = tnumber_sel_box(&vardata, &box, cachedOp, basetypid);
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

  float8 result;
  assert(tempfamily == TEMPORALTYPE || tempfamily == TNUMBERTYPE ||
         tempfamily == TPOINTTYPE || tempfamily == TNPOINTTYPE);
  if (tempfamily == TEMPORALTYPE || tempfamily == TNUMBERTYPE)
    result = temporal_sel(root, operid, args, varRelid, tempfamily);
  else /* (tempfamily == TPOINTTYPE || tempfamily == TNPOINTTYPE) */
    result = tpoint_sel(root, operid, args, varRelid, tempfamily);
  return result;
}

PG_FUNCTION_INFO_V1(Temporal_sel);
/**
 * Estimate the selectivity value of the operators for temporal types whose
 * bounding box is a period, that is, tbool and ttext.
 */
PGDLLEXPORT Datum
Temporal_sel(PG_FUNCTION_ARGS)
{
  return temporal_sel_ext(fcinfo, TEMPORALTYPE);
}

/*****************************************************************************
 * Estimate the join selectivity
 *****************************************************************************/

/**
 * Return an estimate of the join selectivity for columns of temporal values.
 * For the traditional comparison operators (<, <=, ...), we follow the
 * approach for range types in PostgreSQL, this function  computes the
 * selectivity for <, <=, >, and >=, while the selectivity functions for
 * = and <> are eqsel and neqsel, respectively.
 */
float8
temporal_joinsel(PlannerInfo *root, Oid operid, List *args,
  JoinType jointype __attribute__((unused)), SpecialJoinInfo *sjinfo,
  TemporalFamily tempfamily)
{
  Node *arg1 = (Node *) linitial(args);
  Node *arg2 = (Node *) lsecond(args);
  Var *var1 = (Var *) arg1;
  Var *var2 = (Var *) arg2;

  /* We only do column joins right now, no functional joins */
  /* TODO: handle t1 <op> expandX(t2) */
  if (!IsA(arg1, Var) || !IsA(arg2, Var))
    return DEFAULT_TEMP_JOINSEL;

  /* Get enumeration value associated to the operator */
  CachedOp cachedOp;
  if (! temporal_cachedop_family(operid, &cachedOp, tempfamily))
    /* In the case of unknown operator */
    return DEFAULT_TEMP_SEL;

  /*
   * Determine whether the value and/or the time components are
   * taken into account for the selectivity estimation
   */
  bool value, time;
  if (tempfamily == TEMPORALTYPE)
  {
    value = false;
    time = true;
  }
  else /* tempfamily == TNUMBERTYPE */
  {
    CachedType oprleft = oid_type(var1->vartype);
    CachedType oprright = oid_type(var2->vartype);
    if (! tnumber_joinsel_components(cachedOp, oprleft, oprright,
      &value, &time))
      /* In the case of unknown arguments */
      return tnumber_joinsel_default(cachedOp);
  }

  /*
   * Since currently there is no join selectivity estimation for range types
   * in PostgreSQL, for temporal numbers we multiply the default value compute the join selectivity
   * for the temporal part
   */
  float8 selec = 1.0;
  if (value)
  {
    selec *= DEFAULT_TEMP_JOINSEL;
  }
  if (time)
  {
    /*
     * There is no ~= operator for time types and thus it is necessary to
     * take care of this operator here.
     */
    if (cachedOp == SAME_OP)
      // TODO
      selec *= period_joinsel_default(cachedOp);
    else
      /* Estimate join selectivity */
      selec *= period_joinsel(root, cachedOp, args, jointype, sjinfo);
  }

  CLAMP_PROBABILITY(selec);
  return (float8) selec;
}

/*
 * Estimate the join selectivity value of the operators for temporal
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
    PG_RETURN_FLOAT8(DEFAULT_TEMP_JOINSEL);

  /* Only respond to an inner join/unknown context join */
  if (jointype != JOIN_INNER)
    PG_RETURN_FLOAT8(DEFAULT_TEMP_JOINSEL);

  float8 result;
  assert(tempfamily == TEMPORALTYPE || tempfamily == TNUMBERTYPE ||
         tempfamily == TPOINTTYPE || tempfamily == TNPOINTTYPE);
  if (tempfamily == TEMPORALTYPE || tempfamily == TNUMBERTYPE)
    result = temporal_joinsel(root, operid, args, jointype, sjinfo,
      tempfamily);
  else /* (tempfamily == TPOINTTYPE || tempfamily == TNPOINTTYPE) */
  {
    int mode = Int32GetDatum(0) /* ND mode TO GENERALIZE */;
    result = tpoint_joinsel(root, operid, args, jointype, sjinfo,
      mode, tempfamily);
  }
  return result;
}

PG_FUNCTION_INFO_V1(Temporal_joinsel);
/*
 * Estimate the join selectivity value of the operators for temporal types
 * whose bounding box is a period, that is, tbool and ttext.
 */
PGDLLEXPORT Datum
Temporal_joinsel(PG_FUNCTION_ARGS)
{
  return temporal_joinsel_ext(fcinfo, TEMPORALTYPE);
}

/*****************************************************************************/
