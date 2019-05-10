/*****************************************************************************
 *
 * IndexGistTnumber.c
 *	  R-tree GiST index for temporal integers and temporal floats
 *
 * These functions are based on those in the file gistproc.c.
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

 #include "TemporalTypes.h"

/*****************************************************************************
 * Leaf-level consistent method for temporal points using a box
 *****************************************************************************/

/*
 * Leaf-level consistency for boxes
 *
 * Since boxes do not distinguish between inclusive and exclusive bounds it is 
 * necessary to generalize the tests, e.g., 
 * left : (box1->high.x < box2->low.x) => (box1->high.x <= box2->low.x) 
 * e.g., to take into account left([a,b],(b,c])
 * right : (box1->low.x > box2->high.x) => (box1->low.x >= box2->high.x)
 * e.g., to take into account right((b,c],[a,b])
 * and similarly for before and after
 */
bool
index_leaf_consistent_box(BOX *key, BOX *query, StrategyNumber strategy)
{
	bool retval;
	
	switch (strategy)
	{
		case RTOverlapStrategyNumber:
			retval = overlaps_box_box_internal(key, query);
			break;
		case RTContainsStrategyNumber:
			retval = contains_box_box_internal(key, query);
			break;
		case RTContainedByStrategyNumber:
			retval = contained_box_box_internal(key, query);
			break;
		case RTSameStrategyNumber:
			retval = same_box_box_internal(key, query);
			break;
		case RTLeftStrategyNumber:
			retval = /* left_box_box_internal(key, query) */
				(key->high.x <= query->low.x); 
			break;
		case RTOverLeftStrategyNumber:
			retval = overleft_box_box_internal(key, query); 
			break;
		case RTRightStrategyNumber:
			retval = /* right_box_box_internal(key, query) */ 
				(key->low.x >= query->high.x); 
			break;
		case RTOverRightStrategyNumber:
			retval = overright_box_box_internal(key, query);
			break;
		case RTBeforeStrategyNumber:
			retval = /* before_box_box_internal(key, query) */
				(key->high.y <= query->low.y); 
			break;
		case RTOverBeforeStrategyNumber:
			retval = overbefore_box_box_internal(key, query); 
			break;
		case RTAfterStrategyNumber:
			retval = /* after_box_box_internal(key, query) */
				(key->low.y >= query->high.y); 
			break;
		case RTOverAfterStrategyNumber:
			retval = overafter_box_box_internal(key, query);
			break;			
		default:
			elog(ERROR, "unrecognized strategy number: %d", strategy);
			retval = false;		/* keep compiler quiet */
			break;
	}
	return retval;
}
		
/*****************************************************************************
 * Internal-page consistent method for temporal points using a box.
 *
 * Should return false if for all data items x below entry, the predicate 
 * x op query must be false, where op is the oper corresponding to strategy 
 * in the pg_amop table.
 *****************************************************************************/

static bool
gist_internal_consistent_box(BOX *key, BOX *query, StrategyNumber strategy)
{
	bool retval;
	
	switch (strategy)
	{
		case RTOverlapStrategyNumber:
		case RTContainedByStrategyNumber:
			retval = overlaps_box_box_internal(key, query);
			break;
		case RTContainsStrategyNumber:
		case RTSameStrategyNumber:
			retval = contains_box_box_internal(key, query);
			break;
		case RTLeftStrategyNumber:
			retval = !overright_box_box_internal(key, query);
			break;
		case RTOverLeftStrategyNumber:
			retval = !right_box_box_internal(key, query);
			break;
		case RTRightStrategyNumber:
			retval = !overleft_box_box_internal(key, query);
			break;
		case RTOverRightStrategyNumber:
			retval = !left_box_box_internal(key, query);
			break;
		case RTBeforeStrategyNumber:
			retval = !overafter_box_box_internal(key, query);
			break;
		case RTOverBeforeStrategyNumber:
			retval = !after_box_box_internal(key, query);
			break;
		case RTAfterStrategyNumber:
			retval = !overbefore_box_box_internal(key, query);
			break;
		case RTOverAfterStrategyNumber:
			retval = !before_box_box_internal(key, query);
			break;
		default:
			elog(ERROR, "unrecognized strategy number: %d", strategy);
			retval = false;		/* keep compiler quiet */
			break;
	}
	return retval;
}

