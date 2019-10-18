/*****************************************************************************
 *
 * temporal_selfuncs.c
 * 
 * Functions for selectivity estimation of operators on temporal types whose
 * bounding box is a period, that is, tbool and ttext.
 *
 * The operators currently supported are as follows
 * - B-tree comparison operators: <, <=, >, >=
 * - Bounding box operators: &&, @>, <@, ~=
 * - Relative position operators: <<#, &<#, #>>, #>>
 * - Ever/always equal operators: &=, @= // TODO
 *
 * Due to implicit casting, a condition such as tbool <<# timestamptz will be
 * transformed into tbool <<# period. This allows to reduce the number of 
 * cases for the operator definitions, indexes, selectivity, etc. 
 * 
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_selfuncs.h"

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
 * The following functions are streamlined versions of the corresponding 
 * functions from PostgreSQL to only cover the types we are manipulating, 
 * that is, numbers and timestamps. Otherwise, it is necessary to copy too 
 * many other functions.
 *****************************************************************************/

/*
 * Do convert_to_scalar_mobdb()'s work for any number data type.
 */
static double
convert_numeric_to_scalar_mobdb(Oid typid, Datum value)
{
	switch (typid) {
		case BOOLOID:
			return (double) DatumGetBool(value);
		case INT2OID:
			return (double) DatumGetInt16(value);
		case INT4OID:
			return (double) DatumGetInt32(value);
		case INT8OID:
			return (double) DatumGetInt64(value);
		case FLOAT4OID:
			return (double) DatumGetFloat4(value);
		case FLOAT8OID:
			return (double) DatumGetFloat8(value);
		case NUMERICOID:
			/* Note: out-of-range values will be clamped to +-HUGE_VAL */
			return (double) DatumGetFloat8(DirectFunctionCall1(
												  numeric_float8_no_overflow, value));
		default:
			elog(ERROR, "unsupported type: %u", typid);
			return 0;
	}
}

/*
 * Do convert_to_scalar_mobdb()'s work for any timevalue data type.
 */
static double
convert_timevalue_to_scalar_mobdb(Oid typid, Datum value)
{
	switch (typid)
	{
		case TIMESTAMPOID:
			return DatumGetTimestamp(value);
		case TIMESTAMPTZOID:
			return DatumGetTimestampTz(value);
		case DATEOID:
			return date2timestamp_no_overflow(DatumGetDateADT(value));
		default:
			elog(ERROR, "unsupported type: %u", typid);
			return 0;
	}
}

static bool
convert_to_scalar_mobdb(Oid valuetypid, Datum value, double *scaledvalue,
	Datum lobound, Datum hibound, Oid boundstypid, double *scaledlobound,
	double *scaledhibound)
{
	/*
	* Both the valuetypid and the boundstypid should exactly match the
	* declared input type(s) of the operator we are invoked for, so we just
	* error out if either is not recognized.
	*
	* XXX The histogram we are interpolating between points of could belong
	* to a column that's only binary-compatible with the declared type. In
	* essence we are assuming that the semantics of binary-compatible types
	* are enough alike that we can use a histogram generated with one type's
	* operators to estimate selectivity for the other's.  This is outright
	* wrong in some cases --- in particular signed versus unsigned
	* interpretation could trip us up.  But it's useful enough in the
	* majority of cases that we do it anyway.  Should think about more
	* rigorous ways to do it.
	*/
	switch (valuetypid)
	{
		/*
		 * Built-in number types
		 */
		case BOOLOID:
		case INT2OID:
		case INT4OID:
		case INT8OID:
		case FLOAT4OID:
		case FLOAT8OID:
		case NUMERICOID:
			*scaledvalue = convert_numeric_to_scalar_mobdb(valuetypid, value);
			*scaledlobound = convert_numeric_to_scalar_mobdb(boundstypid, lobound);
			*scaledhibound = convert_numeric_to_scalar_mobdb(boundstypid, hibound);
			return true;

		/*
		* Built-in time types
		*/
		case TIMESTAMPTZOID:
			*scaledvalue = convert_timevalue_to_scalar_mobdb(valuetypid, value);
			*scaledlobound = convert_timevalue_to_scalar_mobdb(boundstypid, lobound);
			*scaledhibound = convert_timevalue_to_scalar_mobdb(boundstypid, hibound);
			return true;
	}
	/* Don't know how to convert */
	*scaledvalue = *scaledlobound = *scaledhibound = 0;
	return false;
}

/*****************************************************************************
 * The following functions are copied from PostgreSQL since they are not 
 * exported
 *****************************************************************************/

/*
 * Get one endpoint datum (min or max depending on indexscandir) from the
 * specified index.  Return true if successful, false if index is empty.
 * On success, endpoint value is stored to *endpointDatum (and copied into
 * outercontext).
 *
 * scankeys is a 1-element scankey array set up to reject nulls.
 * typLen/typByVal describe the datatype of the index's first column.
 * (We could compute these values locally, but that would mean computing them
 * twice when get_actual_variable_range needs both the min and the max.)
 */
