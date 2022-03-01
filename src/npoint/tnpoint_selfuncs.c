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
 * tnpoint_selfuncs.c
 * Functions for selectivity estimation of operators on temporal network points
 */

#include "npoint/tnpoint_selfuncs.h"

#include <assert.h>
#include <utils/syscache.h>

#include "general/period.h"
#include "general/temporal_util.h"
#include "general/temporal_selfuncs.h"

#include "point/tpoint_selfuncs.h"

/*****************************************************************************/

/**
 * Get the enum value associated to the operator
 * TODO Adapt to temporal network points
 */
static bool
tnpoint_cachedop(Oid oper, CachedOp *cachedOp)
{
  for (int i = OVERLAPS_OP; i <= OVERAFTER_OP; i++)
  {
    if (oper == oper_oid((CachedOp) i, T_STBOX, T_STBOX) ||
        oper == oper_oid((CachedOp) i, T_GEOMETRY, T_TGEOMPOINT) ||
        oper == oper_oid((CachedOp) i, T_STBOX, T_TGEOMPOINT) ||
        oper == oper_oid((CachedOp) i, T_TGEOMPOINT, T_GEOMETRY) ||
        oper == oper_oid((CachedOp) i, T_TGEOMPOINT, T_STBOX) ||
        oper == oper_oid((CachedOp) i, T_TGEOMPOINT, T_TGEOMPOINT) ||
        oper == oper_oid((CachedOp) i, T_GEOGRAPHY, T_TGEOGPOINT) ||
        oper == oper_oid((CachedOp) i, T_STBOX, T_TGEOGPOINT) ||
        oper == oper_oid((CachedOp) i, T_TGEOGPOINT, T_GEOGRAPHY) ||
        oper == oper_oid((CachedOp) i, T_TGEOGPOINT, T_STBOX) ||
        oper == oper_oid((CachedOp) i, T_TGEOGPOINT, T_TGEOGPOINT))
      {
        *cachedOp = (CachedOp) i;
        return true;
      }
  }
  return false;
}

/**
 * Transform the constant into an STBOX
 *
 * @note Due to implicit casting constants of type TimestampTz, TimestampSet,
 * Period, and PeriodSet are transformed into an STBOX
 */
bool
tnpoint_const_to_stbox(Node *other, STBOX *box)
{
  Oid consttype = ((Const *) other)->consttype;

  if (tgeo_base_type(consttype))
    geo_stbox((GSERIALIZED *) PointerGetDatum(((Const *) other)->constvalue),
      box);
  else if (consttype == type_oid(T_STBOX))
    memcpy(box, DatumGetSTboxP(((Const *) other)->constvalue), sizeof(STBOX));
  else if (tspatial_type(consttype))
    temporal_bbox(DatumGetTemporalP(((Const *) other)->constvalue), box);
  else
    return false;
  return true;
}

/*****************************************************************************/

/**
 * Estimate the restriction selectivity of the operators for temporal network points
 * (internal function)
 */
