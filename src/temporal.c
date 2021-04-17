/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file temporal.c
 * Basic functions for temporal types of any subtype.
 */

#include "temporal.h"

#include <assert.h>
#include <access/heapam.h>
#include <access/htup_details.h>
#if MOBDB_PGSQL_VERSION < 130000
#include <access/tuptoaster.h>
#else
#include <access/heaptoast.h>
#include <access/detoast.h>
#endif
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
 * Array storing the string representation of the concrete subtypes of
 * temporal types
 */
static char *temptypeName[] =
{
  "AnyDuration",
  "Instant",
  "InstantSet",
  "Sequence",
  "SequenceSet"
};

/**
 * Array storing the mapping between the string representation of the
 * subtypes of the temporal types and the corresponding enum value
 */
struct temptype_struct temptype_struct_array[] =
{
  {"ANYTEMPORALTYPE", ANYTEMPORALTYPE},
  {"INSTANT", INSTANT},
  {"INSTANTSET", INSTANTSET},
  {"SEQUENCE", SEQUENCE},
  {"SEQUENCESET", SEQUENCESET},
};

/**
 * Returns the string representation of the subtype of the
 * temporal type corresponding to the enum value
 */
const char *
temptype_name(TemporalType temptype)
{
  return temptypeName[temptype];
}

/**
 * Returns the enum value corresponding to the string representation
 * of the concrete subtype of the temporal type.
 */
bool
temptype_from_string(const char *str, TemporalType *temptype)
{
  char *tmpstr;
  size_t tmpstartpos, tmpendpos;
  size_t i;

  /* Initialize */
  *temptype = 0;
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
  for (i = 0; i < TEMPORALTYPE_STRUCT_ARRAY_LEN; i++)
  {
    if (len == strnlen(temptype_struct_array[i].temptypeName, TEMPORALTYPE_MAX_LEN) &&
      !strncasecmp(tmpstr, temptype_struct_array[i].temptypeName, TEMPORALTYPE_MAX_LEN))
    {
      *temptype = temptype_struct_array[i].temptype;
      pfree(tmpstr);
      return true;
    }
  }
  pfree(tmpstr);
  return false;
}

/**
 * Ensures that the temporal type of the temporal value corresponds to the typmod
 */
static Temporal *
temporal_valid_typmod(Temporal *temp, int32_t typmod)
{
  /* No typmod (-1) */
  if (typmod < 0)
    return temp;
  TemporalType typmod_temptype = TYPMOD_GET_TEMPTYPE(typmod);
  /* Typmod has a preference */
  if (typmod_temptype != ANYTEMPORALTYPE && typmod_temptype != temp->temptype)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Temporal type (%s) does not match column type (%s)",
      temptype_name(temp->temptype), temptype_name(typmod_temptype))));
  return temp;
}

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/**
 * Returns the temporal instant at the timestamp for timestamps that
 * are at an exclusive bound
 */
const TInstant *
tsequence_find_timestamp_excl(const TSequence *seq, TimestampTz t)
{
  const TInstant *result;
  if (t == seq->period.lower)
    result = tsequence_inst_n(seq, 0);
  else
    result = tsequence_inst_n(seq, seq->count - 1);
  return tinstant_copy(result);
}

/**
 * Returns the temporal instant at the timestamp when the timestamp is
 * at an exclusive bound
 */
const TInstant *
tsequenceset_find_timestamp_excl(const TSequenceSet *ts, TimestampTz t)
{
  const TInstant *result;
  int loc;
  tsequenceset_find_timestamp(ts, t, &loc);
  const TSequence *seq1, *seq2;
  if (loc == 0)
  {
    seq1 = tsequenceset_seq_n(ts, 0);
    result = tsequence_inst_n(seq1, 0);
  }
  else if (loc == ts->count)
  {
    seq1 = tsequenceset_seq_n(ts, ts->count - 1);
    result = tsequence_inst_n(seq1, seq1->count - 1);
  }
  else
  {
    seq1 = tsequenceset_seq_n(ts, loc - 1);
    seq2 = tsequenceset_seq_n(ts, loc);
    if (tsequence_end_timestamp(seq1) == t)
      result = tsequence_inst_n(seq1, seq1->count - 1);
    else
      result = tsequence_inst_n(seq2, 0);
  }
  return tinstant_copy(result);
}

/**
 * Returns a copy of the temporal value
 */
Temporal *
temporal_copy(const Temporal *temp)
{
  Temporal *result = (Temporal *)palloc0(VARSIZE(temp));
  memcpy(result, temp, VARSIZE(temp));
  return result;
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] temp1,temp2 Input values
 * @param[in] mode Either intersection or synchronization
 * @param[out] inter1,inter2 Output values
 * @result Returns false if the values do not overlap on time
 */
