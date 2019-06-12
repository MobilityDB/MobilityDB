/*****************************************************************************
 *
 * AggregateFuncs.c
 *	  Temporal aggregate functions
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"
#include "Aggregates.h"

/*****************************************************************************
 * Functions manipulating skip lists
 *****************************************************************************/

static MemoryContext
set_aggregation_context(FunctionCallInfo fcinfo)
{
	MemoryContext ctx;
	if (!AggCheckCallContext(fcinfo, &ctx))
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("Operation not supported")));
	return  MemoryContextSwitchTo(ctx);
}

static void
unset_aggregation_context(MemoryContext ctx)
{
	MemoryContextSwitchTo(ctx);
}

SkipList *
skiplist_make(FunctionCallInfo fcinfo, Temporal **values, int count)
{
	//FIXME: tail should be a constant (e.g. 1) but is not, for ease of construction

	MemoryContext oldctx = set_aggregation_context(fcinfo);
	int capacity = SKIPLIST_INITIAL_CAPACITY;
	count += 2; /* Account for head & tail */
	while (capacity <= count)
		capacity <<= 1;
	SkipList *result = palloc0(sizeof(SkipList));
	result->elems = palloc0(sizeof(Elem) * capacity);
	int height = (int) ceil(log2(count-1));
	result->capacity = capacity;
	result->next = count;
	result->length = count - 2;

	/* Fill values first */
	result->elems[0].value = NULL;
	for (int i = 0; i < count-2; i ++)
		result->elems[i+1].value = temporal_copy(values[i]);
	result->elems[count-1].value = NULL;
	result->tail = count-1;

	/* Link the list in a balanced fashion */
	for (int level = 0; level < height; level ++)
	{
		int step = 1 << level;
		for (int i = 0; i < count; i += step)
		{
			int next = i + step < count ? i + step : count - 1;
			if (i != count - 1)
			{
				result->elems[i].next[level] = next;
				result->elems[i].height = level + 1;
			}
			else
			{
				result->elems[i].next[level] = - 1;
				result->elems[i].height = height;
			}
		}
	}
	unset_aggregation_context(oldctx);
	return result;
}

Temporal *
skiplist_headval(SkipList *list)
{
	return list->elems[list->elems[0].next[0]].value;
}

Temporal *
skiplist_tailval(SkipList *list)
{
	/* Despite the look, this is pretty much O(1) */
	int cur = 0;
	Elem *e = &list->elems[cur];
	int height = e->height;
	while (e->next[height-1] != list->tail)
		e = &list->elems[e->next[height-1]];
	return e->value;
}

int
skiplist_alloc(FunctionCallInfo fcinfo, SkipList *list)
{
	list->length ++;
	if (! list->freecount)
	{
		/* No free list, give first available entry */
		if (list->next >= list->capacity)
		{
			/* No more capacity, let's grow */
			list->capacity <<= SKIPLIST_GROW;
			MemoryContext ctx = set_aggregation_context(fcinfo);
			list->elems = repalloc(list->elems, sizeof(Elem) * list->capacity);
			unset_aggregation_context(ctx);
		}
		list->next ++;
		return list->next - 1;
	}
	else 
	{
		list->freecount --;
		return list->freed[list->freecount];
	}
}

void
skiplist_free(FunctionCallInfo fcinfo, SkipList *list, int cur)
{
	if (! list->freed)
	{
		list->freecap = SKIPLIST_INITIAL_FREELIST;
		MemoryContext ctx = set_aggregation_context(fcinfo);
		list->freed = palloc(sizeof(int) * list->freecap);
		unset_aggregation_context(ctx);
	}
	else if (list->freecount == list->freecap)
	{
		list->freecap <<= 1;
		MemoryContext ctx = set_aggregation_context(fcinfo);
		list->freed = repalloc(list->freed, sizeof(int) * list->freecap);
		unset_aggregation_context(ctx);
	}
	list->freed[list->freecount ++] = cur;
	list->length --;
}

typedef enum
{
	BEFORE,
	DURING,
	AFTER
} RelativeTimePos;

RelativeTimePos
pos_timestamp_timestamp(TimestampTz t1, TimestampTz t)
{
	int32 cmp;

	cmp = timestamp_cmp_internal(t1, t);
	if (cmp > 0)
		return BEFORE;
	if (cmp < 0)
		return AFTER;
	return DURING;
}

RelativeTimePos
pos_period_timestamp(Period *p, TimestampTz t)
{
	int32 cmp;

	cmp = timestamp_cmp_internal(p->lower, t);
	if (cmp > 0)
		return BEFORE;
	if (cmp == 0 && !(p->lower_inc))
		return BEFORE;

	cmp = timestamp_cmp_internal(p->upper, t);
	if (cmp < 0)
		return AFTER;
	if (cmp == 0 && !(p->upper_inc))
		return AFTER;

	return DURING;
}

/* Comparison function used for skiplists */
RelativeTimePos 
skiplist_elmpos(SkipList *list, int cur, TimestampTz t)
{
	if (cur == 0)
		return AFTER; /* Head is -inf */
	else if (cur == -1 || cur == list->tail)
		return BEFORE; /* Tail is +inf */
	else
	{
		if (list->elems[cur].value->duration == TEMPORALINST)
			return pos_timestamp_timestamp(((TemporalInst *)list->elems[cur].value)->t, t);
		else
			return pos_period_timestamp(&((TemporalSeq *)list->elems[cur].value)->period, t);
	}
}

Temporal **
skiplist_values(SkipList *list)
{
	Temporal **result = palloc(sizeof(Temporal *) * list->length);
	int cur = list->elems[0].next[0];
	int count = 0;
	while (cur != list->tail)
	{
		result[count++] = list->elems[cur].value;
		cur = list->elems[cur].next[0];
	}
	return result;
}

