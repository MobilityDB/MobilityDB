/*****************************************************************************
 *
 * TemporalAnalyze.c
 *	  Functions for gathering statistics from temporal columns
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	src/TemporalAnalyze.c
 *
 *****************************************************************************/
#include <TemporalTypes.h>
#include <TemporalAnalyze.h>
/*****************************************************************************/


PG_FUNCTION_INFO_V1(temporal_analyze);
Datum
temporal_analyze(PG_FUNCTION_ARGS)
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	Datum result = 0;   /* keep compiler quiet */
	int type = TYPMOD_GET_DURATION(stats->attrtypmod);
	assert(type == TEMPORAL || type == TEMPORALINST || type == TEMPORALI ||
		   type == TEMPORALSEQ || type == TEMPORALS);
	result = temporal_analyze_internal(stats, type, TEMPORAL_STATISTIC);
	return result;
}

PG_FUNCTION_INFO_V1(tnumber_analyze);
Datum
tnumber_analyze(PG_FUNCTION_ARGS)
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	Datum result = 0;   /* keep compiler quiet */
	int durationType = TYPMOD_GET_DURATION(stats->attrtypmod);
	assert(durationType == TEMPORAL || durationType == TEMPORALINST || durationType == TEMPORALI ||
		   durationType == TEMPORALSEQ || durationType == TEMPORALS);
	result = temporal_analyze_internal(stats, durationType, TNUMBER_STATISTIC);
	return result;
}

Datum
temporal_analyze_internal(VacAttrStats *stats, int durationType, int temporalType)
{
	/*
	 * Call the standard typanalyze function.  It may fail to find needed
	 * operators, in which case we also can't do anything, so just fail.
	 */
	if (!std_typanalyze(stats))
		PG_RETURN_BOOL(false);

    if (durationType == TEMPORALINST)
		temporal_info(stats);
    else
        temporal_extra_info(stats);

	if (durationType == TEMPORALINST && temporalType == TEMPORAL_STATISTIC)
		stats->compute_stats = compute_timestamptz_stats;
	else if (durationType == TEMPORALINST && temporalType == TNUMBER_STATISTIC)
		stats->compute_stats = compute_temporalinst_twodim_stats;
	else if (durationType == TEMPORALI && temporalType == TEMPORAL_STATISTIC)
		stats->compute_stats = compute_timestamptz_set_stats;
	else if (durationType == TEMPORALI && temporalType == TNUMBER_STATISTIC)
		stats->compute_stats = compute_temporali_twodim_stats;
	else if (temporalType == TEMPORAL_STATISTIC)
		stats->compute_stats = compute_timestamptz_traj_stats;
	else if (temporalType == TNUMBER_STATISTIC)
		stats->compute_stats = compute_twodim_traj_stats;

	PG_RETURN_BOOL(true);
}

/*****************************************************************************
 * Statistics information for Temporal types
 *****************************************************************************/
TemporalArrayAnalyzeExtraData *array_extra_data;
void
temporal_info(VacAttrStats *stats)
{
	Oid ltopr;
	Oid eqopr;

	Form_pg_attribute attr = stats->attr;

	if (attr->attstattarget < 0)
		attr->attstattarget = default_statistics_target;

	Oid valueType = base_oid_from_temporal(stats->attrtypid);

	get_sort_group_operators(valueType,
							 false, false, false,
							 &ltopr, &eqopr, NULL,
							 NULL);
}
void
temporal_extra_info(VacAttrStats *stats)
{
	Form_pg_attribute attr = stats->attr;

	if (attr->attstattarget < 0)
		attr->attstattarget = default_statistics_target;

	Oid element_typeid;
	TypeCacheEntry *typentry;
	TypeCacheEntry *value_typentry;
	TypeCacheEntry *temporal_typentry;
	TemporalArrayAnalyzeExtraData *extra_data;

	/*
	 * Check attribute data type is a varlena array (or a domain over one).
	 */
	element_typeid = stats->attrtypid;
	if (!OidIsValid(element_typeid))
		elog(ERROR, "array_typanalyze was invoked for non-array type %u",
			 stats->attrtypid);

	/*
	 * Gather information about the element type.  If we fail to find
	 * something, return leaving the state from std_typanalyze() in place.
	 */
	typentry = lookup_type_cache(element_typeid,
								 TYPECACHE_EQ_OPR |
								 TYPECACHE_CMP_PROC_FINFO |
								 TYPECACHE_HASH_PROC_FINFO);

	value_typentry = lookup_type_cache(base_oid_from_temporal(element_typeid),
									   TYPECACHE_EQ_OPR |
									   TYPECACHE_CMP_PROC_FINFO |
									   TYPECACHE_HASH_PROC_FINFO);

	temporal_typentry = lookup_type_cache(type_oid(T_TIMESTAMPTZ),
										  TYPECACHE_EQ_OPR |
										  TYPECACHE_CMP_PROC_FINFO |
										  TYPECACHE_HASH_PROC_FINFO);

	/* Store our findings for use by compute_array_stats() */
	extra_data = (TemporalArrayAnalyzeExtraData *) palloc(sizeof(TemporalArrayAnalyzeExtraData));
	extra_data->type_id = typentry->type_id;
	extra_data->eq_opr = typentry->eq_opr;
	extra_data->typbyval = typentry->typbyval;
	extra_data->typlen = typentry->typlen;
	extra_data->typalign = typentry->typalign;
	extra_data->cmp = &typentry->cmp_proc_finfo;
	extra_data->hash = &typentry->hash_proc_finfo;

	extra_data->value_type_id = value_typentry->type_id;
	extra_data->value_eq_opr = value_typentry->eq_opr;
	extra_data->value_typbyval = value_typentry->typbyval;
	extra_data->value_typlen = value_typentry->typlen;
	extra_data->value_typalign = value_typentry->typalign;
	extra_data->value_cmp = &value_typentry->cmp_proc_finfo;
	extra_data->value_hash = &value_typentry->hash_proc_finfo;

	extra_data->temporal_type_id = temporal_typentry->type_id;
	extra_data->temporal_eq_opr = temporal_typentry->eq_opr;
	extra_data->temporal_typbyval = temporal_typentry->typbyval;
	extra_data->temporal_typlen = temporal_typentry->typlen;
	extra_data->temporal_typalign = temporal_typentry->typalign;
	extra_data->temporal_cmp = &temporal_typentry->cmp_proc_finfo;
	extra_data->temporal_hash = &temporal_typentry->hash_proc_finfo;

	extra_data->std_extra_data = stats->extra_data;
	stats->extra_data = extra_data;

	stats->minrows = 300 * attr->attstattarget;
}
/*****************************************************************************
 * Statistics functions for TemporalInst type
 *****************************************************************************/