bool
intersection_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  TIntersection mode, Temporal **inter1, Temporal **inter2)
{
  bool result = false;
  ensure_valid_temptype(temp1->temptype);
  ensure_valid_temptype(temp2->temptype);
  if (temp1->temptype == INSTANT)
  {
    if (temp2->temptype == INSTANT)
      result = intersection_tinstant_tinstant(
        (TInstant *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->temptype == INSTANTSET)
      result = intersection_tinstant_tinstantset(
        (TInstant *) temp1, (TInstantSet *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->temptype == SEQUENCE)
      result = intersection_tinstant_tsequence(
        (TInstant *) temp1, (TSequence *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else /* temp2->temptype == SEQUENCESET */
      result = intersection_tinstant_tsequenceset(
        (TInstant *) temp1, (TSequenceSet *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
  }
  else if (temp1->temptype == INSTANTSET)
  {
    if (temp2->temptype == INSTANT)
      result = intersection_tinstantset_tinstant(
        (TInstantSet *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->temptype == INSTANTSET)
      result = intersection_tinstantset_tinstantset(
        (TInstantSet *) temp1, (TInstantSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else if (temp2->temptype == SEQUENCE)
      result = intersection_tinstantset_tsequence(
        (TInstantSet *) temp1, (TSequence *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else /* temp2->temptype == SEQUENCESET */
      result = intersection_tinstantset_tsequenceset(
        (TInstantSet *) temp1, (TSequenceSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
  }
  else if (temp1->temptype == SEQUENCE)
  {
    if (temp2->temptype == INSTANT)
      result = intersection_tsequence_tinstant(
        (TSequence *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->temptype == INSTANTSET)
      result = intersection_tsequence_tinstantset(
        (TSequence *) temp1, (TInstantSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else if (temp2->temptype == SEQUENCE)
      result = (mode == INTERSECT) ?
        intersection_tsequence_tsequence(
          (TSequence *) temp1, (TSequence *) temp2,
          (TSequence **) inter1, (TSequence **) inter2) :
        synchronize_tsequence_tsequence(
          (TSequence *) temp1, (TSequence *) temp2,
          (TSequence **) inter1, (TSequence **) inter2,
            mode == SYNCHRONIZE_CROSS);
    else /* temp2->temptype == SEQUENCESET */
      result = intersection_tsequence_tsequenceset(
          (TSequence *) temp1, (TSequenceSet *) temp2, mode,
          (TSequenceSet **) inter1, (TSequenceSet **) inter2);
  }
  else /* temp1->temptype == SEQUENCESET */
  {
    if (temp2->temptype == INSTANT)
      result = intersection_tsequenceset_tinstant(
        (TSequenceSet *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->temptype == INSTANTSET)
      result = intersection_tsequenceset_tinstantset(
        (TSequenceSet *) temp1, (TInstantSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else if (temp2->temptype == SEQUENCE)
      result = intersection_tsequenceset_tsequence(
          (TSequenceSet *) temp1, (TSequence *) temp2, mode,
          (TSequenceSet **) inter1, (TSequenceSet **) inter2);
    else /* temp2->temptype == SEQUENCESET */
      result = intersection_tsequenceset_tsequenceset(
        (TSequenceSet *) temp1, (TSequenceSet *) temp2, mode,
        (TSequenceSet **) inter1, (TSequenceSet **) inter2);
  }
  return result;
}

/**
 * Returns true if the Oid corresponds to a continuous base type
 */
bool
continuous_base_type(Oid type)
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
 * Returns the Oid of the base type from the Oid of the temporal type
 */
Oid
temporal_valuetypid(Oid temptypid)
{
  Oid catalog = RelnameGetRelid("mobilitydb_typcache");
#if MOBDB_PGSQL_VERSION < 130000
  Relation rel = heap_open(catalog, AccessShareLock);
#else
  Relation rel = table_open(catalog, AccessShareLock);
#endif
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
#if MOBDB_PGSQL_VERSION < 130000
  heap_close(rel, AccessShareLock);
#else
  table_close(rel, AccessShareLock);
#endif
  if (! HeapTupleIsValid(tuple) || isnull)
    elog(ERROR, "type %u is not a temporal type", temptypid);
  return result;
}

/*****************************************************************************
 * Oid functions
 *****************************************************************************/

/**
 * Returns the Oid of the range type corresponding to the Oid of the
 * base type
 */
Oid
range_oid_from_base(Oid valuetypid)
{
  Oid result = 0;
  ensure_tnumber_base_type(valuetypid);
  if (valuetypid == INT4OID)
    result = type_oid(T_INTRANGE);
  else /* valuetypid == FLOAT8OID */
    result = type_oid(T_FLOATRANGE);
  return result;
}

/**
 * Returns the Oid of the temporal type corresponding to the Oid of the
 * base type
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
 * Returns the Oid of the base type corresponding to the Oid of the
 * temporal type
 */
Oid
base_oid_from_temporal(Oid temptypid)
{
  assert(temporal_type(temptypid));
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
 * Returns true if the temporal type corresponding to the Oid of the
 * base type has its trajectory precomputed
 */
bool
type_has_precomputed_trajectory(Oid valuetypid)
{
  if (tgeo_base_type(valuetypid))
    return true;
  return false;
}

/*****************************************************************************
 * Test families of base types and temporal types
 *****************************************************************************/

/**
 * Returns true if the Oid is a alpha base type (i.e., those whose bounding
 * box is a period) supported by MobilityDB
 */
bool
talpha_base_type(Oid typid)
{
  if (typid == BOOLOID || typid == TEXTOID)
    return true;
  return false;
}

/**
 * Test whether the Oid is a number base type supported by MobilityDB
 */
bool
tnumber_base_type(Oid typid)
{
  if (typid == INT4OID || typid == FLOAT8OID)
    return true;
  return false;
}

/**
 * Returns true if the Oid is a temporal number type
 *
 * @note Function used in particular in the indexes
 */
bool
tnumber_range_type(Oid typid)
{
  if (typid == type_oid(T_INTRANGE) || typid == type_oid(T_FLOATRANGE))
    return true;
  return false;
}

/**
 * Ensures that the Oid is a point base type supported by MobilityDB
 */
bool
tgeo_base_type(Oid typid)
{
  if (typid == type_oid(T_GEOMETRY) ||
    typid == type_oid(T_GEOGRAPHY))
    return true;
  return false;
}

/**
 * Returns true if the Oid is a temporal type
 *
 * @note Function used in particular in the indexes
 */
bool
temporal_type(Oid typid)
{
  if (typid == type_oid(T_TBOOL) || typid == type_oid(T_TINT) ||
    typid == type_oid(T_TFLOAT) || typid == type_oid(T_TTEXT) ||
    typid == type_oid(T_TGEOMPOINT) || typid == type_oid(T_TGEOGPOINT))
    return true;
  return false;
}

/**
 * Returns true if the Oid is a temporal number type
 *
 * @note Function used in particular in the indexes
 */
bool
tnumber_type(Oid typid)
{
  if (typid == type_oid(T_TINT) || typid == type_oid(T_TFLOAT))
    return true;
  return false;
}

/**
 * Returns true if the Oid is a temporal point type
 *
 * @note Function used in particular in the indexes
 */
bool
tgeo_type(Oid typid)
{
  if (typid == type_oid(T_TGEOMPOINT) || typid == type_oid(T_TGEOGPOINT))
    return true;
  return false;
}

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * Ensures that the number is positive
 */
void
ensure_positive_double(double size)
{
  if (size <= 0)
    elog(ERROR, "The float value must be positive: %f", size);
  return;
}

/**
 * Ensures that the interval is a positive and absolute duration
 */
void
ensure_valid_duration(const Interval *duration)
{
  if (duration->month != 0)
  {
    ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
      errmsg("Interval defined in terms of month, year, century etc. not supported")));
  }
  Interval intervalzero;
  memset(&intervalzero, 0, sizeof(Interval));
  int cmp = call_function2(interval_cmp, PointerGetDatum(duration),
    PointerGetDatum(&intervalzero));
  if (cmp <= 0)
  {
    char *t = call_output(INTERVALOID, PointerGetDatum(duration));
    elog(ERROR, "The interval must be positive: %s", t);
  }
  return;
}

/**
 * Ensures that the temporal type is valid
 *
 * @note Used for the dispatch functions
 */
void
ensure_valid_temptype(TemporalType temptype)
{
  if (temptype != INSTANT && temptype != INSTANTSET &&
    temptype != SEQUENCE && temptype != SEQUENCESET)
    elog(ERROR, "unknown subtype for temporal type: %d", temptype);
  return;
}

/**
 * Ensures that the temporal type is valid
 *
 * @note Used for the analyze and selectivity functions
 */
void
ensure_valid_temptype_all(TemporalType temptype)
{
  if (temptype != ANYTEMPORALTYPE &&
    temptype != INSTANT && temptype != INSTANTSET &&
    temptype != SEQUENCE && temptype != SEQUENCESET)
    elog(ERROR, "unknown subtype for temporal type: %d", temptype);
  return;
}

/**
 * Ensures that the temporal type is a sequence (set)
 */
void
ensure_sequences_type(TemporalType temptype)
{
  if (temptype != SEQUENCE && temptype != SEQUENCESET)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Input must be a temporal sequence (set)")));
  return;
}

/**
 * Ensures that the elements of the array are of instant subtype
 */
void
ensure_tinstantarr(TInstant **instants, int count)
{
  for (int i = 0; i < count; i++)
  {
    if (instants[i]->temptype != INSTANT)
    {
      pfree(instants);
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Input values must be temporal instants")));
    }
  }
  return;
}

/**
 * Ensures that the Oid is an external base type supported by MobilityDB
 */
void
ensure_temporal_base_type(Oid valuetypid)
{
  if (valuetypid != BOOLOID && valuetypid != INT4OID &&
    valuetypid != FLOAT8OID && valuetypid != TEXTOID &&
    valuetypid != type_oid(T_GEOMETRY) &&
    valuetypid != type_oid(T_GEOGRAPHY))
    elog(ERROR, "unknown base type: %d", valuetypid);
  return;
}

/**
 * Ensures that the Oid is an external or an internal base type
 * supported by MobilityDB
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
  return;
}

/**
 * Ensures that the Oid is an external base type that is continuous
 */
void
ensure_continuous_base_type(Oid valuetypid)
{
  if (valuetypid != FLOAT8OID &&
    valuetypid != type_oid(T_GEOMETRY) &&
    valuetypid != type_oid(T_GEOGRAPHY))
    elog(ERROR, "unknown continuous base type: %d", valuetypid);
  return;
}

/**
 * Ensures that the Oid is an internal or external base type that is continuous
 */
void
ensure_continuous_base_type_all(Oid valuetypid)
{
  if (valuetypid != FLOAT8OID &&
    valuetypid !=  type_oid(T_DOUBLE2) &&
    valuetypid != type_oid(T_GEOMETRY) &&
    valuetypid != type_oid(T_GEOGRAPHY) &&
    valuetypid != type_oid(T_DOUBLE3) &&
    valuetypid != type_oid(T_DOUBLE4))
    elog(ERROR, "unknown continuous base type: %d", valuetypid);
  return;
}

/**
 * Ensures that the Oid is a range type
 */
void
ensure_tnumber_range_type(Oid typid)
{
  if (! tnumber_range_type(typid))
    elog(ERROR, "unknown number range type: %d", typid);
  return;
}

/**
 * Ensures that the Oid is a number base type supported by MobilityDB
 */
void
ensure_tnumber_base_type(Oid valuetypid)
{
  if (! tnumber_base_type(valuetypid))
    elog(ERROR, "unknown number base type: %d", valuetypid);
  return;
}

/**
 * Ensures that the Oid is a point base type supported by MobilityDB
 */
void
ensure_tgeo_base_type(Oid valuetypid)
{
  if (! tgeo_base_type(valuetypid))
    elog(ERROR, "unknown geospatial base type: %d", valuetypid);
  return;
}

/**
 * Ensures that the array is not empty
 *
 * @note Used for the constructor functions
 */
void
ensure_non_empty_array(ArrayType *array)
{
  if (ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array)) == 0)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
      errmsg("The input array cannot be empty")));
  return;
}

/*****************************************************************************/

/**
 * Ensure that the temporal value has linear interpolation
 */
void
ensure_linear_interpolation(int16 flags)
{
  if (! MOBDB_FLAGS_GET_LINEAR(flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal value must have linear interpolation")));
  return;
}

/**
 * Ensure that the temporal values have at least one common dimension
 */
void
ensure_common_dimension(int16 flags1, int16 flags2)
{
  if (MOBDB_FLAGS_GET_X(flags1) != MOBDB_FLAGS_GET_X(flags2) &&
    MOBDB_FLAGS_GET_T(flags1) != MOBDB_FLAGS_GET_T(flags2))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal values must have at least one common dimension")));
  return;
}

/*****************************************************************************/

/**
 * Ensures that the two temporal values have the same base type
 */
void
ensure_same_base_type(const Temporal *temp1, const Temporal *temp2)
{
  if (temp1->valuetypid != temp2->valuetypid)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("The temporal values must be of the same base type")));
  return;
}

/**
 * Ensures that the two temporal values have the same interpolation
 */
void
ensure_same_interpolation(const Temporal *temp1, const Temporal *temp2)
{
  if (MOBDB_FLAGS_GET_LINEAR(temp1->flags) != MOBDB_FLAGS_GET_LINEAR(temp2->flags))
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("The temporal values must be of the same interpolation")));
  return;
}

/**
 * Ensures that the timestamp of the first temporal instant is smaller
 * (or equal if the merge parameter is true) than the one of the second
 * temporal instant. Moreover, ensures that the values are the same
 * if the timestamps are equal
 */
void
ensure_increasing_timestamps(const TInstant *inst1, const TInstant *inst2,
  bool merge)
{
  if ((merge && inst1->t > inst2->t) || (!merge && inst1->t >= inst2->t))
  {
    char *t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
    char *t2 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst2->t));
    ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
      errmsg("Timestamps for temporal value must be increasing: %s, %s", t1, t2)));
  }
  if (merge && inst1->t == inst2->t &&
    ! datum_eq(tinstant_value(inst1), tinstant_value(inst2), inst1->valuetypid))
  {
    char *t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("The temporal values have different value at their overlapping instant %s", t1)));
  }
  return;
}

/**
 * Ensures that all temporal instant values of the array have increasing
 * timestamp (or may be equal if the merge parameter is true), and if they
 * are temporal points, have the same srid and the same dimensionality.
 */
void
ensure_valid_tinstantarr(const TInstant **instants, int count, bool merge)
{
  for (int i = 1; i < count; i++)
  {
    ensure_same_interpolation((Temporal *) instants[i - 1], (Temporal *) instants[i]);
    ensure_increasing_timestamps(instants[i - 1], instants[i], merge);
    ensure_spatial_validity((Temporal *) instants[i - 1], (Temporal *) instants[i]);
  }
  return;
}

/**
 * Ensures that all temporal instant values of the array have increasing
 * timestamp, and if they are temporal points, have the same srid and the
 * same dimensionality
 */
void
ensure_valid_tsequencearr(const TSequence **sequences, int count)
{
  for (int i = 1; i < count; i++)
  {
    ensure_same_interpolation((Temporal *) sequences[i - 1], (Temporal *) sequences[i]);
    if (sequences[i - 1]->period.upper > sequences[i]->period.lower ||
      (sequences[i - 1]->period.upper == sequences[i]->period.lower &&
      sequences[i - 1]->period.upper_inc && sequences[i]->period.lower_inc))
    {
      char *t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(sequences[i - 1]->period.upper));
      char *t2 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(sequences[i]->period.lower));
      ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
        errmsg("Timestamps for temporal value must be increasing: %s, %s", t1, t2)));
    }
    ensure_spatial_validity((Temporal *)sequences[i - 1], (Temporal *)sequences[i]);
  }
  return;
}

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(mobilitydb_version);
/**
 * Version of the MobilityDB extension
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
 * Versions of the MobilityDB extension and its dependencies
 */
PGDLLEXPORT Datum
mobilitydb_full_version(PG_FUNCTION_ARGS)
{
  char ver[128];
  text *result;

  snprintf(ver, 128, "%s, %s, %s", MOBDB_VERSION_STR,
    MOBDB_PGSQL_VERSION_STR, MOBDB_POSTGIS_VERSION_STR);
  ver[127] = '\0';

  result = cstring_to_text(ver);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_in);
/**
 * Generic input function for temporal types
 *
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
 * Returns the string representation of the temporal value
 * (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] value_out Function called to output the base value
 * depending on its Oid
 */
char *
temporal_to_string(const Temporal *temp, char *(*value_out)(Oid, Datum))
{
  char *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_to_string((TInstant *) temp, value_out);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_to_string((TInstantSet *) temp, value_out);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_to_string((TSequence *) temp, false, value_out);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_to_string((TSequenceSet *) temp, value_out);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_out);
/**
 * Generic output function for temporal types
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
 * Write the binary representation of the temporal value
 * into the buffer (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] buf Buffer
 */
void
temporal_write(const Temporal *temp, StringInfo buf)
{
  pq_sendbyte(buf, (uint8) temp->temptype);
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    tinstant_write((TInstant *) temp, buf);
  else if (temp->temptype == INSTANTSET)
    tinstantset_write((TInstantSet *) temp, buf);
  else if (temp->temptype == SEQUENCE)
    tsequence_write((TSequence *) temp, buf);
  else /* temp->temptype == SEQUENCESET */
    tsequenceset_write((TSequenceSet *) temp, buf);
  return;
}

PG_FUNCTION_INFO_V1(temporal_send);
/*
 * Generic send function for temporal types
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
 * Returns a new temporal value from its binary representation
 * read from the buffer (dispatch function)
 *
 * @param[in] buf Buffer
 * @param[in] valuetypid Oid of the base type
 */
Temporal *
temporal_read(StringInfo buf, Oid valuetypid)
{
  int16 type = (int16) pq_getmsgbyte(buf);
  Temporal *result;
  ensure_valid_temptype(type);
  if (type == INSTANT)
    result = (Temporal *) tinstant_read(buf, valuetypid);
  else if (type == INSTANTSET)
    result = (Temporal *) tinstantset_read(buf, valuetypid);
  else if (type == SEQUENCE)
    result = (Temporal *) tsequence_read(buf, valuetypid);
  else /* type == SEQUENCESET */
    result = (Temporal *) tsequenceset_read(buf, valuetypid);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_recv);
/**
 * Generic receive function for temporal types
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
 * Input typmod information for temporal types
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
  TemporalType temptype = ANYTEMPORALTYPE;
  if (!temptype_from_string(s, &temptype))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Invalid temporal type modifier: %s", s)));

  pfree(elem_values);
  PG_RETURN_INT32((int32)temptype);
}

PG_FUNCTION_INFO_V1(temporal_typmod_out);
/**
 * Output typmod information for temporal types
 */
PGDLLEXPORT Datum
temporal_typmod_out(PG_FUNCTION_ARGS)
{
  char *s = (char *) palloc(64);
  char *str = s;
  int32 typmod = PG_GETARG_INT32(0);
  TemporalType temptype = TYPMOD_GET_TEMPTYPE(typmod);
  /* No type? Then no typmod at all. Return empty string.  */
  if (typmod < 0 || !temptype)
  {
    *str = '\0';
    PG_RETURN_CSTRING(str);
  }
  sprintf(str, "(%s)", temptype_name(temptype));
  PG_RETURN_CSTRING(s);
}

PG_FUNCTION_INFO_V1(temporal_enforce_typmod);
/**
 * Enforce typmod information for temporal types
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

PG_FUNCTION_INFO_V1(tinstant_constructor);
/**
 * Construct a temporal instant value from the arguments
 */
PGDLLEXPORT Datum
tinstant_constructor(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Oid  valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  Temporal *result = (Temporal *) tinstant_make(value, t, valuetypid);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tinstantset_constructor);
/**
 * Construct a temporal instant set value from the array of temporal
 * instant values
 */
PGDLLEXPORT Datum
tinstantset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  TInstant **instants = (TInstant **) temporalarr_extract(array, &count);
  ensure_tinstantarr(instants, count);
  Temporal *result = (Temporal *) tinstantset_make((const TInstant **) instants,
    count, MERGE_NO);
  pfree(instants);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Construct a temporal sequence value from the array of temporal
 * instant values
 */
Datum
tsequence_constructor(FunctionCallInfo fcinfo, bool get_interp)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  bool lower_inc = PG_GETARG_BOOL(1);
  bool upper_inc = PG_GETARG_BOOL(2);
  bool linear = get_interp ? PG_GETARG_BOOL(3) : STEP;
  ensure_non_empty_array(array);
  int count;
  TInstant **instants = (TInstant **) temporalarr_extract(array, &count);
  ensure_tinstantarr(instants, count);
  Temporal *result = (Temporal *) tsequence_make((const TInstant **) instants,
    count, lower_inc, upper_inc, linear, NORMALIZE);
  pfree(instants);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tstepseq_constructor);
/**
 * Construct a temporal sequence value with stepwise interpolation from
 * the array of temporal instant values
 */
PGDLLEXPORT Datum
tstepseq_constructor(PG_FUNCTION_ARGS)
{
  return tsequence_constructor(fcinfo, false);
}

PG_FUNCTION_INFO_V1(tlinearseq_constructor);
/**
 * Construct a temporal sequence value with linear or stepwise
 * interpolation from the array of temporal instant values
 */
PGDLLEXPORT Datum
tlinearseq_constructor(PG_FUNCTION_ARGS)
{
  return tsequence_constructor(fcinfo, true);
}

PG_FUNCTION_INFO_V1(tsequenceset_constructor);
/**
 * Construct a temporal sequence set value from the array of temporal
 * sequence values
 */
PGDLLEXPORT Datum
tsequenceset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  TSequence **sequences = (TSequence **) temporalarr_extract(array, &count);
  bool linear = MOBDB_FLAGS_GET_LINEAR(sequences[0]->flags);
  /* Ensure that all values are of sequence subtype and of the same interpolation */
  for (int i = 0; i < count; i++)
  {
    if (sequences[i]->temptype != SEQUENCE)
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
  Temporal *result = (Temporal *) tsequenceset_make(
    (const TSequence **) sequences, count, NORMALIZE);
  pfree(sequences);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Tranformation functions
 ****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_append_tinstant);
/**
 * Append an instant to the end of a temporal value
 */
PGDLLEXPORT Datum
temporal_append_tinstant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *inst = PG_GETARG_TEMPORAL(1);
  /* Validity tests */
  if (inst->temptype != INSTANT)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("The second argument must be of instant subtype")));
  ensure_same_base_type(temp, (Temporal *) inst);
  /* The test to ensure the increasing timestamps must be done in the
   * specific function since the inclusive/exclusive bounds must be
   * taken into account for temporal sequences and sequence sets */
  ensure_spatial_validity(temp, inst);

  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tinstant_merge((TInstant *) temp, (TInstant *) inst);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_append_tinstant((TInstantSet *) temp,
      (TInstant *) inst);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tsequence_append_tinstant((TSequence *) temp,
      (TInstant *) inst);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_append_tinstant((TSequenceSet *) temp,
      (TInstant *) inst);

  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(inst, 1);
  PG_RETURN_POINTER(result);
}

