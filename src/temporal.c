/*****************************************************************************
 *
 * temporal.c
 *	Basic functions for temporal types of any duration.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal.h"

#include <assert.h>
#include <access/heapam.h>
#include <access/htup_details.h>
#include <access/tuptoaster.h>
#include <catalog/namespace.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/fmgroids.h>
#include <utils/lsyscache.h>
#include <utils/rel.h>
#include <utils/timestamp.h>

#include "period.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "temporal_boxops.h"
#include "temporal_parser.h"
#include "rangetypes_ext.h"
#include "temporal.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Typmod 
 *****************************************************************************/

/**
 * @brief Array storing the string representation of the durations of 
 *		temporal types
 */
static char *temporalDurationName[] =
{
	"Unknown",
	"Instant",
	"InstantSet",
	"Sequence",
	"SequenceSet"
};

/**
 * @brief Array storing the mapping between the string representation of the 
 *		durations of the temporal types and the corresponding enum value
 */
struct temporal_duration_struct temporal_duration_struct_array[] =
{
	{"UNKNOWN", TEMPORAL},
	{"INSTANT", TEMPORALINST},
	{"INSTANTSET", TEMPORALI},
	{"SEQUENCE", TEMPORALSEQ},
	{"SEQUENCESET", TEMPORALS},
};

/**
 * @brief Returns the string representation of the duration of the 
 *		temporal type corresponding to the enum value
 */
const char *
temporal_duration_name(int16 duration)
{
	if (duration < 0 || duration > 4)
		return "Invalid duration for temporal type";
	return temporalDurationName[duration];
}

/**
 * @brief Returns the enum value corresponding to the string representation 
 *		of the duration of the temporal type.
 */
bool
temporal_duration_from_string(const char *str, int16 *duration)
{
	char *tmpstr;
	size_t tmpstartpos, tmpendpos;
	size_t i;

	/* Initialize */
	*duration = 0;
	/* Locate any leading/trailing spaces */
	tmpstartpos = 0;
	for (i = 0; i < strlen(str); i++)
	{
		if (str[i] != ' ')
		{
			tmpstartpos = i;
			break;
		}
	}
	tmpendpos = strlen(str) - 1;
	for (i = strlen(str) - 1; i != 0; i--)
	{
		if (str[i] != ' ')
		{
			tmpendpos = i;
			break;
		}
	}
	tmpstr = palloc(tmpendpos - tmpstartpos + 2);
	for (i = tmpstartpos; i <= tmpendpos; i++)
		tmpstr[i - tmpstartpos] = str[i];
	/* Add NULL to terminate */
	tmpstr[i - tmpstartpos] = '\0';
	size_t len = strlen(tmpstr);
	/* Now check for the type */
	for (i = 0; i < DURATION_STRUCT_ARRAY_LEN; i++)
	{
		if (len == strlen(temporal_duration_struct_array[i].durationName) &&
			!strcasecmp(tmpstr, temporal_duration_struct_array[i].durationName))
		{
			*duration = temporal_duration_struct_array[i].duration;
			pfree(tmpstr);
			return true;
		}
	}
	pfree(tmpstr);
	return false;
}

/**
 * @brief Ensures that the duration of the temporal value corresponds to the typmod
 */
static Temporal *
temporal_valid_typmod(Temporal *temp, int32_t typmod)
{
	/* No typmod (-1) */
	if (typmod < 0)
		return temp;
	int32 typmod_duration = TYPMOD_GET_DURATION(typmod);
	/* Typmod has a preference */
	if (typmod_duration > 0 && typmod_duration != temp->duration)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Temporal type (%s) does not match column type (%s)",
			temporal_duration_name(temp->duration), temporal_duration_name(typmod_duration))));
	return temp;
}

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/**
 * @brief Returns the temporal instant at the timestamp for timestamps that
 *		are at an exclusive bound
 */
TemporalInst *
temporalseq_find_timestamp_excl(const TemporalSeq *seq, TimestampTz t)
{
	TemporalInst *result;
	if (t == seq->period.lower)
		result = temporalseq_inst_n(seq, 0);
	else
		result = temporalseq_inst_n(seq, seq->count - 1);
	return temporalinst_copy(result);
}

/**
 * @brief Returns the temporal instant at the timestamp when the timestamp is
 *		at an exclusive bound
 */
TemporalInst *
temporals_find_timestamp_excl(const TemporalS *ts, TimestampTz t)
{
	TemporalInst *result;
	int loc;
	temporals_find_timestamp(ts, t, &loc);
	TemporalSeq *seq1, *seq2;
	if (loc == 0)
	{
		seq1 = temporals_seq_n(ts, 0);
		result = temporalseq_inst_n(seq1, 0);
	}
	else if (loc == ts->count)
	{
		seq1 = temporals_seq_n(ts, ts->count - 1);
		result = temporalseq_inst_n(seq1, seq1->count - 1);
	}
	else
	{
		seq1 = temporals_seq_n(ts, loc - 1);
		seq2 = temporals_seq_n(ts, loc);
		if (temporalseq_end_timestamp(seq1) == t)
			result = temporalseq_inst_n(seq1, seq1->count - 1);
		else
			result = temporalseq_inst_n(seq2, 0);
	}
	return temporalinst_copy(result);
}

/**
 * @brief Returns a copy of the temporal value
 */
Temporal *
temporal_copy(const Temporal *temp)
{
	Temporal *result = (Temporal *)palloc0(VARSIZE(temp));
	memcpy(result, temp, VARSIZE(temp));
	return result;
}

/**
 * @brief Temporally intersect the two temporal values
 * @param[in] temp1,temp2 Input values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the values do not overlap on time
 */
bool
intersection_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
	Temporal **inter1, Temporal **inter2)
{
	bool result = false;
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALINST) 
		result = intersection_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)inter1, (TemporalInst **)inter2);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI) 
		result = intersection_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, 
			(TemporalInst **)inter1, (TemporalInst **)inter2);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ) 
		result = intersection_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, 
			(TemporalInst **)inter1, (TemporalInst **)inter2);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS) 
		result = intersection_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, 
			(TemporalInst **)inter1, (TemporalInst **)inter2);
	
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALINST) 
		result = intersection_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)inter1, (TemporalInst **)inter2);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALI) 
		result = intersection_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2,
			(TemporalI **)inter1, (TemporalI **)inter2);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ) 
		result = intersection_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2,
			(TemporalI **)inter1, (TemporalI **)inter2);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS) 
		result = intersection_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, 
			(TemporalI **)inter1, (TemporalI **)inter2);
	
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALINST) 
		result = intersection_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)inter1, (TemporalInst **)inter2);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALI) 
		result = intersection_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2,
			(TemporalI **)inter1, (TemporalI **)inter2);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALSEQ) 
		result = intersection_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2,
			(TemporalSeq **)inter1, (TemporalSeq **)inter2);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS) 
		result = intersection_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			(TemporalS **)inter1, (TemporalS **)inter2);
	
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALINST) 
		result = intersection_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)inter1, (TemporalInst **)inter2);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALI) 
		result = intersection_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2,
			(TemporalI **)inter1, (TemporalI **)inter2);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALSEQ) 
		result = intersection_temporals_temporalseq(
			(TemporalS *)temp1, (TemporalSeq *)temp2,
			(TemporalS **)inter1, (TemporalS **)inter2);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALS) 
		result = intersection_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			(TemporalS **)inter1, (TemporalS **)inter2);

	return result;
}

/**
 * @brief Synchronize the two temporal values
 * @param[in] temp1,temp2 Input values
 * @param[out] sync1,sync2 Synchronized values
 * @param[in] crossings States whether turning points are added between segments
 * @return Returns false if the values do not overlap on time
 */
bool
synchronize_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
	Temporal **sync1, Temporal **sync2, bool crossings)
{
	bool result = false;
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALINST) 
		result = intersection_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)sync1, (TemporalInst **)sync2);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI) 
		result = intersection_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, 
			(TemporalInst **)sync1, (TemporalInst **)sync2);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ) 
		result = intersection_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, 
			(TemporalInst **)sync1, (TemporalInst **)sync2);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS) 
		result = intersection_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, 
			(TemporalInst **)sync1, (TemporalInst **)sync2);
	
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALINST) 
		result = intersection_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)sync1, (TemporalInst **)sync2);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALI) 
		result = intersection_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2,
			(TemporalI **)sync1, (TemporalI **)sync2);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ) 
		result = intersection_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2,
			(TemporalI **)sync1, (TemporalI **)sync2);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS) 
		result = intersection_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, 
			(TemporalI **)sync1, (TemporalI **)sync2);
	
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALINST) 
		result = intersection_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)sync1, (TemporalInst **)sync2);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALI) 
		result = intersection_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2,
			(TemporalI **)sync1, (TemporalI **)sync2);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALSEQ) 
		result = synchronize_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2,
			(TemporalSeq **)sync1, (TemporalSeq **)sync2, crossings);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS) 
		result = synchronize_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			(TemporalS **)sync1, (TemporalS **)sync2, crossings);
	
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALINST) 
		result = intersection_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)sync1, (TemporalInst **)sync2);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALI) 
		result = intersection_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2,
			(TemporalI **)sync1, (TemporalI **)sync2);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALSEQ) 
		result = synchronize_temporals_temporalseq(
			(TemporalS *)temp1, (TemporalSeq *)temp2,
			(TemporalS **)sync1, (TemporalS **)sync2, crossings);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALS) 
		result = synchronize_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			(TemporalS **)sync1, (TemporalS **)sync2, crossings);

	return result;
}

/**
 * @brief Returns true if the Oid corresponds to a base type that allows 
 *		linear interpolation
 */
bool
linear_interpolation(Oid type)
{
	if (type == FLOAT8OID || type == type_oid(T_DOUBLE2) || 
		type == type_oid(T_DOUBLE3) || type == type_oid(T_DOUBLE4) ||
		type == type_oid(T_GEOGRAPHY) || type == type_oid(T_GEOMETRY))
		return true;
	return false;
}

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

/**
 * @brief Returns the Oid of the base type from the Oid of the temporal type
 */
Oid
temporal_valuetypid(Oid temptypid)
{
	Oid catalog = RelnameGetRelid("pg_temporal");
	Relation rel = heap_open(catalog, AccessShareLock);
	TupleDesc tupDesc = rel->rd_att;
	ScanKeyData scandata;
	ScanKeyInit(&scandata, 1, BTEqualStrategyNumber, F_OIDEQ, 
		ObjectIdGetDatum(temptypid));
#if MOBDB_PGSQL_VERSION >= 120000
		TableScanDesc scan = table_beginscan_catalog(rel, 1, &scandata);
#else
		HeapScanDesc scan = heap_beginscan_catalog(rel, 1, &scandata);
#endif
	HeapTuple tuple = heap_getnext(scan, ForwardScanDirection);
	bool isnull = false;
	Oid result;
	if (HeapTupleIsValid(tuple)) 
		result = DatumGetObjectId(heap_getattr(tuple, 2, tupDesc, &isnull));
	heap_endscan(scan);
	heap_close(rel, AccessShareLock);
	if (! HeapTupleIsValid(tuple) || isnull) 
		elog(ERROR, "type %u is not a temporal type", temptypid);
	return result;
}

/*****************************************************************************
 * Oid functions
 *****************************************************************************/

/**
 * @brief Returns the Oid of the range type corresponding to the Oid of the 
 *		base type
 */
Oid
range_oid_from_base(Oid valuetypid)
{
	Oid result = 0;
	ensure_numeric_base_type(valuetypid);
	if (valuetypid == INT4OID)
		result = type_oid(T_INTRANGE);
	else if (valuetypid == FLOAT8OID)
		result = type_oid(T_FLOATRANGE);
	return result;
}

/**
 * @brief Returns the Oid of the temporal type corresponding to the Oid of the
 *		base type
 */
