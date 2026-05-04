/*
 * Copyright 2024 Zacharias Knudsen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <postgres.h>		 // Datum, etc.
#include <fmgr.h>			 // PG_FUNCTION_ARGS, etc.
#include <access/spgist.h>	   // SP-GiST
#include "catalog/pg_type.h"

#include <h3api.h> // Main H3 include
#include "type.h"
#include "error.h"

#include "inttypes.h"

#include "upstream_macros.h" // Technically not public API, but we need the bit macros

PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_spgist_config);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_spgist_choose);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_spgist_picksplit);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_spgist_inner_consistent);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_spgist_leaf_consistent);

#define H3_ROOT_INDEX -1
#define NUM_BASE_CELLS 122
#define MAX_H3_RES 15

#define H3_NUM_CHILDREN 7

static int
spgist_cmp(H3Index * a, H3Index * b)
{
	int			aRes = getResolution(*a);
	int			bRes = getResolution(*b);
	H3Index		aParent;
	H3Index		bParent;
	H3Error		error;

	cellToParent(*a, bRes, &aParent);
	cellToParent(*b, aRes, &bParent);

	/* a contains b */
	if (*a == H3_ROOT_INDEX || *a == bParent)
	{
		return 1;
	}

	/* a contained by b */
	if (*b == H3_ROOT_INDEX || *b == aParent)
	{
		return -1;
	}

	/* no overlap */
	return 0;
}

/*
 * Returns static information about the index implementation, including the data
 * type OIDs of the prefix and node label data types.
 *
 * The first argument is a pointer to a spgConfigIn C struct, containing input
 * data for the function.
 *
 * The second argument is a pointer to a spgConfigOut C struct, which the
 * function must fill with result data.
 */
Datum
h3index_spgist_config(PG_FUNCTION_ARGS)
{
	// struct {
	//   Oid  attType  data type to be indexed
	// }
	spgConfigIn *in = (spgConfigIn *) PG_GETARG_POINTER(0);

	// struct {
	//   Oid	prefixType		data type of inner-tuple prefixes
	//   Oid	labelType		data type of inner-tuple node labels
	//   Oid	leafType		data type of leaf-tuple values
	//   bool	canReturnData	opclass can reconstruct original data
	//   bool	longValuesOK	opclass can cope with values > 1 page
	// }
	spgConfigOut *out = (spgConfigOut *) PG_GETARG_POINTER(1);

	/* prefix is parent H3 index */
	out->prefixType = in->attType;
	/* no need for labels */
	out->labelType = VOIDOID;

	/*
	 * leafType should match the index storage type defined by the operator
	 * class's opckeytype catalog entry. (Note that opckeytype can be zero,
	 * implying the storage type is the same as the operator class's input type,
	 * which is the most common situation.) For reasons of backward compatibility,
	 * the config method can set leafType to some other value, and that value will
	 * be used; but this is deprecated since the index contents are then
	 * incorrectly identified in the catalogs. Also, it's permissible to leave
	 * leafType uninitialized (zero); that is interpreted as meaning the index
	 * storage type derived from opckeytype.
	 */
	//out->leafType = 0;

	/*
	 * canReturnData should be set true if the operator class is capable of
	 * reconstructing the originally-supplied index value
	 */
	out->canReturnData = true;
	out->longValuesOK = false;

	PG_RETURN_VOID();
}

/*
 * h3index_spgist_choose
 *		Chooses a method for inserting a new value into an inner tuple
 *
 * The choose function can determine either that the new value matches one of
 * the existing child nodes, or that a new child node must be added, or that the
 * new value is inconsistent with the tuple prefix and so the inner tuple must
 * be split to create a less restrictive prefix.
 *
 * NOTE: When working with an inner tuple having unlabeled nodes, it is an error
 * for choose to return spgAddNode, since the set of nodes is supposed to be
 * fixed in such cases. Also, there is no provision for generating an unlabeled
 * node in spgSplitTuple actions, since it is expected that an spgAddNode action
 * will be needed as well.
 */