void
compute_timestamptz_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
						  int samplerows, double totalrows)
{

	int null_cnt = 0;
	int non_null_cnt = 0;
	int temporalinst_no;
	int slot_idx;
	int num_bins = stats->attr->attstattarget;
	int num_hist;

	double total_width = 0;

	Oid timestamp_ltopr;
	Oid timestamp_eqopr;

	ScalarItem *timestamp_values;
	int *timestamp_tupnoLink;
	double corr_xysum;
	SortSupportData ssup;

	ScalarMCVItem *timestamp_track;
	int timestamp_track_cnt = 0;

	int num_mcv = stats->attr->attstattarget;

	timestamp_values = (ScalarItem *) palloc(samplerows * sizeof(ScalarItem));
	timestamp_tupnoLink = (int *) palloc(samplerows * sizeof(int));
	timestamp_track = (ScalarMCVItem *) palloc(num_mcv * sizeof(ScalarMCVItem));


	Oid timestampType = TIMESTAMPTZOID;
	bool timestamp_typbyval = true;
	int timestamp_typlen = sizeof(TimestampTz);


	get_sort_group_operators(timestampType,
							 false, false, false,
							 &timestamp_ltopr, &timestamp_eqopr, NULL,
							 NULL);

	Oid valueType = base_oid_from_temporal(stats->attrtypid);
	/* Loop over the sample periods. */
	for (temporalinst_no= 0; temporalinst_no < samplerows; temporalinst_no++)
	{
		Datum value;
		bool isnull;
		TemporalInst *inst;
		vacuum_delay_point();
		value = fetchfunc(stats, temporalinst_no, &isnull);
		if (isnull)
		{
			null_cnt++;
			continue;
		}

		/* The size of a TemporalInst is variable */
		total_width += VARSIZE_ANY(DatumGetPointer(value));

		/* Get TemporalInst and deserialize it for further analysis. */
		inst = DatumGetTemporalInst(value);

		timestamp_values[non_null_cnt].value = datumCopy(inst->t,
														 timestamp_typbyval,
														 timestamp_typlen);
		timestamp_values[non_null_cnt].tupno = temporalinst_no;
		timestamp_tupnoLink[non_null_cnt] = temporalinst_no;

		non_null_cnt++;

		/* Removing the temporal part from the stats HeapTuples if the base type is geometry */
		if(valueType == type_oid(T_GEOMETRY) || valueType == type_oid(T_GEOGRAPHY))
			stats->rows[temporalinst_no] = remove_temporaldim(stats->rows[temporalinst_no], stats->tupDesc, stats->tupDesc->natts, stats->attrtypid, true, value);
	}
	slot_idx = 2;

	/* If there's no useful features, we can't work out stats */
	if (!non_null_cnt)
	{
		elog(NOTICE, "no non-null/empty features, unable to compute statistics");
		stats->stats_valid = false;
		return;
	}

	/* We can only compute real stats if we found some non-null values. */
	if (non_null_cnt > 0)
	{
		int i;
		MemoryContext old_cxt;

		int timestamp_ndistinct,	/* # distinct values in sample */
				timestamp_nmultiple,	/* # that appear multiple times */
				timestamp_dups_cnt;
		CompareScalarsContext cxt;

		memset(&ssup, 0, sizeof(ssup));
		ssup.ssup_cxt = CurrentMemoryContext;
		/* We always use the default collation for statistics */
		ssup.ssup_collation = DEFAULT_COLLATION_OID;
		ssup.ssup_nulls_first = false;

		/*
		 * For now, don't perform abbreviated key conversion, because full values
		 * are required for MCV slot generation.  Supporting that optimization
		 * would necessitate teaching compare_scalars() to call a tie-breaker.
		 */
		ssup.abbreviate = false;

		PrepareSortSupportFromOrderingOp(timestamp_ltopr, &ssup);

		/* Sort the collected values */
		cxt.ssup = &ssup;
		cxt.tupnoLink = timestamp_tupnoLink;
		qsort_arg((void *) timestamp_values, (size_t)non_null_cnt, sizeof(ScalarItem),
				  compare_scalars, (void *) &cxt);

		/* Must copy the target values into anl_context */
		old_cxt = MemoryContextSwitchTo(stats->anl_context);


		corr_xysum = 0;
		timestamp_ndistinct = 0;
		timestamp_nmultiple = 0;
		timestamp_dups_cnt = 0;
		for (i = 0; i < non_null_cnt; i++)
		{
			int tupno = timestamp_values[i].tupno;

			corr_xysum += ((double) i) * ((double) tupno);
			timestamp_dups_cnt++;
			if (timestamp_tupnoLink[tupno] == tupno)
			{
				/* Reached end of duplicates of this value */
				timestamp_ndistinct++;
				if (timestamp_dups_cnt > 1)
				{
					timestamp_nmultiple++;
					if (timestamp_track_cnt < num_mcv ||
						timestamp_dups_cnt > timestamp_track[timestamp_track_cnt - 1].count)
					{
						/*
						 * Found a new item for the mcv list; find its
						 * position, bubbling down old items if needed. Loop
						 * invariant is that j points at an empty/ replaceable
						 * slot.
						 */
						int j;

						if (timestamp_track_cnt < num_mcv)
							timestamp_track_cnt++;
						for (j = timestamp_track_cnt - 1; j > 0; j--)
						{
							if (timestamp_dups_cnt <= timestamp_track[j - 1].count)
								break;
							timestamp_track[j].count = timestamp_track[j - 1].count;
							timestamp_track[j].first = timestamp_track[j - 1].first;
						}
						timestamp_track[j].count = timestamp_dups_cnt;
						timestamp_track[j].first = i + 1 - timestamp_dups_cnt;
					}
				}
				timestamp_dups_cnt = 0;
			}
		}


		/*
		 * Decide how many values are worth storing as most-common values. If
		 * we are able to generate a complete MCV list (all the values in the
		 * sample will fit, and we think these are all the ones in the table),
		 * then do so.  Otherwise, store only those values that are
		 * significantly more common than the (estimated) average. We set the
		 * threshold rather arbitrarily at 25% more than average, with at
		 * least 2 instances in the sample.  Also, we won't suppress values
		 * that have a frequency of at least 1/K where K is the intended
		 * number of histogram bins; such values might otherwise cause us to
		 * emit duplicate histogram bin boundaries.  (We might end up with
		 * duplicate histogram entries anyway, if the distribution is skewed;
		 * but we prefer to treat such values as MCVs if at all possible.)
		 *
		 * Note: the first of these cases is meant to address columns with
		 * small, fixed sets of possible values, such as boolean or enum
		 * columns.  If we can *completely* represent the column population by
		 * an MCV list that will fit into the stats target, then we should do
		 * so and thus provide the planner with complete information.  But if
		 * the MCV list is not complete, it's generally worth being more
		 * selective, and not just filling it all the way up to the stats
		 * target.  So for an incomplete list, we try to take only MCVs that
		 * are significantly more common than average.
		 */
		if (((timestamp_track_cnt) == (timestamp_ndistinct == 0)) &&
			(stats->stadistinct > 0) &&
			(timestamp_track_cnt <= num_mcv))
		{
			/* Track list includes all values seen, and all will fit */
			num_mcv = timestamp_track_cnt;
		}
		else
		{
			double timestamp_ndistinct_table = stats->stadistinct;
			double avgcount,
					mincount,
					maxmincount;

			/* Re-extract estimate of # distinct nonnull values in table */
			if (timestamp_ndistinct_table < 0)
				timestamp_ndistinct_table = -timestamp_ndistinct_table * totalrows;
			/* estimate # occurrences in sample of a typical nonnull value */
			avgcount = (double) non_null_cnt / timestamp_ndistinct_table;
			/* set minimum threshold count to store a value */
			mincount = avgcount * 1.25;
			if (mincount < 2)
				mincount = 2;
			/* don't let threshold exceed 1/K, however */
			maxmincount = (double) non_null_cnt / (double) num_bins;
			if (mincount > maxmincount)
				mincount = maxmincount;
			if (num_mcv > timestamp_track_cnt)
				num_mcv = timestamp_track_cnt;
			for (i = 0; i < num_mcv; i++)
			{
				if (timestamp_track[i].count < mincount)
				{
					num_mcv = i;
					break;
				}
			}
		}


		/* Generate MCV slot entry */
		if (num_mcv > 0)
		{
			MemoryContext old_context;
			Datum *mcv_values;
			float4 *mcv_freqs;

			/* Must copy the target values into anl_context */
			old_context = MemoryContextSwitchTo(stats->anl_context);
			mcv_values = (Datum *) palloc(num_mcv * sizeof(Datum));
			mcv_freqs = (float4 *) palloc(num_mcv * sizeof(float4));

			for (i = 0; i < num_mcv; i++)
			{
				mcv_values[i] = datumCopy(timestamp_values[timestamp_track[i].first].value,
										  timestamp_typbyval,
										  timestamp_typlen);
				mcv_freqs[i] = (float4) timestamp_track[i].count / (float4) samplerows;
			}
			MemoryContextSwitchTo(old_context);

			stats->stakind[slot_idx] = STATISTIC_KIND_MCV;
			stats->staop[slot_idx] = timestamp_eqopr;
			stats->stanumbers[slot_idx] = mcv_freqs;
			stats->numnumbers[slot_idx] = num_mcv;
			stats->stavalues[slot_idx] = mcv_values;
			stats->numvalues[slot_idx] = num_mcv;
			stats->statyplen[slot_idx] = (int16) timestamp_typlen;
			stats->statypid[slot_idx] = timestampType;
			stats->statypbyval[slot_idx] = timestamp_typbyval;
		}
		slot_idx++;

		/*
		 * Generate a histogram slot entry if there are at least two distinct
		 * values not accounted for in the MCV list.  (This ensures the
		 * histogram won't collapse to empty or a singleton.)
		 */
		num_hist = timestamp_ndistinct - num_mcv;
		if (num_hist > num_bins)
			num_hist = num_bins + 1;
		if (num_hist >= 2)
		{
			MemoryContext old_context;
			Datum *hist_values;
			int nvals;
			int pos,
					posfrac,
					delta,
					deltafrac;

			/* Sort the MCV items into position order to speed next loop */
			qsort((void *) timestamp_track, num_mcv,
				  sizeof(ScalarMCVItem), compare_mcvs);

			/*
			 * Collapse out the MCV items from the values[] array.
			 *
			 * Note we destroy the values[] array here... but we don't need it
			 * for anything more.  We do, however, still need values_cnt.
			 * nvals will be the number of remaining entries in values[].
			 */
			if (num_mcv > 0)
			{
				int src,
						dest;
				int j;

				src = dest = 0;
				j = 0;			/* index of next interesting MCV item */
				while (src < non_null_cnt)
				{
					int ncopy;

					if (j < num_mcv)
					{
						int first = timestamp_track[j].first;

						if (src >= first)
						{
							/* advance past this MCV item */
							src = first + timestamp_track[j].count;
							j++;
							continue;
						}
						ncopy = first - src;
					}
					else
						ncopy = non_null_cnt - src;
					memmove(&timestamp_values[dest], &timestamp_values[src],
							ncopy * sizeof(ScalarItem));
					src += ncopy;
					dest += ncopy;
				}
				nvals = dest;
			}
			else
				nvals = non_null_cnt;
			Assert(nvals >= num_hist);

			/* Must copy the target values into anl_context */
			old_context = MemoryContextSwitchTo(stats->anl_context);
			hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

			/*
			 * The object of this loop is to copy the first and last values[]
			 * entries along with evenly-spaced values in between.  So the
			 * i'th value is values[(i * (nvals - 1)) / (num_hist - 1)].  But
			 * computing that subscript directly risks integer overflow when
			 * the stats target is more than a couple thousand.  Instead we
			 * add (nvals - 1) / (num_hist - 1) to pos at each step, tracking
			 * the integral and fractional parts of the sum separately.
			 */
			delta = (nvals - 1) / (num_hist - 1);
			deltafrac = (nvals - 1) % (num_hist - 1);
			pos = posfrac = 0;

			for (i = 0; i < num_hist; i++)
			{
				hist_values[i] = datumCopy(timestamp_values[pos].value,
										   timestamp_typbyval,
										   timestamp_typlen);
				pos += delta;
				posfrac += deltafrac;
				if (posfrac >= (num_hist - 1))
				{
					/* fractional part exceeds 1, carry to integer part */
					pos++;
					posfrac -= (num_hist - 1);
				}
			}

			MemoryContextSwitchTo(old_context);

			stats->stakind[slot_idx] = STATISTIC_KIND_HISTOGRAM;
			stats->staop[slot_idx] = timestamp_ltopr;
			stats->stavalues[slot_idx] = hist_values;
			stats->numvalues[slot_idx] = num_hist;
			stats->statyplen[slot_idx] = (int16) timestamp_typlen;
			stats->statypid[slot_idx] = timestampType;
			stats->statypbyval[slot_idx] = timestamp_typbyval;
			slot_idx++;
		}

		/* Generate a correlation entry if there are multiple values */
		if (non_null_cnt > 1)
		{
			MemoryContext old_context;
			float4 *corrs;
			double corr_xsum,
					corr_x2sum;

			/* Must copy the target values into anl_context */
			old_context = MemoryContextSwitchTo(stats->anl_context);
			corrs = (float4 *) palloc(sizeof(float4));
			MemoryContextSwitchTo(old_context);

			/*----------
			 * Since we know the x and y value sets are both
			 *		0, 1, ..., values_cnt-1
			 * we have sum(x) = sum(y) =
			 *		(values_cnt-1)*values_cnt / 2
			 * and sum(x^2) = sum(y^2) =
			 *		(values_cnt-1)*values_cnt*(2*values_cnt-1) / 6.
			 *----------
			 */
			corr_xsum = ((double) (non_null_cnt - 1)) *
						((double) non_null_cnt) / 2.0;
			corr_x2sum = ((double) (non_null_cnt - 1)) *
						 ((double) non_null_cnt) * (double) (2 * non_null_cnt - 1) / 6.0;

			/* And the correlation coefficient reduces to */
			corrs[0] = (float4) ((non_null_cnt * corr_xysum - corr_xsum * corr_xsum) /
								 (non_null_cnt * corr_x2sum - corr_xsum * corr_xsum));


			stats->stakind[slot_idx] = STATISTIC_KIND_CORRELATION;
			stats->staop[slot_idx] = timestamp_ltopr;
			stats->stanumbers[slot_idx] = corrs;
			stats->numnumbers[slot_idx] = 1;
			stats->statyplen[slot_idx] = (int16) timestamp_typlen;
			stats->statypid[slot_idx] = timestampType;
			stats->statypbyval[slot_idx] = timestamp_typbyval;


			stats->stats_valid = true;
			stats->stadistinct = -(float4)timestamp_ndistinct/samplerows;
			stats->stanullfrac = (float4)null_cnt/samplerows;
			stats->stawidth = (int32)total_width;
		}

		MemoryContextSwitchTo(old_cxt);
	}
	else if (null_cnt > 0)
	{
		/* We found only nulls; assume the column is entirely null */
		stats->stats_valid = true;
		stats->stanullfrac = 1.0;
		stats->stawidth = 0;	/* "unknown" */
		stats->stadistinct = 0.0;		/* "unknown" */
	}

	/*
	 * We don't need to bother cleaning up any of our temporary palloc's. The
	 * hashtable should also go away, as it used a child memory context.
	 */
}
void
compute_temporalinst_twodim_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
								  int samplerows, double totalrows)
{
	int null_cnt = 0;
	int non_null_cnt = 0;
	int temporalinst_no;
	int slot_idx;
	int num_bins = stats->attr->attstattarget;
	int num_hist;

	double total_width = 0;

	Oid value_ltopr;
	Oid value_eqopr;
	Oid timestamp_ltopr;
	Oid timestamp_eqopr;

	ScalarItem *scalar_values, *timestamp_values;
	int *scalar_tupnoLink, *timestamp_tupnoLink;
	double corr_xysum;
	SortSupportData ssup;

	ScalarMCVItem *scalar_track, *timestamp_track;
	int scalar_track_cnt = 0,
			timestamp_track_cnt = 0;

	int num_mcv = stats->attr->attstattarget;

	scalar_values = (ScalarItem *) palloc(samplerows * sizeof(ScalarItem));
	scalar_tupnoLink = (int *) palloc(samplerows * sizeof(int));
	scalar_track = (ScalarMCVItem *) palloc(num_mcv * sizeof(ScalarMCVItem));

	timestamp_values = (ScalarItem *) palloc(samplerows * sizeof(ScalarItem));
	timestamp_tupnoLink = (int *) palloc(samplerows * sizeof(int));
	timestamp_track = (ScalarMCVItem *) palloc(num_mcv * sizeof(ScalarMCVItem));


	Oid valueType = base_oid_from_temporal(stats->attrtypid);
	bool typbyval = type_byval_fast(valueType);
	int typlen = get_typlen_fast(valueType);
	if (valueType == INT4OID)
		valueType = INT8OID;

	Oid timestampType = TIMESTAMPTZOID;
	bool timestamp_typbyval = true;
	int timestamp_typlen = sizeof(TimestampTz);

	get_sort_group_operators(valueType,
							 false, false, false,
							 &value_ltopr, &value_eqopr, NULL,
							 NULL);

	get_sort_group_operators(timestampType,
							 false, false, false,
							 &timestamp_ltopr, &timestamp_eqopr, NULL,
							 NULL);


	/* Loop over the sample TemporalInstants. */
	for (temporalinst_no = 0; temporalinst_no < samplerows; temporalinst_no++)
	{
		Datum value;
		bool isnull;
		TemporalInst *inst;

		vacuum_delay_point();

		value = fetchfunc(stats, temporalinst_no, &isnull);
		if (isnull)
		{
			/* TemporalInst is null, just count that */
			null_cnt++;
			continue;
		}

		total_width += VARSIZE_ANY(DatumGetPointer(value));

		/* Get period and deserialize it for further analysis. */
		inst = DatumGetTemporalInst(value);


		timestamp_values[non_null_cnt].value = datumCopy(inst->t,
														 timestamp_typbyval,
														 timestamp_typlen);
		timestamp_values[non_null_cnt].tupno = temporalinst_no;
		timestamp_tupnoLink[non_null_cnt] = temporalinst_no;


		if (typbyval)
		{
			scalar_values[non_null_cnt].value = datumCopy(temporalinst_value(inst),
														  typbyval,
														  typlen);
		}
		else
		{
			scalar_values[non_null_cnt].value = PointerGetDatum(temporalinst_value(inst));
		}
		scalar_values[non_null_cnt].tupno = temporalinst_no;
		scalar_tupnoLink[non_null_cnt] = temporalinst_no;

		non_null_cnt++;
	}
	slot_idx = 0;

	/* We can only compute real stats if we found some non-null values. */
	if (non_null_cnt > 0)
	{
		/* Compute the statistics for the base type */

		int i;
		int scalar_ndistinct,	/* # distinct values in sample */
				scalar_nmultiple,	/* # that appear multiple times */
				scalar_dups_cnt;
		CompareScalarsContext cxt;

		memset(&ssup, 0, sizeof(ssup));
		ssup.ssup_cxt = CurrentMemoryContext;
		/* We always use the default collation for statistics */
		ssup.ssup_collation = DEFAULT_COLLATION_OID;
		ssup.ssup_nulls_first = false;

		/*
		 * For now, don't perform abbreviated key conversion, because full values
		 * are required for MCV slot generation.  Supporting that optimization
		 * would necessitate teaching compare_scalars() to call a tie-breaker.
		 */
		ssup.abbreviate = false;

		PrepareSortSupportFromOrderingOp(value_ltopr, &ssup);

		/* Sort the collected values */
		cxt.ssup = &ssup;
		cxt.tupnoLink = scalar_tupnoLink;
		qsort_arg((void *) scalar_values, (size_t) non_null_cnt, sizeof(ScalarItem),
				  compare_scalars, (void *) &cxt);


		stats->stats_valid = true;
		/* Do the simple null-frac and width stats */
		stats->stanullfrac = (float4)((double) null_cnt / (double) samplerows);
		stats->stawidth = (int32) (total_width / (double) non_null_cnt);

		/* Estimate that non-null values are unique */
		stats->stadistinct = -1.0f * (1.0f - stats->stanullfrac);


		corr_xysum = 0;
		scalar_ndistinct = 0;
		scalar_nmultiple = 0;
		scalar_dups_cnt = 0;
		for (i = 0; i < non_null_cnt; i++)
		{
			int tupno = scalar_values[i].tupno;

			corr_xysum += ((double) i) * ((double) tupno);
			scalar_dups_cnt++;
			if (scalar_tupnoLink[tupno] == tupno)
			{
				/* Reached end of duplicates of this value */
				scalar_ndistinct++;
				if (scalar_dups_cnt > 1)
				{
					scalar_nmultiple++;
					if (scalar_track_cnt < num_mcv ||
						scalar_dups_cnt > scalar_track[scalar_track_cnt - 1].count)
					{
						/*
						 * Found a new item for the mcv list; find its
						 * position, bubbling down old items if needed. Loop
						 * invariant is that j points at an empty/ replaceable
						 * slot.
						 */
						int j;

						if (scalar_track_cnt < num_mcv)
							scalar_track_cnt++;
						for (j = scalar_track_cnt - 1; j > 0; j--)
						{
							if (scalar_dups_cnt <= scalar_track[j - 1].count)
								break;
							scalar_track[j].count = scalar_track[j - 1].count;
							scalar_track[j].first = scalar_track[j - 1].first;
						}
						scalar_track[j].count = scalar_dups_cnt;
						scalar_track[j].first = i + 1 - scalar_dups_cnt;
					}
				}
				scalar_dups_cnt = 0;
			}
		}


		/*
		 * Decide how many values are worth storing as most-common values. If
		 * we are able to generate a complete MCV list (all the values in the
		 * sample will fit, and we think these are all the ones in the table),
		 * then do so.  Otherwise, store only those values that are
		 * significantly more common than the (estimated) average. We set the
		 * threshold rather arbitrarily at 25% more than average, with at
		 * least 2 instances in the sample.  Also, we won't suppress values
		 * that have a frequency of at least 1/K where K is the intended
		 * number of histogram bins; such values might otherwise cause us to
		 * emit duplicate histogram bin boundaries.  (We might end up with
		 * duplicate histogram entries anyway, if the distribution is skewed;
		 * but we prefer to treat such values as MCVs if at all possible.)
		 *
		 * Note: the first of these cases is meant to address columns with
		 * small, fixed sets of possible values, such as boolean or enum
		 * columns.  If we can *completely* represent the column population by
		 * an MCV list that will fit into the stats target, then we should do
		 * so and thus provide the planner with complete information.  But if
		 * the MCV list is not complete, it's generally worth being more
		 * selective, and not just filling it all the way up to the stats
		 * target.  So for an incomplete list, we try to take only MCVs that
		 * are significantly more common than average.
		 */
		if (((scalar_track_cnt) == (scalar_ndistinct == 0)) &&
			(stats->stadistinct > 0) && (scalar_track_cnt <= num_mcv))
		{
			/* Track list includes all values seen, and all will fit */
			num_mcv = scalar_track_cnt;
		}
		else
		{
			double scalar_ndistinct_table = stats->stadistinct;
			double avgcount,
					mincount,
					maxmincount;

			/* Re-extract estimate of # distinct nonnull values in table */
			if (scalar_ndistinct_table < 0)
				scalar_ndistinct_table = -scalar_ndistinct_table * totalrows;
			/* estimate # occurrences in sample of a typical nonnull value */
			avgcount = (double) non_null_cnt / scalar_ndistinct_table;
			/* set minimum threshold count to store a value */
			mincount = avgcount * 1.25;
			if (mincount < 2)
				mincount = 2;
			/* don't let threshold exceed 1/K, however */
			maxmincount = (double) non_null_cnt / (double) num_bins;
			if (mincount > maxmincount)
				mincount = maxmincount;
			if (num_mcv > scalar_track_cnt)
				num_mcv = scalar_track_cnt;
			for (i = 0; i < num_mcv; i++)
			{
				if (scalar_track[i].count < mincount)
				{
					num_mcv = i;
					break;
				}
			}
		}


		/* Generate MCV slot entry */
		if (num_mcv > 0)
		{
			MemoryContext old_context;
			Datum *mcv_values;
			float4 *mcv_freqs;

			/* Must copy the target values into anl_context */
			old_context = MemoryContextSwitchTo(stats->anl_context);
			mcv_values = (Datum *) palloc(num_mcv * sizeof(Datum));
			mcv_freqs = (float4 *) palloc(num_mcv * sizeof(float4));


			for (i = 0; i < num_mcv; i++)
			{
				mcv_values[i] = datumCopy(scalar_values[scalar_track[i].first].value,
										  typbyval,
										  typlen);

				mcv_freqs[i] = (float4) scalar_track[i].count / (float4) samplerows;
			}
			MemoryContextSwitchTo(old_context);
			stats->stakind[slot_idx] = STATISTIC_KIND_MCV;
			stats->staop[slot_idx] = value_eqopr;
			stats->stanumbers[slot_idx] = mcv_freqs;
			stats->numnumbers[slot_idx] = num_mcv;
			stats->stavalues[slot_idx] = mcv_values;
			stats->numvalues[slot_idx] = num_mcv;
			stats->statyplen[slot_idx] = (int16) typlen;
			stats->statypid[slot_idx] = valueType;
			stats->statypbyval[slot_idx] = typbyval;
		}
		slot_idx++;

		/*
		 * Generate a histogram slot entry if there are at least two distinct
		 * values not accounted for in the MCV list.  (This ensures the
		 * histogram won't collapse to empty or a singleton.)
		 */
		num_hist = scalar_ndistinct - num_mcv;
		if (num_hist > num_bins)
			num_hist = num_bins + 1;
		if (num_hist >= 2)
		{
			MemoryContext old_context;
			Datum *hist_values;
			int nvals;
			int pos,
					posfrac,
					delta,
					deltafrac;

			/* Sort the MCV items into position order to speed next loop */
			qsort((void *) scalar_track, (size_t) num_mcv,
				  sizeof(ScalarMCVItem), compare_mcvs);

			/*
			 * Collapse out the MCV items from the values[] array.
			 *
			 * Note we destroy the values[] array here... but we don't need it
			 * for anything more.  We do, however, still need values_cnt.
			 * nvals will be the number of remaining entries in values[].
			 */
			if (num_mcv > 0)
			{
				int src,
						dest;
				int j;

				src = dest = 0;
				j = 0;			/* index of next interesting MCV item */
				while (src < non_null_cnt)
				{
					int ncopy;

					if (j < num_mcv)
					{
						int first = scalar_track[j].first;

						if (src >= first)
						{
							/* advance past this MCV item */
							src = first + scalar_track[j].count;
							j++;
							continue;
						}
						ncopy = first - src;
					}
					else
						ncopy = non_null_cnt - src;
					memmove(&scalar_values[dest], &scalar_values[src],
							ncopy * sizeof(ScalarItem));
					src += ncopy;
					dest += ncopy;
				}
				nvals = dest;
			}
			else
				nvals = non_null_cnt;
			Assert(nvals >= num_hist);

			/* Must copy the target values into anl_context */
			old_context = MemoryContextSwitchTo(stats->anl_context);
			hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

			/*
			 * The object of this loop is to copy the first and last values[]
			 * entries along with evenly-spaced values in between.  So the
			 * i'th value is values[(i * (nvals - 1)) / (num_hist - 1)].  But
			 * computing that subscript directly risks integer overflow when
			 * the stats target is more than a couple thousand.  Instead we
			 * add (nvals - 1) / (num_hist - 1) to pos at each step, tracking
			 * the integral and fractional parts of the sum separately.
			 */
			delta = (nvals - 1) / (num_hist - 1);
			deltafrac = (nvals - 1) % (num_hist - 1);
			pos = posfrac = 0;

			for (i = 0; i < num_hist; i++)
			{
				hist_values[i] = datumCopy(scalar_values[pos].value,
										   typbyval,
										   typlen);
				pos += delta;
				posfrac += deltafrac;
				if (posfrac >= (num_hist - 1))
				{
					/* fractional part exceeds 1, carry to integer part */
					pos++;
					posfrac -= (num_hist - 1);
				}
			}

			MemoryContextSwitchTo(old_context);

			stats->stakind[slot_idx] = STATISTIC_KIND_HISTOGRAM;
			stats->staop[slot_idx] = value_ltopr;
			stats->stavalues[slot_idx] = hist_values;
			stats->numvalues[slot_idx] = num_hist;
			stats->statyplen[slot_idx] = (int16) typlen;
			stats->statypid[slot_idx] = valueType;
			stats->statypbyval[slot_idx] = typbyval;
			stats->stadistinct = -1.0f * (float4)scalar_ndistinct/non_null_cnt;
			stats->stanullfrac = (float4)null_cnt/non_null_cnt;
			slot_idx++;
		}


		/* Compute the statistics for the temporal type */

		MemoryContext old_cxt;

		int timestamp_ndistinct,	/* # distinct values in sample */
				timestamp_nmultiple,	/* # that appear multiple times */
				timestamp_dups_cnt;

		memset(&ssup, 0, sizeof(ssup));
		ssup.ssup_cxt = CurrentMemoryContext;
		/* We always use the default collation for statistics */
		ssup.ssup_collation = DEFAULT_COLLATION_OID;
		ssup.ssup_nulls_first = false;

		/*
		 * For now, don't perform abbreviated key conversion, because full values
		 * are required for MCV slot generation.  Supporting that optimization
		 * would necessitate teaching compare_scalars() to call a tie-breaker.
		 */
		ssup.abbreviate = false;

		PrepareSortSupportFromOrderingOp(timestamp_ltopr, &ssup);

		/* Sort the collected values */
		cxt.ssup = &ssup;
		cxt.tupnoLink = timestamp_tupnoLink;
		qsort_arg((void *) timestamp_values, (size_t) non_null_cnt, sizeof(ScalarItem),
				  compare_scalars, (void *) &cxt);

		/* Must copy the target values into anl_context */
		old_cxt = MemoryContextSwitchTo(stats->anl_context);


		corr_xysum = 0;
		timestamp_ndistinct = 0;
		timestamp_nmultiple = 0;
		timestamp_dups_cnt = 0;
		for (i = 0; i < non_null_cnt; i++)
		{
			int tupno = timestamp_values[i].tupno;

			corr_xysum += ((double) i) * ((double) tupno);
			timestamp_dups_cnt++;
			if (timestamp_tupnoLink[tupno] == tupno)
			{
				/* Reached end of duplicates of this value */
				timestamp_ndistinct++;
				if (timestamp_dups_cnt > 1)
				{
					timestamp_nmultiple++;
					if (timestamp_track_cnt < num_mcv ||
						timestamp_dups_cnt > timestamp_track[timestamp_track_cnt - 1].count)
					{
						/*
						 * Found a new item for the mcv list; find its
						 * position, bubbling down old items if needed. Loop
						 * invariant is that j points at an empty/ replaceable
						 * slot.
						 */
						int j;

						if (timestamp_track_cnt < num_mcv)
							timestamp_track_cnt++;
						for (j = timestamp_track_cnt - 1; j > 0; j--)
						{
							if (timestamp_dups_cnt <= timestamp_track[j - 1].count)
								break;
							timestamp_track[j].count = timestamp_track[j - 1].count;
							timestamp_track[j].first = timestamp_track[j - 1].first;
						}
						timestamp_track[j].count = timestamp_dups_cnt;
						timestamp_track[j].first = i + 1 - timestamp_dups_cnt;
					}
				}
				timestamp_dups_cnt = 0;
			}
		}


		/*
		 * Decide how many values are worth storing as most-common values. If
		 * we are able to generate a complete MCV list (all the values in the
		 * sample will fit, and we think these are all the ones in the table),
		 * then do so.  Otherwise, store only those values that are
		 * significantly more common than the (estimated) average. We set the
		 * threshold rather arbitrarily at 25% more than average, with at
		 * least 2 instances in the sample.  Also, we won't suppress values
		 * that have a frequency of at least 1/K where K is the intended
		 * number of histogram bins; such values might otherwise cause us to
		 * emit duplicate histogram bin boundaries.  (We might end up with
		 * duplicate histogram entries anyway, if the distribution is skewed;
		 * but we prefer to treat such values as MCVs if at all possible.)
		 *
		 * Note: the first of these cases is meant to address columns with
		 * small, fixed sets of possible values, such as boolean or enum
		 * columns.  If we can *completely* represent the column population by
		 * an MCV list that will fit into the stats target, then we should do
		 * so and thus provide the planner with complete information.  But if
		 * the MCV list is not complete, it's generally worth being more
		 * selective, and not just filling it all the way up to the stats
		 * target.  So for an incomplete list, we try to take only MCVs that
		 * are significantly more common than average.
		 */
		if (((timestamp_track_cnt) == (timestamp_ndistinct == 0)) &&
			(stats->stadistinct > 0) &&
			(timestamp_track_cnt <= num_mcv))
		{
			/* Track list includes all values seen, and all will fit */
			num_mcv = timestamp_track_cnt;
		}
		else
		{
			double timestamp_ndistinct_table = stats->stadistinct;
			double avgcount,
					mincount,
					maxmincount;

			/* Re-extract estimate of # distinct nonnull values in table */
			if (timestamp_ndistinct_table < 0)
				timestamp_ndistinct_table = -timestamp_ndistinct_table * totalrows;
			/* estimate # occurrences in sample of a typical nonnull value */
			avgcount = (double) non_null_cnt / timestamp_ndistinct_table;
			/* set minimum threshold count to store a value */
			mincount = avgcount * 1.25;
			if (mincount < 2)
				mincount = 2;
			/* don't let threshold exceed 1/K, however */
			maxmincount = (double) non_null_cnt / (double) num_bins;
			if (mincount > maxmincount)
				mincount = maxmincount;
			if (num_mcv > timestamp_track_cnt)
				num_mcv = timestamp_track_cnt;
			for (i = 0; i < num_mcv; i++)
			{
				if (timestamp_track[i].count < mincount)
				{
					num_mcv = i;
					break;
				}
			}
		}


		/* Generate MCV slot entry */
		if (num_mcv > 0)
		{
			MemoryContext old_context;
			Datum *mcv_values;
			float4 *mcv_freqs;

			/* Must copy the target values into anl_context */
			old_context = MemoryContextSwitchTo(stats->anl_context);
			mcv_values = (Datum *) palloc(num_mcv * sizeof(Datum));
			mcv_freqs = (float4 *) palloc(num_mcv * sizeof(float4));

			for (i = 0; i < num_mcv; i++)
			{
				mcv_values[i] = datumCopy(timestamp_values[timestamp_track[i].first].value,
										  timestamp_typbyval,
										  timestamp_typlen);
				mcv_freqs[i] = (float4) timestamp_track[i].count / (float4) samplerows;
			}
			MemoryContextSwitchTo(old_context);
			stats->stakind[slot_idx] = STATISTIC_KIND_MCV;
			stats->staop[slot_idx] = timestamp_eqopr;
			stats->stanumbers[slot_idx] = mcv_freqs;
			stats->numnumbers[slot_idx] = num_mcv;
			stats->stavalues[slot_idx] = mcv_values;
			stats->numvalues[slot_idx] = num_mcv;
			stats->statyplen[slot_idx] = (int16) timestamp_typlen;
			stats->statypid[slot_idx] = timestampType;
			stats->statypbyval[slot_idx] = timestamp_typbyval;
			slot_idx++;
		}

		/*
		 * Generate a histogram slot entry if there are at least two distinct
		 * values not accounted for in the MCV list.  (This ensures the
		 * histogram won't collapse to empty or a singleton.)
		 */
		num_hist = timestamp_ndistinct - num_mcv;
		if (num_hist > num_bins)
			num_hist = num_bins + 1;
		if (num_hist >= 2)
		{
			MemoryContext old_context;
			Datum *hist_values;
			int nvals;
			int pos,
					posfrac,
					delta,
					deltafrac;

			/* Sort the MCV items into position order to speed next loop */
			qsort((void *) timestamp_track, (size_t) num_mcv,
				  sizeof(ScalarMCVItem), compare_mcvs);

			/*
			 * Collapse out the MCV items from the values[] array.
			 *
			 * Note we destroy the values[] array here... but we don't need it
			 * for anything more.  We do, however, still need values_cnt.
			 * nvals will be the number of remaining entries in values[].
			 */
			if (num_mcv > 0)
			{
				int src,
						dest;
				int j;

				src = dest = 0;
				j = 0;			/* index of next interesting MCV item */
				while (src < non_null_cnt)
				{
					int ncopy;

					if (j < num_mcv)
					{
						int first = timestamp_track[j].first;

						if (src >= first)
						{
							/* advance past this MCV item */
							src = first + timestamp_track[j].count;
							j++;
							continue;
						}
						ncopy = first - src;
					}
					else
						ncopy = non_null_cnt - src;
					memmove(&timestamp_values[dest], &timestamp_values[src],
							ncopy * sizeof(ScalarItem));
					src += ncopy;
					dest += ncopy;
				}
				nvals = dest;
			}
			else
				nvals = non_null_cnt;
			Assert(nvals >= num_hist);

			/* Must copy the target values into anl_context */
			old_context = MemoryContextSwitchTo(stats->anl_context);
			hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

			/*
			 * The object of this loop is to copy the first and last values[]
			 * entries along with evenly-spaced values in between.  So the
			 * i'th value is values[(i * (nvals - 1)) / (num_hist - 1)].  But
			 * computing that subscript directly risks integer overflow when
			 * the stats target is more than a couple thousand.  Instead we
			 * add (nvals - 1) / (num_hist - 1) to pos at each step, tracking
			 * the integral and fractional parts of the sum separately.
			 */
			delta = (nvals - 1) / (num_hist - 1);
			deltafrac = (nvals - 1) % (num_hist - 1);
			pos = posfrac = 0;

			for (i = 0; i < num_hist; i++)
			{
				hist_values[i] = datumCopy(timestamp_values[pos].value,
										   timestamp_typbyval,
										   timestamp_typlen);
				pos += delta;
				posfrac += deltafrac;
				if (posfrac >= (num_hist - 1))
				{
					/* fractional part exceeds 1, carry to integer part */
					pos++;
					posfrac -= (num_hist - 1);
				}
			}

			MemoryContextSwitchTo(old_context);

			stats->stakind[slot_idx] = STATISTIC_KIND_HISTOGRAM;
			stats->staop[slot_idx] = timestamp_ltopr;
			stats->stavalues[slot_idx] = hist_values;
			stats->numvalues[slot_idx] = num_hist;
			stats->statyplen[slot_idx] = (int16) timestamp_typlen;
			stats->statypid[slot_idx] = timestampType;
			stats->statypbyval[slot_idx] = timestamp_typbyval;
			slot_idx++;
		}

		/* Generate a correlation entry if there are multiple values */
		if (non_null_cnt > 1 && slot_idx < 5)
		{
			MemoryContext old_context;
			float4 *corrs;
			double corr_xsum,
					corr_x2sum;

			/* Must copy the target values into anl_context */
			old_context = MemoryContextSwitchTo(stats->anl_context);
			corrs = (float4 *) palloc(sizeof(float4));
			MemoryContextSwitchTo(old_context);

			/*----------
			 * Since we know the x and y value sets are both
			 *		0, 1, ..., values_cnt-1
			 * we have sum(x) = sum(y) =
			 *		(values_cnt-1)*values_cnt / 2
			 * and sum(x^2) = sum(y^2) =
			 *		(values_cnt-1)*values_cnt*(2*values_cnt-1) / 6.
			 *----------
			 */
			corr_xsum = ((double) (non_null_cnt - 1)) *
						((double) non_null_cnt) / 2.0;
			corr_x2sum = ((double) (non_null_cnt - 1)) *
						 ((double) non_null_cnt) * (double) (2 * non_null_cnt - 1) / 6.0;

			/* And the correlation coefficient reduces to */
			corrs[0] = (float4) ((non_null_cnt * corr_xysum - corr_xsum * corr_xsum) /
								 (non_null_cnt * corr_x2sum - corr_xsum * corr_xsum));

			stats->stakind[slot_idx] = STATISTIC_KIND_CORRELATION;
			stats->staop[slot_idx] = timestamp_ltopr;
			stats->stanumbers[slot_idx] = corrs;
			stats->numnumbers[slot_idx] = 1;
			stats->statyplen[slot_idx] = (int16) timestamp_typlen;
			stats->statypid[slot_idx] = timestampType;
			stats->statypbyval[slot_idx] = timestamp_typbyval;
		}


		MemoryContextSwitchTo(old_cxt);
	}
	else if (null_cnt > 0)
	{
		/* We found only nulls; assume the column is entirely null */
		stats->stats_valid = true;
		stats->stanullfrac = 1.0;
		stats->stawidth = 0;	/* "unknown" */
		stats->stadistinct = 0.0;		/* "unknown" */
	}
}
/*****************************************************************************
 * Statistics functions for TemporalI type
 *****************************************************************************/
