/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2021, PostGIS contributors
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
 * Index support functions for temporal types.
 */

// #include "../mobdb_config.h"

#if POSTGRESQL_VERSION_NUMBER >= 120000

/* PostgreSQL */
#include "postgres.h"
#include "funcapi.h"
#include "access/htup_details.h"
#include "access/stratnum.h"
#include "catalog/namespace.h"
#include "catalog/pg_opfamily.h"
#include "catalog/pg_type_d.h"
#include "catalog/pg_am_d.h"
#include "nodes/supportnodes.h"
#include "nodes/nodeFuncs.h"
#include "nodes/makefuncs.h"
#include "optimizer/optimizer.h"
#include "parser/parse_func.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "utils/numeric.h"
#include "utils/syscache.h"

/* PostGIS */
// #include "liblwgeom.h"
// #include "lwgeom_pg.h"

/* MobilityDB */

#include "general/tempcache.h"
#include "point/tpoint_selfuncs.h"

/* Local prototypes */
Datum tpoint_index_supportfn(PG_FUNCTION_ARGS);

enum FUNCTION_IDX
{
  CONTAINS_IDX = 0,
  DISJOINT_IDX = 1,
  INTERSECTS_IDX = 2,
  TOUCHES_IDX = 3,
  DWITHIN_IDX = 4,
};

static const int16 TGeomPointStrategies[] = {
  [CONTAINS_IDX]               = RTOverlapStrategyNumber,
  [DISJOINT_IDX]               = RTOverlapStrategyNumber,
  [INTERSECTS_IDX]             = RTOverlapStrategyNumber,
  [TOUCHES_IDX]                = RTOverlapStrategyNumber,
  [DWITHIN_IDX]                = RTOverlapStrategyNumber,
};

/* We use InvalidStrategy for the functions that don't currently exist for geography */
static const int16 TGeogPointStrategies[] = {
  [CONTAINS_IDX]               = InvalidStrategy,
  [DISJOINT_IDX]               = RTOverlapStrategyNumber,
  [INTERSECTS_IDX]             = RTOverlapStrategyNumber,
  [TOUCHES_IDX]                = InvalidStrategy,
  [DWITHIN_IDX]                = RTOverlapStrategyNumber,
};

static int16
get_strategy_by_type(Oid firtype, uint16_t index)
{
  if (firtype == type_oid(T_TGEOMPOINT))
  {
    return TGeomPointStrategies[index];
  }

  if (firtype == type_oid(T_TGEOGPOINT))
  {
    return TGeogPointStrategies[index];
  }

  return InvalidStrategy;
}

/*
* Depending on the function, we will deploy different
* index enhancement strategies. Containment functions
* can use a more strict index strategy than overlapping
* functions. For within-distance functions, we need
* to construct expanded boxes, on the non-indexed
* function argument. We store the metadata to drive
* these choices in the IndexableFunctions array.
*/
typedef struct
{
  const char *fn_name;
  uint16_t index;     /* Position of the strategy in the arrays */
  uint8_t nargs;      /* Expected number of function arguments */
  uint8_t expand_arg; /* Radius argument for "within distance" search */
} IndexableFunction;

/*
* Metadata currently scanned from start to back,
* so most common functions first. Could be sorted
* and searched with binary search.
*/
static const IndexableFunction IndexableFunctions[] = {
  {"contains", CONTAINS_IDX, 2, 0},
  {"disjoint", DISJOINT_IDX, 2, 0},
  {"intersects", INTERSECTS_IDX, 2, 0},
  {"touches", TOUCHES_IDX, 2, 0},
  {"dwithin", DWITHIN_IDX, 3, 3},
  {NULL, 0, 0, 0}
};

/*
* Is the function calling the support function
* one of those we will enhance with index ops? If
* so, copy the metadata for the function into
* idxfn and return true. If false... how did the
* support function get added, anyways?
*/
static bool
needsSpatiotemporalIndex(Oid funcid, IndexableFunction *idxfn)
{
  const IndexableFunction *idxfns = IndexableFunctions;
  const char *fn_name = get_func_name(funcid);

  do
  {
    if(strcmp(idxfns->fn_name, fn_name) == 0)
    {
      *idxfn = *idxfns;
      return true;
    }
    idxfns++;
  }
  while (idxfns->fn_name);

  return false;
}