Datum
h3index_spgist_choose(PG_FUNCTION_ARGS)
{
	/*--------------------------------------------------------------------------
	 * Datum	datum			original datum to be indexed
	 * Datum	leafDatum		current datum to be stored at leaf
	 * int		level			current level (counting from zero)
	 *
	 *	   (data from current inner tuple)
	 * bool		allTheSame		tuple is marked all-the-same?
	 * bool		hasPrefix		tuple has a prefix?
	 * Datum	prefixDatum		if so, the prefix value
	 * int		nNodes			number of nodes in the inner tuple
	 * Datum*	nodeLabels		node label values (NULL if none)
	 *--------------------------------------------------------------------------
	 */
	spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);

	/*--------------------------------------------------------------------------
	 * spgChooseResultType	resultType		action code, see below
	 *
	 *		-- results for spgMatchNode --
	 * int		nodeN		descend to this node (index from 0)
	 * int		levelAdd	increment level by this much
	 * Datum	restDatum	new leaf datum
	 *
	 *		-- results for spgAddNode --
	 * Datum	nodeLabel	new node's label
	 * int		nodeN		where to insert it (index from 0)
	 *
	 *		-- results for spgSplitTuple --
	 *		(info to form new upper-level inner tuple with one child tuple)
	 * bool		prefixHasPrefix		tuple should have a prefix?
	 * Datum	prefixPrefixDatum	if so, its value
	 * int		prefixNNodes		number of nodes
	 * Datum*	prefixNodeLabels	their labels (or NULL for no labels)
	 * int		childNodeN			which node gets child tuple
	 *		(info to form new lower-level inner tuple with all old nodes)
	 * bool		postfixHasPrefix	tuple should have a prefix?
	 * Datum	postfixPrefixDatum	if so, its value
	 *--------------------------------------------------------------------------
	 */
	spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);

	int			resolution = in->level;
	H3Index     insert = DatumGetH3Index(in->datum);
	int			node;

	out->resultType = spgMatchNode;
	out->result.matchNode.levelAdd = 1;
	out->result.matchNode.restDatum = H3IndexGetDatum(insert);

	if (!in->allTheSame)
	{
		if (resolution == 0)
		{
			node = getBaseCellNumber(insert);
		}
		else
		{
			node = H3_GET_INDEX_DIGIT(insert, resolution);
		}

		out->result.matchNode.nodeN = node;
	}

	PG_RETURN_VOID();
}

/**
 * Decides how to create a new inner tuple over a set of leaf tuples.
 *
 * An inner tuple contains a set of one or more nodes,
 * which represent groups of similar leaf values.
 *
 * struct spgPickSplitIn
 *	 int	nTuples number of leaf tuples
 *	 Datum *datums	their datums (array of length nTuples)
 *	 int	level	current level (counting from zero)
 *
 * struct spgPickSplitOut
 *	 bool	hasPrefix		 new inner tuple should have a prefix?
 *	 Datum	prefixDatum		 if so, its value
 *	 int	nNodes			 number of nodes for new inner tuple
 *	 Datum *nodeLabels		 their labels (or NULL for no labels)
 *	 int   *mapTuplesToNodes node index for each leaf tuple
 *	 Datum *leafTupleDatums  datum to store in each new leaf tuple
 */
Datum
h3index_spgist_picksplit(PG_FUNCTION_ARGS)
{
	spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
	spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
	int			resolution = in->level;

	/* we don't need node labels */
	out->nodeLabels = NULL;
	out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
	out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

	if (resolution == 0)
	{
		/* at resolution 0 there is one node per base cell */
		out->nNodes = NUM_BASE_CELLS;
		out->hasPrefix = false;
	}
	else
	{
		/* at finer resolutions there is exactly 7 nodes, one per child */
		H3Index first = DatumGetH3Index(in->datums[0]);
		H3Index parent;
		H3Error error;
		h3_assert(cellToParent(first, resolution, &parent));


		/*
		 * TODO: consider decreasing nNodes for pentagons which only have 6
		 * children?
		 */
		out->nNodes = H3_NUM_CHILDREN;
		out->hasPrefix = true;
		out->prefixDatum = H3IndexGetDatum(parent);
	}

	/* map each leaf tuple to node in the new inner tuple */
	for (int i = 0; i < in->nTuples; i++)
	{
		H3Index     insert = DatumGetH3Index(in->datums[i]);
		int			node;

		if (resolution == 0)
		{
			/* first resolution is base cells */
			node = getBaseCellNumber(insert);
		}
		else if (getResolution(insert) >= resolution)
		{
			/* finer resolutions use index digit 0-6 */
			node = H3_GET_INDEX_DIGIT(insert, resolution);
		}
		else
		{
			/* coarse indexes are put into center node */
			node = 0;
		}

		out->leafTupleDatums[i] = H3IndexGetDatum(insert);
		out->mapTuplesToNodes[i] = node;
	}

	PG_RETURN_VOID();
}

