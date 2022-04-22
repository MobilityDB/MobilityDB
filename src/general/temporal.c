/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Basic functions for temporal types of any subtype.
 */

#include "general/temporal.h"

/* PostgreSQL */
#include <assert.h>
#include <access/heapam.h>
#include <access/htup_details.h>
#if POSTGRESQL_VERSION_NUMBER < 130000
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
/* MobilityDB */
#include "general/doxygen_libmeos_api.h"
#include "general/period.h"
#include "general/time_ops.h"
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "general/temporal_boxops.h"
#include "general/temporal_parser.h"
#include "general/rangetypes_ext.h"
#include "general/tnumber_distance.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * Ensures that the temporal type is valid
 *
 * @note Used for the dispatch functions
 */
void
ensure_valid_tempsubtype(int16 subtype)
{
  if (subtype != INSTANT && subtype != INSTANTSET &&
    subtype != SEQUENCE && subtype != SEQUENCESET)
    elog(ERROR, "unknown subtype for temporal type: %d", subtype);
  return;
}

/**
 * Ensures that the temporal type is valid
 *
 * @note Used for the the analyze and selectivity functions
 */
void
ensure_valid_tempsubtype_all(int16 subtype)
{
  if (subtype != ANYTEMPSUBTYPE &&
    subtype != INSTANT && subtype != INSTANTSET &&
    subtype != SEQUENCE && subtype != SEQUENCESET)
    elog(ERROR, "unknown subtype for temporal type: %d", subtype);
  return;
}

/**
 * Ensures that the temporal type is a sequence (set)
 */
void
ensure_seq_subtypes(int16 subtype)
{
  if (subtype != SEQUENCE && subtype != SEQUENCESET)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Input must be a temporal sequence (set)")));
  return;
}

/**
 * Ensures that the elements of the array are of instant subtype
 */
void
ensure_tinstarr(const TInstant **instants, int count)
{
  for (int i = 0; i < count; i++)
  {
    if (instants[i]->subtype != INSTANT)
    {
      pfree(instants);
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Input values must be temporal instants")));
    }
  }
  return;
}

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

/**
 * Ensures that the two temporal values have the same base type
 */
void
ensure_same_temptype(const Temporal *temp1, const Temporal *temp2)
{
  if (temp1->temptype != temp2->temptype)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("The temporal values must be of the same temporal type")));
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
      errmsg("The temporal values must have the same interpolation")));
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
    ! datum_eq(tinstant_value(inst1), tinstant_value(inst2),
        temptype_basetype(inst1->temptype)))
  {
    char *t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("The temporal values have different value at their overlapping instant %s", t1)));
  }
  return;
}

/**
 * Ensures that two temporal instant values have increasing
 * timestamp (or may be equal if the merge parameter is true), and if they
 * are temporal points, have the same srid and the same dimensionality.
 *
 * @param[in] inst1, inst2 Temporal instants
 * @param[in] merge True if a merge operation, which implies that the two
 *   consecutive instants may be equal
 * @param[in] subtype Subtype for which the function is called
 */
void
ensure_valid_tinstarr1(const TInstant *inst1, const TInstant *inst2,
  bool merge, int16 subtype)
{
  ensure_same_interpolation((Temporal *) inst1, (Temporal *) inst2);
  ensure_increasing_timestamps(inst1, inst2, merge);
  ensure_spatial_validity((Temporal *) inst1, (Temporal *) inst2);
  if (subtype == SEQUENCE && inst1->temptype == T_TNPOINT)
    ensure_same_rid_tnpointinst(inst1, inst2);
  return;
}

/**
 * Ensures that all temporal instant values of the array have increasing
 * timestamp (or may be equal if the merge parameter is true), and if they
 * are temporal points, have the same srid and the same dimensionality.
 *
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of elements in the input array
 * @param[in] merge True if a merge operation, which implies that the two
 *   consecutive instants may be equal
 * @param[in] subtype Subtype for which the function is called
 */
void
ensure_valid_tinstarr(const TInstant **instants, int count, bool merge,
  int16 subtype)
{
  for (int i = 1; i < count; i++)
    ensure_valid_tinstarr1(instants[i - 1], instants[i], merge, subtype);
  return;
}

/**
 * Ensures that all temporal instant values of the array have increasing
 * timestamp (or may be equal if the merge parameter is true), and if they
 * are temporal points, have the same srid and the same dimensionality.
 * This function extends function ensure_valid_tinstarr by determining
 * the splits that must be made according the maximum distance or interval
 * between consecutive instants.
 *
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of elements in the input array
 * @param[in] merge True if a merge operation, which implies that the two
 *   consecutive instants may be equal
 * @param[in] subtype Subtype for which the function is called
 * @param[in] maxdist Maximum distance to split the temporal sequence
 * @param[in] maxt Maximum time interval to split the temporal sequence
 * @param[out] countsplits Number of splits
 * @result Array of indices at which the temporal sequence is split
 */
int *
ensure_valid_tinstarr_gaps(const TInstant **instants, int count, bool merge,
  int16 subtype, double maxdist, Interval *maxt, int *countsplits)
{
  CachedType basetype = temptype_basetype(instants[0]->temptype);
  int *result = palloc(sizeof(int) * count);
  Datum value1 = tinstant_value(instants[0]);
  Datum geom1 = 0; /* Used only for temporal network points */
  datum_func2 point_distance = NULL;
  if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY)
    point_distance = pt_distance_fn(instants[0]->flags);
  else if (basetype == T_NPOINT)
    geom1 = npoint_geom(DatumGetNpointP(value1));
  int k = 0;
  for (int i = 1; i < count; i++)
  {
    ensure_valid_tinstarr1(instants[i - 1], instants[i], merge, subtype);
    bool split = false;
    Datum value2 = tinstant_value(instants[i]);
    Datum geom2 = 0; /* Used only for temporal network points */
    if (maxdist > 0 && ! datum_eq(value1, value2, basetype))
    {
      double dist = -1;
      if (tnumber_basetype(basetype))
        dist = (basetype == T_INT4) ?
          (double) DatumGetInt32(number_distance(value1, value2, basetype, basetype)) :
          DatumGetFloat8(number_distance(value1, value2, basetype, basetype));
      else if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY)
        dist = DatumGetFloat8(point_distance(value1, value2));
      else if (basetype == T_NPOINT)
      {
        geom2 = npoint_geom(DatumGetNpointP(value2));
        dist = DatumGetFloat8(pt_distance2d(geom1, geom2));
      }
      if (dist > maxdist)
        split = true;
    }
    /* If there is not already a split by distance */
    if (maxt != NULL && ! split)
    {
      Datum duration = DirectFunctionCall2(timestamp_mi,
        TimestampTzGetDatum(instants[i]->t), TimestampTzGetDatum(instants[i - 1]->t));
      int cmp = DatumGetInt32(DirectFunctionCall2(interval_cmp, duration,
        IntervalPGetDatum(maxt)));
      if (cmp > 0)
        split = true;
    }
    if (split)
      result[k++] = i;
    value1 = value2;
    geom1 = geom2;
  }
  *countsplits = k;
  return result;
}

/**
 * Ensures that all temporal instant values of the array have increasing
 * timestamp, and if they are temporal points, have the same srid and the
 * same dimensionality
 */
void
ensure_valid_tseqarr(const TSequence **sequences, int count)
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

/*****************************************************************************/

/**
 * Ensures that the number is positive
 */