/**
 * Convert two temporal values into a common subtype
 *
 * @param[in] temp1,temp2 Input values
 * @param[out] out1,out2 Output values
 */
static void
temporal_convert_same_type(const Temporal *temp1, const Temporal *temp2,
  Temporal **out1, Temporal **out2)
{
  assert(temp1->valuetypid == temp2->valuetypid);
  ensure_valid_temptype(temp1->temptype);
  ensure_valid_temptype(temp2->temptype);

  /* If both are of the same subtype do nothing */
  if (temp1->temptype == temp2->temptype)
  {
    *out1 = temporal_copy(temp1);
    *out2 = temporal_copy(temp2);
    return;
  }

  /* Different subtype */
  bool swap = false;
  Temporal *new1, *new2;
  if (temp1->temptype > temp2->temptype)
  {
    new1 = (Temporal *) temp2;
    new2 = (Temporal *) temp1;
    swap = true;
  }
  else
  {
    new1 = (Temporal *) temp1;
    new2 = (Temporal *) temp2;
  }

  Temporal *new, *newts = NULL;
  if (new1->temptype == INSTANT)
  {
    if (new2->temptype == INSTANTSET)
      new = (Temporal *) tinstant_to_tinstantset((TInstant *) new1);
    else if (new2->temptype == SEQUENCE)
      new = (Temporal *) tinstant_to_tsequence((TInstant *) new1,
        MOBDB_FLAGS_GET_LINEAR(new2->flags));
    else /* new2->temptype == SEQUENCESET */
      new = (Temporal *) tinstant_to_tsequenceset((TInstant *) new1,
      MOBDB_FLAGS_GET_LINEAR(new2->flags));
  }
  else if (new1->temptype == INSTANTSET)
  {
    if (new2->temptype == SEQUENCE)
    {
      if (((TInstantSet *) new1)->count == 1)
        new = (Temporal *) tinstantset_to_tsequence((TInstantSet *) new1,
          MOBDB_FLAGS_GET_LINEAR(new2->flags));
      else
      {
        new = (Temporal *) tinstantset_to_tsequenceset((TInstantSet *) new1,
          MOBDB_FLAGS_GET_LINEAR(new2->flags));
        newts = (Temporal *) tsequence_to_tsequenceset((TSequence *) new2);
      }
    }
    else /* new2->temptype == SEQUENCESET */
      new = (Temporal *) tinstantset_to_tsequenceset((TInstantSet *) new1,
        MOBDB_FLAGS_GET_LINEAR(new2->flags));
  }
  else /* new1->temptype == SEQUENCE && new2->temptype == SEQUENCESET */
    new = (Temporal *) tsequence_to_tsequenceset((TSequence *) new1);
  if (swap)
  {
    *out1 = (newts == NULL) ? temporal_copy(temp1) : newts;
    *out2 = new;
  }
  else
  {
    *out1 = new;
    *out2 = (newts == NULL) ? temporal_copy(temp2) : newts;
  }
  return;
}