/* Outputs the skiplist in graphviz dot format for visualisation & debugging purposes */
static void 
skiplist_print(SkipList *list)
{
	int len = 0;
	char buf[16384];
	len += sprintf(buf+len, "digraph skiplist {\n");
	len += sprintf(buf+len, "\trankdir = LR;\n");
	len += sprintf(buf+len, "\tnode [shape = record];\n");
	int cur = 0;
	while (cur != -1)
	{
		Elem *e = &list->elems[cur];
		len += sprintf(buf+len, "\telm%d [label=\"", cur);
		for (int l = e->height - 1; l > 0; l --)
		{
			len += sprintf(buf+len, "<p%d>|", l);
		}
		if (! e->value)
			len += sprintf(buf+len, "<p0>\"];\n");
		else
			len += sprintf(buf+len, "<p0>%f\"];\n", 
				DatumGetFloat8(temporal_min_value_internal(e->value)));
		if (e->next[0] != -1)
		{
			for (int l = 0; l < e->height; l ++)
			{
				int next = e->next[l];
				len += sprintf(buf+len, "\telm%d:p%d -> elm%d:p%d ", cur, l, next, l);
				if (l == 0)
					len += sprintf(buf+len, "[weight=100];\n");
				else
					len += sprintf(buf+len, ";\n");
			}
		}
		cur = e->next[0];
	}
	sprintf(buf+len, "}\n");
	ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("SKIPLIST: %s", buf)));
}

/* This simulates up to SKIPLIST_MAXLEVEL repeated coin flips without 
	spinning the RNG every time (courtesy of the internet) */
int
random_level()
{
	return ffsl(~(random() & ((1l << SKIPLIST_MAXLEVEL) - 1)));
}

void
skiplist_splice(FunctionCallInfo fcinfo, SkipList *list, Temporal **values,
	int count, Period *period, Datum (*operator)(Datum, Datum), bool crossings)
{
	// O(count*log(n)) average (unless I'm mistaken)
	// O(n+count*log(n)) worst case (when period spans the whole list so everything has to be deleted)
	assert(list->length > 0);
	int16 duration = skiplist_headval(list)->duration;
	int update[SKIPLIST_MAXLEVEL];
	memset(update, 0, sizeof(update));
	int cur = 0;
	int height = list->elems[cur].height;
	Elem *e = &list->elems[cur];
	for (int level = height-1; level >= 0; level --)
	{
		while (e->next[level] != -1 && 
			skiplist_elmpos(list, e->next[level], period->lower) == AFTER)
		{
			cur = e->next[level];
			e = &list->elems[cur];
		}
		update[level] = cur;
	}

	int lower = e->next[0];
	cur = lower;
	e = &list->elems[cur];

	int spliced_count = 0;
	while (skiplist_elmpos(list, cur, period->upper) == AFTER)
	{
		cur = e->next[0];
		e = &list->elems[cur];
		spliced_count ++;
	}
	int upper = cur;
	if (upper >= 0 && skiplist_elmpos(list, upper, period->upper) == DURING)
	{
		upper = e->next[0]; /* if found upper, one more to remove */
		spliced_count ++;
	}

	/* Delete spliced-out elements but remember their values for later */
	cur = lower;
	Temporal **spliced = palloc(sizeof(Temporal *) * spliced_count);
	spliced_count = 0;
	while (cur != upper && cur != -1)
	{
		for (int level = 0; level < height; level ++)
		{
			Elem *prev = &list->elems[update[level]];
			if (prev->next[level] != cur)
				break;

			prev->next[level] = list->elems[cur].next[level];
		}
		spliced[spliced_count++] = list->elems[cur].value;
		skiplist_free(fcinfo, list, cur);
		cur = list->elems[cur].next[0];
	}

	/* Level down head & tail if necessary */
	Elem *head = &list->elems[0];
	Elem *tail = &list->elems[list->tail];
	while (head->height > 1 && head->next[head->height-1] == list->tail)
	{
		head->height --;
		tail->height --;
		height --;
	}

	if (spliced_count != 0)
	{
		/* We are not in a gap, we need to compute the aggregation */
		int newcount = 0;
		Temporal **newtemps;
		if (duration == TEMPORALINST)
			newtemps = (Temporal **)temporalinst_tagg2((TemporalInst **)spliced, spliced_count,
			(TemporalInst **)values, count, operator, &newcount);
		else
			newtemps = (Temporal **)temporalseq_tagg2((TemporalSeq **)spliced, spliced_count,
			(TemporalSeq **)values, count, operator, crossings, &newcount);
		values = newtemps;
		count = newcount;
		/* We need to delete the spliced-out temporal values */
		for (int i = 0; i < spliced_count; i ++)
			pfree(spliced[i]);
	}

	/* Insert new elements */
	for (int i = count - 1; i >= 0; i --)
	{
		int rheight = random_level();
		if (rheight > height)
		{
			for (int l = height; l < rheight; l ++)
				update[l] = 0;
			/* Grow head & tail as appropriate */
			head->height = rheight;
			tail->height = rheight;
		}
		int new = skiplist_alloc(fcinfo, list);
		Elem *newelm = &list->elems[new];
		MemoryContext ctx = set_aggregation_context(fcinfo);
		newelm->value = temporal_copy(values[i]);
		unset_aggregation_context(ctx);
		newelm->height = rheight;

		for (int level = 0; level < rheight; level ++)
		{
			newelm->next[level] = list->elems[update[level]].next[level];
			list->elems[update[level]].next[level] = new;
			if (level >= height && update[0] != list->tail)
			{
				newelm->next[level] = list->tail;
			}
		}
		if (rheight > height)
			height = rheight;
	}
}

