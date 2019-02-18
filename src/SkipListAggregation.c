#include "Aggregates.h"


/******************/

#define SKIPLIST_MAXLEVEL 32
#define SKIPLIST_INITIAL_CAPACITY 1024
#define SKIPLIST_GROW 2
#define SKIPLIST_INITIAL_FREELIST 32

typedef struct {
    TemporalSeq* value ;
    int height ;
    int next[SKIPLIST_MAXLEVEL] ;
} Elem ;

typedef struct {
    int capacity ;
    int next ;
    int length ;
    int* freed ;
    int freecount ;
    int freecap ;
    int tail ;
    Elem* elems ;
} SkipList ;

MemoryContext set_aggregation_context(FunctionCallInfo fcinfo) {
    MemoryContext ctx;
    if (!AggCheckCallContext(fcinfo, &ctx))
        ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
                errmsg("Operation not supported")));
    return  MemoryContextSwitchTo(ctx) ;
}

void unset_aggregation_context(MemoryContext ctx) {
    MemoryContextSwitchTo(ctx) ;
}

SkipList* skiplist_make(FunctionCallInfo fcinfo, TemporalSeq** values, int count) {
    //FIXME: tail should be a constant (e.g. 1) but is not, for ease of construction

    MemoryContext oldctx = set_aggregation_context(fcinfo) ;
    int capacity = SKIPLIST_INITIAL_CAPACITY ;
    count += 2 ; // account for head & tail
    while(capacity <= count)
        capacity <<= 1 ;
    SkipList* result = palloc0(sizeof(SkipList)) ;
    result->elems = palloc0(capacity * sizeof(Elem)) ;
    int height = (int) ceil(log2(count-1)) ;
    result->capacity = capacity ;
    result->next = count ;
    result->length = count - 2 ;

    // fill values first:
    result->elems[0].value = NULL ;
    for(int i = 0 ; i < count-2 ; i ++) {
        result->elems[i+1].value = temporalseq_copy(values[i]) ;
    }
    result->elems[count-1].value = NULL ;
    result->tail = count-1 ;

    // link the list in a balanced fashion
    for(int level = 0 ; level < height ; level ++) {
        int step = 1 << level ;
        for(int i = 0 ; i < count ; i += step) {
            int next = i + step < count ? i + step : count - 1 ;
            if(i != count - 1) {
                result->elems[i].next[level] = next ;
                result->elems[i].height = level + 1 ;
            } else {
                result->elems[i].next[level] = - 1 ;
                result->elems[i].height = height ;
            }
        }
    }

    unset_aggregation_context(oldctx) ;
    return result ;
}

TemporalSeq* skiplist_headval(SkipList* list) {
    return list->elems[list->elems[0].next[0]].value ;
}

TemporalSeq* skiplist_tailval(SkipList* list) {
    // despite the look, this is pretty much O(1)
    int cur = 0 ;
    Elem* e = &list->elems[cur] ;
    int height = e->height ;
    while(e->next[height-1] != list->tail)
        e = &list->elems[e->next[height-1]] ;
    return e->value ;
}

int skiplist_alloc(FunctionCallInfo fcinfo, SkipList* list) {
    list->length ++ ;
    if(! list->freecount) {
        // no free list, give first available entry
        if(list->next >= list->capacity){
            // no more capacity, let's grow
            list->capacity <<= SKIPLIST_GROW ;
            MemoryContext ctx = set_aggregation_context(fcinfo) ;
            list->elems = repalloc(list->elems, list->capacity * sizeof(Elem)) ;
            unset_aggregation_context(ctx) ;
        }
        list->next ++ ;
        return list->next - 1 ;
    } else {
        list->freecount -- ;
        return list->freed[list->freecount] ;
    }
}

void skiplist_free(FunctionCallInfo fcinfo, SkipList* list, int cur) {
    if(! list->freed) {
        list->freecap = SKIPLIST_INITIAL_FREELIST ;
        MemoryContext ctx = set_aggregation_context(fcinfo) ;
        list->freed = palloc(list->freecap * sizeof(int));
        unset_aggregation_context(ctx) ;
    } else if(list->freecount == list->freecap) {
        list->freecap <<= 1 ;
        MemoryContext ctx = set_aggregation_context(fcinfo) ;
        list->freed = repalloc(list->freed, list->freecap * sizeof(int)) ;
        unset_aggregation_context(ctx) ;
    }

    list->freed[list->freecount ++] = cur ;
    list->length -- ;
}