float8
tnpoint_sel_internal(PlannerInfo *root, Oid oper, List *args, int varRelid)
{
  VariableStatData vardata;
  Node *other;
  bool varonleft;
  Selectivity selec;
  CachedOp cachedOp;
  STBOX constBox;
  Period constperiod;

  /*
   * Get enumeration value associated to the operator
   */
  bool found = tnpoint_cachedop(oper, &cachedOp);
  /* In the case of unknown operator */
  if (!found)
    return DEFAULT_TEMP_SEL;

  /*
   * If expression is not (variable op something) or (something op
   * variable), then punt and return a default estimate.
   */
  if (!get_restriction_variable(root, args, varRelid, &vardata, &other,
      &varonleft))
    return tpoint_sel_default(cachedOp);

  /*
   * Can't do anything useful if the something is not a constant, either.
   */
  if (!IsA(other, Const))
  {
    ReleaseVariableStats(vardata);
    return tpoint_sel_default(cachedOp);
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
    oper = get_commutator(oper);
    if (!oper)
    {
      /* Use default selectivity (should we raise an error instead?) */
      ReleaseVariableStats(vardata);
      return tpoint_sel_default(cachedOp);
    }
  }

  /*
   * Transform the constant into an STBOX
   */
  found = tnpoint_const_to_stbox(other, &constBox);
  /* In the case of unknown constant */
  if (!found)
    return tpoint_sel_default(cachedOp);

  assert(MOBDB_FLAGS_GET_X(constBox.flags) || MOBDB_FLAGS_GET_T(constBox.flags));

  /* Enable the multiplication of the selectivity of the spatial and time
   * dimensions since either may be missing */
  selec = 1.0;

  /*
   * Estimate selectivity for the spatial dimension
   */
  if (MOBDB_FLAGS_GET_X(constBox.flags))
  {
    /* PostGIS does not provide selectivity for the traditional
     * comparisons <, <=, >, >= */
    if (cachedOp == LT_OP || cachedOp == LE_OP || cachedOp == GT_OP ||
      cachedOp == GE_OP)
      selec *= tpoint_sel_default(cachedOp);
    else
      selec *= geo_selectivity(&vardata, &constBox, cachedOp);
  }
  /*
   * Estimate selectivity for the time dimension
   */
  if (MOBDB_FLAGS_GET_T(constBox.flags))
  {
    /* Transform the STBOX into a Period */
    period_set(constBox.tmin, constBox.tmax, true, true, &constperiod);
    int16 subtype = TYPMOD_GET_SUBTYPE(vardata.atttypmod);
    ensure_valid_tempsubtype_all(subtype);

    /* Compute the selectivity */
    selec *= temporal_sel_period(&vardata, &constperiod, cachedOp);
  }

  ReleaseVariableStats(vardata);
  CLAMP_PROBABILITY(selec);
  return selec;
}

PG_FUNCTION_INFO_V1(tnpoint_sel);
/**
 * Estimate the restriction selectivity of the operators for temporal network points
 */
PGDLLEXPORT Datum
tnpoint_sel(PG_FUNCTION_ARGS)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid oper = PG_GETARG_OID(1);
  List *args = (List *) PG_GETARG_POINTER(2);
  int varRelid = PG_GETARG_INT32(3);
  float8 selec = tnpoint_sel_internal(root, oper, args, varRelid);
  PG_RETURN_FLOAT8((float8) selec);  
}

/*****************************************************************************
 * Join selectivity
 *****************************************************************************/

/**
 * Estimate the join selectivity of the operators for temporal network points
 * (internal function)
 */
float8
tnpoint_joinsel_internal(PlannerInfo *root, Oid oper, List *args,
  JoinType jointype, SpecialJoinInfo *sjinfo)
{
  VariableStatData vardata1, vardata2;
  bool join_is_reversed;
  float8 selec;

  /* Check length of args and punt on > 2 */
  if (list_length(args) != 2)
    return DEFAULT_TEMP_JOINSEL;

  /* Only respond to an inner join/unknown context join */
  if (jointype != JOIN_INNER)
    return DEFAULT_TEMP_JOINSEL;

  get_join_variables(root, args, sjinfo, &vardata1, &vardata2,
    &join_is_reversed);

  /*
   * Get enumeration value associated to the operator
   */
  CachedOp cachedOp;
  bool found = tnpoint_cachedop(oper, &cachedOp);
  /* In the case of unknown operator */
  if (!found)
  {
    ReleaseVariableStats(vardata1);
    ReleaseVariableStats(vardata2);
    return tpoint_joinsel_default(oper);
  }

  /* Estimate join selectivity TODO */
  // selec = tnpoint_joinsel_hist(&vardata1, &vardata2, cachedOp);
  selec = tpoint_joinsel_default(oper);

  ReleaseVariableStats(vardata1);
  ReleaseVariableStats(vardata2);
  CLAMP_PROBABILITY(selec);
  return (float8) selec;
}

PG_FUNCTION_INFO_V1(tnpoint_joinsel);
/**
 * Estimate the join selectivity of the operators for temporal network points
 */
PGDLLEXPORT Datum
tnpoint_joinsel(PG_FUNCTION_ARGS)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid oper = PG_GETARG_OID(1);
  List *args = (List *) PG_GETARG_POINTER(2);
  JoinType jointype = (JoinType) PG_GETARG_INT16(3);
  SpecialJoinInfo *sjinfo = (SpecialJoinInfo *) PG_GETARG_POINTER(4);
  float8 selec = tnpoint_joinsel_internal(root, oper, args, jointype, sjinfo);
  PG_RETURN_FLOAT8((float8) selec);
}

/*****************************************************************************/
