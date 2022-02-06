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
 * @file tpoint_supportfn.c
 * Index support functions for temporal point types.
 */

#if POSTGRESQL_VERSION_NUMBER >= 120000

#include "general/temporal_supportfn.h"
#include "point/tpoint_supportfn.h"

/* PostgreSQL */
#include <postgres.h>
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
#include "point/tpoint_selfuncs.h"

enum TPOINT_FUNCTION_IDX
{
  /* Ever/always comparison functions */
  EVER_EQ_IDX                    = 0,
  ALWAYS_EQ_IDX                  = 1,
  /* intersects<Time> functions */
  INTERSECTS_TIMESTAMP_IDX       = 2,
  INTERSECTS_TIMESTAMPSET_IDX    = 3,
  INTERSECTS_PERIOD_IDX          = 4,
  INTERSECTS_PERIODSET_IDX       = 5,
  /* Ever spatial relationships */
  CONTAINS_IDX                   = 6,
  DISJOINT_IDX                   = 7,
  INTERSECTS_IDX                 = 8,
  TOUCHES_IDX                    = 9,
  DWITHIN_IDX                    = 10,
};

static const int16 TGeomPointStrategies[] =
{
  /* Ever/always comparison functions */
  [EVER_EQ_IDX]                  = RTOverlapStrategyNumber,
  [ALWAYS_EQ_IDX]                = RTOverlapStrategyNumber,
  /* intersects<Time> functions */
  [INTERSECTS_TIMESTAMP_IDX]     = RTOverlapStrategyNumber,
  [INTERSECTS_TIMESTAMPSET_IDX]  = RTOverlapStrategyNumber,
  [INTERSECTS_PERIOD_IDX]        = RTOverlapStrategyNumber,
  [INTERSECTS_PERIODSET_IDX]     = RTOverlapStrategyNumber,
  /* Ever spatial relationships */
  [CONTAINS_IDX]                 = RTOverlapStrategyNumber,
  [DISJOINT_IDX]                 = RTOverlapStrategyNumber,
  [INTERSECTS_IDX]               = RTOverlapStrategyNumber,
  [TOUCHES_IDX]                  = RTOverlapStrategyNumber,
  [DWITHIN_IDX]                  = RTOverlapStrategyNumber,
};

/**
 * We use InvalidStrategy for the functions that don't currently exist for
 * tgeogpoint/geography
 */
static const int16 TGeogPointStrategies[] =
{
  /* Ever/always comparison functions */
  [EVER_EQ_IDX]                  = RTOverlapStrategyNumber,
  [ALWAYS_EQ_IDX]                = RTOverlapStrategyNumber,
  /* intersects<Time> functions */
  [INTERSECTS_TIMESTAMP_IDX]     = RTOverlapStrategyNumber,
  [INTERSECTS_TIMESTAMPSET_IDX]  = RTOverlapStrategyNumber,
  [INTERSECTS_PERIOD_IDX]        = RTOverlapStrategyNumber,
  [INTERSECTS_PERIODSET_IDX]     = RTOverlapStrategyNumber,
  /* Ever spatial relationships */
  [CONTAINS_IDX]                 = InvalidStrategy,
  [DISJOINT_IDX]                 = RTOverlapStrategyNumber,
  [INTERSECTS_IDX]               = RTOverlapStrategyNumber,
  [TOUCHES_IDX]                  = InvalidStrategy,
  [DWITHIN_IDX]                  = RTOverlapStrategyNumber,
};

/**
 * Metadata currently scanned from start to back, so most common
 * functions first. Could be sorted and searched with binary search.
 */
static const IndexableFunction TPointIndexableFunctions[] = {
  /* Ever/always comparison functions */
  {"ever_eq", EVER_EQ_IDX, 2, 0},
  {"always_eq", ALWAYS_EQ_IDX, 2, 0},
  /* intersects<Time> functions */
  {"intersectstimestamp", INTERSECTS_TIMESTAMP_IDX, 2, 0},
  {"intersectstimestampset", INTERSECTS_TIMESTAMPSET_IDX, 2, 0},
  {"intersectsperiod", INTERSECTS_PERIOD_IDX, 2, 0},
  {"intersectsperiodset", INTERSECTS_PERIODSET_IDX, 2, 0},
  /* Ever spatial relationships */
  {"contains", CONTAINS_IDX, 2, 0},
  {"disjoint", DISJOINT_IDX, 2, 0},
  {"intersects", INTERSECTS_IDX, 2, 0},
  {"touches", TOUCHES_IDX, 2, 0},
  {"dwithin", DWITHIN_IDX, 3, 3},
  {NULL, 0, 0, 0}
};