/**
 * Returns set of nodes (branches) to follow during tree search.
 *
 * Each query is a single H3 index to be checked against the parent prefix
 *
 * We either return all or none (except for res 0)
 */
Datum
h3index_spgist_inner_consistent(PG_FUNCTION_ARGS)
{
	spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
	spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
	H3Index     parent = H3_NULL;
	int			bc,
				i;
	bool		stop;
	int			innerNodes = in->nNodes;

	if (in->hasPrefix)
	{
		parent = DatumGetH3Index(in->prefixDatum);
	}

	if (in->allTheSame)
	{
		/* Report that all nodes should be visited */
		out->nNodes = in->nNodes;
		out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
		for (i = 0; i < in->nNodes; i++)
		{
			out->nodeNumbers[i] = i;
		}
		PG_RETURN_VOID();
	}

	out->levelAdds = palloc(sizeof(int) * innerNodes);
	for (i = 0; i < innerNodes; ++i)
		out->levelAdds[i] = 1;

	/* We must descend into the quadrant(s) identified by which */
	out->nodeNumbers = (int *) palloc(sizeof(int) * innerNodes);
	out->nNodes = 0;

	/* "which" is a bitmask of child nodes that satisfy all constraints */
	bc = -1;
	stop = false;
	for (i = 0; i < in->nkeys; i++)
	{
		/* each scankey is a constraint to be checked against */
		StrategyNumber strategy = in->scankeys[i].sk_strategy;
		H3Index    query = DatumGetH3Index(in->scankeys[i].sk_argument);

		if (parent == H3_NULL)
		{
			if (bc > -1)
			{
				stop = true;
			}
			bc = getBaseCellNumber(query);
		}
		else
		{
			switch (strategy)
			{
				case RTSameStrategyNumber:
					if (spgist_cmp(&parent, &query) == 0)
						stop = true;
					break;
				case RTContainsStrategyNumber:
					if (spgist_cmp(&parent, &query) == 0)
						stop = true;
					/* no overlap */
					break;
				case RTContainedByStrategyNumber:
					if (spgist_cmp(&parent, &query) == 0)
						stop = true;
					/* no overlap */
					break;
				default:
					elog(ERROR, "unrecognized strategy number: %d", strategy);
					break;
			}
		}

		if (stop)
			break;				/* no need to consider remaining conditions */
	}

	if (!stop)
	{
		if (bc > -1)
		{
			out->nodeNumbers[out->nNodes] = bc;
			out->nNodes++;
		}
		else
		{
			for (i = 0; i < innerNodes; i++)
			{
				out->nodeNumbers[out->nNodes] = i;
				out->nNodes++;
			}
		}
	}

	PG_RETURN_VOID();
}

/**
 * Returns true if a leaf satisfies a query.
 *
 * To satisfy the query, the leaf must satisfy all the conditions described by scankeys.
 */
Datum
h3index_spgist_leaf_consistent(PG_FUNCTION_ARGS)
{
	spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
	spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);

	H3Index     leaf = DatumGetH3Index(in->leafDatum);
	bool		retval = true;

	out->leafValue = in->leafDatum;
	/* leafDatum is what it is... */
	out->recheck = false;
	/* all tests are exact */

	/* Perform the required comparison(s) */
	for (int i = 0; i < in->nkeys; i++)
	{
		StrategyNumber strategy = in->scankeys[i].sk_strategy;
		H3Index    query = DatumGetH3Index(in->scankeys[i].sk_argument);

		switch (strategy)
		{
			case RTSameStrategyNumber:
				/* leaf is equal to query */
				retval = (leaf == query);
				break;
			case RTContainsStrategyNumber:
				/* leaf contains the query */
				retval = (spgist_cmp(&leaf, &query) > 0);
				break;
			case RTContainedByStrategyNumber:
				/* leaf is contained by the query */
				retval = (spgist_cmp(&leaf, &query) < 0);
				break;
			default:
				ereport(ERROR, (
								errcode(ERRCODE_INTERNAL_ERROR),
						 errmsg("unrecognized StrategyNumber: %d", strategy))
					);
		}

		if (!retval)
			break;
	}

	PG_RETURN_BOOL(retval);
}