PG_FUNCTION_INFO_V1(sl_test);
Datum
sl_test(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count = -1;
	//int count2 = -1;
	//Period *p = PG_GETARG_PERIOD(1);
	//ArrayType *array2 = PG_GETARG_ARRAYTYPE_P(2);
	Temporal **temps = temporalarr_extract(array, &count);
	//Temporal **temps2 = temporalarr_extract(array2, &count2);
	SkipList *sl = skiplist_make(fcinfo, temps, count);
	skiplist_print(sl);
	//skiplist_splice(fcinfo, sl, temps2, count2, p);
	//skiplist_print(sl);
	PG_RETURN_INT32(0);
}

/*****************************************************************************
 * Numeric aggregate functions on datums
 *****************************************************************************/

/* Get the minimum/maximum value of the two arguments */

Datum
datum_min_int32(Datum l, Datum r)
{
	return DatumGetInt32(l) < DatumGetInt32(r) ? l : r;
}

Datum
datum_max_int32(Datum l, Datum r)
{
	return DatumGetInt32(l) > DatumGetInt32(r) ? l : r;
}

Datum
datum_min_float8(Datum l, Datum r)
{
	return DatumGetFloat8(l) < DatumGetFloat8(r) ? l : r;
}

Datum
datum_max_float8(Datum l, Datum r)
{
	return DatumGetFloat8(l) > DatumGetFloat8(r) ? l : r;
}

Datum
datum_min_text(Datum l, Datum r)
{
	return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) < 0 ? l : r;
}

Datum
datum_max_text(Datum l, Datum r)
{
	return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) > 0 ? l : r;
}

/* Get the sum of the two arguments */

Datum
datum_sum_int32(Datum l, Datum r)
{
	return Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
}

Datum
datum_sum_float8(Datum l, Datum r)
{
	return Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
}

Datum
datum_sum_double2(Datum l, Datum r)
{
	return PointerGetDatum(double2_add((double2 *)DatumGetPointer(l), 
		(double2 *)DatumGetPointer(r)));
}

Datum
datum_sum_double3(Datum l, Datum r)
{
	return PointerGetDatum(double3_add((double3 *)DatumGetPointer(l), 
		(double3 *)DatumGetPointer(r)));
}

Datum
datum_sum_double4(Datum l, Datum r)
{
	return PointerGetDatum(double4_add((double4 *)DatumGetPointer(l), 
		(double4 *)DatumGetPointer(r)));
}

/*****************************************************************************
 * Generic binary aggregate functions needed for parallelization
 *****************************************************************************/

void 
aggstate_write(SkipList *state, StringInfo buf)
{
    Temporal** values = skiplist_values(state) ;
	pq_sendint32(buf, (uint32) state->length);
	Oid valuetypid = InvalidOid;
	if (state->length > 0)
		valuetypid = values[0]->valuetypid;

	pq_sendint32(buf, valuetypid);
	for (int i = 0; i < state->length; i ++)
		temporal_write(values[i], buf);
	pq_sendint64(buf, state->extrasize);
	if(state->extra)
		pq_sendbytes(buf, state->extra, state->extrasize);
}

SkipList *
aggstate_read(FunctionCallInfo fcinfo, StringInfo buf)
{
	MemoryContext ctx;
	assert(AggCheckCallContext(fcinfo, &ctx));
	MemoryContext oldctx = MemoryContextSwitchTo(ctx);

	int size = pq_getmsgint(buf, 4);
	Oid valuetypid = pq_getmsgint(buf, 4);
	Temporal** values = palloc0(size * sizeof(Temporal *));

	for (int i = 0; i < size; i ++)
		values[i] = temporal_read(buf, valuetypid);

	SkipList* result = skiplist_make(fcinfo, values, size) ;

	result->extrasize = pq_getmsgint64(buf);
	if(result->extrasize) 
	{
		const char* extra = pq_getmsgbytes(buf, result->extrasize);
		result->extra = palloc(result->extrasize);
		memcpy(result->extra, extra, result->extrasize);
	}

	MemoryContextSwitchTo(oldctx);
	return result;
}


PG_FUNCTION_INFO_V1(temporal_tagg_serialize);

PGDLLEXPORT Datum
temporal_tagg_serialize(PG_FUNCTION_ARGS)
{
    SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
    StringInfoData buf;
    pq_begintypsend(&buf);
    aggstate_write(state, &buf);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(temporal_tagg_deserialize);

PGDLLEXPORT Datum
temporal_tagg_deserialize(PG_FUNCTION_ARGS)
{
    bytea* data = PG_GETARG_BYTEA_P(0);
    StringInfoData buf =
            {
                    .cursor = 0,
                    .data = VARDATA(data),
                    .len = VARSIZE(data),
                    .maxlen = VARSIZE(data)
            };
    SkipList *result = aggstate_read(fcinfo, &buf);
    PG_RETURN_POINTER(result);
}

void
aggstate_set_extra(FunctionCallInfo fcinfo, SkipList *state, void *data,
	size_t size)
{
	MemoryContext ctx;
	assert(AggCheckCallContext(fcinfo, &ctx));
	MemoryContext oldctx = MemoryContextSwitchTo(ctx);
	state->extra = palloc(size);
	state->extrasize = size;
	memcpy(state->extra, data, size);
	MemoryContextSwitchTo(oldctx);
}

/*****************************************************************************
 * Transformation functions for count and avg
 *****************************************************************************/

/*
 * Transform a temporal number into a temporal integer for performing count
 * aggregation
 */
 
static TemporalInst *
temporalinst_transform_tcount(TemporalInst *inst)
{
	return temporalinst_make(Int32GetDatum(1), inst->t, INT4OID);
}

static TemporalI *
temporali_transform_tcount(TemporalI *ti)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = temporalinst_make(Int32GetDatum(1), inst->t, INT4OID);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);

	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	
	return result;
}