void
compute_timestamptz_set_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
							  int samplerows, double totalrows)
{
	int			num_mcelem;
	int			null_cnt = 0;
	int			null_elem_cnt = 0;
	int			analyzed_rows = 0;
	/* This is D from the LC algorithm. */
	HTAB	   *elements_tab;
	HASHCTL		elem_hash_ctl;
	HASH_SEQ_STATUS scan_status;

	/* This is the current bucket number from the LC algorithm */
	int			b_current;

	/* This is 'w' from the LC algorithm */
	int			bucket_width;
	int			array_no;
	int64		element_no;
	TrackItem  *item;
	int			slot_idx;
	HTAB	   *count_tab;
	HASHCTL		count_hash_ctl;
	DECountItem *count_item;

	array_extra_data = (TemporalArrayAnalyzeExtraData *) stats->extra_data;

	Oid valueType = base_oid_from_temporal(stats->attrtypid);
	/*
	 * We want statistics_target * 10 elements in the MCELEM array. This
	 * multiplier is pretty arbitrary, but is meant to reflect the fact that
	 * the number of individual elements tracked in pg_statistic ought to be
	 * more than the number of values for a simple scalar column.
	 */
	num_mcelem = stats->attr->attstattarget * 10;

	/*
	 * We set bucket width equal to num_mcelem / 0.007 as per the comment
	 * above.
	 */
	bucket_width = num_mcelem * 1000 / 7;

	/*
	 * Create the hashtable. It will be in local memory, so we don't need to
	 * worry about overflowing the initial size. Also we don't need to pay any
	 * attention to locking and memory management.
	 */
	MemSet(&elem_hash_ctl, 0, sizeof(elem_hash_ctl));
	elem_hash_ctl.keysize = sizeof(Datum);
	elem_hash_ctl.entrysize = sizeof(TrackItem);
	elem_hash_ctl.hash = element_hash_temporal;
	elem_hash_ctl.match = element_match;
	elem_hash_ctl.hcxt = CurrentMemoryContext;
	elements_tab = hash_create("Analyzed elements table",
							   num_mcelem,
							   &elem_hash_ctl,
							   HASH_ELEM | HASH_FUNCTION | HASH_COMPARE | HASH_CONTEXT);

	/* hashtable for array distinct elements counts */
	MemSet(&count_hash_ctl, 0, sizeof(count_hash_ctl));
	count_hash_ctl.keysize = sizeof(int);
	count_hash_ctl.entrysize = sizeof(DECountItem);
	count_hash_ctl.hcxt = CurrentMemoryContext;
	count_tab = hash_create("Array distinct element count table",
							64,
							&count_hash_ctl,
							HASH_ELEM | HASH_BLOBS | HASH_CONTEXT);

	/* Initialize counters. */
	b_current = 1;
	element_no = 0;

	/* Loop over the arrays. */
	for (array_no = 0; array_no < samplerows; array_no++)
	{
		Datum		value;
		bool		isnull;
		TemporalI  *array;
		bool		null_present;
		int			j;
		int64		prev_element_no = element_no;
		int			distinct_count;
		bool		count_item_found;

		vacuum_delay_point();

		value = fetchfunc(stats, array_no, &isnull);
		if (isnull)
		{
			/* array is null, just count that */
			null_cnt++;
			continue;
		}

		/*
		 * Now detoast the array if needed, and deconstruct into datums.
		 */
		array = DatumGetTemporalI(value);
		/*
		 * We loop through the elements in the array and add them to our
		 * tracking hashtable.
		 */
		null_present = false;
		if(array->count < 1)
			null_present = true;
		for (j = 0; j < array->count; j++)
		{
			TemporalInst *inst = temporali_inst_n(array, j);
			Datum elem_value;
			bool found;

			/* No null element processing other than flag setting here */
			/* if (elem_nulls[j]) {
				 null_present = true;
				 continue;
			 }*/

			/* Lookup current element in hashtable, adding it if new */
			elem_value = TimestampTzGetDatum(inst->t);
			item = (TrackItem *) hash_search(elements_tab,
											 (const void *) &elem_value,
											 HASH_ENTER, &found);

			if (found) {
				/* The element value is already on the tracking list */

				/*
				 * The operators we assist ignore duplicate array elements, so
				 * count a given distinct element only once per array.
				 */
				if (item->last_container == array_no)
					continue;

				item->frequency++;
				item->last_container = array_no;
			} else {
				/* Initialize new tracking list element */

				/*
				 * If element type is pass-by-reference, we must copy it into
				 * palloc'd space, so that we can release the array below. (We
				 * do this so that the space needed for element values is
				 * limited by the size of the hashtable; if we kept all the
				 * array values around, it could be much more.)
				 */
				item->key = elem_value;

				item->frequency = 1;
				item->delta = b_current - 1;
				item->last_container = array_no;
			}

			/* element_no is the number of elements processed (ie N) */
			element_no++;
		}

		/* Count null element presence once per array. */
		if (null_present)
			null_elem_cnt++;

		/* Update frequency of the particular array distinct element count. */
		distinct_count = (int) (element_no - prev_element_no);
		count_item = (DECountItem *) hash_search(count_tab, &distinct_count,
												 HASH_ENTER,
												 &count_item_found);

		if (count_item_found)
			count_item->frequency++;
		else
			count_item->frequency = 1;

		/* Removing the temporal part from the stats HeapTuples if the base type is geometry */
		if(valueType == type_oid(T_GEOMETRY) || valueType == type_oid(T_GEOGRAPHY))
			stats->rows[analyzed_rows] = remove_temporaldim(stats->rows[analyzed_rows], stats->tupDesc, stats->tupattnum, stats->attrtypid, true, value);

		analyzed_rows++;
	}

	/* Skip pg_statistic slots occupied by standard statistics */
	slot_idx = 2;

	/* We can only compute real stats if we found some non-null values. */
	if (analyzed_rows > 0)
	{
		int			nonnull_cnt = analyzed_rows;
		int			count_items_count;
		int			i;
		TrackItem **sort_table;
		int			track_len;
		int64		cutoff_freq;
		int64		minfreq,
				maxfreq;

		/*
		 * We assume the standard stats code already took care of setting
		 * stats_valid, stanullfrac, stawidth, stadistinct.  We'd have to
		 * re-compute those values if we wanted to not store the standard
		 * stats.
		 */

		/*
		 * Construct an array of the interesting hashtable items, that is,
		 * those meeting the cutoff frequency (s - epsilon)*N.  Also identify
		 * the minimum and maximum frequencies among these items.
		 *
		 * Since epsilon = s/10 and bucket_width = 1/epsilon, the cutoff
		 * frequency is 9*N / bucket_width.
		 */
		cutoff_freq = 9 * element_no / bucket_width;

		i = (int)hash_get_num_entries(elements_tab); /* surely enough space */
		sort_table = (TrackItem **) palloc(sizeof(TrackItem *) * i);

		hash_seq_init(&scan_status, elements_tab);
		track_len = 0;
		minfreq = element_no;
		maxfreq = 0;
		while ((item = (TrackItem *) hash_seq_search(&scan_status)) != NULL)
		{
			if (item->frequency > cutoff_freq)
			{
				sort_table[track_len++] = item;
				minfreq = Min(minfreq, item->frequency);
				maxfreq = Max(maxfreq, item->frequency);
			}
		}
		Assert(track_len <= i);

		/* emit some statistics for debug purposes */
		elog(DEBUG3, "compute_array_stats: target # mces = %d, "
					 "bucket width = %d, "
					 "# elements = " INT64_FORMAT ", hashtable size = %d, "
												  "usable entries = %d",
			 num_mcelem, bucket_width, element_no, i, track_len);

		/*
		 * If we obtained more elements than we really want, get rid of those
		 * with least frequencies.  The easiest way is to qsort the array into
		 * descending frequency order and truncate the array.
		 */
		if (num_mcelem < track_len)
		{
			qsort(sort_table, track_len, sizeof(TrackItem *),
				  trackitem_compare_frequencies_desc);
			/* reset minfreq to the smallest frequency we're keeping */
			minfreq = sort_table[num_mcelem - 1]->frequency;
		}
		else
			num_mcelem = track_len;

		/* Generate MCELEM slot entry */
		if (num_mcelem > 0)
		{
			MemoryContext old_context;
			Datum	   *mcelem_values;
			float4	   *mcelem_freqs;

			/*
			 * We want to store statistics sorted on the element value using
			 * the element type's default comparison function.  This permits
			 * fast binary searches in selectivity estimation functions.
			 */
			qsort(sort_table, num_mcelem, sizeof(TrackItem *),
				  trackitem_compare_element);

			/* Must copy the target values into anl_context */
			old_context = MemoryContextSwitchTo(stats->anl_context);

			/*
			 * We sorted statistics on the element value, but we want to be
			 * able to find the minimal and maximal frequencies without going
			 * through all the values.  We also want the frequency of null
			 * elements.  Store these three values at the end of mcelem_freqs.
			 */
			mcelem_values = (Datum *) palloc(num_mcelem * sizeof(Datum));
			mcelem_freqs = (float4 *) palloc((num_mcelem + 3) * sizeof(float4));

			/*
			 * See comments above about use of nonnull_cnt as the divisor for
			 * the final frequency estimates.
			 */
			for (i = 0; i < num_mcelem; i++)
			{
				TrackItem  *item = sort_table[i];

				mcelem_values[i] = item->key;
				mcelem_freqs[i] = (float4) item->frequency /
								  (float4) nonnull_cnt;
			}
			mcelem_freqs[i++] = (float4) minfreq / (float4) nonnull_cnt;
			mcelem_freqs[i++] = (float4) maxfreq / (float4) nonnull_cnt;
			mcelem_freqs[i++] = (float4) null_elem_cnt / (float4) nonnull_cnt;

			MemoryContextSwitchTo(old_context);

			stats->stakind[slot_idx] = STATISTIC_KIND_MCELEM;
			stats->staop[slot_idx] = array_extra_data->temporal_eq_opr;
			stats->stanumbers[slot_idx] = mcelem_freqs;
			/* See above comment about extra stanumber entries */
			stats->numnumbers[slot_idx] = num_mcelem + 3;
			stats->stavalues[slot_idx] = mcelem_values;
			stats->numvalues[slot_idx] = num_mcelem;
			/* We are storing values of element type */
			stats->statypid[slot_idx] = array_extra_data->temporal_type_id;
			stats->statyplen[slot_idx] = array_extra_data->temporal_typlen;
			stats->statypbyval[slot_idx] = array_extra_data->temporal_typbyval;
			stats->statypalign[slot_idx] = array_extra_data->temporal_typalign;
			stats->stats_valid = true;
			slot_idx++;
		}

		/* Generate DECHIST slot entry */
		count_items_count = (int)hash_get_num_entries(count_tab);
		if (count_items_count > 0)
		{
			int			num_hist = stats->attr->attstattarget;
			DECountItem **sorted_count_items;
			int			j;
			int			delta;
			int64		frac;
			float4	   *hist;

			/* num_hist must be at least 2 for the loop below to work */
			num_hist = Max(num_hist, 2);

			/*
			 * Create an array of DECountItem pointers, and sort them into
			 * increasing count order.
			 */
			sorted_count_items = (DECountItem **)
					palloc(sizeof(DECountItem *) * count_items_count);
			hash_seq_init(&scan_status, count_tab);
			j = 0;
			while ((count_item = (DECountItem *) hash_seq_search(&scan_status)) != NULL)
			{
				sorted_count_items[j++] = count_item;
			}
			qsort(sorted_count_items, count_items_count,
				  sizeof(DECountItem *), countitem_compare_count);

			/*
			 * Prepare to fill stanumbers with the histogram, followed by the
			 * average count.  This array must be stored in anl_context.
			 */
			hist = (float4 *)
					MemoryContextAlloc(stats->anl_context,
									   sizeof(float4) * (num_hist + 1));
			hist[num_hist] = (float4) element_no / (float4) nonnull_cnt;

			/*----------
			 * Construct the histogram of distinct-element counts (DECs).
			 *
			 * The object of this loop is to copy the min and max DECs to
			 * hist[0] and hist[num_hist - 1], along with evenly-spaced DECs
			 * in between (where "evenly-spaced" is with reference to the
			 * whole input population of arrays).  If we had a complete sorted
			 * array of DECs, one per analyzed row, the i'th hist value would
			 * come from DECs[i * (analyzed_rows - 1) / (num_hist - 1)]
			 * (compare the histogram-making loop in compute_scalar_stats()).
			 * But instead of that we have the sorted_count_items[] array,
			 * which holds unique DEC values with their frequencies (that is,
			 * a run-length-compressed version of the full array).  So we
			 * control advancing through sorted_count_items[] with the
			 * variable "frac", which is defined as (x - y) * (num_hist - 1),
			 * where x is the index in the notional DECs array corresponding
			 * to the start of the next sorted_count_items[] element's run,
			 * and y is the index in DECs from which we should take the next
			 * histogram value.  We have to advance whenever x <= y, that is
			 * frac <= 0.  The x component is the sum of the frequencies seen
			 * so far (up through the current sorted_count_items[] element),
			 * and of course y * (num_hist - 1) = i * (analyzed_rows - 1),
			 * per the subscript calculation above.  (The subscript calculation
			 * implies dropping any fractional part of y; in this formulation
			 * that's handled by not advancing until frac reaches 1.)
			 *
			 * Even though frac has a bounded range, it could overflow int32
			 * when working with very large statistics targets, so we do that
			 * math in int64.
			 *----------
			 */
			delta = analyzed_rows - 1;
			j = 0;				/* current index in sorted_count_items */
			/* Initialize frac for sorted_count_items[0]; y is initially 0 */
			frac = (int64) sorted_count_items[0]->frequency * (num_hist - 1);
			for (i = 0; i < num_hist; i++)
			{
				while (frac <= 0)
				{
					/* Advance, and update x component of frac */
					j++;
					frac += (int64) sorted_count_items[j]->frequency * (num_hist - 1);
				}
				hist[i] = sorted_count_items[j]->count;
				frac -= delta;	/* update y for upcoming i increment */
			}
			Assert(j == count_items_count - 1);

			stats->stakind[slot_idx] = STATISTIC_KIND_DECHIST;
			stats->staop[slot_idx] = array_extra_data->temporal_eq_opr;
			stats->stanumbers[slot_idx] = hist;
			stats->numnumbers[slot_idx] = num_hist + 1;
		}
	}

	/*
	 * We don't need to bother cleaning up any of our temporary palloc's. The
	 * hashtable should also go away, as it used a child memory context.
	 */
}
void
compute_temporali_twodim_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
							   int samplerows, double totalrows)
{
	int			num_mcelem;
	int			null_cnt = 0;
	int			null_elem_cnt = 0;
	int			analyzed_rows = 0;
	double	  total_width = 0;


	/* This is D from the LC algorithm. */
	HTAB	   *elements_tab_value, *elements_tab_temporal;
	HASHCTL		elem_hash_ctl_value, elem_hash_ctl_temporal;
	HASH_SEQ_STATUS scan_status_value, scan_status_temporal;

	/* This is the current bucket number from the LC algorithm */
	int			b_current_value, b_current_temporal;

	/* This is 'w' from the LC algorithm */
	int			bucket_width;
	int			array_no;
	int64		element_no_value, element_no_temporal;
	TrackItem  *item_value, *item_temporal;
	int			slot_idx;
	HTAB	   *count_tab_value, *count_tab_temporal;
	HASHCTL		count_hash_ctl_value, count_hash_ctl_temporal;
	DECountItem *count_item_value, *count_item_temporal;

	array_extra_data = (TemporalArrayAnalyzeExtraData *) stats->extra_data;
	/*
	 * We want statistics_target * 10 elements in the MCELEM array. This
	 * multiplier is pretty arbitrary, but is meant to reflect the fact that
	 * the number of individual elements tracked in pg_statistic ought to be
	 * more than the number of values for a simple scalar column.
	 */
	num_mcelem = stats->attr->attstattarget * 10;

	/*
	 * We set bucket width equal to num_mcelem / 0.007 as per the comment
	 * above.
	 */
	bucket_width = num_mcelem * 1000 / 7;

	/*
	 * Create the hashtable. It will be in local memory, so we don't need to
	 * worry about overflowing the initial size. Also we don't need to pay any
	 * attention to locking and memory management.
	 */
	MemSet(&elem_hash_ctl_value, 0, sizeof(elem_hash_ctl_value));
	elem_hash_ctl_value.keysize = sizeof(Datum);
	elem_hash_ctl_value.entrysize = sizeof(TrackItem);
	elem_hash_ctl_value.hash = element_hash_value;
	elem_hash_ctl_value.match = element_match;
	elem_hash_ctl_value.hcxt = CurrentMemoryContext;
	elements_tab_value = hash_create("Analyzed elements table",
									 num_mcelem,
									 &elem_hash_ctl_value,
									 HASH_ELEM | HASH_FUNCTION | HASH_COMPARE | HASH_CONTEXT);

	/* hashtable for array distinct elements counts */
	MemSet(&count_hash_ctl_value, 0, sizeof(count_hash_ctl_value));
	count_hash_ctl_value.keysize = sizeof(int);
	count_hash_ctl_value.entrysize = sizeof(DECountItem);
	count_hash_ctl_value.hcxt = CurrentMemoryContext;
	count_tab_value = hash_create("Array distinct element count table",
								  64,
								  &count_hash_ctl_value,
								  HASH_ELEM | HASH_BLOBS | HASH_CONTEXT);

	MemSet(&elem_hash_ctl_temporal, 0, sizeof(elem_hash_ctl_temporal));
	elem_hash_ctl_temporal.keysize = sizeof(Datum);
	elem_hash_ctl_temporal.entrysize = sizeof(TrackItem);
	elem_hash_ctl_temporal.hash = element_hash_temporal;
	elem_hash_ctl_temporal.match = element_match;
	elem_hash_ctl_temporal.hcxt = CurrentMemoryContext;
	elements_tab_temporal = hash_create("Analyzed elements table",
										num_mcelem,
										&elem_hash_ctl_temporal,
										HASH_ELEM | HASH_FUNCTION | HASH_COMPARE | HASH_CONTEXT);

	/* hashtable for array distinct elements counts */
	MemSet(&count_hash_ctl_temporal, 0, sizeof(count_hash_ctl_temporal));
	count_hash_ctl_temporal.keysize = sizeof(int);
	count_hash_ctl_temporal.entrysize = sizeof(DECountItem);
	count_hash_ctl_temporal.hcxt = CurrentMemoryContext;
	count_tab_temporal = hash_create("Array distinct element count table",
									 64,
									 &count_hash_ctl_temporal,
									 HASH_ELEM | HASH_BLOBS | HASH_CONTEXT);



	/* Initialize counters. */
	b_current_value = 1;
	b_current_temporal = 1;
	element_no_value = 0;
	element_no_temporal = 0;

	/* Loop over the arrays. */
	for (array_no = 0; array_no < samplerows; array_no++)
	{
		Datum		value;
		bool		isnull;
		TemporalI  *array;
		bool		null_present;
		int			j;
		int64		prev_element_no_value = element_no_value,
				prev_element_no_temporal = element_no_temporal;
		int			distinct_count_value, distinct_count_temporal;
		bool		count_item_found_value, count_item_found_temporal;

		vacuum_delay_point();

		value = fetchfunc(stats, array_no, &isnull);
		if (isnull)
		{
			/* array is null, just count that */
			null_cnt++;
			continue;
		}

		/*
		 * Now detoast the array if needed, and deconstruct into datums.
		 */
		array = DatumGetTemporalI(value);
		/*
		 * We loop through the elements in the array and add them to our
		 * tracking hashtable.
		 */
		null_present = false;

		if(array->count < 1)
			null_present = true;
		for (j = 0; j < array->count; j++)
		{
			TemporalInst *inst = temporali_inst_n(array, j);
			Datum elem_value, elem_temporal;
			bool found_value, found_temporal;


			/* Lookup current element in hashtable, adding it if new */
			elem_value = temporalinst_value(inst);
			item_value = (TrackItem *) hash_search(elements_tab_value,
												   (const void *) &elem_value,
												   HASH_ENTER, &found_value);

			if (found_value) {
				/* The element value is already on the tracking list */

				/*
				 * The operators we assist ignore duplicate array elements, so
				 * count a given distinct element only once per array.
				 */
				if (item_value->last_container == array_no)
					continue;

				item_value->frequency++;
				item_value->last_container = array_no;
			} else {
				/* Initialize new tracking list element */

				/*
				 * If element type is pass-by-reference, we must copy it into
				 * palloc'd space, so that we can release the array below. (We
				 * do this so that the space needed for element values is
				 * limited by the size of the hashtable; if we kept all the
				 * array values around, it could be much more.)
				 */
				item_value->key = elem_value;

				item_value->frequency = 1;
				item_value->delta = b_current_value - 1;
				item_value->last_container = array_no;
			}

			/* element_no is the number of elements processed (ie N) */
			element_no_value++;

			/* Lookup current element in hashtable, adding it if new */
			elem_temporal = TimestampTzGetDatum(inst->t);
			item_temporal = (TrackItem *) hash_search(elements_tab_temporal,
													  (const void *) &elem_temporal,
													  HASH_ENTER, &found_temporal);

			if (found_temporal) {
				/* The element value is already on the tracking list */

				/*
				 * The operators we assist ignore duplicate array elements, so
				 * count a given distinct element only once per array.
				 */
				if (item_temporal->last_container == array_no)
					continue;

				item_temporal->frequency++;
				item_temporal->last_container = array_no;
			} else {
				/* Initialize new tracking list element */

				/*
				 * If element type is pass-by-reference, we must copy it into
				 * palloc'd space, so that we can release the array below. (We
				 * do this so that the space needed for element values is
				 * limited by the size of the hashtable; if we kept all the
				 * array values around, it could be much more.)
				 */
				item_temporal->key = elem_temporal;

				item_temporal->frequency = 1;
				item_temporal->delta = b_current_temporal - 1;
				item_temporal->last_container = array_no;
			}

			/* element_no is the number of elements processed (ie N) */
			element_no_temporal++;
		}

		total_width += VARSIZE_ANY(DatumGetPointer(value));


		/* Count null element presence once per array. */
		if (null_present)
			null_elem_cnt++;

		/* Update frequency of the particular array distinct element count. */
		distinct_count_value = (int) (element_no_value - prev_element_no_value);
		count_item_value = (DECountItem *) hash_search(count_tab_value, &distinct_count_value,
													   HASH_ENTER,
													   &count_item_found_value);

		distinct_count_temporal = (int) (element_no_temporal - prev_element_no_temporal);
		count_item_temporal = (DECountItem *) hash_search(count_tab_temporal, &distinct_count_temporal,
														  HASH_ENTER,
														  &count_item_found_temporal);

		if (count_item_found_value)
			count_item_value->frequency++;
		else
			count_item_value->frequency = 1;

		if (count_item_found_temporal)
			count_item_temporal->frequency++;
		else
			count_item_temporal->frequency = 1;

		analyzed_rows++;
	}

	/* We can only compute real stats if we found some non-null values. */
	if (analyzed_rows > 0)
	{


		stats->stats_valid = true;
		/* Do the simple null-frac and width stats */
		stats->stanullfrac = (double) null_cnt / (double) samplerows;
		stats->stawidth = total_width / (double) analyzed_rows;

		/* Estimate that non-null values are unique */
		stats->stadistinct = -1.0 * (1.0 - stats->stanullfrac);




		/*  Value Part  statistics */
		slot_idx = 0;
		int			nonnull_cnt = analyzed_rows;
		int			count_items_count;
		TrackItem **sort_table;
		int			track_len;
		int64		cutoff_freq;
		int64		minfreq,
				maxfreq;
		int		 i;

		/*
		 * We assume the standard stats code already took care of setting
		 * stats_valid, stanullfrac, stawidth, stadistinct.  We'd have to
		 * re-compute those values if we wanted to not store the standard
		 * stats.
		 */

		/*
		 * Construct an array of the interesting hashtable items, that is,
		 * those meeting the cutoff frequency (s - epsilon)*N.  Also identify
		 * the minimum and maximum frequencies among these items.
		 *
		 * Since epsilon = s/10 and bucket_width = 1/epsilon, the cutoff
		 * frequency is 9*N / bucket_width.
		 */
		cutoff_freq = 9 * element_no_value / bucket_width;

		i = (int)hash_get_num_entries(elements_tab_value); /* surely enough space */
		sort_table = (TrackItem **) palloc(sizeof(TrackItem *) * i);

		hash_seq_init(&scan_status_value, elements_tab_value);
		track_len = 0;
		minfreq = element_no_value;
		maxfreq = 0;
		while ((item_value = (TrackItem *) hash_seq_search(&scan_status_value)) != NULL)
		{
			if (item_value->frequency > cutoff_freq)
			{
				sort_table[track_len++] = item_value;
				minfreq = Min(minfreq, item_value->frequency);
				maxfreq = Max(maxfreq, item_value->frequency);
			}
		}

		/*
		 * If we obtained more elements than we really want, get rid of those
		 * with least frequencies.  The easiest way is to qsort the array into
		 * descending frequency order and truncate the array.
		 */
		if (num_mcelem < track_len)
		{
			qsort(sort_table, track_len, sizeof(TrackItem *),
				  trackitem_compare_frequencies_desc);
			/* reset minfreq to the smallest frequency we're keeping */
			minfreq = sort_table[num_mcelem - 1]->frequency;
		}
		else
			num_mcelem = track_len;

		/* Generate MCELEM slot entry */
		if (num_mcelem > 0)
		{
			MemoryContext old_context;
			Datum	   *mcelem_values;
			float4	   *mcelem_freqs;

			/*
			 * We want to store statistics sorted on the element value using
			 * the element type's default comparison function.  This permits
			 * fast binary searches in selectivity estimation functions.
			 */
			qsort(sort_table, num_mcelem, sizeof(TrackItem *),
				  trackitem_compare_element);

			/* Must copy the target values into anl_context */
			old_context = MemoryContextSwitchTo(stats->anl_context);

			/*
			 * We sorted statistics on the element value, but we want to be
			 * able to find the minimal and maximal frequencies without going
			 * through all the values.  We also want the frequency of null
			 * elements.  Store these three values at the end of mcelem_freqs.
			 */
			mcelem_values = (Datum *) palloc(num_mcelem * sizeof(Datum));
			mcelem_freqs = (float4 *) palloc((num_mcelem + 3) * sizeof(float4));

			/*
			 * See comments above about use of nonnull_cnt as the divisor for
			 * the final frequency estimates.
			 */
			for (i = 0; i < num_mcelem; i++)
			{
				TrackItem  *item = sort_table[i];

				mcelem_values[i] = item->key;
				mcelem_freqs[i] = (float4) item->frequency /
								  (float4) nonnull_cnt;
			}
			mcelem_freqs[i++] = (float4) minfreq / (float4) nonnull_cnt;
			mcelem_freqs[i++] = (float4) maxfreq / (float4) nonnull_cnt;
			mcelem_freqs[i++] = (float4) null_elem_cnt / (float4) nonnull_cnt;

			MemoryContextSwitchTo(old_context);

			stats->stakind[slot_idx] = STATISTIC_KIND_MCELEM;
			stats->staop[slot_idx] = array_extra_data->value_eq_opr;
			stats->stanumbers[slot_idx] = mcelem_freqs;
			/* See above comment about extra stanumber entries */
			stats->numnumbers[slot_idx] = num_mcelem + 3;
			stats->stavalues[slot_idx] = mcelem_values;
			stats->numvalues[slot_idx] = num_mcelem;
			/* We are storing values of element type */
			stats->statypid[slot_idx] = array_extra_data->value_type_id;
			stats->statyplen[slot_idx] = array_extra_data->value_typlen;
			stats->statypbyval[slot_idx] = array_extra_data->value_typbyval;
			stats->statypalign[slot_idx] = array_extra_data->value_typalign;
			slot_idx++;
		}

		/* Generate DECHIST slot entry */
		count_items_count = (int)hash_get_num_entries(count_tab_value);
		if (count_items_count > 0)
		{
			int			num_hist = stats->attr->attstattarget;
			DECountItem **sorted_count_items;
			int			j;
			int			delta;
			int64		frac;
			float4	   *hist;

			/* num_hist must be at least 2 for the loop below to work */
			num_hist = Max(num_hist, 2);

			/*
			 * Create an array of DECountItem pointers, and sort them into
			 * increasing count order.
			 */
			sorted_count_items = (DECountItem **)
					palloc(sizeof(DECountItem *) * count_items_count);
			hash_seq_init(&scan_status_value, count_tab_value);
			j = 0;
			while ((count_item_value = (DECountItem *) hash_seq_search(&scan_status_value)) != NULL)
			{
				sorted_count_items[j++] = count_item_value;
			}
			qsort(sorted_count_items, count_items_count,
				  sizeof(DECountItem *), countitem_compare_count);

			/*
			 * Prepare to fill stanumbers with the histogram, followed by the
			 * average count.  This array must be stored in anl_context.
			 */
			hist = (float4 *)
					MemoryContextAlloc(stats->anl_context,
									   sizeof(float4) * (num_hist + 1));
			hist[num_hist] = (float4) element_no_value / (float4) nonnull_cnt;

			/*----------
			 * Construct the histogram of distinct-element counts (DECs).
			 *
			 * The object of this loop is to copy the min and max DECs to
			 * hist[0] and hist[num_hist - 1], along with evenly-spaced DECs
			 * in between (where "evenly-spaced" is with reference to the
			 * whole input population of arrays).  If we had a complete sorted
			 * array of DECs, one per analyzed row, the i'th hist value would
			 * come from DECs[i * (analyzed_rows - 1) / (num_hist - 1)]
			 * (compare the histogram-making loop in compute_scalar_stats()).
			 * But instead of that we have the sorted_count_items[] array,
			 * which holds unique DEC values with their frequencies (that is,
			 * a run-length-compressed version of the full array).  So we
			 * control advancing through sorted_count_items[] with the
			 * variable "frac", which is defined as (x - y) * (num_hist - 1),
			 * where x is the index in the notional DECs array corresponding
			 * to the start of the next sorted_count_items[] element's run,
			 * and y is the index in DECs from which we should take the next
			 * histogram value.  We have to advance whenever x <= y, that is
			 * frac <= 0.  The x component is the sum of the frequencies seen
			 * so far (up through the current sorted_count_items[] element),
			 * and of course y * (num_hist - 1) = i * (analyzed_rows - 1),
			 * per the subscript calculation above.  (The subscript calculation
			 * implies dropping any fractional part of y; in this formulation
			 * that's handled by not advancing until frac reaches 1.)
			 *
			 * Even though frac has a bounded range, it could overflow int32
			 * when working with very large statistics targets, so we do that
			 * math in int64.
			 *----------
			 */
			delta = analyzed_rows - 1;
			j = 0;				/* current index in sorted_count_items */
			/* Initialize frac for sorted_count_items[0]; y is initially 0 */
			frac = (int64) sorted_count_items[0]->frequency * (num_hist - 1);
			for (i = 0; i < num_hist; i++)
			{
				while (frac <= 0)
				{
					/* Advance, and update x component of frac */
					j++;
					frac += (int64) sorted_count_items[j]->frequency * (num_hist - 1);
				}
				hist[i] = sorted_count_items[j]->count;
				frac -= delta;	/* update y for upcoming i increment */
			}
			Assert(j == count_items_count - 1);

			stats->stakind[slot_idx] = STATISTIC_KIND_DECHIST;
			stats->staop[slot_idx] = array_extra_data->value_eq_opr;
			stats->stanumbers[slot_idx] = hist;
			stats->numnumbers[slot_idx] = num_hist + 1;
			slot_idx++;
		}


		/*  Temporal Part  statistics */

		/*
		 * Construct an array of the interesting hashtable items, that is,
		 * those meeting the cutoff frequency (s - epsilon)*N.  Also identify
		 * the minimum and maximum frequencies among these items.
		 *
		 * Since epsilon = s/10 and bucket_width = 1/epsilon, the cutoff
		 * frequency is 9*N / bucket_width.
		 */
		cutoff_freq = 9 * element_no_temporal / bucket_width;

		i = (int)hash_get_num_entries(elements_tab_temporal); /* surely enough space */
		sort_table = (TrackItem **) palloc(sizeof(TrackItem *) * i);

		hash_seq_init(&scan_status_temporal, elements_tab_temporal);
		track_len = 0;
		minfreq = element_no_temporal;
		maxfreq = 0;
		while ((item_temporal = (TrackItem *) hash_seq_search(&scan_status_temporal)) != NULL)
		{
			if (item_temporal->frequency > cutoff_freq)
			{
				sort_table[track_len++] = item_temporal;
				minfreq = Min(minfreq, item_temporal->frequency);
				maxfreq = Max(maxfreq, item_temporal->frequency);
			}
		}
		Assert(track_len <= i);

		/* emit some statistics for debug purposes */
		elog(DEBUG3, "compute_array_stats: target # mces = %d, "
					 "bucket width = %d, "
					 "# elements = " INT64_FORMAT ", hashtable size = %d, "
												  "usable entries = %d",
			 num_mcelem, bucket_width, element_no_temporal, i, track_len);

		/*
		 * If we obtained more elements than we really want, get rid of those
		 * with least frequencies.  The easiest way is to qsort the array into
		 * descending frequency order and truncate the array.
		 */
		if (num_mcelem < track_len)
		{
			qsort(sort_table, track_len, sizeof(TrackItem *),
				  trackitem_compare_frequencies_desc);
			/* reset minfreq to the smallest frequency we're keeping */
			minfreq = sort_table[num_mcelem - 1]->frequency;
		}
		else
			num_mcelem = track_len;

		/* Generate MCELEM slot entry */
		if (num_mcelem > 0)
		{
			MemoryContext old_context;
			Datum	   *mcelem_values;
			float4	   *mcelem_freqs;

			/*
			 * We want to store statistics sorted on the element value using
			 * the element type's default comparison function.  This permits
			 * fast binary searches in selectivity estimation functions.
			 */
			qsort(sort_table, num_mcelem, sizeof(TrackItem *),
				  trackitem_compare_element);

			/* Must copy the target values into anl_context */
			old_context = MemoryContextSwitchTo(stats->anl_context);

			/*
			 * We sorted statistics on the element value, but we want to be
			 * able to find the minimal and maximal frequencies without going
			 * through all the values.  We also want the frequency of null
			 * elements.  Store these three values at the end of mcelem_freqs.
			 */
			mcelem_values = (Datum *) palloc(num_mcelem * sizeof(Datum));
			mcelem_freqs = (float4 *) palloc((num_mcelem + 3) * sizeof(float4));

			/*
			 * See comments above about use of nonnull_cnt as the divisor for
			 * the final frequency estimates.
			 */
			for (i = 0; i < num_mcelem; i++)
			{
				TrackItem  *item = sort_table[i];

				mcelem_values[i] = item->key;
				mcelem_freqs[i] = (float4) item->frequency /
								  (float4) nonnull_cnt;
			}
			mcelem_freqs[i++] = (float4) minfreq / (float4) nonnull_cnt;
			mcelem_freqs[i++] = (float4) maxfreq / (float4) nonnull_cnt;
			mcelem_freqs[i++] = (float4) null_elem_cnt / (float4) nonnull_cnt;

			MemoryContextSwitchTo(old_context);

			stats->stakind[slot_idx] = STATISTIC_KIND_MCELEM;
			stats->staop[slot_idx] = array_extra_data->temporal_eq_opr;
			stats->stanumbers[slot_idx] = mcelem_freqs;
			/* See above comment about extra stanumber entries */
			stats->numnumbers[slot_idx] = num_mcelem + 3;
			stats->stavalues[slot_idx] = mcelem_values;
			stats->numvalues[slot_idx] = num_mcelem;
			/* We are storing values of element type */
			stats->statypid[slot_idx] = array_extra_data->temporal_type_id;
			stats->statyplen[slot_idx] = array_extra_data->temporal_typlen;
			stats->statypbyval[slot_idx] = array_extra_data->temporal_typbyval;
			stats->statypalign[slot_idx] = array_extra_data->temporal_typalign;
			stats->stats_valid = true;
			slot_idx++;
		}

		/* Generate DECHIST slot entry */
		count_items_count = (int)hash_get_num_entries(count_tab_temporal);
		if (count_items_count > 0)
		{
			int			num_hist = stats->attr->attstattarget;
			DECountItem **sorted_count_items;
			int			j;
			int			delta;
			int64		frac;
			float4	   *hist;

			/* num_hist must be at least 2 for the loop below to work */
			num_hist = Max(num_hist, 2);

			/*
			 * Create an array of DECountItem pointers, and sort them into
			 * increasing count order.
			 */
			sorted_count_items = (DECountItem **)
					palloc(sizeof(DECountItem *) * count_items_count);
			hash_seq_init(&scan_status_temporal, count_tab_temporal);
			j = 0;
			while ((count_item_temporal = (DECountItem *) hash_seq_search(&scan_status_temporal)) != NULL)
			{
				sorted_count_items[j++] = count_item_temporal;
			}
			qsort(sorted_count_items, count_items_count,
				  sizeof(DECountItem *), countitem_compare_count);

			/*
			 * Prepare to fill stanumbers with the histogram, followed by the
			 * average count.  This array must be stored in anl_context.
			 */
			hist = (float4 *)
					MemoryContextAlloc(stats->anl_context,
									   sizeof(float4) * (num_hist + 1));
			hist[num_hist] = (float4) element_no_temporal / (float4) nonnull_cnt;

			/*----------
			 * Construct the histogram of distinct-element counts (DECs).
			 *
			 * The object of this loop is to copy the min and max DECs to
			 * hist[0] and hist[num_hist - 1], along with evenly-spaced DECs
			 * in between (where "evenly-spaced" is with reference to the
			 * whole input population of arrays).  If we had a complete sorted
			 * array of DECs, one per analyzed row, the i'th hist value would
			 * come from DECs[i * (analyzed_rows - 1) / (num_hist - 1)]
			 * (compare the histogram-making loop in compute_scalar_stats()).
			 * But instead of that we have the sorted_count_items[] array,
			 * which holds unique DEC values with their frequencies (that is,
			 * a run-length-compressed version of the full array).  So we
			 * control advancing through sorted_count_items[] with the
			 * variable "frac", which is defined as (x - y) * (num_hist - 1),
			 * where x is the index in the notional DECs array corresponding
			 * to the start of the next sorted_count_items[] element's run,
			 * and y is the index in DECs from which we should take the next
			 * histogram value.  We have to advance whenever x <= y, that is
			 * frac <= 0.  The x component is the sum of the frequencies seen
			 * so far (up through the current sorted_count_items[] element),
			 * and of course y * (num_hist - 1) = i * (analyzed_rows - 1),
			 * per the subscript calculation above.  (The subscript calculation
			 * implies dropping any fractional part of y; in this formulation
			 * that's handled by not advancing until frac reaches 1.)
			 *
			 * Even though frac has a bounded range, it could overflow int32
			 * when working with very large statistics targets, so we do that
			 * math in int64.
			 *----------
			 */
			delta = analyzed_rows - 1;
			j = 0;				/* current index in sorted_count_items */
			/* Initialize frac for sorted_count_items[0]; y is initially 0 */
			frac = (int64) sorted_count_items[0]->frequency * (num_hist - 1);
			for (i = 0; i < num_hist; i++)
			{
				while (frac <= 0)
				{
					/* Advance, and update x component of frac */
					j++;
					frac += (int64) sorted_count_items[j]->frequency * (num_hist - 1);
				}
				hist[i] = sorted_count_items[j]->count;
				frac -= delta;	/* update y for upcoming i increment */
			}
			Assert(j == count_items_count - 1);

			stats->stakind[slot_idx] = STATISTIC_KIND_DECHIST;
			stats->staop[slot_idx] = array_extra_data->temporal_eq_opr;
			stats->stanumbers[slot_idx] = hist;
			stats->numnumbers[slot_idx] = num_hist + 1;
		}
	}

	/*
	 * We don't need to bother cleaning up any of our temporary palloc's. The
	 * hashtable should also go away, as it used a child memory context.
	 */
}
/*****************************************************************************
 * Statistics functions for Trajectory types (TemporalSeq and TemporalS)
 *****************************************************************************/