/**
 * Dispatch the strategy lookup by type
 */
static int16
tpoint_get_strategy_by_type(Oid type, uint16_t index)
{
  if (type == type_oid(T_TGEOMPOINT))
    return TGeomPointStrategies[index];
  if (type == type_oid(T_TGEOGPOINT))
    return TGeogPointStrategies[index];
  return InvalidStrategy;
}

/**
 * To apply the "expand for radius search" pattern we need access to the expand
 * function, so lookup the function Oid using the function name and type number.
 */
static FuncExpr *
makeExpandExpr(Node *arg, Node *radiusarg, Oid argoid, Oid retoid,
  Oid callingfunc)
{
  const Oid radiusoid = FLOAT8OID;
  const Oid funcargs[2] = {argoid, radiusoid};
  const bool noError = true;
  List *nspfunc;
  Oid funcoid;

  /* Expand function must be in same namespace as the caller */
  char *nspname = get_namespace_name(get_func_namespace(callingfunc));
  char *funcname;
  if (argoid == type_oid(T_GEOMETRY) ||
      argoid == type_oid(T_GEOGRAPHY) ||
      argoid == type_oid(T_STBOX) ||
      argoid == type_oid(T_TGEOMPOINT) ||
      argoid == type_oid(T_TGEOGPOINT))
    funcname = "expandspatial";
  else
    elog(ERROR, "Unknown expand function for type %d", argoid);
  nspfunc = list_make2(makeString(nspname), makeString(funcname));
  funcoid = LookupFuncName(nspfunc, 2, funcargs, noError);
  if (funcoid == InvalidOid)
    elog(ERROR, "unable to lookup '%s(Oid[%u], Oid[%u])'", funcname,
      argoid, radiusoid);

  return makeFuncExpr(funcoid, retoid, list_make2(arg, radiusarg),
    InvalidOid, InvalidOid, COERCE_EXPLICIT_CALL);
}

PG_FUNCTION_INFO_V1(tpoint_supportfn);
/**
 * For functions that we want enhanced with spatial index lookups, add
 * this support function to the SQL function definition, for example:
 *
 * CREATE OR REPLACE FUNCTION intersects(tgeompoint, tgeompoint)
 *   RETURNS boolean
 *   AS 'MODULE_PATHNAME','intersects_tpoint_tpoint'
 *   SUPPORT tpoint_supportfn
 *   LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 *
 * The function must also have an entry above in the IndexableFunctions array
 * so that we know what index search strategy we want to apply.
 */