static TemporalSeq *
temporalseq_transform_tcount(TemporalSeq *seq)
{
	TemporalSeq *result;
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalinst_make(Int32GetDatum(1), 
			seq->period.lower, INT4OID); 
		result = temporalseq_from_temporalinstarr(&inst, 1,
			true, true, false);
		pfree(inst);
		return result;
	}

	TemporalInst *instants[2];
	instants[0] = temporalinst_make(Int32GetDatum(1), seq->period.lower,
		INT4OID); 
	instants[1] = temporalinst_make(Int32GetDatum(1), seq->period.upper,
		INT4OID); 
	result = temporalseq_from_temporalinstarr(instants, 2,
		seq->period.lower_inc, seq->period.upper_inc, false);
	pfree(instants[0]); pfree(instants[1]); 
	return result;
}

static TemporalS *
temporals_transform_tcount(TemporalS *ts)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = temporalseq_transform_tcount(seq);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, ts->count, true);

	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

/* Dispatch function */

Temporal *
temporal_transform_tcount(Temporal *temp)
{
	temporal_duration_is_valid(temp->duration);
	Temporal *result = NULL;
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_transform_tcount((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)temporali_transform_tcount((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_transform_tcount((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)temporals_transform_tcount((TemporalS *)temp);
	return result;
}

/*****************************************************************************/

/*
 * Transform a temporal number type into a temporal double2 type for 
 * performing average aggregation 
 */

TemporalInst *
tnumberinst_transform_tavg(TemporalInst *inst)
{
	double value = datum_double(temporalinst_value(inst), inst->valuetypid);
	double2 *dvalue = double2_construct(value, 1);
	TemporalInst *result = temporalinst_make(PointerGetDatum(dvalue), inst->t,
		type_oid(T_DOUBLE2));
	pfree(dvalue);
	return result;
}

TemporalInst **
tnumberi_transform_tavg(TemporalI *ti)
{
	TemporalInst **result = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		result[i] = tnumberinst_transform_tavg(inst);
	}
	return result;
}

int
tintseq_transform_tavg(TemporalSeq **result, TemporalSeq *seq)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = tnumberinst_transform_tavg(inst);
		result[0] = temporalseq_from_temporalinstarr(&inst1, 1,
			true, true, false);
		pfree(inst1);
		return 1;
	}
	
	int count;
	TemporalInst *instants[2];
	TemporalInst *inst1, *inst2;
	inst1 = temporalseq_inst_n(seq, 0);
	Datum value1 = temporalinst_value(inst1);
	Datum value2;
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count-1; i++)
	{
		inst2 = temporalseq_inst_n(seq, i+1);
		value2 = temporalinst_value(inst2);
		instants[0] = tnumberinst_transform_tavg(inst1);
		TemporalInst *inst = temporalinst_make(value1, inst2->t,
			inst1->valuetypid);
		instants[1] = tnumberinst_transform_tavg(inst);
		bool upper_inc = (i == seq->count-2) ? seq->period.upper_inc : false;
		bool upper_inc1 = upper_inc && 
			datum_eq(value1, value2, inst1->valuetypid);
		result[i] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc1, false);
		pfree(inst); pfree(instants[0]); pfree(instants[1]);
		inst1 = inst2;
		lower_inc = true;
	}
	if (seq->period.upper_inc && seq->count > 1 &&
		datum_ne(value1, value2, inst1->valuetypid))
	{
		instants[0] = tnumberinst_transform_tavg(inst2);
		result[seq->count-1] = temporalseq_from_temporalinstarr(instants, 1,
			true, true, false);
		count = seq->count;
		pfree(instants[0]);
	}
	else
		count = seq->count - 1;
	return count;
}

int 
tfloatseq_transform_tavg(TemporalSeq **result, TemporalSeq *seq)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tnumberinst_transform_tavg(inst);
	}
	result[0] = temporalseq_from_temporalinstarr(instants, 
		seq->count,	seq->period.lower_inc, seq->period.upper_inc, false);
		
	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return 1;
}

int
tnumberseq_transform_tavg(TemporalSeq **result, TemporalSeq *seq)
{
	int returnvalue = 0;
	number_base_type_oid(seq->valuetypid);
	if (seq->valuetypid == INT4OID)
		returnvalue = tintseq_transform_tavg(result, seq);
	if (seq->valuetypid == FLOAT8OID)
		returnvalue = tfloatseq_transform_tavg(result, seq);
	return returnvalue;
}

int
tnumbers_transform_tavg(TemporalSeq **result, TemporalS *ts)
{
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = tnumberseq_transform_tavg(&result[k], seq);
		k += countstep;
	}
	return k;
}

/*****************************************************************************
 * TemporalInst generic aggregation functions
 *****************************************************************************/

/*
 * Generic aggregate function for temporal instants.
 * Arguments:
 * - instants1 is the accumulated state 
 * - instants2 are the instants of a TemporalI value
 * The function returns new instants in the result that must
 * be freed by the calling function. 
 */

TemporalInst **
temporalinst_tagg2(TemporalInst **instants1, int count1, TemporalInst **instants2, 
	int count2, Datum (*func)(Datum, Datum), int *newcount)
{
	TemporalInst **result = palloc(sizeof(TemporalInst *) * (count1 + count2));
	int i = 0, j = 0, count = 0;
	while (i < count1 && j < count2)
	{
		TemporalInst *inst1 = instants1[i];
		TemporalInst *inst2 = instants2[j];
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			result[count++] = temporalinst_make(
				func(temporalinst_value(inst1), temporalinst_value(inst2)),
				inst1->t, inst1->valuetypid);
			i++;
			j++;
		}
		else if (cmp < 0)
		{
			result[count++] = temporalinst_copy(inst1);
			i++;
		}
		else
		{
			result[count++] = temporalinst_copy(inst2);
			j++;
		}
	}
	/* Copy the instants from state2 that are after the end of state1 */
	while (j < count2)
		result[count++] = temporalinst_copy(instants2[j++]);
	*newcount = count;	
	return result;
}