typedef enum {
    BEFORE,
    DURING,
    AFTER
} RelativeTimePos ;

RelativeTimePos pos_period_timestamp(Period *p, TimestampTz t)
{
    int32		cmp;

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

// the comparison function used for skiplists
RelativeTimePos skiplist_elmpos(SkipList* list, int cur, TimestampTz t) {
    if(cur == 0)
        return AFTER ; // head is -inf
    else if(cur == -1 || cur == list->tail)
        return BEFORE ; // tail is +inf
    else
        return pos_period_timestamp(&list->elems[cur].value->period, t) ;
}

static TemporalSeq** skiplist_values(SkipList* list) {
    TemporalSeq** result = palloc(list->length * sizeof(TemporalSeq*)) ;
    int cur = list->elems[0].next[0] ;
    int count = 0 ;
    while(cur != list->tail) {
        result[count++] = list->elems[cur].value ;
        cur = list->elems[cur].next[0] ;
    }
    return result ;
}

// outputs the skiplist in graphviz dot format for visualisation & debugging purposes
static void skiplist_print(SkipList* list) {
    int len = 0 ;
    char buf[16384] ;
    len += sprintf(buf+len, "digraph skiplist {\n") ;
    len += sprintf(buf+len, "\trankdir = LR ;\n") ;
    len += sprintf(buf+len, "\tnode [shape = record] ;\n") ;
    int cur = 0 ;
    while(cur != -1){
        Elem* e = &list->elems[cur] ;
        len += sprintf(buf+len, "\telm%d [label=\"", cur) ;
        for(int l = e->height - 1 ; l > 0 ; l --) {
            len += sprintf(buf+len, "<p%d>|", l) ;
        }
        if(! e->value)
            len += sprintf(buf+len, "<p0>\"] ;\n") ;
        else
            len += sprintf(buf+len, "<p0>%f\"] ;\n", DatumGetFloat8(temporalseq_min_value(e->value))) ;
        if(e->next[0] != -1) {
            for(int l = 0 ; l < e->height ; l ++) {
                int next = e->next[l] ;
                len += sprintf(buf+len, "\telm%d:p%d -> elm%d:p%d ", cur, l, next, l) ;
                if(l == 0)
                    len += sprintf(buf+len, "[weight=100] ;\n") ;
                else
                    len += sprintf(buf+len, ";\n") ;
            }
        }
        cur = e->next[0] ;
    }
    sprintf(buf+len, "}\n") ;
    ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("SKIPLIST: %s", buf))) ;
}


int random_level() {
    // this simulates up to SKIPLIST_MAXLEVEL repeated coin flips without spinning the RNG every time (courtesy of the internet)
    return ffsl(~(random() & ((1l << SKIPLIST_MAXLEVEL) - 1))) ;
}