void
compute_timestamptz_traj_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
							   int samplerows, double totalrows)
{
	int null_cnt = 0;
	int analyzed_arrays = 0;
	int array_no;
	int slot_idx;
	int num_bins = stats->attr->attstattarget;
	int num_hist;
	float8 *temporal_lengths;
	PeriodBound *temporal_lowers,
			*temporal_uppers;
	double total_width = 0;


	Oid baseType = base_oid_from_temporal(stats->attrtypid);



	temporal_lowers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
	temporal_uppers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
	temporal_lengths = (float8 *) palloc(sizeof(float8) * samplerows);
	/* Loop over the arrays. */
	for (array_no = 0; array_no < samplerows; array_no++) {
		Datum value;
		bool isnull;
		PeriodBound temp_lower,
				temp_upper;
		float8 length = 0;
		vacuum_delay_point();

		value = fetchfunc(stats, array_no, &isnull);
		if (isnull) {
			/* array is null, just count that */
			null_cnt++;
			continue;
		}
		Period *period = get_bbox_onedim(value, stats->attrtypid);
		period_deserialize(period, &temp_lower, &temp_upper);
		//pfree(period);
		/* The size of a period is 24 */
		total_width += VARSIZE_ANY(DatumGetPointer(value));

		/* Remember bounds and length for further usage in histograms */
		temporal_lowers[analyzed_arrays] = temp_lower;
		temporal_uppers[analyzed_arrays] = temp_upper;


		// For an ordinary period, use subdiff function between upper
		// and lower bound values.
		length = period_duration_secs(temp_upper.val, temp_lower.val);
		temporal_lengths[analyzed_arrays] = length;


		/* Removing the temporal part from the stats HeapTuples if the base type is geometry or geography */
		if(baseType == type_oid(T_GEOMETRY) || baseType == type_oid(T_GEOGRAPHY))
			stats->rows[array_no] = remove_temporaldim(stats->rows[array_no], stats->tupDesc, stats->tupDesc->natts, stats->attrtypid, true, value);

		analyzed_arrays++;
	}

	/* We can only compute real stats if we found some non-null values. */
	if (analyzed_arrays > 0) {

		int pos,
				posfrac,
				delta,
				deltafrac,
				i;

		MemoryContext old_cxt;


		stats->stats_valid = true;
		/* Do the simple null-frac and width stats */
		stats->stanullfrac = (double) null_cnt / (double) samplerows;
		stats->stawidth = total_width / (double) analyzed_arrays;

		/* Estimate that non-null values are unique */
		stats->stadistinct = -1.0 * (1.0 - stats->stanullfrac);

		/* Must copy the target values into anl_context */
		old_cxt = MemoryContextSwitchTo(stats->anl_context);

		slot_idx = 2;

		Datum *bound_hist_values;
		Datum *length_hist_values;

		/*
		 * Generate a bounds histogram slot entry if there are at least two
		 * values.
		 */
		//problem start
		if (analyzed_arrays >= 2) {
			/* Sort bound values */
			qsort(temporal_lowers, analyzed_arrays, sizeof(PeriodBound), period_bound_qsort_cmp);
			qsort(temporal_uppers, analyzed_arrays, sizeof(PeriodBound), period_bound_qsort_cmp);

			num_hist = analyzed_arrays;
			if (num_hist > num_bins)
				num_hist = num_bins + 1;

			bound_hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

			/*
			 * The object of this loop is to construct periods from first and
			 * last entries in lowers[] and uppers[] along with evenly-spaced
			 * values in between. So the i'th value is a period of lowers[(i *
			 * (nvals - 1)) / (num_hist - 1)] and uppers[(i * (nvals - 1)) /
			 * (num_hist - 1)]. But computing that subscript directly risks
			 * integer overflow when the stats target is more than a couple
			 * thousand.  Instead we add (nvals - 1) / (num_hist - 1) to pos
			 * at each step, tracking the integral and fractional parts of the
			 * sum separately.
			 */
			delta = (analyzed_arrays - 1) / (num_hist - 1);
			deltafrac = (analyzed_arrays - 1) % (num_hist - 1);
			pos = posfrac = 0;

			for (i = 0; i < num_hist; i++) {
				bound_hist_values[i] =
						PointerGetDatum(period_make(temporal_lowers[pos].val, temporal_uppers[pos].val,
													temporal_lowers[pos].inclusive, temporal_uppers[pos].inclusive));

				pos += delta;
				posfrac += deltafrac;
				if (posfrac >= (num_hist - 1)) {
					/* fractional part exceeds 1, carry to integer part */
					pos++;
					posfrac -= (num_hist - 1);
				}
			}

			stats->stakind[slot_idx] = STATISTIC_KIND_BOUNDS_HISTOGRAM;
			stats->stavalues[slot_idx] = bound_hist_values;
			stats->numvalues[slot_idx] = num_hist;
			stats->statypid[slot_idx] = type_oid(T_PERIOD);
			stats->statyplen[slot_idx] = sizeof(Period);

			stats->statypbyval[slot_idx] = false;
			stats->statypalign[slot_idx] = 'd';
			slot_idx++;
		}

		/*
		 * Generate a length histogram slot entry if there are at least two
		 * values.
		 */
		if (analyzed_arrays >= 2) {
			/*
			 * Ascending sort of period lengths for further filling of
			 * histogram
			 */
			qsort(temporal_lengths, analyzed_arrays, sizeof(float8), float8_qsort_cmp);

			num_hist = analyzed_arrays;
			if (num_hist > num_bins)
				num_hist = num_bins + 1;

			length_hist_values = (Datum *) palloc(num_hist * sizeof(Datum));


			/*
			 * The object of this loop is to copy the first and last lengths[]
			 * entries along with evenly-spaced values in between. So the i'th
			 * value is lengths[(i * (nvals - 1)) / (num_hist - 1)]. But
			 * computing that subscript directly risks integer overflow when
			 * the stats target is more than a couple thousand.  Instead we
			 * add (nvals - 1) / (num_hist - 1) to pos at each step, tracking
			 * the integral and fractional parts of the sum separately.
			 */
			delta = (analyzed_arrays - 1) / (num_hist - 1);
			deltafrac = (analyzed_arrays - 1) % (num_hist - 1);
			pos = posfrac = 0;

			for (i = 0; i < num_hist; i++) {
				length_hist_values[i] = Float8GetDatum(temporal_lengths[pos]);
				pos += delta;
				posfrac += deltafrac;
				if (posfrac >= (num_hist - 1)) {
					/* fractional part exceeds 1, carry to integer part */
					pos++;
					posfrac -= (num_hist - 1);
				}
			}
		} else {
			/*
			 * Even when we don't create the histogram, store an empty array
			 * to mean "no histogram". We can't just leave stavalues NULL,
			 * because get_attstatsslot() errors if you ask for stavalues, and
			 * it's NULL.
			 */
			length_hist_values = palloc(0);
			num_hist = 0;
		}
		stats->stakind[slot_idx] = STATISTIC_KIND_RANGE_LENGTH_HISTOGRAM;
		stats->staop[slot_idx] = Float8LessOperator;
		stats->stavalues[slot_idx] = length_hist_values;
		stats->numvalues[slot_idx] = num_hist;
		stats->statypid[slot_idx] = FLOAT8OID;
		stats->statyplen[slot_idx] = sizeof(float8);
#ifdef USE_FLOAT8_BYVAL
		stats->statypbyval[slot_idx] = true;
#else
		stats->statypbyval[slot_idx] = false;
#endif
		stats->statypalign[slot_idx] = 'd';

		MemoryContextSwitchTo(old_cxt);
	} else if (null_cnt > 0) {
		/* We found only nulls; assume the column is entirely null */
		stats->stats_valid = true;
		stats->stanullfrac = 1.0;
		stats->stawidth = 0;	/* "unknown" */
		stats->stadistinct = 0.0;		/* "unknown" */
	}

	/*
	 * We don't need to bother cleaning up any of our temporary palloc's. The
	 * hashtable should also go away, as it used a child memory context.
	 */
}