Oid
temporal_oid_from_base(Oid valuetypid)
{
	Oid result = 0;
	ensure_temporal_base_type(valuetypid);
	if (valuetypid == BOOLOID) 
		result = type_oid(T_TBOOL);
	if (valuetypid == INT4OID) 
		result = type_oid(T_TINT);
	if (valuetypid == FLOAT8OID) 
		result = type_oid(T_TFLOAT);
	if (valuetypid == TEXTOID) 
		result = type_oid(T_TTEXT);
	if (valuetypid == type_oid(T_GEOMETRY))
		result = type_oid(T_TGEOMPOINT);
	if (valuetypid == type_oid(T_GEOGRAPHY)) 
		result = type_oid(T_TGEOGPOINT);
	return result;
}

/**
 * @brief Returns true if the Oid is a temporal type
 * @note Function used in particular in the indexes
 */
bool
temporal_type_oid(Oid temptypid)
{
	if (temptypid == type_oid(T_TBOOL) || temptypid == type_oid(T_TINT) ||
		temptypid == type_oid(T_TFLOAT) || temptypid == type_oid(T_TTEXT) ||
		temptypid == type_oid(T_TGEOMPOINT) ||
		temptypid == type_oid(T_TGEOGPOINT)
		)
		return true;
	return false;
}

/**
 * @brief Returns true if the Oid is a temporal numeric type
 * @note Function used in particular in the indexes
 */
bool
tnumber_type_oid(Oid temptypid)
{
	if (temptypid == type_oid(T_TINT) || temptypid == type_oid(T_TFLOAT))
		return true;
	return false;
}

/**
 * @brief Returns true if the Oid is a temporal point type
 * @note Function used in particular in the indexes
 */
bool
tpoint_type_oid(Oid temptypid)
{
	if (temptypid == type_oid(T_TGEOMPOINT) ||
		temptypid == type_oid(T_TGEOGPOINT))
		return true;
	return false;
}

/**
 * @brief Returns the Oid of the base type corresponding to the Oid of the
 *		temporal type
 */
Oid
base_oid_from_temporal(Oid temptypid)
{
	assert(temporal_type_oid(temptypid));
	Oid result = 0;
	if (temptypid == type_oid(T_TBOOL)) 
		result = BOOLOID;
	else if (temptypid == type_oid(T_TINT)) 
		result = INT4OID;
	else if (temptypid == type_oid(T_TFLOAT)) 
		result = FLOAT8OID;
	else if (temptypid == type_oid(T_TTEXT)) 
		result = TEXTOID;
	else if (temptypid == type_oid(T_TGEOMPOINT))
		result = type_oid(T_GEOMETRY);
	else if (temptypid == type_oid(T_TGEOGPOINT)) 
		result = type_oid(T_GEOGRAPHY);
	return result;
}

/*****************************************************************************
 * Trajectory functions
 *****************************************************************************/

/**
 * @brief Returns true if the temporal type corresponding to the Oid of the 
 *		base type has its trajectory precomputed
 */
bool
type_has_precomputed_trajectory(Oid valuetypid) 
{
	if (valuetypid == type_oid(T_GEOMETRY) ||
		valuetypid == type_oid(T_GEOGRAPHY))
		return true;
	return false;
} 
 
/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensures that the duration is a valid duration
 * @note Used for the dispatch functions
 */
void 
ensure_valid_duration(int16 duration)
{
	if (duration != TEMPORALINST && duration != TEMPORALI && 
		duration != TEMPORALSEQ && duration != TEMPORALS)
		elog(ERROR, "unknown duration for temporal type: %d", duration);
}

/**
 * @brief Ensures that the duration is a valid duration
 * @note Used for the analyze and selectivity functions
 */
void 
ensure_valid_duration_all(int16 duration)
{
	if (duration != TEMPORAL && 
		duration != TEMPORALINST && duration != TEMPORALI && 
		duration != TEMPORALSEQ && duration != TEMPORALS)
		elog(ERROR, "unknown duration for temporal type: %d", duration);
}

/**
 * @brief Ensures that the Oid is a range type
 */
void 
ensure_numrange_type(Oid typid)
{
	if (typid != type_oid(T_INTRANGE) && typid != type_oid(T_FLOATRANGE))
		elog(ERROR, "unknown numeric range type: %d", typid);
}

/**
 * @brief Ensures that the Oid is an external base type supported by MobilityDB
 */
void
ensure_temporal_base_type(Oid valuetypid)
{
	if (valuetypid != BOOLOID && valuetypid != INT4OID && 
		valuetypid != FLOAT8OID && valuetypid != TEXTOID &&
		valuetypid != type_oid(T_GEOMETRY) &&
		valuetypid != type_oid(T_GEOGRAPHY))
		elog(ERROR, "unknown base type: %d", valuetypid);
}

/**
 * @brief Ensures that the Oid is an external or an internal base type 
 *		supported by MobilityDB
 */
void
ensure_temporal_base_type_all(Oid valuetypid)
{
	if (valuetypid != BOOLOID && valuetypid != INT4OID && 
		valuetypid != FLOAT8OID && valuetypid != TEXTOID &&
		valuetypid != TIMESTAMPTZOID && valuetypid != type_oid(T_DOUBLE2) &&
		valuetypid != type_oid(T_GEOMETRY) &&
		valuetypid != type_oid(T_GEOGRAPHY) &&
		valuetypid != type_oid(T_DOUBLE3) &&
		valuetypid != type_oid(T_DOUBLE4))
		elog(ERROR, "unknown base type: %d", valuetypid);
}

/**
 * @brief Ensures that the Oid is an external base type that allows linear 
 *		interpolation
 */
void
ensure_linear_interpolation(Oid valuetypid)
{
	if (valuetypid != FLOAT8OID &&
		valuetypid != type_oid(T_GEOMETRY) &&
		valuetypid != type_oid(T_GEOGRAPHY))
		elog(ERROR, "unknown base type with linear interpolation: %d", valuetypid);
}

/**
 * @brief Ensures that the Oid is an external or external base type that allows
 *		linear interpolation
 */
void
ensure_linear_interpolation_all(Oid valuetypid)
{
	if (valuetypid != FLOAT8OID &&
		valuetypid !=  type_oid(T_DOUBLE2) &&
		valuetypid != type_oid(T_GEOMETRY) &&
		valuetypid != type_oid(T_GEOGRAPHY) &&
		valuetypid != type_oid(T_DOUBLE3) &&
		valuetypid != type_oid(T_DOUBLE4))
		elog(ERROR, "unknown base type with linear interpolation: %d", valuetypid);
}

/**
 * @brief Ensures that the Oid is a numeric base type supported by MobilityDB 
 */
void 
ensure_numeric_base_type(Oid valuetypid)
{
	if (valuetypid != INT4OID && valuetypid != FLOAT8OID)
		elog(ERROR, "unknown numeric base type: %d", valuetypid);
}

/**
 * @brief Ensures that the Oid is a point base type supported by MobilityDB
 */
void
ensure_point_base_type(Oid valuetypid)
{
	if (valuetypid != type_oid(T_GEOMETRY) && valuetypid != type_oid(T_GEOGRAPHY))
		elog(ERROR, "unknown point base type: %d", valuetypid);
}

/*****************************************************************************/

/**
 * @brief Ensures that the two temporal values have the same duration
 */
void
ensure_same_duration(const Temporal *temp1, const Temporal *temp2)
{
	if (temp1->duration != temp2->duration)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("The temporal values must be of the same duration")));
}

/**
 * @brief Ensures that the two temporal values have the same base type
 */
void
ensure_same_base_type(const Temporal *temp1, const Temporal *temp2)
{
	if (temp1->valuetypid != temp2->valuetypid)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("The temporal values must be of the same base type")));
}

/**
 * @brief Ensures that the two temporal values have the same interpolation
 */
void
ensure_same_interpolation(const Temporal *temp1, const Temporal *temp2)
{
	if (MOBDB_FLAGS_GET_LINEAR(temp1->flags) != MOBDB_FLAGS_GET_LINEAR(temp2->flags))
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("The temporal values must be of the same interpolation")));
}

/**
 * @brief Ensures that the timestamp of the first temporal instant value is smaller
 *		than the one of the second temporal instant value
 */
void
ensure_increasing_timestamps(const TemporalInst *inst1, const TemporalInst *inst2)
{
	if (inst1->t >= inst2->t)
	{
		char *t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
		char *t2 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst2->t));
		ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
			errmsg("Timestamps for temporal value must be increasing: %s, %s", t1, t2)));
	}
}

/**
 * @brief Ensures that all temporal instant values of the array have increasing
 *		timestamp, and if they are temporal points, have the same srid and the
 * 		same dimensionality
 */
void
ensure_valid_temporalinstarr(TemporalInst **instants, int count, bool isgeo)
{
	for (int i = 1; i < count; i++)
	{
		ensure_increasing_timestamps(instants[i - 1], instants[i]);
		if (isgeo)
		{
			ensure_same_srid_tpoint((Temporal *)instants[i - 1], (Temporal *)instants[i]);
			ensure_same_dimensionality_tpoint((Temporal *)instants[i - 1], (Temporal *)instants[i]);
		}
	}
}

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(mobilitydb_version);
/**
 * @brief Version of the MobilityDB extension
 */
PGDLLEXPORT Datum
mobilitydb_version(PG_FUNCTION_ARGS)
{
	char *ver = MOBDB_VERSION_STR;
	text *result = cstring_to_text(ver);
	PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(mobilitydb_full_version);
/**
 * @brief Versions of the MobilityDB extension and its dependencies
 */
PGDLLEXPORT Datum
mobilitydb_full_version(PG_FUNCTION_ARGS)
{
	char ver[64];
	text *result;

	snprintf(ver, 64, "%s, %s, %s", MOBDB_VERSION_STR, 
		MOBDB_PGSQL_VERSION_STR, MOBDB_POSTGIS_VERSION_STR);
	ver[63] = '\0';

	result = cstring_to_text(ver);
	PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_in);
/** 
 * @brief Generic input function for temporal types
 * @note Examples of input for temporal instant values:
 * @code 
 * false @ 2012-01-01 08:00:00
 * 1.5 @ 2012-01-01 08:00:00
 * @endcode
 */
PGDLLEXPORT Datum
temporal_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	Oid temptypid = PG_GETARG_OID(1);
	int32 temp_typmod = -1;
	Oid valuetypid = temporal_valuetypid(temptypid);
	Temporal *result = temporal_parse(&input, valuetypid);
	if (PG_NARGS() > 2 && !PG_ARGISNULL(2)) 
		temp_typmod = PG_GETARG_INT32(2);
	if (temp_typmod >= 0)
		result = temporal_valid_typmod(result, temp_typmod);
	PG_RETURN_POINTER(result);
}

/**
 * @brief Returns the string representation of the temporal value
 *		(dispatch function)
 * @param[in] temp Temporal value
 * @param[in] value_out Function called to output the base value
 *		depending on its Oid
 */
char *
temporal_to_string(const Temporal *temp, char *(*value_out)(Oid, Datum))
{
	char *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_to_string((TemporalInst *)temp, value_out);
	else if (temp->duration == TEMPORALI) 
		result = temporali_to_string((TemporalI *)temp, value_out);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_to_string((TemporalSeq *)temp, false, value_out);
	else /* temp->duration == TEMPORALS */
		result = temporals_to_string((TemporalS *)temp, value_out);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_out);
/**
 * @brief Generic output function for temporal types
 */
PGDLLEXPORT Datum
temporal_out(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	char *result = temporal_to_string(temp, &call_output);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_CSTRING(result);
}

/**
 * @brief Write the binary representation of the temporal value
 *		into the buffer (dispatch function)
 * @param[in] temp Temporal value
 * @param[in] buf Buffer
 */void
temporal_write(Temporal *temp, StringInfo buf)
{
	pq_sendbyte(buf, (uint8) temp->duration);
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		temporalinst_write((TemporalInst *) temp, buf);
	else if (temp->duration == TEMPORALI)
		temporali_write((TemporalI *) temp, buf);
	else if (temp->duration == TEMPORALSEQ)
		temporalseq_write((TemporalSeq *) temp, buf);
	else /* temp->duration == TEMPORALS */
		temporals_write((TemporalS *) temp, buf);
}

