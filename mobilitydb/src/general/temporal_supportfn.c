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
 * @brief Index support functions for temporal types.
 */

#include "pg_general/temporal_supportfn.h"

/* C */
#include <assert.h>
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
/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h"
#include "general/temporal_boxops.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal_selfuncs.h"
#include "pg_point/tpoint_selfuncs.h"

enum TEMPORAL_FUNCTION_IDX
{
  /* Ever/always comparison functions */
  EVER_EQ_IDX                    = 1,
  ALWAYS_EQ_IDX                  = 2,
  /* Ever spatial relationships */
  ECONTAINS_IDX                   = 3,
  EDISJOINT_IDX                   = 4,
  EINTERSECTS_IDX                 = 5,
  ETOUCHES_IDX                    = 6,
  EDWITHIN_IDX                    = 7,
};

static const int16 TNumberStrategies[] =
{
  /* Ever/always comparison functions */
  [EVER_EQ_IDX]                  = RTOverlapStrategyNumber,
  [ALWAYS_EQ_IDX]                = RTOverlapStrategyNumber,
};

static const int16 TPointStrategies[] =
{
  /* Ever/always comparison functions */
  [EVER_EQ_IDX]                  = RTOverlapStrategyNumber,
  [ALWAYS_EQ_IDX]                = RTOverlapStrategyNumber,
  /* Ever spatial relationships */
  [ECONTAINS_IDX]                 = RTOverlapStrategyNumber,
  [EDISJOINT_IDX]                 = RTOverlapStrategyNumber,
  [EINTERSECTS_IDX]               = RTOverlapStrategyNumber,
  [ETOUCHES_IDX]                  = RTOverlapStrategyNumber,
  [EDWITHIN_IDX]                  = RTOverlapStrategyNumber,
};

#if NPOINT
static const int16 TNPointStrategies[] =
{
  /* Ever spatial relationships */
  [ECONTAINS_IDX]                 = RTOverlapStrategyNumber,
  [EDISJOINT_IDX]                 = RTOverlapStrategyNumber,
  [EINTERSECTS_IDX]               = RTOverlapStrategyNumber,
  [ETOUCHES_IDX]                  = RTOverlapStrategyNumber,
  [EDWITHIN_IDX]                  = RTOverlapStrategyNumber,
};
#endif /* NPOINT */

/*
* Metadata currently scanned from start to back,
* so most common functions first. Could be sorted
* and searched with binary search.
*/
static const IndexableFunction TemporalIndexableFunctions[] =
{
  {NULL, 0, 0, 0}
};

static const IndexableFunction TNumberIndexableFunctions[] = {
  /* Ever/always comparison functions */
  {"ever_eq", EVER_EQ_IDX, 2, 0},
  {"always_eq", ALWAYS_EQ_IDX, 2, 0},
  {NULL, 0, 0, 0}
};

static const IndexableFunction TPointIndexableFunctions[] = {
  /* Ever/always comparison functions */
  {"ever_eq", EVER_EQ_IDX, 2, 0},
  {"always_eq", ALWAYS_EQ_IDX, 2, 0},
  /* Ever spatial relationships */
  {"econtains", ECONTAINS_IDX, 2, 0},
  {"edisjoint", EDISJOINT_IDX, 2, 0},
  {"eintersects", EINTERSECTS_IDX, 2, 0},
  {"etouches", ETOUCHES_IDX, 2, 0},
  {"edwithin", EDWITHIN_IDX, 3, 3},
  {NULL, 0, 0, 0}
};

#if NPOINT
static const IndexableFunction TNPointIndexableFunctions[] = {
  /* Ever spatial relationships */
  {"econtains", ECONTAINS_IDX, 2, 0},
  {"edisjoint", EDISJOINT_IDX, 2, 0},
  {"eintersects", EINTERSECTS_IDX, 2, 0},
  {"etouches", ETOUCHES_IDX, 2, 0},
  {"edwithin", EDWITHIN_IDX, 3, 3},
  {NULL, 0, 0, 0}
};
#endif /* NPOINT */