static bool
get_actual_variable_endpoint(Relation heapRel,
							 Relation indexRel,
							 ScanDirection indexscandir,
							 ScanKey scankeys,
							 int16 typLen,
							 bool typByVal,
#if MOBDB_PGSQL_VERSION >= 120
							 TupleTableSlot *tableslot,
#endif
							 MemoryContext outercontext,
							 Datum *endpointDatum)
{
	bool		have_data = false;
	SnapshotData SnapshotNonVacuumable;
	IndexScanDesc index_scan;
	Buffer		vmbuffer = InvalidBuffer;
	ItemPointer tid;
	Datum		values[INDEX_MAX_KEYS];
	bool		isnull[INDEX_MAX_KEYS];
	MemoryContext oldcontext;

	/*
	 * We use the index-only-scan machinery for this.  With mostly-static
	 * tables that's a win because it avoids a heap visit.  It's also a win
	 * for dynamic data, but the reason is less obvious; read on for details.
	 *
	 * In principle, we should scan the index with our current active
	 * snapshot, which is the best approximation we've got to what the query
	 * will see when executed.  But that won't be exact if a new snap is taken
	 * before running the query, and it can be very expensive if a lot of
	 * recently-dead or uncommitted rows exist at the beginning or end of the
	 * index (because we'll laboriously fetch each one and reject it).
	 * Instead, we use SnapshotNonVacuumable.  That will accept recently-dead
	 * and uncommitted rows as well as normal visible rows.  On the other
	 * hand, it will reject known-dead rows, and thus not give a bogus answer
	 * when the extreme value has been deleted (unless the deletion was quite
	 * recent); that case motivates not using SnapshotAny here.
	 *
	 * A crucial point here is that SnapshotNonVacuumable, with
	 * RecentGlobalXmin as horizon, yields the inverse of the condition that
	 * the indexscan will use to decide that index entries are killable (see
	 * heap_hot_search_buffer()).  Therefore, if the snapshot rejects a tuple
	 * (or more precisely, all tuples of a HOT chain) and we have to continue
	 * scanning past it, we know that the indexscan will mark that index entry
	 * killed.  That means that the next get_actual_variable_endpoint() call
	 * will not have to re-consider that index entry.  In this way we avoid
	 * repetitive work when this function is used a lot during planning.
	 *
	 * But using SnapshotNonVacuumable creates a hazard of its own.  In a
	 * recently-created index, some index entries may point at "broken" HOT
	 * chains in which not all the tuple versions contain data matching the
	 * index entry.  The live tuple version(s) certainly do match the index,
	 * but SnapshotNonVacuumable can accept recently-dead tuple versions that
	 * don't match.  Hence, if we took data from the selected heap tuple, we
	 * might get a bogus answer that's not close to the index extremal value,
	 * or could even be NULL.  We avoid this hazard because we take the data
	 * from the index entry not the heap.
	 */
	InitNonVacuumableSnapshot(SnapshotNonVacuumable, RecentGlobalXmin);

	index_scan = index_beginscan(heapRel, indexRel,
								 &SnapshotNonVacuumable,
								 1, 0);
	/* Set it up for index-only scan */
	index_scan->xs_want_itup = true;
	index_rescan(index_scan, scankeys, 1, NULL, 0);

	/* Fetch first/next tuple in specified direction */
	while ((tid = index_getnext_tid(index_scan, indexscandir)) != NULL)
	{
		if (!VM_ALL_VISIBLE(heapRel,
							ItemPointerGetBlockNumber(tid),
							&vmbuffer))
		{
			/* Rats, we have to visit the heap to check visibility */
#if MOBDB_PGSQL_VERSION >= 120
			if (!index_fetch_heap(index_scan, tableslot))
#else
			if (index_fetch_heap(index_scan) == NULL)
#endif
				continue;		/* no visible tuple, try next index entry */

#if MOBDB_PGSQL_VERSION >= 120
			/* We don't actually need the heap tuple for anything */
			ExecClearTuple(tableslot);
#endif

			/*
			 * We don't care whether there's more than one visible tuple in
			 * the HOT chain; if any are visible, that's good enough.
			 */
		}

		/*
		 * We expect that btree will return data in IndexTuple not HeapTuple
		 * format.  It's not lossy either.
		 */
		if (!index_scan->xs_itup)
			elog(ERROR, "no data returned for index-only scan");
		if (index_scan->xs_recheck)
			elog(ERROR, "unexpected recheck indication from btree");

		/* OK to deconstruct the index tuple */
		index_deform_tuple(index_scan->xs_itup,
						   index_scan->xs_itupdesc,
						   values, isnull);

		/* Shouldn't have got a null, but be careful */
		if (isnull[0])
			elog(ERROR, "found unexpected null value in index \"%s\"",
				 RelationGetRelationName(indexRel));

		/* Copy the index column value out to caller's context */
		oldcontext = MemoryContextSwitchTo(outercontext);
		*endpointDatum = datumCopy(values[0], typByVal, typLen);
		MemoryContextSwitchTo(oldcontext);
		have_data = true;
		break;
	}

	if (vmbuffer != InvalidBuffer)
		ReleaseBuffer(vmbuffer);
	index_endscan(index_scan);

	return have_data;
}