void
compute_twodim_traj_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
						  int samplerows, double totalrows)
{
	int null_cnt = 0;
	int analyzed_arrays = 0;
	int array_no;
	int slot_idx;
	int num_bins = stats->attr->attstattarget;
	int num_hist;
	float8 *value_lengths;
	RangeBound *value_lowers,
			*value_uppers;
	float8 *temporal_lengths;
	PeriodBound *temporal_lowers,
			*temporal_uppers;
	double total_width = 0;
	Oid valueRangeType = 0;

	TemporalArrayAnalyzeExtraData *extra_data = (TemporalArrayAnalyzeExtraData *)stats->extra_data;

	valueRangeType = range_oid_from_base(extra_data->value_type_id);

	value_lowers = (RangeBound *) palloc(sizeof(RangeBound) * samplerows);
	value_uppers = (RangeBound *) palloc(sizeof(RangeBound) * samplerows);
	value_lengths = (float8 *) palloc(sizeof(float8) * samplerows);

	temporal_lowers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
	temporal_uppers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
	temporal_lengths = (float8 *) palloc(sizeof(float8) * samplerows);


	/* Loop over the arrays. */
	for (array_no = 0; array_no < samplerows; array_no++) {
		Datum value;
		bool isnull;
		BOX *box;
		RangeBound lower1,
				upper1;
		PeriodBound lower2,
				upper2;
		float8 value_length = 0, temporal_length = 0;
		vacuum_delay_point();

		value = fetchfunc(stats, array_no, &isnull);
		if (isnull) {
			/* array is null, just count that */
			null_cnt++;
			continue;
		}



		/* The size of a period is 24 */
		total_width += VARSIZE_ANY(DatumGetPointer(value));

		/* Remember bounds and length for further usage in histograms */
		box = get_bbox_twodim(value, stats->attrtypid);
		box_deserialize(box, &lower1, &upper1, &lower2, &upper2);

		value_lowers[analyzed_arrays] = lower1;
		value_uppers[analyzed_arrays] = upper1;


		// For an ordinary period, use subdiff function between upper
		// and lower bound values.

		if (valueRangeType == type_oid(T_FLOATRANGE))
			value_length = DatumGetFloat8(value_uppers[analyzed_arrays].val) - DatumGetFloat8(value_lowers[analyzed_arrays].val);
		else if (valueRangeType == type_oid(T_INTRANGE))
			value_length = (float8)(DatumGetInt32(value_uppers[analyzed_arrays].val) - DatumGetInt32(value_lowers[analyzed_arrays].val));

		value_lengths[analyzed_arrays] = value_length;


		/* Remember bounds and length for further usage in histograms */
		temporal_lowers[analyzed_arrays] = lower2;
		temporal_uppers[analyzed_arrays] = upper2;


		// For an ordinary period, use subdiff function between upper
		// and lower bound values.
		temporal_length = period_duration_secs(upper2.val, lower2.val);
		temporal_lengths[analyzed_arrays] = temporal_length;

		analyzed_arrays++;
	}


	slot_idx = 0;

	/* We can only compute real stats if we found some non-null values. */
	if (analyzed_arrays > 0) {

		int pos,
				posfrac,
				delta,
				deltafrac,
				i;

		MemoryContext old_cxt;


		stats->stats_valid = true;
		/* Do the simple null-frac and width stats */
		stats->stanullfrac = (double) null_cnt / (double) samplerows;
		stats->stawidth = total_width / (double) analyzed_arrays;

		/* Estimate that non-null values are unique */
		stats->stadistinct = -1.0 * (1.0 - stats->stanullfrac);

		/* Must copy the target values into anl_context */
		old_cxt = MemoryContextSwitchTo(stats->anl_context);





		Datum *value_bound_hist_values;
		Datum *value_length_hist_values;

		//p start ******************************************************************************************
		/*
		 * Generate a bounds histogram slot entry if there are at least two
		 * values.
		 */
		if (analyzed_arrays >= 2) {

			/* Sort bound values */
			qsort(value_lowers, analyzed_arrays, sizeof(RangeBound), range_bound_qsort_cmp);
			qsort(value_uppers, analyzed_arrays, sizeof(RangeBound), range_bound_qsort_cmp);



			num_hist = analyzed_arrays;
			if (num_hist > num_bins)
				num_hist = num_bins + 1;

			value_bound_hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

			/*
			 * The object of this loop is to construct ranges from first and
			 * last entries in lowers[] and uppers[] along with evenly-spaced
			 * values in between. So the i'th value is a range of lowers[(i *
			 * (nvals - 1)) / (num_hist - 1)] and uppers[(i * (nvals - 1)) /
			 * (num_hist - 1)]. But computing that subscript directly risks
			 * integer overflow when the stats target is more than a couple
			 * thousand.  Instead we add (nvals - 1) / (num_hist - 1) to pos
			 * at each step, tracking the integral and fractional parts of the
			 * sum separately.
			 */
			delta = (analyzed_arrays - 1) / (num_hist - 1);
			deltafrac = (analyzed_arrays - 1) % (num_hist - 1);
			pos = posfrac = 0;

			for (i = 0; i < num_hist; i++) {
				value_bound_hist_values[i] = PointerGetDatum(
						range_make(value_lowers[pos].val, value_uppers[pos].val, true, true, extra_data->value_type_id));

				pos += delta;
				posfrac += deltafrac;
				if (posfrac >= (num_hist - 1)) {
					/* fractional part exceeds 1, carry to integer part */
					pos++;
					posfrac -= (num_hist - 1);
				}
			}

			TypeCacheEntry *typentry;
			typentry = lookup_type_cache(range_oid_from_base(extra_data->value_type_id),
										 TYPECACHE_EQ_OPR |
										 TYPECACHE_CMP_PROC_FINFO |
										 TYPECACHE_HASH_PROC_FINFO);

			stats->stakind[slot_idx] = STATISTIC_KIND_BOUNDS_HISTOGRAM;
			stats->stavalues[slot_idx] = value_bound_hist_values;
			stats->numvalues[slot_idx] = num_hist;
			stats->statypid[slot_idx] = typentry->type_id;
			stats->statyplen[slot_idx] = typentry->typlen;
			stats->statypbyval[slot_idx] =typentry->typbyval;
			stats->statypalign[slot_idx] = typentry->typalign;

			slot_idx++;
		}

		//p end ******************************************************************************************

		/*
		 * Generate a length histogram slot entry if there are at least two
		 * values.
		 */
		if (analyzed_arrays >= 2) {
			/*
			 * Ascending sort of range lengths for further filling of
			 * histogram
			 */
			qsort(value_lengths, analyzed_arrays, sizeof(float8), float8_qsort_cmp);

			num_hist = analyzed_arrays;
			if (num_hist > num_bins)
				num_hist = num_bins + 1;

			value_length_hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

			/*
			 * The object of this loop is to copy the first and last lengths[]
			 * entries along with evenly-spaced values in between. So the i'th
			 * value is lengths[(i * (nvals - 1)) / (num_hist - 1)]. But
			 * computing that subscript directly risks integer overflow when
			 * the stats target is more than a couple thousand.  Instead we
			 * add (nvals - 1) / (num_hist - 1) to pos at each step, tracking
			 * the integral and fractional parts of the sum separately.
			 */
			delta = (analyzed_arrays - 1) / (num_hist - 1);
			deltafrac = (analyzed_arrays - 1) % (num_hist - 1);
			pos = posfrac = 0;

			for (i = 0; i < num_hist; i++) {
				value_length_hist_values[i] = Float8GetDatum(value_lengths[pos]);
				pos += delta;
				posfrac += deltafrac;
				if (posfrac >= (num_hist - 1)) {
					/* fractional part exceeds 1, carry to integer part */
					pos++;
					posfrac -= (num_hist - 1);
				}
			}
		} else {
			/*
			 * Even when we don't create the histogram, store an empty array
			 * to mean "no histogram". We can't just leave stavalues NULL,
			 * because get_attstatsslot() errors if you ask for stavalues, and
			 * it's NULL. We'll still store the empty fraction in stanumbers.
			 */
			value_length_hist_values = palloc(0);
			num_hist = 0;
		}
		stats->stakind[slot_idx] = STATISTIC_KIND_RANGE_LENGTH_HISTOGRAM;
		stats->staop[slot_idx] = Float8LessOperator;
		stats->stavalues[slot_idx] = value_length_hist_values;
		stats->numvalues[slot_idx] = num_hist;
		stats->statypid[slot_idx] = FLOAT8OID;
		stats->statyplen[slot_idx] = sizeof(float8);
#ifdef USE_FLOAT8_BYVAL
		stats->statypbyval[slot_idx] = true;
#else
		stats->statypbyval[slot_idx] = false;
#endif
		stats->statypalign[slot_idx] = 'd';

		slot_idx = 2;

		Datum *bound_hist_temporal;
		Datum *length_hist_temporal;

		/*
		 * Generate a bounds histogram slot entry if there are at least two
		 * values.
		 */
		//problem start
		if (analyzed_arrays >= 2) {
			/* Sort bound values */
			qsort(temporal_lowers, analyzed_arrays, sizeof(PeriodBound), period_bound_qsort_cmp);
			qsort(temporal_uppers, analyzed_arrays, sizeof(PeriodBound), period_bound_qsort_cmp);

			num_hist = analyzed_arrays;
			if (num_hist > num_bins)
				num_hist = num_bins + 1;

			bound_hist_temporal = (Datum *) palloc(num_hist * sizeof(Datum));

			/*
			 * The object of this loop is to construct periods from first and
			 * last entries in lowers[] and uppers[] along with evenly-spaced
			 * values in between. So the i'th value is a period of lowers[(i *
			 * (nvals - 1)) / (num_hist - 1)] and uppers[(i * (nvals - 1)) /
			 * (num_hist - 1)]. But computing that subscript directly risks
			 * integer overflow when the stats target is more than a couple
			 * thousand.  Instead we add (nvals - 1) / (num_hist - 1) to pos
			 * at each step, tracking the integral and fractional parts of the
			 * sum separately.
			 */
			delta = (analyzed_arrays - 1) / (num_hist - 1);
			deltafrac = (analyzed_arrays - 1) % (num_hist - 1);
			pos = posfrac = 0;

			for (i = 0; i < num_hist; i++) {
				bound_hist_temporal[i] =
						PointerGetDatum(period_make(temporal_lowers[pos].val, temporal_uppers[pos].val,
													temporal_lowers[pos].inclusive, temporal_uppers[pos].inclusive));

				pos += delta;
				posfrac += deltafrac;
				if (posfrac >= (num_hist - 1)) {
					/* fractional part exceeds 1, carry to integer part */
					pos++;
					posfrac -= (num_hist - 1);
				}
			}

			stats->stakind[slot_idx] = STATISTIC_KIND_BOUNDS_HISTOGRAM;
			stats->stavalues[slot_idx] = bound_hist_temporal;
			stats->numvalues[slot_idx] = num_hist;
			stats->statypid[slot_idx] = type_oid(T_PERIOD);
			stats->statyplen[slot_idx] = sizeof(Period);

			stats->statypbyval[slot_idx] = false;
			stats->statypalign[slot_idx] = 'd';
			slot_idx++;
		}

		/*
		 * Generate a length histogram slot entry if there are at least two
		 * values.
		 */
		if (analyzed_arrays >= 2) {
			/*
			 * Ascending sort of period lengths for further filling of
			 * histogram
			 */
			qsort(temporal_lengths, analyzed_arrays, sizeof(float8), float8_qsort_cmp);

			num_hist = analyzed_arrays;
			if (num_hist > num_bins)
				num_hist = num_bins + 1;

			length_hist_temporal = (Datum *) palloc(num_hist * sizeof(Datum));


			/*
			 * The object of this loop is to copy the first and last lengths[]
			 * entries along with evenly-spaced values in between. So the i'th
			 * value is lengths[(i * (nvals - 1)) / (num_hist - 1)]. But
			 * computing that subscript directly risks integer overflow when
			 * the stats target is more than a couple thousand.  Instead we
			 * add (nvals - 1) / (num_hist - 1) to pos at each step, tracking
			 * the integral and fractional parts of the sum separately.
			 */
			delta = (analyzed_arrays - 1) / (num_hist - 1);
			deltafrac = (analyzed_arrays - 1) % (num_hist - 1);
			pos = posfrac = 0;

			for (i = 0; i < num_hist; i++) {
				length_hist_temporal[i] = Float8GetDatum(temporal_lengths[pos]);
				pos += delta;
				posfrac += deltafrac;
				if (posfrac >= (num_hist - 1)) {
					/* fractional part exceeds 1, carry to integer part */
					pos++;
					posfrac -= (num_hist - 1);
				}
			}
		} else {
			/*
			 * Even when we don't create the histogram, store an empty array
			 * to mean "no histogram". We can't just leave stavalues NULL,
			 * because get_attstatsslot() errors if you ask for stavalues, and
			 * it's NULL.
			 */
			length_hist_temporal = palloc(0);
			num_hist = 0;
		}
		stats->stakind[slot_idx] = STATISTIC_KIND_RANGE_LENGTH_HISTOGRAM;
		stats->staop[slot_idx] = Float8LessOperator;
		stats->stavalues[slot_idx] = length_hist_temporal;
		stats->numvalues[slot_idx] = num_hist;
		stats->statypid[slot_idx] = FLOAT8OID;
		stats->statyplen[slot_idx] = sizeof(float8);
#ifdef USE_FLOAT8_BYVAL
		stats->statypbyval[slot_idx] = true;
#else
		stats->statypbyval[slot_idx] = false;
#endif
		stats->statypalign[slot_idx] = 'd';


		MemoryContextSwitchTo(old_cxt);
	} else if (null_cnt > 0) {
		/* We found only nulls; assume the column is entirely null */
		stats->stats_valid = true;
		stats->stanullfrac = 1.0;
		stats->stawidth = 0;	/* "unknown" */
		stats->stadistinct = 0.0;		/* "unknown" */
	}

	/*
	 * We don't need to bother cleaning up any of our temporary palloc's. The
	 * hashtable should also go away, as it used a child memory context.
	 */
}