void skiplist_splice(FunctionCallInfo fcinfo, SkipList* list, TemporalSeq** values, int count, Period* period, Datum (*operator)(Datum, Datum), bool crossings) {
    // O(count*log(n)) average (unless I'm mistaken)
    // O(n+count*log(n)) worst case (when period spans the whole list so everything has to be deleted)
    int update[SKIPLIST_MAXLEVEL] ;
    memset(update, 0, sizeof(update)) ;
    int cur = 0 ;
    int height = list->elems[cur].height ;
    Elem* e = &list->elems[cur] ;
    for(int level = height-1 ; level >= 0 ; level --) {
        while(e->next[level] != -1 && skiplist_elmpos(list, e->next[level], period->lower) == AFTER) {
            cur = e->next[level] ;
            e = &list->elems[cur] ;
        }
        update[level] = cur ;
    }

    int lower = e->next[0] ;
    cur = lower ;
    e = &list->elems[cur] ;

    int spliced_count = 0 ;
    while(skiplist_elmpos(list, cur, period->upper) == AFTER) {
        cur = e->next[0] ;
        e = &list->elems[cur] ;
        spliced_count ++ ;
    }
    int upper = cur ;
    if(upper >= 0 && skiplist_elmpos(list, upper, period->upper) == DURING) {
        upper = e->next[0] ; // if found upper, one more to remove
        spliced_count ++ ;
    }

    // delete spliced-out elements but remember their values for later
    cur = lower ;
    TemporalSeq** spliced = palloc(spliced_count * sizeof(TemporalSeq*)) ;
    spliced_count = 0 ;
    while(cur != upper && cur != -1) {
        for(int level = 0 ; level < height ; level ++) {
            Elem* prev = &list->elems[update[level]] ;
            if(prev->next[level] != cur)
                break ;

            prev->next[level] = list->elems[cur].next[level] ;
        }
        spliced[spliced_count++] = list->elems[cur].value ;
        skiplist_free(fcinfo, list, cur) ;
        cur = list->elems[cur].next[0] ;
    }

    // level down head & tail if necessary
    Elem* head = &list->elems[0] ;
    Elem* tail = &list->elems[list->tail] ;
    while(head->height > 1 && head->next[head->height-1] == list->tail) {
        head->height -- ;
        tail->height -- ;
        height -- ;
    }

    if(spliced_count != 0) {
        // we're not in a gap -- we need to compute the aggregation
        int newcount = 0 ;
        TemporalSeq **newseqs = temporalseq_tagg2(spliced, spliced_count,
                                                  values, count, operator, crossings, &newcount) ;
        values = newseqs ;
        count = newcount ;
        // we need to delete the spliced-out sequences
        for(int i = 0 ; i < spliced_count ; i ++)
            pfree(spliced[i]) ;
    }

    // insert new elements
    for(int i = count - 1 ; i >= 0; i --) {
        int rheight = random_level() ;
        if(rheight > height) {
            for(int l = height ; l < rheight ; l ++)
                update[l] = 0 ;
            // grow head & tail as appropriate
            head->height = rheight ;
            tail->height = rheight ;
        }
        int new = skiplist_alloc(fcinfo, list) ;
        Elem* newelm = &list->elems[new] ;
        MemoryContext ctx = set_aggregation_context(fcinfo) ;
        newelm->value = temporalseq_copy(values[i]) ;
        unset_aggregation_context(ctx) ;
        newelm->height = rheight ;

        for(int level = 0 ; level < rheight ; level ++) {
            newelm->next[level] = list->elems[update[level]].next[level] ;
            list->elems[update[level]].next[level] = new ;
            if(level >= height && update[0] != list->tail) {
                newelm->next[level] = list->tail ;
            }
        }
        if(rheight > height)
            height = rheight ;
    }
}

PG_FUNCTION_INFO_V1(sl_test) ;

Datum sl_test(PG_FUNCTION_ARGS) {
    ArrayType* array = PG_GETARG_ARRAYTYPE_P(0) ;
    int count = -1 ;
    //int count2 = -1 ;
    //Period* p = PG_GETARG_PERIOD(1) ;
    //ArrayType* array2 = PG_GETARG_ARRAYTYPE_P(2) ;
    TemporalSeq** seqs = (TemporalSeq**) temporalarr_extract(array, &count) ;
    //TemporalSeq** seqs2 = (TemporalSeq**) temporalarr_extract(array2, &count2) ;
    SkipList* sl = skiplist_make(fcinfo, seqs, count) ;
    skiplist_print(sl) ;
    //skiplist_splice(fcinfo, sl, seqs2, count2, p) ;
    //skiplist_print(sl) ;
    PG_RETURN_INT32(0) ;
}


/***********************************************************/

#define LEAVE_FOR_LATER
#ifdef LEAVE_FOR_LATER

/*
 * Transform a temporal number type into a temporal double2 type for
 * performing average aggregation
 */

static TemporalInst *
tnumberinst_transform_tavg2(TemporalInst *inst)
{
    float8 value = datum_double(temporalinst_value(inst), inst->valuetypid);
    double2 *dvalue = double2_construct(value, 1);
    return temporalinst_make(PointerGetDatum(dvalue), inst->t,
                             type_oid(T_DOUBLE2));
}

static TemporalSeq*
tfloatseq_transform_tavg2(TemporalSeq *seq)
{
    TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
    for (int i = 0; i < seq->count; i++)
    {
        TemporalInst *inst = temporalseq_inst_n(seq, i);
        instants[i] = tnumberinst_transform_tavg2(inst);
    }
    TemporalSeq* result = temporalseq_from_temporalinstarr(instants,
                                                           seq->count,	seq->period.lower_inc, seq->period.upper_inc, false);

    for (int i = 0; i < seq->count; i++)
        pfree(instants[i]);
    pfree(instants);

    return result;
}

static TemporalSeq *
tnumberseq_transform_tavg2(TemporalSeq *seq)
{
    /*   if (seq->valuetypid == INT4OID)
           return tintseq_transform_tavg2(seq, count);*/
    if (seq->valuetypid == FLOAT8OID)
        return tfloatseq_transform_tavg2(seq);
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
            errmsg("Operation not supported")));
}