/*****************************************************************************
 * TemporalSeq generic aggregation functions
 *****************************************************************************/

/* 
 * Generic aggregate function for temporal sequences.
 * Returns new sequences that must be freed by the calling function.
 */

void
temporalseq_tagg1(TemporalSeq **result,
	TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*func)(Datum, Datum), bool crossings, int *newcount)
{
	Period *intersect = intersection_period_period_internal(&seq1->period, &seq2->period);
	if (intersect == NULL)
	{
		TemporalSeq *sequences[2];
		/* The two sequences do not intersect: copy the sequences in the right order */
		if (period_cmp_internal(&seq1->period, &seq2->period) < 0)
		{
			sequences[0] = seq1;
			sequences[1] = seq2;
		}
		else
		{
			sequences[0] = seq2;
			sequences[1] = seq1;
		}
		/* Normalization */
		int l;
		TemporalSeq **normsequences = temporalseqarr_normalize(sequences, 2, &l);
		for (int i = 0; i < l; i++)
			result[i] = normsequences[i];
		pfree(normsequences);
		*newcount = l;	
		return;
	}

	/* 
	 * If the two sequences intersect there will be at most 3 sequences in the
	 * result: one before the intersection, one for the intersection, and one 
	 * after the intersection. This will be also the case for discrete sequences
	 * (e.g., tint) that has the last value different from the previous one as
	 * tint '[1@2000-01-03, 2@2000-01-04]' and tint '[3@2000-01-01, 4@2000-01-05]'
	 * whose result for sum would be the following three sequences
	 * [3@2000-01-01, 3@2000-01-03), [4@2000-01-03, 5@2000-01-04], and
	 * (3@2000-01-04, 3@2000-01-05]
	 */
	Period period;
	TimestampTz lower1 = seq1->period.lower;
	TimestampTz upper1 = seq1->period.upper;
	bool lower1_inc = seq1->period.lower_inc;
	bool upper1_inc = seq1->period.upper_inc;

	TimestampTz lower2 = seq2->period.lower;
	TimestampTz upper2 = seq2->period.upper;
	bool lower2_inc = seq2->period.lower_inc;
	bool upper2_inc = seq2->period.upper_inc;

	TimestampTz lower = intersect->lower;
	TimestampTz upper = intersect->upper;
	bool lower_inc = intersect->lower_inc;
	bool upper_inc = intersect->upper_inc;
	TemporalSeq *sequences[3];
	int k = 0;

	/* Compute the aggregation on the period before the 
	 * intersection of the intervals */
	if (timestamp_cmp_internal(lower1, lower) < 0 ||
		(lower1_inc && !lower_inc && timestamp_cmp_internal(lower1, lower) == 0))
	{
		period_set(&period, lower1, lower, lower1_inc, !lower_inc);
		sequences[k++] = temporalseq_at_period(seq1, &period);
	}
	else if (timestamp_cmp_internal(lower2, lower) < 0 ||
		(lower2_inc && !lower_inc && timestamp_cmp_internal(lower2, lower) == 0))
	{
		period_set(&period, lower2, lower, lower2_inc, !lower_inc);
		sequences[k++] = temporalseq_at_period(seq2, &period);
	}
	
	/*
	 * Compute the aggregation on the intersection of intervals
	 */
	TemporalSeq *syncseq1, *syncseq2;
	synchronize_temporalseq_temporalseq(seq1, seq2, &syncseq1, &syncseq2, crossings);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * syncseq1->count);
	for (int i = 0; i < syncseq1->count; i++)
	{
		TemporalInst *inst1 = temporalseq_inst_n(syncseq1, i);
		TemporalInst *inst2 = temporalseq_inst_n(syncseq2, i);
		instants[i] = temporalinst_make(
			func(temporalinst_value(inst1), temporalinst_value(inst2)),
			inst1->t, inst1->valuetypid);
	}
	sequences[k++] = temporalseq_from_temporalinstarr(instants, syncseq1->count, 
		lower_inc, upper_inc, true);
	for (int i = 0; i < syncseq1->count; i++)
		pfree(instants[i]);
	pfree(instants); pfree(syncseq1); pfree(syncseq2);
	
	/* Compute the aggregation on the period after the intersection 
	 * of the intervals */
	if (timestamp_cmp_internal(upper, upper1) < 0 ||
		(!upper_inc && upper1_inc && timestamp_cmp_internal(upper, upper1) == 0))
	{
		period_set(&period, upper, upper1, !upper_inc, upper1_inc);
		sequences[k++] = temporalseq_at_period(seq1, &period);
	}
	else if (timestamp_cmp_internal(upper, upper2) < 0 ||
		(!upper_inc && upper2_inc && timestamp_cmp_internal(upper, upper2) == 0))
	{
		period_set(&period, upper, upper2, !upper_inc, upper2_inc);
		sequences[k++] = temporalseq_at_period(seq2, &period);
	}
	pfree(intersect); 

	/* Normalization */
	int l;
	TemporalSeq **normsequences = temporalseqarr_normalize(sequences, k, &l);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	for (int i = 0; i < l; i++)
		result[i] = normsequences[i];
	pfree(normsequences);
	*newcount = l;	
	return;
}

/* 
 * Generic aggregate function for temporal sequences.
 * Arguments:
 * - sequences1 is the accumulated state 
 * - sequences2 are the sequences of a TemporalS value
 * where both may be non contiguous
 * The function returns new sequences in the result that must
 * be freed by the calling function. 
 */
