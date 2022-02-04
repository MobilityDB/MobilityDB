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
 * @file temporal_supportfn.c
 * Index support functions for temporal types.
 */

#include "general/time_supportfn.h"

#if POSTGRESQL_VERSION_NUMBER >= 120000

/* PostgreSQL */
#include <postgres.h>
#include <assert.h>
#include <funcapi.h>
#include <access/htup_details.h>
#include <access/stratnum.h>
#include <catalog/namespace.h>
#include <catalog/pg_opfamily.h>
#include <catalog/pg_type_d.h>
#include <catalog/pg_am_d.h>
#include <nodes/supportnodes.h>
#include <nodes/nodeFuncs.h>
#include <nodes/makefuncs.h>
#include <optimizer/optimizer.h>
#include <parser/parse_func.h>
#include <utils/array.h>
#include <utils/builtins.h>
#include <utils/lsyscache.h>
#include <utils/numeric.h>
#include <utils/syscache.h>

/* MobilityDB */
#include "general/tempcache.h"
#include "general/temporal_selfuncs.h"

enum TEMPORAL_FUNCTION_IDX
{
  /* intersects<Time> functions */
  INTERSECTS_TIMESTAMP_IDX       = 0,
  INTERSECTS_TIMESTAMPSET_IDX    = 1,
  INTERSECTS_PERIOD_IDX          = 2,
  INTERSECTS_PERIODSET_IDX       = 3,
};

static const int16 TemporalStrategies[] = {
  /* intersects<Time> functions */
  [INTERSECTS_TIMESTAMP_IDX]     = RTOverlapStrategyNumber,
  [INTERSECTS_TIMESTAMPSET_IDX]  = RTOverlapStrategyNumber,
  [INTERSECTS_PERIOD_IDX]        = RTOverlapStrategyNumber,
  [INTERSECTS_PERIODSET_IDX]     = RTOverlapStrategyNumber,
};

/*
* Metadata currently scanned from start to back,
* so most common functions first. Could be sorted
* and searched with binary search.
*/
static const IndexableFunction TemporalIndexableFunctions[] = {
  /* intersects<Time> functions */
  {"intersectstimestamp", INTERSECTS_TIMESTAMP_IDX, 2, 0},
  {"intersectstimestampset", INTERSECTS_TIMESTAMPSET_IDX, 2, 0},
  {"intersectsperiod", INTERSECTS_PERIOD_IDX, 2, 0},
  {"intersectsperiodset", INTERSECTS_PERIODSET_IDX, 2, 0},
  {NULL, 0, 0, 0}
};

static int16
temporal_get_strategy_by_type(Oid type, uint16_t index)
{
  if (type == type_oid(T_TBOOL) || type == type_oid(T_TINT) ||
      type == type_oid(T_TFLOAT) || type == type_oid(T_TTEXT) )
    return TemporalStrategies[index];
  return InvalidStrategy;
}

/*
 * For functions that we want enhanced with spatial index lookups, add
 * this support function to the SQL function defintion, for example:
 *
 * CREATE OR REPLACE FUNCTION ever_eq(tfloat, float)
 *   RETURNS boolean
 *   AS 'MODULE_PATHNAME','temporal_ever_eq'
 *   SUPPORT temporal_supportfn
 *   LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 *
 * The function must also have an entry above in the IndexableFunctions array
 * so that we know what index search strategy we want to apply.
 */