/*
* We only add spatial index enhancements for indexes that support 
* spatiotemporal searches (range based searches like the && operator), so only
* implementations based on GIST and SPGIST.
*/
static Oid
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
  // elog(NOTICE, "%s: found opfamily %s [%u]", __func__, opfamilyname, opfamilyam);
  ReleaseSysCache(familytup);
  return opfamilyam;
}

/*
* To apply the "expand for radius search" pattern we need access to the expand
* function, so lookup the function Oid using the function name and type number.
*/
static Oid
expandFunctionOid(Oid geotype, Oid callingfunc)
{
  const Oid radiustype = FLOAT8OID; /* Should always be FLOAT8OID */
  const Oid expandfn_args[2] = {geotype, radiustype};
  const bool noError = true;
  /* Expand function must be in same namespace as the caller */
  char *nspname = get_namespace_name(get_func_namespace(callingfunc));
  List *expandfn_name = list_make2(makeString(nspname), makeString("expand"));
  Oid expandfn_oid = LookupFuncName(expandfn_name, 2, expandfn_args, noError);
  if (expandfn_oid == InvalidOid)
  {
    /*
    * This is ugly, but we first lookup the geometry variant of expand
    * and if we fail, we look up the geography variant. The alternative
    * is re-naming the geography variant to match the geometry
    * one, which would not be the end of the world.
    */
    expandfn_name = list_make2(makeString(nspname), makeString("_expand"));
    expandfn_oid = LookupFuncName(expandfn_name, 2, expandfn_args, noError);
    if (expandfn_oid == InvalidOid)
      elog(ERROR, "%s: unable to lookup 'expand(Oid[%u], Oid[%u])'", __func__, geotype, radiustype);
  }
  return expandfn_oid;
}

