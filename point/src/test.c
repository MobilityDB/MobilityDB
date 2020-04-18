/* Append an array of temporal values */

TemporalS *
temporals_merge_array(TemporalS **seqsets, int count)
{
	/* Test the validity of the temporal values */
	int totalcount = seqsets[0]->count;
	bool linear = MOBDB_FLAGS_GET_LINEAR(seqsets[0]->flags);
	Oid valuetypid = seqsets[0]->valuetypid;
	bool isgeo = (seqsets[0]->valuetypid == type_oid(T_GEOMETRY) ||
		seqsets[0]->valuetypid == type_oid(T_GEOGRAPHY));
	for (int i = 1; i < count; i++)
	{
		assert(valuetypid == seqsets[i]->valuetypid);
		assert(linear == MOBDB_FLAGS_GET_LINEAR(seqsets[i]->flags));
		if (isgeo)
		{
			assert(MOBDB_FLAGS_GET_GEODETIC(seqsets[0]->flags) ==
				MOBDB_FLAGS_GET_GEODETIC(seqsets[i]->flags));
			ensure_same_srid_tpoint((Temporal *)seqsets[0], (Temporal *)seqsets[i]);
			ensure_same_dimensionality_tpoint((Temporal *)seqsets[0], (Temporal *)seqsets[i]);
		}
		totalcount += seqsets[i]->count;
	}
	/* Collect the composing sequences */
	TemporalSeq **sequences = palloc0(sizeof(TemporalSeq *) * totalcount);
	int k = 0;
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < seqsets[i]->count; j++)
			sequences[k++] = temporals_seq_n(seqsets[i], j);
	}
	temporalseqarr_sort(sequences, count);
	/* Test the validity of the composing sequences */
	TemporalSeq *seq1 = sequences[0];
	for (int i = 1; i < totalcount; i++)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq1, seq1->count - 1);
		TemporalSeq *seq2 = sequences[i];
		TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
		char *t1, *t2;
		if (inst1->t > inst2->t)
		{
			t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
			t2 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst2->t));
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("The temporal values cannot overlap on time: %s, %s", t1, t2)));
		}
		if (inst1->t == inst2->t && seq1->period.upper_inc && seq2->period.lower_inc)
		{
			if (! datum_eq(temporalinst_value(inst1), temporalinst_value(inst2), inst1->valuetypid))
			{
				t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
				ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
					errmsg("The temporal values have different value at their overlapping instant %s", t1)));
			}
		}
		seq1 = seq2;
	}
	/* Create the result */
	int count1;
	TemporalSeq **normseqs = temporalseqarr_normalize(sequences, count, &count1);
	TemporalS *result = temporals_make(normseqs, count1,
		MOBDB_FLAGS_GET_LINEAR(ts1->flags));
	for (int i = 0; i < count1; i++)
		pfree(normseqs[i]);
	pfree(normseqs);
	pfree(sequences);
	return (Temporal *) result;
}