PG_FUNCTION_INFO_V1(temporal_send);
/* 
 * @brief Generic send function for temporal types
 */
PGDLLEXPORT Datum
temporal_send(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	temporal_write(temp, &buf) ;
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/**
 * @brief Returns a new temporal value from its binary representation 
 *		read from the buffer (dispatch function)
 * @param[in] buf Buffer
 * @param[in] valuetypid Oid of the base type
 */
Temporal *
temporal_read(StringInfo buf, Oid valuetypid)
{
	int16 type = (int16) pq_getmsgbyte(buf);
	Temporal *result;
	ensure_valid_duration(type);
	if (type == TEMPORALINST)
		result = (Temporal *) temporalinst_read(buf, valuetypid);
	else if (type == TEMPORALI)
		result = (Temporal *) temporali_read(buf, valuetypid);
	else if (type == TEMPORALSEQ)
		result = (Temporal *) temporalseq_read(buf, valuetypid);
	else /* type == TEMPORALS */
		result = (Temporal *) temporals_read(buf, valuetypid);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_recv);
/**
 * @brief Generic receive function for temporal types
 */
PGDLLEXPORT Datum
temporal_recv(PG_FUNCTION_ARGS)
{
	StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
	Oid temptypid = PG_GETARG_OID(1);
	Oid valuetypid = temporal_valuetypid(temptypid);
	Temporal *result = temporal_read(buf, valuetypid) ;
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_typmod_in);
/**
 * @brief Input typmod information for temporal types
 */
PGDLLEXPORT Datum 
temporal_typmod_in(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	Datum *elem_values;
	int n = 0;

	if (ARR_ELEMTYPE(array) != CSTRINGOID)
		ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
				errmsg("typmod array must be type cstring[]")));
	if (ARR_NDIM(array) != 1)
		ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
				errmsg("typmod array must be one-dimensional")));
	if (ARR_HASNULL(array))
		ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
				errmsg("typmod array must not contain nulls")));

	deconstruct_array(array, CSTRINGOID, -2, false, 'c', &elem_values, NULL, &n);
	if (n != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Invalid temporal type modifier")));

	/* Temporal Type */
	char *s = DatumGetCString(elem_values[0]);
	int16 duration = 0;
	if (!temporal_duration_from_string(s, &duration))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Invalid temporal type modifier: %s", s)));

	pfree(elem_values);
	PG_RETURN_INT32((int32)duration);
}

PG_FUNCTION_INFO_V1(temporal_typmod_out);
/**
 * @brief Output typmod information for temporal types
 */
PGDLLEXPORT Datum 
temporal_typmod_out(PG_FUNCTION_ARGS)
{
	char *s = (char *) palloc(64);
	char *str = s;
	int32 typmod = PG_GETARG_INT32(0);
	int16 duration = TYPMOD_GET_DURATION(typmod);
	/* No type? Then no typmod at all. Return empty string.  */
	if (typmod < 0 || !duration)
	{
		*str = '\0';
		PG_RETURN_CSTRING(str);
	}
	sprintf(str, "(%s)", temporal_duration_name(duration));
	PG_RETURN_CSTRING(s);
}

PG_FUNCTION_INFO_V1(temporal_enforce_typmod);
/**
 * @brief Enforce typmod information for temporal types
 */
PGDLLEXPORT Datum temporal_enforce_typmod(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int32 typmod = PG_GETARG_INT32(1);
	/* Check if temporal typmod is consistent with the supplied one */
	temp = temporal_valid_typmod(temp, typmod);
	PG_RETURN_POINTER(temp);
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

PG_FUNCTION_INFO_V1(temporalinst_constructor);
/**
 * @brief Construct a temporal instant value from the arguments
 */
PGDLLEXPORT Datum
temporalinst_constructor(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Temporal *result = (Temporal *)temporalinst_make(value, t, valuetypid);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporali_constructor);
/**
 * @brief Construct a temporal instant set value from the array of temporal
 *		instant values
 */
PGDLLEXPORT Datum
temporali_constructor(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	if (count == 0)
	{
		PG_FREE_IF_COPY(array, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("A temporal instant set must have at least one temporal instant")));
	}
	
	TemporalInst **instants = (TemporalInst **)temporalarr_extract(array, &count);
	/* Ensure that all values are of type temporal instant */
	for (int i = 0; i < count; i++)
	{
		if (instants[i]->duration != TEMPORALINST)
		{
			PG_FREE_IF_COPY(array, 0);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Input values must be of type temporal instant")));
		}
	}
	
	Temporal *result = (Temporal *)temporali_make(instants, count);
	pfree(instants);
	PG_FREE_IF_COPY(array, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tstepseq_constructor);
/**
 * @brief Construct a temporal sequence value with stepwise interpolation from
 *		the array of temporal instant values
 */
PGDLLEXPORT Datum
tstepseq_constructor(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	bool lower_inc = PG_GETARG_BOOL(1);
	bool upper_inc = PG_GETARG_BOOL(2);
	int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	if (count == 0)
	{
		PG_FREE_IF_COPY(array, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("A temporal sequence must have at least one temporal instant")));
	}

	TemporalInst **instants = (TemporalInst **)temporalarr_extract(array, &count);
	/* Ensure that all values are of type temporal instant */
	for (int i = 0; i < count; i++)
	{
		if (instants[i]->duration != TEMPORALINST)
		{
			pfree(instants);
			PG_FREE_IF_COPY(array, 0);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Input values must be temporal instants")));
		}
	}

	Temporal *result = (Temporal *)temporalseq_make(instants,
		count, lower_inc, upper_inc, false, true);
	pfree(instants);
	PG_FREE_IF_COPY(array, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tlinearseq_constructor);
/**
 * @brief Construct a temporal sequence value with linear or stepwise
 * 		interpolation from the array of temporal instant values
 */
PGDLLEXPORT Datum
tlinearseq_constructor(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	bool lower_inc = PG_GETARG_BOOL(1);
	bool upper_inc = PG_GETARG_BOOL(2);
	bool linear = PG_GETARG_BOOL(3);
	int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	if (count == 0)
	{
		PG_FREE_IF_COPY(array, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("A temporal sequence must have at least one temporal instant")));
	}
	
	TemporalInst **instants = (TemporalInst **)temporalarr_extract(array, &count);
	/* Ensure that all values are of type temporal instant */
	for (int i = 0; i < count; i++)
	{
		if (instants[i]->duration != TEMPORALINST)
		{
			pfree(instants);
			PG_FREE_IF_COPY(array, 0);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Input values must be temporal instants")));
		}
	}

	Temporal *result = (Temporal *)temporalseq_make(instants, 
		count, lower_inc, upper_inc, linear, true);
	pfree(instants);
	PG_FREE_IF_COPY(array, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporals_constructor);
/**
 * @brief Construct a temporal sequence set value from the array of temporal
 *		sequence values
 */
PGDLLEXPORT Datum
temporals_constructor(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	if (count == 0)
	{
		PG_FREE_IF_COPY(array, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("A temporal sequence set value must at least one sequence")));
	}
	
	TemporalSeq **sequences = (TemporalSeq **)temporalarr_extract(array, &count);
	bool linear = MOBDB_FLAGS_GET_LINEAR(sequences[0]->flags);
	/* Ensure that all values are of sequence duration and of the same interpolation */
	for (int i = 0; i < count; i++)
	{
		if (sequences[i]->duration != TEMPORALSEQ)
		{
			PG_FREE_IF_COPY(array, 0);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Input values must be temporal sequences")));
		}
		if (MOBDB_FLAGS_GET_LINEAR(sequences[i]->flags) != linear)
		{
			PG_FREE_IF_COPY(array, 0);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Input sequences must have the same interpolation")));
		}
	}

	Temporal *result = (Temporal *)temporals_make(sequences, count, true);
	
	pfree(sequences);
	PG_FREE_IF_COPY(array, 0);
	
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Tranformation functions
 ****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_append_instant);
/**
 * @brief Append an instant to the end of a temporal value
 */
PGDLLEXPORT Datum
temporal_append_instant(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *inst = PG_GETARG_TEMPORAL(1);
	if (inst->duration != TEMPORALINST) 
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("The second argument must be of instant duration")));
	ensure_same_base_type(temp, (Temporal *)inst);

	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_append_instant((TemporalInst *)temp,
			(TemporalInst *)inst);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_append_instant((TemporalI *)temp,
			(TemporalInst *)inst);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_append_instant((TemporalSeq *)temp,
			(TemporalInst *)inst);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_append_instant((TemporalS *)temp,
			(TemporalInst *)inst);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(inst, 1);
	PG_RETURN_POINTER(result);
}

/**
 * @brief Convert two temporal values into a common duration
 * @param[in] temp1,temp2 Input values
 * @param[out] new1,new2 Output values
 */
static void
temporal_convert_same_duration(const Temporal *temp1, const Temporal *temp2,
	Temporal **new1, Temporal **new2)
{
	assert(temp1->valuetypid == temp2->valuetypid);
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);

	/* If both are of the same duration do nothing */
	if (temp1->duration == temp2->duration)
	{
		*new1 = (Temporal *) temp1;
		*new2 = (Temporal *) temp2;
		return;
	}

	/* Different duration */
	bool swap = false;
	if (temp1->duration > temp2->duration)
	{
		Temporal *temp = (Temporal *) temp1;
		temp1 = temp2;
		temp2 = temp;
		swap = true;
	}
	Temporal *new;
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI)
		new = (Temporal *) temporalinst_to_temporali((TemporalInst *) temp1);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ)
		new = (Temporal *) temporalinst_to_temporalseq((TemporalInst *) temp1,
			MOBDB_FLAGS_GET_LINEAR(temp2->flags));
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS)
		new = (Temporal *) temporalinst_to_temporals((TemporalInst *) temp1,
			MOBDB_FLAGS_GET_LINEAR(temp2->flags));
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ)
		new = ((TemporalI *) temp1)->count == 1 ?
			(Temporal *) temporali_to_temporalseq((TemporalI *) temp1,
				MOBDB_FLAGS_GET_LINEAR(temp2->flags)) :
			(Temporal *) temporali_to_temporals((TemporalI *) temp1,
				MOBDB_FLAGS_GET_LINEAR(temp2->flags));
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS)
		new = (Temporal *) temporali_to_temporals((TemporalI *) temp1,
			MOBDB_FLAGS_GET_LINEAR(temp2->flags));
	else /* temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS */
		new = (Temporal *) temporalseq_to_temporals((TemporalSeq *) temp1);
	if (swap)
	{
		*new1 = (Temporal *) temp1;
		*new2 = new;
	}
	else
	{
		*new1 = new;
		*new2 = (Temporal *) temp2;
	}
}

PG_FUNCTION_INFO_V1(temporal_merge);
/**
 * @brief Merge the two temporal values
 * @param[in] temp1 First value
 * @param[in] temp2 Second value
 * @result Merged value. Returns NULL if both arguments are NULL.
 *		If one argument is null the other argument is output.
 */