void
ensure_positive_datum(Datum size, CachedType basetype)
{
  ensure_tnumber_basetype(basetype);
  if (basetype == T_INT4)
  {
    int isize = DatumGetInt32(size);
    if (isize <= 0)
      elog(ERROR, "The value must be positive: %d", isize);
  }
  else /* basetype == T_FLOAT8 */
  {
    double dsize = DatumGetFloat8(size);
    if (dsize <= 0.0)
      elog(ERROR, "The value must be positive: %f", dsize);
  }
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

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Return a pointer to the precomputed bounding box of the temporal value
 *
 * @return Return NULL for temporal instant values since they do not have
 * precomputed bounding box.
 */
void *
temporal_bbox_ptr(const Temporal *temp)
{
  void *result = NULL;
  /* Values of INSTANT subtype have not bounding box */
  if (temp->subtype == INSTANTSET)
    result = tinstantset_bbox_ptr((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_bbox_ptr((TSequence *) temp);
  else if (temp->subtype == SEQUENCESET)
    result = tsequenceset_bbox_ptr((TSequenceSet *) temp);
  return result;
}

/**
 * Set the second argument to the bounding box of the temporal value
 *
 * For temporal instant values the bounding box must be computed.
 * For the other subtypes a copy of the precomputed bounding box
 * is made.
 */
void
temporal_bbox(const Temporal *temp, void *box)
{
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    tinstant_make_bbox((TInstant *) temp, box);
  else if (temp->subtype == INSTANTSET)
    tinstantset_bbox((TInstantSet *) temp, box);
  else if (temp->subtype == SEQUENCE)
    tsequence_bbox((TSequence *) temp, box);
  else /* temp->subtype == SEQUENCESET */
    tsequenceset_bbox((TSequenceSet *) temp, box);
  return;
}

/**
 * Return a copy of the temporal value
 */
Temporal *
temporal_copy(const Temporal *temp)
{
  Temporal *result = (Temporal *) palloc0(VARSIZE(temp));
  memcpy(result, temp, VARSIZE(temp));
  return result;
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] temp1,temp2 Input values
 * @param[in] mode Either intersection or synchronization
 * @param[out] inter1,inter2 Output values
 * @result Return false if the values do not overlap on time
 */
bool
intersection_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  SyncMode mode, Temporal **inter1, Temporal **inter2)
{
  bool result = false;
  ensure_valid_tempsubtype(temp1->subtype);
  ensure_valid_tempsubtype(temp2->subtype);
  if (temp1->subtype == INSTANT)
  {
    if (temp2->subtype == INSTANT)
      result = intersection_tinstant_tinstant(
        (TInstant *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == INSTANTSET)
      result = intersection_tinstant_tinstantset(
        (TInstant *) temp1, (TInstantSet *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == SEQUENCE)
      result = intersection_tinstant_tsequence(
        (TInstant *) temp1, (TSequence *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else /* temp2->subtype == SEQUENCESET */
      result = intersection_tinstant_tsequenceset(
        (TInstant *) temp1, (TSequenceSet *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
  }
  else if (temp1->subtype == INSTANTSET)
  {
    if (temp2->subtype == INSTANT)
      result = intersection_tinstantset_tinstant(
        (TInstantSet *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == INSTANTSET)
      result = intersection_tinstantset_tinstantset(
        (TInstantSet *) temp1, (TInstantSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else if (temp2->subtype == SEQUENCE)
      result = intersection_tinstantset_tsequence(
        (TInstantSet *) temp1, (TSequence *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else /* temp2->subtype == SEQUENCESET */
      result = intersection_tinstantset_tsequenceset(
        (TInstantSet *) temp1, (TSequenceSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
  }
  else if (temp1->subtype == SEQUENCE)
  {
    if (temp2->subtype == INSTANT)
      result = intersection_tsequence_tinstant(
        (TSequence *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == INSTANTSET)
      result = intersection_tsequence_tinstantset(
        (TSequence *) temp1, (TInstantSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else if (temp2->subtype == SEQUENCE)
      result = synchronize_tsequence_tsequence(
          (TSequence *) temp1, (TSequence *) temp2,
          (TSequence **) inter1, (TSequence **) inter2,
            mode == SYNCHRONIZE_CROSS);
    else /* temp2->subtype == SEQUENCESET */
      result = intersection_tsequence_tsequenceset(
          (TSequence *) temp1, (TSequenceSet *) temp2, mode,
          (TSequenceSet **) inter1, (TSequenceSet **) inter2);
  }
  else /* temp1->subtype == SEQUENCESET */
  {
    if (temp2->subtype == INSTANT)
      result = intersection_tsequenceset_tinstant(
        (TSequenceSet *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == INSTANTSET)
      result = intersection_tsequenceset_tinstantset(
        (TSequenceSet *) temp1, (TInstantSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else if (temp2->subtype == SEQUENCE)
      result = synchronize_tsequenceset_tsequence(
          (TSequenceSet *) temp1, (TSequence *) temp2, mode,
          (TSequenceSet **) inter1, (TSequenceSet **) inter2);
    else /* temp2->subtype == SEQUENCESET */
      result = synchronize_tsequenceset_tsequenceset(
        (TSequenceSet *) temp1, (TSequenceSet *) temp2, mode,
        (TSequenceSet **) inter1, (TSequenceSet **) inter2);
  }
  return result;
}

/**
 * Return the n-th instant of the temporal instant set or
 * a temporal sequence value.
 */
const TInstant *
tinstarr_inst_n(const Temporal *temp, int n)
{
  assert(temp->subtype == INSTANTSET || temp->subtype == SEQUENCE);
  if (temp->subtype == INSTANTSET)
    return tinstantset_inst_n((TInstantSet *) temp, n);
  else /* temp->subtype == SEQUENCE */
    return tsequence_inst_n((TSequence *) temp, n);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the string representation of the temporal value.
 *
 * @param[in] temp Temporal value
 * @param[in] value_out Function called to output the base value
 * depending on its Oid
 */
char *
temporal_to_string(const Temporal *temp, char *(*value_out)(Oid, Datum))
{
  char *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_to_string((TInstant *) temp, value_out);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_to_string((TInstantSet *) temp, value_out);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_to_string((TSequence *) temp, false, value_out);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_to_string((TSequenceSet *) temp, value_out);
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Write the binary representation of the temporal value into the buffer.
 *
 * @param[in] temp Temporal value
 * @param[in] buf Buffer
 */
void
temporal_write(const Temporal *temp, StringInfo buf)
{
  ensure_valid_tempsubtype(temp->subtype);
  pq_sendbyte(buf, temp->subtype);
  if (temp->subtype == INSTANT)
    tinstant_write((TInstant *) temp, buf);
  else if (temp->subtype == INSTANTSET)
    tinstantset_write((TInstantSet *) temp, buf);
  else if (temp->subtype == SEQUENCE)
    tsequence_write((TSequence *) temp, buf);
  else /* temp->subtype == SEQUENCESET */
    tsequenceset_write((TSequenceSet *) temp, buf);
  return;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a new temporal value from its binary representation read from
 * the buffer.
 *
 * @param[in] buf Buffer
 * @param[in] temptype Temporal type
 */
Temporal *
temporal_read(StringInfo buf, CachedType temptype)
{
  uint8 subtype = pq_getmsgbyte(buf);
  Temporal *result;
  ensure_valid_tempsubtype(subtype);
  if (subtype == INSTANT)
    result = (Temporal *) tinstant_read(buf, temptype);
  else if (subtype == INSTANTSET)
    result = (Temporal *) tinstantset_read(buf, temptype);
  else if (subtype == SEQUENCE)
    result = (Temporal *) tsequence_read(buf, temptype);
  else /* subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_read(buf, temptype);
  return result;
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Transform a temporal value into a constant value with the same time
 * frame.
 */
Temporal *
temporal_from_base(const Temporal *temp, Datum value, CachedType temptype,
  bool linear)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    TInstant *inst = (TInstant *) temp;
    result = (Temporal *) tinstant_make(value, inst->t, temptype);
  }
  else if (temp->subtype == INSTANTSET)
  {
    TInstantSet *ti = (TInstantSet *) temp;
    TimestampTz *times = tinstantset_timestamps(ti);
    TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
    for (int i = 0; i < ti->count; i++)
      instants[i] = tinstant_make(value, times[i], temptype);
    result = (Temporal *) tinstantset_make_free(instants, ti->count, MERGE_NO);
    pfree(times);
  }
  else if (temp->subtype == SEQUENCE)
  {
    TSequence *seq = (TSequence *) temp;
    result = (Temporal *) tsequence_from_base(value, temptype,
      &seq->period, linear);
  }
  else /* temp->subtype == SEQUENCESET */
  {
    TSequenceSet *ts = (TSequenceSet *) temp;
    TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
    for (int i = 0; i < ts->count; i++)
    {
      const TSequence *seq = tsequenceset_seq_n(ts, i);
      sequences[i] = tsequence_from_base(value, temptype,
        &seq->period, linear);
    }
    result = (Temporal *) tsequenceset_make_free(sequences, ts->count,
      NORMALIZE_NO);
  }
  return result;
}

/*****************************************************************************
 * Append and merge functions
 ****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Append an instant to the end of a temporal value.
 */
Temporal *
temporal_append_tinstant(const Temporal *temp, const Temporal *inst)
{
  /* Validity tests */
  if (inst->subtype != INSTANT)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("The second argument must be of instant subtype")));
  ensure_same_temptype(temp, (Temporal *) inst);
  /* The test to ensure the increasing timestamps must be done in the
   * specific function since the inclusive/exclusive bounds must be
   * taken into account for temporal sequences and sequence sets */
  ensure_spatial_validity(temp, inst);

  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinstant_merge((TInstant *) temp, (TInstant *) inst);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_append_tinstant((TInstantSet *) temp,
      (TInstant *) inst);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tsequence_append_tinstant((TSequence *) temp,
      (TInstant *) inst);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_append_tinstant((TSequenceSet *) temp,
      (TInstant *) inst);
  return result;
}

/**
 * Convert two temporal values into a common subtype
 *
 * @param[in] temp1,temp2 Input values
 * @param[out] out1,out2 Output values
 */
static void
temporal_convert_same_subtype(const Temporal *temp1, const Temporal *temp2,
  Temporal **out1, Temporal **out2)
{
  assert(temp1->temptype == temp2->temptype);
  ensure_valid_tempsubtype(temp1->subtype);
  ensure_valid_tempsubtype(temp2->subtype);

  /* If both are of the same subtype do nothing */
  if (temp1->subtype == temp2->subtype)
  {
    *out1 = temporal_copy(temp1);
    *out2 = temporal_copy(temp2);
    return;
  }

  /* Different subtype */
  bool swap = false;
  Temporal *new1, *new2;
  if (temp1->subtype > temp2->subtype)
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
  if (new1->subtype == INSTANT)
  {
    if (new2->subtype == INSTANTSET)
      new = (Temporal *) tinstant_tinstantset((TInstant *) new1);
    else if (new2->subtype == SEQUENCE)
      new = (Temporal *) tinstant_tsequence((TInstant *) new1,
        MOBDB_FLAGS_GET_CONTINUOUS(new1->flags));
    else /* new2->subtype == SEQUENCESET */
      new = (Temporal *) tinstant_tsequenceset((TInstant *) new1,
        MOBDB_FLAGS_GET_CONTINUOUS(new1->flags));
  }
  else if (new1->subtype == INSTANTSET)
  {
    if (new2->subtype == SEQUENCE)
    {
      if (((TInstantSet *) new1)->count == 1)
        new = (Temporal *) tinstantset_tsequence((TInstantSet *) new1,
          MOBDB_FLAGS_GET_CONTINUOUS(new1->flags));
      else /* new2->subtype == SEQUENCESET */
      {
        new = (Temporal *) tinstantset_tsequenceset((TInstantSet *) new1,
          MOBDB_FLAGS_GET_CONTINUOUS(new1->flags));
        newts = (Temporal *) tsequence_tsequenceset((TSequence *) new2);
      }
    }
    else /* new2->subtype == SEQUENCESET */
      new = (Temporal *) tinstantset_tsequenceset((TInstantSet *) new1,
        MOBDB_FLAGS_GET_CONTINUOUS(new1->flags));
  }
  else /* new1->subtype == SEQUENCE && new2->subtype == SEQUENCESET */
    new = (Temporal *) tsequence_tsequenceset((TSequence *) new1);
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

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge the two temporal values.
 *
 * @result Merged value. Return NULL if both arguments are NULL.
 * If one argument is null the other argument is output.
 */
Temporal *
temporal_merge(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *result;
  /* Can't do anything with null inputs */
  if (! temp1 && ! temp2)
    return NULL;
  /* One argument is null, return a copy of the other temporal */
  if (! temp1)
    return temporal_copy(temp2);
  if (! temp2)
    return temporal_copy(temp1);

  /* Both arguments are temporal */
  ensure_same_temptype(temp1, temp2);
  ensure_same_interpolation(temp1, temp2);

  /* Convert to the same subtype if possible */
  Temporal *new1, *new2;
  temporal_convert_same_subtype(temp1, temp2, &new1, &new2);

  ensure_valid_tempsubtype(new1->subtype);
  if (new1->subtype == INSTANT)
    result = tinstant_merge(
      (TInstant *) new1, (TInstant *)new2);
  else if (new1->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_merge(
      (TInstantSet *)new1, (TInstantSet *)new2);
  else if (new1->subtype == SEQUENCE)
    result = (Temporal *) tsequence_merge((TSequence *)new1,
      (TSequence *)new2);
  else /* new1->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_merge((TSequenceSet *)new1,
      (TSequenceSet *)new2);
  if (temp1 != new1)
    pfree(new1);
  if (temp2 != new2)
    pfree(new2);
  return result;
}

/**
 * Convert the array of temporal values into a common subtype
 *
 * @param[in] temparr Array of values
 * @param[in] count Number of values
 * @param[in] subtype common subtype
 * @result  Array of output values
 */
static Temporal **
temporalarr_convert_subtype(Temporal **temparr, int count, uint8 subtype)
{
  ensure_valid_tempsubtype(subtype);
  Temporal **result = palloc(sizeof(Temporal *) * count);
  for (int i = 0; i < count; i++)
  {
    uint8 subtype1 = temparr[i]->subtype;
    assert(subtype >= subtype1);
    if (subtype == subtype1)
      result[i] = temporal_copy(temparr[i]);
    else if (subtype1 == INSTANT)
    {
      if (subtype == INSTANTSET)
        result[i] = (Temporal *) tinstant_tinstantset((TInstant *) temparr[i]);
      else if (subtype == SEQUENCE)
        result[i] = (Temporal *) tinstant_tsequence((TInstant *) temparr[i],
          MOBDB_FLAGS_GET_LINEAR(temparr[i]->flags));
      else /* subtype == SEQUENCESET */
        result[i] = (Temporal *) tinstant_tsequenceset((TInstant *) temparr[i],
          MOBDB_FLAGS_GET_LINEAR(temparr[i]->flags));
    }
    else if (subtype1 == INSTANTSET)
    {
      /* An instant set can only be converted to a sequence set */
      assert(subtype == SEQUENCESET);
      result[i] = (Temporal *) tinstantset_tsequenceset((TInstantSet *) temparr[i],
        MOBDB_FLAGS_GET_LINEAR(temparr[i]->flags));
    }
    else /* subtype1 == SEQUENCE && subtype == SEQUENCESET */
      result[i] = (Temporal *) tsequence_tsequenceset((TSequence *) temparr[i]);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge the array of temporal values.
 */
Temporal *
temporal_merge_array(Temporal **temparr, int count)
{
  assert(count > 0);
  if (count == 1)
  {
    Temporal *result = temporal_copy(temparr[0]);
    return result;
  }

  /* Ensure all values have the same interpolation and determine
   * temporal subtype of the result */
  uint8 subtype, origsubtype;
  subtype = origsubtype = temparr[0]->subtype;
  bool interpolation = MOBDB_FLAGS_GET_LINEAR(temparr[0]->flags);
  for (int i = 1; i < count; i++)
  {
    if (MOBDB_FLAGS_GET_LINEAR(temparr[i]->flags) != interpolation)
    {
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Input values must be of the same interpolation")));
    }
    uint8 subtype1 = temparr[i]->subtype;
    if (subtype != subtype1)
    {
      /* A TInstantSet cannot be converted to a TSequence */
      int16 newsubtype = Max(subtype, subtype1);
      if (subtype == INSTANTSET && newsubtype == SEQUENCE)
        newsubtype = SEQUENCESET;
      subtype = newsubtype;
    }
  }
  /* Convert all temporal values to a single subtype if needed */
  Temporal **newtemps;
  if (subtype != origsubtype)
    newtemps = temporalarr_convert_subtype(temparr, count, subtype);
  else
    newtemps = temparr;

  Temporal *result;
  ensure_valid_tempsubtype(subtype);
  if (subtype == INSTANT)
    result = (Temporal *) tinstant_merge_array(
      (const TInstant **) newtemps, count);
  else if (subtype == INSTANTSET)
    result = tinstantset_merge_array(
      (const TInstantSet **) newtemps, count);
  else if (subtype == SEQUENCE)
    result = (Temporal *) tsequence_merge_array(
      (const TSequence **) newtemps, count);
  else /* subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_merge_array(
      (const TSequenceSet **) newtemps, count);
  if (subtype != origsubtype)
    pfree_array((void **) newtemps, count);
  return result;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal integer to an intrange.
 *
 * @note Note that the temporal subtype INSTANT does not have bounding box.
 */
RangeType *
tint_range(const Temporal *temp)
{
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    Datum value = tinstant_value((TInstant *) temp);
    return range_make(value, value, true, true, T_INT4);
  }

  TBOX *box;
  if (temp->subtype == INSTANTSET)
    box = tinstantset_bbox_ptr((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    box = tsequence_bbox_ptr((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    box = tsequenceset_bbox_ptr((TSequenceSet *) temp);
  Datum min = Int32GetDatum(((int) box->xmin));
  Datum max = Int32GetDatum(((int) box->xmax));
  return range_make(min, max, true, true, T_INT4);
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal integer to an intrange.
 *
 * @note Note that the temporal subtype INSTANT does not have bounding box.
 */
RangeType *
tfloat_range(const Temporal *temp)
{
  ensure_valid_tempsubtype(temp->subtype);
  RangeType *result;
  if (temp->subtype == INSTANT)
  {
    Datum value = tinstant_value((TInstant *) temp);
    result = range_make(value, value, true, true, T_FLOAT8);
  }
  else if (temp->subtype == INSTANTSET)
  {
    TBOX *box = tinstantset_bbox_ptr((TInstantSet *) temp);
    Datum min = Float8GetDatum(box->xmin);
    Datum max = Float8GetDatum(box->xmax);
    result = range_make(min, max, true, true, T_FLOAT8);
  }
  else if (temp->subtype == SEQUENCE)
    result = tfloatseq_range((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tfloatseqset_range((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast the temporal integer value as a temporal float value.
 */
Temporal *
tint_tfloat(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tintinst_tfloatinst((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tintinstset_tfloatinstset((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tintseq_tfloatseq((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tintseqset_tfloatseqset((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast the temporal float value as a temporal integer value.
 */
Temporal *
tfloat_tint(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tfloatinst_tintinst((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tfloatinstset_tintinstset((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tfloatseq_tintseq((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tfloatseqset_tintseqset((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Return the bounding period of the temporal value.
 */
void
temporal_period(const Temporal *temp, Period *p)
{
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    tinstant_period((TInstant *) temp, p);
  else if (temp->subtype == INSTANTSET)
    tinstantset_period((TInstantSet *) temp, p);
  else if (temp->subtype == SEQUENCE)
    tsequence_period((TSequence *) temp, p);
  else /* temp->subtype == SEQUENCESET */
    tsequenceset_period((TSequenceSet *) temp, p);
  return;
}

#ifdef MEOS
/**
 * @ingroup libmeos_box_cast
 * @brief Return the bounding box of the temporal number.
 */
TBOX *
tnumber_tbox(Temporal *temp)
{
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  temporal_bbox(temp, result);
  return result;
}
#endif

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal value into a temporal instant value.
 */
Temporal *
temporal_tinstant(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = temporal_copy(temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_tinstant((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tsequence_tinstant((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_tinstant((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal value into a temporal instant set value.
 */
Temporal *
temporal_tinstantset(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinstant_tinstantset((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = temporal_copy(temp);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tsequence_tinstantset((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_tinstantset((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal value into a temporal sequence value.
 */
Temporal *
temporal_tsequence(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinstant_tsequence((TInstant *) temp,
      MOBDB_FLAGS_GET_CONTINUOUS(temp->flags));
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_tsequence((TInstantSet *) temp,
      MOBDB_FLAGS_GET_CONTINUOUS(temp->flags));
  else if (temp->subtype == SEQUENCE)
    result = temporal_copy(temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_tsequence((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal value into a temporal sequence set value.
 */
Temporal *
temporal_tsequenceset(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinstant_tsequenceset((TInstant *) temp,
      MOBDB_FLAGS_GET_CONTINUOUS(temp->flags));
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_tsequenceset((TInstantSet *) temp,
      MOBDB_FLAGS_GET_CONTINUOUS(temp->flags));
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tsequence_tsequenceset((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = temporal_copy(temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal value with continuous base type from stepwise
 * to linear interpolation.
 */
Temporal *
tempstep_templinear(const Temporal *temp)
{
  ensure_seq_subtypes(temp->subtype);
  ensure_temptype_continuous(temp->temptype);

  if (MOBDB_FLAGS_GET_LINEAR(temp->flags))
    return temporal_copy(temp);

  Temporal *result;
  if (temp->subtype == SEQUENCE)
    result = (Temporal *) tstepseq_tlinearseq((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tstepseqset_tlinearseqset((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Shift and/or scale the time span of the temporal value by the two
 * intervals.
 *
 * @param[in] temp Temporal value
 * @param[in] shift True when a shift of the timespan must be performed
 * @param[in] tscale True when a scale of the timespan must be performed
 * @param[in] start Interval for shift
 * @param[in] duration Interval for scale
 * @pre The duration is greater than 0 if is not NULL
 */
Temporal *
temporal_shift_tscale(const Temporal *temp, bool shift, bool tscale,
  Interval *start, Interval *duration)
{
  assert((! shift || start != NULL) && (! tscale || duration != NULL));
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (start != NULL) ?
      (Temporal *) tinstant_shift((TInstant *) temp, start) :
      (Temporal *) tinstant_copy((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_shift_tscale((TInstantSet *) temp,
    start, duration);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tsequence_shift_tscale((TSequence *) temp,
      start, duration);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_shift_tscale((TSequenceSet *) temp,
      start, duration);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the string representation of the temporal type.
 */
char *
temporal_subtype(const Temporal *temp)
{
  char *result = palloc(sizeof(char) * 12);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    strcpy(result, "Instant");
  else if (temp->subtype == INSTANTSET)
    strcpy(result, "InstantSet");
  else if (temp->subtype == SEQUENCE)
    strcpy(result, "Sequence");
  else /* temp->subtype == SEQUENCESET */
    strcpy(result, "SequenceSet");
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the string representation of the temporal type.
 */
char *
temporal_interpolation(const Temporal *temp)
{
  char *result = palloc(sizeof(char) * 12);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT || temp->subtype == INSTANTSET)
    strcpy(result, "Discrete");
  else if (temp->subtype == SEQUENCE || temp->subtype == SEQUENCESET)
  {
    if (MOBDB_FLAGS_GET_LINEAR(temp->flags))
      strcpy(result, "Linear");
    else
      strcpy(result, "Stepwise");
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of base values of the temporal value.
 */
Datum *
temporal_values(const Temporal *temp, int *count)
{
  Datum *result;  /* make the compiler quiet */
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    result = tinstant_values((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_values((TInstantSet *) temp, count);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_values((TSequence *) temp, count);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_values((TSequenceSet *) temp, count);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the base values of the temporal float value as an array of
 * ranges.
 */
RangeType **
tfloat_ranges(const Temporal *temp, int *count)
{
  RangeType **result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    result = tfloatinst_ranges((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == INSTANTSET)
    result = tfloatinstset_ranges((TInstantSet *) temp, count);
  else if (temp->subtype == SEQUENCE)
    result = tfloatseq_ranges((TSequence *) temp, count);
  else /* temp->subtype == SEQUENCESET */
    result = tfloatseqset_ranges((TSequenceSet *) temp, count);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the time on which the temporal value is defined as a period set.
 */
PeriodSet *
temporal_time(const Temporal *temp)
{
  PeriodSet *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_time((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_time((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_time((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_time((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the value range of the temporal number value.
 */
RangeType *
tnumber_range(const Temporal *temp)
{
  RangeType *result = NULL;
  CachedType basetype = temptype_basetype(temp->temptype);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    Datum value = tinstant_value((TInstant *) temp);
    result = range_make(value, value, true, true, basetype);
  }
  else
  {
    TBOX *box = (TBOX *) temporal_bbox_ptr(temp);
    Datum min = 0, max = 0;
    ensure_tnumber_type(temp->temptype);
    if (temp->temptype == T_TINT)
    {
      min = Int32GetDatum((int)(box->xmin));
      max = Int32GetDatum((int)(box->xmax));
    }
    else /* temp->temptype == T_TFLOAT */
    {
      min = Float8GetDatum(box->xmin);
      max = Float8GetDatum(box->xmax);
    }
    result = range_make(min, max, true, true, basetype);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start base value of the temporal value
 */
Datum
temporal_start_value(Temporal *temp)
{
  Datum result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = tinstant_value_copy(tinstantset_inst_n((TInstantSet *) temp, 0));
  else if (temp->subtype == SEQUENCE)
    result = tinstant_value_copy(tsequence_inst_n((TSequence *) temp, 0));
  else /* temp->subtype == SEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp, 0);
    result = tinstant_value_copy(tsequence_inst_n(seq, 0));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end base value of the temporal value
 */
Datum
temporal_end_value(Temporal *temp)
{
  Datum result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = tinstant_value_copy(tinstantset_inst_n((TInstantSet *) temp,
      ((TInstantSet *) temp)->count - 1));
  else if (temp->subtype == SEQUENCE)
    result = tinstant_value_copy(tsequence_inst_n((TSequence *) temp,
      ((TSequence *) temp)->count - 1));
  else /* temp->subtype == SEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp,
      ((TSequenceSet *) temp)->count - 1);
    result = tinstant_value_copy(tsequence_inst_n(seq, seq->count - 1));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with the minimum base value of
 * the temporal value
 */
Datum
temporal_min_value(const Temporal *temp)
{
  Datum result;
  CachedType basetype = temptype_basetype(temp->temptype);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = datum_copy(tinstantset_min_value((TInstantSet *) temp), basetype);
  else if (temp->subtype == SEQUENCE)
    result = datum_copy(tsequence_min_value((TSequence *) temp), basetype);
  else /* temp->subtype == SEQUENCESET */
    result = datum_copy(tsequenceset_min_value((TSequenceSet *) temp), basetype);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum base value of the temporal value.
 */
Datum
temporal_max_value(const Temporal *temp)
{
  Datum result;
  CachedType basetype = temptype_basetype(temp->temptype);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = datum_copy(tinstantset_max_value((TInstantSet *) temp), basetype);
  else if (temp->subtype == SEQUENCE)
    result = datum_copy(tsequence_max_value((TSequence *) temp), basetype);
  else /* temp->subtype == SEQUENCESET */
    result = datum_copy(tsequenceset_max_value((TSequenceSet *) temp), basetype);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of the
 * temporal value.
 *
 * @note The function does not take into account whether the instant is at
 * an exclusive bound or not.
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance
 */
const TInstant *
temporal_min_instant(const Temporal *temp)
{
  const TInstant *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_min_instant((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_min_instant((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_min_instant((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with maximum base value of the
 * temporal value.
 */
const TInstant *
temporal_max_instant(const Temporal *temp)
{
  const TInstant *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_max_instant((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_min_instant((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_max_instant((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the timespan of the temporal value.
 */
Interval *
temporal_timespan(const Temporal *temp)
{
  Interval *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Interval *) palloc0(sizeof(Interval));
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_timespan((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_duration((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_timespan((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the duration of the temporal value.
 */
Interval *
temporal_duration(const Temporal *temp)
{
  Interval *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT || temp->subtype == INSTANTSET)
    result = (Interval *) palloc0(sizeof(Interval));
  else if (temp->subtype == SEQUENCE)
    result = tsequence_duration((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_duration((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the number of sequences of the temporal sequence (set) value.
 */
int
temporal_num_sequences(const Temporal *temp)
{
  ensure_seq_subtypes(temp->subtype);
  int result = 1;
  if (temp->subtype == SEQUENCESET)
    result = ((TSequenceSet *) temp)->count;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end sequence of the temporal sequence (set) value.
 */
TSequence *
temporal_start_sequence(const Temporal *temp)
{
  ensure_seq_subtypes(temp->subtype);
  TSequence *result;
  if (temp->subtype == SEQUENCE)
    result = tsequence_copy((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
  {
    const TSequenceSet *ts = (const TSequenceSet *) temp;
    result = tsequence_copy(tsequenceset_seq_n(ts, 0));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end sequence of the temporal sequence (set) value.
 */
TSequence *
temporal_end_sequence(const Temporal *temp)
{
  ensure_seq_subtypes(temp->subtype);
  TSequence *result;
  if (temp->subtype == SEQUENCE)
    result = tsequence_copy((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
  {
    const TSequenceSet *ts = (const TSequenceSet *) temp;
    result = tsequence_copy(tsequenceset_seq_n(ts, ts->count - 1));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th sequence of the temporal sequence (set) value.
 *
 * @note n is assumed to be 1-based.
 */
TSequence *
temporal_sequence_n(const Temporal *temp, int i)
{
  ensure_seq_subtypes(temp->subtype);
  TSequence *result = NULL;
  if (temp->subtype == SEQUENCE)
  {
    if (i == 1)
      result = tsequence_copy((TSequence *) temp);
  }
  else /* temp->subtype == SEQUENCESET */
  {
    const TSequenceSet *ts = (const TSequenceSet *) temp;
    if (i >= 1 && i <= ts->count)
      result = tsequence_copy(tsequenceset_seq_n(ts, i - 1));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of sequences of the temporal value.
 */
TSequence **
temporal_sequences(const Temporal *temp, int *count)
{
  TSequence **result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    result = tinstant_sequences((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == INSTANTSET)
  {
    result = tinstantset_sequences((TInstantSet *) temp);
    *count = ((TInstantSet *) temp)->count;
  }
  else if (temp->subtype == SEQUENCE)
  {
    result = tsequence_sequences((TSequence *) temp);
    *count = 1;
  }
  else /* temp->subtype == SEQUENCE */
  {
    result = tsequenceset_sequences((TSequenceSet *) temp);
    *count = ((TSequenceSet *) temp)->count;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of sequences of the temporal value.
 */
TSequence **
temporal_segments(const Temporal *temp, int *count)
{
  TSequence **result;
  if (temp->subtype == INSTANT)
  {
    result = tinstant_sequences((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == INSTANTSET)
  {
    result = tinstantset_sequences((TInstantSet *) temp);
    *count = ((TInstantSet *) temp)->count;
  }
  else if (temp->subtype == SEQUENCE)
  {
    result = tsequence_segments((TSequence *) temp, count);
  }
  else /* temp->subtype == SEQUENCESET */
  {
    result = tsequenceset_segments((TSequenceSet *) temp, count);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the number of distinct instants of the temporal value.
 */
int
temporal_num_instants(const Temporal *temp)
{
  int result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = 1;
  else if (temp->subtype == INSTANTSET)
    result = ((TInstantSet *) temp)->count;
  else if (temp->subtype == SEQUENCE)
    result = ((TSequence *) temp)->count;
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_num_instants((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start instant of the temporal value.
 */
const TInstant *
temporal_start_instant(const Temporal *temp)
{
  const TInstant *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_inst_n((TInstantSet *) temp, 0);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_inst_n((TSequence *) temp, 0);
  else /* temp->subtype == SEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp, 0);
    result = tsequence_inst_n(seq, 0);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end instant of the temporal value.
 *
 * @note This function is used for validity testing and thus returns a
 * pointer to the last instant.
 */
const TInstant *
temporal_end_instant(const Temporal *temp)
{
  const TInstant *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_inst_n((TInstantSet *) temp,
      ((TInstantSet *) temp)->count - 1);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_inst_n((TSequence *) temp,
      ((TSequence *) temp)->count - 1);
  else /* temp->subtype == SEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp,
      ((TSequenceSet *) temp)->count - 1);
    result = tsequence_inst_n(seq, seq->count - 1);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th instant of the temporal value.
 *
 * @note n is assumed 1-based
 */
const TInstant *
temporal_instant_n(Temporal *temp, int n)
{
  const TInstant *result = NULL;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    if (n == 1)
      result = (const TInstant *) temp;
  }
  else if (temp->subtype == INSTANTSET)
  {
    if (n >= 1 && n <= ((TInstantSet *) temp)->count)
      result = tinstantset_inst_n((TInstantSet *) temp, n - 1);
  }
  else if (temp->subtype == SEQUENCE)
  {
    if (n >= 1 && n <= ((TSequence *) temp)->count)
      result = tsequence_inst_n((TSequence *) temp, n - 1);
  }
  else /* temp->subtype == SEQUENCESET */
  {
    /* This test is necessary since the n-th DISTINCT instant is requested */
    if (n >= 1 && n <= ((TSequenceSet *) temp)->totalcount)
      result = tsequenceset_inst_n((TSequenceSet *) temp, n);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of instants of the temporal value.
 */
const TInstant **
temporal_instants(const Temporal *temp, int *count)
{
  const TInstant **result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    result = tinstant_instants((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == INSTANTSET)
  {
    result = tinstantset_instants((TInstantSet *) temp);
    *count = ((TInstantSet *) temp)->count;
  }
  else if (temp->subtype == SEQUENCE)
  {
    result = tsequence_instants((TSequence *) temp);
    *count = ((TSequence *) temp)->count;
  }
  else /* temp->subtype == SEQUENCESET */
  {
    result = tsequenceset_instants((TSequenceSet *) temp);
    *count = ((TSequenceSet *) temp)->totalcount;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the number of distinct timestamps of the temporal value.
 */
int
temporal_num_timestamps(const Temporal *temp)
{
  int result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = 1;
  else if (temp->subtype == INSTANTSET)
    result = ((TInstantSet *) temp)->count;
  else if (temp->subtype == SEQUENCE)
    result = ((TSequence *) temp)->count;
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_num_timestamps((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start timestamp of the temporal value.
 */
TimestampTz
temporal_start_timestamp(const Temporal *temp)
{
  TimestampTz result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = ((TInstant *) temp)->t;
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_inst_n((TInstantSet *) temp, 0)->t;
  else if (temp->subtype == SEQUENCE)
    result = tsequence_start_timestamp((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_start_timestamp((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end timestamp of the temporal value.
 */
TimestampTz
temporal_end_timestamp(Temporal *temp)
{
  TimestampTz result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = ((TInstant *) temp)->t;
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_inst_n((TInstantSet *) temp, ((TInstantSet *) temp)->count - 1)->t;
  else if (temp->subtype == SEQUENCE)
    result = tsequence_end_timestamp((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_end_timestamp((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th distinct timestamp of the temporal value.
 *
 * @note n is assume 1-based
 */
bool
temporal_timestamp_n(Temporal *temp, int n, TimestampTz *result)
{
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    if (n == 1)
    {
      *result = ((TInstant *) temp)->t;
      return true;
    }
  }
  else if (temp->subtype == INSTANTSET)
  {
    if (n >= 1 && n <= ((TInstantSet *) temp)->count)
    {
      *result = (tinstantset_inst_n((TInstantSet *) temp, n - 1))->t;
      return true;
    }
  }
  else if (temp->subtype == SEQUENCE)
  {
    if (n >= 1 && n <= ((TSequence *) temp)->count)
    {
      *result = (tsequence_inst_n((TSequence *) temp, n - 1))->t;
      return true;
    }
  }
  else /* temp->subtype == SEQUENCESET */
    return tsequenceset_timestamp_n((TSequenceSet *) temp, n, result);
  return false;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of distinct timestamps of the temporal value.
 */
TimestampTz *
temporal_timestamps(const Temporal *temp, int *count)
{
  TimestampTz *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
  {
    result = tinstant_timestamps((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == INSTANTSET)
  {
    result = tinstantset_timestamps((TInstantSet *) temp);
    *count = ((TInstantSet *) temp)->count;
  }
  else if (temp->subtype == SEQUENCE)
  {
    result = tsequence_timestamps((TSequence *) temp);
    *count = ((TSequence *) temp)->count;
  }
  else /* temp->subtype == SEQUENCESET */
  {
    result = tsequenceset_timestamps((TSequenceSet *) temp, count);
  }
  return result;
}

/*****************************************************************************
 * Ever/always functions
 * The functions assume that the temporal value and the datum value are of
 * the same basetype. Ever/always equal are valid for all temporal types
 * including temporal points. All the other comparisons are only valid for
 * temporal alphanumeric types.
 *****************************************************************************/

/**
 * Return true if the bounding box of the temporal value is ever/always equal
 * to the base value
 * @param[in] temp Temporal value
 * @param[in] value Value to be found
 * @param[in] ever True when testing ever, false when testing always
 */
bool
temporal_bbox_ev_al_eq(const Temporal *temp, Datum value, bool ever)
{
  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    TBOX box;
    temporal_bbox(temp, &box);
    double d = datum_double(value, temptype_basetype(temp->temptype));
    return (ever && box.xmin <= d && d <= box.xmax) ||
      (!ever && box.xmin == d && d == box.xmax);
  }
  else if (tspatial_type(temp->temptype))
  {
    STBOX box1, box2;
    temporal_bbox(temp, &box1);
    if (tgeo_type(temp->temptype))
      geo_stbox((GSERIALIZED *) DatumGetPointer(value), &box2);
    else if (temp->temptype == T_TNPOINT)
    {
      Datum geom = npoint_geom(DatumGetNpointP(value));
      geo_stbox((GSERIALIZED *) DatumGetPointer(geom), &box2);
      pfree(DatumGetPointer(geom));
    }
    return (ever && contains_stbox_stbox(&box1, &box2)) ||
      (!ever && same_stbox_stbox(&box1, &box2));
  }
  return true;
}

/**
 * Return true if the bounding box of the temporal value is ever/always less
 * than or equal to the base value. The same test is used for both since the
 * bounding box does not distinguish between the inclusive/exclusive bounds.
 *
 * @param[in] temp Temporal value
 * @param[in] value Base value
 * @param[in] ever True when testing ever, false when testing always
 */
bool
temporal_bbox_ev_al_lt_le(const Temporal *temp, Datum value, bool ever)
{
  if (tnumber_type(temp->temptype))
  {
    TBOX box;
    temporal_bbox(temp, &box);
    double d = datum_double(value, temptype_basetype(temp->temptype));
    if ((ever && d < box.xmin) || (!ever && d < box.xmax))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever equal to the base value.
 */
bool
temporal_ever_eq(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_ever_eq((TInstant *) temp, value);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_ever_eq((TInstantSet *) temp, value);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_ever_eq((TSequence *) temp, value);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_ever_eq((TSequenceSet *) temp, value);
  return result;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always equal to the base value.
 */
bool
temporal_always_eq(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_always_eq((TInstant *) temp, value);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_always_eq((TInstantSet *) temp, value);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_always_eq((TSequence *) temp, value);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_always_eq((TSequenceSet *) temp, value);
  return result;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever less than the base value.
 */
bool
temporal_ever_lt(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_ever_lt((TInstant *) temp, value);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_ever_lt((TInstantSet *) temp, value);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_ever_lt((TSequence *) temp, value);
  else /* subtype == SEQUENCESET */
    result = tsequenceset_ever_lt((TSequenceSet *) temp, value);
  return result;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always less than the base value.
 */
bool
temporal_always_lt(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_always_lt((TInstant *) temp, value);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_always_lt((TInstantSet *) temp, value);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_always_lt((TSequence *) temp, value);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_always_lt((TSequenceSet *) temp, value);
  return result;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever less than or equal to the
 * base value.
 */
bool
temporal_ever_le(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_ever_le((TInstant *) temp, value);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_ever_le((TInstantSet *) temp, value);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_ever_le((TSequence *) temp, value);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_ever_le((TSequenceSet *) temp, value);
  return result;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always less than or equal to the
 * base value.
 */
bool
temporal_always_le(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_always_le((TInstant *) temp, value);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_always_le((TInstantSet *) temp, value);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_always_le((TSequence *) temp, value);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_always_le((TSequenceSet *) temp, value);
  return result;
}

/*****************************************************************************
 * Bounding box tests for the restriction functions
 *****************************************************************************/

/**
 * Return true if the bounding box of the temporal value contains the base value
 */
bool
temporal_bbox_restrict_value(const Temporal *temp, Datum value)
{
  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    TBOX box1, box2;
    temporal_bbox(temp, &box1);
    number_tbox(value, temptype_basetype(temp->temptype), &box2);
    return contains_tbox_tbox(&box1, &box2);
  }
  if (tgeo_type(temp->temptype))
  {
    /* Test that the geometry is not empty */
    GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(value);
    ensure_point_type(gs);
    ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
    ensure_same_dimensionality_tpoint_gs(temp, gs);
    if (gserialized_is_empty(gs))
      return false;
    if (temp->subtype != INSTANT)
    {
      STBOX box1, box2;
      temporal_bbox(temp, &box1);
      geo_stbox(gs, &box2);
      return contains_stbox_stbox(&box1, &box2);
    }
  }
  return true;
}

/**
 * Return the array of base values that are contained in the bounding box
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
  CachedType basetype = temptype_basetype(temp->temptype);

  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    TBOX box1;
    temporal_bbox(temp, &box1);
    for (int i = 0; i < count; i++)
    {
      TBOX box2;
      number_tbox(values[i], basetype, &box2);
      if (contains_tbox_tbox(&box1, &box2))
        newvalues[k++] = values[i];
    }
  }
  if (tgeo_type(temp->temptype))
  {
    STBOX box1;
    temporal_bbox(temp, &box1);
    for (int i = 0; i < count; i++)
    {
      /* Test that the geometry is not empty */
      GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(values[i]);
      ensure_point_type(gs);
      ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
      ensure_same_dimensionality_tpoint_gs(temp, gs);
      if (! gserialized_is_empty(gs))
      {
        STBOX box2;
        geo_stbox(gs, &box2);
        if (contains_stbox_stbox(&box1, &box2))
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
  if (k > 1)
  {
    datumarr_sort(newvalues, k, basetype);
    k = datumarr_remove_duplicates(newvalues, k, basetype);
  }
  *newcount = k;
  return newvalues;
}

/**
 * Return true if the bounding box of the temporal number overlaps the range
 * of base values
 */
bool
tnumber_bbox_restrict_range(const Temporal *temp, const RangeType *range)
{
  /* Bounding box test */
  assert(tnumber_type(temp->temptype));
  TBOX box1, box2;
  temporal_bbox(temp, &box1);
  range_tbox(range, &box2);
  return overlaps_tbox_tbox(&box1, &box2);
}

/**
 * Return the array of ranges of base values that overlap with the bounding box
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
  assert(tnumber_type(temp->temptype));
  RangeType **newranges = palloc(sizeof(Datum) * count);
  int k = 0;
  TBOX box1;
  temporal_bbox(temp, &box1);
  for (int i = 0; i < count; i++)
  {
    char flags = range_get_flags(ranges[i]);
    if (flags & RANGE_EMPTY)
      continue;
    TBOX box2;
    range_tbox(ranges[i], &box2);
    if (overlaps_tbox_tbox(&box1, &box2))
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
 * Restrict the temporal value to the (complement of the) base value.
 *
 * @note This function does a bounding box test for the temporal types
 * different from instant. The singleton tests are done in the functions for
 * the specific temporal types.
 */
Temporal *
temporal_restrict_value(const Temporal *temp, Datum value, bool atfunc)
{
  /* Bounding box test */
  if (! temporal_bbox_restrict_value(temp, value))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->subtype != SEQUENCE) ?
        temporal_copy(temp) :
        (Temporal *) tsequence_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinstant_restrict_value(
      (TInstant *) temp, value, atfunc);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_value(
      (TInstantSet *) temp, value, atfunc);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tsequence_restrict_value((TSequence *) temp,
      value, atfunc);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_value(
      (TSequenceSet *) temp, value, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the (complement of the) array of base values.
 */
Temporal *
temporal_restrict_values(const Temporal *temp, Datum *values, int count,
  bool atfunc)
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
      return (temp->subtype != SEQUENCE) ?
        temporal_copy(temp) :
        (Temporal *) tsequence_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinstant_restrict_values(
      (TInstant *) temp, newvalues, newcount, atfunc);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_values(
      (TInstantSet *) temp, newvalues, newcount, atfunc);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tsequence_restrict_values((TSequence *) temp,
      newvalues, newcount, atfunc);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_values(
      (TSequenceSet *) temp, newvalues, newcount, atfunc);

  pfree(newvalues);
  return result;
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the (complement of the) range of base values.
 */
Temporal *
tnumber_restrict_range(const Temporal *temp, RangeType *range, bool atfunc)
{
  /* Empty range and Bounding box test */
  char flags = range_get_flags(range);
  if (flags & RANGE_EMPTY ||
    ! tnumber_bbox_restrict_range(temp, range))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->subtype != SEQUENCE) ?
        temporal_copy(temp) :
        (Temporal *) tsequence_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tnumberinst_restrict_range(
      (TInstant *) temp, range, atfunc);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tnumberinstset_restrict_range(
      (TInstantSet *) temp, range, atfunc);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tnumberseq_restrict_range(
      (TSequence *) temp, range, atfunc);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tnumberseqset_restrict_range(
      (TSequenceSet *) temp, range, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the (complement of the) array of ranges
 * of base values.
 */
Temporal *
tnumber_restrict_ranges(const Temporal *temp, RangeType **ranges, int count,
  bool atfunc)
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
      return (temp->subtype != SEQUENCE) ?
        temporal_copy(temp) :
        (Temporal *) tsequence_tsequenceset((TSequence *) temp);
  }
  if (newcount == 1)
    return tnumber_restrict_range(temp, newranges[0], atfunc);

  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tnumberinst_restrict_ranges((TInstant *) temp,
      newranges, newcount, atfunc);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tnumberinstset_restrict_ranges((TInstantSet *) temp,
      newranges, newcount, atfunc);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tnumberseq_restrict_ranges((TSequence *) temp,
        newranges, newcount, atfunc, BBOX_TEST_NO);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tnumberseqset_restrict_ranges((TSequenceSet *) temp,
      newranges, newcount, atfunc);

  pfree_array((void **) newranges, newcount);

  return result;
}

/*****************************************************************************/

/**
 * Restrict the temporal value to (the complement of) the minimum base value
 */
Temporal *
temporal_restrict_minmax(const Temporal *temp, bool min, bool atfunc)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = atfunc ? (Temporal *) tinstant_copy((TInstant *) temp) : NULL;
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_minmax((TInstantSet *) temp,
      min, atfunc);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tsequence_restrict_minmax((TSequence *) temp,
      min, atfunc);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_minmax((TSequenceSet *) temp,
      min, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the timestamp.
 */
Temporal *
temporal_restrict_timestamp(const Temporal *temp, TimestampTz t,
  bool atfunc)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinstant_restrict_timestamp((TInstant *) temp, t, atfunc);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_timestamp((TInstantSet *) temp,
    t, atfunc);
  else if (temp->subtype == SEQUENCE)
    result = atfunc ?
      (Temporal *) tsequence_at_timestamp((TSequence *) temp, t) :
      (Temporal *) tsequence_minus_timestamp((TSequence *) temp, t);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_timestamp((TSequenceSet *) temp,
      t, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * Return the base value of the temporal value at the timestamp when the
 * timestamp may be at an exclusive bound
 */
bool
temporal_value_at_timestamp_inc(const Temporal *temp, TimestampTz t, Datum *value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_value_at_timestamp((TInstant *) temp, t, value);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_value_at_timestamp((TInstantSet *) temp, t, value);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_value_at_timestamp_inc((TSequence *) temp, t, value);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_value_at_timestamp_inc((TSequenceSet *) temp, t, value);
  return result;
}

/**
 * Return the base value of the temporal value at the timestamp
 */
bool
temporal_value_at_timestamp(const Temporal *temp, TimestampTz t, Datum *result)
{
  bool found = false;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    found = tinstant_value_at_timestamp((TInstant *) temp, t, result);
  else if (temp->subtype == INSTANTSET)
    found = tinstantset_value_at_timestamp((TInstantSet *) temp, t, result);
  else if (temp->subtype == SEQUENCE)
    found = tsequence_value_at_timestamp((TSequence *) temp, t, result);
  else /* subtype == SEQUENCESET */
    found = tsequenceset_value_at_timestamp((TSequenceSet *) temp, t, result);
  return found;
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the (complement of the) timestamp set
 */
Temporal *
temporal_restrict_timestampset(const Temporal *temp, const TimestampSet *ts,
  bool atfunc)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinstant_restrict_timestampset(
      (TInstant *) temp, ts, atfunc);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_timestampset(
      (TInstantSet *) temp, ts, atfunc);
  else if (temp->subtype == SEQUENCE)
    result = atfunc ?
      (Temporal *) tsequence_at_timestampset((TSequence *) temp, ts) :
      (Temporal *) tsequence_minus_timestampset((TSequence *) temp, ts);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_timestampset(
      (TSequenceSet *) temp, ts, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the (complement of the) period.
 */
Temporal *
temporal_restrict_period(const Temporal *temp, const Period *p,
  bool atfunc)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinstant_restrict_period(
      (TInstant *) temp, p, atfunc);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_period(
      (TInstantSet *) temp, p, atfunc);
  else if (temp->subtype == SEQUENCE)
    result = atfunc ?
      (Temporal *) tsequence_at_period((TSequence *) temp, p) :
      (Temporal *) tsequence_minus_period((TSequence *) temp, p);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_period(
      (TSequenceSet *) temp, p, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the (complement of the) period set.
 */
Temporal *
temporal_restrict_periodset(const Temporal *temp, const PeriodSet *ps,
  bool atfunc)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinstant_restrict_periodset(
      (TInstant *) temp, ps, atfunc);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tinstantset_restrict_periodset(
      (TInstantSet *) temp, ps, atfunc);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tsequence_restrict_periodset(
      (TSequence *) temp, ps, atfunc);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_periodset(
      (TSequenceSet *) temp, ps, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal number to the temporal box.
 */
Temporal *
tnumber_at_tbox(const Temporal *temp, const TBOX *box)
{
  /* Bounding box test */
  TBOX box1;
  temporal_bbox(temp, &box1);
  if (! overlaps_tbox_tbox(box, &box1))
    return NULL;

  /* At least one of MOBDB_FLAGS_GET_T and MOBDB_FLAGS_GET_X is true */
  Temporal *temp1;
  bool hasx = MOBDB_FLAGS_GET_X(box->flags);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  if (hast)
  {
    Period p;
    period_set(box->tmin, box->tmax, true, true, &p);
    temp1 = temporal_restrict_period(temp, &p, REST_AT);
    /* Despite the bounding box test above, temp1 may be NULL due to
     * exclusive bounds */
    if (temp1 == NULL)
      return NULL;
  }
  else
    temp1 = (Temporal *) temp;

  Temporal *result;
  if (hasx)
  {
    /* Ensure function is called for temporal numbers */
    ensure_tnumber_type(temp->temptype);
    /* The basetype of the temporal value determines wheter the
     * argument box is converted into an intrange or a floatrange */
    RangeType *range;
    if (temp->temptype == T_TINT)
      range = range_make(Int32GetDatum((int) box->xmin),
        Int32GetDatum((int) box->xmax), true, true, T_INT4);
    else /* temp->temptype == T_TFLOAT */
      range = range_make(Float8GetDatum(box->xmin),
        Float8GetDatum(box->xmax), true, true, T_FLOAT8);
    result = tnumber_restrict_range(temp1, range, true);
    pfree(range);
  }
  else
    result = temp1;
  if (hasx && hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal number to the complement of the temporal box.
 *
 * We cannot make the difference from each dimension separately, i.e.,
 * restrict at the period and then restrict to the range. Therefore, we
 * compute the atTbox and then compute the complement of the value obtained.
 *
 */
Temporal *
tnumber_minus_tbox(const Temporal *temp, const TBOX *box)
{
  /* Bounding box test */
  TBOX box1;
  temporal_bbox(temp, &box1);
  if (! overlaps_tbox_tbox(box, &box1))
    return temporal_copy(temp);

  Temporal *result = NULL;
  Temporal *temp1 = tnumber_at_tbox(temp, box);
  if (temp1 != NULL)
  {
    PeriodSet *ps1 = temporal_time(temp);
    PeriodSet *ps2 = temporal_time(temp1);
    PeriodSet *ps = minus_periodset_periodset(ps1, ps2);
    if (ps != NULL)
    {
      result = temporal_restrict_periodset(temp, ps, true);
      pfree(ps);
    }
    pfree(temp1); pfree(ps1); pfree(ps2);
  }
  return result;
}

/*****************************************************************************
 * Intersects functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the timestamp
 */
bool
temporal_intersects_timestamp(const Temporal *temp, TimestampTz t)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_intersects_timestamp((TInstant *) temp, t);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_intersects_timestamp((TInstantSet *) temp, t);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_intersects_timestamp((TSequence *) temp, t);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_intersects_timestamp((TSequenceSet *) temp, t);
  return result;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the timestamp set
 */
bool
temporal_intersects_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_intersects_timestampset((TInstant *) temp, ts);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_intersects_timestampset((TInstantSet *) temp, ts);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_intersects_timestampset((TSequence *) temp, ts);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_intersects_timestampset((TSequenceSet *) temp, ts);
  return result;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the period
 */
bool
temporal_intersects_period(const Temporal *temp, const Period *p)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_intersects_period((TInstant *) temp, p);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_intersects_period((TInstantSet *) temp, p);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_intersects_period((TSequence *) temp, p);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_intersects_period((TSequenceSet *) temp, p);
  return result;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the period set
 */
bool
temporal_intersects_periodset(const Temporal *temp, const PeriodSet *ps)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_intersects_periodset((TInstant *) temp, ps);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_intersects_periodset((TInstantSet *) temp, ps);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_intersects_periodset((TSequence *) temp, ps);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_intersects_periodset((TSequenceSet *) temp, ps);
  return result;
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_agg
 * @brief Return the integral (area under the curve) of the temporal
 * number value
 */
double
tnumber_integral(const Temporal *temp)
{
  double result = 0.0;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT || temp->subtype == INSTANTSET)
    ;
  else if (temp->subtype == SEQUENCE)
    result = tnumberseq_integral((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tnumberseqset_integral((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Return the time-weighted average of the temporal number
 */
double
tnumber_twavg(const Temporal *temp)
{
  double result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tnumberinst_double((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = tnumberinstset_twavg((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = tnumberseq_twavg((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tnumberseqset_twavg((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the two temporal values are equal.
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
temporal_eq(const Temporal *temp1, const Temporal *temp2)
{
  assert(temp1->temptype == temp2->temptype);
  ensure_valid_tempsubtype(temp1->subtype);
  ensure_valid_tempsubtype(temp2->subtype);

  const TInstant *inst1, *inst2;
  const TInstantSet *ti;
  const TSequence *seq;
  const TSequenceSet *ts;
  /* If both are of the same temporal type use the specific equality */
  if (temp1->subtype == temp2->subtype)
  {
    if (temp1->subtype == INSTANT)
      return tinstant_eq((TInstant *) temp1, (TInstant *) temp2);
    else if (temp1->subtype == INSTANTSET)
      return tinstantset_eq((TInstantSet *) temp1, (TInstantSet *) temp2);
    else if (temp1->subtype == SEQUENCE)
      return tsequence_eq((TSequence *) temp1, (TSequence *) temp2);
    else /* temp1->subtype == SEQUENCESET */
      return tsequenceset_eq((TSequenceSet *) temp1, (TSequenceSet *) temp2);
  }

  /* Different temporal type */
  if (temp1->subtype > temp2->subtype)
  {
    const Temporal *temp = (Temporal *) temp1;
    temp1 = temp2;
    temp2 = temp;
  }
  if (temp1->subtype == INSTANT)
  {
    const TInstant *inst = (TInstant *) temp1;
    if (temp2->subtype == INSTANTSET)
    {
      ti = (TInstantSet *) temp2;
      if (ti->count != 1)
        return false;
      inst1 = tinstantset_inst_n(ti, 0);
      return tinstant_eq(inst, inst1);
    }
    if (temp2->subtype == SEQUENCE)
    {
      seq = (TSequence *) temp2;
      if (seq->count != 1)
        return false;
      inst1 = tsequence_inst_n(seq, 0);
      return tinstant_eq(inst, inst1);
    }
    if (temp2->subtype == SEQUENCESET)
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
  else if (temp1->subtype == INSTANTSET)
  {
    ti = (TInstantSet *) temp1;
    if (temp2->subtype == SEQUENCE)
    {
      seq = (TSequence *) temp2;
      if (ti->count != 1 || seq->count != 1)
        return false;
      inst1 = tinstantset_inst_n(ti, 0);
      inst2 = tsequence_inst_n(seq, 0);
      return tinstant_eq(inst1, inst2);
    }
    if (temp2->subtype == SEQUENCESET)
    {
      ts = (TSequenceSet *) temp2;
      for (int i = 0; i < ti->count; i ++)
      {
        seq = tsequenceset_seq_n(ts, i);
        if (seq->count != 1)
          return false;
        inst1 = tinstantset_inst_n(ti, i);
        inst2 = tsequence_inst_n(seq, 0);
        if (! tinstant_eq(inst1, inst2))
          return false;
      }
      return true;
    }
  }
  /* temp1->subtype == SEQUENCE && temp2->subtype == SEQUENCESET */
  seq = (TSequence *) temp1;
  ts = (TSequenceSet *) temp2;
  if (ts->count != 1)
    return false;
  const TSequence *seq1 = tsequenceset_seq_n(ts, 0);
  return tsequence_eq(seq, seq1);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the two temporal values are different
 */
bool
temporal_ne(const Temporal *temp1, const Temporal *temp2)
{
  bool result = ! temporal_eq(temp1, temp2);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal value is
 * less than, equal, or greater than the second one.
 *
 * @note Function used for B-tree comparison
 */
int
temporal_cmp(const Temporal *temp1, const Temporal *temp2)
{
  assert(temp1->temptype == temp2->temptype);

  /* Compare bounding period
   * We need to compare periods AND bounding boxes since the bounding boxes
   * do not distinguish between inclusive and exclusive bounds */
  Period p1, p2;
  temporal_period(temp1, &p1);
  temporal_period(temp2, &p2);
  int result = period_cmp(&p1, &p2);
  if (result)
    return result;

  /* Compare bounding box */
  bboxunion box1, box2;
  temporal_bbox(temp1, &box1);
  temporal_bbox(temp2, &box2);
  result = temporal_bbox_cmp(&box1, &box2, temp1->temptype);
  if (result)
    return result;

  /* If both are of the same temporal type use the specific comparison */
  if (temp1->subtype == temp2->subtype)
  {
    ensure_valid_tempsubtype(temp1->subtype);
    if (temp1->subtype == INSTANT)
      return tinstant_cmp((TInstant *) temp1, (TInstant *) temp2);
    else if (temp1->subtype == INSTANTSET)
      return tinstantset_cmp((TInstantSet *) temp1, (TInstantSet *) temp2);
    else if (temp1->subtype == SEQUENCE)
      return tsequence_cmp((TSequence *) temp1, (TSequence *) temp2);
    else /* temp1->subtype == SEQUENCESET */
      return tsequenceset_cmp((TSequenceSet *) temp1, (TSequenceSet *) temp2);
  }

  /* Use the hash comparison */
  uint32 hash1 = temporal_hash(temp1);
  uint32 hash2 = temporal_hash(temp2);
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
  if (temp1->subtype < temp2->subtype)
    return -1;
  else if (temp1->subtype > temp2->subtype)
    return 1;
  else
    return 0;
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first temporal value is less than the second one
 */
bool
temporal_lt(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first temporal value is less than or equal to
 * the second one
 */
bool
temporal_le(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first temporal value is greater than or equal to
 * the second one
 */
bool
temporal_ge(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first temporal value is greater than the second one
 */
bool
temporal_gt(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp > 0;
}

/*****************************************************************************
 * Functions for defining hash index
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the hash value of the temporal value.
 */
uint32
temporal_hash(const Temporal *temp)
{
  uint32 result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_hash((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_hash((TInstantSet *) temp);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_hash((TSequence *) temp);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_hash((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*
 * This is required in a SINGLE file for builds against pgsql
 */
PG_MODULE_MAGIC;

/*****************************************************************************
 * Initialization function
 *****************************************************************************/

/**
 * Initialize the extension
 */
void
_PG_init(void)
{
  /* elog(WARNING, "This is MobilityDB."); */
  temporalgeom_init();
}

/*****************************************************************************
 * Typmod functions
 *****************************************************************************/

/**
 * Array storing the string representation of the concrete subtypes of
 * temporal types
 */
static char *tempsubtypeName[] =
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
struct tempsubtype_struct tempsubtype_struct_array[] =
{
  {"ANYTEMPSUBTYPE", ANYTEMPSUBTYPE},
  {"INSTANT", INSTANT},
  {"INSTANTSET", INSTANTSET},
  {"SEQUENCE", SEQUENCE},
  {"SEQUENCESET", SEQUENCESET},
};

/**
 * Return the string representation of the subtype of the
 * temporal type corresponding to the enum value
 */
const char *
tempsubtype_name(int16 subtype)
{
  return tempsubtypeName[subtype];
}

/**
 * Return the enum value corresponding to the string representation
 * of the concrete subtype of the temporal type.
 */
bool
tempsubtype_from_string(const char *str, int16 *subtype)
{
  char *tmpstr;
  size_t tmpstartpos, tmpendpos;
  size_t i;

  /* Initialize */
  *subtype = 0;
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
  for (i = 0; i < TEMPSUBTYPE_STRUCT_ARRAY_LEN; i++)
  {
    if (len == strnlen(tempsubtype_struct_array[i].subtypeName, TEMPSUBTYPE_MAX_LEN) &&
      !strncasecmp(tmpstr, tempsubtype_struct_array[i].subtypeName, TEMPSUBTYPE_MAX_LEN))
    {
      *subtype = tempsubtype_struct_array[i].subtype;
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
  uint8 typmod_subtype = TYPMOD_GET_SUBTYPE(typmod);
  uint8 subtype = temp->subtype;
  /* Typmod has a preference */
  if (typmod_subtype != ANYTEMPSUBTYPE && typmod_subtype != subtype)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Temporal type (%s) does not match column type (%s)",
      tempsubtype_name(subtype), tempsubtype_name(typmod_subtype))));
  return temp;
}

PG_FUNCTION_INFO_V1(Temporal_typmod_in);
/**
 * Input typmod information for temporal types
 */
PGDLLEXPORT Datum
Temporal_typmod_in(PG_FUNCTION_ARGS)
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
  if (strlen(s) == 0)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Empty temporal type modifier")));

  int16 subtype = ANYTEMPSUBTYPE;
  if (! tempsubtype_from_string(s, &subtype))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Invalid temporal type modifier: %s", s)));

  pfree(elem_values);
  PG_RETURN_INT32((int32) subtype);
}

PG_FUNCTION_INFO_V1(Temporal_typmod_out);
/**
 * Output typmod information for temporal types
 */
PGDLLEXPORT Datum
Temporal_typmod_out(PG_FUNCTION_ARGS)
{
  char *s = (char *) palloc(64);
  char *str = s;
  int32 typmod = PG_GETARG_INT32(0);
  int16 subtype = TYPMOD_GET_SUBTYPE(typmod);
  /* No type? Then no typmod at all. Return empty string.  */
  if (typmod < 0 || !subtype)
  {
    *str = '\0';
    PG_RETURN_CSTRING(str);
  }
  sprintf(str, "(%s)", tempsubtype_name(subtype));
  PG_RETURN_CSTRING(s);
}

PG_FUNCTION_INFO_V1(Temporal_enforce_typmod);
/**
 * Enforce typmod information for temporal types
 */
PGDLLEXPORT Datum
Temporal_enforce_typmod(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 typmod = PG_GETARG_INT32(1);
  /* Check if temporal typmod is consistent with the supplied one */
  temp = temporal_valid_typmod(temp, typmod);
  PG_RETURN_POINTER(temp);
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Peak into a temporal datum to find the bounding box. If the datum needs
 * needs to be detoasted, extract only the header and not the full object.
 */
void
temporal_bbox_slice(Datum tempdatum, void *box)
{
  Temporal *temp = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) tempdatum))
    temp = (Temporal *) PG_DETOAST_DATUM_SLICE(tempdatum, 0,
      temporal_max_header_size());
  else
    temp = (Temporal *) tempdatum;
  temporal_bbox(temp, box);
  PG_FREE_IF_COPY_P(temp, DatumGetPointer(tempdatum));
  return;
}

/*****************************************************************************
 * Version functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Mobilitydb_version);
/**
 * Version of the MobilityDB extension
 */
PGDLLEXPORT Datum
Mobilitydb_version(PG_FUNCTION_ARGS __attribute__((unused)))
{
  char *ver = MOBILITYDB_VERSION_STR;
  text *result = cstring_to_text(ver);
  PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(Mobilitydb_full_version);
/**
 * Versions of the MobilityDB extension and its dependencies
 */
PGDLLEXPORT Datum
Mobilitydb_full_version(PG_FUNCTION_ARGS __attribute__((unused)))
{
  char ver[128];
  text *result;

  snprintf(ver, 128, "%s, %s, %s", MOBILITYDB_VERSION_STR,
    POSTGRESQL_VERSION_STRING, POSTGIS_VERSION_STR);
  ver[127] = '\0';

  result = cstring_to_text(ver);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_in);
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
Temporal_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  Oid temptypid = PG_GETARG_OID(1);
  Temporal *result = temporal_parse(&input, oid_type(temptypid));
  int32 temp_typmod = -1;
  if (PG_NARGS() > 2 && !PG_ARGISNULL(2))
    temp_typmod = PG_GETARG_INT32(2);
  if (temp_typmod >= 0)
    result = temporal_valid_typmod(result, temp_typmod);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_out);
/**
 * Generic output function for temporal types
 */
PGDLLEXPORT Datum
Temporal_out(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *result = temporal_to_string(temp, &call_output);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(Temporal_send);
/*
 * Generic send function for temporal types
 */
PGDLLEXPORT Datum
Temporal_send(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  temporal_write(temp, &buf) ;
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(Temporal_recv);
/**
 * Generic receive function for temporal types
 */
PGDLLEXPORT Datum
Temporal_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
  Oid temptypid = PG_GETARG_OID(1);
  Temporal *result = temporal_read(buf, oid_type(temptypid));
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

PG_FUNCTION_INFO_V1(Tinstant_constructor);
/**
 * Construct a temporal instant value from the arguments
 */
PGDLLEXPORT Datum
Tinstant_constructor(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  CachedType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  Temporal *result = (Temporal *) tinstant_make(value, t, temptype);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tinstantset_constructor);
/**
 * Construct a temporal instant set value from the array of temporal
 * instant values
 */
PGDLLEXPORT Datum
Tinstantset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  TInstant **instants = (TInstant **) temporalarr_extract(array, &count);
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
static Datum
tsequence_constructor_ext(FunctionCallInfo fcinfo, bool get_interp)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  bool lower_inc = PG_GETARG_BOOL(1);
  bool upper_inc = PG_GETARG_BOOL(2);
  bool linear = get_interp ? PG_GETARG_BOOL(3) : STEP;
  ensure_non_empty_array(array);
  int count;
  TInstant **instants = (TInstant **) temporalarr_extract(array, &count);
  Temporal *result = (Temporal *) tsequence_make((const TInstant **) instants,
    count, lower_inc, upper_inc, linear, NORMALIZE);
  pfree(instants);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tstepseq_constructor);
/**
 * Construct a temporal sequence value with stepwise interpolation from
 * the array of temporal instant values
 */
PGDLLEXPORT Datum
Tstepseq_constructor(PG_FUNCTION_ARGS)
{
  return tsequence_constructor_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Tlinearseq_constructor);
/**
 * Construct a temporal sequence value with linear or stepwise
 * interpolation from the array of temporal instant values
 */
PGDLLEXPORT Datum
Tlinearseq_constructor(PG_FUNCTION_ARGS)
{
  return tsequence_constructor_ext(fcinfo, true);
}

PG_FUNCTION_INFO_V1(Tsequenceset_constructor);
/**
 * Construct a temporal sequence set value from the array of temporal
 * sequence values
 */
PGDLLEXPORT Datum
Tsequenceset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  TSequence **sequences = (TSequence **) temporalarr_extract(array, &count);
  Temporal *result = (Temporal *) tsequenceset_make(
    (const TSequence **) sequences, count, NORMALIZE);
  pfree(sequences);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Construct a temporal sequence set value from the array of temporal
 * instant values that are split in various sequences if two consecutive
 * instants have a spatial or temporal gap defined by the arguments
 */
Datum
tsequenceset_constructor_gaps_ext(FunctionCallInfo fcinfo, bool get_interp)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  bool linear;
  float maxdist;
  Interval *maxt;
  if (get_interp)
  {
    linear = PG_GETARG_BOOL(1);
    maxdist = PG_GETARG_FLOAT8(2);
    maxt = PG_GETARG_INTERVAL_P(3);
  }
  else
  {
    linear = STEP;
    maxdist = PG_GETARG_FLOAT8(1);
    maxt = PG_GETARG_INTERVAL_P(2);
  }
  /* Store fcinfo into a global variable */
  /* Needed for the distance function for temporal geographic points */
  store_fcinfo(fcinfo);
  /* Extract the array of instants */
  int count;
  TInstant **instants = (TInstant **) temporalarr_extract(array, &count);
  TSequenceSet *result = tsequenceset_make_gaps((const TInstant **) instants,
    count, linear, maxdist, maxt);
  pfree(instants);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tstepseqset_constructor_gaps);
/**
 * Construct a temporal sequence set value with stepwise interpolation from
 * the array of temporal instant values
 */
PGDLLEXPORT Datum
Tstepseqset_constructor_gaps(PG_FUNCTION_ARGS)
{
  return tsequenceset_constructor_gaps_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Tlinearseqset_constructor_gaps);
/**
 * Construct a temporal sequence set value with linear or stepwise
 * interpolation from the array of temporal instant values
 */
PGDLLEXPORT Datum
Tlinearseqset_constructor_gaps(PG_FUNCTION_ARGS)
{
  return tsequenceset_constructor_gaps_ext(fcinfo, true);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tinstantset_from_base);
/**
 * Construct a temporal instant set value from a base value and a timestamp set
 */
PGDLLEXPORT Datum
Tinstantset_from_base(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  CachedType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  TInstantSet *result = tinstantset_from_base(value, temptype, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tsequence_from_base);
/**
 * Construct a temporal sequence value from a base value and a period
 */
PGDLLEXPORT Datum
Tsequence_from_base(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  bool linear;
  if (PG_NARGS() == 2)
    linear = false;
  else
    linear = PG_GETARG_BOOL(2);
  CachedType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  TSequence *result = tsequence_from_base(value, temptype, p, linear);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tsequenceset_from_base);
/**
 * Construct a temporal sequence set value from from a base value and a
 * timestamp set
 */
PGDLLEXPORT Datum
Tsequenceset_from_base(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool linear;
  if (PG_NARGS() == 2)
    linear = false;
  else
    linear = PG_GETARG_BOOL(2);
  CachedType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  TSequenceSet *result = tsequenceset_from_base(value, temptype,
    ps, linear);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Append and merge functions
 ****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_append_tinstant);
/**
 * Append an instant to the end of a temporal value
 */
PGDLLEXPORT Datum
Temporal_append_tinstant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *inst = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = temporal_append_tinstant(temp, inst);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(inst, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_merge);
/**
 * Merge the two temporal values
 */
PGDLLEXPORT Datum
Temporal_merge(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL_P(1);
  Temporal *result = temporal_merge(temp1, temp2);
  if (temp1)
    PG_FREE_IF_COPY(temp1, 0);
  if (temp2)
    PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_merge_array);
/**
 * Merge the array of temporal values
 */
PGDLLEXPORT Datum
Temporal_merge_array(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  Temporal **temparr = temporalarr_extract(array, &count);
  Temporal *result = temporal_merge_array(temparr, count);
  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tint_to_range);
/**
 * Cast the temporal integer value as an intrange
 */
PGDLLEXPORT Datum
Tint_to_range(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  RangeType *result = tint_range(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tfloat_to_range);
/**
 * Cast the temporal integer value as an intrange
 */
PGDLLEXPORT Datum
Tfloat_to_range(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  RangeType *result = tfloat_range(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tint_to_tfloat);
/**
 * Cast the temporal integer value as a temporal float value
 */
PGDLLEXPORT Datum
Tint_to_tfloat(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tint_tfloat(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tfloat_to_tint);
/**
 * Cast the temporal float value as a temporal integer value
 */
PGDLLEXPORT Datum
Tfloat_to_tint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tfloat_tint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_to_period);
/**
 * Return the bounding period on which the temporal value is defined
 *
 * @note We cannot detoast only the header since we don't know whether the
 * lower and upper bounds of the period are inclusive or not
 */
PGDLLEXPORT Datum
Temporal_to_period(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Period *result = (Period *) palloc(sizeof(Period));
  temporal_period(temp, result);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_PERIOD_P(result);
}

PG_FUNCTION_INFO_V1(Tnumber_to_tbox);
/**
 * Return the bounding box of the temporal number
 */
PGDLLEXPORT Datum
Tnumber_to_tbox(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  temporal_bbox_slice(tempdatum, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_to_tinstant);
/**
 * Transform the temporal value into a temporal instant value
 */
PGDLLEXPORT Datum
Temporal_to_tinstant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_tinstant(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_to_tinstantset);
/**
 * Transform the temporal value into a temporal instant set value
 */
PGDLLEXPORT Datum
Temporal_to_tinstantset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_tinstantset(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_to_tsequence);
/**
 * Transform the temporal value into a temporal sequence value
 */
PGDLLEXPORT Datum
Temporal_to_tsequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_tsequence(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_to_tsequenceset);
/**
 * Transform the temporal value into a temporal sequence set value
 */
PGDLLEXPORT Datum
Temporal_to_tsequenceset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_tsequenceset(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tempstep_to_templinear);
/**
 * Transform the temporal value with continuous base type from stepwise
 * to linear interpolation
 */
PGDLLEXPORT Datum
Tempstep_to_templinear(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  ensure_seq_subtypes(temp->subtype);
  Temporal *result = tempstep_templinear(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_shift);
/**
 * Shift the time span of the temporal value by the interval
 */
PGDLLEXPORT Datum
Temporal_shift(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Temporal *result = temporal_shift_tscale(temp, true, false, start, NULL);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_tscale);
/**
 * Scale the time span of the temporal value by the interval
 */
PGDLLEXPORT Datum
Temporal_tscale(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  Temporal *result = temporal_shift_tscale(temp, false, true, NULL, duration);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_shift_tscale);
/**
 * Shift and scale the time span of the temporal value by the two intervals
 */
PGDLLEXPORT Datum
Temporal_shift_tscale(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  ensure_valid_duration(duration);
  Temporal *result = temporal_shift_tscale(temp, true, true, start, duration);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_subtype);
/**
 * Return the string representation of the temporal type
 */
PGDLLEXPORT Datum
Temporal_subtype(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *str = temporal_subtype(temp);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_interpolation);
/**
 * Return the string representation of the temporal interpolation
 */
PGDLLEXPORT Datum
Temporal_interpolation(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *str = temporal_interpolation(temp);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_memory_size);
/**
 * Return the size in bytes of the temporal value
 */
PGDLLEXPORT Datum
Temporal_memory_size(PG_FUNCTION_ARGS)
{
  Datum result = toast_datum_size(PG_GETARG_DATUM(0));
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Temporal_values);
/**
 * Return the base values of the temporal value as an array
 */
PGDLLEXPORT Datum
Temporal_values(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  Datum *values = temporal_values(temp, &count);
  ArrayType *result = datumarr_to_array(values, count,
    temptype_basetype(temp->temptype));
  pfree(values);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tfloat_ranges);
/**
 * Return the base values of the temporal float value as an array
 * of ranges
 */
PGDLLEXPORT Datum
Tfloat_ranges(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  RangeType **ranges = tfloat_ranges(temp, &count);
  ArrayType *result = rangearr_to_array(ranges, count, T_FLOATRANGE);
  pfree_array((void **) ranges, count);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tinstant_get_value);
/**
 * Return the base value of the temporal instant value
 */
PGDLLEXPORT Datum
Tinstant_get_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  if (temp->subtype != INSTANT)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal value must be of subtype instant")));

  TInstant *inst = (TInstant *) temp;
  Datum result = tinstant_value_copy(inst);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Temporal_time);
/**
 * Return the time on which the temporal value is defined as a period set
 */
PGDLLEXPORT Datum
Temporal_time(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PeriodSet *result = temporal_time(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tinstant_timestamp);
/**
 * Return the timestamp of the temporal instant value
 */
PGDLLEXPORT Datum
Tinstant_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  if (temp->subtype != INSTANT)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal value must be of subtype instant")));

  TimestampTz result = ((TInstant *) temp)->t;
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Tnumber_range);
/**
 * Return the value range of the temporal integer value
 */
PGDLLEXPORT Datum
Tnumber_range(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  RangeType *result = tnumber_range(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_RANGE_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_start_value);
/**
 * Return the start base value of the temporal value
 */
PGDLLEXPORT Datum
Temporal_start_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_start_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Temporal_end_value);
/**
 * Return the end base value of the temporal value
 */
PGDLLEXPORT Datum
Temporal_end_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_end_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Temporal_min_value);
/**
 * Return the minimum base value of the temporal value
 */
PGDLLEXPORT Datum
Temporal_min_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_min_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Temporal_max_value);
/**
 * Return the maximum base value of the temporal value
 */
PGDLLEXPORT Datum
Temporal_max_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_max_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Temporal_min_instant);
/**
 * Return a pointer to the instant with the minimum base value of the
 * temporal value
 */
PGDLLEXPORT Datum
Temporal_min_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = tinstant_copy(temporal_min_instant(temp));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_max_instant);
/**
 * Return a pointer to the instant with maximum base value of the
 * temporal value.
 */
PGDLLEXPORT Datum
Temporal_max_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = tinstant_copy(temporal_max_instant(temp));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_timespan);
/**
 * Return the timespan of the temporal value
 */
PGDLLEXPORT Datum
Temporal_timespan(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *result = temporal_timespan(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_duration);
/**
 * Return the duration of the temporal value
 */
PGDLLEXPORT Datum
Temporal_duration(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *result = temporal_duration(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_num_sequences);
/**
 * Return the number of sequences of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
Temporal_num_sequences(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  ensure_seq_subtypes(temp->subtype);
  int result = temporal_num_sequences(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Temporal_start_sequence);
/**
 * Return the start sequence of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
Temporal_start_sequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TSequence *result = temporal_start_sequence(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_end_sequence);
/**
 * Return the end sequence of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
Temporal_end_sequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TSequence *result = temporal_end_sequence(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_sequence_n);
/**
 * Return the n-th sequence of the temporal sequence (set) value
 */
PGDLLEXPORT Datum
Temporal_sequence_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TSequence *result = temporal_sequence_n(temp, n);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_sequences);
/**
 * Return the sequences of the temporal sequence (set) value as
 * an array
 */
PGDLLEXPORT Datum
Temporal_sequences(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  TSequence **sequences = temporal_sequences(temp, &count);
  ArrayType *result = temporalarr_to_array((const Temporal **) sequences,
    count);
  pfree_array((void **) sequences, count);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_segments);
/**
 * Return the segments of the temporal sequence (set) value as
 * an array
 */
PGDLLEXPORT Datum
Temporal_segments(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  TSequence **segments = temporal_segments(temp, &count);
  ArrayType *result = temporalarr_to_array((const Temporal **) segments,
    count);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_num_instants);
/**
 * Return the number of distinct instants of the temporal value
 */
PGDLLEXPORT Datum
Temporal_num_instants(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = temporal_num_instants(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Temporal_start_instant);
/**
 * Return the start instant of the temporal value
 */
PGDLLEXPORT Datum
Temporal_start_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = tinstant_copy(temporal_start_instant(temp));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_end_instant);
/**
 * Return the end instant of the temporal value
 */
PGDLLEXPORT Datum
Temporal_end_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = tinstant_copy(temporal_end_instant(temp));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_instant_n);
/**
 * Return the n-th instant of the temporal value
 */
PGDLLEXPORT Datum
Temporal_instant_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  const TInstant *inst = temporal_instant_n(temp, n);
  TInstant *result = (inst == NULL) ? NULL : tinstant_copy(inst);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_instants);
/**
 * Return the distinct instants of the temporal value as an array
 */
PGDLLEXPORT Datum
Temporal_instants(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  const TInstant **instants = temporal_instants(temp, &count);
  ArrayType *result = temporalarr_to_array((const Temporal **) instants,
    count);
  pfree(instants);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_num_timestamps);
/**
 * Return the number of distinct timestamps of the temporal value
 */
PGDLLEXPORT Datum
Temporal_num_timestamps(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = temporal_num_timestamps(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_start_timestamp);
/**
 * Return the start timestamp of the temporal value
 */
PGDLLEXPORT Datum
Temporal_start_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz result = temporal_start_timestamp(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Temporal_end_timestamp);
/**
 * Return the end timestamp of the temporal value
 */
PGDLLEXPORT Datum
Temporal_end_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz result = temporal_end_timestamp(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Temporal_timestamp_n);
/**
 * Return the n-th distinct timestamp of the temporal value
 */
PGDLLEXPORT Datum
Temporal_timestamp_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TimestampTz result;
  bool found = temporal_timestamp_n(temp, n, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Temporal_timestamps);
/**
 * Return the distinct timestamps of the temporal value as an array
 */
PGDLLEXPORT Datum
Temporal_timestamps(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  TimestampTz *times = temporal_timestamps(temp, &count);
  ArrayType *result = timestamparr_to_array(times, count);
  pfree(times);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * Generic function for the temporal ever/always comparison operators
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
temporal_ev_al_comp_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, Datum))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  bool result = func(temp, value);
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, temptype_basetype(temp->temptype), 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_ever_eq);
/**
 * Return true if the temporal value is ever equal to the base value
 */
PGDLLEXPORT Datum
Temporal_ever_eq(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_ever_eq);
}

PG_FUNCTION_INFO_V1(Temporal_always_eq);
/**
 * Return true if the temporal value is always equal to the base value
 */
PGDLLEXPORT Datum
Temporal_always_eq(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_always_eq);
}

PG_FUNCTION_INFO_V1(Temporal_ever_ne);
/**
 * Return true if the temporal value is ever different from the base value
 */
PGDLLEXPORT Datum
Temporal_ever_ne(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_always_eq);
}

PG_FUNCTION_INFO_V1(Temporal_always_ne);
/**
 * Return true if the temporal value is always different from the base value
 */
PGDLLEXPORT Datum
Temporal_always_ne(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_ever_eq);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_ever_lt);
/**
 * Return true if the temporal value is ever less than the base value
 */
PGDLLEXPORT Datum
Temporal_ever_lt(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_ever_lt);
}

PG_FUNCTION_INFO_V1(Temporal_always_lt);
/**
 * Return true if the temporal value is always less than the base value
 */
PGDLLEXPORT Datum
Temporal_always_lt(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_always_lt);
}

PG_FUNCTION_INFO_V1(Temporal_ever_le);
/**
 * Return true if the temporal value is ever less than or equal to the base value
 */
PGDLLEXPORT Datum
Temporal_ever_le(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_ever_le);
}

PG_FUNCTION_INFO_V1(Temporal_always_le);
/**
 * Return true if the temporal value is always less than or equal to the base value
 */
PGDLLEXPORT Datum
Temporal_always_le(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_always_le);
}

PG_FUNCTION_INFO_V1(Temporal_ever_gt);
/**
 * Return true if the temporal value is ever greater than the base value
 */
PGDLLEXPORT Datum
Temporal_ever_gt(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_always_le);
}

PG_FUNCTION_INFO_V1(Temporal_always_gt);
/**
 * Return true if the temporal value is always greater than the base value
 */
PGDLLEXPORT Datum
Temporal_always_gt(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_ever_le);
}

PG_FUNCTION_INFO_V1(Temporal_ever_ge);
/**
 * Return true if the temporal value is ever greater than or equal
 * to the base value
 */
PGDLLEXPORT Datum
Temporal_ever_ge(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_always_lt);
}

PG_FUNCTION_INFO_V1(Temporal_always_ge);
/**
 * Return true if the temporal value is always greater than or equal
 * to the base value
 */
PGDLLEXPORT Datum
Temporal_always_ge(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_ever_lt);
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * Restrict the temporal value to the (complement of the) array of base values
 */
static Datum
temporal_restrict_value_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Temporal *result = temporal_restrict_value(temp, value, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, basetype, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_at_value);
/**
 * Restrict the temporal value to the base value
 */
PGDLLEXPORT Datum
Temporal_at_value(PG_FUNCTION_ARGS)
{
  return temporal_restrict_value_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Temporal_minus_value);
/**
 * Restrict the temporal value to the complement of the base value
 */
PGDLLEXPORT Datum
Temporal_minus_value(PG_FUNCTION_ARGS)
{
  return temporal_restrict_value_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the (complement of the) array of base values
 */
static Datum
temporal_restrict_values_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
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
    temporal_restrict_values(temp, values, count, atfunc) :
    temporal_restrict_value(temp, values[0], atfunc);

  pfree(values);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(array, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_at_values);
/**
 * Restrict the temporal value to the array of base values
 */
PGDLLEXPORT Datum
Temporal_at_values(PG_FUNCTION_ARGS)
{
  return temporal_restrict_values_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Temporal_minus_values);
/**
 * Restrict the temporal value to the complement of the array of base values
 */
PGDLLEXPORT Datum
Temporal_minus_values(PG_FUNCTION_ARGS)
{
  return temporal_restrict_values_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

static Datum
tnumber_restrict_range_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  RangeType *range = PG_GETARG_RANGE_P(1);
  Temporal *result = tnumber_restrict_range(temp, range, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(range, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tnumber_at_range);
/**
 * Restrict the temporal value to the range of base values
 */
PGDLLEXPORT Datum
Tnumber_at_range(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_range_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Tnumber_minus_range);
/**
 * Restrict the temporal value to the complement of the range of base values
 */
PGDLLEXPORT Datum
Tnumber_minus_range(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_range_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the (complement of the) array of ranges
 * of base values
 */
static Datum
tnumber_restrict_ranges_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
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
    tnumber_restrict_ranges(temp, ranges, count, atfunc) :
    tnumber_restrict_range(temp, ranges[0], atfunc);
  pfree(ranges);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(array, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tnumber_at_ranges);
/**
 * Restrict the temporal value to the array of ranges of base values
 */
PGDLLEXPORT Datum
Tnumber_at_ranges(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_ranges_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Tnumber_minus_ranges);
/**
 * Restrict the temporal value to the complement of the array of ranges
 * of base values
 */
PGDLLEXPORT Datum
Tnumber_minus_ranges(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_ranges_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_at_min);
/**
 * Restrict the temporal value to the minimum base value
 */
PGDLLEXPORT Datum
Temporal_at_min(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_restrict_minmax(temp, GET_MIN, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_minus_min);
/**
 * Restrict the temporal value to the complement of the minimum base value
 */
PGDLLEXPORT Datum
Temporal_minus_min(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_restrict_minmax(temp, GET_MIN, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_at_max);
/**
 * Restrict the temporal value to the maximum base value
 */
PGDLLEXPORT Datum
Temporal_at_max(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_restrict_minmax(temp, GET_MAX, REST_AT);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_minus_max);
/**
 * Restrict the temporal value to the complement of the maximum base value
 */
PGDLLEXPORT Datum
Temporal_minus_max(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_restrict_minmax(temp, GET_MAX, REST_MINUS);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the (complement of the) timestamp
 */
static Datum
temporal_restrict_timestamp_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Temporal *result = temporal_restrict_timestamp(temp, t, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_at_timestamp);
/**
 * Restrict the temporal value to the timestamp
 */
PGDLLEXPORT Datum
Temporal_at_timestamp(PG_FUNCTION_ARGS)
{
  return temporal_restrict_timestamp_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Temporal_minus_timestamp);
/**
 * Restrict the temporal value to the complement of the timestamp
 */
PGDLLEXPORT Datum
Temporal_minus_timestamp(PG_FUNCTION_ARGS)
{
  return temporal_restrict_timestamp_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_value_at_timestamp);
/**
 * Return the base value of the temporal value at the timestamp
 */
PGDLLEXPORT Datum
Temporal_value_at_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Datum result;
  bool found = temporal_value_at_timestamp(temp, t, &result);
  PG_FREE_IF_COPY(temp, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_at_timestampset);
/**
 * Restrict the temporal value to the timestamp set
 */
PGDLLEXPORT Datum
Temporal_at_timestampset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  Temporal *result = temporal_restrict_timestampset(temp, ts, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_minus_timestampset);
/**
 * Restrict the temporal value to the complement of the timestamp set
 */
PGDLLEXPORT Datum
Temporal_minus_timestampset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  Temporal *result = temporal_restrict_timestampset(temp, ts, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

static Datum
temporal_restrict_period_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  Temporal *result = temporal_restrict_period(temp, p, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_at_period);
/**
 * Restrict the temporal value to the period
 */
PGDLLEXPORT Datum
Temporal_at_period(PG_FUNCTION_ARGS)
{
  return temporal_restrict_period_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Temporal_minus_period);
/**
 * Restrict the temporal value to the complement of the period
 */
PGDLLEXPORT Datum
Temporal_minus_period(PG_FUNCTION_ARGS)
{
  return temporal_restrict_period_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_at_periodset);
/**
 * Restrict the temporal value to the period set
 */
PGDLLEXPORT Datum
Temporal_at_periodset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  Temporal *result = temporal_restrict_periodset(temp, ps, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_minus_periodset);
/**
 * Restrict the temporal value to the complement of the period set
 */
PGDLLEXPORT Datum
Temporal_minus_periodset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  Temporal *result = temporal_restrict_periodset(temp, ps, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Restrict the temporal value to the (complement of the) temporal box
  */
static Datum
tnumber_restrict_tbox_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TBOX *box = PG_GETARG_TBOX_P(1);
  Temporal *result = atfunc ? tnumber_at_tbox(temp, box) :
    tnumber_minus_tbox(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tnumber_at_tbox);
/**
 * Restrict the temporal value to the temporal box
 */
PGDLLEXPORT Datum
Tnumber_at_tbox(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_tbox_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Tnumber_minus_tbox);
/**
 * Restrict the temporal value to the complement of the temporal box
 */
PGDLLEXPORT Datum
Tnumber_minus_tbox(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_tbox_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************
 * Intersects functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_intersects_timestamp);
/**
 * Return true if the temporal value intersects the timestamp
 */
PGDLLEXPORT Datum
Temporal_intersects_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = temporal_intersects_timestamp(temp, t);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_intersects_timestampset);
/**
 * Return true if the temporal value intersects the timestamp set
 */
PGDLLEXPORT Datum
Temporal_intersects_timestampset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = temporal_intersects_timestampset(temp, ts);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_intersects_period);
/**
 * Return true if the temporal value intersects the period
 */
PGDLLEXPORT Datum
Temporal_intersects_period(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  bool result = temporal_intersects_period(temp, p);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_intersects_periodset);
/**
 * Return true if the temporal value intersects the period set
 */
PGDLLEXPORT Datum
Temporal_intersects_periodset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = temporal_intersects_periodset(temp, ps);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnumber_integral);
/**
 * Return the integral (area under the curve) of the temporal
 * number value
 */
PGDLLEXPORT Datum
Tnumber_integral(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = tnumber_integral(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Tnumber_twavg);
/**
 * Return the time-weighted average of the temporal number
 */
PGDLLEXPORT Datum
Tnumber_twavg(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = tnumber_twavg(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_eq);
/**
 * Return true if the two temporal values are equal
 */
PGDLLEXPORT Datum
Temporal_eq(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_eq(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_ne);
/**
 * Return true if the two temporal values are different
 */
PGDLLEXPORT Datum
Temporal_ne(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_ne(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first temporal value
 * is less than, equal, or greater than the second temporal value
 *
 * @note Function used for B-tree comparison
 */
PGDLLEXPORT Datum
Temporal_cmp(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = temporal_cmp(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Temporal_lt);
/**
 * Return true if the first temporal value is less than the second one
 */
PGDLLEXPORT Datum
Temporal_lt(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_lt(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_le);
/**
 * Return true if the first temporal value is less than or equal to
 * the second one
 */
PGDLLEXPORT Datum
Temporal_le(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_le(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_ge);
/**
 * Return true if the first temporal value is greater than or equal to
 * the second one
 */
PGDLLEXPORT Datum
Temporal_ge(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_ge(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_gt);
/**
 * Return true if the first temporal value is greater than the second one
 */
PGDLLEXPORT Datum
Temporal_gt(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_gt(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Functions for defining hash index
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_hash);
/**
 * Return the hash value of the temporal value
 */
PGDLLEXPORT Datum
Temporal_hash(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint32 result = temporal_hash(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_UINT32(result);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
