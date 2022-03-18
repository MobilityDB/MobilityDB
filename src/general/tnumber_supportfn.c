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
 * @file tnumber_supportfn.c
 * Index support functions for temporal types.
 */

#if POSTGRESQL_VERSION_NUMBER >= 120000

#include "general/temporal_supportfn.h"

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
#include "general/temporal_util.h"
#include "general/tnumber_selfuncs.h"

enum TNUMBER_FUNCTION_IDX
{
  /* Ever/always comparison functions */
  EVER_EQ_IDX                    = 0,
  ALWAYS_EQ_IDX                  = 1,
};

static const int16 TNumberStrategies[] =
{
  /* Ever/always comparison functions */
  [EVER_EQ_IDX]                  = RTOverlapStrategyNumber,
  [ALWAYS_EQ_IDX]                = RTOverlapStrategyNumber,
};

/**
 * Metadata currently scanned from start to back, so most common
 * functions first. Could be sorted and searched with binary search.
 */
static const IndexableFunction TNumberIndexableFunctions[] =
{
  /* Ever/always comparison functions */
  {"ever_eq", EVER_EQ_IDX, 2, 0},
  {"always_eq", ALWAYS_EQ_IDX, 2, 0},
  {NULL, 0, 0, 0}
};

static int16
tnumber_get_strategy_by_type(Oid type, uint16_t index)
{
  if (tnumber_type(type))
    return TNumberStrategies[index];
  return InvalidStrategy;
}

PG_FUNCTION_INFO_V1(tnumber_supportfn);
/**
 * For functions that we want enhanced with index lookups, add
 * this support function to the SQL function definition, for example:
 *
 * CREATE OR REPLACE FUNCTION ever_eq(tfloat, float)
 *   RETURNS boolean
 *   AS 'MODULE_PATHNAME','temporal_ever_eq'
 *   SUPPORT tnumber_supportfn
 *   LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 *
 * The function must also have an entry above in the TNumberIndexableFunctions array
 * so that we know what index search strategy we want to apply.
 */
Datum
tnumber_supportfn(PG_FUNCTION_ARGS)
{
  Node *rawreq = (Node *) PG_GETARG_POINTER(0);
  Node *ret = NULL;
  Oid funcoid, oproid, leftoid, rightoid;

  /* Return estimated selectivity */
  if (IsA(rawreq, SupportRequestSelectivity))
  {
    SupportRequestSelectivity *req = (SupportRequestSelectivity *) rawreq;
    Oid leftoid = exprType(linitial(req->args));
    Oid rightoid = exprType(lsecond(req->args));
    CachedType ltype = cachedtype_oid(leftoid);
    CachedType rtype = cachedtype_oid(rightoid);
    oproid = oper_oid(OVERLAPS_OP, ltype, rtype);
    if (req->is_join)
      req->selectivity = tnumber_joinsel_internal(req->root, oproid, req->args,
        req->jointype, req->sjinfo);
    else
      req->selectivity = tnumber_sel_internal(req->root, oproid, req->args,
        req->varRelid);
    PG_RETURN_POINTER(req);
  }

  /* Add index support */
  if (IsA(rawreq, SupportRequestIndexCondition))
  {
    SupportRequestIndexCondition *req = (SupportRequestIndexCondition *) rawreq;
    bool isfunc = is_funcclause(req->node); /* Something() */
    bool isbinop = isfunc ? false : /* left OP right */
      (is_opclause(req->node) &&
       list_length(((OpExpr *) req->node)->args) == 2);
    if (isfunc || isbinop)
    {
      Oid idx_oproid; /* operator of the new index support node generated */
      /* set depending on whether the node is of type FuncExpr or OpExpr */
      funcoid = InvalidOid; oproid = InvalidOid; 
      List *args;
      Node *leftarg, *rightarg;

      if (isfunc)
      {
        FuncExpr *clause = (FuncExpr *) req->node;
        funcoid = clause->funcid;
        args = clause->args;
      }
      else
      {
        OpExpr *clause = (OpExpr *) req->node;
        oproid = clause->opno;
        funcoid = clause->opfuncid;
        args = clause->args;
      }
      int nargs = list_length(args);
      IndexableFunction idxfn = {NULL, 0, 0, 0};
      Oid opfamilyoid = req->opfamily; /* OPERATOR FAMILY of the index */

      if (! func_needs_index(funcoid, TNumberIndexableFunctions, &idxfn))
      {
        if (isfunc)
          elog(WARNING, "support function called from unsupported spatial function %d",
            funcoid);
        else
          elog(WARNING, "support function called from unsupported spatial operator %d",
            oproid);
      }

      /*
       * Only add an operator condition for GIST and SPGIST indexes.
       * This means only the following opclasses
       *   tint_gist_ops, tfloat_gist_ops, tint_spgist_ops,
       *   tfloat_spgist_ops
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
      leftoid = exprType(leftarg);
      rightoid = exprType(rightarg);

      /*
       * Given the index operator family and the arguments and the desired
       * strategy number we can now lookup the operator we want (usually &&).
       */
      int16 strategy = tnumber_get_strategy_by_type(leftoid, idxfn.index);
      /* If no strategy was found for the left argument simply return */
      if (strategy == InvalidStrategy)
        PG_RETURN_POINTER((Node*) NULL);

      idx_oproid = get_opfamily_member(opfamilyoid, leftoid, rightoid,
        strategy);
      if (!OidIsValid(idx_oproid))
        elog(ERROR, "no temporal operator found for '%s': opfamily %u type %d",
          idxfn.fn_name, opfamilyoid, leftoid);

      /*
       * For temporal number types we currently do not use expand expressions
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

      expr = make_opclause(idx_oproid, BOOLOID, false, (Expr *) leftarg,
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