PGDLLEXPORT Datum
temporal_merge(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL(1);

	Temporal *result;
	/* Can't do anything with null inputs */
	if (!temp1 && !temp2)
		PG_RETURN_NULL();
	/* One argument is null, return a copy of the other temporal */
	if (!temp1)
	{
		result = temporal_copy(temp2);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_POINTER(result);
	}
	/* Non-null period and null temporal, return the period */
	if (!temp2)
	{
		result = temporal_copy(temp1);
		PG_FREE_IF_COPY(temp1, 0);
		PG_RETURN_POINTER(result);
	}

	/* Both arguments are temporal */
	ensure_same_base_type(temp1, temp2);
	ensure_same_interpolation(temp1, temp2);

	/* Convert to the same duration if possible */
	Temporal *temp1new, *temp2new;
	temporal_convert_same_duration(temp1, temp2, &temp1new, &temp2new);

	ensure_valid_duration(temp1new->duration);
	if (temp1new->duration == TEMPORALINST)
		result = temporalinst_merge(
			(TemporalInst *) temp1new, (TemporalInst *)temp2new);
	else if (temp1new->duration == TEMPORALI)
		result = (Temporal *)temporali_merge(
			(TemporalI *)temp1new, (TemporalI *)temp2new);
	else if (temp1new->duration == TEMPORALSEQ)
		result = (Temporal *) temporalseq_merge((TemporalSeq *)temp1new,
			(TemporalSeq *)temp2new);
	else /* temp1new->duration == TEMPORALS */
		result = (Temporal *) temporals_merge((TemporalS *)temp1new,
			(TemporalS *)temp2new);
	if (temp1 != temp1new)
		pfree(temp1new);
	if (temp2 != temp2new)
		pfree(temp2new);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_POINTER(result);
}

/**
 * @brief Convert the array of temporal values into a common duration
 * @param[in] temporals Array of values
 * @param[in] count Number of values
 * @param[in] duration common duration
 * @result  Array of output values
 */
static Temporal **
temporalarr_convert_duration(Temporal **temporals, int count, int16 duration)
{
	ensure_valid_duration(duration);
	Temporal **result = palloc(sizeof(Temporal *) * count);
	for (int i = 0; i < count; i++)
	{
		assert(duration >= temporals[i]->duration);
		if (temporals[i]->duration == duration)
			result[i] = temporal_copy(temporals[i]);
		else if (temporals[i]->duration == TEMPORALINST && duration == TEMPORALI)
			result[i] = (Temporal *) temporalinst_to_temporali((TemporalInst *) temporals[i]);
		else if (temporals[i]->duration == TEMPORALINST && duration == TEMPORALSEQ)
			result[i] = (Temporal *) temporalinst_to_temporalseq((TemporalInst *) temporals[i],
				MOBDB_FLAGS_GET_LINEAR(temporals[i]->flags));
		else if (temporals[i]->duration == TEMPORALINST && duration == TEMPORALS)
			result[i] = (Temporal *) temporalinst_to_temporals((TemporalInst *) temporals[i],
				MOBDB_FLAGS_GET_LINEAR(temporals[i]->flags));
		else if (temporals[i]->duration == TEMPORALI && duration == TEMPORALSEQ)
			result[i] = (Temporal *) temporali_to_temporals((TemporalI *) temporals[i],
					MOBDB_FLAGS_GET_LINEAR(temporals[i]->flags));
		else if (temporals[i]->duration == TEMPORALI && duration == TEMPORALS)
			result[i] = (Temporal *) temporali_to_temporals((TemporalI *) temporals[i],
				MOBDB_FLAGS_GET_LINEAR(temporals[i]->flags));
		else /* temporals[i]->duration == TEMPORALSEQ && duration == TEMPORALS */
			result[i] = (Temporal *) temporalseq_to_temporals((TemporalSeq *) temporals[i]);
	}
	return result;
}

PG_FUNCTION_INFO_V1(temporal_merge_array);
/**
 * @brief Merge the array of temporal values
 * @param[in] array Array of values
 * @result Merged value
 */