/*
* For functions that we want enhanced with spatial
* index lookups, add this support function to the
* SQL function defintion, for example:
*
* CREATE OR REPLACE FUNCTION intersects(tgeompoint, tgeompoint)
*  RETURNS boolean
*  AS 'MODULE_PATHNAME','intersects_tpoint_tpoint'
*  SUPPORT tpoint_index_supportfn
*  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE
*  COST 100;
*
* The function must also have an entry above in the IndexableFunctions array
* so that we know what index search strategy we want to apply.
*/
PG_FUNCTION_INFO_V1(tpoint_index_supportfn);
Datum tpoint_index_supportfn(PG_FUNCTION_ARGS)
{
  Node *rawreq = (Node *) PG_GETARG_POINTER(0);
  Node *ret = NULL;

  if (IsA(rawreq, SupportRequestSelectivity))
  {
    SupportRequestSelectivity *req = (SupportRequestSelectivity *) rawreq;
    Oid lt = exprType(linitial(req->args));
    Oid rt = exprType(lsecond(req->args));
    CachedType ltype = cachedtype_oid(lt);
    CachedType rtype = cachedtype_oid(rt);
    Oid oper = oper_oid(OVERLAPS_OP, ltype, rtype);
    
    if (req->is_join)
    {
      // TODO 
      // req->selectivity = tpoint_joinsel_internal(req->root, oper, req->args, req->jointype);
      PG_RETURN_POINTER((Node *)NULL);      
    }
    else
    {
      req->selectivity = tpoint_sel_internal(req->root, oper, req->args, req->varRelid);
      PG_RETURN_POINTER(req);
    }
  }

  /*
  * This support function is strictly for adding spatial index
  * support.
  */
  if (IsA(rawreq, SupportRequestIndexCondition))
  {
    SupportRequestIndexCondition *req = (SupportRequestIndexCondition *) rawreq;

    if (is_funcclause(req->node))  /* Something() */
    {
      FuncExpr *clause = (FuncExpr *) req->node;
      Oid funcid = clause->funcid;
      IndexableFunction idxfn = {NULL, 0, 0, 0};
      Oid opfamilyoid = req->opfamily; /* OPERATOR FAMILY of the index */

      if (needsSpatiotemporalIndex(funcid, &idxfn))
      {
        int nargs = list_length(clause->args);
        Node *leftarg, *rightarg;
        Oid leftdatatype, rightdatatype, oproid;
        bool swapped = false;

        /*
        * Only add an operator condition for GIST and SPGIST indexes.
        * Effectively this means only these opclasses will get automatic
        * indexing when used with one of the indexable functions
        * gist_tgeompoint_ops, gist_tgeogpoint_ops,
        * spgist_tgeompoint_ops, spgist_tgeogpoint_ops,
        * spgigeometry_ops_2d, spgigeometry_ops_nd
        */
        Oid opfamilyam = opFamilyAmOid(opfamilyoid);
        if (opfamilyam != GIST_AM_OID && opfamilyam != SPGIST_AM_OID)
        {
          PG_RETURN_POINTER((Node *)NULL);
        }

        /*
        * We can only do something with index matches on the first
        * or second argument.
        */
        if (req->indexarg > 1)
          PG_RETURN_POINTER((Node *)NULL);

        /*
        * Make sure we have enough arguments.
        */
        if (nargs < 2 || nargs < idxfn.expand_arg)
          elog(ERROR, "%s: associated with function with %d arguments", __func__, nargs);

        /*
        * Extract "leftarg" as the arg matching
        * the index and "rightarg" as the other, even if
        * they were in the opposite order in the call.
        * NOTE: The functions we deal with here treat
        * their first two arguments symmetrically
        * enough that we needn't distinguish between
        * the two cases beyond this. Could be more
        * complications in the future.
        */
        if (req->indexarg == 0)
        {
          leftarg = linitial(clause->args);
          rightarg = lsecond(clause->args);
        }
        else
        {
          rightarg = linitial(clause->args);
          leftarg = lsecond(clause->args);
          swapped = true;
        }
        /*
        * Need the argument types (which should always be geometry/geography) as
        * this support function is only ever bound to functions
        * using those types.
        */
        leftdatatype = exprType(leftarg);
        rightdatatype = exprType(rightarg);

        /*
        * Given the index operator family and the arguments and the
        * desired strategy number we can now lookup the operator
        * we want (usually && or &&&).
        */
        oproid = get_opfamily_member(opfamilyoid, leftdatatype, rightdatatype,
          get_strategy_by_type(leftdatatype, idxfn.index));
        if (!OidIsValid(oproid))
          elog(ERROR, "no spatial operator found for '%s': opfamily %u type %d",
            idxfn.fn_name, opfamilyoid, leftdatatype);

        /*
        * For the DWithin variants we need to build a more complex return.
        * We want to expand the non-indexed side of the call by the
        * radius and then apply the operator.
        * dwithin(g1, g2, radius) yields this, if g1 is the indexarg:
        * g1 && expand(g2, radius)
        */
        if (idxfn.expand_arg)
        {
          Expr *expr;
          Node *radiusarg = (Node *) list_nth(clause->args, idxfn.expand_arg-1);
          Oid expandfn_oid = expandFunctionOid(rightdatatype, clause->funcid);

          FuncExpr *expandexpr = makeFuncExpr(expandfn_oid, rightdatatype,
            list_make2(rightarg, radiusarg), InvalidOid, InvalidOid,
            COERCE_EXPLICIT_CALL);

          /*
          * The comparison expression has to be a pseudo constant,
          * (not volatile or dependent on the target index table)
          */
#if POSTGRESQL_VERSION_NUMBER >= 140000
          if (!is_pseudo_constant_for_index(req->root, (Node*)expandexpr, req->index))
#else
          if (!is_pseudo_constant_for_index((Node*)expandexpr, req->index))
#endif
            PG_RETURN_POINTER((Node*)NULL);

          /* OK, we can make an index expression */
          expr = make_opclause(oproid, BOOLOID, false, (Expr *) leftarg,
            (Expr *) expandexpr, InvalidOid, InvalidOid);

          ret = (Node *)(list_make1(expr));
        }
        /*
        * For the Intersects variants we just need to return
        * an index OpExpr with the original arguments on each
        * side.
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
            PG_RETURN_POINTER((Node*)NULL);

          /*
          * Arguments were swapped to put the index value on the
          * left, so we need the commutated operator for
          * the OpExpr
          */
          if (swapped)
          {
            oproid = get_commutator(oproid);
            if (!OidIsValid(oproid))
              PG_RETURN_POINTER((Node *)NULL);
          }

          expr = make_opclause(oproid, BOOLOID, false, (Expr *) leftarg,
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
      else
      {
        elog(WARNING, "support function '%s' called from unsupported spatial function", __func__);
      }
    }
  }

  PG_RETURN_POINTER(ret);
}

#endif /* POSTGRESQL_VERSION_NUMBER >= 120000 */