HeapTuple
remove_temporaldim(HeapTuple tuple, TupleDesc tupDesc, int attrNum, Oid attrtypid, bool geom, Datum value)
{
	/* The below variables are used to remove the temporal part from
	 * the sample rows after getting the temporal value,
	 * to be able to collect other statistics in the same stats variable.
	 */
	Datum *values = (Datum *) palloc(attrNum * sizeof(Datum));
	bool *null_v = (bool *) palloc(attrNum * sizeof(bool));
	bool *rep_v = (bool *) palloc(attrNum * sizeof(bool));

	for (int j = 0; j < attrNum; j++)
	{
		int typemod = TYPMOD_GET_DURATION(tupDesc->attrs[j].atttypmod);
		switch (typemod)
		{
			case TEMPORAL:
			case TEMPORALINST:
			case TEMPORALI:
			case TEMPORALSEQ:
			case TEMPORALS:
			{
				if(attrtypid == tupDesc->attrs[j].atttypid)
				{
					if (geom)
						values[j] = tpoint_values_internal(DatumGetTemporal(value));
					else
						values[j] = tempdisc_get_values_internal(DatumGetTemporal(value));
					rep_v[j] = true;
					null_v[j] = false;
				}
				else
				{
					values[j] = 0;
					rep_v[j] = false;
					null_v[j] = false;
				}

				break;
			}
			default:
			{
				values[j] = 0;
				rep_v[j] = false;
				null_v[j] = false;
				break;
			}
		}
	}
	return heap_modify_tuple(tuple, tupDesc, values, null_v, rep_v);
}