PG_FUNCTION_INFO_V1(temporal_merge);
/**
 * Merge the two temporal values
 *
 * @result Merged value. Returns NULL if both arguments are NULL.
 * If one argument is null the other argument is output.
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
  if (!temp2)
  {
    result = temporal_copy(temp1);
    PG_FREE_IF_COPY(temp1, 0);
    PG_RETURN_POINTER(result);
  }

  /* Both arguments are temporal */
  ensure_same_base_type(temp1, temp2);
  ensure_same_interpolation(temp1, temp2);

  /* Convert to the same subtype if possible */
  Temporal *new1, *new2;
  temporal_convert_same_type(temp1, temp2, &new1, &new2);

  ensure_valid_temptype(new1->temptype);
  if (new1->temptype == INSTANT)
    result = tinstant_merge(
      (TInstant *) new1, (TInstant *)new2);
  else if (new1->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_merge(
      (TInstantSet *)new1, (TInstantSet *)new2);
  else if (new1->temptype == SEQUENCE)
    result = (Temporal *) tsequence_merge((TSequence *)new1,
      (TSequence *)new2);
  else /* new1->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_merge((TSequenceSet *)new1,
      (TSequenceSet *)new2);
  if (temp1 != new1)
    pfree(new1);
  if (temp2 != new2)
    pfree(new2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_POINTER(result);
}

/**
 * Convert the array of temporal values into a common subtype
 *
 * @param[in] temparr Array of values
 * @param[in] count Number of values
 * @param[in] temptype common subtype
 * @result  Array of output values
 */
static Temporal **
temporalarr_convert_temptype(Temporal **temparr, int count, TemporalType temptype)
{
  ensure_valid_temptype(temptype);
  Temporal **result = palloc(sizeof(Temporal *) * count);
  for (int i = 0; i < count; i++)
  {
    assert(temptype >= temparr[i]->temptype);
    if (temparr[i]->temptype == temptype)
      result[i] = temporal_copy(temparr[i]);
    else if (temparr[i]->temptype == INSTANT)
    {
      if (temptype == INSTANTSET)
        result[i] = (Temporal *) tinstant_to_tinstantset((TInstant *) temparr[i]);
      else if (temptype == SEQUENCE)
        result[i] = (Temporal *) tinstant_to_tsequence((TInstant *) temparr[i],
          MOBDB_FLAGS_GET_LINEAR(temparr[i]->flags));
      else /* temptype == SEQUENCESET */
        result[i] = (Temporal *) tinstant_to_tsequenceset((TInstant *) temparr[i],
          MOBDB_FLAGS_GET_LINEAR(temparr[i]->flags));
    }
    else if (temparr[i]->temptype == INSTANTSET)
    {
      /* An instant set can only be converted to a sequence set */
      assert(temptype == SEQUENCESET);
      result[i] = (Temporal *) tinstantset_to_tsequenceset((TInstantSet *) temparr[i],
        MOBDB_FLAGS_GET_LINEAR(temparr[i]->flags));
    }
    else /* temparr[i]->temptype == SEQUENCE && temptype == SEQUENCESET */
      result[i] = (Temporal *) tsequence_to_tsequenceset((TSequence *) temparr[i]);
  }
  return result;
}

PG_FUNCTION_INFO_V1(temporal_merge_array);
/**
 * Merge the array of temporal values
 */
PGDLLEXPORT Datum
temporal_merge_array(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  Temporal **temparr = temporalarr_extract(array, &count);
  if (count == 1)
  {
    Temporal *result = temporal_copy(temparr[0]);
    pfree(temparr);
    PG_FREE_IF_COPY(array, 0);
    PG_RETURN_POINTER(result);
  }

  /* Ensure all values have the same interpolation and determine
   * temporal subtype of the result */
  TemporalType temptype = temparr[0]->temptype;
  bool interpolation = MOBDB_FLAGS_GET_LINEAR(temparr[0]->flags);
  for (int i = 1; i < count; i++)
  {
    if (MOBDB_FLAGS_GET_LINEAR(temparr[i]->flags) != interpolation)
    {
      PG_FREE_IF_COPY(array, 0);
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Input values must be of the same interpolation")));
    }
    if (temptype != temparr[i]->temptype)
    {
      /* A TInstantSet cannot be converted to a TSequence */
      TemporalType newtemptype = Max((int16) temptype, (int16) temparr[i]->temptype);
      if (temptype == INSTANTSET && newtemptype == SEQUENCE)
        newtemptype = SEQUENCESET;
      temptype = newtemptype;
    }
  }
  /* Convert all temporal values to a single subtype if needed */
  Temporal **newtemps;
  if (temptype != temparr[0]->temptype)
    newtemps = temporalarr_convert_temptype(temparr, count, temptype);
  else
    newtemps = temparr;

  Temporal *result;
  ensure_valid_temptype(temptype);
  if (temptype == INSTANT)
    result = (Temporal *) tinstant_merge_array(
      (const TInstant **) newtemps, count);
  else if (temptype == INSTANTSET)
    result = tinstantset_merge_array(
      (const TInstantSet **) newtemps, count);
  else if (temptype == SEQUENCE)
    result = (Temporal *) tsequence_merge_array(
      (const TSequence **) newtemps, count);
  else /* temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_merge_array(
      (const TSequenceSet **) newtemps, count);

  pfree(temparr);
  if (temptype != temparr[0]->temptype)
    pfree_array((void **) newtemps, count);

  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * Cast the temporal integer value as a temporal float value
 *(dispatch function)
 */
Temporal *
tint_to_tfloat_internal(Temporal *temp)
{
  Temporal *result;
  if (temp->temptype == INSTANT)
    result = (Temporal *) tintinst_to_tfloatinst((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tintinstset_to_tfloatinstset((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tintseq_to_tfloatseq((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tintseqset_to_tfloatseqset((TSequenceSet *) temp);
  return result;
}

PG_FUNCTION_INFO_V1(tint_to_tfloat);
/**
 * Cast the temporal integer value as a temporal float value
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
 * Cast the temporal float value as a temporal integer value
 * (dispatch function)
 */
Temporal *
tfloat_to_tint_internal(Temporal *temp)
{
  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tfloatinst_to_tintinst((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tfloatinstset_to_tintinstset((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tfloatseq_to_tintseq((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tfloatseqset_to_tintseqset((TSequenceSet *) temp);
  return result;
}

PG_FUNCTION_INFO_V1(tfloat_to_tint);
/**
 * Cast the temporal float value as a temporal integer value
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
 * Returns the bounding period on which the temporal value is defined
 * (dispatch function)
 */
void
temporal_period(Period *p, const Temporal *temp)
{
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    tinstant_period(p, (TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    tinstantset_period(p, (TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    tsequence_period(p, (TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    tsequenceset_period(p, (TSequenceSet *) temp);
  return;
}

PG_FUNCTION_INFO_V1(temporal_to_period);
/**
 * Returns the bounding period on which the temporal value is defined
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

PG_FUNCTION_INFO_V1(temporal_to_tinstant);
/**
 * Transform the temporal value into a temporal instant value
 */
PGDLLEXPORT Datum
temporal_to_tinstant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = temporal_copy(temp);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_to_tinstant((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tsequence_to_tinstant((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_to_tinstant((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_to_tinstantset);
/**
 * Transform the temporal value into a temporal instant set value
 */
PGDLLEXPORT Datum
temporal_to_tinstantset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tinstant_to_tinstantset((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = temporal_copy(temp);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tsequence_to_tinstantset((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_to_tinstantset((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_to_tsequence);
/**
 * Transform the temporal value into a temporal sequence value
 */
PGDLLEXPORT Datum
temporal_to_tsequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result;
  bool linear = MOBDB_FLAGS_GET_LINEAR(temp->flags);
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tinstant_to_tsequence((TInstant *) temp, linear);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_to_tsequence((TInstantSet *) temp, linear);
  else if (temp->temptype == SEQUENCE)
    result = temporal_copy(temp);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_to_tsequence((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_to_tsequenceset);
/**
 * Transform the temporal value into a temporal sequence set value
 */
PGDLLEXPORT Datum
temporal_to_tsequenceset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result;
  bool linear = MOBDB_FLAGS_GET_LINEAR(temp->flags);
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tinstant_to_tsequenceset((TInstant *) temp, linear);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_to_tsequenceset((TInstantSet *) temp, linear);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = temporal_copy(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tstep_to_linear);
/**
 * Transform the temporal value with continuous base type from stepwise
 * to linear interpolation
 */
PGDLLEXPORT Datum
tstep_to_linear(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_sequences_type(temp->temptype);
  ensure_continuous_base_type(temp->valuetypid);

  if (MOBDB_FLAGS_GET_LINEAR(temp->flags))
    PG_RETURN_POINTER(temporal_copy(temp));

  Temporal *result;
  if (temp->temptype == SEQUENCE)
    result = (Temporal *) tstepseq_to_linear((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tstepseqset_to_linear((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_subtype);
/**
 * Returns the string representation of the temporal type
 */
Datum temporal_subtype(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  char str[12];
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    strcpy(str, "Instant");
  else if (temp->temptype == INSTANTSET)
    strcpy(str, "InstantSet");
  else if (temp->temptype == SEQUENCE)
    strcpy(str, "Sequence");
  else /* temp->temptype == SEQUENCESET */
    strcpy(str, "SequenceSet");
  text *result = cstring_to_text(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(temporal_interpolation);
/**
 * Returns the string representation of the temporal interpolation
 */
Datum temporal_interpolation(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  char str[12];
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT || temp->temptype == INSTANTSET)
    strcpy(str, "Discrete");
  else if (temp->temptype == SEQUENCE || temp->temptype == SEQUENCESET)
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
 * Returns the size in bytes of the temporal value
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
 * Returns the base values of the temporal value as a PostgreSQL array
 * (dispatch function)
 */
Datum
temporal_values(Temporal *temp)
{
  ArrayType *result;  /* make the compiler quiet */
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_values((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_values((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_values((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_values((TSequenceSet *) temp);
  return PointerGetDatum(result);
}

PG_FUNCTION_INFO_V1(temporal_get_values);
/**
 * Returns the base values of the temporal value as an array
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
 * Returns the base values of the temporal float value as an array of ranges
 * (dispatch function)
 */
Datum
tfloat_ranges(const Temporal *temp)
{
  ArrayType *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tfloatinst_ranges((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = tfloatinstset_ranges((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = tfloatseq_ranges((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tfloatseqset_ranges((TSequenceSet *) temp);
  return PointerGetDatum(result);
}

PG_FUNCTION_INFO_V1(tfloat_get_ranges);
/**
 * Returns the base values of the temporal float value as an array
 * of ranges
 */
PGDLLEXPORT Datum
tfloat_get_ranges(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result = tfloat_ranges(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tinstant_get_value);
/**
 * Returns the base value of the temporal instant value
 */
PGDLLEXPORT Datum
tinstant_get_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  if (temp->temptype != INSTANT)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Input must be a temporal instant")));

  TInstant *inst = (TInstant *) temp;
  Datum result = tinstant_value_copy(inst);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/**
 * Returns the time on which the temporal value is defined as a period set
 * (dispatch function)
 */
PeriodSet *
temporal_get_time_internal(const Temporal *temp)
{
  PeriodSet *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_get_time((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_get_time((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_get_time((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_get_time((TSequenceSet *) temp);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_get_time);
/**
 * Returns the time on which the temporal value is defined as a period set
 */
PGDLLEXPORT Datum
temporal_get_time(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  PeriodSet *result = temporal_get_time_internal(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tinstant_timestamp);
/**
 * Returns the timestamp of the temporal instant value
 */
PGDLLEXPORT Datum
tinstant_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  if (temp->temptype != INSTANT)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Input must be a temporal instant")));

  TimestampTz result = ((TInstant *) temp)->t;
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

/**
 * Returns a pointer to the precomputed bounding box of the temporal value
 *
 * @return Returns NULL for temporal instant values since they do not have
 * precomputed bounding box.
 */
void *
temporal_bbox_ptr(const Temporal *temp)
{
  void *result = NULL;
  if (temp->temptype == INSTANTSET)
    result = tinstantset_bbox_ptr((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_bbox_ptr((TSequence *) temp);
  else if (temp->temptype == SEQUENCESET)
    result = tsequenceset_bbox_ptr((TSequenceSet *) temp);
  return result;
}

/**
 * Set the first argument to the bounding box of the temporal value
 *
 * For temporal instant values the bounding box must be computed.
 * For the other subtypes a copy of the precomputed bounding box
 * is made.
 */
void
temporal_bbox(void *box, const Temporal *temp)
{
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    tinstant_make_bbox(box, (TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    tinstantset_bbox(box, (TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    tsequence_bbox(box, (TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    tsequenceset_bbox(box, (TSequenceSet *) temp);
  return;
}

PG_FUNCTION_INFO_V1(tnumber_to_tbox);
/**
 * Returns the bounding box of the temporal value
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
 * Returns the value range of the temporal integer value
 * (internal function)
 */
RangeType *
tnumber_value_range_internal(const Temporal *temp)
{
  RangeType *result = NULL;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
  {
    Datum value = tinstant_value((TInstant *) temp);
    result = range_make(value, value, true, true, temp->valuetypid);
  }
  else
  {
    TBOX *box = (TBOX *) temporal_bbox_ptr(temp);
    Datum min = 0, max = 0;
    ensure_tnumber_base_type(temp->valuetypid);
    if (temp->valuetypid == INT4OID)
    {
      min = Int32GetDatum((int)(box->xmin));
      max = Int32GetDatum((int)(box->xmax));
    }
    else /* temp->valuetypid == FLOAT8OID */
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
 * Returns the value range of the temporal integer value
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
 * Returns the start base value of the temporal value
 */
PGDLLEXPORT Datum
temporal_start_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = tinstant_value_copy(tinstantset_inst_n((TInstantSet *) temp, 0));
  else if (temp->temptype == SEQUENCE)
    result = tinstant_value_copy(tsequence_inst_n((TSequence *) temp, 0));
  else /* temp->temptype == SEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp, 0);
    result = tinstant_value_copy(tsequence_inst_n(seq, 0));
  }
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(temporal_end_value);
/**
 * Returns the end base value of the temporal value
 */
PGDLLEXPORT Datum
temporal_end_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = tinstant_value_copy(tinstantset_inst_n((TInstantSet *) temp,
      ((TInstantSet *) temp)->count - 1));
  else if (temp->temptype == SEQUENCE)
    result = tinstant_value_copy(tsequence_inst_n((TSequence *) temp,
      ((TSequence *) temp)->count - 1));
  else /* temp->temptype == SEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp,
      ((TSequenceSet *) temp)->count - 1);
    result = tinstant_value_copy(tsequence_inst_n(seq, seq->count - 1));
  }
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/**
 * Returns a pointer to the instant with minimum base value of the
 * temporal value.
 *
 * The function does not take into account whether the
 * instant is at an exclusive bound or not.
 *
 * @note Function used, e.g., for computing the shortest line between two
 *temporal points from their temporal distance
 */
const TInstant *
temporal_min_instant(const Temporal *temp)
{
  const TInstant *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (TInstant *) temp;
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_min_instant((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_min_instant((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_min_instant((TSequenceSet *) temp);
  return result;
}

/**
 * Returns the minimum base value of the temporal value
 *(dispatch function)
 */
Datum
temporal_min_value_internal(const Temporal *temp)
{
  Datum result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = datum_copy(tinstantset_min_value((TInstantSet *) temp),
      temp->valuetypid);
  else if (temp->temptype == SEQUENCE)
    result = datum_copy(tsequence_min_value((TSequence *) temp),
      temp->valuetypid);
  else /* temp->temptype == SEQUENCESET */
    result = datum_copy(tsequenceset_min_value((TSequenceSet *) temp),
      temp->valuetypid);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_min_value);
/**
 * Returns the minimum base value of the temporal value
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
 * Returns the maximum base value of the temporal value
 */
Datum
temporal_max_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = datum_copy(tinstantset_max_value((TInstantSet *) temp),
      temp->valuetypid);
  else if (temp->temptype == SEQUENCE)
    result = datum_copy(tsequence_max_value((TSequence *) temp),
      temp->valuetypid);
  else /* temp->temptype == SEQUENCESET */
    result = datum_copy(tsequenceset_max_value((TSequenceSet *) temp),
      temp->valuetypid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(temporal_timespan);
/**
 * Returns the timespan of the temporal value
 */
PGDLLEXPORT Datum
temporal_timespan(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
  {
    Interval *interval = (Interval *) palloc(sizeof(Interval));
    interval->month = interval->day =  0;
    interval->time = (TimeOffset) 0;
    result = PointerGetDatum(interval);
  }
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_timespan((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_duration((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_timespan((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(temporal_duration);
/**
 * Returns the duration of the temporal value
 */
PGDLLEXPORT Datum
temporal_duration(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT || temp->temptype == INSTANTSET)
  {
    Interval *interval = (Interval *) palloc(sizeof(Interval));
    interval->month = interval->day = 0;
    interval->time = (TimeOffset) 0;
    result = PointerGetDatum(interval);
  }
  else if (temp->temptype == SEQUENCE)
    result = tsequence_duration((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_duration((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(temporal_num_sequences);
/**
 * Returns the number of sequences of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
temporal_num_sequences(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_sequences_type(temp->temptype);
  int result = 1;
  if (temp->temptype == SEQUENCESET)
    result = ((TSequenceSet *) temp)->count;
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(temporal_start_sequence);
/**
 * Returns the start sequence of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
temporal_start_sequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_sequences_type(temp->temptype);
  TSequence *result;
  if (temp->temptype == SEQUENCE)
    result = tsequence_copy((TSequence *) temp);
  else
    result = tsequence_copy(tsequenceset_seq_n((TSequenceSet *) temp, 0));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_end_sequence);
/**
 * Returns the end sequence of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
temporal_end_sequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_sequences_type(temp->temptype);
  TSequence *result;
  if (temp->temptype == SEQUENCE)
    result = tsequence_copy((TSequence *) temp);
  else
  {
    TSequenceSet *ts = (TSequenceSet *) temp;
    result = tsequence_copy(tsequenceset_seq_n(ts, ts->count - 1));
  }
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_sequence_n);
/**
 * Returns the n-th sequence of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
temporal_sequence_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_sequences_type(temp->temptype);
  int i = PG_GETARG_INT32(1); /* Assume 1-based */
  TSequence *result = NULL;
  if (temp->temptype == SEQUENCE)
  {
    if (i == 1)
      result = tsequence_copy((TSequence *) temp);
  }
  else
  {
    TSequenceSet *ts = (TSequenceSet *) temp;
    if (i >= 1 && i <= ts->count)
      result = tsequence_copy(tsequenceset_seq_n(ts, i - 1));
  }
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_sequences);
/**
 * Returns the sequences of the temporal sequence (set) value as
 * an array
 */
PGDLLEXPORT Datum
temporal_sequences(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ArrayType *result;
  if (temp->temptype == INSTANT)
    result = tinstant_sequences_array((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_sequences_array((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = temporalarr_to_array((const Temporal **) &temp, 1);
  else /* temp->temptype == SEQUENCE */
    result = tsequenceset_sequences_array((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(temporal_segments);
/**
 * Returns the segments of the temporal sequence (set) value as
 * an array
 */
PGDLLEXPORT Datum
temporal_segments(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ArrayType *result;
  if (temp->temptype == INSTANT)
    result = tinstant_sequences_array((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_sequences_array((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_segments_array((TSequence *) temp);
  else
    result = tsequenceset_segments_array((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(temporal_num_instants);
/**
 * Returns the number of distinct instants of the temporal value
 */
PGDLLEXPORT Datum
temporal_num_instants(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  int result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = 1;
  else if (temp->temptype == INSTANTSET)
    result = ((TInstantSet *) temp)->count;
  else if (temp->temptype == SEQUENCE)
    result = ((TSequence *) temp)->count;
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_num_instants((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(temporal_start_instant);
/**
 * Returns the start instant of the temporal value
 */
PGDLLEXPORT Datum
temporal_start_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TInstant *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_copy((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = tinstant_copy(tinstantset_inst_n((TInstantSet *) temp, 0));
  else if (temp->temptype == SEQUENCE)
    result = tinstant_copy(tsequence_inst_n((TSequence *) temp, 0));
  else /* temp->temptype == SEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp, 0);
    result = tinstant_copy(tsequence_inst_n(seq, 0));
  }
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the end instant of the temporal value (internal function).
 * @note This function is used for validity testing and thus returns a
 * pointer to the last instant.
 */
const TInstant *
temporal_end_instant_internal(const Temporal *temp)
{
  const TInstant *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (TInstant *) temp;
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_inst_n((TInstantSet *) temp, ((TInstantSet *) temp)->count - 1);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_inst_n((TSequence *) temp, ((TSequence *) temp)->count - 1);
  else /* temp->temptype == SEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp,
      ((TSequenceSet *) temp)->count - 1);
    result = tsequence_inst_n(seq, seq->count - 1);
  }
  return result;
}

PG_FUNCTION_INFO_V1(temporal_end_instant);
/**
 * Returns the end instant of the temporal value
 */
PGDLLEXPORT Datum
temporal_end_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TInstant *result = tinstant_copy(temporal_end_instant_internal(temp));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the n-th instant of the temporal instant set or
 * a temporal sequence value.
 */
const TInstant *
tinstarr_inst_n(const Temporal *temp, int n)
{
  assert(temp->temptype == INSTANTSET || temp->temptype == SEQUENCE);
  if (temp->temptype == INSTANTSET)
    return tinstantset_inst_n((TInstantSet *) temp, n);
  else
    return tsequence_inst_n((TSequence *) temp, n);
}

PG_FUNCTION_INFO_V1(temporal_instant_n);
/**
 * Returns the n-th instant of the temporal value
 */
PGDLLEXPORT Datum
temporal_instant_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TInstant *result = NULL;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
  {
    if (n == 1)
      result = tinstant_copy((TInstant *) temp);
  }
  else if (temp->temptype == INSTANTSET)
  {
    if (n >= 1 && n <= ((TInstantSet *) temp)->count)
      result = tinstant_copy(
        tinstantset_inst_n((TInstantSet *) temp, n - 1));
  }
  else if (temp->temptype == SEQUENCE)
  {
    if (n >= 1 && n <= ((TSequence *) temp)->count)
      result = tinstant_copy(
        tsequence_inst_n((TSequence *) temp, n - 1));
  }
  else /* temp->temptype == SEQUENCESET */
  {
    /* This test is necessary since the n-th DISTINCT instant is requested */
    if (n >= 1 && n <= ((TSequenceSet *) temp)->totalcount)
    {
      const TInstant *inst = tsequenceset_inst_n((TSequenceSet *) temp, n);
      if (inst != NULL)
        result = tinstant_copy(inst);
    }
  }
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_instants);
/**
 * Returns the distinct instants of the temporal value as an array
 */
PGDLLEXPORT Datum
temporal_instants(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ArrayType *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_instants_array((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_instants_array((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_instants_array((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_instants_array((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/**
 * Returns the start timestamp of the temporal value
 * (dispatch function)
 */
TimestampTz
temporal_start_timestamp_internal(const Temporal *temp)
{
  TimestampTz result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = ((TInstant *) temp)->t;
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_inst_n((TInstantSet *) temp, 0)->t;
  else if (temp->temptype == SEQUENCE)
    result = tsequence_start_timestamp((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_start_timestamp((TSequenceSet *) temp);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_start_timestamp);
/**
 * Returns the start timestamp of the temporal value
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
 * Returns the end timestamp of the temporal value
 */
PGDLLEXPORT Datum
temporal_end_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TimestampTz result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = ((TInstant *) temp)->t;
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_inst_n((TInstantSet *) temp, ((TInstantSet *) temp)->count - 1)->t;
  else if (temp->temptype == SEQUENCE)
    result = tsequence_end_timestamp((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_end_timestamp((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(temporal_num_timestamps);
/**
 * Returns the number of distinct timestamps of the temporal value
 */
PGDLLEXPORT Datum
temporal_num_timestamps(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  int result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = 1;
  else if (temp->temptype == INSTANTSET)
    result = ((TInstantSet *) temp)->count;
  else if (temp->temptype == SEQUENCE)
    result = ((TSequence *) temp)->count;
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_num_timestamps((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_timestamp_n);
/**
 * Returns the n-th distinct timestamp of the temporal value
 */
PGDLLEXPORT Datum
temporal_timestamp_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TimestampTz result;
  bool found = false;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
  {
    if (n == 1)
    {
      found = true;
      result = ((TInstant *) temp)->t;
    }
  }
  else if (temp->temptype == INSTANTSET)
  {
    if (n >= 1 && n <= ((TInstantSet *) temp)->count)
    {
      found = true;
      result = (tinstantset_inst_n((TInstantSet *) temp, n - 1))->t;
    }
  }
  else if (temp->temptype == SEQUENCE)
  {
    if (n >= 1 && n <= ((TSequence *) temp)->count)
    {
      found = true;
      result = (tsequence_inst_n((TSequence *) temp, n - 1))->t;
    }
  }
  else /* temp->temptype == SEQUENCESET */
    found = tsequenceset_timestamp_n((TSequenceSet *) temp, n, &result);
  PG_FREE_IF_COPY(temp, 0);
  if (!found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(temporal_timestamps);
/**
 * Returns the distinct timestamps of the temporal value as an array
 */
PGDLLEXPORT Datum
temporal_timestamps(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ArrayType *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_timestamps((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_timestamps((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_timestamps((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_timestamps((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/**
 * Shift and/or scale the time span of the temporal value by the two intervals
 * (internal function)
 *
 * @param[in] temp Temporal value
 * @param[in] shift True when a shift of the timespan must be performed
 * @param[in] tscale True when a scale of the timespan must be performed
 * @param[in] start Interval for shift
 * @param[in] duration Interval for scale
 * @pre The duration is greater than 0 if is not NULL
 */
Temporal *
temporal_shift_tscale_internal(Temporal *temp, bool shift, bool tscale,
  Interval *start, Interval *duration)
{
  assert(start != NULL || duration != NULL);
  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (start != NULL) ?
      (Temporal *) tinstant_shift((TInstant *) temp, start) :
      (Temporal *) tinstant_copy((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_shift_tscale((TInstantSet *) temp,
    start, duration);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tsequence_shift_tscale((TSequence *) temp,
      start, duration);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_shift_tscale((TSequenceSet *) temp,
      start, duration);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_shift);
/**
 * Shift the time span of the temporal value by the interval
 */
PGDLLEXPORT Datum
temporal_shift(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Temporal *result = temporal_shift_tscale_internal(temp, true, false,
    start, NULL);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_tscale);
/**
 * Scale the time span of the temporal value by the interval
 */
PGDLLEXPORT Datum
temporal_tscale(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  Temporal *result = temporal_shift_tscale_internal(temp, false, true,
    NULL, duration);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_shift_tscale);
/**
 * Shift and scale the time span of the temporal value by the two intervals
 */
PGDLLEXPORT Datum
temporal_shift_tscale(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  ensure_valid_duration(duration);
  Temporal *result = temporal_shift_tscale_internal(temp, true, true,
    start, duration);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Bounding box tests for the ever/always comparison operators
 *****************************************************************************/

/**
 * Returns true if the bounding box of the temporal value is ever equal to
 * the base value
 */
bool
temporal_bbox_ev_al_eq(const Temporal *temp, Datum value, bool ever)
{
  /* Bounding box test */
  if (tnumber_base_type(temp->valuetypid))
  {
    TBOX box;
    memset(&box, 0, sizeof(TBOX));
    temporal_bbox(&box, temp);
    double d = datum_double(value, temp->valuetypid);
    return (ever && box.xmin <= d && d <= box.xmax) ||
      (!ever && box.xmin == d && d == box.xmax);
  }
  else if (tgeo_base_type(temp->valuetypid))
  {
    STBOX box1, box2;
    memset(&box1, 0, sizeof(STBOX));
    memset(&box2, 0, sizeof(STBOX));
    temporal_bbox(&box1, temp);
    geo_to_stbox_internal(&box2, (GSERIALIZED *)DatumGetPointer(value));
    return (ever && contains_stbox_stbox_internal(&box1, &box2)) ||
      (!ever && same_stbox_stbox_internal(&box1, &box2));
  }
  return true;
}

/**
 * Returns true if the bounding box of the temporal value is ever/always less
 * than or equal to the base value. The same test is used for both since the
 * bounding box does not distinguish between the inclusive/exclusive bounds.
 *
 * @param[in] temp Temporal value
 * @param[in] value Base value
 * @param[in] ever True when testing ever false when testing always
 */
bool
temporal_bbox_ev_al_lt_le(const Temporal *temp, Datum value, bool ever)
{
  if (tnumber_base_type(temp->valuetypid))
  {
    TBOX box;
    memset(&box, 0, sizeof(TBOX));
    temporal_bbox(&box, temp);
    double d = datum_double(value, temp->valuetypid);
    if ((ever && d < box.xmin) || (!ever && d < box.xmax))
      return false;
  }
  return true;
}

/*****************************************************************************
 * Ever/always comparison operators
 *****************************************************************************/

/**
 * Returns true if the temporal value is ever equal to the base value
 * (internal function)
 */
bool
temporal_ever_eq_internal(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_ever_eq((TInstant *) temp, value);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_ever_eq((TInstantSet *) temp, value);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_ever_eq((TSequence *) temp, value);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_ever_eq((TSequenceSet *) temp, value);
  return result;
}

/**
 * Returns true if the temporal value is always equal to the base value
 * (internal function)
 */
bool
temporal_always_eq_internal(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_always_eq((TInstant *) temp, value);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_always_eq((TInstantSet *) temp, value);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_always_eq((TSequence *) temp, value);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_always_eq((TSequenceSet *) temp, value);
  return result;
}

/**
 * Returns true if the temporal value is ever less than the base value
 * (internal function)
 */
bool
temporal_ever_lt_internal(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_ever_lt((TInstant *) temp, value);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_ever_lt((TInstantSet *) temp, value);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_ever_lt((TSequence *) temp, value);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_ever_lt((TSequenceSet *) temp, value);
  return result;
}

/**
 * Returns true if the temporal value is always less than the base value
 * (internal function)
 */
bool
temporal_always_lt_internal(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_always_lt((TInstant *) temp, value);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_always_lt((TInstantSet *) temp, value);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_always_lt((TSequence *) temp, value);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_always_lt((TSequenceSet *) temp, value);
  return result;
}

/**
 * Returns true if the temporal value is ever less than or equal to
 * the base value (internal function)
 */
bool
temporal_ever_le_internal(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_ever_le((TInstant *) temp, value);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_ever_le((TInstantSet *) temp, value);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_ever_le((TSequence *) temp, value);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_ever_le((TSequenceSet *) temp, value);
  return result;
}

/**
 * Returns true if the temporal value is always less than or equal to
 * the base value (internal function)
 */
bool
temporal_always_le_internal(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_always_le((TInstant *) temp, value);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_always_le((TInstantSet *) temp, value);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_always_le((TSequence *) temp, value);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_always_le((TSequenceSet *) temp, value);
  return result;
}

/**
 * Returns the interval in the same representation as Postgres timestamps.
 */
int64
get_interval_units(Interval *interval)
{
  return interval->time + (interval->day * USECS_PER_DAY);
}

/*****************************************************************************/

/**
 * Generic function for the temporal ever/always comparison operators
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
Datum
temporal_ev_al_comp(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, Datum))
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  /* For temporal points test that the geometry is not empty */
  if (tgeo_base_type(temp->valuetypid))
  {
    GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
    ensure_point_type(gs);
    ensure_same_srid_tpoint_gs(temp, gs);
    ensure_same_dimensionality_tpoint_gs(temp, gs);
    if (gserialized_is_empty(gs))
    {
      PG_FREE_IF_COPY(temp, 0);
      PG_RETURN_BOOL(false);
    }
  }
  bool result = func(temp, value);
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, temp->valuetypid, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_ever_eq);
/**
 * Returns true if the temporal value is ever equal to the base value
 */
PGDLLEXPORT Datum
temporal_ever_eq(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp(fcinfo, &temporal_ever_eq_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_eq);
/**
 * Returns true if the temporal value is always equal to the base value
 */
PGDLLEXPORT Datum
temporal_always_eq(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp(fcinfo, &temporal_always_eq_internal);
}

PG_FUNCTION_INFO_V1(temporal_ever_ne);
/**
 * Returns true if the temporal value is ever different from the base value
 */
PGDLLEXPORT Datum
temporal_ever_ne(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp(fcinfo, &temporal_always_eq_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_ne);
/**
 * Returns true if the temporal value is always different from the base value
 */
PGDLLEXPORT Datum
temporal_always_ne(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp(fcinfo, &temporal_ever_eq_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_ever_lt);
/**
 * Returns true if the temporal value is ever less than the base value
 */
PGDLLEXPORT Datum
temporal_ever_lt(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp(fcinfo, &temporal_ever_lt_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_lt);
/**
 * Returns true if the temporal value is always less than the base value
 */
PGDLLEXPORT Datum
temporal_always_lt(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp(fcinfo, &temporal_always_lt_internal);
}

PG_FUNCTION_INFO_V1(temporal_ever_le);
/**
 * Returns true if the temporal value is ever less than or equal to the base value
 */
PGDLLEXPORT Datum
temporal_ever_le(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp(fcinfo, &temporal_ever_le_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_le);
/**
 * Returns true if the temporal value is always less than or equal to the base value
 */
PGDLLEXPORT Datum
temporal_always_le(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp(fcinfo, &temporal_always_le_internal);
}

PG_FUNCTION_INFO_V1(temporal_ever_gt);
/**
 * Returns true if the temporal value is ever greater than the base value
 */
PGDLLEXPORT Datum
temporal_ever_gt(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp(fcinfo, &temporal_always_le_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_gt);
/**
 * Returns true if the temporal value is always greater than the base value
 */
PGDLLEXPORT Datum
temporal_always_gt(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp(fcinfo, &temporal_ever_le_internal);
}

PG_FUNCTION_INFO_V1(temporal_ever_ge);
/**
 * Returns true if the temporal value is ever greater than or equal
 * to the base value
 */
PGDLLEXPORT Datum
temporal_ever_ge(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp(fcinfo, &temporal_always_lt_internal);
}

PG_FUNCTION_INFO_V1(temporal_always_ge);
/**
 * Returns true if the temporal value is always greater than or equal
 * to the base value
 */
PGDLLEXPORT Datum
temporal_always_ge(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp(fcinfo, &temporal_ever_lt_internal);
}

/*****************************************************************************
 * Bounding box tests for the restriction functions
 *****************************************************************************/

/**
 * Returns true if the bounding box of the temporal value contains the base value
 */
bool
temporal_bbox_restrict_value(const Temporal *temp, Datum value)
{
  /* Bounding box test */
  if (tnumber_base_type(temp->valuetypid))
  {
    TBOX box1, box2;
    memset(&box1, 0, sizeof(TBOX));
    memset(&box2, 0, sizeof(TBOX));
    temporal_bbox(&box1, temp);
    number_to_box(&box2, value, temp->valuetypid);
    return contains_tbox_tbox_internal(&box1, &box2);
  }
  if (tgeo_base_type(temp->valuetypid))
  {
    /* Test that the geometry is not empty */
    GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(value);
    ensure_point_type(gs);
    ensure_same_srid_tpoint_gs(temp, gs);
    ensure_same_dimensionality_tpoint_gs(temp, gs);
    if (gserialized_is_empty(gs))
      return false;
    if (temp->temptype != INSTANT)
    {
      STBOX box1, box2;
      memset(&box1, 0, sizeof(STBOX));
      memset(&box2, 0, sizeof(STBOX));
      temporal_bbox(&box1, temp);
      geo_to_stbox_internal(&box2, gs);
      return contains_stbox_stbox_internal(&box1, &box2);
    }
  }
  return true;
}

/**
 * Returns the array of base values that are contained in the bounding box
 * of the temporal value.
 *
 * @param[in] temp Temporal value
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 * @return Filtered array of values.
 */
Datum *
temporal_bbox_restrict_values(const Temporal *temp, const Datum *values,
  int count, int *newcount)
{
  Datum *newvalues = palloc(sizeof(Datum) * count);
  int k = 0;

  /* Bounding box test */
  if (tnumber_base_type(temp->valuetypid))
  {
    TBOX box1;
    memset(&box1, 0, sizeof(TBOX));
    temporal_bbox(&box1, temp);
    for (int i = 0; i < count; i++)
    {
      TBOX box2;
      memset(&box2, 0, sizeof(TBOX));
      number_to_box(&box2, values[i], temp->valuetypid);
      if (contains_tbox_tbox_internal(&box1, &box2))
        newvalues[k++] = values[i];
    }
  }
  if (tgeo_base_type(temp->valuetypid))
  {
    STBOX box1;
    memset(&box1, 0, sizeof(STBOX));
    temporal_bbox(&box1, temp);
    for (int i = 0; i < count; i++)
    {
      /* Test that the geometry is not empty */
      GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(values[i]);
      ensure_point_type(gs);
      ensure_same_srid_tpoint_gs(temp, gs);
      ensure_same_dimensionality_tpoint_gs(temp, gs);
      if (! gserialized_is_empty(gs))
      {
        STBOX box2;
        memset(&box2, 0, sizeof(STBOX));
        geo_to_stbox_internal(&box2, gs);
        if (contains_stbox_stbox_internal(&box1, &box2))
          newvalues[k++] = values[i];
      }
    }
  }
  else
  {  /* For other types than the ones above */
    for (int i = 0; i < count; i++)
      newvalues[i] = values[i];
    k = count;
  }
  if (k == 0)
  {
    *newcount = k;
    pfree(newvalues);
    return NULL;
  }
  datumarr_sort(newvalues, k, temp->valuetypid);
  k = datumarr_remove_duplicates(newvalues, k, temp->valuetypid);
  *newcount = k;
  return newvalues;
}

/**
 * Returns true if the bounding box of the temporal number overlaps the range
 * of base values
 */
bool
tnumber_bbox_restrict_range(const Temporal *temp, const RangeType *range)
{
  /* Bounding box test */
  assert(tnumber_base_type(temp->valuetypid));
  TBOX box1, box2;
  memset(&box1, 0, sizeof(TBOX));
  memset(&box2, 0, sizeof(TBOX));
  temporal_bbox(&box1, temp);
  range_to_tbox_internal(&box2, range);
  return overlaps_tbox_tbox_internal(&box1, &box2);
}

/**
 * Returns the array of ranges of base values that overlap with the bounding box
 * of the temporal value.
 *
 * @param[in] temp Temporal value
 * @param[in] ranges Array of ranges of base values
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 * @return Filtered array of ranges.
 */
RangeType **
tnumber_bbox_restrict_ranges(const Temporal *temp, RangeType **ranges,
  int count, int *newcount)
{
  assert(tnumber_base_type(temp->valuetypid));
  RangeType **newranges = palloc(sizeof(Datum) * count);
  int k = 0;
  TBOX box1;
  memset(&box1, 0, sizeof(TBOX));
  temporal_bbox(&box1, temp);
  for (int i = 0; i < count; i++)
  {
    char flags = range_get_flags(ranges[i]);
    if (flags & RANGE_EMPTY)
      continue;
    TBOX box2;
    memset(&box2, 0, sizeof(TBOX));
    range_to_tbox_internal(&box2, ranges[i]);
    if (overlaps_tbox_tbox_internal(&box1, &box2))
      newranges[k++] = ranges[i];
  }
  if (k == 0)
  {
    *newcount = 0;
    pfree(newranges);
    return NULL;
  }
  RangeType **normranges = rangearr_normalize(newranges, k, newcount);
  pfree(newranges);
  return normranges;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * Restricts the temporal value to the (complement of the) base value
 * (dispatch function).
 *
 * @note This function does a bounding box test for the temporal types
 * different from instant. The singleton tests are done in the functions for
 * the specific temporal types.
 */
static Temporal *
temporal_restrict_value_internal(const Temporal *temp, Datum value,
  bool atfunc)
{
  /* Bounding box test */
  if (! temporal_bbox_restrict_value(temp, value))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->temptype != SEQUENCE) ? temporal_copy(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tinstant_restrict_value(
      (TInstant *) temp, value, atfunc);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_value(
      (TInstantSet *) temp, value, atfunc);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tsequence_restrict_value((TSequence *) temp,
      value, atfunc);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_value(
      (TSequenceSet *) temp, value, atfunc);
  return result;
}

/**
 * Restricts the temporal value to the (complement of the) array of base values
 */
Datum
temporal_restrict_value(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  Temporal *result = temporal_restrict_value_internal(temp, value, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, valuetypid, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_at_value);
/**
 * Restricts the temporal value to the base value
 */
PGDLLEXPORT Datum
temporal_at_value(PG_FUNCTION_ARGS)
{
  return temporal_restrict_value(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(temporal_minus_value);
/**
 * Restricts the temporal value to the complement of the base value
 */
PGDLLEXPORT Datum
temporal_minus_value(PG_FUNCTION_ARGS)
{
  return temporal_restrict_value(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * Restricts the temporal value to the (complement of the) array of base values
 * (dispatch function)
 */
static Temporal *
temporal_restrict_values_internal(const Temporal *temp, Datum *values,
  int count, bool atfunc)
{
  /* Bounding box test */
  int newcount;
  Datum *newvalues = temporal_bbox_restrict_values(temp, values, count,
    &newcount);
  if (newcount == 0)
  {
    if (atfunc)
      return NULL;
    else
      return (temp->temptype != SEQUENCE) ? temporal_copy(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tinstant_restrict_values(
      (TInstant *) temp, newvalues, newcount, atfunc);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_values(
      (TInstantSet *) temp, newvalues, newcount, atfunc);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tsequence_restrict_values((TSequence *) temp,
      newvalues, newcount, atfunc);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_values(
      (TSequenceSet *) temp, newvalues, newcount, atfunc);

  pfree(newvalues);
  return result;
}

/**
 * Restricts the temporal value to the (complement of the) array of base values
 */
Datum
temporal_restrict_values(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
  /* Return NULL or a copy of the temporal value on empty array */
  int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  if (count == 0)
  {
    PG_FREE_IF_COPY(array, 1);
    if (atfunc)
    {
      PG_FREE_IF_COPY(temp, 0);
      PG_RETURN_NULL();
    }
    else
    {
      Temporal *result = temporal_copy(temp);
      PG_FREE_IF_COPY(temp, 0);
      PG_RETURN_POINTER(result);
    }
  }

  Datum *values = datumarr_extract(array, &count);
  /* For temporal points the validity of values in the array is done in
   * bounding box function */
  Temporal *result = (count > 1) ?
    temporal_restrict_values_internal(temp, values, count, atfunc) :
    temporal_restrict_value_internal(temp, values[0], atfunc);

  pfree(values);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(array, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_at_values);
/**
 * Restricts the temporal value to the array of base values
 */
PGDLLEXPORT Datum
temporal_at_values(PG_FUNCTION_ARGS)
{
  return temporal_restrict_values(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(temporal_minus_values);
/**
 * Restricts the temporal value to the complement of the array of base values
 */
PGDLLEXPORT Datum
temporal_minus_values(PG_FUNCTION_ARGS)
{
  return temporal_restrict_values(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * Restricts the temporal value to the (complement of the) range of base values
 * (dispatch function)
 */
Temporal *
tnumber_restrict_range_internal(const Temporal *temp, RangeType *range,
  bool atfunc)
{
  /* Empty range and Bounding box test */
  char flags = range_get_flags(range);
  if (flags & RANGE_EMPTY ||
    ! tnumber_bbox_restrict_range(temp, range))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->temptype != SEQUENCE) ? temporal_copy(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tnumberinst_restrict_range(
      (TInstant *) temp, range, atfunc);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tnumberinstset_restrict_range(
      (TInstantSet *) temp, range, atfunc);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tnumberseq_restrict_range(
      (TSequence *) temp, range, atfunc);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tnumberseqset_restrict_range(
      (TSequenceSet *) temp, range, atfunc);
  return result;
}

Datum
tnumber_restrict_range(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
#if MOBDB_PGSQL_VERSION < 110000
  RangeType *range = PG_GETARG_RANGE(1);
#else
  RangeType *range = PG_GETARG_RANGE_P(1);
#endif
  Temporal *result = tnumber_restrict_range_internal(temp, range, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(range, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tnumber_at_range);
/**
 * Restricts the temporal value to the range of base values
 */
PGDLLEXPORT Datum
tnumber_at_range(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_range(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(tnumber_minus_range);
/**
 * Restricts the temporal value to the complement of the range of base values
 */
PGDLLEXPORT Datum
tnumber_minus_range(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_range(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * Restricts the temporal value to the (complement of the) array of ranges
 * of base values (internal function)
 */
static Temporal *
tnumber_restrict_ranges_internal(const Temporal *temp, RangeType **ranges,
  int count, bool atfunc)
{
  /* Bounding box test */
  int newcount;
  RangeType **newranges = tnumber_bbox_restrict_ranges(temp, ranges,
    count, &newcount);
  if (newcount == 0)
  {
    if (atfunc)
      return NULL;
    else
      return (temp->temptype != SEQUENCE) ? temporal_copy(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tnumberinst_restrict_ranges((TInstant *) temp,
      newranges, newcount, atfunc);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tnumberinstset_restrict_ranges((TInstantSet *) temp,
      newranges, newcount, atfunc);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tnumberseq_restrict_ranges((TSequence *) temp,
        newranges, newcount, atfunc, BBOX_TEST_NO);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tnumberseqset_restrict_ranges((TSequenceSet *) temp,
      newranges, newcount, atfunc);

  pfree_array((void **) newranges, newcount);

  return result;
}

/**
 * Restricts the temporal value to the (complement of the) array of ranges
 * of base values
 */
Datum
tnumber_restrict_ranges(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
  /* Return NULL or a copy of the temporal value on empty array */
  int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  if (count == 0)
  {
    PG_FREE_IF_COPY(array, 1);
    if (atfunc)
    {
      PG_FREE_IF_COPY(temp, 0);
      PG_RETURN_NULL();
    }
    else
    {
      Temporal *result = temporal_copy(temp);
      PG_FREE_IF_COPY(temp, 0);
      PG_RETURN_POINTER(result);
    }
  }
  RangeType **ranges = rangearr_extract(array, &count);
  Temporal *result = (count > 1) ?
    tnumber_restrict_ranges_internal(temp, ranges, count, atfunc) :
    tnumber_restrict_range_internal(temp, ranges[0], atfunc);
  pfree(ranges);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(array, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tnumber_at_ranges);
/**
 * Restricts the temporal value to the array of ranges of base values
 */
PGDLLEXPORT Datum
tnumber_at_ranges(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_ranges(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(tnumber_minus_ranges);
/**
 * Restricts the temporal value to the complement of the array of ranges
 * of base values
 */
PGDLLEXPORT Datum
tnumber_minus_ranges(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_ranges(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * Restricts the temporal value to (the complement of) the minimum base value
 * (dispatch function)
 */
Temporal *
temporal_restrict_minmax_internal(const Temporal *temp, bool min, bool atfunc)
{
  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = atfunc ? (Temporal *) tinstant_copy((TInstant *) temp) : NULL;
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_minmax((TInstantSet *) temp,
      min, atfunc);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tsequence_restrict_minmax((TSequence *) temp,
      min, atfunc);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_minmax((TSequenceSet *) temp,
      min, atfunc);
  return result;
}

/**
 * Restricts the temporal value to the minimum base value
 */
Datum
temporal_restrict_minmax(FunctionCallInfo fcinfo, bool min, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result = temporal_restrict_minmax_internal(temp, min, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_at_min);
/**
 * Restricts the temporal value to the minimum base value
 */
PGDLLEXPORT Datum
temporal_at_min(PG_FUNCTION_ARGS)
{
  return temporal_restrict_minmax(fcinfo, MIN, REST_AT);
}

PG_FUNCTION_INFO_V1(temporal_minus_min);
/**
 * Restricts the temporal value to the complement of the minimum base value
 */
PGDLLEXPORT Datum
temporal_minus_min(PG_FUNCTION_ARGS)
{
  return temporal_restrict_minmax(fcinfo, MIN, REST_MINUS);
}

PG_FUNCTION_INFO_V1(temporal_at_max);
/**
 * Restricts the temporal value to the maximum base value
 */
PGDLLEXPORT Datum
temporal_at_max(PG_FUNCTION_ARGS)
{
  return temporal_restrict_minmax(fcinfo, MAX, REST_AT);
}

PG_FUNCTION_INFO_V1(temporal_minus_max);
/**
 * Restricts the temporal value to the complement of the maximum base value
 */
PGDLLEXPORT Datum
temporal_minus_max(PG_FUNCTION_ARGS)
{
  return temporal_restrict_minmax(fcinfo, MAX, REST_MINUS);
}

/*****************************************************************************/

/**
 * Restricts the temporal value to the timestamp
 * (dispatch function)
 */
Temporal *
temporal_restrict_timestamp_internal(const Temporal *temp, TimestampTz t,
  bool atfunc)
{
  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tinstant_restrict_timestamp((TInstant *) temp, t, atfunc);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_timestamp((TInstantSet *) temp,
    t, atfunc);
  else if (temp->temptype == SEQUENCE)
    result = atfunc ?
      (Temporal *) tsequence_at_timestamp((TSequence *) temp, t) :
      (Temporal *) tsequence_minus_timestamp((TSequence *) temp, t);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_timestamp((TSequenceSet *) temp,
      t, atfunc);
  return result;
}

/**
 * Restricts the temporal value to the (complement of the) timestamp
 */
Datum
temporal_restrict_timestamp(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Temporal *result = temporal_restrict_timestamp_internal(temp, t, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_at_timestamp);
/**
 * Restricts the temporal value to the timestamp
 */
PGDLLEXPORT Datum
temporal_at_timestamp(PG_FUNCTION_ARGS)
{
  return temporal_restrict_timestamp(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(temporal_minus_timestamp);
/**
 * Restricts the temporal value to the complement of the timestamp
 */
PGDLLEXPORT Datum
temporal_minus_timestamp(PG_FUNCTION_ARGS)
{
  return temporal_restrict_timestamp(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * Returns the base value of the temporal value at the timestamp when the
 * timestamp may be at an exclusive bound
 */
bool
temporal_value_at_timestamp_inc(const Temporal *temp, TimestampTz t, Datum *value)
{
  bool result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_value_at_timestamp((TInstant *) temp, t, value);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_value_at_timestamp((TInstantSet *) temp, t, value);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_value_at_timestamp_inc((TSequence *) temp, t, value);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_value_at_timestamp_inc((TSequenceSet *) temp, t, value);
  return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_value_at_timestamp);
/**
 * Returns the base value of the temporal value at the timestamp
 */
PGDLLEXPORT Datum
temporal_value_at_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool found = false;
  Datum result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    found = tinstant_value_at_timestamp((TInstant *) temp, t, &result);
  else if (temp->temptype == INSTANTSET)
    found = tinstantset_value_at_timestamp((TInstantSet *) temp, t, &result);
  else if (temp->temptype == SEQUENCE)
    found = tsequence_value_at_timestamp((TSequence *) temp, t, &result);
  else /* temp->temptype == SEQUENCESET */
    found = tsequenceset_value_at_timestamp((TSequenceSet *) temp, t, &result);
  PG_FREE_IF_COPY(temp, 0);
  if (!found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/

/**
 * Restricts the temporal value to the (complement of the) timestamp set
 */
Datum
temporal_restrict_timestampset(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tinstant_restrict_timestampset(
      (TInstant *) temp, ts, atfunc);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_timestampset(
      (TInstantSet *) temp, ts, atfunc);
  else if (temp->temptype == SEQUENCE)
    result = atfunc ?
      (Temporal *) tsequence_at_timestampset((TSequence *) temp, ts) :
      (Temporal *) tsequence_minus_timestampset((TSequence *) temp, ts);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_timestampset(
      (TSequenceSet *) temp, ts, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}
PG_FUNCTION_INFO_V1(temporal_at_timestampset);
/**
 * Restricts the temporal value to the timestamp set
 */
PGDLLEXPORT Datum
temporal_at_timestampset(PG_FUNCTION_ARGS)
{
  return temporal_restrict_timestampset(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(temporal_minus_timestampset);
/**
 * Restricts the temporal value to the complement of the timestamp set
 */
PGDLLEXPORT Datum
temporal_minus_timestampset(PG_FUNCTION_ARGS)
{
  return temporal_restrict_timestampset(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * Restricts the temporal value to the (complement of the) period
 * (dispatch function)
 */
Temporal *
temporal_restrict_period_internal(const Temporal *temp, const Period *p,
  bool atfunc)
{
  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tinstant_restrict_period(
      (TInstant *) temp, p, atfunc);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_period(
      (TInstantSet *) temp, p, atfunc);
  else if (temp->temptype == SEQUENCE)
    result = atfunc ?
      (Temporal *) tsequence_at_period((TSequence *) temp, p) :
      (Temporal *) tsequence_minus_period((TSequence *) temp, p);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_period(
      (TSequenceSet *) temp, p, atfunc);
  return result;
}

Temporal *
temporal_at_period_internal(const Temporal *temp, const Period *p)
{
  return temporal_restrict_period_internal(temp, p, REST_AT);
}

Datum
temporal_restrict_period(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Period *p = PG_GETARG_PERIOD(1);
  Temporal *result = temporal_restrict_period_internal(temp, p, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_at_period);
/**
 * Restricts the temporal value to the period
 */
PGDLLEXPORT Datum
temporal_at_period(PG_FUNCTION_ARGS)
{
  return temporal_restrict_period(fcinfo, REST_AT);
}


PG_FUNCTION_INFO_V1(temporal_minus_period);
/**
 * Restricts the temporal value to the complement of the period
 */
PGDLLEXPORT Datum
temporal_minus_period(PG_FUNCTION_ARGS)
{
  return temporal_restrict_period(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * Restricts the temporal value to the (complement of the) period set
 * (dispatch function)
 */
Temporal *
temporal_restrict_periodset_internal(const Temporal *temp,
  const PeriodSet *ps, bool atfunc)
{
  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tinstant_restrict_periodset(
      (TInstant *) temp, ps, atfunc);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_periodset(
      (TInstantSet *) temp, ps, atfunc);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tsequence_restrict_periodset(
      (TSequence *) temp, ps, atfunc);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_periodset(
      (TSequenceSet *) temp, ps, atfunc);
  return result;
}

/**
 * Restricts the temporal value to the (complement of the) period set
  */
Datum
temporal_restrict_periodset(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  Temporal *result = temporal_restrict_periodset_internal(temp, ps, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_at_periodset);
/**
 * Restricts the temporal value to the period set
 */
PGDLLEXPORT Datum
temporal_at_periodset(PG_FUNCTION_ARGS)
{
  return temporal_restrict_periodset(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(temporal_minus_periodset);
/**
 * Restricts the temporal value to the complement of the period set
 */
PGDLLEXPORT Datum
temporal_minus_periodset(PG_FUNCTION_ARGS)
{
  return temporal_restrict_periodset(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * Restrict the temporal number to the temporal box (internal function)
 */
Temporal *
tnumber_at_tbox_internal(const Temporal *temp, const TBOX *box)
{
  /* Bounding box test */
  TBOX box1;
  memset(&box1, 0, sizeof(TBOX));
  temporal_bbox(&box1, temp);
  if (!overlaps_tbox_tbox_internal(box, &box1))
    return NULL;

  /* At least one of MOBDB_FLAGS_GET_T and MOBDB_FLAGS_GET_X is true */
  Temporal *temp1;
  if (MOBDB_FLAGS_GET_T(box->flags))
  {
    Period p;
    period_set(&p, box->tmin, box->tmax, true, true);
    /* Due to the bounding box test above temp is never NULL */
    temp1 = temporal_at_period_internal(temp, &p);
  }
  else
    temp1 = temporal_copy(temp);

  Temporal *result;
  if (MOBDB_FLAGS_GET_X(box->flags))
  {
    /* Ensure function is called for temporal numbers */
    ensure_tnumber_base_type(temp->valuetypid);
    /* The valuetypid of the temporal value determines wheter the
     * argument box is converted into an intrange or a floatrange */
    RangeType *range;
    if (temp->valuetypid == INT4OID)
      range = range_make(Int32GetDatum((int) box->xmin),
        Int32GetDatum((int) box->xmax), true, true, INT4OID);
    else /* temp->valuetypid == FLOAT8OID */
      range = range_make(Float8GetDatum(box->xmin),
        Float8GetDatum(box->xmax), true, true, FLOAT8OID);
    result = tnumber_restrict_range_internal(temp1, range, true);
    pfree(DatumGetPointer(range));
    pfree(temp1);
  }
  else
    result = temp1;
  return result;
}

/**
 * Restrict the temporal number to the complement of the temporal box
 * (internal function).
 * We cannot make the difference from each dimension separately, i.e.,
 * restrict at the period and then restrict to the range. Therefore, we
 * compute the atTbox and then compute the complement of the value obtained.
 *
 */
Temporal *
tnumber_minus_tbox_internal(const Temporal *temp, const TBOX *box)
{
  /* Bounding box test */
  TBOX box1;
  memset(&box1, 0, sizeof(TBOX));
  temporal_bbox(&box1, temp);
  if (!overlaps_tbox_tbox_internal(box, &box1))
    return temporal_copy(temp);

  Temporal *result = NULL;
  Temporal *temp1 = tnumber_at_tbox_internal(temp, box);
  if (temp1 != NULL)
  {
    PeriodSet *ps1 = temporal_get_time_internal(temp);
    PeriodSet *ps2 = temporal_get_time_internal(temp1);
    PeriodSet *ps = minus_periodset_periodset_internal(ps1, ps2);
    if (ps != NULL)
    {
      result = temporal_restrict_periodset_internal(temp, ps, true);
      pfree(ps);
    }
    pfree(temp1); pfree(ps1); pfree(ps2);
  }
  return result;
}

/**
 * Restricts the temporal value to the (complement of the) temporal box
  */
Datum
tnumber_restrict_tbox(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TBOX *box = PG_GETARG_TBOX_P(1);
  Temporal *result = atfunc ? tnumber_at_tbox_internal(temp, box) :
    tnumber_minus_tbox_internal(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tnumber_at_tbox);
/**
 * Restricts the temporal value to the temporal box
 */
PGDLLEXPORT Datum
tnumber_at_tbox(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_tbox(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(tnumber_minus_tbox);
/**
 * Restricts the temporal value to the complement of the temporal box
 */
PGDLLEXPORT Datum
tnumber_minus_tbox(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_tbox(fcinfo, REST_MINUS);
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_intersects_timestamp);
/**
 * Returns true if the temporal value intersects the timestamp
 */
PGDLLEXPORT Datum
temporal_intersects_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_intersects_timestamp((TInstant *) temp, t);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_intersects_timestamp((TInstantSet *) temp, t);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_intersects_timestamp((TSequence *) temp, t);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_intersects_timestamp((TSequenceSet *) temp, t);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(temporal_intersects_timestampset);
/**
 * Returns true if the temporal value intersects the timestamp set
 */
PGDLLEXPORT Datum
temporal_intersects_timestampset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_intersects_timestampset((TInstant *) temp, ts);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_intersects_timestampset((TInstantSet *) temp, ts);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_intersects_timestampset((TSequence *) temp, ts);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_intersects_timestampset((TSequenceSet *) temp, ts);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(temporal_intersects_period);
/**
 * Returns true if the temporal value intersects the period
 */
PGDLLEXPORT Datum
temporal_intersects_period(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_intersects_period((TInstant *) temp, p);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_intersects_period((TInstantSet *) temp, p);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_intersects_period((TSequence *) temp, p);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_intersects_period((TSequenceSet *) temp, p);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(temporal_intersects_periodset);
/**
 * Returns true if the temporal value intersects the period set
 */
PGDLLEXPORT Datum
temporal_intersects_periodset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_intersects_periodset((TInstant *) temp, ps);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_intersects_periodset((TInstantSet *) temp, ps);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_intersects_periodset((TSequence *) temp, ps);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_intersects_periodset((TSequenceSet *) temp, ps);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnumber_integral);
/**
 * Returns the integral (area under the curve) of the temporal
 * number value
 */
PGDLLEXPORT Datum
tnumber_integral(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  double result = 0.0;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT || temp->temptype == INSTANTSET)
    ;
  else if (temp->temptype == SEQUENCE)
    result = tnumberseq_integral((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tnumberseqset_integral((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(tnumber_twavg);
/**
 * Returns the time-weighted average of the temporal number
 */
PGDLLEXPORT Datum
tnumber_twavg(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  double result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = datum_double(tinstant_value((TInstant *) temp),
      temp->valuetypid);
  else if (temp->temptype == INSTANTSET)
    result = tnumberinstset_twavg((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = tnumberseq_twavg((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tnumberseqset_twavg((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * Returns -1, 0, or 1 depending on whether the first temporal value
 * is less than, equal, or greater than the second one
 * (internal function)
 *
 * @note Function used for B-tree comparison
 */
static int
temporal_cmp_internal(const Temporal *temp1, const Temporal *temp2)
{
  assert(temp1->valuetypid == temp2->valuetypid);

  /* Compare bounding period
   * We need to compare periods AND bounding boxes since the bounding boxes
   * do not distinguish between inclusive and exclusive bounds */
  Period p1, p2;
  temporal_period(&p1, temp1);
  temporal_period(&p2, temp2);
  int result = period_cmp_internal(&p1, &p2);
  if (result)
    return result;

  /* Compare bounding box */
  bboxunion box1, box2;
  memset(&box1, 0, sizeof(bboxunion));
  memset(&box2, 0, sizeof(bboxunion));
  temporal_bbox(&box1, temp1);
  temporal_bbox(&box2, temp2);
  result = temporal_bbox_cmp(&box1, &box2, temp1->valuetypid);
  if (result)
    return result;

  /* If both are of the same temporal type use the specific comparison */
  if (temp1->temptype == temp2->temptype)
  {
    ensure_valid_temptype(temp1->temptype);
    if (temp1->temptype == INSTANT)
      return tinstant_cmp((TInstant *) temp1, (TInstant *) temp2);
    else if (temp1->temptype == INSTANTSET)
      return tinstantset_cmp((TInstantSet *) temp1, (TInstantSet *) temp2);
    else if (temp1->temptype == SEQUENCE)
      return tsequence_cmp((TSequence *) temp1, (TSequence *) temp2);
    else /* temp1->temptype == SEQUENCESET */
      return tsequenceset_cmp((TSequenceSet *) temp1, (TSequenceSet *) temp2);
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

  /* Compare flags */
  if (temp1->flags < temp2->flags)
    return -1;
  if (temp1->flags > temp2->flags)
    return 1;

  /* Finally compare temporal type */
  if (temp1->temptype < temp2->temptype)
    return -1;
  else if (temp1->temptype > temp2->temptype)
    return 1;
  else
    return 0;
}

PG_FUNCTION_INFO_V1(temporal_cmp);
/**
 * Returns -1, 0, or 1 depending on whether the first temporal value
 * is less than, equal, or greater than the second temporal value
 *
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
 * Returns true if the two temporal values are equal
 * (internal function)
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
static bool
temporal_eq_internal(const Temporal *temp1, const Temporal *temp2)
{
  assert(temp1->valuetypid == temp2->valuetypid);
  ensure_valid_temptype(temp1->temptype);
  ensure_valid_temptype(temp2->temptype);

  const TInstant *inst1, *inst2;
  const TInstantSet *ti;
  const TSequence *seq;
  const TSequenceSet *ts;
  /* If both are of the same temporal type use the specific equality */
  if (temp1->temptype == temp2->temptype)
  {
    if (temp1->temptype == INSTANT)
      return tinstant_eq((TInstant *) temp1, (TInstant *) temp2);
    else if (temp1->temptype == INSTANTSET)
      return tinstantset_eq((TInstantSet *) temp1, (TInstantSet *) temp2);
    else if (temp1->temptype == SEQUENCE)
      return tsequence_eq((TSequence *) temp1, (TSequence *) temp2);
    else /* temp1->temptype == SEQUENCESET */
      return tsequenceset_eq((TSequenceSet *) temp1, (TSequenceSet *) temp2);
  }

  /* Different temporal type */
  if (temp1->temptype > temp2->temptype)
  {
    const Temporal *temp = (Temporal *) temp1;
    temp1 = temp2;
    temp2 = temp;
  }
  if (temp1->temptype == INSTANT)
  {
    const TInstant *inst = (TInstant *) temp1;
    if (temp2->temptype == INSTANTSET)
    {
      ti = (TInstantSet *) temp2;
      if (ti->count != 1)
        return false;
      inst1 = tinstantset_inst_n(ti, 0);
      return tinstant_eq(inst, inst1);
    }
    if (temp2->temptype == SEQUENCE)
    {
      seq = (TSequence *) temp2;
      if (seq->count != 1)
        return false;
      inst1 = tsequence_inst_n(seq, 0);
      return tinstant_eq(inst, inst1);
    }
    if (temp2->temptype == SEQUENCESET)
    {
      ts = (TSequenceSet *) temp2;
      if (ts->count != 1)
        return false;
      seq = tsequenceset_seq_n(ts, 0);
      if (seq->count != 1)
        return false;
      inst1 = tsequence_inst_n(seq, 0);
      return tinstant_eq(inst, inst1);
    }

  }
  else if (temp1->temptype == INSTANTSET)
  {
    ti = (TInstantSet *) temp1;
    if (temp2->temptype == SEQUENCE)
    {
      seq = (TSequence *) temp2;
      if (ti->count != 1 || seq->count != 1)
        return false;
      inst1 = tinstantset_inst_n(ti, 0);
      inst2 = tsequence_inst_n(seq, 0);
      return tinstant_eq(inst1, inst2);
    }
    if (temp2->temptype == SEQUENCESET)
    {
      ts = (TSequenceSet *) temp2;
      for (int i = 0; i < ti->count; i ++)
      {
        seq = tsequenceset_seq_n(ts, i);
        if (seq->count != 1)
          return false;
        inst1 = tinstantset_inst_n(ti, i);
        inst2 = tsequence_inst_n(seq, 0);
        if (!tinstant_eq(inst1, inst2))
          return false;
      }
      return true;
    }
  }
  /* temp1->temptype == SEQUENCE && temp2->temptype == SEQUENCESET */
  seq = (TSequence *) temp1;
  ts = (TSequenceSet *) temp2;
  if (ts->count != 1)
    return false;
  const TSequence *seq1 = tsequenceset_seq_n(ts, 0);
  return tsequence_eq(seq, seq1);
}

PG_FUNCTION_INFO_V1(temporal_eq);
/**
 * Returns true if the two temporal values are equal
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
 * Returns true if the two temporal values are different
 * (internal function)
 */
bool
temporal_ne_internal(Temporal *temp1, Temporal *temp2)
{
  return !temporal_eq_internal(temp1, temp2);
}

PG_FUNCTION_INFO_V1(temporal_ne);
/**
 * Returns true if the two temporal values are different
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
 * Returns true if the first temporal value is less than the second one
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
 * Returns true if the first temporal value is less than or equal to
 * the second one
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
 * Returns true if the first temporal value is greater than or equal to
 * the second one
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
 * Returns true if the first temporal value is greater than the second one
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
 * Returns the hash value of the temporal value
 * (dispatch function)
 */
uint32
temporal_hash_internal(const Temporal *temp)
{
  uint32 result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_hash((TInstant *) temp);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_hash((TInstantSet *) temp);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_hash((TSequence *) temp);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_hash((TSequenceSet *) temp);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_hash);
/**
 * Returns the hash value of the temporal value
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
