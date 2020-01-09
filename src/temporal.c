/*****************************************************************************
 *
 * temporal.c
 *	Basic functionss of any duration.
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

#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "temporal_boxops.h"
#include "temporal_parser.h"
#include "rangetypes_ext.h"

/*****************************************************************************
 * Typmod 
 *****************************************************************************/

static char *temporalTypeName[] =
{
	"Unknown",
	"Instant",
	"InstantSet",
	"Sequence",
	"SequenceSet"
};

struct temporal_duration_struct temporal_duration_struct_array[] =
{
	{"UNKNOWN", TEMPORAL},
	{"INSTANT", TEMPORALINST},
	{"INSTANTSET", TEMPORALI},
	{"SEQUENCE", TEMPORALSEQ},
	{"SEQUENCESET", TEMPORALS},
};

const char *
temporal_duration_name(uint8_t type)
{
	if (type > 4)
		return "Invalid temporal type";
	return temporalTypeName[(int)type];
}

bool
temporal_duration_from_string(const char *str, uint8_t *type)
{
	char *tmpstr;
	size_t tmpstartpos, tmpendpos;
	size_t i;

	/* Initialize */
	*type = 0;
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
		if (len == strlen(temporal_duration_struct_array[i].typename) && 
			!strcasecmp(tmpstr, temporal_duration_struct_array[i].typename))
		{
			*type = temporal_duration_struct_array[i].type;
			pfree(tmpstr);
			return true;
		}
	}
	pfree(tmpstr);
	return false;
}

Temporal*
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
 * Internal functions
 *****************************************************************************/

/* Copy a Temporal */
Temporal *
temporal_copy(Temporal *temp)
{
	Temporal *result = (Temporal *)palloc0(VARSIZE(temp));
	memcpy(result, temp, VARSIZE(temp));
	return result;
}

/* 
 * intersection two temporal values
 * Returns false if the values do not overlap on time
 * intersection values are returned as last two arguments
 */

bool
intersection_temporal_temporal(Temporal *temp1, Temporal *temp2,
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

/* 
 * Synchronize two temporal values
 * Returns false if the values do not overlap on time
 * Synchronized values are returned as last two arguments
 */

bool
synchronize_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Temporal **synctemp1, Temporal **synctemp2,  bool crossings)
{
	bool result = false;
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALINST) 
		result = intersection_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)synctemp1, (TemporalInst **)synctemp2);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI) 
		result = intersection_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, 
			(TemporalInst **)synctemp1, (TemporalInst **)synctemp2);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ) 
		result = intersection_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, 
			(TemporalInst **)synctemp1, (TemporalInst **)synctemp2);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS) 
		result = intersection_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, 
			(TemporalInst **)synctemp1, (TemporalInst **)synctemp2);
	
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALINST) 
		result = intersection_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)synctemp1, (TemporalInst **)synctemp2);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALI) 
		result = intersection_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2,
			(TemporalI **)synctemp1, (TemporalI **)synctemp2);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ) 
		result = intersection_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2,
			(TemporalI **)synctemp1, (TemporalI **)synctemp2);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS) 
		result = intersection_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, 
			(TemporalI **)synctemp1, (TemporalI **)synctemp2);
	
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALINST) 
		result = intersection_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)synctemp1, (TemporalInst **)synctemp2);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALI) 
		result = intersection_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2,
			(TemporalI **)synctemp1, (TemporalI **)synctemp2);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALSEQ) 
		result = synchronize_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2,
			(TemporalSeq **)synctemp1, (TemporalSeq **)synctemp2, crossings);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS) 
		result = synchronize_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			(TemporalS **)synctemp1, (TemporalS **)synctemp2, crossings);
	
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALINST) 
		result = intersection_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2,
			(TemporalInst **)synctemp1, (TemporalInst **)synctemp2);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALI) 
		result = intersection_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2,
			(TemporalI **)synctemp1, (TemporalI **)synctemp2);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALSEQ) 
		result = synchronize_temporals_temporalseq(
			(TemporalS *)temp1, (TemporalSeq *)temp2,
			(TemporalS **)synctemp1, (TemporalS **)synctemp2, crossings);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALS) 
		result = synchronize_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			(TemporalS **)synctemp1, (TemporalS **)synctemp2, crossings);

	return result;
}

/* Does the base type implies a linear interpolation? */

bool
linear_interpolation(Oid type)
{
	if (type == FLOAT8OID || type == type_oid(T_DOUBLE2) || 
		type == type_oid(T_DOUBLE3) || type == type_oid(T_DOUBLE4))
		return true;
#ifdef WITH_POSTGIS
	if (type == type_oid(T_GEOGRAPHY) || type == type_oid(T_GEOMETRY)) 
		return true;
#endif
	return false;
}

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

/* Obtain the typinfo for the temporal type from the catalog */

void
temporal_typinfo(Oid temptypid, Oid* valuetypid) 
{
	Oid catalog = RelnameGetRelid("pg_temporal");
	Relation rel = heap_open(catalog, AccessShareLock);
	TupleDesc tupDesc = rel->rd_att;
	ScanKeyData scandata;
	ScanKeyInit(&scandata, 1, BTEqualStrategyNumber, F_OIDEQ, 
		ObjectIdGetDatum(temptypid));
	HeapScanDesc scan = heap_beginscan_catalog(rel, 1, &scandata);
	HeapTuple tuple = heap_getnext(scan, ForwardScanDirection);
	bool isnull = false;
	if (HeapTupleIsValid(tuple)) 
		*valuetypid = DatumGetObjectId(heap_getattr(tuple, 2, tupDesc, &isnull));
	heap_endscan(scan);
	heap_close(rel, AccessShareLock);
	if (! HeapTupleIsValid(tuple) || isnull) 
		elog(ERROR, "type %u is not a temporal type", temptypid);
}

/*****************************************************************************
 * Oid functions
 *****************************************************************************/

/* 
 * Obtain the Oid of the range type from the Oid of the base type 
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
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOMETRY)) 
		result = type_oid(T_TGEOMPOINT);
	if (valuetypid == type_oid(T_GEOGRAPHY)) 
		result = type_oid(T_TGEOGPOINT);
#endif			
	return result;
}

/* 
 * Is the Oid a temporal type ? 
 * Function used in particular in the indexes.
 */
bool
temporal_type_oid(Oid temptypid)
{
	if (temptypid == type_oid(T_TBOOL) ||
		temptypid == type_oid(T_TINT) ||
		temptypid == type_oid(T_TFLOAT) ||
		temptypid == type_oid(T_TTEXT)
#ifdef WITH_POSTGIS
		|| temptypid == type_oid(T_TGEOMPOINT)
		|| temptypid == type_oid(T_TGEOGPOINT)
#endif
		)
		return true;
	return false;
}

/* 
 * Obtain the Oid of the base type from the Oid of the temporal type  
 */
Oid
base_oid_from_temporal(Oid temptypid)
{
	assert(temporal_type_oid(temptypid));
	int result = 0;
	if (temptypid == type_oid(T_TBOOL)) 
		result = BOOLOID;
	else if (temptypid == type_oid(T_TINT)) 
		result = INT4OID;
	else if (temptypid == type_oid(T_TFLOAT)) 
		result = FLOAT8OID;
	else if (temptypid == type_oid(T_TTEXT)) 
		result = TEXTOID;
#ifdef WITH_POSTGIS
	else if (temptypid == type_oid(T_TGEOMPOINT)) 
		result = type_oid(T_GEOMETRY);
	else if (temptypid == type_oid(T_TGEOGPOINT)) 
		result = type_oid(T_GEOGRAPHY);
#endif
	return result;
}