/*****************************************************************************
 * GiST consistent method for temporal numbers
 *****************************************************************************/

PG_FUNCTION_INFO_V1(gist_tnumber_consistent);

PGDLLEXPORT Datum
gist_tnumber_consistent(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
	Oid subtype = PG_GETARG_OID(3);
	bool *recheck = (bool *) PG_GETARG_POINTER(4), result;
	BOX *key = DatumGetBoxP(entry->key), query;
	
	/* 
	 * All tests are lossy since boxes do not distinghish between inclusive  
	 * and exclusive bounds. 
	 */
	*recheck = true;
	
	if (key == NULL)
		PG_RETURN_BOOL(false);
	
	/*
	 * Transform the query into a box.
	 */
	if (subtype == BOXOID)
	{
		BOX *box = PG_GETARG_BOX_P(1);
		if (box == NULL)
			PG_RETURN_BOOL(false);
		query = *box;
	}
	else if (temporal_oid(subtype))
	{
		Temporal *temp = PG_GETARG_TEMPORAL(1);
		if (temp == NULL)
			PG_RETURN_BOOL(false);
		temporal_bbox(&query, temp);
		PG_FREE_IF_COPY(temp, 1);
	}
	else
		elog(ERROR, "unrecognized strategy number: %d", strategy);
	
	if (GIST_LEAF(entry))
		result = index_leaf_consistent_box(key, &query, strategy);
	else
		result = gist_internal_consistent_box(key, &query, strategy);
	
	PG_RETURN_BOOL(result);	
}

/*****************************************************************************
 * Compress methods for temporal numbers
 *****************************************************************************/

PG_FUNCTION_INFO_V1(gist_tnumber_compress);

PGDLLEXPORT Datum
gist_tnumber_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	if (entry->leafkey)
	{
		GISTENTRY *retval = palloc(sizeof(GISTENTRY));
		Temporal *temp = DatumGetTemporal(entry->key);
		BOX *box = palloc(sizeof(BOX));
		temporal_bbox(box, temp);
		gistentryinit(*retval, PointerGetDatum(box),
			entry->rel, entry->page, entry->offset, false);
		PG_RETURN_POINTER(retval);
	}
	PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * Fetch methods for temporal numbers (only for tintinst and tfloatinst)
 * Get point coordinates from its bounding box coordinates and form new
 * gistentry.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(gist_tintinst_fetch);

PGDLLEXPORT Datum
gist_tintinst_fetch(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	BOX *in = DatumGetBoxP(entry->key);
	GISTENTRY  *retval = palloc(sizeof(GISTENTRY));
	TemporalInst *inst = temporalinst_make(Int32GetDatum((int)in->high.x),
		in->high.y, INT4OID);
	gistentryinit(*retval, PointerGetDatum(inst),
		entry->rel, entry->page, entry->offset, false);
	PG_RETURN_POINTER(retval);
}

PG_FUNCTION_INFO_V1(gist_tfloatinst_fetch);

PGDLLEXPORT Datum
gist_tfloatinst_fetch(PG_FUNCTION_ARGS)
{
	GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	BOX *in = DatumGetBoxP(entry->key);
	GISTENTRY *retval = palloc(sizeof(GISTENTRY));
	TemporalInst *inst = temporalinst_make(Float8GetDatum(in->high.x), in->high.y, 
		FLOAT8OID);
	gistentryinit(*retval, PointerGetDatum(inst), 
		entry->rel, entry->page, entry->offset, false);
	pfree(inst);
	PG_RETURN_POINTER(retval);
}

/*****************************************************************************/