PGDLLEXPORT Datum
temporal_merge_array(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	if (count == 0)
	{
		PG_FREE_IF_COPY(array, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("A temporal value must have at least one temporal instant")));
	}

	Temporal **temporals = temporalarr_extract(array, &count);
	/* Ensure all values have the same interpolation and determine
	 * duration of the result */
	int16 duration = temporals[0]->duration;
	bool interpolation = MOBDB_FLAGS_GET_LINEAR(temporals[0]->flags);
	for (int i = 1; i < count; i++)
	{
		if (MOBDB_FLAGS_GET_LINEAR(temporals[i]->flags) != interpolation)
		{
			PG_FREE_IF_COPY(array, 0);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Input values must be of the same interpolation")));
		}
		if (duration != temporals[i]->duration)
		{
			/* A TemporalInst cannot be converted to a TemporalSeq */
			int16 new_duration = Max(duration, temporals[i]->duration);
			if (new_duration == TEMPORALSEQ && duration == TEMPORALI)
				new_duration = TEMPORALS;
			duration = new_duration;
		}
	}
	Temporal **newtemps = temporalarr_convert_duration(temporals, count,
		duration);

	Temporal *result;
	ensure_valid_duration(duration);
	if (duration == TEMPORALINST)
		result = (Temporal *) temporalinst_merge_array(
			(TemporalInst **) newtemps, count);
	else if (duration == TEMPORALI)
		result = temporali_merge_array(
			(TemporalI **) newtemps, count);
	else if (duration == TEMPORALSEQ)
		result = (Temporal *) temporalseq_merge_array(
			(TemporalSeq **) newtemps, count);
	else /* duration == TEMPORALS */
		result = (Temporal *) temporals_merge_array(
			(TemporalS **) newtemps, count);

	pfree(temporals);
	for (int i = 1; i < count; i++)
		pfree(newtemps[i]);
	pfree(newtemps);
	PG_FREE_IF_COPY(array, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @brief Cast the temporal integer value as a temporal float value
 *		(dispatch function)
 */
Temporal *
tint_to_tfloat_internal(Temporal *temp)
{
	Temporal *result;
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tintinst_to_tfloatinst((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tinti_to_tfloati((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)tintseq_to_tfloatseq((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tints_to_tfloats((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(tint_to_tfloat);
/**
 * @brief Cast the temporal integer value as a temporal float value
 */
PGDLLEXPORT Datum
tint_to_tfloat(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tint_to_tfloat_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/**
 * @brief Cast the temporal float value as a temporal integer value
 *		(dispatch function)
 */
Temporal *
tfloat_to_tint_internal(Temporal *temp)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfloatinst_to_tintinst((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfloati_to_tinti((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)tfloatseq_to_tintseq((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tfloats_to_tints((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(tfloat_to_tint);
/**
 * @brief Cast the temporal float value as a temporal integer value
 */
PGDLLEXPORT Datum
tfloat_to_tint(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tfloat_to_tint_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/**
 * @brief Returns the bounding period on which the temporal value is defined 
 *		(dispatch function)
 */
void
temporal_period(Period *p, const Temporal *temp)
{
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		temporalinst_period(p, (TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		temporali_period(p, (TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		temporalseq_period(p, (TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		temporals_period(p, (TemporalS *)temp);
}

PG_FUNCTION_INFO_V1(temporal_to_period);
/**
 * @brief Returns the bounding period on which the temporal value is defined
 */
PGDLLEXPORT Datum
temporal_to_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *result = (Period *) palloc(sizeof(Period));
	temporal_period(result, temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_PERIOD(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_to_temporalinst);
/**
 * @brief Transform the temporal value into a temporal instant value
 */
PGDLLEXPORT Datum
temporal_to_temporalinst(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporal_copy(temp);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)temporali_to_temporalinst((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_to_temporalinst((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_to_temporalinst((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_to_temporali);
/**
 * @brief Transform the temporal value into a temporal instant set value
 */
PGDLLEXPORT Datum
temporal_to_temporali(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_to_temporali((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = temporal_copy(temp);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_to_temporali((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_to_temporali((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_to_temporalseq);
/**
 * @brief Transform the temporal value into a temporal sequence value
 */
PGDLLEXPORT Datum
temporal_to_temporalseq(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp->flags);
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_to_temporalseq((TemporalInst *)temp, linear);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)temporali_to_temporalseq((TemporalI *)temp, linear);
	else if (temp->duration == TEMPORALSEQ)
		result = temporal_copy(temp);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_to_temporalseq((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result); 
}

PG_FUNCTION_INFO_V1(temporal_to_temporals);
/**
 * @brief Transform the temporal value into a temporal sequence set value
 */
PGDLLEXPORT Datum
temporal_to_temporals(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp->flags);
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_to_temporals((TemporalInst *)temp, linear);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)temporali_to_temporals((TemporalI *)temp, linear);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_to_temporals((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = temporal_copy(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result); 
}

PG_FUNCTION_INFO_V1(tstep_to_linear);
/**
 * @brief Transform the temporal value with continuous base type from stepwise 
 *		to linear interpolation
 */
PGDLLEXPORT Datum
tstep_to_linear(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->duration != TEMPORALSEQ && temp->duration != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));
	ensure_linear_interpolation(temp->valuetypid);

	if (MOBDB_FLAGS_GET_LINEAR(temp->flags))
			PG_RETURN_POINTER(temporal_copy(temp)); 

	Temporal *result;
	if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)tstepseq_to_linear((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tsteps_to_linear((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result); 
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_duration);
/**
 * @brief Returns the string representation of the temporal duration
 */
Datum temporal_duration(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	char str[12];
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		strcpy(str, "Instant");
	else if (temp->duration == TEMPORALI) 
		strcpy(str, "InstantSet");
	else if (temp->duration == TEMPORALSEQ) 
		strcpy(str, "Sequence");
	else /* temp->duration == TEMPORALS */
		strcpy(str, "SequenceSet");
	text *result = cstring_to_text(str);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(temporal_interpolation);
/**
 * @brief Returns the string representation of the temporal interpolation
 */
Datum temporal_interpolation(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	char str[12];
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST || temp->duration == TEMPORALI) 
		strcpy(str, "Discrete");
	else if (temp->duration == TEMPORALSEQ || temp->duration == TEMPORALS)
	{
		if (MOBDB_FLAGS_GET_LINEAR(temp->flags))
			strcpy(str, "Linear");
		else
			strcpy(str, "Stepwise");
	}
	text *result = cstring_to_text(str);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TEXT_P(result);
}


PG_FUNCTION_INFO_V1(temporal_mem_size);
/**
 * @brief Returns the size in bytes of the temporal value
 */
PGDLLEXPORT Datum
temporal_mem_size(PG_FUNCTION_ARGS)
{
	Datum result = toast_datum_size(PG_GETARG_DATUM(0));
	PG_RETURN_DATUM(result);
}
/*
PGDLLEXPORT Datum
temporal_mem_size(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	size_t result = VARSIZE(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_INT32(result);
}
*/

/**
 * @brief Returns the base values of the temporal value as a PostgreSQL array
 *		(dispatch function)
 */
Datum
temporal_values(Temporal *temp)
{
	ArrayType *result;	/* make the compiler quiet */
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_values((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = temporali_values((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_values((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = temporals_values((TemporalS *)temp);
	return PointerGetDatum(result);
}

PG_FUNCTION_INFO_V1(temporal_get_values);
/**
 * @brief Returns the base values of the temporal value as an array
 */
PGDLLEXPORT Datum
temporal_get_values(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = temporal_values(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/**
 * @brief Returns the base values of the temporal float value as an array of ranges
 *		(dispatch function)
 */
Datum
tfloat_ranges(const Temporal *temp)
{
	ArrayType *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = tfloatinst_ranges((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = tfloati_ranges((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = tfloatseq_ranges((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = tfloats_ranges((TemporalS *)temp);
	return PointerGetDatum(result);
}

PG_FUNCTION_INFO_V1(tfloat_get_ranges);
/**
 * @brief Returns the base values of the temporal float value as an array
 *		of ranges
 */
PGDLLEXPORT Datum
tfloat_get_ranges(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = tfloat_ranges(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporalinst_get_value);
/**
 * @brief Returns the base value of the temporal instant value
 */
PGDLLEXPORT Datum
temporalinst_get_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->duration != TEMPORALINST)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal instant")));
		
	TemporalInst *inst = (TemporalInst *)temp;
	Datum result = temporalinst_value_copy(inst);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/**
 * @brief Returns the time on which the temporal value is defined as a period set
 *		(dispatch function)
 */
PeriodSet *
temporal_get_time_internal(const Temporal *temp)
{
	PeriodSet *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_get_time((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = temporali_get_time((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_get_time((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = temporals_get_time((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_get_time);
/**
 * @brief Returns the time on which the temporal value is defined as a period set
 */
PGDLLEXPORT Datum
temporal_get_time(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *result = temporal_get_time_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporalinst_timestamp);
/**
 * @brief Returns the timestamp of the temporal instant value
 */
PGDLLEXPORT Datum
temporalinst_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->duration != TEMPORALINST)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal instant")));

	TimestampTz result = ((TemporalInst *)temp)->t;
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TIMESTAMPTZ(result);
}

/**
 * @brief Returns a pointer to the precomputed bounding box of the temporal value
 * @return Returns NULL for temporal instant values since they do not have
 *		precomputed bounding box.
 */
void *
temporal_bbox_ptr(const Temporal *temp)
{
	void *result = NULL;
	if (temp->duration == TEMPORALI)
		result = temporali_bbox_ptr((TemporalI *) temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_bbox_ptr((TemporalSeq *) temp);
	else if (temp->duration == TEMPORALS) 
		result = temporals_bbox_ptr((TemporalS *) temp);
	return result;
}

/**
 * @brief Set the first argument to the bounding box of the temporal value
 * @details For temporal instant values the bounding box must be computed.
 *		For the other durations a copy of the precomputed bounding box 
 *		is made.
 */
void 
temporal_bbox(void *box, const Temporal *temp)
{
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		temporalinst_make_bbox(box, (TemporalInst *) temp);
	else if (temp->duration == TEMPORALI) 
		temporali_bbox(box, (TemporalI *) temp);
	else if (temp->duration == TEMPORALSEQ) 
		temporalseq_bbox(box, (TemporalSeq *) temp);
	else /* temp->duration == TEMPORALS */
		temporals_bbox(box, (TemporalS *) temp);
}

PG_FUNCTION_INFO_V1(tnumber_to_tbox);
/**
 * @brief Returns the bounding box of the temporal value
 */
PGDLLEXPORT Datum
tnumber_to_tbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *result = palloc0(sizeof(TBOX));
	temporal_bbox(result, temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/**
 * @brief Returns the value range of the temporal integer value
 *		(internal function)
 */
RangeType *
tnumber_value_range_internal(const Temporal *temp)
{
	RangeType *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
	{
		Datum value = temporalinst_value((TemporalInst *)temp);
		result = range_make(value, value, true, true, temp->valuetypid);		
	}
	else 
	{
		TBOX *box = (TBOX *) temporal_bbox_ptr(temp);
		Datum min = 0, max = 0;
		ensure_numeric_base_type(temp->valuetypid);
		if (temp->valuetypid == INT4OID)
		{
			min = Int32GetDatum((int)(box->xmin));
			max = Int32GetDatum((int)(box->xmax));
		}
		else if (temp->valuetypid == FLOAT8OID)
		{
			min = Float8GetDatum(box->xmin);
			max = Float8GetDatum(box->xmax);
		}
		result = range_make(min, max, true, true, temp->valuetypid);
	}	
	return result;
}

PG_FUNCTION_INFO_V1(tnumber_value_range);
/**
 * @brief Returns the value range of the temporal integer value
 */
PGDLLEXPORT Datum
tnumber_value_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *result = tnumber_value_range_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
#if MOBDB_PGSQL_VERSION < 110000
	PG_RETURN_RANGE(result);
#else
	PG_RETURN_RANGE_P(result);
#endif
}

PG_FUNCTION_INFO_V1(temporal_start_value);
/**
 * @brief Returns the start base value of the temporal value
 */
PGDLLEXPORT Datum
temporal_start_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = temporalinst_value_copy(temporali_inst_n((TemporalI *)temp, 0));
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalinst_value_copy(temporalseq_inst_n((TemporalSeq *)temp, 0));
	else /* temp->duration == TEMPORALS */
	{
		TemporalSeq *seq = temporals_seq_n((TemporalS *)temp, 0);
		result = temporalinst_value_copy(temporalseq_inst_n(seq, 0));
	}
	PG_FREE_IF_COPY(temp, 0);	
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(temporal_end_value);
/**
 * @brief Returns the end base value of the temporal value
 */
PGDLLEXPORT Datum
temporal_end_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = temporalinst_value_copy(temporali_inst_n((TemporalI *)temp, 
			((TemporalI *)temp)->count - 1));
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalinst_value_copy(temporalseq_inst_n((TemporalSeq *)temp, 
			((TemporalSeq *)temp)->count - 1));
	else /* temp->duration == TEMPORALS */
	{
		TemporalSeq *seq = temporals_seq_n((TemporalS *)temp, ((TemporalS *)temp)->count - 1);
		result = temporalinst_value_copy(temporalseq_inst_n(seq, seq->count - 1));
	}
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/**
 * @brief Returns a pointer to the instant with minimum base value of the
 *		temporal value
 * @details The function does not take into account whether the
 *		instant is at an exclusive bound or not
 * @note Function used, e.g., for computing the shortest line between two
 *		temporal points from their temporal distance
 */
TemporalInst *
temporal_min_instant(const Temporal *temp)
{
	TemporalInst *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (TemporalInst *)temp;
	else if (temp->duration == TEMPORALI)
		result = temporali_min_instant((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_min_instant((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = temporals_min_instant((TemporalS *)temp);
	return result;
}

/**
 * @brief Returns the minimum base value of the temporal value
 *		(dispatch function)
 */
Datum
temporal_min_value_internal(const Temporal *temp)
{
	Datum result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = datum_copy(temporali_min_value((TemporalI *)temp),
			temp->valuetypid);
	else if (temp->duration == TEMPORALSEQ) 
		result = datum_copy(temporalseq_min_value((TemporalSeq *)temp),
			temp->valuetypid);
	else /* temp->duration == TEMPORALS */
		result = datum_copy(temporals_min_value((TemporalS *)temp),
			temp->valuetypid);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_min_value);
/**
 * @brief Returns the minimum base value of the temporal value
 */
PGDLLEXPORT Datum
temporal_min_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = temporal_min_value_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(temporal_max_value);
/**
 * @brief Returns the maximum base value of the temporal value
 */
Datum
temporal_max_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = datum_copy(temporali_max_value((TemporalI *)temp),
			temp->valuetypid);
	else if (temp->duration == TEMPORALSEQ) 
		result = datum_copy(temporalseq_max_value((TemporalSeq *)temp),
			temp->valuetypid);
	else /* temp->duration == TEMPORALS */
		result = datum_copy(temporals_max_value((TemporalS *)temp),
			temp->valuetypid);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(temporal_timespan);
/**
 * @brief Returns the timespan of the temporal value
 */
PGDLLEXPORT Datum
temporal_timespan(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST || temp->duration == TEMPORALI) 
	{
		Interval *interval = (Interval *) palloc(sizeof(Interval));
		interval->month = interval->day =  0;
		interval->time = (TimeOffset) 0;
		result = PointerGetDatum(interval);
	}
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_timespan((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = temporals_timespan((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(temporal_num_sequences);
/**
 * @brief Returns the number of sequences of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
temporal_num_sequences(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->duration != TEMPORALSEQ && temp->duration != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));

	int result = 1;
	if (temp->duration == TEMPORALS)
		result = ((TemporalS *)temp)->count;
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(temporal_start_sequence);
/**
 * @brief Returns the start sequence of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
temporal_start_sequence(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->duration != TEMPORALSEQ && temp->duration != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));

	TemporalSeq *result;
	if (temp->duration == TEMPORALSEQ)
		result = temporalseq_copy((TemporalSeq *)temp);
	else
		result = temporalseq_copy(temporals_seq_n((TemporalS *)temp, 0));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_end_sequence);
/**
 * @brief Returns the end sequence of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
temporal_end_sequence(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->duration != TEMPORALSEQ && temp->duration != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));

	TemporalSeq *result;
	if (temp->duration == TEMPORALSEQ)
		result = temporalseq_copy((TemporalSeq *)temp);
	else
	{
		TemporalS *ts = (TemporalS *)temp;
		result = temporalseq_copy(temporals_seq_n(ts, ts->count - 1));
	}
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_sequence_n);
/**
 * @brief Returns the n-th sequence of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
temporal_sequence_n(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->duration != TEMPORALSEQ && temp->duration != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));

	int i = PG_GETARG_INT32(1); /* Assume 1-based */
	TemporalSeq *result = NULL;
	if (temp->duration == TEMPORALSEQ)
	{
		if (i == 1)
			result = temporalseq_copy((TemporalSeq *)temp);
	}
	else
	{
		TemporalS *ts = (TemporalS *)temp;
		if (i >= 1 && i <= ts->count)
			result = temporalseq_copy(temporals_seq_n(ts, i - 1));
	}
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
} 

PG_FUNCTION_INFO_V1(temporal_sequences);
/**
 * @brief Returns the sequences of the temporal sequence (set) value as 
 *		an array
 */
PGDLLEXPORT Datum
temporal_sequences(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->duration != TEMPORALSEQ && temp->duration != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));
				
	ArrayType *result;
	if (temp->duration == TEMPORALSEQ)
		result = temporalarr_to_array(&temp, 1);
	else
		result = temporals_sequences_array((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(temporal_num_instants);
/**
 * @brief Returns the number of distinct instants of the temporal value
 */
PGDLLEXPORT Datum
temporal_num_instants(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = 1;
	else if (temp->duration == TEMPORALI) 
		result = ((TemporalI *)temp)->count;
	else if (temp->duration == TEMPORALSEQ) 
		result = ((TemporalSeq *)temp)->count;
	else /* temp->duration == TEMPORALS */
		result = temporals_num_instants((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(temporal_start_instant);
/**
 * @brief Returns the start instant of the temporal value
 */
PGDLLEXPORT Datum
temporal_start_instant(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporalinst_copy(temporali_inst_n((TemporalI *)temp, 0));
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalinst_copy(temporalseq_inst_n((TemporalSeq *)temp, 0));
	else /* temp->duration == TEMPORALS */
	{
		TemporalSeq *seq = temporals_seq_n((TemporalS *)temp, 0);
		result = (Temporal *)temporalinst_copy(temporalseq_inst_n(seq, 0));
	}
	PG_FREE_IF_COPY(temp, 0);	
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_end_instant);
/**
 * @brief Returns the end instant of the temporal value
 */
PGDLLEXPORT Datum
temporal_end_instant(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporalinst_copy(temporali_inst_n((TemporalI *)temp, 
			((TemporalI *)temp)->count - 1));
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalinst_copy(temporalseq_inst_n((TemporalSeq *)temp, 
			((TemporalSeq *)temp)->count - 1));
	else /* temp->duration == TEMPORALS */
	{
		TemporalSeq *seq = temporals_seq_n((TemporalS *)temp, 
			((TemporalS *)temp)->count - 1);
		result = (Temporal *)temporalinst_copy(temporalseq_inst_n(seq, seq->count - 1));
	}	
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_instant_n);
/**
 * @brief Returns the n-th instant of the temporal value
 */
PGDLLEXPORT Datum
temporal_instant_n(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int n = PG_GETARG_INT32(1); /* Assume 1-based */
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
	{
		if (n == 1)
			result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	}
	else if (temp->duration == TEMPORALI) 
	{
		if (n >= 1 && n <= ((TemporalI *)temp)->count)
			result = (Temporal *)temporalinst_copy(
				temporali_inst_n((TemporalI *)temp, n - 1));
	}
	else if (temp->duration == TEMPORALSEQ) 
	{
		if (n >= 1 && n <= ((TemporalSeq *)temp)->count)
			result = (Temporal *)temporalinst_copy(
				temporalseq_inst_n((TemporalSeq *)temp, n - 1));
	}
	else /* temp->duration == TEMPORALS */
	{
		if (n >= 1 && n <= ((TemporalS *)temp)->totalcount)
		{
			TemporalInst *inst = temporals_instant_n((TemporalS *)temp, n);
			if (inst != NULL)
				result = (Temporal *)temporalinst_copy(inst);
		}
	}
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL) 
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_instants);
/**
 * @brief Returns the distinct instants of the temporal value as an array
 */
PGDLLEXPORT Datum
temporal_instants(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_instants_array((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = temporali_instants_array((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_instants_array((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = temporals_instants_array((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_ARRAYTYPE_P(result);
}

/**
 * @brief Returns the start timestamp of the temporal value
 *		(dispatch function)
 */
TimestampTz
temporal_start_timestamp_internal(const Temporal *temp)
{
	TimestampTz result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = ((TemporalInst *)temp)->t;
	else if (temp->duration == TEMPORALI) 
		result = temporali_inst_n((TemporalI *)temp, 0)->t;
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_start_timestamp((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = temporals_start_timestamp((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_start_timestamp);
/**
 * @brief Returns the start timestamp of the temporal value
 */
PGDLLEXPORT Datum
temporal_start_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz result = temporal_start_timestamp_internal(temp);
	PG_FREE_IF_COPY(temp, 0);	
	PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(temporal_end_timestamp);
/**
 * @brief Returns the end timestamp of the temporal value
 */
PGDLLEXPORT Datum
temporal_end_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = ((TemporalInst *)temp)->t;
	else if (temp->duration == TEMPORALI) 
		result = temporali_inst_n((TemporalI *)temp, ((TemporalI *)temp)->count - 1)->t;
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_end_timestamp((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = temporals_end_timestamp((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(temporal_num_timestamps);
/**
 * @brief Returns the number of distinct timestamps of the temporal value
 */
PGDLLEXPORT Datum
temporal_num_timestamps(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = 1;
	else if (temp->duration == TEMPORALI) 
		result = ((TemporalI *)temp)->count;
	else if (temp->duration == TEMPORALSEQ) 
		result = ((TemporalSeq *)temp)->count;
	else /* temp->duration == TEMPORALS */
		result = temporals_num_timestamps((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_timestamp_n);
/**
 * @brief Returns the n-th distinct timestamp of the temporal value
 */
PGDLLEXPORT Datum
temporal_timestamp_n(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int n = PG_GETARG_INT32(1); /* Assume 1-based */
	TimestampTz result;
	bool found = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
	{
		if (n == 1)
		{
			found = true;
			result = ((TemporalInst *)temp)->t;
		}
	}
	else if (temp->duration == TEMPORALI) 
	{
		if (n >= 1 && n <= ((TemporalI *)temp)->count)
		{
			found = true;
			result = (temporali_inst_n((TemporalI *)temp, n - 1))->t;
		}
	}
	else if (temp->duration == TEMPORALSEQ) 
	{
		if (n >= 1 && n <= ((TemporalSeq *)temp)->count)
		{
			found = true;
			result = (temporalseq_inst_n((TemporalSeq *)temp, n - 1))->t;
		}
	}
	else /* temp->duration == TEMPORALS */
		found = temporals_timestamp_n((TemporalS *)temp, n, &result);
	PG_FREE_IF_COPY(temp, 0);
	if (!found) 
		PG_RETURN_NULL();
	PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(temporal_timestamps);
/**
 * @brief Returns the distinct timestamps of the temporal value as an array
 */
PGDLLEXPORT Datum
temporal_timestamps(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_timestamps((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = temporali_timestamps((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_timestamps((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = temporals_timestamps((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(temporal_shift);
/**
 * @brief Shift the time span of the temporal value by the interval
 */
PGDLLEXPORT Datum
temporal_shift(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Interval *interval = PG_GETARG_INTERVAL_P(1);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_shift((TemporalInst *)temp, interval);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_shift((TemporalI *)temp, interval);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_shift((TemporalSeq *)temp, interval);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_shift((TemporalS *)temp, interval);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Ever/always comparison operators
 *****************************************************************************/

/**
 * @brief Returns true if the temporal value is ever equal to the base value
 *		(internal function)
 */
bool
temporal_ever_eq_internal(const Temporal *temp, Datum value)
{
	bool result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_ever_eq((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI) 
		result = temporali_ever_eq((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_ever_eq((TemporalSeq *)temp, value);
	else /* temp->duration == TEMPORALS */
		result = temporals_ever_eq((TemporalS *)temp, value);
	return result;
}

/**
 * @brief Returns true if the temporal value is always equal to the base value
 *		(internal function)
 */
bool
temporal_always_eq_internal(const Temporal *temp, Datum value)
{
	bool result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_always_eq((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI)
		result = temporali_always_eq((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_always_eq((TemporalSeq *)temp, value);
	else /* temp->duration == TEMPORALS */
		result = temporals_always_eq((TemporalS *)temp, value);
	return result;
}

/**
 * @brief Returns true if the temporal value is ever less than the base value
 *		(internal function)
 */
bool
temporal_ever_lt_internal(const Temporal *temp, Datum value)
{
	bool result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_ever_lt((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI)
		result = temporali_ever_lt((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_ever_lt((TemporalSeq *)temp, value);
	else /* temp->duration == TEMPORALS */
		result = temporals_ever_lt((TemporalS *)temp, value);
	return result;
}

/**
 * @brief Returns true if the temporal value is always less than the base value
 *		(internal function)
 */
bool
temporal_always_lt_internal(const Temporal *temp, Datum value)
{
	bool result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_always_lt((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI)
		result = temporali_always_lt((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_always_lt((TemporalSeq *)temp, value);
	else /* temp->duration == TEMPORALS */
		result = temporals_always_lt((TemporalS *)temp, value);
	return result;
}

/**
 * @brief Returns true if the temporal value is ever less than or equal to 
 *		the base value (internal function)
 */
bool
temporal_ever_le_internal(const Temporal *temp, Datum value)
{
	bool result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_ever_le((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI)
		result = temporali_ever_le((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_ever_le((TemporalSeq *)temp, value);
	else /* temp->duration == TEMPORALS */
		result = temporals_ever_le((TemporalS *)temp, value);
	return result;
}

/**
 * @brief Returns true if the temporal value is always less than or equal to
 *		the base value (internal function)
 */
bool
temporal_always_le_internal(const Temporal *temp, Datum value)
{
	bool result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_always_le((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI)
		result = temporali_always_le((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_always_le((TemporalSeq *)temp, value);
	else /* temp->duration == TEMPORALS */
		result = temporals_always_le((TemporalS *)temp, value);
	return result;
}

/*****************************************************************************/

/**
 * @brief Generic function for the temporal ever/always comparison operators
 */
Datum
temporal_ev_al_comp(FunctionCallInfo fcinfo, 
	bool (*func)(const Temporal *, Datum))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	bool result = func(temp, value);
	PG_FREE_IF_COPY(temp, 0);
	DATUM_FREE_IF_COPY(value, temp->valuetypid, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
 
PG_FUNCTION_INFO_V1(temporal_ever_eq);
/**
 * @brief Returns true if the temporal value is ever equal to the base value
 */
PGDLLEXPORT Datum
temporal_ever_eq(PG_FUNCTION_ARGS)
{
	return temporal_ev_al_comp(fcinfo, &temporal_ever_eq_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_eq);
/**
 * @brief Returns true if the temporal value is always equal to the base value
 */
PGDLLEXPORT Datum
temporal_always_eq(PG_FUNCTION_ARGS)
{
	return temporal_ev_al_comp(fcinfo, &temporal_always_eq_internal);
}

PG_FUNCTION_INFO_V1(temporal_ever_ne);
/**
 * @brief Returns true if the temporal value is ever different from the base value
 */
PGDLLEXPORT Datum
temporal_ever_ne(PG_FUNCTION_ARGS)
{
	return ! temporal_ev_al_comp(fcinfo, &temporal_always_eq_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_ne);
/**
 * @brief Returns true if the temporal value is always different from the base value
 */
PGDLLEXPORT Datum
temporal_always_ne(PG_FUNCTION_ARGS)
{
	return ! temporal_ev_al_comp(fcinfo, &temporal_ever_eq_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_ever_lt);
/**
 * @brief Returns true if the temporal value is ever less than the base value
 */
PGDLLEXPORT Datum
temporal_ever_lt(PG_FUNCTION_ARGS)
{
	return temporal_ev_al_comp(fcinfo, &temporal_ever_lt_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_lt);
/**
 * @brief Returns true if the temporal value is always less than the base value
 */
PGDLLEXPORT Datum
temporal_always_lt(PG_FUNCTION_ARGS)
{
	return temporal_ev_al_comp(fcinfo, &temporal_always_lt_internal);
}

PG_FUNCTION_INFO_V1(temporal_ever_le);
/**
 * @brief Returns true if the temporal value is ever less than or equal to the base value
 */
PGDLLEXPORT Datum
temporal_ever_le(PG_FUNCTION_ARGS)
{
	return temporal_ev_al_comp(fcinfo, &temporal_ever_le_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_le);
/**
 * @brief Returns true if the temporal value is always less than or equal to the base value
 */
PGDLLEXPORT Datum
temporal_always_le(PG_FUNCTION_ARGS)
{
	return temporal_ev_al_comp(fcinfo, &temporal_always_le_internal);
}

PG_FUNCTION_INFO_V1(temporal_ever_gt);
/**
 * @brief Returns true if the temporal value is ever greater than the base value
 */
PGDLLEXPORT Datum
temporal_ever_gt(PG_FUNCTION_ARGS)
{
	return ! temporal_ev_al_comp(fcinfo, &temporal_always_le_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_gt);
/**
 * @brief Returns true if the temporal value is always greater than the base value
 */
PGDLLEXPORT Datum
temporal_always_gt(PG_FUNCTION_ARGS)
{
	return ! temporal_ev_al_comp(fcinfo, &temporal_ever_le_internal);
}

PG_FUNCTION_INFO_V1(temporal_ever_ge);
/**
 * @brief Returns true if the temporal value is ever greater than or equal 
 *		to the base value
 */
PGDLLEXPORT Datum
temporal_ever_ge(PG_FUNCTION_ARGS)
{
	return ! temporal_ev_al_comp(fcinfo, &temporal_always_lt_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_ge);
/**
 * @brief Returns true if the temporal value is always greater than or equal 
 *		to the base value
 */
PGDLLEXPORT Datum
temporal_always_ge(PG_FUNCTION_ARGS)
{
	return ! temporal_ev_al_comp(fcinfo, &temporal_ever_lt_internal);
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/**
 * @brief Restricts the temporal value to the base value
 *		(dispatch function)
 */
Temporal *
temporal_at_value_internal(const Temporal *temp, Datum value)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_at_value(
			(TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_at_value(
			(TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_at_value(
			(TemporalSeq *)temp, value);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_at_value(
			(TemporalS *)temp, value);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_at_value);
/**
 * @brief Restricts the temporal value to the base value
 */
PGDLLEXPORT Datum
temporal_at_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Temporal *result = temporal_at_value_internal(temp, value);
	PG_FREE_IF_COPY(temp, 0);
	DATUM_FREE_IF_COPY(value, valuetypid, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/**
 * @brief Restricts the temporal value to the complement of the base value
 *		(dispatch function)
 */
Temporal *
temporal_minus_value_internal(const Temporal *temp, Datum value)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_minus_value(
			(TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_minus_value(
			(TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_minus_value(
			(TemporalSeq *)temp, value);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_minus_value(
			(TemporalS *)temp, value);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_minus_value);
/**
 * @brief Restricts the temporal value to the complement of the base value
 */
PGDLLEXPORT Datum
temporal_minus_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Temporal *result = temporal_minus_value_internal(temp, value);
	PG_FREE_IF_COPY(temp, 0);
	DATUM_FREE_IF_COPY(value, valuetypid, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/**
 * @brief Restricts the temporal value to the array of base values
 *		(dispatch function)
 */
Temporal *
temporal_at_values_internal(const Temporal *temp, Datum *values, int count)
{
	Oid valuetypid = temp->valuetypid;
	datumarr_sort(values, count, valuetypid);
	int count1 = datumarr_remove_duplicates(values, count, valuetypid);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_at_values(
			(TemporalInst *)temp, values, count1);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_at_values(
			(TemporalI *)temp, values, count1);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_at_values(
			(TemporalSeq *)temp, values, count1);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_at_values(
			(TemporalS *)temp, values, count1);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_at_values);
/**
 * @brief Restricts the temporal value to the array of base values
 */
PGDLLEXPORT Datum
temporal_at_values(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
	int count;
	Datum *values = datumarr_extract(array, &count);
	if (count == 0)
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(array, 1);
		PG_RETURN_NULL();	
	}

	Temporal *result = temporal_at_values_internal(temp, values, count);

	pfree(values);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(array, 1);
	if (result == NULL) 
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/**
 * @brief Restricts the temporal value to the complement of the array of base values
 *		(dispatch function)
 */
Temporal *
temporal_minus_values_internal(const Temporal *temp, Datum *values, int count)
{
	Oid valuetypid = temp->valuetypid;
	datumarr_sort(values, count, valuetypid);
	int count1 = datumarr_remove_duplicates(values, count, valuetypid);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_minus_values(
			(TemporalInst *)temp, values, count1);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_minus_values(
			(TemporalI *)temp, values, count1);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_minus_values(
			(TemporalSeq *)temp, values, count1);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_minus_values(
			(TemporalS *)temp, values, count1);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_minus_values);
/**
 * @brief Restricts the temporal value to the complement of the array of base values
 */
PGDLLEXPORT Datum
temporal_minus_values(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
	int count;
	Datum *values = datumarr_extract(array, &count);
	if (count == 0)
	{
		Temporal *result = temporal_copy(temp);
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(array, 1);
		PG_RETURN_POINTER(result);
	}

	Temporal *result = temporal_minus_values_internal(temp, values, count);

	pfree(values);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(array, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/**
 * @brief Restricts the temporal value to the range of base values
 *		(dispatch function)
 */
Temporal *
tnumber_at_range_internal(const Temporal *temp, RangeType *range)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tnumberinst_at_range(
			(TemporalInst *)temp, range);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tnumberi_at_range(
			(TemporalI *)temp, range);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tnumberseq_at_range(
			(TemporalSeq *)temp, range);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tnumbers_at_range(
			(TemporalS *)temp, range);
	return result;
}

PG_FUNCTION_INFO_V1(tnumber_at_range);
/**
 * @brief Restricts the temporal value to the range of base values
 */
PGDLLEXPORT Datum
tnumber_at_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
#if MOBDB_PGSQL_VERSION < 110000
	RangeType  *range = PG_GETARG_RANGE(1);
#else
	RangeType  *range = PG_GETARG_RANGE_P(1);
#endif
	Temporal *result = tnumber_at_range_internal(temp, range);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/**
 * @brief Restricts the temporal value to the complement of the range of base values
 *		(dispatch function)
 */
Temporal *
tnumber_minus_range_internal(const Temporal *temp, RangeType *range)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tnumberinst_minus_range(
			(TemporalInst *)temp, range);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tnumberi_minus_range(
			(TemporalI *)temp, range);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tnumberseq_minus_range(
			(TemporalSeq *)temp, range);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tnumbers_minus_range(
			(TemporalS *)temp, range);
	return result;
}

PG_FUNCTION_INFO_V1(tnumber_minus_range);
/**
 * @brief Restricts the temporal value to the complement of the range of base values
 */
PGDLLEXPORT Datum
tnumber_minus_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
#if MOBDB_PGSQL_VERSION < 110000
	RangeType  *range = PG_GETARG_RANGE(1);
#else
	RangeType  *range = PG_GETARG_RANGE_P(1);
#endif
	Temporal *result = tnumber_minus_range_internal(temp, range);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tnumber_at_ranges);
/**
 * @brief Restricts the temporal value to the array of ranges of base values
 */
PGDLLEXPORT Datum
tnumber_at_ranges(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
	int count;
	RangeType **ranges = rangearr_extract(array, &count);
	if (count == 0)
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(array, 1);
		PG_RETURN_NULL();	
	}

	RangeType **normranges = ranges;
	int newcount = count;
	if (count > 1)
		normranges = rangearr_normalize(ranges, &newcount);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tnumberinst_at_ranges(
			(TemporalInst *)temp, normranges, newcount);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tnumberi_at_ranges(
			(TemporalI *)temp, normranges, newcount);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)tnumberseq_at_ranges(
			(TemporalSeq *)temp, normranges, newcount);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tnumbers_at_ranges(
			(TemporalS *)temp, normranges, newcount);

	pfree(ranges);
	if (count > 1)
	{
		for (int i = 0; i < newcount; i++)
			pfree(normranges[i]);
		pfree(normranges);
	}
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(array, 1);
	if (result == NULL) 
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tnumber_minus_ranges);
/**
 * @brief Restricts the temporal value to the complement of the array of ranges
 *		of base values
 */
PGDLLEXPORT Datum
tnumber_minus_ranges(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
	int count;
	RangeType **ranges = rangearr_extract(array, &count);
	if (count == 0)
	{
		Temporal *result = temporal_copy(temp);
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(array, 1);
		PG_RETURN_POINTER(result);
	}

	RangeType **normranges = ranges;
	int newcount = count;
	if (count > 1)
		normranges = rangearr_normalize(ranges, &newcount);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tnumberinst_minus_ranges((TemporalInst *)temp,
			normranges, newcount);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tnumberi_minus_ranges((TemporalI *)temp,
			normranges, newcount);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)tnumberseq_minus_ranges((TemporalSeq *)temp,
			normranges, newcount);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tnumbers_minus_ranges((TemporalS *)temp,
			normranges, newcount);

	pfree(ranges);
	if (count > 1)
	{
		for (int i = 0; i < newcount; i++)
			pfree(normranges[i]);
		pfree(normranges);
	}
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(array, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/**
 * @brief Restricts the temporal value to the minimum base value
 *		(dispatch function)
 */
Temporal *
temporal_at_min_internal(const Temporal *temp)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_at_min((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_at_min((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_at_min((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_at_min);
/**
 * @brief Restricts the temporal value to the minimum base value
 */
PGDLLEXPORT Datum
temporal_at_min(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = temporal_at_min_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_minus_min);
/**
 * @brief Restricts the temporal value to the complement of the minimum
 *		base value
 */
PGDLLEXPORT Datum
temporal_minus_min(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		;
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_minus_min((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_minus_min((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_minus_min((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}
 
PG_FUNCTION_INFO_V1(temporal_at_max);
/**
 * @brief Restricts the temporal value to the maximum base value
 */ 
PGDLLEXPORT Datum
temporal_at_max(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_at_max((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_at_max((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_at_max((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_minus_max);
/**
 * @brief Restricts the temporal value to the complement of a maximum
 *		base value
 */ 
PGDLLEXPORT Datum
temporal_minus_max(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = NULL;
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_minus_max((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_minus_max((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_minus_max((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/**
 * @brief Restricts the temporal value to the timestamp
 *		(dispatch function)
 */ 
TemporalInst *
temporal_at_timestamp_internal(const Temporal *temp, TimestampTz t)
{
	TemporalInst *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_at_timestamp((TemporalInst *)temp, t);
	else if (temp->duration == TEMPORALI) 
		result = temporali_at_timestamp((TemporalI *)temp, t);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_at_timestamp((TemporalSeq *)temp, t);
	else /* temp->duration == TEMPORALS */
		result = temporals_at_timestamp((TemporalS *)temp, t);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_at_timestamp);
/**
 * @brief Restricts the temporal value to the timestamp
 */ 
PGDLLEXPORT Datum
temporal_at_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	TemporalInst *result = temporal_at_timestamp_internal(temp, t);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();	
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_minus_timestamp);
/**
 * @brief Restricts the temporal value to the complement of the timestamp
 */ 
PGDLLEXPORT Datum
temporal_minus_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_minus_timestamp((TemporalInst *)temp, t);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_minus_timestamp((TemporalI *)temp, t);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_minus_timestamp((TemporalSeq *)temp, t);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_minus_timestamp((TemporalS *)temp, t);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_value_at_timestamp);
/**
 * @brief Returns the base value of the temporal value at the timestamp
 */
PGDLLEXPORT Datum
temporal_value_at_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	bool found = false;
	Datum result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		found = temporalinst_value_at_timestamp((TemporalInst *)temp, t, &result);
	else if (temp->duration == TEMPORALI) 
		found = temporali_value_at_timestamp((TemporalI *)temp, t, &result);
	else if (temp->duration == TEMPORALSEQ) 
		found = temporalseq_value_at_timestamp((TemporalSeq *)temp, t, &result);
	else /* temp->duration == TEMPORALS */
		found = temporals_value_at_timestamp((TemporalS *)temp, t, &result);
	PG_FREE_IF_COPY(temp, 0);
	if (!found)
		PG_RETURN_NULL();
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(temporal_at_timestampset);
/**
 * @brief Restricts the temporal value to the timestamp set
 */
PGDLLEXPORT Datum
temporal_at_timestampset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_at_timestampset(
			(TemporalInst *)temp, ts);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_at_timestampset(
			(TemporalI *)temp, ts);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_at_timestampset(
			(TemporalSeq *)temp, ts);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_at_timestampset(
			(TemporalS *)temp, ts);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_minus_timestampset);
/**
 * @brief Restricts the temporal value to the complement of the timestamp set
 */
PGDLLEXPORT Datum
temporal_minus_timestampset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_minus_timestampset(
			(TemporalInst *)temp, ts);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_minus_timestampset(
			(TemporalI *)temp, ts);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_minus_timestampset(
			(TemporalSeq *)temp, ts);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_minus_timestampset(
			(TemporalS *)temp, ts);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
	if (result == NULL)
		PG_RETURN_NULL();	
	PG_RETURN_POINTER(result);
}

/**
 * @brief Restricts the temporal value to the period
 *		(dispatch function)
 */
Temporal *
temporal_at_period_internal(const Temporal *temp, const Period *p)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_at_period(
			(TemporalInst *)temp, p);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)temporali_at_period(
			(TemporalI *)temp, p);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_at_period(
			(TemporalSeq *)temp, p);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_at_period(
			(TemporalS *)temp, p);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_at_period);
/**
 * @brief Restricts the temporal value to the period
 */
PGDLLEXPORT Datum
temporal_at_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Temporal *result = temporal_at_period_internal(temp, p);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();	
	PG_RETURN_POINTER(result);
}

/**
 * @brief Restricts the temporal value to the complement of the period
 *		(dispatch function)
 */
Temporal *
temporal_minus_period_internal(const Temporal *temp, const Period *p)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_minus_period(
			(TemporalInst *)temp, p);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)temporali_minus_period(
			(TemporalI *)temp, p);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_minus_period(
			(TemporalSeq *)temp, p);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_minus_period(
			(TemporalS *)temp, p);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_minus_period);
/**
 * @brief Restricts the temporal value to the complement of the period
 */
PGDLLEXPORT Datum
temporal_minus_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Temporal *result = temporal_minus_period_internal(temp, p);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/**
 * @brief Restricts the temporal value to the period set
 *		(dispatch function)
 */
Temporal *
temporal_at_periodset_internal(const Temporal *temp, const PeriodSet *ps)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_at_periodset(
			(TemporalInst *)temp, ps);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_at_periodset(
			(TemporalI *)temp, ps);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_at_periodset(
			(TemporalSeq *)temp, ps);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_at_periodset(
			(TemporalS *)temp, ps);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_at_periodset);
/**
 * @brief Restricts the temporal value to the period set
 */
PGDLLEXPORT Datum
temporal_at_periodset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Temporal *result = temporal_at_periodset_internal(temp, ps);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/**
 * @brief Restricts the temporal value to the complement of the period set
 *		(dispatch function)
 */
Temporal *
temporal_minus_periodset_internal(const Temporal *temp, const PeriodSet *ps)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_minus_periodset(
			(TemporalInst *)temp, ps);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)temporali_minus_periodset(
			(TemporalI *)temp, ps);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_minus_periodset(
			(TemporalSeq *)temp, ps);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)temporals_minus_periodset(
			(TemporalS *)temp, ps);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_minus_periodset);
/**
 * @brief Restricts the temporal value to the complement of the period set
 */
PGDLLEXPORT Datum
temporal_minus_periodset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Temporal *result = temporal_minus_periodset_internal(temp, ps);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_intersects_timestamp);
/**
 * @brief Returns true if the temporal value intersects the timestamp
 */
PGDLLEXPORT Datum
temporal_intersects_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	bool result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_intersects_timestamp((TemporalInst *)temp, t);
	else if (temp->duration == TEMPORALI) 
		result = temporali_intersects_timestamp((TemporalI *)temp, t);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_intersects_timestamp((TemporalSeq *)temp, t);
	else /* temp->duration == TEMPORALS */
		result = temporals_intersects_timestamp((TemporalS *)temp, t);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(temporal_intersects_timestampset);
/**
 * @brief Returns true if the temporal value intersects the timestamp set
 */
PGDLLEXPORT Datum
temporal_intersects_timestampset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	bool result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_intersects_timestampset((TemporalInst *)temp, ts);
	else if (temp->duration == TEMPORALI) 
		result = temporali_intersects_timestampset((TemporalI *)temp, ts);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_intersects_timestampset((TemporalSeq *)temp, ts);
	else /* temp->duration == TEMPORALS */
		result = temporals_intersects_timestampset((TemporalS *)temp, ts);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(temporal_intersects_period);
/**
 * @brief Returns true if the temporal value intersects the period
 */
PGDLLEXPORT Datum
temporal_intersects_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	bool result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_intersects_period((TemporalInst *)temp, p);
	else if (temp->duration == TEMPORALI) 
		result = temporali_intersects_period((TemporalI *)temp, p);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_intersects_period((TemporalSeq *)temp, p);
	else /* temp->duration == TEMPORALS */
		result = temporals_intersects_period((TemporalS *)temp, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(temporal_intersects_periodset);
/**
 * @brief Returns true if the temporal value intersects the period set
 */
PGDLLEXPORT Datum
temporal_intersects_periodset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	bool result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_intersects_periodset((TemporalInst *)temp, ps);
	else if (temp->duration == TEMPORALI) 
		result = temporali_intersects_periodset((TemporalI *)temp, ps);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_intersects_periodset((TemporalSeq *)temp, ps);
	else /* temp->duration == TEMPORALS */
		result = temporals_intersects_periodset((TemporalS *)temp, ps);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Local aggregate functions 
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnumber_integral);
/**
 * @brief Returns the integral (area under the curve) of the temporal
 *		numeric value
 */
PGDLLEXPORT Datum
tnumber_integral(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	double result = 0.0; 
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST || temp->duration == TEMPORALI)
		;
	else if (temp->duration == TEMPORALSEQ)
		result = tnumberseq_integral((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = tnumbers_integral((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(tnumber_twavg);
/**
 * @brief Returns the time-weighted average of the temporal numeric value
 */
PGDLLEXPORT Datum
tnumber_twavg(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	double result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = datum_double(temporalinst_value((TemporalInst *)temp), 
			temp->valuetypid);
	else if (temp->duration == TEMPORALI)
		result = tnumberi_twavg((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = tnumberseq_twavg((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = tnumbers_twavg((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * @brief Returns -1, 0, or 1 depending on whether the first temporal value 
 *		is less than, equal, or greater than the second temporal value
 *		(internal function)
 * @note Function used for B-tree comparison
 */
static int
temporal_cmp_internal(const Temporal *temp1, const Temporal *temp2)
{
	assert(temp1->valuetypid == temp2->valuetypid);

	/* Compare bounding period */
	Period p1, p2;
	temporal_period(&p1, temp1);
	temporal_period(&p2, temp2);
	int result = period_cmp_internal(&p1, &p2);
	if (result)
		return result;

	/* Compare bounding box */
	union bboxunion box1, box2;
	memset(&box1, 0, sizeof(bboxunion));
	memset(&box2, 0, sizeof(bboxunion));
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	result = temporal_bbox_cmp(&box1, &box2, temp1->valuetypid);
	if (result)
		return result;

	/* If both are of the same duration use the specific comparison */
	if (temp1->duration == temp2->duration)
	{
		ensure_valid_duration(temp1->duration);
		if (temp1->duration == TEMPORALINST) 
			return temporalinst_cmp((TemporalInst *)temp1, (TemporalInst *)temp2);
		else if (temp1->duration == TEMPORALI) 
			return temporali_cmp((TemporalI *)temp1, (TemporalI *)temp2);
		else if (temp1->duration == TEMPORALSEQ) 
			return temporalseq_cmp((TemporalSeq *)temp1, (TemporalSeq *)temp2);
		else /* temp1->duration == TEMPORALS */
			return temporals_cmp((TemporalS *)temp1, (TemporalS *)temp2);
	}
	
	/* Use the hash comparison */
	uint32 hash1 = temporal_hash_internal(temp1);
	uint32 hash2 = temporal_hash_internal(temp2);
	if (hash1 < hash2)
		return -1;
	else if (hash1 > hash2)
		return 1;
	
	/* Compare memory size */
	size_t size1 = VARSIZE(DatumGetPointer(temp1));
	size_t size2 = VARSIZE(DatumGetPointer(temp2));
	if (size1 < size2)
		return -1;
	else if (size1 > size2)
		return 1;

	/* Finally compare temporal type */
	if (temp1->duration < temp2->duration)
		return -1;
	else if (temp1->duration > temp2->duration)
		return 1;
	else
		return 0;
}

PG_FUNCTION_INFO_V1(temporal_cmp);
/**
 * @brief Returns -1, 0, or 1 depending on whether the first temporal value 
 *		is less than, equal, or greater than the second temporal value
 * @note Function used for B-tree comparison
 */
PGDLLEXPORT Datum
temporal_cmp(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	int result = temporal_cmp_internal(temp1, temp2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_INT32(result);
}

/**
 * @brief Returns true if the two temporal values are equal 
 *		(internal function)
 * @note The internal B-tree comparator is not used to increase efficiency
 */
static bool
temporal_eq_internal(const Temporal *temp1, const Temporal *temp2)
{
	assert(temp1->valuetypid == temp2->valuetypid);
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);

	/* If both are of the same duration use the specific equality */
	if (temp1->duration == temp2->duration)
	{
		if (temp1->duration == TEMPORALINST) 
			return temporalinst_eq((TemporalInst *)temp1, (TemporalInst *)temp2);
		else if (temp1->duration == TEMPORALI) 
			return temporali_eq((TemporalI *)temp1, (TemporalI *)temp2);
		else if (temp1->duration == TEMPORALSEQ) 
			return temporalseq_eq((TemporalSeq *)temp1, (TemporalSeq *)temp2);
		else /* temp1->duration == TEMPORALS */
			return temporals_eq((TemporalS *)temp1, (TemporalS *)temp2);
	}	
	
	/* Different duration */
	if (temp1->duration > temp2->duration)
	{
		Temporal *temp = (Temporal *) temp1;
		temp1 = temp2;
		temp2 = temp;
	}
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI)
	{
		TemporalInst *inst = (TemporalInst *)temp1;
		TemporalI *ti = (TemporalI *)temp2;
		if (ti->count != 1) 
			return false;
		TemporalInst *inst1 = temporali_inst_n(ti, 0);
		return temporalinst_eq(inst, inst1);	
	}
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ)
	{
		TemporalInst *inst = (TemporalInst *)temp1;
		TemporalSeq *seq = (TemporalSeq *)temp2; 
		if (seq->count != 1) 
			return false;
		TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
		return temporalinst_eq(inst, inst1);	
	}
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS)
	{
		TemporalInst *inst = (TemporalInst *)temp1;
		TemporalS *ts = (TemporalS *)temp2; 
		if (ts->count != 1) 
			return false;
		TemporalSeq *seq = temporals_seq_n(ts, 0);
		if (seq->count != 1) 
			return false;
		TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
		return temporalinst_eq(inst, inst1);	
	}
	if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ)
	{
		TemporalI *ti = (TemporalI *)temp1; 
		TemporalSeq *seq = (TemporalSeq *)temp2; 
		if (ti->count != 1 || seq->count != 1) 
			return false;
		TemporalInst *inst1 = temporali_inst_n(ti, 0);
		TemporalInst *inst2 = temporalseq_inst_n(seq, 0);
		return temporalinst_eq(inst1, inst2);	
	}
	if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS)
	{
		TemporalI *ti = (TemporalI *)temp1; 
		TemporalS *ts = (TemporalS *)temp2; 
		for (int i = 0; i < ti->count; i ++)
		{
			TemporalSeq *seq = temporals_seq_n(ts, i);
			if (seq->count != 1) 
				return false;
			TemporalInst *inst1 = temporali_inst_n(ti, i);
			TemporalInst *inst2 = temporalseq_inst_n(seq, 0);
			if (!temporalinst_eq(inst1, inst2))
				return false;	
		}
		return true;
	}
	if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS)
	{
		TemporalSeq *seq = (TemporalSeq *)temp1; 
		TemporalS *ts = (TemporalS *)temp2; 
		if (ts->count != 1) 
			return false;
		TemporalSeq *seq1 = temporals_seq_n(ts, 0);
		return temporalseq_eq(seq, seq1);	
	}
	return false; /* make compiler quiet */
}

PG_FUNCTION_INFO_V1(temporal_eq);
/**
 * @brief Returns true if the two temporal values are equal
 */
PGDLLEXPORT Datum
temporal_eq(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	bool result = temporal_eq_internal(temp1, temp2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/**
 * @brief Returns true if the two temporal values are different
 *		(internal function)
 */
bool
temporal_ne_internal(Temporal *temp1, Temporal *temp2)
{
	return !temporal_eq_internal(temp1, temp2);
}

PG_FUNCTION_INFO_V1(temporal_ne);
/**
 * @brief Returns true if the two temporal values are different
 */
PGDLLEXPORT Datum
temporal_ne(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	bool result = temporal_ne_internal(temp1, temp2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/* Comparison operators using the internal B-tree comparator */

PG_FUNCTION_INFO_V1(temporal_lt);
/**
 * @brief Returns true if the first temporal value is less than the second one
 */
PGDLLEXPORT Datum
temporal_lt(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	int cmp = temporal_cmp_internal(temp1, temp2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (cmp < 0)
		PG_RETURN_BOOL(true);
	else
		PG_RETURN_BOOL(false);
}

PG_FUNCTION_INFO_V1(temporal_le);
/**
 * @brief Returns true if the first temporal value is less than or equal to
 *		the second one
 */
PGDLLEXPORT Datum
temporal_le(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	int cmp = temporal_cmp_internal(temp1, temp2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (cmp == 0)
		PG_RETURN_BOOL(true);
	else
		PG_RETURN_BOOL(false);
}

PG_FUNCTION_INFO_V1(temporal_ge);
/**
 * @brief Returns true if the first temporal value is greater than or equal to
 *		the second one
 */
PGDLLEXPORT Datum
temporal_ge(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	int cmp = temporal_cmp_internal(temp1, temp2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (cmp >= 0)
		PG_RETURN_BOOL(true);
	else
		PG_RETURN_BOOL(false);
}

PG_FUNCTION_INFO_V1(temporal_gt);
/**
 * @brief Returns true if the first temporal value is greater than the second one
 */
PGDLLEXPORT Datum 
temporal_gt(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	int cmp = temporal_cmp_internal(temp1, temp2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (cmp > 0)
		PG_RETURN_BOOL(true);
	else
		PG_RETURN_BOOL(false);
}

/*****************************************************************************
 * Functions for defining hash index
 *****************************************************************************/

/**
 * @brief Returns the hash value of the temporal value
 *		(dispatch function)
 */
uint32 
temporal_hash_internal(const Temporal *temp)
{
	uint32 result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_hash((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = temporali_hash((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_hash((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = temporals_hash((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_hash);
/**
 * @brief Returns the hash value of the temporal value
 */
PGDLLEXPORT Datum 
temporal_hash(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	uint32 result = temporal_hash_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_UINT32(result);
}

/*****************************************************************************/