/*
 * get_actual_variable_range
 *		Attempt to identify the current *actual* minimum and/or maximum
 *		of the specified variable, by looking for a suitable btree index
 *		and fetching its low and/or high values.
 *		If successful, store values in *min and *max, and return true.
 *		(Either pointer can be NULL if that endpoint isn't needed.)
 *		If no data available, return false.
 *
 * sortop is the "<" comparison operator to use.
 */
static bool
get_actual_variable_range(PlannerInfo *root, VariableStatData *vardata,
						  Oid sortop,
						  Datum *min, Datum *max)
{
	bool		have_data = false;
	RelOptInfo *rel = vardata->rel;
	RangeTblEntry *rte;
	ListCell   *lc;

	/* No hope if no relation or it doesn't have indexes */
	if (rel == NULL || rel->indexlist == NIL)
		return false;
	/* If it has indexes it must be a plain relation */
	rte = root->simple_rte_array[rel->relid];
	Assert(rte->rtekind == RTE_RELATION);

	/* Search through the indexes to see if any match our problem */
	foreach(lc, rel->indexlist)
	{
		IndexOptInfo *index = (IndexOptInfo *) lfirst(lc);
		ScanDirection indexscandir;

		/* Ignore non-btree indexes */
		if (index->relam != BTREE_AM_OID)
			continue;

		/*
		 * Ignore partial indexes --- we only want stats that cover the entire
		 * relation.
		 */
		if (index->indpred != NIL)
			continue;

		/*
		 * The index list might include hypothetical indexes inserted by a
		 * get_relation_info hook --- don't try to access them.
		 */
		if (index->hypothetical)
			continue;

		/*
		 * The first index column must match the desired variable and sort
		 * operator --- but we can use a descending-order index.
		 */
		if (!match_index_to_operand(vardata->var, 0, index))
			continue;
		switch (get_op_opfamily_strategy(sortop, index->sortopfamily[0]))
		{
			case BTLessStrategyNumber:
				if (index->reverse_sort[0])
					indexscandir = BackwardScanDirection;
				else
					indexscandir = ForwardScanDirection;
				break;
			case BTGreaterStrategyNumber:
				if (index->reverse_sort[0])
					indexscandir = ForwardScanDirection;
				else
					indexscandir = BackwardScanDirection;
				break;
			default:
				/* index doesn't match the sortop */
				continue;
		}

		/*
		 * Found a suitable index to extract data from.  Set up some data that
		 * can be used by both invocations of get_actual_variable_endpoint.
		 */
		{
			MemoryContext tmpcontext;
			MemoryContext oldcontext;
			Relation	heapRel;
			Relation	indexRel;
#if MOBDB_PGSQL_VERSION >= 120
			TupleTableSlot *slot;
#endif
			int16		typLen;
			bool		typByVal;
			ScanKeyData scankeys[1];

			/* Make sure any cruft gets recycled when we're done */
			tmpcontext = AllocSetContextCreate(CurrentMemoryContext,
											   "get_actual_variable_range workspace",
											   ALLOCSET_DEFAULT_SIZES);
			oldcontext = MemoryContextSwitchTo(tmpcontext);

			/*
			 * Open the table and index so we can read from them.  We should
			 * already have at least AccessShareLock on the table, but not
			 * necessarily on the index.
			 */
			heapRel = heap_open(rte->relid, NoLock);
			indexRel = index_open(index->indexoid, AccessShareLock);

			/* build some stuff needed for indexscan execution */
#if MOBDB_PGSQL_VERSION >= 120
			slot = table_slot_create(heapRel, NULL);
#endif
			get_typlenbyval(vardata->atttype, &typLen, &typByVal);

			/* set up an IS NOT NULL scan key so that we ignore nulls */
			ScanKeyEntryInitialize(&scankeys[0],
								   SK_ISNULL | SK_SEARCHNOTNULL,
								   1,	/* index col to scan */
								   InvalidStrategy, /* no strategy */
								   InvalidOid,	/* no strategy subtype */
								   InvalidOid,	/* no collation */
								   InvalidOid,	/* no reg proc for this */
								   (Datum) 0);	/* constant */

			/* If min is requested ... */
			if (min)
			{
				have_data = get_actual_variable_endpoint(heapRel,
														 indexRel,
														 indexscandir,
														 scankeys,
														 typLen,
														 typByVal,
#if MOBDB_PGSQL_VERSION >= 120
														 slot,
#endif
														 oldcontext,
														 min);
			}
			else
			{
				/* If min not requested, assume index is nonempty */
				have_data = true;
			}

			/* If max is requested, and we didn't find the index is empty */
			if (max && have_data)
			{
				/* scan in the opposite direction; all else is the same */
				have_data = get_actual_variable_endpoint(heapRel,
														 indexRel,
														 -indexscandir,
														 scankeys,
														 typLen,
														 typByVal,
#if MOBDB_PGSQL_VERSION >= 120
														 slot,
#endif
														 oldcontext,
														 max);
			}

			/* Clean everything up */
#if MOBDB_PGSQL_VERSION >= 120
			ExecDropSingleTupleTableSlot(slot);
#endif
			index_close(indexRel, AccessShareLock);
			heap_close(heapRel, NoLock);

			MemoryContextSwitchTo(oldcontext);
			MemoryContextDelete(tmpcontext);

			/* And we're done */
			break;
		}
	}

	return have_data;
}