PG_FUNCTION_INFO_V1(temporal_supportfn);
Datum temporal_supportfn(PG_FUNCTION_ARGS)
{
  Node *rawreq = (Node *) PG_GETARG_POINTER(0);
  Node *ret = NULL;

  /* Return estimated selectivity */
  if (IsA(rawreq, SupportRequestSelectivity))
  {
    SupportRequestSelectivity *req = (SupportRequestSelectivity *) rawreq;
    Oid lt = exprType(linitial(req->args));
    Oid rt = exprType(lsecond(req->args));
    CachedType ltype = cachedtype_oid(lt);
    CachedType rtype = cachedtype_oid(rt);
    Oid oper = oper_oid(OVERLAPS_OP, ltype, rtype);
    if (req->is_join)
      req->selectivity = temporal_joinsel_internal(req->root, oper, req->args,
        req->jointype);
    else
      req->selectivity = temporal_sel_internal(req->root, oper, req->args, req->varRelid);
    PG_RETURN_POINTER(req);
  }

  /* Add spatial index support */
  if (IsA(rawreq, SupportRequestIndexCondition))
  {
    SupportRequestIndexCondition *req = (SupportRequestIndexCondition *) rawreq;

    if (is_funcclause(req->node) ||  /* Something() */
       (is_opclause(req->node) &&
         list_length(((OpExpr *) req->node)->args) == 2)) /* LEFT OP RIGHT */
    {
      Oid funcid;
      List *args;
      if (is_funcclause(req->node))  /* Something() */
      {
        FuncExpr *clause = (FuncExpr *) req->node;
        funcid = clause->funcid;
        args = clause->args;
      }
      else
      {
        OpExpr *clause = (OpExpr *) req->node;
        funcid = clause->opfuncid;
        args = clause->args;
      }
      int nargs = list_length(args);
      IndexableFunction idxfn = {NULL, 0, 0, 0};
      Oid opfamilyoid = req->opfamily; /* OPERATOR FAMILY of the index */

      if (! func_needs_index(funcid, TemporalIndexableFunctions, &idxfn))
        elog(WARNING, "support function '%s' called from unsupported temporal function",
          __func__);

      Node *leftarg, *rightarg;
      Oid leftdatatype, rightdatatype, oproid;

      /*
       * Only add an operator condition for GIST and SPGIST indexes.
       * This means only these opclasses will get automatic indexing
       * when used with one of the following indexable functions:
       * gist_tgeompoint_ops, gist_tgeogpoint_ops,
       * spgist_tgeompoint_ops, spgist_tgeogpoint_ops
       */
      Oid opfamilyam = opFamilyAmOid(opfamilyoid);
      if (opfamilyam != GIST_AM_OID && opfamilyam != SPGIST_AM_OID)
        PG_RETURN_POINTER((Node *)NULL);

      /*
       * We can only do something with index matches on the first
       * or second argument.
       */
      if (req->indexarg > 1)
        PG_RETURN_POINTER((Node *)NULL);

      /* Make sure we have enough arguments */
      if (nargs < 2 || nargs < idxfn.expand_arg)
        elog(ERROR, "%s: associated with function with %d arguments", __func__, nargs);

      /*
       * Extract "leftarg" as the arg matching the index and "rightarg" as
       * the other,even if they were in the opposite order in the call.
       * N.B. This only works for symmetric operators like overlaps &&
       */
      if (req->indexarg == 0)
      {
        leftarg = linitial(args);
        rightarg = lsecond(args);
      }
      else
      {
        rightarg = linitial(args);
        leftarg = lsecond(args);
      }
      /*
       * Need the argument types as this support function is only ever bound
       * to functions using those types.
       */
      leftdatatype = exprType(leftarg);
      rightdatatype = exprType(rightarg);

      /*
      * Given the index operator family and the arguments and the
      * desired strategy number we can now lookup the operator
      * we want (usually && or &&&).
      */
      oproid = get_opfamily_member(opfamilyoid, leftdatatype, rightdatatype,
        temporal_get_strategy_by_type(leftdatatype, idxfn.index));
      if (!OidIsValid(oproid))
        elog(ERROR, "no temporal operator found for '%s': opfamily %u type %d",
          idxfn.fn_name, opfamilyoid, leftdatatype);

      /*
       * For temporal types we do not use expand expand expressions
       */
      assert(!idxfn.expand_arg);
      Expr *expr;
      /*
       * The comparison expression has to be a pseudoconstant
       * (not volatile or dependent on the target index's table)
       */
#if POSTGRESQL_VERSION_NUMBER >= 140000
      if (!is_pseudo_constant_for_index(req->root, rightarg, req->index))
#else
      if (!is_pseudo_constant_for_index(rightarg, req->index))
#endif
        PG_RETURN_POINTER((Node*)NULL);

      expr = make_opclause(oproid, BOOLOID, false, (Expr *) leftarg,
        (Expr *) rightarg, InvalidOid, InvalidOid);

      ret = (Node *)(list_make1(expr));

      /*
       * Set the lossy field on the SupportRequestIndexCondition parameter
       * to indicate that the index alone is not sufficient to evaluate
       * the condition. The function must also still be applied.
       */
      req->lossy = true;

      PG_RETURN_POINTER(ret);
    }
  }

  PG_RETURN_POINTER(ret);
}

#endif /* POSTGRESQL_VERSION_NUMBER >= 120000 */