TemporalSeq **
temporalseq_tagg2(TemporalSeq **sequences1, int count1, TemporalSeq **sequences2, 
	int count2, Datum (*func)(Datum, Datum), bool crossings, int *newcount)
{
	/*
	 * Each sequence can be split 3 times, there may be count - 1 holes between
	 * sequences for both sequences 1 and sequences2, and there may be 
	 * 2 sequences before and after.
	 * TODO Verify this formula
	 */
	int seqcount = (count1 * 3) + count1 + count2 + 1;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seqcount);
	int i = 0, j = 0, k = 0, countstep;
	TemporalSeq *seq1 = sequences1[i];
	TemporalSeq *seq2 = sequences2[j];
	while (i < count1 && j < count2)
	{
		temporalseq_tagg1(&sequences[k], seq1, seq2, func, crossings,
			&countstep);
		k += countstep - 1;
		/* If both upper bounds are equal */
		if (timestamp_cmp_internal(seq1->period.upper, seq2->period.upper) == 0 &&
			seq1->period.upper_inc == seq2->period.upper_inc)
		{
			k++; i++; j++;
			if (i == count1 || j == count2)
				break;
			seq1 = sequences1[i];
			seq2 = sequences2[j];
		}
		/* If upper bound of seq1 is less than or equal to the upper bound of seq2 */
		else if (timestamp_cmp_internal(seq1->period.upper, seq2->period.upper) < 0 ||
			(!seq1->period.upper_inc && seq2->period.upper_inc &&
			timestamp_cmp_internal(seq1->period.upper, seq2->period.upper) == 0))
		{
			i++;
			if (i == count1)
			{
				k++; j++;
				break;				
			}
			seq1 = sequences1[i];
			seq2 = sequences[k];
		}
		else
		{
			j++;
			if (j == count2)
			{
				k++; i++;
				break;				
			}
			seq1 = sequences[k];
			seq2 = sequences2[j];
		}
	}
	while (i < count1)
		sequences[k++] = temporalseq_copy(sequences1[i++]);
	while (j < count2)
		sequences[k++] = temporalseq_copy(sequences2[j++]);

	/* Normalization */
	int l;
	TemporalSeq **result = temporalseqarr_normalize(sequences, k, &l);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	*newcount = l;	
	return result;
}

/*****************************************************************************
 * Generic aggregate transition functions
 *****************************************************************************/

SkipList *
temporalinst_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state,
	TemporalInst *inst, Datum (*func)(Datum, Datum))
{
	SkipList *result;
	if (! state)
		result = skiplist_make(fcinfo, (Temporal **)&inst, 1);
	else
	{
        if (skiplist_headval(state)->duration != TEMPORALINST)
            ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
                    errmsg("Cannot aggregate temporal values of different duration")));
		Period period_state2;
		period_set(&period_state2, inst->t, inst->t, true, true);
		skiplist_splice(fcinfo, state, (Temporal **)&inst, 1, &period_state2, 
			func, false);
		result = state;
	}
	return result;
}

static SkipList *
temporali_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state, 
	TemporalI *ti, Datum (*func)(Datum, Datum))
{
	TemporalInst **instants = temporali_instants(ti);
	SkipList *result;
	if (! state)
		result = skiplist_make(fcinfo, (Temporal **)instants, ti->count);
	else
	{
        if (skiplist_headval(state)->duration != TEMPORALINST)
            ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
                    errmsg("Cannot aggregate temporal values of different duration")));
		Period period_state2;
		period_set(&period_state2, instants[0]->t, instants[ti->count - 1]->t, true, true);
		skiplist_splice(fcinfo, state, (Temporal **)instants, ti->count, &period_state2, 
			func, false);
		result = state;
	}
	pfree(instants);
	return result;
}

SkipList *
temporalseq_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state, 
	TemporalSeq *seq, Datum (*func)(Datum, Datum), bool crossings)
{
	SkipList *result;
	if (! state)
		result = skiplist_make(fcinfo, (Temporal **)&seq, 1);
	else
	{
        if (skiplist_headval(state)->duration != TEMPORALSEQ)
            ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
                    errmsg("Cannot aggregate temporal values of different duration")));
		skiplist_splice(fcinfo, state, (Temporal **)&seq, 1, &seq->period, 
			func, crossings);
		result = state;
	}
	return result;
}

static SkipList *
temporals_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state, 
	TemporalS *ts, Datum (*func)(Datum, Datum), bool crossings)
{
	TemporalSeq **sequences = temporals_sequences(ts);
	SkipList *result;
	if (! state)
		result = skiplist_make(fcinfo, (Temporal **)sequences, ts->count);
	else
	{
        if (skiplist_headval(state)->duration != TEMPORALSEQ)
            ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
                    errmsg("Cannot aggregate temporal values of different duration")));
		Period period_state2;
		period_set(&period_state2, sequences[0]->period.lower, sequences[ts->count - 1]->period.upper,
			sequences[0]->period.lower_inc, sequences[ts->count - 1]->period.upper_inc);
		skiplist_splice(fcinfo, state, (Temporal **)sequences, ts->count, &period_state2, 
			func, crossings);
		result = state;
	}
	pfree(sequences);
	return result;
}

static SkipList *
temporal_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state, 
	Temporal *temp, Datum (*func)(Datum, Datum), bool crossings)
{
	temporal_duration_is_valid(temp->duration);
	SkipList *result = NULL;
	if (temp->duration == TEMPORALINST) 
		result =  temporalinst_tagg_transfn(fcinfo, state, (TemporalInst *)temp, 
			func);
	else if (temp->duration == TEMPORALI) 
		result =  temporali_tagg_transfn(fcinfo, state, (TemporalI *)temp, 
			func);
	else if (temp->duration == TEMPORALSEQ) 
		result =  temporalseq_tagg_transfn(fcinfo, state, (TemporalSeq *)temp, 
			func, crossings);
	else if (temp->duration == TEMPORALS) 
		result = temporals_tagg_transfn(fcinfo, state, (TemporalS *)temp, 
			func, crossings);
	return result;
}