static int16
temporal_get_strategy_by_type(meosType temptype, uint16_t index)
{
  if (tnumber_type(temptype))
    return TNumberStrategies[index];
  if (tgeo_type(temptype))
    return TPointStrategies[index];
#if NPOINT
  if (temptype == T_TNPOINT)
    return TNPointStrategies[index];
#endif /* NPOINT */
  return InvalidStrategy;
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Is the function calling the support function one of those we will
 * enhance with index ops? If so, copy the metadata for the function into idxfn
 * and return true. If false, how did the support function get added, anyways?
 */
bool
func_needs_index(Oid funcid, const IndexableFunction *idxfns,
  IndexableFunction *result)
{
  const char *fn_name = get_func_name(funcid);
  do
  {
    if (strcmp(idxfns->fn_name, fn_name) == 0)
    {
      *result = *idxfns;
      return true;
    }
    idxfns++;
  }
  while (idxfns->fn_name);

  return false;
}

/**
 * @brief We only add index enhancements for indexes that support range-based
 * searches like the && operator), so only implementations based on GIST
 * and SPGIST.
*/
Oid
opFamilyAmOid(Oid opfamilyoid)
{
  Form_pg_opfamily familyform;
  // char *opfamilyname;
  Oid opfamilyam;
  HeapTuple familytup = SearchSysCache1(OPFAMILYOID, ObjectIdGetDatum(opfamilyoid));
  if (!HeapTupleIsValid(familytup))
    elog(ERROR, "cache lookup failed for operator family %u", opfamilyoid);
  familyform = (Form_pg_opfamily) GETSTRUCT(familytup);
  opfamilyam = familyform->opfmethod;
  // opfamilyname = NameStr(familyform->opfname);
  // elog(NOTICE, "found opfamily %s [%u]", opfamilyname, opfamilyam);
  ReleaseSysCache(familytup);
  return opfamilyam;
}

/*****************************************************************************/

/**
 * @brief To apply the "expand for radius search" pattern we need access to the
 * expand function, so lookup the function Oid using the function name and
 * type number.
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
  char *funcname = NULL; /* make compiler quiet */
  meosType argtype = oid_type(argoid);
  if (argtype == T_GEOMETRY || argtype == T_GEOGRAPHY || argtype == T_STBOX ||
      argtype == T_TGEOMPOINT || argtype == T_TGEOGPOINT
#if NPOINT
      || argtype == T_TNPOINT
#endif /* NPOINT */
      )
    funcname = "expandspace";
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

/**
 * @brief To apply the "bunding box search" pattern we need access to the
 * corresponding bbox function, so lookup the function Oid using the function
 * name and type number.
 */
static FuncExpr *
makeBboxExpr(Node *arg, Oid argoid, Oid retoid, Oid callingfunc)
{
  const Oid funcargs[1] = {argoid};
  const bool noError = true;
  List *nspfunc;
  Oid funcoid;

  /* Expand function must be in same namespace as the caller */
  char *nspname = get_namespace_name(get_func_namespace(callingfunc));
  char *funcname = NULL; /* make compiler quiet */
  meosType argtype = oid_type(argoid);
  if (argtype == T_TBOOL || argtype == T_TTEXT)
    funcname = "span";
  else if (argtype == T_INT4 || argtype == T_FLOAT8 ||
           argtype == T_TINT || argtype == T_TFLOAT)
    funcname = "tbox";
  else if (argtype == T_GEOMETRY || argtype == T_GEOGRAPHY ||
      argtype == T_TGEOMPOINT || argtype == T_TGEOGPOINT
#if NPOINT
      || argtype == T_NPOINT || argtype == T_TNPOINT
#endif /* NPOINT */
      )
    funcname = "stbox";
  else
    elog(ERROR, "Unknown stbox function for type %d", argoid);
  nspfunc = list_make2(makeString(nspname), makeString(funcname));
  funcoid = LookupFuncName(nspfunc, 1, funcargs, noError);
  if (funcoid == InvalidOid)
    elog(ERROR, "unable to lookup '%s(Oid[%u])'", funcname, argoid);

  return makeFuncExpr(funcoid, retoid, list_make1(arg),
    InvalidOid, InvalidOid, COERCE_EXPLICIT_CALL);
}

/*****************************************************************************/

/**
 * @brief Transform the constant into a bounding box
 */
static meosType
type_to_bbox(meosType type)
{
  if (span_basetype(type))
    return basetype_spantype(type);
  if (set_type(type))
    return basetype_spantype(settype_basetype(type));
  if (spanset_type(type))
    return spansettype_spantype(type);
  if (spatial_basetype(type))
    return T_STBOX;
  return type;
}

/**
 * @brief For functions that we want enhanced with spatial index lookups, add
 * this support function to the SQL function definition, for example:
 * @code
 * CREATE OR REPLACE FUNCTION ever_eq(tfloat, float)
 *   RETURNS boolean
 *   AS 'MODULE_PATHNAME','temporal_ever_eq'
 *   SUPPORT temporal_supportfn
 *   LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 * @endcode
 * The function must also have an entry above in the IndexableFunctions array
 * so that we know what index search strategy we want to apply.
 */
Datum
temporal_supportfn_ext(FunctionCallInfo fcinfo, TemporalFamily tempfamily)
{
  Node *rawreq = (Node *) PG_GETARG_POINTER(0);
  Node *ret = NULL;
  Oid leftoid, rightoid, operid;

  /* Return estimated selectivity */
  assert (tempfamily == TEMPORALTYPE || tempfamily == TNUMBERTYPE ||
    tempfamily == TPOINTTYPE
#if NPOINT
    || tempfamily == TNPOINTTYPE
#endif /* NPOINT */
    );
  if (IsA(rawreq, SupportRequestSelectivity))
  {
    SupportRequestSelectivity *req = (SupportRequestSelectivity *) rawreq;
    leftoid = exprType(linitial(req->args));
    rightoid = exprType(lsecond(req->args));
    meosType ltype = oid_type(leftoid);
    meosType rtype = oid_type(rightoid);
    /* Convert base type to bbox type */
    meosType ltype1 = type_to_bbox(ltype);
    meosType rtype1 = type_to_bbox(rtype);
    operid = oper_oid(OVERLAPS_OP, ltype1, rtype1);
    if (req->is_join)
      req->selectivity = temporal_joinsel(req->root, operid, req->args,
        req->jointype, req->sjinfo, tempfamily);
    else
      req->selectivity = temporal_sel(req->root, operid, req->args,
        req->varRelid, tempfamily);
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
      Oid idxoperid;
      /* Oid of the right argument of the index support expression */
      Oid exproid;
      List *args;
      Node *leftarg, *rightarg;

      operid = InvalidOid;
      if (isfunc)
      {
        FuncExpr *funcexpr = (FuncExpr *) req->node;
        funcoid = funcexpr->funcid;
        args = funcexpr->args;
      }
      else
      {
        OpExpr *opexpr = (OpExpr *) req->node;
        operid = opexpr->opno;
        funcoid = opexpr->opfuncid;
        args = opexpr->args;
      }
      int nargs = list_length(args);
      IndexableFunction idxfn = {NULL, 0, 0, 0};
      Oid opfamilyoid = req->opfamily; /* Operator family of the index */
      const IndexableFunction *funcarr = NULL;
      if (tempfamily == TEMPORALTYPE)
        funcarr = TemporalIndexableFunctions;
      else if (tempfamily == TNUMBERTYPE)
        funcarr = TNumberIndexableFunctions;
      else if (tempfamily == TPOINTTYPE)
        funcarr = TPointIndexableFunctions;
#if NPOINT
      else if (tempfamily == TNPOINTTYPE)
        funcarr = TNPointIndexableFunctions;
#endif /* NPOINT */
      else
      {
        /* We should never arrive here */
        elog(WARNING, "Unknown temporal family for support functions: %d",
          tempfamily);
        PG_RETURN_POINTER((Node *) NULL);
      }
      if (! func_needs_index(funcoid, funcarr, &idxfn))
      {
        if (isfunc)
          elog(WARNING, "support function called from unsupported function %d",
            funcoid);
        else
          elog(WARNING, "support function called from unsupported operator %d",
            operid);
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
      meosType lefttype = oid_type(leftoid);
      meosType righttype = oid_type(rightoid);

      /*
       * Given the index operator family and the arguments and the desired
       * strategy number we can now lookup the operator we want (usually &&).
       */
      int16 strategy = temporal_get_strategy_by_type(lefttype, idxfn.index);
      /* If no strategy was found for the left argument simply return */
      if (strategy == InvalidStrategy)
        PG_RETURN_POINTER((Node *) NULL);

      /* Determine type of right argument of the index support expression
       * which is a bounding box */
      exproid = rightoid;
      if (righttype == T_TBOOL || righttype == T_TTEXT)
        exproid = type_oid(T_TSTZSPAN);
      else if (righttype == T_INT4 || righttype == T_FLOAT8 ||
          righttype == T_TINT || righttype == T_TFLOAT || righttype == T_TBOX)
        exproid = type_oid(T_TBOX);
      else if (righttype == T_GEOMETRY || righttype == T_GEOGRAPHY ||
          righttype == T_TGEOMPOINT || righttype == T_TGEOGPOINT ||
          righttype == T_STBOX
#if NPOINT
          || righttype == T_NPOINT || righttype == T_TNPOINT
#endif /* NPOINT */
          )
        exproid = type_oid(T_STBOX);
      else
        PG_RETURN_POINTER((Node *) NULL);

      idxoperid = get_opfamily_member(opfamilyoid, leftoid, exproid, strategy);
      if (idxoperid == InvalidOid)
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
        if (!is_pseudo_constant_for_index(req->root, (Node *) expandexpr,
          req->index))
#else
        if (!is_pseudo_constant_for_index((Node *)expandexpr, req->index))
#endif
          PG_RETURN_POINTER((Node *) NULL);

        /* OK, we can make an index expression */
        expr = make_opclause(idxoperid, BOOLOID, false, (Expr *) leftarg,
          (Expr *) expandexpr, InvalidOid, InvalidOid);

        ret = (Node *)(list_make1(expr));
      }
      /*
       * For the intersects variants we just need to return an index OpExpr
       * where the original argument on one side may be replaced by a bounding
       * box if it is not already one. For example, if g2 is a geometry then
       * intersects(g1, g2) yields: g1 && stbox(g2)
       */
      else
      {
        Expr *expr;
        FuncExpr *bboxexpr;
        if (bbox_type(righttype))
          bboxexpr = (FuncExpr *) rightarg;
        else
          bboxexpr = makeBboxExpr(rightarg, rightoid, exproid, funcoid);

        /*
         * The comparison expression has to be a pseudoconstant
         * (not volatile or dependent on the target index's table)
         */
#if POSTGRESQL_VERSION_NUMBER >= 140000
        if (!is_pseudo_constant_for_index(req->root, (Node *) bboxexpr,
          req->index))
#else
        if (!is_pseudo_constant_for_index((Node *) bboxexpr, req->index))
#endif
          PG_RETURN_POINTER((Node *) NULL);

        expr = make_opclause(idxoperid, BOOLOID, false, (Expr *) leftarg,
          (Expr *) bboxexpr, InvalidOid, InvalidOid);

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

PGDLLEXPORT Datum Tnumber_supportfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_supportfn);
/**
 * @brief Support function for temporal number types
 */
Datum
Tnumber_supportfn(PG_FUNCTION_ARGS)
{
  return temporal_supportfn_ext(fcinfo, TNUMBERTYPE);
}

PGDLLEXPORT Datum Tpoint_supportfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_supportfn);
/**
 * @brief Support function for temporal number types
 */
Datum
Tpoint_supportfn(PG_FUNCTION_ARGS)
{
  return temporal_supportfn_ext(fcinfo, TPOINTTYPE);
}

#if NPOINT
PGDLLEXPORT Datum Tnpoint_supportfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_supportfn);
/**
 * @brief Support function for temporal number types
 */
Datum
Tnpoint_supportfn(PG_FUNCTION_ARGS)
{
  return temporal_supportfn_ext(fcinfo, TNPOINTTYPE);
}
#endif /* NPOINT */

/*****************************************************************************/