/*****************************************************************************
 * Comparison functions for different data types
 *****************************************************************************/

/*
 * Hash function for elements.
 *
 * We use the element type's default hash opclass, and the column collation
 * if the type is collation-sensitive.
 */
uint32
element_hash_value(const void *key, Size keysize)
{
	return generic_element_hash(key,keysize,array_extra_data->value_hash);
}

uint32
element_hash_temporal(const void *key, Size keysize)
{
	return generic_element_hash(key,keysize,array_extra_data->temporal_hash);
}

/*
 * Matching function for elements, to be used in hashtable lookups.
 */
int
element_match(const void *key1, const void *key2, Size keysize)
{
	/* The keysize parameter is superfluous here */
	return element_compare(key1, key2);
}
/*
 * qsort() comparator for sorting TrackItems by frequencies (descending sort)
 */
int
trackitem_compare_frequencies_desc(const void *e1, const void *e2)
{
	const TrackItem *const *t1 = (const TrackItem *const *) e1;
	const TrackItem *const *t2 = (const TrackItem *const *) e2;

	return (*t2)->frequency - (*t1)->frequency;
}

/*
 * qsort() comparator for sorting TrackItems by element values
 */
int
trackitem_compare_element(const void *e1, const void *e2)
{
	const TrackItem *const *t1 = (const TrackItem *const *) e1;
	const TrackItem *const *t2 = (const TrackItem *const *) e2;

	return element_compare(&(*t1)->key, &(*t2)->key);
}