/*****************************************************************************
 * Trajectory functions
 *****************************************************************************/

bool
type_has_precomputed_trajectory(Oid valuetypid) 
{
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOMETRY) || 
		valuetypid == type_oid(T_GEOGRAPHY))
		return true;
#endif
	return false;
} 
 
/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/* Used for the dispatch functions */
void 
ensure_valid_duration(int16 duration)
{
	if (duration != TEMPORALINST && duration != TEMPORALI && 
		duration != TEMPORALSEQ && duration != TEMPORALS)
		elog(ERROR, "unknown duration for temporal type: %d", duration);
	return;
}

/* Used for the analyze and selectivity functions */
void 
ensure_valid_duration_all(int16 duration)
{
	if (duration != TEMPORAL && 
		duration != TEMPORALINST && duration != TEMPORALI && 
		duration != TEMPORALSEQ && duration != TEMPORALS)
		elog(ERROR, "unknown duration for temporal type: %d", duration);
	return;
}

void 
ensure_numrange_type(Oid typid)
{
	if (typid != type_oid(T_INTRANGE) && typid != type_oid(T_FLOATRANGE))
		elog(ERROR, "unknown numeric range type: %d", typid);
	return;
}

void
ensure_temporal_base_type(Oid valuetypid)
{
	if (valuetypid != BOOLOID && valuetypid != INT4OID && 
		valuetypid != FLOAT8OID && valuetypid != TEXTOID
#ifdef WITH_POSTGIS
		&& valuetypid != type_oid(T_GEOMETRY)
		&& valuetypid != type_oid(T_GEOGRAPHY)
#endif
		)
		elog(ERROR, "unknown base type: %d", valuetypid);
	return;
}

void
ensure_temporal_base_type_all(Oid valuetypid)
{
	if (valuetypid != BOOLOID && valuetypid != INT4OID && 
		valuetypid != FLOAT8OID && valuetypid != TEXTOID &&
		valuetypid != TIMESTAMPTZOID && valuetypid !=  type_oid(T_DOUBLE2)
#ifdef WITH_POSTGIS
		&& valuetypid != type_oid(T_GEOMETRY)
		&& valuetypid != type_oid(T_GEOGRAPHY)
		&& valuetypid != type_oid(T_DOUBLE3)
		&& valuetypid != type_oid(T_DOUBLE4)
#endif
		)
		elog(ERROR, "unknown base type: %d", valuetypid);
	return;
}

void
ensure_linear_interpolation(Oid valuetypid)
{
	if (valuetypid != FLOAT8OID
#ifdef WITH_POSTGIS
		&& valuetypid != type_oid(T_GEOMETRY)
		&& valuetypid != type_oid(T_GEOGRAPHY)
#endif
		)
		elog(ERROR, "unknown base type with linear interpolation: %d", valuetypid);
	return;
}

void
ensure_linear_interpolation_all(Oid valuetypid)
{
	if (valuetypid != FLOAT8OID &&
		valuetypid !=  type_oid(T_DOUBLE2)
#ifdef WITH_POSTGIS
		&& valuetypid != type_oid(T_GEOMETRY)
		&& valuetypid != type_oid(T_GEOGRAPHY)
		&& valuetypid != type_oid(T_DOUBLE3)
		&& valuetypid != type_oid(T_DOUBLE4)
#endif
		)
		elog(ERROR, "unknown base type with linear interpolation: %d", valuetypid);
	return;
}

void 
ensure_numeric_base_type(Oid valuetypid)
{
	if (valuetypid != INT4OID && valuetypid != FLOAT8OID)
		elog(ERROR, "unknown numeric base type: %d", valuetypid);
	return;
}

#ifdef WITH_POSTGIS
void 
ensure_point_base_type(Oid valuetypid)
{
	if (valuetypid != type_oid(T_GEOMETRY) && valuetypid != type_oid(T_GEOGRAPHY))
		elog(ERROR, "unknown point base type: %d", valuetypid);
	return;
}
#endif

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(mobdb_lib_version);

PGDLLEXPORT Datum
mobdb_lib_version(PG_FUNCTION_ARGS)
{
	char *ver = MOBDB_LIB_VERSION_STR;
	text *result = cstring_to_text(ver);
	PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(mobdb_full_version);

PGDLLEXPORT Datum
mobdb_full_version(PG_FUNCTION_ARGS)
{
	char ver[64];
	text *result;

	snprintf(ver, 64, "%s, %s, %s", MOBDB_LIB_VERSION_STR, 
		MOBDB_PGSQL_VERSION_STR, MOBDB_POSTGIS_VERSION_STR);
	ver[63] = '\0';

	result = cstring_to_text(ver);
	PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/* 
 * Input function. 
 * Examples of input:
 * - TemporalInst
 * 		false @ 2012-01-01 08:00:00 
 * 		1.5 @ 2012-01-01 08:00:00 
 */
 
PG_FUNCTION_INFO_V1(temporal_in);

PGDLLEXPORT Datum
temporal_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	Oid temptypid = PG_GETARG_OID(1);
	int32 temp_typmod = -1;
	Oid valuetypid;
	temporal_typinfo(temptypid, &valuetypid);
	Temporal *result = temporal_parse(&input, valuetypid);
	if (PG_NARGS() > 2 && !PG_ARGISNULL(2)) 
		temp_typmod = PG_GETARG_INT32(2);
	if (temp_typmod >= 0)
		result = temporal_valid_typmod(result, temp_typmod);
	PG_RETURN_POINTER(result);
}

/* Output function */

char *
temporal_to_string(Temporal *temp, char *(*value_out)(Oid, Datum))
{
	char *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_to_string((TemporalInst *)temp, value_out);
	else if (temp->duration == TEMPORALI) 
		result = temporali_to_string((TemporalI *)temp, value_out);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_to_string((TemporalSeq *)temp, false, value_out);
	else if (temp->duration == TEMPORALS) 
		result = temporals_to_string((TemporalS *)temp, value_out);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_out);

PGDLLEXPORT Datum
temporal_out(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	char *result = temporal_to_string(temp, &call_output);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_CSTRING(result);
}

/* Send function */

void temporal_write(Temporal *temp, StringInfo buf) 
{
	pq_sendint(buf, temp->duration, 2);
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		temporalinst_write((TemporalInst *) temp, buf);
	else if (temp->duration == TEMPORALI)
		temporali_write((TemporalI *) temp, buf);
	else if (temp->duration == TEMPORALSEQ)
		temporalseq_write((TemporalSeq *) temp, buf);
	else if (temp->duration == TEMPORALS)
		temporals_write((TemporalS *) temp, buf);
	return;
}

PG_FUNCTION_INFO_V1(temporal_send);

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

/* Receive function */

Temporal *temporal_read(StringInfo buf, Oid valuetypid) 
{
	int type = (int) pq_getmsgint(buf, 2);
	Temporal *result = NULL;
	ensure_valid_duration(type);
	if (type == TEMPORALINST)
		result = (Temporal *) temporalinst_read(buf, valuetypid);
	else if (type == TEMPORALI)
		result = (Temporal *) temporali_read(buf, valuetypid);
	else if (type == TEMPORALSEQ)
		result = (Temporal *) temporalseq_read(buf, valuetypid);
	else if (type == TEMPORALS)
		result = (Temporal *) temporals_read(buf, valuetypid);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_recv);

PGDLLEXPORT Datum
temporal_recv(PG_FUNCTION_ARGS)
{
	StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
	Oid temptypid = PG_GETARG_OID(1);
	Oid valuetypid;
	temporal_typinfo(temptypid, &valuetypid);
	Temporal *result = temporal_read(buf, valuetypid) ;
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_typmod_in);

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
	uint8_t type = 0;
	if (!temporal_duration_from_string(s, &type))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Invalid temporal type modifier: %s", s)));

	pfree(elem_values);
	PG_RETURN_INT32((int32)type);
}