/*
 *	ineq_histogram_selectivity_mobdb	- Examine the histogram for scalarineqsel
 *
 * Determine the fraction of the variable's histogram population that
 * satisfies the inequality condition, ie, VAR < (or <=, >, >=) CONST.
 * The isgt and iseq flags distinguish which of the four cases apply.
 *
 * Returns -1 if there is no histogram (valid results will always be >= 0).
 *
 * Note that the result disregards both the most-common-values (if any) and
 * null entries.  The caller is expected to combine this result with
 * statistics for those portions of the column population.
 */
// EZ We needed to add the operator as additional argument of the function
static double
ineq_histogram_selectivity_mobdb(PlannerInfo *root,
						   VariableStatData *vardata, Oid operator,
						   FmgrInfo *opproc, bool isgt, bool iseq,
						   Datum constval, Oid consttype)
{
	double		hist_selec;
	AttStatsSlot sslot;

	hist_selec = -1.0;

	/*
	 * Someday, ANALYZE might store more than one histogram per rel/att,
	 * corresponding to more than one possible sort ordering defined for the
	 * column type.  However, to make that work we will need to figure out
	 * which staop to search for --- it's not necessarily the one we have at
	 * hand!  (For example, we might have a '<=' operator rather than the '<'
	 * operator that will appear in staop.)  For now, assume that whatever
	 * appears in pg_statistic is sorted the same way our operator sorts, or
	 * the reverse way if isgt is true.
	 */
	if (HeapTupleIsValid(vardata->statsTuple) &&
		statistic_proc_security_check(vardata, opproc->fn_oid) &&
		// EZ replaced InvalidOid by operator
		get_attstatsslot(&sslot, vardata->statsTuple,
						 STATISTIC_KIND_HISTOGRAM, operator, 
						 ATTSTATSSLOT_VALUES))
	{
		if (sslot.nvalues > 1)
		{
			/*
			 * Use binary search to find the desired location, namely the
			 * right end of the histogram bin containing the comparison value,
			 * which is the leftmost entry for which the comparison operator
			 * succeeds (if isgt) or fails (if !isgt).  (If the given operator
			 * isn't actually sort-compatible with the histogram, you'll get
			 * garbage results ... but probably not any more garbage-y than
			 * you would have from the old linear search.)
			 *
			 * In this loop, we pay no attention to whether the operator iseq
			 * or not; that detail will be mopped up below.  (We cannot tell,
			 * anyway, whether the operator thinks the values are equal.)
			 *
			 * If the binary search accesses the first or last histogram
			 * entry, we try to replace that endpoint with the true column min
			 * or max as found by get_actual_variable_range().  This
			 * ameliorates misestimates when the min or max is moving as a
			 * result of changes since the last ANALYZE.  Note that this could
			 * result in effectively including MCVs into the histogram that
			 * weren't there before, but we don't try to correct for that.
			 */
			double		histfrac;
			int			lobound = 0;	/* first possible slot to search */
			int			hibound = sslot.nvalues;	/* last+1 slot to search */
			bool		have_end = false;

			/*
			 * If there are only two histogram entries, we'll want up-to-date
			 * values for both.  (If there are more than two, we need at most
			 * one of them to be updated, so we deal with that within the
			 * loop.)
			 */
			if (sslot.nvalues == 2)
				have_end = get_actual_variable_range(root,
													 vardata,
													 sslot.staop,
													 &sslot.values[0],
													 &sslot.values[1]);

			while (lobound < hibound)
			{
				int			probe = (lobound + hibound) / 2;
				bool		ltcmp;

				/*
				 * If we find ourselves about to compare to the first or last
				 * histogram entry, first try to replace it with the actual
				 * current min or max (unless we already did so above).
				 */
				if (probe == 0 && sslot.nvalues > 2)
					have_end = get_actual_variable_range(root,
														 vardata,
														 sslot.staop,
														 &sslot.values[0],
														 NULL);
				else if (probe == sslot.nvalues - 1 && sslot.nvalues > 2)
					have_end = get_actual_variable_range(root,
														 vardata,
														 sslot.staop,
														 NULL,
														 &sslot.values[probe]);

				ltcmp = DatumGetBool(FunctionCall2Coll(opproc,
													   DEFAULT_COLLATION_OID,
													   sslot.values[probe],
													   constval));
				if (isgt)
					ltcmp = !ltcmp;
				if (ltcmp)
					lobound = probe + 1;
				else
					hibound = probe;
			}

			if (lobound <= 0)
			{
				/*
				 * Constant is below lower histogram boundary.  More
				 * precisely, we have found that no entry in the histogram
				 * satisfies the inequality clause (if !isgt) or they all do
				 * (if isgt).  We estimate that that's true of the entire
				 * table, so set histfrac to 0.0 (which we'll flip to 1.0
				 * below, if isgt).
				 */
				histfrac = 0.0;
			}
			else if (lobound >= sslot.nvalues)
			{
				/*
				 * Inverse case: constant is above upper histogram boundary.
				 */
				histfrac = 1.0;
			}
			else
			{
				/* We have values[i-1] <= constant <= values[i]. */
				int			i = lobound;
				double		eq_selec = 0;
				double		val,
							high,
							low;
				double		binfrac;

				/*
				 * In the cases where we'll need it below, obtain an estimate
				 * of the selectivity of "x = constval".  We use a calculation
				 * similar to what var_eq_const() does for a non-MCV constant,
				 * ie, estimate that all distinct non-MCV values occur equally
				 * often.  But multiplication by "1.0 - sumcommon - nullfrac"
				 * will be done by our caller, so we shouldn't do that here.
				 * Therefore we can't try to clamp the estimate by reference
				 * to the least common MCV; the result would be too small.
				 *
				 * Note: since this is effectively assuming that constval
				 * isn't an MCV, it's logically dubious if constval in fact is
				 * one.  But we have to apply *some* correction for equality,
				 * and anyway we cannot tell if constval is an MCV, since we
				 * don't have a suitable equality operator at hand.
				 */
				if (i == 1 || isgt == iseq)
				{
					double		otherdistinct;
					bool		isdefault;
					AttStatsSlot mcvslot;

					/* Get estimated number of distinct values */
					otherdistinct = get_variable_numdistinct(vardata,
															 &isdefault);

					/* Subtract off the number of known MCVs */
					// EZ replaced InvalidOid by operator
					if (get_attstatsslot(&mcvslot, vardata->statsTuple,
										 STATISTIC_KIND_MCV, operator,
										 ATTSTATSSLOT_NUMBERS))
					{
						otherdistinct -= mcvslot.nnumbers;
						free_attstatsslot(&mcvslot);
					}

					/* If result doesn't seem sane, leave eq_selec at 0 */
					if (otherdistinct > 1)
						eq_selec = 1.0 / otherdistinct;
				}

				/*
				 * Convert the constant and the two nearest bin boundary
				 * values to a uniform comparison scale, and do a linear
				 * interpolation within this bin.
				 */
				if (convert_to_scalar_mobdb(constval, consttype, &val,
									  sslot.values[i - 1], sslot.values[i],
									  vardata->vartype,
									  &low, &high))
				{
					if (high <= low)
					{
						/* cope if bin boundaries appear identical */
						binfrac = 0.5;
					}
					else if (val <= low)
						binfrac = 0.0;
					else if (val >= high)
						binfrac = 1.0;
					else
					{
						binfrac = (val - low) / (high - low);

						/*
						 * Watch out for the possibility that we got a NaN or
						 * Infinity from the division.  This can happen
						 * despite the previous checks, if for example "low"
						 * is -Infinity.
						 */
						if (isnan(binfrac) ||
							binfrac < 0.0 || binfrac > 1.0)
							binfrac = 0.5;
					}
				}
				else
				{
					/*
					 * Ideally we'd produce an error here, on the grounds that
					 * the given operator shouldn't have scalarXXsel
					 * registered as its selectivity func unless we can deal
					 * with its operand types.  But currently, all manner of
					 * stuff is invoking scalarXXsel, so give a default
					 * estimate until that can be fixed.
					 */
					binfrac = 0.5;
				}

				/*
				 * Now, compute the overall selectivity across the values
				 * represented by the histogram.  We have i-1 full bins and
				 * binfrac partial bin below the constant.
				 */
				histfrac = (double) (i - 1) + binfrac;
				histfrac /= (double) (sslot.nvalues - 1);

				/*
				 * At this point, histfrac is an estimate of the fraction of
				 * the population represented by the histogram that satisfies
				 * "x <= constval".  Somewhat remarkably, this statement is
				 * true regardless of which operator we were doing the probes
				 * with, so long as convert_to_scalar() delivers reasonable
				 * results.  If the probe constant is equal to some histogram
				 * entry, we would have considered the bin to the left of that
				 * entry if probing with "<" or ">=", or the bin to the right
				 * if probing with "<=" or ">"; but binfrac would have come
				 * out as 1.0 in the first case and 0.0 in the second, leading
				 * to the same histfrac in either case.  For probe constants
				 * between histogram entries, we find the same bin and get the
				 * same estimate with any operator.
				 *
				 * The fact that the estimate corresponds to "x <= constval"
				 * and not "x < constval" is because of the way that ANALYZE
				 * constructs the histogram: each entry is, effectively, the
				 * rightmost value in its sample bucket.  So selectivity
				 * values that are exact multiples of 1/(histogram_size-1)
				 * should be understood as estimates including a histogram
				 * entry plus everything to its left.
				 *
				 * However, that breaks down for the first histogram entry,
				 * which necessarily is the leftmost value in its sample
				 * bucket.  That means the first histogram bin is slightly
				 * narrower than the rest, by an amount equal to eq_selec.
				 * Another way to say that is that we want "x <= leftmost" to
				 * be estimated as eq_selec not zero.  So, if we're dealing
				 * with the first bin (i==1), rescale to make that true while
				 * adjusting the rest of that bin linearly.
				 */
				if (i == 1)
					histfrac += eq_selec * (1.0 - binfrac);

				/*
				 * "x <= constval" is good if we want an estimate for "<=" or
				 * ">", but if we are estimating for "<" or ">=", we now need
				 * to decrease the estimate by eq_selec.
				 */
				if (isgt == iseq)
					histfrac -= eq_selec;
			}

			/*
			 * Now the estimate is finished for "<" and "<=" cases.  If we are
			 * estimating for ">" or ">=", flip it.
			 */
			hist_selec = isgt ? (1.0 - histfrac) : histfrac;

			/*
			 * The histogram boundaries are only approximate to begin with,
			 * and may well be out of date anyway.  Therefore, don't believe
			 * extremely small or large selectivity estimates --- unless we
			 * got actual current endpoint values from the table, in which
			 * case just do the usual sanity clamp.  Somewhat arbitrarily, we
			 * set the cutoff for other cases at a hundredth of the histogram
			 * resolution.
			 */
			if (have_end)
				CLAMP_PROBABILITY(hist_selec);
			else
			{
				double		cutoff = 0.01 / (double) (sslot.nvalues - 1);

				if (hist_selec < cutoff)
					hist_selec = cutoff;
				else if (hist_selec > 1.0 - cutoff)
					hist_selec = 1.0 - cutoff;
			}
		}

		free_attstatsslot(&sslot);
	}

	return hist_selec;
}