/*****************************************************************************
 * Generic aggregate combine function for TemporalInst and TemporalSeq
 *****************************************************************************/

SkipList *
temporal_tagg_combinefn(FunctionCallInfo fcinfo, SkipList *state1, 
	SkipList *state2, Datum (*operator)(Datum, Datum), bool crossings)
{
	if (! state1)
		return state2;
	if (! state2)
		return state1;
	if (skiplist_headval(state1)->duration != skiplist_headval(state2)->duration)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Cannot aggregate temporal values of different duration")));

	//int count1 = state1->length;
	int count2 = state2->length;
	Temporal **values2 = skiplist_values(state2);
	Period period_state2;
	if (skiplist_headval(state1)->duration == TEMPORALINST)
		period_set(&period_state2, ((TemporalInst *)values2[0])->t, 
			((TemporalInst *)values2[count2-1])->t, true, true);
	else
		period_set(&period_state2, ((TemporalSeq *)values2[0])->period.lower, 
			((TemporalSeq *)values2[count2-1])->period.upper,
			((TemporalSeq *)values2[0])->period.lower_inc, 
			((TemporalSeq *)values2[count2-1])->period.upper_inc);
	skiplist_splice(fcinfo, state1, values2, count2, &period_state2, operator, 
		crossings);
	pfree(values2);
	return state1;
}

/*****************************************************************************
 * Temporal aggregate functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbool_tand_transfn);

PGDLLEXPORT Datum
tbool_tand_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	SkipList *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_and, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tbool_tand_combinefn);

PGDLLEXPORT Datum
tbool_tand_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL : 
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);
	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_and, false);

	if(result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tbool_tor_transfn);

PGDLLEXPORT Datum
tbool_tor_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	SkipList *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_or, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tbool_tor_combinefn);

PGDLLEXPORT Datum
tbool_tor_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL : 
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);
	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_or, false);

	if(result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tint_tmin_transfn);

PGDLLEXPORT Datum
tint_tmin_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	SkipList *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_min_int32, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tint_tmin_combinefn);

PGDLLEXPORT Datum
tint_tmin_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);
	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_min_int32, true);

	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tmin_transfn);

PGDLLEXPORT Datum
tfloat_tmin_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	SkipList *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_min_float8, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tmin_combinefn);

PGDLLEXPORT Datum
tfloat_tmin_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);
	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_min_float8, true);

	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tint_tmax_transfn);

PGDLLEXPORT Datum
tint_tmax_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	SkipList *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_max_int32, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tint_tmax_combinefn);

PGDLLEXPORT Datum
tint_tmax_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);
	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_max_int32, true);

	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tmax_transfn);

PGDLLEXPORT Datum
tfloat_tmax_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	SkipList *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_max_float8, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tmax_combinefn);

PGDLLEXPORT Datum
tfloat_tmax_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);
	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_max_float8, true);

	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tint_tsum_transfn);

PGDLLEXPORT Datum
tint_tsum_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	SkipList *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_sum_int32, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tint_tsum_combinefn);

PGDLLEXPORT Datum
tint_tsum_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);
	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_sum_int32, false);

	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tsum_transfn);

PGDLLEXPORT Datum
tfloat_tsum_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	SkipList *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_sum_float8, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tsum_combinefn);

PGDLLEXPORT Datum
tfloat_tsum_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);
	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_sum_float8, false);

	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(ttext_tmin_transfn);

PGDLLEXPORT Datum
ttext_tmin_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	SkipList *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_min_text, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttext_tmin_combinefn);

PGDLLEXPORT Datum
ttext_tmin_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);
	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_min_text, false);

	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttext_tmax_transfn);

PGDLLEXPORT Datum
ttext_tmax_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	SkipList *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_max_text, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttext_tmax_combinefn);

PGDLLEXPORT Datum
ttext_tmax_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);
	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_max_text, false);

	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for temporal count
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_tcount_transfn);

PGDLLEXPORT Datum 
temporal_tcount_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL : 
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}

	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *tempcount = temporal_transform_tcount(temp);
	SkipList *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_tagg_transfn(fcinfo, state, 
            (TemporalInst *)tempcount, &datum_sum_int32);
	else if (temp->duration == TEMPORALI)
		result = temporali_tagg_transfn(fcinfo, state, 
            (TemporalI *)tempcount, &datum_sum_int32);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_tagg_transfn(fcinfo, state, 
            (TemporalSeq *)tempcount, &datum_sum_int32, false);
	else if (temp->duration == TEMPORALS)
		result = temporals_tagg_transfn(fcinfo, state, 
            (TemporalS *)tempcount, &datum_sum_int32, false);
	pfree(tempcount);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_tcount_combinefn);

PGDLLEXPORT Datum 
temporal_tcount_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL : 
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);

	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2,
		&datum_sum_int32, false);

	if (result != state2)
		pfree(state2);

	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_tagg_finalfn);

PGDLLEXPORT Datum
temporal_tagg_finalfn(PG_FUNCTION_ARGS)
{
	/* The final function is strict, we do not need to test for null values */
	SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
	if (state->length == 0)
		PG_RETURN_NULL();

	Temporal **values = skiplist_values(state);
	Temporal *result = NULL;
	assert(values[0]->duration == TEMPORALINST || 
		values[0]->duration == TEMPORALSEQ);
	if (values[0]->duration == TEMPORALINST)
		result = (Temporal *)temporali_from_temporalinstarr(
			(TemporalInst **)values, state->length);
	else if (values[0]->duration == TEMPORALSEQ)
		result = (Temporal *)temporals_from_temporalseqarr(
			(TemporalSeq **)values, state->length, true);
	pfree(values);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for temporal average
 *****************************************************************************/