/*
 * Generic aggregate combine function for TemporalSeq
 */
SkipList* temporalseq_tagg_combinefn2(FunctionCallInfo fcinfo, SkipList* state1, TemporalSeq** state2, int count2,
                                            Datum (*operator)(Datum, Datum), bool crossings)
{
    if (!state1)
        return skiplist_make(fcinfo, state2, count2) ;
    if (count2 == 0)
        return state1;

    if (skiplist_headval(state1)->type != TEMPORALSEQ || state2[0]->type != TEMPORALSEQ)
        ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
                errmsg("Cannot aggregate temporal values of different duration")));

    Period period_state2;
    period_set(&period_state2,
               state2[0]->period.lower, state2[count2-1]->period.upper,
               state2[0]->period.lower_inc, state2[count2-1]->period.upper_inc);

    skiplist_splice(fcinfo, state1, state2, count2, &period_state2, operator, crossings) ;

    return state1 ;
}


SkipList*
temporalseq_tavg_transfn2(FunctionCallInfo fcinfo, TemporalSeq *seq, SkipList* state)
{
    //int count;
    TemporalSeq* newseq = tnumberseq_transform_tavg2(seq) ;

    SkipList* result = temporalseq_tagg_combinefn2(fcinfo, state, &newseq, 1,
                                                         &datum_sum_double2, false);

    if(state && result != state)
        pfree(state) ;

    pfree(newseq) ;

    return result;
}

PG_FUNCTION_INFO_V1(temporal_tavg_transfn2);

PGDLLEXPORT Datum
temporal_tavg_transfn2(PG_FUNCTION_ARGS)
{
    SkipList* state ;
    if(PG_ARGISNULL(0))
        state = NULL ;
    else
        state = (SkipList*) PG_GETARG_POINTER(0) ;

    Temporal *temp = PG_GETARG_TEMPORAL(1);
    SkipList* result = NULL ;
    if (temp->type == TEMPORALSEQ)
        result = temporalseq_tavg_transfn2(fcinfo, (TemporalSeq *)temp, state);
    else
        ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
                errmsg("Operation not supported")));
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_POINTER(result);
}


static SkipList*
temporal_tagg_combinefn2(FunctionCallInfo fcinfo, SkipList* state1, SkipList* state2,
                         Datum (*operator)(Datum, Datum), bool crossings)
{
    if (!state1)
        return state2;
    if (!state2)
        return state1;

    //int count1 = state1->length ;
    int count2 = state2->length ;
    TemporalSeq** vals2 = skiplist_values(state2) ;

    /* Get a pointer to the first element of the first array */
    Temporal *temp = (Temporal*) skiplist_headval(state1) ;
    if (temp->type == TEMPORALSEQ)
        return temporalseq_tagg_combinefn2(fcinfo, state1, vals2, count2, operator, crossings);
    else
        ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
                errmsg("Operation not supported")));
}

PG_FUNCTION_INFO_V1(temporal_tavg_combinefn2);

PGDLLEXPORT Datum
temporal_tavg_combinefn2(PG_FUNCTION_ARGS)
{
    SkipList* state1 ;
    if(PG_ARGISNULL(0))
        state1 = NULL ;
    else
        state1 = (SkipList*) PG_GETARG_POINTER(0) ;

    SkipList* state2 ;
    if(PG_ARGISNULL(1))
        state2 = NULL ;
    else
        state2 = (SkipList*) PG_GETARG_POINTER(1) ;

    SkipList* result = temporal_tagg_combinefn2(fcinfo, state1, state2,
                                                      &datum_sum_double2, false);

    if(result != state1)
        pfree(state1) ;
    if(result != state2)
        pfree(state2) ;

    PG_RETURN_POINTER(result);
}


PG_FUNCTION_INFO_V1(temporal_tavg_finalfn2);

PGDLLEXPORT Datum
temporal_tavg_finalfn2(PG_FUNCTION_ARGS)
{
    if(PG_ARGISNULL(0))
        PG_RETURN_NULL();

    SkipList* state = (SkipList*) PG_GETARG_POINTER(0);
    TemporalSeq** values = skiplist_values(state) ;

    Temporal *result = NULL ;

    if (values[0]->type == TEMPORALSEQ)
        result = (Temporal *)temporalseq_tavg_finalfn(values, state->length);
    else
        ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
                errmsg("Operation not supported")));

    PG_RETURN_POINTER(result);
}

#endif