/*
 *	mcv_selectivity			- Examine the MCV list for selectivity estimates
 *
 * Determine the fraction of the variable's MCV population that satisfies
 * the predicate (VAR OP CONST), or (CONST OP VAR) if !varonleft.  Also
 * compute the fraction of the total column population represented by the MCV
 * list.  This code will work for any boolean-returning predicate operator.
 *
 * The function result is the MCV selectivity, and the fraction of the
 * total population is returned into *sumcommonp.  Zeroes are returned
 * if there is no MCV list.
 */
// EZ added the operator parameter
// The sufix was added since the function mcv_selectivity is exported in selfuncs.h
static double
mcv_selectivity_mobdb(VariableStatData *vardata, Oid operator, FmgrInfo *opproc,
				Datum constval, bool varonleft, double *sumcommonp)
{
	double		mcv_selec,
				sumcommon;
	AttStatsSlot sslot;
	int			i;

	mcv_selec = 0.0;
	sumcommon = 0.0;

	if (HeapTupleIsValid(vardata->statsTuple) &&
		statistic_proc_security_check(vardata, opproc->fn_oid) &&
		// EZ changed InvalidOid by parameter
		get_attstatsslot(&sslot, vardata->statsTuple,
						 STATISTIC_KIND_MCV, operator,
						 ATTSTATSSLOT_VALUES | ATTSTATSSLOT_NUMBERS))
	{
		for (i = 0; i < sslot.nvalues; i++)
		{
			if (varonleft ?
				DatumGetBool(FunctionCall2Coll(opproc,
											   DEFAULT_COLLATION_OID,
											   sslot.values[i],
											   constval)) :
				DatumGetBool(FunctionCall2Coll(opproc,
											   DEFAULT_COLLATION_OID,
											   constval,
											   sslot.values[i])))
				mcv_selec += sslot.numbers[i];
			sumcommon += sslot.numbers[i];
		}
		free_attstatsslot(&sslot);
	}

	*sumcommonp = sumcommon;
	return mcv_selec;
}