PG_FUNCTION_INFO_V1(temporal_typmod_out);

PGDLLEXPORT Datum 
temporal_typmod_out(PG_FUNCTION_ARGS)
{
	char *s = (char *) palloc(64);
	char *str = s;
	int32 typmod = PG_GETARG_INT32(0);
	int32 duration_type = TYPMOD_GET_DURATION(typmod);
	/* No type? Then no typmod at all. Return empty string.  */
	if (typmod < 0 || !duration_type)
	{
		*str = '\0';
		PG_RETURN_CSTRING(str);
	}
	str += sprintf(str, "(%s)", temporal_duration_name(duration_type));
	PG_RETURN_CSTRING(s);
}

PG_FUNCTION_INFO_V1(temporal_enforce_typmod);
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

 /* Make temporal instant value from two arguments */

PG_FUNCTION_INFO_V1(temporalinst_constructor);

PGDLLEXPORT Datum
temporalinst_constructor(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Temporal *result = (Temporal *)temporalinst_make(value, t, valuetypid);
	PG_RETURN_POINTER(result);
}

/* Make a TemporalI from an array of TemporalInst */

PG_FUNCTION_INFO_V1(temporali_constructor);

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
	
	Temporal *result = (Temporal *)temporali_from_temporalinstarr(instants, count);
	pfree(instants);
	PG_FREE_IF_COPY(array, 0);
	PG_RETURN_POINTER(result);
}

/* Make a TemporalSeq from an array of TemporalInst */

PG_FUNCTION_INFO_V1(temporalseq_constructor);

PGDLLEXPORT Datum
temporalseq_constructor(PG_FUNCTION_ARGS)
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

	Temporal *result = (Temporal *)temporalseq_from_temporalinstarr(instants, 
		count, lower_inc, upper_inc, linear, true);
	pfree(instants);
	PG_FREE_IF_COPY(array, 0);
	PG_RETURN_POINTER(result);
}

/* Make a TemporalS from an array of TemporalSeq */

PG_FUNCTION_INFO_V1(temporals_constructor);

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

	Temporal *result = (Temporal *)temporals_from_temporalseqarr(sequences, count,
		linear, true);
	
	pfree(sequences);
	PG_FREE_IF_COPY(array, 0);
	
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Append function
 ****************************************************************************/

 /* Append an instant to the end of a temporal */

PG_FUNCTION_INFO_V1(temporal_append_instant);