SkipList *
temporalinst_tavg_transfn(FunctionCallInfo fcinfo, SkipList *state, TemporalInst *inst)
{
	TemporalInst *newinst = tnumberinst_transform_tavg(inst);
	SkipList *result;
	if (! state)
		result = skiplist_make(fcinfo, (Temporal **)&newinst, 1);
	else
	{
		Period period_state2;
		period_set(&period_state2, inst->t, inst->t, true, true);
		skiplist_splice(fcinfo, state, (Temporal **)&newinst, 1, &period_state2, 
			&datum_sum_double2, false);
		result = state;
	}
	pfree(newinst);
	return result;
}

SkipList *
temporali_tavg_transfn(FunctionCallInfo fcinfo, SkipList *state, TemporalI *ti)
{
	TemporalInst **instants = tnumberi_transform_tavg(ti);
	SkipList *result;
	if (! state)
		result = skiplist_make(fcinfo, (Temporal **)instants, ti->count);
	else
	{
		Period period_state2;
		period_set(&period_state2, instants[0]->t, instants[ti->count - 1]->t, true, true);
		skiplist_splice(fcinfo, state, (Temporal **)instants, ti->count, &period_state2, 
			&datum_sum_double2, false);
		result = state;
	}
	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

SkipList *
temporalseq_tavg_transfn(FunctionCallInfo fcinfo, SkipList *state, TemporalSeq *seq)
{
	int maxcount = 0;
	number_base_type_oid(seq->valuetypid);
	if (seq->valuetypid == INT4OID)
		maxcount = seq->count;
	else if (seq->valuetypid == FLOAT8OID)
		maxcount = 1;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int count = tnumberseq_transform_tavg(sequences, seq);
	SkipList *result;
	if (! state)
		result = skiplist_make(fcinfo, (Temporal **)sequences, count);
	else
	{
		skiplist_splice(fcinfo, state, (Temporal **)sequences, count, &seq->period, 
			&datum_sum_double2, false);
		result = state;
	}
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

SkipList *
temporals_tavg_transfn(FunctionCallInfo fcinfo, SkipList *state, TemporalS *ts)
{
	int maxcount = 0;
	number_base_type_oid(ts->valuetypid);
	if (ts->valuetypid == INT4OID)
		maxcount = ts->totalcount;
	else if (ts->valuetypid == FLOAT8OID)
		maxcount = ts->count;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int count = tnumbers_transform_tavg(sequences, ts);
	SkipList *result;
	if (! state)
		result = skiplist_make(fcinfo, (Temporal **)sequences, count);
	else
	{
		Period period_state2;
		period_set(&period_state2, sequences[0]->period.lower, sequences[count - 1]->period.upper,
			sequences[0]->period.lower_inc, sequences[count - 1]->period.upper_inc);
		skiplist_splice(fcinfo, state, (Temporal **)sequences, count, &period_state2, 
			&datum_sum_double2, false);
		result = state;
	}
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_tavg_transfn);

PGDLLEXPORT Datum
temporal_tavg_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL : 
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1)) {
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}

	Temporal *temp = PG_GETARG_TEMPORAL(1);
	SkipList *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_tavg_transfn(fcinfo, state, (TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = temporali_tavg_transfn(fcinfo, state, (TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_tavg_transfn(fcinfo, state, (TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = temporals_tavg_transfn(fcinfo, state, (TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_tavg_combinefn);

PGDLLEXPORT Datum
temporal_tavg_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL : 
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);

	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2,
		&datum_sum_double2, false);

	if (result != state2)
		pfree(state2);

	PG_RETURN_POINTER(result);
}

/* Final function for tavg */

TemporalI *
temporalinst_tavg_finalfn(TemporalInst **instants, int count)
{
	TemporalInst **newinstants = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++)
	{
		TemporalInst *inst = instants[i];
		double2 *value = (double2 *)DatumGetPointer(temporalinst_value(inst));
		double tavg = value->a / value->b;
		newinstants[i] = temporalinst_make(Float8GetDatum(tavg), inst->t,
			FLOAT8OID);
	}
	TemporalI *result = temporali_from_temporalinstarr(newinstants, count);

	for (int i = 0; i < count; i++)
		pfree(newinstants[i]);
	pfree(newinstants);
	
	return result;
}

TemporalS *
temporalseq_tavg_finalfn(TemporalSeq **sequences, int count)
{
	TemporalSeq **newsequences = palloc(sizeof(TemporalSeq *) * count);
	for (int i = 0; i < count; i++)
	{
		TemporalSeq *seq = sequences[i];
		TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			double2 *value2 = (double2 *)DatumGetPointer(temporalinst_value(inst));
			double value = value2->a / value2->b;
			instants[j] = temporalinst_make(Float8GetDatum(value), inst->t,
				FLOAT8OID);
		}
		newsequences[i] = temporalseq_from_temporalinstarr(instants, 
			seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
		for (int j = 0; j < seq->count; j++)
			pfree(instants[j]);
		pfree(instants);
	}
	TemporalS *result = temporals_from_temporalseqarr(newsequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(newsequences[i]);
	pfree(newsequences);
	
	return result;
}

PG_FUNCTION_INFO_V1(temporal_tavg_finalfn);

PGDLLEXPORT Datum
temporal_tavg_finalfn(PG_FUNCTION_ARGS)
{
	/* The final function is strict, we do not need to test for null values */
	SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
	if (state->length == 0)
		PG_RETURN_NULL();

	Temporal **values = skiplist_values(state);
	Temporal *result = NULL;
	assert(values[0]->duration == TEMPORALINST || 
		values[0]->duration == TEMPORALSEQ);
	if (values[0]->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_tavg_finalfn(
			(TemporalInst **)values, state->length);
	else if (values[0]->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_tavg_finalfn(
			(TemporalSeq **)values, state->length);
	pfree(values);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