/*
 * qsort() comparator for sorting DECountItems by count
 */
int
countitem_compare_count(const void *e1, const void *e2)
{
	const DECountItem *const *t1 = (const DECountItem *const *) e1;
	const DECountItem *const *t2 = (const DECountItem *const *) e2;

	if ((*t1)->count < (*t2)->count)
		return -1;
	else if ((*t1)->count == (*t2)->count)
		return 0;
	else
		return 1;
}

/*
 * Comparison function for elements.
 *
 * We use the element type's default btree opclass, and the column collation
 * if the type is collation-sensitive.
 *
 * XXX consider using SortSupport infrastructure
 */
int
element_compare(const void *key1, const void *key2)
{
	Datum	   d1 = *((const Datum *) key1);
	Datum	   d2 = *((const Datum *) key2);
	Datum	   c;

	c = FunctionCall2Coll(array_extra_data->temporal_cmp,
						  DEFAULT_COLLATION_OID,
						  d1, d2);
	return DatumGetInt32(c);
}

uint32
generic_element_hash(const void *key, Size keysize, FmgrInfo * hash)
{
	Datum	   d = *((const Datum *) key);
	Datum	   h;

	h = FunctionCall1Coll(hash,
						  DEFAULT_COLLATION_OID,
						  d);
	return DatumGetUInt32(h);
}

/*
 * Comparison function for sorting PeriodTypeBounds.
 */
int
period_bound_qsort_cmp(const void *a1, const void *a2)
{
	PeriodBound *b1 = (PeriodBound *) a1;
	PeriodBound *b2 = (PeriodBound *) a2;
	return period_cmp_bounds(b1->val, b2->val, b1->lower, b2->lower, b1->inclusive, b2->inclusive);
}

/*
 * Comparison function for sorting float8s, used for period lengths.
 */
int
float8_qsort_cmp(const void *a1, const void *a2)
{
	const float8 *f1 = (const float8 *) a1;
	const float8 *f2 = (const float8 *) a2;

	if (*f1 < *f2)
		return -1;
	else if (*f1 == *f2)
		return 0;
	else
		return 1;
}

/*
 * Comparison function for sorting RangeBounds.
 */
int
range_bound_qsort_cmp(const void *a1, const void *a2)
{
	RangeBound *r1 = (RangeBound *) a1;
	RangeBound *r2 = (RangeBound *) a2;
	return period_cmp_bounds(DatumGetTimestampTz(r1->val),
							 DatumGetTimestampTz(r2->val),
							 r1->lower, r2->lower,
							 r1->inclusive, r2->inclusive);
}

/*
 * qsort_arg comparator for sorting ScalarItems
 *
 * Aside from sorting the items, we update the tupnoLink[] array
 * whenever two ScalarItems are found to contain equal datums.  The array
 * is indexed by tupno; for each ScalarItem, it contains the highest
 * tupno that that item's datum has been found to be equal to.  This allows
 * us to avoid additional comparisons in compute_scalar_stats().
 */
int
compare_scalars(const void *a, const void *b, void *arg)
{
	Datum da = ((const ScalarItem *) a)->value;
	int ta = ((const ScalarItem *) a)->tupno;
	Datum db = ((const ScalarItem *) b)->value;
	int tb = ((const ScalarItem *) b)->tupno;
	CompareScalarsContext *cxt = (CompareScalarsContext *) arg;
	int compare;

	compare = ApplySortComparator(da, false, db, false, cxt->ssup);
	if (compare != 0)
		return compare;

	/*
	 * The two datums are equal, so update cxt->tupnoLink[].
	 */
	if (cxt->tupnoLink[ta] < tb)
		cxt->tupnoLink[ta] = tb;
	if (cxt->tupnoLink[tb] < ta)
		cxt->tupnoLink[tb] = ta;

	/*
	 * For equal datums, sort by tupno
	 */
	return ta - tb;
}

/*
 * qsort comparator for sorting ScalarMCVItems by position
 */
int
compare_mcvs(const void *a, const void *b)
{
	int da = ((const ScalarMCVItem *) a)->first;
	int db = ((const ScalarMCVItem *) b)->first;

	return da - db;
}

/*****************************************************************************
 * Different functions used for 1D, 2D, and 3D types.
 *****************************************************************************/
/*
 * get_bbox_onedim()--returns the bbox of a one dimensional temporal object
 */
Period *
get_bbox_onedim(Datum value, Oid oid)
{
	if (oid == type_oid(T_PERIOD))
		return DatumGetPeriod(value);
	if (oid == type_oid(T_TBOOL) || oid == type_oid(T_TTEXT))
	{
		Period *p = (Period *)palloc(sizeof(Period));
		/* TODO MEMORY LEAK HERE !!!! */
		temporal_bbox(p, DatumGetTemporal(value));
		return p;
	}
	if (oid == type_oid(T_TGEOGPOINT) || oid == type_oid(T_TGEOMPOINT))
	{
		Period *p = (Period *)palloc(sizeof(Period));
		/* TODO MEMORY LEAK HERE !!!! */
		temporal_timespan_internal(p, DatumGetTemporal(value));
		return p;
	}
	return NULL;
}

/*
 * get_bbox_twodim()--returns the bbox of a two dimensional temporal object
 */
BOX*
get_bbox_twodim(Datum value, Oid oid)
{
	if (oid == type_oid(T_TINT) || oid == type_oid(T_TFLOAT) )
	{
		BOX *box = palloc(sizeof(BOX));
		temporal_bbox(box, DatumGetTemporal(value));
		return box;
	}
	return NULL;
}

/*
 * get_bbox_threedim()--returns the bbox of a three dimensional temporal object
 */
GBOX*
get_bbox_threedim(Datum value, Oid oid)
{
	if (oid == type_oid(T_TGEOMPOINT) || oid == type_oid(T_TGEOGPOINT))
	{
		GBOX *box = palloc0(sizeof(GBOX));
		temporal_bbox(box, DatumGetTemporal(value));
		return box;
	}
	return NULL;
}

/*
 * box_deserialize: decompose a box value
 */

void
box_deserialize(BOX *box, RangeBound *lowerdim1, RangeBound *upperdim1,
				PeriodBound *lowerdim2, PeriodBound *upperdim2)
{
	if (box && lowerdim1)
	{
		lowerdim1->val = (Datum)box->low.x;
		lowerdim1->inclusive = true;
		lowerdim1->lower = true;
	}
	if (box && upperdim1)
	{
		upperdim1->val = (Datum)box->high.x;
		upperdim1->inclusive = true;
		upperdim1->lower = false;
	}
	if (box && lowerdim2)
	{
		lowerdim2->val = (TimestampTz)box->low.y;
		lowerdim2->inclusive = true;
		lowerdim2->lower = true;
	}
	if (box && upperdim2)
	{
		upperdim2->val = (TimestampTz)box->high.y;
		upperdim2->inclusive = true;
		upperdim2->lower = false;
	}
}

/*
 * gbox_deserialize: decompose a GBOX value
 */
void
gbox_deserialize(GBOX *box, RangeBound *lowerdim1, RangeBound *upperdim1,
				 RangeBound *lowerdim2, RangeBound *upperdim2,
				 PeriodBound *lowerdim3, PeriodBound *upperdim3	)
{
	if (box && lowerdim1)
	{
		lowerdim1->val =(Datum) box->xmin;
		lowerdim1->inclusive = true;
		lowerdim1->lower = true;
	}
	if (box && upperdim1)
	{
		upperdim1->val =(Datum) box->xmax;
		upperdim1->inclusive = true;
		upperdim1->lower = false;
	}
	if (box && lowerdim2)
	{
		lowerdim2->val =(Datum) box->ymin;
		lowerdim2->inclusive = true;
		lowerdim2->lower = true;
	}
	if (box && upperdim2)
	{
		upperdim2->val =(Datum) box->ymax;
		upperdim2->inclusive = true;
		upperdim2->lower = false;
	}
	if (box && lowerdim3)
	{
		lowerdim3->val = (TimestampTz)box->zmin;
		lowerdim3->inclusive = true;
		lowerdim3->lower = true;
	}
	if (box && upperdim3)
	{
		upperdim3->val = (TimestampTz)box->zmax;
		upperdim3->inclusive = true;
		upperdim3->lower = false;
	}
}