PGDLLEXPORT Datum
temporal_append_instant(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *inst = PG_GETARG_TEMPORAL(1);
	if (inst->duration != TEMPORALINST) 
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("The second argument must be of instant duration")));
	assert(temp->valuetypid == inst->valuetypid);

	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_append_instant((TemporalS *)temp,
			(TemporalInst *)inst);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(inst, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/* Cast a temporal integer as a temporal float */

Temporal *
tint_to_tfloat_internal(Temporal *temp)
{
	Temporal *result = NULL;
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tintinst_to_tfloatinst((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tinti_to_tfloati((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)tintseq_to_tfloatseq((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)tints_to_tfloats((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(tint_to_tfloat);

PGDLLEXPORT Datum
tint_to_tfloat(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tint_to_tfloat_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}


/* Cast a temporal integer as a temporal float */

Temporal *
tfloat_to_tint_internal(Temporal *temp)
{
	Temporal *result = NULL;
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tfloatinst_to_tintinst((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfloati_to_tinti((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)tfloatseq_to_tintseq((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)tfloats_to_tints((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(tfloat_to_tint);

PGDLLEXPORT Datum
tfloat_to_tint(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tfloat_to_tint_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/* Bounding period on which the temporal value is defined */

void
temporal_period(Period *p, Temporal *temp)
{
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		temporalinst_period(p, (TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		temporali_period(p, (TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		temporalseq_period(p, (TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		temporals_period(p, (TemporalS *)temp);
	return;
}

PG_FUNCTION_INFO_V1(temporal_to_period);

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

/* Transform Temporal to TemporalInst */

PG_FUNCTION_INFO_V1(temporal_to_temporalinst);

PGDLLEXPORT Datum
temporal_to_temporalinst(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporal_copy(temp);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)temporali_to_temporalinst((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_to_temporalinst((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)temporals_to_temporalinst((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/* Transform Temporal to TemporalI */

PG_FUNCTION_INFO_V1(temporal_to_temporali);

PGDLLEXPORT Datum
temporal_to_temporali(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_to_temporali((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = temporal_copy(temp);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_to_temporali((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)temporals_to_temporali((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/* Transform Temporal to TemporalSeq */

PG_FUNCTION_INFO_V1(temporal_to_temporalseq);

PGDLLEXPORT Datum
temporal_to_temporalseq(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp->flags);
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_to_temporalseq((TemporalInst *)temp, linear);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)temporali_to_temporalseq((TemporalI *)temp, linear);
	else if (temp->duration == TEMPORALSEQ)
		result = temporal_copy(temp);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)temporals_to_temporalseq((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result); 
}

/* Transform Temporal as TemporalS */

PG_FUNCTION_INFO_V1(temporal_to_temporals);

PGDLLEXPORT Datum
temporal_to_temporals(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp->flags);
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_to_temporals((TemporalInst *)temp, linear);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)temporali_to_temporals((TemporalI *)temp, linear);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_to_temporals((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = temporal_copy(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result); 
}

/* Transform a temporal value with continuous base type from stepwise to linear interpolation */

PG_FUNCTION_INFO_V1(tstepw_to_linear);

PGDLLEXPORT Datum
tstepw_to_linear(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->duration != TEMPORALSEQ && temp->duration != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));
	ensure_linear_interpolation(temp->valuetypid);

	if (MOBDB_FLAGS_GET_LINEAR(temp->flags))
			PG_RETURN_POINTER(temporal_copy(temp)); 

	Temporal *result = NULL;
	if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)tstepwseq_to_linear((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)tstepws_to_linear((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result); 
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* Returns a string representation of the temporal duration */

PG_FUNCTION_INFO_V1(temporal_duration);

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
	else if (temp->duration == TEMPORALS) 
		strcpy(str, "SequenceSet");
	text *result = cstring_to_text(str);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TEXT_P(result);
}

/* Returns a string representation of the temporal interpolation */

PG_FUNCTION_INFO_V1(temporal_interpolation);

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

/* Size in bytes of the temporal value */

PG_FUNCTION_INFO_V1(temporal_mem_size);

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

PGDLLEXPORT Datum
temporal_mem_size(PG_FUNCTION_ARGS)
{
	Datum result = toast_datum_size(PG_GETARG_DATUM(0));
	PG_RETURN_DATUM(result);
}

/* Values of a temporal type */ 

Datum
temporal_values(Temporal *temp)
{
	ArrayType *result = NULL;	/* make the compiler quiet */
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_values((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = temporali_values((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_values((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = temporals_values((TemporalS *)temp);
	return PointerGetDatum(result);
}

PG_FUNCTION_INFO_V1(temporal_get_values);

PGDLLEXPORT Datum
temporal_get_values(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = temporal_values(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/* Ranges of a temporal float with either stepwise/linear interpolation */

Datum
tfloat_ranges(Temporal *temp)
{
	ArrayType *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = tfloatinst_ranges((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = tfloati_ranges((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = tfloatseq_ranges((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = tfloats_ranges((TemporalS *)temp);
	return PointerGetDatum(result);
}


PG_FUNCTION_INFO_V1(tfloat_get_ranges);

PGDLLEXPORT Datum
tfloat_get_ranges(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = tfloat_ranges(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/* Value of a temporal instant */

PG_FUNCTION_INFO_V1(temporalinst_get_value);

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

/* Returns the time of the temporal type */

PeriodSet *
temporal_get_time_internal(Temporal *temp)
{
	PeriodSet *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_get_time((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = temporali_get_time((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_get_time((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = temporals_get_time((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_get_time);

PGDLLEXPORT Datum
temporal_get_time(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *result = temporal_get_time_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/* TimestampTz of a temporal instant */

PG_FUNCTION_INFO_V1(temporalinst_timestamp);

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

/* Get the precomputed bounding box of a Temporal (if any) 
   Notice that TemporalInst do not have precomputed bonding box */

TBOX * 
temporal_bbox_ptr(const Temporal *temp)
{
	TBOX *result = NULL;
	if (temp->duration == TEMPORALINST) 
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Temporal instants do not have bounding box")));
	else if (temp->duration == TEMPORALI) 
		result = temporali_bbox_ptr((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_bbox_ptr((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = temporals_bbox_ptr((TemporalS *)temp);
	return result;
}

void 
temporal_bbox(void *box, const Temporal *temp)
{
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		temporalinst_bbox(box, (TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		temporali_bbox(box, (TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		temporalseq_bbox(box, (TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		temporals_bbox(box, (TemporalS *)temp);
	return;
}

PG_FUNCTION_INFO_V1(tnumber_to_tbox);

PGDLLEXPORT Datum
tnumber_to_tbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *result = palloc0(sizeof(TBOX));
	temporal_bbox(result, temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/* Value range of a temporal integer */

RangeType *
tnumber_value_range_internal(Temporal *temp)
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
		TBOX *box = temporal_bbox_ptr(temp);
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

PGDLLEXPORT Datum
tnumber_value_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *result = tnumber_value_range_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_RANGE_P(result);
}

/* Start value */

PG_FUNCTION_INFO_V1(temporal_start_value);

PGDLLEXPORT Datum
temporal_start_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = temporalinst_value_copy(temporali_inst_n((TemporalI *)temp, 0));
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalinst_value_copy(temporalseq_inst_n((TemporalSeq *)temp, 0));
	else if (temp->duration == TEMPORALS) 
	{
		TemporalSeq *seq = temporals_seq_n((TemporalS *)temp, 0);
		result = temporalinst_value_copy(temporalseq_inst_n(seq, 0));
	}
	PG_FREE_IF_COPY(temp, 0);	
	PG_RETURN_DATUM(result);
}

/* End value */

PG_FUNCTION_INFO_V1(temporal_end_value);

PGDLLEXPORT Datum
temporal_end_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = temporalinst_value_copy(temporali_inst_n((TemporalI *)temp, 
			((TemporalI *)temp)->count - 1));
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalinst_value_copy(temporalseq_inst_n((TemporalSeq *)temp, 
			((TemporalSeq *)temp)->count - 1));
	else if (temp->duration == TEMPORALS) 
	{
		TemporalSeq *seq = temporals_seq_n((TemporalS *)temp, ((TemporalS *)temp)->count - 1);
		result = temporalinst_value_copy(temporalseq_inst_n(seq, seq->count - 1));
	}
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/* Minimum value */

Datum
temporal_min_value_internal(Temporal *temp)
{
	Datum result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = datum_copy(temporali_min_value((TemporalI *)temp),
			temp->valuetypid);
	else if (temp->duration == TEMPORALSEQ) 
		result = datum_copy(temporalseq_min_value((TemporalSeq *)temp),
			temp->valuetypid);
	else if (temp->duration == TEMPORALS) 
		result = datum_copy(temporals_min_value((TemporalS *)temp),
			temp->valuetypid);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_min_value);

PGDLLEXPORT Datum
temporal_min_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = temporal_min_value_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/* Maximum value */

PG_FUNCTION_INFO_V1(temporal_max_value);

Datum
temporal_max_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = datum_copy(temporali_max_value((TemporalI *)temp),
			temp->valuetypid);
	else if (temp->duration == TEMPORALSEQ) 
		result = datum_copy(temporalseq_max_value((TemporalSeq *)temp),
			temp->valuetypid);
	else if (temp->duration == TEMPORALS) 
		result = datum_copy(temporals_max_value((TemporalS *)temp),
			temp->valuetypid);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/* Timespan */

PG_FUNCTION_INFO_V1(temporal_timespan);

PGDLLEXPORT Datum
temporal_timespan(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST || temp->duration == TEMPORALI) 
	{
		Interval *interval = (Interval *) palloc(sizeof(Interval));
		interval->month = interval->day = interval->time = 0;
		result = PointerGetDatum(interval);
	}
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_timespan((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = temporals_timespan((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/* Number of sequences */

PG_FUNCTION_INFO_V1(temporal_num_sequences);

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

/* Initial sequence */

PG_FUNCTION_INFO_V1(temporal_start_sequence);

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

/* End sequence */

PG_FUNCTION_INFO_V1(temporal_end_sequence);

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

/* N-th sequence */

PG_FUNCTION_INFO_V1(temporal_sequence_n);

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

/* Sequences */

PG_FUNCTION_INFO_V1(temporal_sequences);

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

/* Number of distinct instants */

PG_FUNCTION_INFO_V1(temporal_num_instants);

PGDLLEXPORT Datum
temporal_num_instants(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = 1;
	else if (temp->duration == TEMPORALI) 
		result = ((TemporalI *)temp)->count;
	else if (temp->duration == TEMPORALSEQ) 
		result = ((TemporalSeq *)temp)->count;
	else if (temp->duration == TEMPORALS) 
		result = temporals_num_instants((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(temporal_start_instant);

PGDLLEXPORT Datum
temporal_start_instant(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporalinst_copy(temporali_inst_n((TemporalI *)temp, 0));
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalinst_copy(temporalseq_inst_n((TemporalSeq *)temp, 0));
	else if (temp->duration == TEMPORALS) 
	{
		TemporalSeq *seq = temporals_seq_n((TemporalS *)temp, 0);
		result = (Temporal *)temporalinst_copy(temporalseq_inst_n(seq, 0));
	}
	PG_FREE_IF_COPY(temp, 0);	
	PG_RETURN_POINTER(result);
}

/* End value */

PG_FUNCTION_INFO_V1(temporal_end_instant);

PGDLLEXPORT Datum
temporal_end_instant(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporalinst_copy(temporali_inst_n((TemporalI *)temp, 
			((TemporalI *)temp)->count - 1));
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalinst_copy(temporalseq_inst_n((TemporalSeq *)temp, 
			((TemporalSeq *)temp)->count - 1));
	else if (temp->duration == TEMPORALS) 
	{
		TemporalSeq *seq = temporals_seq_n((TemporalS *)temp, 
			((TemporalS *)temp)->count - 1);
		result = (Temporal *)temporalinst_copy(temporalseq_inst_n(seq, seq->count - 1));
	}	
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/* N-th instant */

PG_FUNCTION_INFO_V1(temporal_instant_n);

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
	else if (temp->duration == TEMPORALS)
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

/* Distinct instants */

PG_FUNCTION_INFO_V1(temporal_instants);

PGDLLEXPORT Datum
temporal_instants(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_instants_array((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = temporali_instants_array((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_instants_array((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = temporals_instants_array((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_ARRAYTYPE_P(result);
}

TimestampTz
temporal_start_timestamp_internal(Temporal *temp)
{
	TimestampTz result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = ((TemporalInst *)temp)->t;
	else if (temp->duration == TEMPORALI) 
		result = temporali_inst_n((TemporalI *)temp, 0)->t;
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_start_timestamp((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = temporals_start_timestamp((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_start_timestamp);

PGDLLEXPORT Datum
temporal_start_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz result = temporal_start_timestamp_internal(temp);
	PG_FREE_IF_COPY(temp, 0);	
	PG_RETURN_TIMESTAMPTZ(result);
}

/* End value */

PG_FUNCTION_INFO_V1(temporal_end_timestamp);

PGDLLEXPORT Datum
temporal_end_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = ((TemporalInst *)temp)->t;
	else if (temp->duration == TEMPORALI) 
		result = temporali_inst_n((TemporalI *)temp, ((TemporalI *)temp)->count - 1)->t;
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_end_timestamp((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = temporals_end_timestamp((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TIMESTAMPTZ(result);
}

/* Number of distinct timestamps */

PG_FUNCTION_INFO_V1(temporal_num_timestamps);

PGDLLEXPORT Datum
temporal_num_timestamps(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = 1;
	else if (temp->duration == TEMPORALI) 
		result = ((TemporalI *)temp)->count;
	else if (temp->duration == TEMPORALSEQ) 
		result = ((TemporalSeq *)temp)->count;
	else if (temp->duration == TEMPORALS) 
		result = temporals_num_timestamps((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/* N-th distinct timestamp */

PG_FUNCTION_INFO_V1(temporal_timestamp_n);

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
	else if (temp->duration == TEMPORALS) 
		found = temporals_timestamp_n((TemporalS *)temp, n, &result);
	PG_FREE_IF_COPY(temp, 0);
	if (!found) 
		PG_RETURN_NULL();
	PG_RETURN_TIMESTAMPTZ(result);
}

/* Distinct timestamps */

PG_FUNCTION_INFO_V1(temporal_timestamps);

PGDLLEXPORT Datum
temporal_timestamps(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_timestamps((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = temporali_timestamps((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_timestamps((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = temporals_timestamps((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_ARRAYTYPE_P(result);
}

/* Shift the time span of a temporal value by an interval */

PG_FUNCTION_INFO_V1(temporal_shift);

PGDLLEXPORT Datum
temporal_shift(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Interval *interval = PG_GETARG_INTERVAL_P(1);
	Temporal *result = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_shift((TemporalInst *)temp, interval);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_shift((TemporalI *)temp, interval);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_shift((TemporalSeq *)temp, interval);
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_shift((TemporalS *)temp, interval);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Ever/Always Comparison Functions 
 *****************************************************************************/

/* Is the temporal value ever equal to the value? */

bool
temporal_ever_eq_internal(Temporal *temp, Datum value)
{
	bool result = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_ever_eq((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI) 
		result = temporali_ever_eq((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_ever_eq((TemporalSeq *)temp, value);
	else if (temp->duration == TEMPORALS) 
		result = temporals_ever_eq((TemporalS *)temp, value);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_ever_eq);

PGDLLEXPORT Datum
temporal_ever_eq(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	bool result = temporal_ever_eq_internal(temp, value);
	PG_FREE_IF_COPY(temp, 0);
	FREE_DATUM(value, temp->valuetypid);
	PG_RETURN_BOOL(result);
}

/* Is the temporal value always equal to the value? */

PG_FUNCTION_INFO_V1(temporal_always_eq);

PGDLLEXPORT Datum
temporal_always_eq(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	bool result = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_always_eq((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI) 
		result = temporali_always_eq((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_always_eq((TemporalSeq *)temp, value);
	else if (temp->duration == TEMPORALS) 
		result = temporals_always_eq((TemporalS *)temp, value);
	PG_FREE_IF_COPY(temp, 0);
	FREE_DATUM(value, temp->valuetypid);
	PG_RETURN_BOOL(result);
}

/* Is the temporal value ever not equal to the value? */

PG_FUNCTION_INFO_V1(temporal_ever_ne);

PGDLLEXPORT Datum
temporal_ever_ne(PG_FUNCTION_ARGS)
{
	return ! temporal_always_eq(fcinfo);
}

/* Is the temporal value always not equal to the value? */

PG_FUNCTION_INFO_V1(temporal_always_ne);

PGDLLEXPORT Datum
temporal_always_ne(PG_FUNCTION_ARGS)
{
	return ! temporal_ever_eq(fcinfo);
}

/*****************************************************************************/

/* Is the temporal value ever less than the value? */

PG_FUNCTION_INFO_V1(temporal_ever_lt);

PGDLLEXPORT Datum
temporal_ever_lt(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	bool result = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_ever_lt((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI) 
		result = temporali_ever_lt((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_ever_lt((TemporalSeq *)temp, value);
	else if (temp->duration == TEMPORALS) 
		result = temporals_ever_lt((TemporalS *)temp, value);
	PG_FREE_IF_COPY(temp, 0);
	FREE_DATUM(value, temp->valuetypid);
	PG_RETURN_BOOL(result);
}

/* Is the temporal value always less than the value? */

PG_FUNCTION_INFO_V1(temporal_always_lt);

PGDLLEXPORT Datum
temporal_always_lt(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	bool result = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_always_lt((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI) 
		result = temporali_always_lt((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_always_lt((TemporalSeq *)temp, value);
	else if (temp->duration == TEMPORALS) 
		result = temporals_always_lt((TemporalS *)temp, value);
	PG_FREE_IF_COPY(temp, 0);
	FREE_DATUM(value, temp->valuetypid);
	PG_RETURN_BOOL(result);
}

/* Is the temporal value ever less than or equal to the value? */

PG_FUNCTION_INFO_V1(temporal_ever_le);

PGDLLEXPORT Datum
temporal_ever_le(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	bool result = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_ever_le((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI) 
		result = temporali_ever_le((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_ever_le((TemporalSeq *)temp, value);
	else if (temp->duration == TEMPORALS) 
		result = temporals_ever_le((TemporalS *)temp, value);
	PG_FREE_IF_COPY(temp, 0);
	FREE_DATUM(value, temp->valuetypid);
	PG_RETURN_BOOL(result);
}

/* Is the temporal value always less than or equal to the value? */

PG_FUNCTION_INFO_V1(temporal_always_le);

PGDLLEXPORT Datum
temporal_always_le(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	bool result = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_always_le((TemporalInst *)temp, value);
	else if (temp->duration == TEMPORALI) 
		result = temporali_always_le((TemporalI *)temp, value);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_always_le((TemporalSeq *)temp, value);
	else if (temp->duration == TEMPORALS) 
		result = temporals_always_le((TemporalS *)temp, value);
	PG_FREE_IF_COPY(temp, 0);
	FREE_DATUM(value, temp->valuetypid);
	PG_RETURN_BOOL(result);
}

/* Is the temporal value ever greater than the value? */

PG_FUNCTION_INFO_V1(temporal_ever_gt);

PGDLLEXPORT Datum
temporal_ever_gt(PG_FUNCTION_ARGS)
{
	return ! temporal_always_le(fcinfo);
}

/* Is the temporal value always greater than the value? */

PG_FUNCTION_INFO_V1(temporal_always_gt);

PGDLLEXPORT Datum
temporal_always_gt(PG_FUNCTION_ARGS)
{
	return ! temporal_ever_le(fcinfo);
}

/* Is the temporal value ever greater than the value? */

PG_FUNCTION_INFO_V1(temporal_ever_ge);

PGDLLEXPORT Datum
temporal_ever_ge(PG_FUNCTION_ARGS)
{
	return ! temporal_always_lt(fcinfo);
}

/* Is the temporal value always greater than the value? */

PG_FUNCTION_INFO_V1(temporal_always_ge);

PGDLLEXPORT Datum
temporal_always_ge(PG_FUNCTION_ARGS)
{
	return ! temporal_ever_lt(fcinfo);
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/* Restriction to a value */

PG_FUNCTION_INFO_V1(temporal_at_value);

PGDLLEXPORT Datum
temporal_at_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_at_value(
			(TemporalS *)temp, value);
	PG_FREE_IF_COPY(temp, 0);
	FREE_DATUM(value, valuetypid);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Restriction to the complement of a value */

PG_FUNCTION_INFO_V1(temporal_minus_value);

PGDLLEXPORT Datum
temporal_minus_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_minus_value(
			(TemporalS *)temp, value);
	PG_FREE_IF_COPY(temp, 0);
	FREE_DATUM(value, valuetypid);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Restriction to an array of values */

PG_FUNCTION_INFO_V1(temporal_at_values);

PGDLLEXPORT Datum
temporal_at_values(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
	Oid valuetypid = temp->valuetypid;
	int count;
	Datum *values = datumarr_extract(array, &count);
	if (count == 0)
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(array, 1);
		PG_RETURN_NULL();	
	}

	datum_sort(values, count, valuetypid);
	int count1 = datum_remove_duplicates(values, count, valuetypid);
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_at_values(
			(TemporalS *)temp, values, count1);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(array, 1);
	if (result == NULL) 
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Restriction to the complement of an array of values */

PG_FUNCTION_INFO_V1(temporal_minus_values);

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

	Oid valuetypid = temp->valuetypid;
	datum_sort(values, count, valuetypid);
	int count1 = datum_remove_duplicates(values, count, valuetypid);
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_minus_values(
			(TemporalS *)temp, values, count1);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(array, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Restriction to a range */

PG_FUNCTION_INFO_V1(tnumber_at_range);

PGDLLEXPORT Datum
tnumber_at_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)tnumbers_at_range(
			(TemporalS *)temp, range);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Restriction to minus range */

PG_FUNCTION_INFO_V1(tnumber_minus_range);

PGDLLEXPORT Datum
tnumber_minus_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)tnumbers_minus_range(
			(TemporalS *)temp, range);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Restriction to an array of ranges */

PG_FUNCTION_INFO_V1(tnumber_at_ranges);

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
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
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

/* Restriction to the complement of an array of ranges. */

PG_FUNCTION_INFO_V1(tnumber_minus_ranges);

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
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
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

/* Restriction to the minimum value */

Temporal *
temporal_at_min_internal(Temporal *temp)
{
	Temporal *result = NULL;
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_at_min((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_at_min((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_at_min((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_at_min);

PGDLLEXPORT Datum
temporal_at_min(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = temporal_at_min_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	/* Never returns null event if min is at an exclusive bound */
	PG_RETURN_POINTER(result);
}

/* Restriction to the complement of the minimum value */

PG_FUNCTION_INFO_V1(temporal_minus_min);

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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_minus_min((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}
 
/* Restriction to the maximum value */
 
PG_FUNCTION_INFO_V1(temporal_at_max);

PGDLLEXPORT Datum
temporal_at_max(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_at_max((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_at_max((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_at_max((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	/* Never returns null event if max is at an exclusive bound */
	PG_RETURN_POINTER(result);
}

/* Restriction to the complement of the maximum value */

PG_FUNCTION_INFO_V1(temporal_minus_max);

PGDLLEXPORT Datum
temporal_minus_max(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = NULL;
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_minus_max((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_minus_max((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_minus_max((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Restriction to a timestamp */

TemporalInst *
temporal_at_timestamp_internal(Temporal *temp, TimestampTz t)
{
	TemporalInst *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_at_timestamp((TemporalInst *)temp, t);
	else if (temp->duration == TEMPORALI) 
		result = temporali_at_timestamp((TemporalI *)temp, t);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_at_timestamp((TemporalSeq *)temp, t);
	else if (temp->duration == TEMPORALS) 
		result = temporals_at_timestamp((TemporalS *)temp, t);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_at_timestamp);

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

/* Restriction to the complement of a timestamp */

PG_FUNCTION_INFO_V1(temporal_minus_timestamp);

PGDLLEXPORT Datum
temporal_minus_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)temporalinst_minus_timestamp((TemporalInst *)temp, t);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)temporali_minus_timestamp((TemporalI *)temp, t);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_minus_timestamp((TemporalSeq *)temp, t);
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_minus_timestamp((TemporalS *)temp, t);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Value at a timestamp */

PG_FUNCTION_INFO_V1(temporal_value_at_timestamp);

PGDLLEXPORT Datum
temporal_value_at_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	bool found = false;
	Datum result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		found = temporalinst_value_at_timestamp((TemporalInst *)temp, t, &result);
	else if (temp->duration == TEMPORALI) 
		found = temporali_value_at_timestamp((TemporalI *)temp, t, &result);
	else if (temp->duration == TEMPORALSEQ) 
		found = temporalseq_value_at_timestamp((TemporalSeq *)temp, t, &result);
	else if (temp->duration == TEMPORALS) 
		found = temporals_value_at_timestamp((TemporalS *)temp, t, &result);
	PG_FREE_IF_COPY(temp, 0);
	if (!found)
		PG_RETURN_NULL();
	PG_RETURN_DATUM(result);
}

/* Restriction to a timestampset */

PG_FUNCTION_INFO_V1(temporal_at_timestampset);

PGDLLEXPORT Datum
temporal_at_timestampset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Temporal *result = 0;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_at_timestampset(
			(TemporalS *)temp, ts);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Restriction to the complement of a timestampset */

PG_FUNCTION_INFO_V1(temporal_minus_timestampset);

PGDLLEXPORT Datum
temporal_minus_timestampset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Temporal *result = 0;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_minus_timestampset(
			(TemporalS *)temp, ts);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
	if (result == NULL)
		PG_RETURN_NULL();	
	PG_RETURN_POINTER(result);
}

/* Restriction to a period */

PG_FUNCTION_INFO_V1(temporal_at_period);

PGDLLEXPORT Datum
temporal_at_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_at_period(
			(TemporalS *)temp, p);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();	
	PG_RETURN_POINTER(result);
}

/* Restriction to the complement of a period */

PG_FUNCTION_INFO_V1(temporal_minus_period);

PGDLLEXPORT Datum
temporal_minus_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_minus_period(
			(TemporalS *)temp, p);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Restriction to a periodset */

Temporal *
temporal_at_periodset_internal(Temporal *temp, PeriodSet *ps)
{
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_at_periodset(
			(TemporalS *)temp, ps);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_at_periodset);

PGDLLEXPORT Datum
temporal_at_periodset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_at_periodset(
			(TemporalS *)temp, ps);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Restriction to the complement of a periodset */

PG_FUNCTION_INFO_V1(temporal_minus_periodset);

PGDLLEXPORT Datum
temporal_minus_periodset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Temporal *result = NULL;
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
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)temporals_minus_periodset(
			(TemporalS *)temp, ps);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Does the temporal value intersects the timestamp? */

PG_FUNCTION_INFO_V1(temporal_intersects_timestamp);

PGDLLEXPORT Datum
temporal_intersects_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	bool result = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_intersects_timestamp((TemporalInst *)temp, t);
	else if (temp->duration == TEMPORALI) 
		result = temporali_intersects_timestamp((TemporalI *)temp, t);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_intersects_timestamp((TemporalSeq *)temp, t);
	else if (temp->duration == TEMPORALS) 
		result = temporals_intersects_timestamp((TemporalS *)temp, t);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/* Does the temporal value intersects the timestamp set? */

PG_FUNCTION_INFO_V1(temporal_intersects_timestampset);

PGDLLEXPORT Datum
temporal_intersects_timestampset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	bool result = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_intersects_timestampset((TemporalInst *)temp, ts);
	else if (temp->duration == TEMPORALI) 
		result = temporali_intersects_timestampset((TemporalI *)temp, ts);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_intersects_timestampset((TemporalSeq *)temp, ts);
	else if (temp->duration == TEMPORALS) 
		result = temporals_intersects_timestampset((TemporalS *)temp, ts);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
	PG_RETURN_BOOL(result);
}

/* Does the temporal value intersects the period? */

PG_FUNCTION_INFO_V1(temporal_intersects_period);

PGDLLEXPORT Datum
temporal_intersects_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	bool result = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_intersects_period((TemporalInst *)temp, p);
	else if (temp->duration == TEMPORALI) 
		result = temporali_intersects_period((TemporalI *)temp, p);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_intersects_period((TemporalSeq *)temp, p);
	else if (temp->duration == TEMPORALS) 
		result = temporals_intersects_period((TemporalS *)temp, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/* Does the temporal value intersects the period set? */

PG_FUNCTION_INFO_V1(temporal_intersects_periodset);

PGDLLEXPORT Datum
temporal_intersects_periodset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	bool result = false;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = temporalinst_intersects_periodset((TemporalInst *)temp, ps);
	else if (temp->duration == TEMPORALI) 
		result = temporali_intersects_periodset((TemporalI *)temp, ps);
	else if (temp->duration == TEMPORALSEQ) 
		result = temporalseq_intersects_periodset((TemporalSeq *)temp, ps);
	else if (temp->duration == TEMPORALS) 
		result = temporals_intersects_periodset((TemporalS *)temp, ps);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Local aggregate functions 
 *****************************************************************************/

/* Integral of temporal numbers */

PG_FUNCTION_INFO_V1(tnumber_integral);

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
	else if (temp->duration == TEMPORALS)
		result = tnumbers_integral((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_FLOAT8(result);
}

/* Time-weighted average of temporal numbers */

PG_FUNCTION_INFO_V1(tnumber_twavg);

PGDLLEXPORT Datum
tnumber_twavg(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	double result = 0.0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = datum_double(temporalinst_value((TemporalInst *)temp), 
			temp->valuetypid);
	else if (temp->duration == TEMPORALI)
		result = tnumberi_twavg((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = tnumberseq_twavg((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = tnumbers_twavg((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/* B-tree comparator */

int
temporal_cmp_internal(const Temporal *t1, const Temporal *t2)
{
	assert(t1->valuetypid == t2->valuetypid);

	/* Compare bounding box */
	union bboxunion box1, box2;
	memset(&box1, 0, sizeof(bboxunion));
	memset(&box2, 0, sizeof(bboxunion));
	temporal_bbox(&box1, t1);
	temporal_bbox(&box2, t2);
	int result = temporal_bbox_cmp(t1->valuetypid, &box1, &box2);
	if (result)
		return result;

	/* If both are of the same duration use the specific comparison */
	if (t1->duration == t2->duration)
	{
		ensure_valid_duration(t1->duration);
		if (t1->duration == TEMPORALINST) 
			return temporalinst_cmp((TemporalInst *)t1, (TemporalInst *)t2);
		else if (t1->duration == TEMPORALI) 
			return temporali_cmp((TemporalI *)t1, (TemporalI *)t2);
		else if (t1->duration == TEMPORALSEQ) 
			return temporalseq_cmp((TemporalSeq *)t1, (TemporalSeq *)t2);
		else if (t1->duration == TEMPORALS) 
			return temporals_cmp((TemporalS *)t1, (TemporalS *)t2);
	}
	
	/* Use the hash comparison */
	uint32 hash1 = temporal_hash_internal(t1);
	uint32 hash2 = temporal_hash_internal(t2);
	if (hash1 < hash2)
		return -1;
	else if (hash2 > hash1)
		return 1;
	
	/* Compare memory size */
	size_t size1 = VARSIZE(DatumGetPointer(t1));
	size_t size2 = VARSIZE(DatumGetPointer(t2));
	if (size1 < size2)
		return -1;
	else if (size1 > size2)
		return 1;
	else
		return 0;
	
	/* Finally compare temporal type */
	if (t1->duration < t2->duration)
		return -1;
	else if (t1->duration > t2->duration)
		return 1;
	else
		return 0;
}

PG_FUNCTION_INFO_V1(temporal_cmp);

PGDLLEXPORT Datum
temporal_cmp(PG_FUNCTION_ARGS)
{
	Temporal *t1 = PG_GETARG_TEMPORAL(0);
	Temporal *t2 = PG_GETARG_TEMPORAL(1);
	int result = temporal_cmp_internal(t1, t2);
	PG_FREE_IF_COPY(t1, 0);
	PG_FREE_IF_COPY(t2, 1);
	PG_RETURN_INT32(result);
}

/* 
 * Equality operator
 * The internal B-tree comparator is not used to increase efficiency 
 */
bool
temporal_eq_internal(Temporal *t1, Temporal *t2)
{
	assert(t1->valuetypid == t2->valuetypid);
	ensure_valid_duration(t1->duration);
	ensure_valid_duration(t2->duration);

	/* If both are of the same duration use the specific equality */
	if (t1->duration == t2->duration)
	{
		if (t1->duration == TEMPORALINST) 
			return temporalinst_eq((TemporalInst *)t1, (TemporalInst *)t2);
		else if (t1->duration == TEMPORALI) 
			return temporali_eq((TemporalI *)t1, (TemporalI *)t2);
		else if (t1->duration == TEMPORALSEQ) 
			return temporalseq_eq((TemporalSeq *)t1, (TemporalSeq *)t2);
		else if (t1->duration == TEMPORALS) 
			return temporals_eq((TemporalS *)t1, (TemporalS *)t2);
	}	
	
	/* Different duration */
	if (t1->duration > t2->duration)
	{
		Temporal *temp = t1;
		t1 = t2;
		t2 = temp;
	}
	if (t1->duration == TEMPORALINST && t2->duration == TEMPORALI)
	{
		TemporalInst *inst = (TemporalInst *)t1;
		TemporalI *ti = (TemporalI *)t2;
		if (ti->count != 1) 
			return false;
		TemporalInst *inst1 = temporali_inst_n(ti, 0);
		return temporalinst_eq(inst, inst1);	
	}
	else if (t1->duration == TEMPORALINST && t2->duration == TEMPORALSEQ)
	{
		TemporalInst *inst = (TemporalInst *)t1;
		TemporalSeq *seq = (TemporalSeq *)t2; 
		if (seq->count != 1) 
			return false;
		TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
		return temporalinst_eq(inst, inst1);	
	}
	else if (t1->duration == TEMPORALINST && t2->duration == TEMPORALS)
	{
		TemporalInst *inst = (TemporalInst *)t1;
		TemporalS *ts = (TemporalS *)t2; 
		if (ts->count != 1) 
			return false;
		TemporalSeq *seq = temporals_seq_n(ts, 0);
		if (seq->count != 1) 
			return false;
		TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
		return temporalinst_eq(inst, inst1);	
	}
	else if (t1->duration == TEMPORALI && t2->duration == TEMPORALSEQ)
	{
		TemporalI *ti = (TemporalI *)t1; 
		TemporalSeq *seq = (TemporalSeq *)t2; 
		if (ti->count != 1 || seq->count != 1) 
			return false;
		TemporalInst *inst1 = temporali_inst_n(ti, 0);
		TemporalInst *inst2 = temporalseq_inst_n(seq, 0);
		return temporalinst_eq(inst1, inst2);	
	}
	else if (t1->duration == TEMPORALI && t2->duration == TEMPORALS)
	{
		TemporalI *ti = (TemporalI *)t1; 
		TemporalS *ts = (TemporalS *)t2; 
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
	else if (t1->duration == TEMPORALSEQ && t2->duration == TEMPORALS)
	{
		TemporalSeq *seq = (TemporalSeq *)t1; 
		TemporalS *ts = (TemporalS *)t2; 
		if (ts->count != 1) 
			return false;
		TemporalSeq *seq1 = temporals_seq_n(ts, 0);
		return temporalseq_eq(seq, seq1);	
	}
	return false; /* make compiler quiet */
}

PG_FUNCTION_INFO_V1(temporal_eq);

PGDLLEXPORT Datum
temporal_eq(PG_FUNCTION_ARGS)
{
	Temporal *t1 = PG_GETARG_TEMPORAL(0);
	Temporal *t2 = PG_GETARG_TEMPORAL(1);
	bool result = temporal_eq_internal(t1, t2);
	PG_FREE_IF_COPY(t1, 0);
	PG_FREE_IF_COPY(t2, 1);
	PG_RETURN_BOOL(result);
}

/* 
 * Inequality operator
 * The internal B-tree comparator is not used to increase efficiency 
 */
bool
temporal_ne_internal(Temporal *t1, Temporal *t2)
{
	return !temporal_eq_internal(t1, t2);
}

PG_FUNCTION_INFO_V1(temporal_ne);

PGDLLEXPORT Datum
temporal_ne(PG_FUNCTION_ARGS)
{
	Temporal *t1 = PG_GETARG_TEMPORAL(0);
	Temporal *t2 = PG_GETARG_TEMPORAL(1);
	bool result = temporal_ne_internal(t1, t2);
	PG_FREE_IF_COPY(t1, 0);
	PG_FREE_IF_COPY(t2, 1);
	PG_RETURN_BOOL(result);
}

/* Comparison operators using the internal B-tree comparator */

PG_FUNCTION_INFO_V1(temporal_lt);

PGDLLEXPORT Datum
temporal_lt(PG_FUNCTION_ARGS)
{
	Temporal *t1 = PG_GETARG_TEMPORAL(0);
	Temporal *t2 = PG_GETARG_TEMPORAL(1);
	int cmp = temporal_cmp_internal(t1, t2);
	PG_FREE_IF_COPY(t1, 0);
	PG_FREE_IF_COPY(t2, 1);
	if (cmp < 0)
		PG_RETURN_BOOL(true);
	else
		PG_RETURN_BOOL(false);
}

PG_FUNCTION_INFO_V1(temporal_le);

PGDLLEXPORT Datum
temporal_le(PG_FUNCTION_ARGS)
{
	Temporal *t1 = PG_GETARG_TEMPORAL(0);
	Temporal *t2 = PG_GETARG_TEMPORAL(1);
	int cmp = temporal_cmp_internal(t1, t2);
	PG_FREE_IF_COPY(t1, 0);
	PG_FREE_IF_COPY(t2, 1);
	if (cmp == 0)
		PG_RETURN_BOOL(true);
	else
		PG_RETURN_BOOL(false);
}

PG_FUNCTION_INFO_V1(temporal_ge);

PGDLLEXPORT Datum
temporal_ge(PG_FUNCTION_ARGS)
{
	Temporal *t1 = PG_GETARG_TEMPORAL(0);
	Temporal *t2 = PG_GETARG_TEMPORAL(1);
	int cmp = temporal_cmp_internal(t1, t2);
	PG_FREE_IF_COPY(t1, 0);
	PG_FREE_IF_COPY(t2, 1);
	if (cmp >= 0)
		PG_RETURN_BOOL(true);
	else
		PG_RETURN_BOOL(false);
}

PG_FUNCTION_INFO_V1(temporal_gt);

PGDLLEXPORT Datum 
temporal_gt(PG_FUNCTION_ARGS)
{
	Temporal *t1 = PG_GETARG_TEMPORAL(0);
	Temporal *t2 = PG_GETARG_TEMPORAL(1);
	int cmp = temporal_cmp_internal(t1, t2);
	PG_FREE_IF_COPY(t1, 0);
	PG_FREE_IF_COPY(t2, 1);
	if (cmp > 0)
		PG_RETURN_BOOL(true);
	else
		PG_RETURN_BOOL(false);
}

/*****************************************************************************
 * Functions for defining hash index
 *****************************************************************************/

uint32 
temporal_hash_internal(const Temporal *temp)
{
	uint32 result = 0;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = temporalinst_hash((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = temporali_hash((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_hash((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = temporals_hash((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(temporal_hash);

PGDLLEXPORT Datum 
temporal_hash(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	uint32 result = temporal_hash_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_UINT32(result);
}

/*****************************************************************************/