/*
 *	scalarineqsel		- Selectivity of "<", "<=", ">", ">=" for scalars.
 *
 * This is the guts of scalarltsel/scalarlesel/scalargtsel/scalargesel.
 * The isgt and iseq flags distinguish which of the four cases apply.
 *
 * The caller has commuted the clause, if necessary, so that we can treat
 * the variable as being on the left.  The caller must also make sure that
 * the other side of the clause is a non-null Const, and dissect that into
 * a value and datatype.  (This definition simplifies some callers that
 * want to estimate against a computed value instead of a Const node.)
 *
 * This routine works for any datatype (or pair of datatypes) known to
 * convert_to_scalar().  If it is applied to some other datatype,
 * it will return an approximate estimate based on assuming that the constant
 * value falls in the middle of the bin identified by binary search.
 */
double
scalarineqsel(PlannerInfo *root, Oid operator, bool isgt, bool iseq,
			  VariableStatData *vardata, Datum constval, Oid consttype)
{
	Form_pg_statistic stats;
	FmgrInfo	opproc;
	double		mcv_selec,
				hist_selec,
				sumcommon;
	double		selec;

	if (!HeapTupleIsValid(vardata->statsTuple))
	{
		/* no stats available, so default result */
		return DEFAULT_INEQ_SEL;
	}
	stats = (Form_pg_statistic) GETSTRUCT(vardata->statsTuple);

	fmgr_info(get_opcode(operator), &opproc);

	/*
	 * If we have most-common-values info, add up the fractions of the MCV
	 * entries that satisfy MCV OP CONST.  These fractions contribute directly
	 * to the result selectivity.  Also add up the total fraction represented
	 * by MCV entries.
	 */
	// EZ added the operator parameter
	mcv_selec = mcv_selectivity_mobdb(vardata, operator, &opproc, constval, true,
								&sumcommon);

	/*
	 * If there is a histogram, determine which bin the constant falls in, and
	 * compute the resulting contribution to selectivity.
	 */
	// EZ added the operator parameter
	hist_selec = ineq_histogram_selectivity_mobdb(root, vardata, operator,
											&opproc, isgt, iseq,
											constval, consttype);

	/*
	 * Now merge the results from the MCV and histogram calculations,
	 * realizing that the histogram covers only the non-null values that are
	 * not listed in MCV.
	 */
	selec = 1.0 - stats->stanullfrac - sumcommon;

	if (hist_selec >= 0.0)
		selec *= hist_selec;
	else
	{
		/*
		 * If no histogram but there are values not accounted for by MCV,
		 * arbitrarily assume half of them will match.
		 */
		selec *= 0.5;
	}

	selec += mcv_selec;

	/* result should be in range, but make sure... */
	CLAMP_PROBABILITY(selec);

	return selec;
}