Datum
tpoint_supportfn(PG_FUNCTION_ARGS)
{
  Node *rawreq = (Node *) PG_GETARG_POINTER(0);
  Node *ret = NULL;
  Oid leftoid, rightoid, oproid;

  /* Return estimated selectivity */
  if (IsA(rawreq, SupportRequestSelectivity))
  {
    SupportRequestSelectivity *req = (SupportRequestSelectivity *) rawreq;
    leftoid = exprType(linitial(req->args));
    rightoid = exprType(lsecond(req->args));
    CachedType ltype = cachedtype_oid(leftoid);
    CachedType rtype = cachedtype_oid(rightoid);
    oproid = oper_oid(OVERLAPS_OP, ltype, rtype);
    if (req->is_join)
      req->selectivity = tpoint_joinsel_internal(req->root, oproid, req->args,
        req->jointype, Int32GetDatum(0) /* ND mode TO GENERALIZE */);
    else
      req->selectivity = tpoint_sel_internal(req->root, oproid, req->args,
        req->varRelid);
    PG_RETURN_POINTER(req);
  }

  /* Add index support */
  if (IsA(rawreq, SupportRequestIndexCondition))
  {
    SupportRequestIndexCondition *req = (SupportRequestIndexCondition *) rawreq;
    bool isfunc = is_funcclause(req->node); /* Function() */
    bool isbinop = isfunc ? false : /* left OP right */
      (is_opclause(req->node) &&
       list_length(((OpExpr *) req->node)->args) == 2);
    if (isfunc || isbinop)
    {
      /* Oid of the calling function or of the function associated to the
       * calling operator */
      Oid funcoid;
      /* Oid of the operator of the index support expression */
      Oid idxoproid;
      /* Oid of the right argument of the index support expression */
      Oid exproid;
      List *args;
      Node *leftarg, *rightarg;

      oproid = InvalidOid;
      if (isfunc)
      {
        FuncExpr *funcexpr = (FuncExpr *) req->node;
        funcoid = funcexpr->funcid;
        args = funcexpr->args;
      }
      else
      {
        OpExpr *opexpr = (OpExpr *) req->node;
        oproid = opexpr->opno;
        funcoid = opexpr->opfuncid;
        args = opexpr->args;
      }
      int nargs = list_length(args);
      IndexableFunction idxfn = {NULL, 0, 0, 0};
      Oid opfamilyoid = req->opfamily; /* Operator family of the index */

      if (! func_needs_index(funcoid, TPointIndexableFunctions, &idxfn))
      {
        if (isfunc)
          elog(WARNING, "support function called from unsupported function %d",
            funcoid);
        else
          elog(WARNING, "support function called from unsupported operator %d",
            oproid);
      }

      /*
       * Only add an operator condition for GIST and SPGIST indexes.
       * This means only the following opclasses
       *   tgeompoint_gist_ops, tgeogpoint_gist_ops,
       *   tgeompoint_spgist_ops, tgeogpoint_spgist_ops
       * will get automatic indexing when used with one of the indexable
       * functions
       */
      Oid opfamilyam = opFamilyAmOid(opfamilyoid);
      if (opfamilyam != GIST_AM_OID && opfamilyam != SPGIST_AM_OID)
        PG_RETURN_POINTER((Node *) NULL);

      /*
       * We can only do something with index matches on the first
       * or second argument.
       */
      if (req->indexarg > 1)
        PG_RETURN_POINTER((Node *) NULL);

      /* Make sure we have enough arguments */
      if (nargs < 2 || nargs < idxfn.expand_arg)
        elog(ERROR, "support function called from function %d with %d arguments",
          funcoid, nargs);

      /*
       * Extract "leftarg" as the arg matching the index and "rightarg" as
       * the other, even if they were in the opposite order in the call.
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
      leftoid = exprType(leftarg);
      rightoid = exprType(rightarg);

      /*
       * Given the index operator family and the arguments and the desired
       * strategy number we can now lookup the operator we want (usually &&).
       */
      int16 strategy = tpoint_get_strategy_by_type(leftoid, idxfn.index);
      /* If no strategy was found for the left argument simply return */
      if (strategy == InvalidStrategy)
        PG_RETURN_POINTER((Node *) NULL);

      /* Determine type of right argument of the index support expression
       * depending on whether there is an expand function */
      exproid = rightoid;
      if (idxfn.expand_arg &&
          (rightoid == type_oid(T_GEOMETRY) ||
           rightoid == type_oid(T_GEOGRAPHY) ||
           rightoid == type_oid(T_STBOX) ||
           rightoid == type_oid(T_TGEOMPOINT) ||
           rightoid == type_oid(T_TGEOGPOINT)))
          exproid = type_oid(T_STBOX);

      idxoproid = get_opfamily_member(opfamilyoid, leftoid, exproid, strategy);
      if (idxoproid == InvalidOid)
        elog(ERROR, "no operator found for '%s': opfamily %u type %d",
          idxfn.fn_name, opfamilyoid, leftoid);

      /*
       * For DWithin we need to build a more complex return.
       * We want to expand the non-indexed side of the call by the
       * radius and then apply the operator.
       * dwithin(temp1, temp2, radius) yields this, if temp1 is the indexarg:
       * temp1 && expand(temp2, radius)
       */
      if (idxfn.expand_arg)
      {
        Expr *expr;
        Node *radiusarg = (Node *) list_nth(args, idxfn.expand_arg - 1);

        FuncExpr *expandexpr = makeExpandExpr(rightarg, radiusarg, rightoid,
          exproid, funcoid);

        /*
         * The comparison expression has to be a pseudo constant,
         * (not volatile or dependent on the target index table)
         */
#if POSTGRESQL_VERSION_NUMBER >= 140000
        if (!is_pseudo_constant_for_index(req->root, (Node *)expandexpr, req->index))
#else
        if (!is_pseudo_constant_for_index((Node *)expandexpr, req->index))
#endif
          PG_RETURN_POINTER((Node *) NULL);

        /* OK, we can make an index expression */
        expr = make_opclause(idxoproid, BOOLOID, false, (Expr *) leftarg,
          (Expr *) expandexpr, InvalidOid, InvalidOid);

        ret = (Node *)(list_make1(expr));
      }
      /*
       * For the intersects variants we just need to return an index OpExpr
       * with the original arguments on each side. For example,
       * intersects(g1, g2) yields: g1 && g2
       */
      else
      {
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
          PG_RETURN_POINTER((Node *) NULL);

        expr = make_opclause(idxoproid, BOOLOID, false, (Expr *) leftarg,
          (Expr *) rightarg, InvalidOid, InvalidOid);

        ret = (Node *)(list_make1(expr));
      }

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