/*
 * var_eq_const --- eqsel for var = const case
 *
 * This is split out so that some other estimation functions can use it.
 */
double
var_eq_const(VariableStatData *vardata, Oid operator,
			 Datum constval, bool constisnull,
			 bool varonleft, bool negate)
{
	double		selec;
	double		nullfrac = 0.0;
	bool		isdefault;
	Oid			opfuncoid;

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
		bool		match = false;
		int			i;

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
			FmgrInfo	eqproc;

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
			i = 0;				/* keep compiler quiet */
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
			double		sumcommon = 0.0;
			double		otherdistinct;

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

/*****************************************************************************
 * Internal functions computing selectivity
 * The functions assume that the value and time dimensions of temporal values 
 * are independent and thus the selectivity values obtained by analyzing the 
 * histograms for each dimension can be multiplied.
 *****************************************************************************/

/*
 * Transform the constant into a period
 */
static bool
temporal_const_to_period(Node *other, Period *period)
{
	Oid consttype = ((Const *) other)->consttype;

	if (consttype == TIMESTAMPTZOID)
	{
		TimestampTz t = DatumGetTimestampTz(((Const *) other)->constvalue);
		period_set(period, t, t, true, true);
	}
	else if (consttype == type_oid(T_TIMESTAMPSET))
		memcpy(period, timestampset_bbox(
				DatumGetTimestampSet(((Const *) other)->constvalue)), sizeof(Period));
	else if (consttype == type_oid(T_PERIOD))
		memcpy(period, DatumGetPeriod(((Const *) other)->constvalue), sizeof(Period));
	else if (consttype== type_oid(T_PERIODSET))
		memcpy(period, periodset_bbox(
				DatumGetPeriodSet(((Const *) other)->constvalue)), sizeof(Period));
	else if (consttype == type_oid(T_TBOOL) || consttype == type_oid(T_TTEXT))
		temporal_bbox(period, DatumGetTemporal(((Const *) other)->constvalue));
	else
		return false;
	return true;
}

/* Get the enum value associated to the operator */
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

/*
 * Returns a default selectivity estimate for given operator, when we don't
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

/* 
 * Compute selectivity for columns of TemporalInst duration 
 */
Selectivity
temporalinst_sel(PlannerInfo *root, VariableStatData *vardata,
	Period *period, CachedOp cachedOp)
{
	double selec = 0.0;
	Oid operator;
	bool iseq;

	if (cachedOp == SAME_OP || cachedOp == CONTAINS_OP)
	{
		/* If the period is not equivalent to a TimestampTz return 0.0 */
		if (period->lower != period->upper)
			return selec;
		
		operator = oper_oid(EQ_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		selec = var_eq_const(vardata, operator, TimestampTzGetDatum(period->lower), 
			false, false, false);
	}
	else if (cachedOp == CONTAINED_OP || cachedOp == OVERLAPS_OP)
	{
		/* 
		 * For TemporalInst, the two conditions TimestampTz t <@ Period p and
		 * TimestampTz t && Period p are equivalent. Furtheremore, if the
		 * lower and upper bounds of the period are inclusive, then
		 * 
		 * TimestampTz t <@ Period p <=> lower(p) <= t AND t <= upper(p)
		 * 		<=> NOT (lower(p) > t OR t > upper(p))
		 * 		<=> NOT (t < lower(p) OR t > upper(p))
		 *
		 * Since t < lower(p) and t > upper(p) are mutually exclusive 
		 * events we can sum their probabilities to find probability of 
		 * t < lower(p) OR t > upper(p). In the code that follows we
		 * take care of whether the lower bounds are inclusive or not
		 */			
		operator = period->lower_inc ? 
			oper_oid(LT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ) :
			oper_oid(LE_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		selec = scalarineqsel(root, operator, false, period->lower_inc, 
			vardata, TimestampTzGetDatum(period->lower), TIMESTAMPTZOID);
		operator = period->upper_inc ? 
			oper_oid(GT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ) :
			oper_oid(GE_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		selec += scalarineqsel(root, operator, true, period->upper_inc, 
			vardata, TimestampTzGetDatum(period->upper), TIMESTAMPTZOID);
		selec = fabs(1 - selec);
	}
	/* For b-tree comparisons, temporal values are first compared wrt 
	 * their bounding boxes, and if these are equal, other criteria apply.
	 * For selectivity estimation we approximate by taking into account
	 * only the bounding boxes. In the case here we use the scalar 
	 * inequality selectivity */
	else if (cachedOp == BEFORE_OP || cachedOp == LT_OP || cachedOp == LE_OP)
	{
		/* TimestampTz t <<# Period p <=> t < (<=) lower(p) depending on
		 * whether lower_inc(p) is true or false */
		operator = period->lower_inc ? 
			oper_oid(LT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ) :
			oper_oid(LE_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		iseq = (cachedOp == LE_OP) ? true : ! period->lower_inc;
		selec = scalarineqsel(root, operator, false, iseq, vardata, 
			period->lower, TIMESTAMPTZOID);
	}
	else if (cachedOp == AFTER_OP || cachedOp == GT_OP || cachedOp == GE_OP)
	{
		/* TimestampTz t #>> Period p <=> t > (>=) upper(p) depending on 
		 * whether lower_inc(p) is true or false */
		operator = period->upper_inc ? 
			oper_oid(GT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ) :
			oper_oid(GE_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		iseq = (cachedOp == LE_OP) ? true : ! period->upper_inc;
		selec = scalarineqsel(root, operator, true, iseq, vardata, 
			period->lower, TIMESTAMPTZOID);
	}
	else if (cachedOp == OVERBEFORE_OP)
	{
		/* TimestampTz t &<# Period p <=> t <= (<) upper(p) depending on 
		 * whether lower_inc(p) is true or false */
		operator = period->upper_inc ? 
			oper_oid(LE_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ) :
			oper_oid(LT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		selec = scalarineqsel(root, operator, false, period->upper_inc, vardata, 
			period->upper, TIMESTAMPTZOID);
	}
	else if (cachedOp == OVERAFTER_OP)
	{
		/* TimestampTz t #&> Period p <=> t >= (>) lower(p) depending on
		 * whether lower_inc(p) is true or false */
		operator = period->lower_inc ? 
			oper_oid(GE_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ) :
			oper_oid(GT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		selec = scalarineqsel(root, operator, true, period->lower_inc, vardata, 
			period->lower, TIMESTAMPTZOID);
	}
	else /* Unknown operator */
	{
		selec = default_temporal_selectivity(cachedOp);
	}
	return selec;
}

/*
 * Compute selectivity for columns of durations distinct from TemporalInst,
 * including columns containing temporal values of mixed durations.
 */
Selectivity
temporals_sel(PlannerInfo *root, VariableStatData *vardata,
	Period *period, CachedOp cachedOp)
{
	double selec = 0.0;
	Oid operator = oper_oid(cachedOp, T_PERIOD, T_PERIOD);

	/*
	 * There is no ~= operator for time types and thus it is necessary to
	 * take care of this operator here.
	 */
	if (cachedOp == SAME_OP)
	{
		operator = oper_oid(EQ_OP, T_PERIOD, T_PERIOD);
		selec = var_eq_const(vardata, operator, PeriodGetDatum(period), 
			false, false, false);
	}
	else if (cachedOp == OVERLAPS_OP || cachedOp == CONTAINS_OP ||
		cachedOp == CONTAINED_OP || cachedOp == BEFORE_OP ||
		cachedOp == AFTER_OP || cachedOp == OVERBEFORE_OP || 
		cachedOp == OVERAFTER_OP) 
	{
		selec = calc_period_hist_selectivity(vardata, period, cachedOp);
	}
	else if (cachedOp == LT_OP || cachedOp == LE_OP || 
		cachedOp == GT_OP || cachedOp == GE_OP) 
	{
		/* For b-tree comparisons, temporal values are first compared wrt 
		 * their bounding boxes, and if these are equal, other criteria apply.
		 * For selectivity estimation we approximate by taking into account
		 * only the bounding boxes. In the case here the bounding box is a
		 * period and thus we can use the period selectivity estimation */
		selec = calc_period_hist_selectivity(vardata, period, cachedOp);
	}
	else /* Unknown operator */
	{
		selec = default_temporal_selectivity(cachedOp);
	}
	return selec;
}

/*****************************************************************************/

/*
 * Estimate the selectivity value of the operators for temporal types whose
 * bounding box is a Period, that is, tbool and ttext.
 */
PG_FUNCTION_INFO_V1(temporal_sel);

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
	Selectivity selec = DEFAULT_TEMP_SELECTIVITY;
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

	/* Get the duration of the temporal column */
	int duration = TYPMOD_GET_DURATION(vardata.atttypmod);
	temporal_duration_all_is_valid(duration);

	/* Dispatch based on duration */
	if (duration == TEMPORALINST)
		selec = temporalinst_sel(root, &vardata, &constperiod, cachedOp);
	else
		selec = temporals_sel(root, &vardata, &constperiod, cachedOp);

	ReleaseVariableStats(vardata);
	CLAMP_PROBABILITY(selec);
	PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(temporal_joinsel);

PGDLLEXPORT Datum
temporal_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(DEFAULT_TEMP_SELECTIVITY);
}

/*****************************************************************************/
