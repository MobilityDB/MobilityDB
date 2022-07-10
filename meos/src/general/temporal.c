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
 * @brief Basic functions for temporal types of any subtype.
 */

#include "general/temporal.h"

/* C */
#include <assert.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/doxygen_libmeos.h"
#include "general/pg_call.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/temporal_boxops.h"
#include "general/temporal_parser.h"
#include "general/tnumber_distance.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_spatialfuncs.h"
  #include "npoint/tnpoint_spatialfuncs.h"
#endif

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * Ensure that the subtype of a temporal value is valid
 *
 * @note Used for the dispatch functions
 */
void
ensure_valid_tempsubtype(int16 subtype)
{
  if (subtype != TINSTANT && subtype != TINSTANTSET &&
    subtype != TSEQUENCE && subtype != TSEQUENCESET)
    elog(ERROR, "unknown subtype for temporal type: %d", subtype);
  return;
}

/**
 * Ensure that the subtype of a temporal value is valid
 *
 * @note Used for the the analyze and selectivity functions
 */
void
ensure_valid_tempsubtype_all(int16 subtype)
{
  if (subtype != ANYTEMPSUBTYPE &&
    subtype != TINSTANT && subtype != TINSTANTSET &&
    subtype != TSEQUENCE && subtype != TSEQUENCESET)
    elog(ERROR, "unknown subtype for temporal type: %d", subtype);
  return;
}

/**
 * Ensure that the subtype of temporal type is a sequence (set)
 */
void
ensure_seq_subtypes(int16 subtype)
{
  if (subtype != TSEQUENCE && subtype != TSEQUENCESET)
    elog(ERROR, "Input must be a temporal sequence (set)");
  return;
}

/**
 * Ensure that the elements of an array are of instant subtype
 */
void
ensure_tinstarr(const TInstant **instants, int count)
{
  for (int i = 0; i < count; i++)
  {
    if (instants[i]->subtype != TINSTANT)
    {
      pfree(instants);
      elog(ERROR, "Input values must be temporal instants");
    }
  }
  return;
}

/**
 * Ensure that a temporal value has linear interpolation
 */
void
ensure_linear_interpolation(int16 flags)
{
  if (! MOBDB_FLAGS_GET_LINEAR(flags))
    elog(ERROR, "The temporal value must have linear interpolation");
  return;
}

/**
 * Ensure that two temporal values have at least one common dimension based on
 * their flags
 */
void
ensure_common_dimension(int16 flags1, int16 flags2)
{
  if (MOBDB_FLAGS_GET_X(flags1) != MOBDB_FLAGS_GET_X(flags2) &&
    MOBDB_FLAGS_GET_T(flags1) != MOBDB_FLAGS_GET_T(flags2))
    elog(ERROR, "The temporal values must have at least one common dimension");
  return;
}

/**
 * Ensure that two temporal values have the same base type
 */
void
ensure_same_temptype(const Temporal *temp1, const Temporal *temp2)
{
  if (temp1->temptype != temp2->temptype)
    elog(ERROR, "The temporal values must be of the same temporal type");
  return;
}

/**
 * Ensure that two temporal values have the same interpolation
 */
void
ensure_same_interpolation(const Temporal *temp1, const Temporal *temp2)
{
  if (MOBDB_FLAGS_GET_LINEAR(temp1->flags) !=
      MOBDB_FLAGS_GET_LINEAR(temp2->flags))
    elog(ERROR, "The temporal values must have the same interpolation");
  return;
}

/**
 * Ensure that the timestamp of the first temporal instant is smaller
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
    char *t1 = basetype_output(T_TIMESTAMPTZ, TimestampTzGetDatum(inst1->t));
    char *t2 = basetype_output(T_TIMESTAMPTZ, TimestampTzGetDatum(inst2->t));
    elog(ERROR, "Timestamps for temporal value must be increasing: %s, %s", t1, t2);
  }
  if (merge && inst1->t == inst2->t &&
    ! datum_eq(tinstant_value(inst1), tinstant_value(inst2),
        temptype_basetype(inst1->temptype)))
  {
    char *t1 = basetype_output(T_TIMESTAMPTZ, TimestampTzGetDatum(inst1->t));
    elog(ERROR, "The temporal values have different value at their overlapping instant %s", t1);
  }
  return;
}

/**
 * Ensure that two temporal instants have increasing timestamp
 * (or may be equal if the merge parameter is true), and if they
 * are temporal points, have the same srid and the same dimensionality.
 *
 * @param[in] inst1, inst2 Temporal instants
 * @param[in] merge True if a merge operation, which implies that the two
 *   consecutive instants may be equal
 * @param[in] subtype Subtype for which the function is called
 */
void
ensure_valid_tinstarr1(const TInstant *inst1, const TInstant *inst2,
#if NPOINT
  bool merge, int16 subtype)
#else
  bool merge, int16 subtype __attribute__((unused)))
#endif
{
  ensure_same_interpolation((Temporal *) inst1, (Temporal *) inst2);
  ensure_increasing_timestamps(inst1, inst2, merge);
  ensure_spatial_validity((Temporal *) inst1, (Temporal *) inst2);
#if NPOINT
  if (subtype == TSEQUENCE && inst1->temptype == T_TNPOINT)
    ensure_same_rid_tnpointinst(inst1, inst2);
#endif
  return;
}

/**
 * Ensure that all temporal instants of the array have increasing
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
 * Ensure that all temporal instants of the array have increasing
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
  mobdbType basetype = temptype_basetype(instants[0]->temptype);
  int *result = palloc(sizeof(int) * count);
  Datum value1 = tinstant_value(instants[0]);
#if NPOINT
  Datum geom1 = 0; /* Used only for temporal network points */
#endif
  datum_func2 point_distance = NULL;
  if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY)
    point_distance = pt_distance_fn(instants[0]->flags);
#if NPOINT
  else if (basetype == T_NPOINT)
    geom1 = PointerGetDatum(npoint_geom(DatumGetNpointP(value1)));
#endif
  int k = 0;
  for (int i = 1; i < count; i++)
  {
    ensure_valid_tinstarr1(instants[i - 1], instants[i], merge, subtype);
    bool split = false;
    Datum value2 = tinstant_value(instants[i]);
#if NPOINT
    Datum geom2 = 0; /* Used only for temporal network points */
#endif
    if (maxdist > 0 && ! datum_eq(value1, value2, basetype))
    {
      double dist = -1;
      if (tnumber_basetype(basetype))
        dist = (basetype == T_INT4) ?
          (double) DatumGetInt32(number_distance(value1, value2, basetype, basetype)) :
          DatumGetFloat8(number_distance(value1, value2, basetype, basetype));
      else if (basetype == T_GEOMETRY || basetype == T_GEOGRAPHY)
        dist = DatumGetFloat8(point_distance(value1, value2));
#if NPOINT
      else if (basetype == T_NPOINT)
      {
        geom2 = PointerGetDatum(npoint_geom(DatumGetNpointP(value2)));
        dist = DatumGetFloat8(pt_distance2d(geom1, geom2));
      }
#endif
      if (dist > maxdist)
        split = true;
    }
    /* If there is not already a split by distance */
    if (maxt != NULL && ! split)
    {
      Interval *duration = pg_timestamp_mi(instants[i]->t, instants[i - 1]->t);
      int cmp = pg_interval_cmp(duration, maxt);
      if (cmp > 0)
        split = true;
    }
    if (split)
      result[k++] = i;
    value1 = value2;
#if NPOINT
    if (basetype == T_NPOINT)
      geom1 = geom2;
#endif
  }
  *countsplits = k;
  return result;
}

/**
 * Ensure that all temporal instants of the array have increasing
 * timestamp, and if they are temporal points, have the same srid and the
 * same dimensionality
 */
void
ensure_valid_tseqarr(const TSequence **sequences, int count)
{
  for (int i = 1; i < count; i++)
  {
    ensure_same_interpolation((Temporal *) sequences[i - 1],
      (Temporal *) sequences[i]);
    TimestampTz upper1 = DatumGetTimestampTz(sequences[i - 1]->period.upper);
    TimestampTz lower2 = DatumGetTimestampTz(sequences[i]->period.lower);
    if ( upper1 > lower2 ||
         ( upper1 == lower2 && sequences[i - 1]->period.upper_inc &&
           sequences[i]->period.lower_inc ) )
    {
      char *t1 = basetype_output(T_TIMESTAMPTZ, sequences[i - 1]->period.upper);
      char *t2 = basetype_output(T_TIMESTAMPTZ, sequences[i]->period.lower);
      elog(ERROR, "Timestamps for temporal value must be increasing: %s, %s", t1, t2);
    }
    ensure_spatial_validity((Temporal *)sequences[i - 1], (Temporal *)sequences[i]);
  }
  return;
}

/*****************************************************************************/

/**
 * Ensure that the number is positive
 */
void
ensure_positive_datum(Datum size, mobdbType basetype)
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
 * Ensure that the interval is a positive and absolute duration
 */
void
ensure_valid_duration(const Interval *duration)
{
  if (duration->month != 0)
  {
    elog(ERROR, "Interval defined in terms of month, year, century etc. not supported");
  }
  Interval intervalzero;
  memset(&intervalzero, 0, sizeof(Interval));
  int cmp = pg_interval_cmp(duration, &intervalzero);
  if (cmp <= 0)
    elog(ERROR, "The interval must be positive");
  return;
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Return a pointer to the precomputed bounding box of a temporal value
 *
 * @return Return NULL for temporal instants since they do not have
 * precomputed bounding box.
 */
void *
temporal_bbox_ptr(const Temporal *temp)
{
  void *result = NULL;
  /* Values of TINSTANT subtype have not bounding box */
  if (temp->subtype == TINSTANTSET)
    result = tinstantset_bbox_ptr((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_bbox_ptr((TSequence *) temp);
  else if (temp->subtype == TSEQUENCESET)
    result = tsequenceset_bbox_ptr((TSequenceSet *) temp);
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
  if (temp1->subtype == TINSTANT)
  {
    if (temp2->subtype == TINSTANT)
      result = intersection_tinstant_tinstant(
        (TInstant *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == TINSTANTSET)
      result = intersection_tinstant_tinstantset(
        (TInstant *) temp1, (TInstantSet *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == TSEQUENCE)
      result = intersection_tinstant_tsequence(
        (TInstant *) temp1, (TSequence *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else /* temp2->subtype == TSEQUENCESET */
      result = intersection_tinstant_tsequenceset(
        (TInstant *) temp1, (TSequenceSet *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
  }
  else if (temp1->subtype == TINSTANTSET)
  {
    if (temp2->subtype == TINSTANT)
      result = intersection_tinstantset_tinstant(
        (TInstantSet *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == TINSTANTSET)
      result = intersection_tinstantset_tinstantset(
        (TInstantSet *) temp1, (TInstantSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else if (temp2->subtype == TSEQUENCE)
      result = intersection_tinstantset_tsequence(
        (TInstantSet *) temp1, (TSequence *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else /* temp2->subtype == TSEQUENCESET */
      result = intersection_tinstantset_tsequenceset(
        (TInstantSet *) temp1, (TSequenceSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
  }
  else if (temp1->subtype == TSEQUENCE)
  {
    if (temp2->subtype == TINSTANT)
      result = intersection_tsequence_tinstant(
        (TSequence *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == TINSTANTSET)
      result = intersection_tsequence_tinstantset(
        (TSequence *) temp1, (TInstantSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else if (temp2->subtype == TSEQUENCE)
      result = synchronize_tsequence_tsequence(
          (TSequence *) temp1, (TSequence *) temp2,
          (TSequence **) inter1, (TSequence **) inter2,
            mode == SYNCHRONIZE_CROSS);
    else /* temp2->subtype == TSEQUENCESET */
      result = intersection_tsequence_tsequenceset(
          (TSequence *) temp1, (TSequenceSet *) temp2, mode,
          (TSequenceSet **) inter1, (TSequenceSet **) inter2);
  }
  else /* temp1->subtype == TSEQUENCESET */
  {
    if (temp2->subtype == TINSTANT)
      result = intersection_tsequenceset_tinstant(
        (TSequenceSet *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == TINSTANTSET)
      result = intersection_tsequenceset_tinstantset(
        (TSequenceSet *) temp1, (TInstantSet *) temp2,
        (TInstantSet **) inter1, (TInstantSet **) inter2);
    else if (temp2->subtype == TSEQUENCE)
      result = synchronize_tsequenceset_tsequence(
          (TSequenceSet *) temp1, (TSequence *) temp2, mode,
          (TSequenceSet **) inter1, (TSequenceSet **) inter2);
    else /* temp2->subtype == TSEQUENCESET */
      result = synchronize_tsequenceset_tsequenceset(
        (TSequenceSet *) temp1, (TSequenceSet *) temp2, mode,
        (TSequenceSet **) inter1, (TSequenceSet **) inter2);
  }
  return result;
}

/**
 * Return the n-th instant of the temporal instant set or a temporal sequence.
 */
const TInstant *
tinstarr_inst_n(const Temporal *temp, int n)
{
  assert(temp->subtype == TINSTANTSET || temp->subtype == TSEQUENCE);
  if (temp->subtype == TINSTANTSET)
    return tinstantset_inst_n((TInstantSet *) temp, n);
  else /* temp->subtype == TSEQUENCE */
    return tsequence_inst_n((TSequence *) temp, n);
}

/*****************************************************************************
 * Version functions
 *****************************************************************************/

#define MOBDB_VERSION_STR_MAXLEN 128
/**
 * Version of the MobilityDB extension
 */
char *
mobilitydb_version(void)
{
  char *result = MOBILITYDB_VERSION_STR;
  return result;
}

/**
 * Versions of the MobilityDB extension and its dependencies
 */
char *
mobilitydb_full_version(void)
{
  char *result = palloc(sizeof(char) * MOBDB_VERSION_STR_MAXLEN);
  int len = snprintf(result, MOBDB_VERSION_STR_MAXLEN, "%s, %s, %s",
    MOBILITYDB_VERSION_STR, POSTGRESQL_VERSION_STRING, POSTGIS_VERSION_STR);
  result[len] = '\0';
  return result;
}


/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal value from its Well-Known Text (WKT) representation.
 *
 * @param[in] str String
 * @param[in] temptype Temporal type
 * @see tinstant_in
 * @see tinstantset_in
 * @see tsequence_in
 * @see tsequenceset_in
 */
Temporal *
temporal_in(char *str, mobdbType temptype)
{
  return temporal_parse(&str, temptype);
}

#if MEOS
/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return a temporal boolean from its Well-Known Text (WKT)
 * representation.
 */
Temporal *
tbool_in(char *str)
{
  return temporal_parse(&str, T_BOOL);
}

/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return a temporal integer from its Well-Known Text (WKT)
 * representation.
 */
Temporal *
tint_in(char *str)
{
  return temporal_parse(&str, T_INT4);
}

/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return a temporal float from its Well-Known Text (WKT) representation.
 */
Temporal *
tfloat_in(char *str)
{
  return temporal_parse(&str, T_FLOAT8);
}

/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return a temporal text from its Well-Known Text (WKT) representation.
 */
Temporal *
ttext_in(char *str)
{
  return temporal_parse(&str, T_TTEXT);
}

/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return a temporal geometric point from its Well-Known Text (WKT)
 * representation.
 */
Temporal *
tgeompoint_in(char *str)
{
  return temporal_parse(&str, T_TGEOMPOINT);
}
/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return a temporal geographic point from its Well-Known Text (WKT)
 * representation.
 */
Temporal *
tgeogpoint_in(char *str)
{
  return temporal_parse(&str, T_TGEOGPOINT);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return the Well-Known Text (WKT) representation of a temporal value.
 * @see tinstant_out
 * @see tinstantset_out
 * @see tsequence_out
 * @see tsequenceset_out
 */
char *
temporal_out(const Temporal *temp)
{
  char *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_out((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_out((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_out((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_out((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Return a copy of a temporal value
 * @see tinstant_copy
 * @see tinstantset_copy
 * @see tsequence_copy
 * @see tsequenceset_copy
 */
Temporal *
temporal_copy(const Temporal *temp)
{
  Temporal *result = palloc(VARSIZE(temp));
  memcpy(result, temp, VARSIZE(temp));
  return result;
}

/**
 * @ingroup libmeos_int_temporal_constructor
 * @brief Construct a temporal value from a base value and the time frame of
 * another temporal value.
 * @see tinstantset_from_base
 * @see tsequence_from_base
 * @see tsequenceset_from_base
 */
Temporal *
temporal_from_base(Datum value, mobdbType temptype, const Temporal *temp,
  bool linear)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
  {
    TInstant *inst = (TInstant *) temp;
    result = (Temporal *) tinstant_make(value, temptype, inst->t);
  }
  else if (temp->subtype == TINSTANTSET)
  {
    TInstantSet *ti = (TInstantSet *) temp;
    int count;
    TimestampTz *times = tinstantset_timestamps(ti, &count);
    TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
    for (int i = 0; i < ti->count; i++)
      instants[i] = tinstant_make(value, temptype, times[i]);
    result = (Temporal *) tinstantset_make_free(instants, ti->count, MERGE_NO);
    pfree(times);
  }
  else if (temp->subtype == TSEQUENCE)
  {
    TSequence *seq = (TSequence *) temp;
    result = (Temporal *) tsequence_from_base(value, temptype, &seq->period,
      linear);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    TSequenceSet *ts = (TSequenceSet *) temp;
    TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
    for (int i = 0; i < ts->count; i++)
    {
      const TSequence *seq = tsequenceset_seq_n(ts, i);
      sequences[i] = tsequence_from_base(value, temptype, &seq->period,
        linear);
    }
    result = (Temporal *) tsequenceset_make_free(sequences, ts->count,
      NORMALIZE_NO);
  }
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean from a boolean and the time frame of
 * another temporal value.
 */
Temporal *
tbool_from_base(bool b, const Temporal *temp)
{
  return temporal_from_base(BoolGetDatum(b), T_TBOOL, temp, false);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer from an integer and the time frame of
 * another temporal value.
 */
Temporal *
tint_from_base(int i, const Temporal *temp)
{
  return temporal_from_base(Int32GetDatum(i), T_TINT, temp, false);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float from a float and the time frame of
 * another temporal value.
 */
Temporal *
tfloat_from_base(bool b, const Temporal *temp, bool linear)
{
  return temporal_from_base(BoolGetDatum(b), T_TFLOAT, temp, linear);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text from a text and the time frame of
 * another temporal value.
 */
Temporal *
ttext_from_base(text *txt, const Temporal *temp)
{
  return temporal_from_base(PointerGetDatum(txt), T_TTEXT, temp, false);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point from a point and the time frame
 * of another temporal value.
 */
Temporal *
tgeompoint_from_base(GSERIALIZED *gs, const Temporal *temp, bool linear)
{
  return temporal_from_base(PointerGetDatum(gs), T_TGEOMPOINT, temp, linear);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point from a point and the time frame
 * of another temporal value.
 */
Temporal *
tgeogpoint_from_base(GSERIALIZED *gs, const Temporal *temp, bool linear)
{
  return temporal_from_base(PointerGetDatum(gs), T_TGEOGPOINT, temp, linear);
}
#endif /* MEOS */

/*****************************************************************************
 * Append and merge functions
 ****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Append an instant to the end of a temporal value.
 * @see tinstant_merge
 * @see tinstantset_append_tinstant
 * @see tsequence_append_tinstant
 * @see tsequenceset_append_tinstant
 * @sqlfunc appendInstant()
 */
Temporal *
temporal_append_tinstant(const Temporal *temp, const TInstant *inst)
{
  /* Validity tests */
  if (inst->subtype != TINSTANT)
    elog(ERROR, "The second argument must be of instant subtype");
  ensure_same_temptype(temp, (Temporal *) inst);
  /* The test to ensure the increasing timestamps must be done in the
   * specific function since the inclusive/exclusive bounds must be
   * taken into account for temporal sequences and sequence sets */
  ensure_spatial_validity(temp, (const Temporal *) inst);

  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_merge((TInstant *) temp, inst);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_append_tinstant((TInstantSet *) temp,
      inst);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_append_tinstant((TSequence *) temp, inst);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_append_tinstant((TSequenceSet *) temp,
      inst);
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
  if (new1->subtype == TINSTANT)
  {
    if (new2->subtype == TINSTANTSET)
      new = (Temporal *) tinstant_to_tinstantset((TInstant *) new1);
    else if (new2->subtype == TSEQUENCE)
      new = (Temporal *) tinstant_to_tsequence((TInstant *) new1,
        MOBDB_FLAGS_GET_CONTINUOUS(new1->flags));
    else /* new2->subtype == TSEQUENCESET */
      new = (Temporal *) tinstant_to_tsequenceset((TInstant *) new1,
        MOBDB_FLAGS_GET_CONTINUOUS(new1->flags));
  }
  else if (new1->subtype == TINSTANTSET)
  {
    if (new2->subtype == TSEQUENCE)
    {
      if (((TInstantSet *) new1)->count == 1)
        new = (Temporal *) tinstantset_to_tsequence((TInstantSet *) new1,
          MOBDB_FLAGS_GET_CONTINUOUS(new1->flags));
      else /* new2->subtype == TSEQUENCESET */
      {
        new = (Temporal *) tinstantset_to_tsequenceset((TInstantSet *) new1,
          MOBDB_FLAGS_GET_CONTINUOUS(new1->flags));
        newts = (Temporal *) tsequence_to_tsequenceset((TSequence *) new2);
      }
    }
    else /* new2->subtype == TSEQUENCESET */
      new = (Temporal *) tinstantset_to_tsequenceset((TInstantSet *) new1,
        MOBDB_FLAGS_GET_CONTINUOUS(new1->flags));
  }
  else /* new1->subtype == TSEQUENCE && new2->subtype == TSEQUENCESET */
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

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge two temporal values.
 *
 * @result Merged value. Return NULL if both arguments are NULL.
 * If one argument is null the other argument is output.
 * @see tinstant_merge
 * @see tinstantset_merge
 * @see tsequence_merge
 * @see tsequenceset_merge
 * @sqlfunc merge()
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
  if (new1->subtype == TINSTANT)
    result = tinstant_merge((TInstant *) new1, (TInstant *) new2);
  else if (new1->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_merge((TInstantSet *) new1,
      (TInstantSet *) new2);
  else if (new1->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_merge((TSequence *) new1,
      (TSequence *) new2);
  else /* new1->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_merge((TSequenceSet *) new1,
      (TSequenceSet *) new2);
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
    else if (subtype1 == TINSTANT)
    {
      if (subtype == TINSTANTSET)
        result[i] = (Temporal *) tinstant_to_tinstantset((TInstant *) temparr[i]);
      else if (subtype == TSEQUENCE)
        result[i] = (Temporal *) tinstant_to_tsequence((TInstant *) temparr[i],
          MOBDB_FLAGS_GET_LINEAR(temparr[i]->flags));
      else /* subtype == TSEQUENCESET */
        result[i] = (Temporal *) tinstant_to_tsequenceset((TInstant *) temparr[i],
          MOBDB_FLAGS_GET_LINEAR(temparr[i]->flags));
    }
    else if (subtype1 == TINSTANTSET)
    {
      /* An instant set can only be converted to a sequence set */
      assert(subtype == TSEQUENCESET);
      result[i] = (Temporal *) tinstantset_to_tsequenceset((TInstantSet *) temparr[i],
        MOBDB_FLAGS_GET_LINEAR(temparr[i]->flags));
    }
    else /* subtype1 == TSEQUENCE && subtype == TSEQUENCESET */
      result[i] = (Temporal *) tsequence_to_tsequenceset((TSequence *) temparr[i]);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge an array of temporal values.
 * @sqlfunc merge()
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
      elog(ERROR, "Input values must be of the same interpolation");
    }
    uint8 subtype1 = temparr[i]->subtype;
    if (subtype != subtype1)
    {
      /* A TInstantSet cannot be converted to a TSequence */
      int16 newsubtype = Max(subtype, subtype1);
      if (subtype == TINSTANTSET && newsubtype == TSEQUENCE)
        newsubtype = TSEQUENCESET;
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
  if (subtype == TINSTANT)
    result = (Temporal *) tinstant_merge_array(
      (const TInstant **) newtemps, count);
  else if (subtype == TINSTANTSET)
    result = tinstantset_merge_array(
      (const TInstantSet **) newtemps, count);
  else if (subtype == TSEQUENCE)
    result = (Temporal *) tsequence_merge_array(
      (const TSequence **) newtemps, count);
  else /* subtype == TSEQUENCESET */
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
 * @brief Cast a temporal integer to a temporal float.
 * @see tintinst_to_tfloatinst
 * @see tintinstset_to_tfloatinstset
 * @see tintseq_to_tfloatseq
 * @see tintseqset_to_tfloatseqset
 * @sqlop @p ::
 */
Temporal *
tint_to_tfloat(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tintinst_to_tfloatinst((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tintinstset_to_tfloatinstset((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tintseq_to_tfloatseq((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tintseqset_to_tfloatseqset((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal float to a temporal integer.
 * @see tfloatinst_to_tintinst
 * @see tfloatinstset_to_tintinstset
 * @see tfloatseq_to_tintseq
 * @see tfloatseqset_to_tintseqset
 * @sqlop @p ::
 */
Temporal *
tfloat_to_tint(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tfloatinst_to_tintinst((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tfloatinstset_to_tintinstset((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tfloatseq_to_tintseq((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tfloatseqset_to_tintseqset((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_cast
 * @brief Set a period to the bounding period of a temporal value.
 * @see tinstant_set_period
 * @see tinstantset_set_period
 * @see tsequence_set_period
 * @see tsequenceset_set_period
 */
void
temporal_set_period(const Temporal *temp, Period *p)
{
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    tinstant_set_period((TInstant *) temp, p);
  else if (temp->subtype == TINSTANTSET)
    tinstantset_set_period((TInstantSet *) temp, p);
  else if (temp->subtype == TSEQUENCE)
    tsequence_set_period((TSequence *) temp, p);
  else /* temp->subtype == TSEQUENCESET */
    tsequenceset_set_period((TSequenceSet *) temp, p);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_cast
 * @brief Return the bounding period of a temporal value.
 * @sqlfunc period()
 * @sqlop @p ::
 * @pymeosfunc period()
 */
Period *
temporal_to_period(const Temporal *temp)
{
  Period *result = palloc(sizeof(Period));
  temporal_set_period(temp, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_temporal_cast
 * @brief Return the value span of a temporal number.
 * @sqlfunc valueSpan()
 */
Span *
tnumber_to_span(const Temporal *temp)
{
  Span *result = NULL;
  mobdbType basetype = temptype_basetype(temp->temptype);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
  {
    Datum value = tinstant_value((TInstant *) temp);
    result = span_make(value, value, true, true, basetype);
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
    result = span_make(min, max, true, true, basetype);
  }
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_box_cast
 * @brief Return the bounding box of a temporal number.
 * @sqlop @p ::
 */
TBOX *
tnumber_to_tbox(Temporal *temp)
{
  TBOX *result = palloc(sizeof(TBOX));
  temporal_set_bbox(temp, result);
  return result;
}
#endif

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value transformed into a temporal instant.
 * @see tinstant_copy
 * @see tinstantset_to_tinstant
 * @see tsequence_to_tinstant
 * @see tsequenceset_to_tinstant
 * @sqlfunc tbool_inst(), tint_inst(), tfloat_inst(), ttext_inst(), etc.
 */
Temporal *
temporal_to_tinstant(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_copy((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_to_tinstant((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_to_tinstant((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_to_tinstant((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value transformed into a temporal instant set.
 * @see tinstant_to_tinstantset
 * @see tinstantset_copy
 * @see tsequence_to_tinstantset
 * @see tsequenceset_to_tinstantset
 * @sqlfunc tbool_instset(), tint_instset(), tfloat_instset(), ttext_instset(),
 * etc.
 */
Temporal *
temporal_to_tinstantset(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_to_tinstantset((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_copy((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_to_tinstantset((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_to_tinstantset((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value transformed into a temporal sequence.
 * @see tinstant_to_tsequence
 * @see tinstantset_to_tsequence
 * @see tsequence_copy
 * @see tsequenceset_to_tsequence
 * @sqlfunc tbool_seq(), tint_seq(), tfloat_seq(), ttext_seq(), etc.
 */
Temporal *
temporal_to_tsequence(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_to_tsequence((TInstant *) temp,
      MOBDB_FLAGS_GET_CONTINUOUS(temp->flags));
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_to_tsequence((TInstantSet *) temp,
      MOBDB_FLAGS_GET_CONTINUOUS(temp->flags));
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_copy((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_to_tsequence((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value transformed into a temporal sequence set.
 * @see tinstant_to_tsequenceset
 * @see tinstantset_to_tsequenceset
 * @see tsequence_to_tsequenceset
 * @see tsequenceset_copy
 * @sqlfunc tbool_seqset(), tint_seqset(), tfloat_seqset(), ttext_seqset(), etc.
 */
Temporal *
temporal_to_tsequenceset(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_to_tsequenceset((TInstant *) temp,
      MOBDB_FLAGS_GET_CONTINUOUS(temp->flags));
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_to_tsequenceset((TInstantSet *) temp,
      MOBDB_FLAGS_GET_CONTINUOUS(temp->flags));
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_copy((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value with continuous base type transformed from
 * stepwise to linear interpolation.
 * @see tsequence_step_to_linear
 * @see tsequenceset_step_to_linear
 * @sqlfunc toLinear()
 */
Temporal *
temporal_step_to_linear(const Temporal *temp)
{
  ensure_seq_subtypes(temp->subtype);
  ensure_temptype_continuous(temp->temptype);

  if (MOBDB_FLAGS_GET_LINEAR(temp->flags))
    return temporal_copy(temp);

  Temporal *result;
  if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_step_to_linear((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_step_to_linear((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value shifted and/or scaled by the intervals.
 *
 * @param[in] temp Temporal value
 * @param[in] shift True when a shift of the timespan must be performed
 * @param[in] tscale True when a scale of the timespan must be performed
 * @param[in] start Interval for shift
 * @param[in] duration Interval for scale
 * @pre The duration is greater than 0 if is not NULL
 * @see tinstant_shift
 * @see tinstantset_shift_tscale
 * @see tsequence_shift_tscale
 * @see tsequenceset_shift_tscale
 * @sqlfunc shift(), scale(), shiftTscale()
 * @pymeosfunc shift()
 */
Temporal *
temporal_shift_tscale(const Temporal *temp, bool shift, bool tscale,
  Interval *start, Interval *duration)
{
  assert((! shift || start != NULL) && (! tscale || duration != NULL));
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (start != NULL) ?
      (Temporal *) tinstant_shift((TInstant *) temp, start) :
      (Temporal *) tinstant_copy((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_shift_tscale((TInstantSet *) temp,
    start, duration);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_shift_tscale((TSequence *) temp,
      start, duration);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_shift_tscale((TSequenceSet *) temp,
      start, duration);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#define MOBDB_SUBTYPE_STR_MAXLEN 12
#define MOBDB_INTERPOLATION_STR_MAXLEN 12

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the string representation of the subtype of a temporal value.
 * @sqlfunc tempSubtype();
 * @pymeosfunc tempSubtype()
 */
char *
temporal_subtype(const Temporal *temp)
{
  char *result = palloc(sizeof(char) * MOBDB_SUBTYPE_STR_MAXLEN);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    strcpy(result, "Instant");
  else if (temp->subtype == TINSTANTSET)
    strcpy(result, "InstantSet");
  else if (temp->subtype == TSEQUENCE)
    strcpy(result, "Sequence");
  else /* temp->subtype == TSEQUENCESET */
    strcpy(result, "SequenceSet");
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the string representation of the interpolation of a temporal
 * value.
 * @sqlfunc interpolation()
 * @pymeosfunc interpolation()
 */
char *
temporal_interpolation(const Temporal *temp)
{
  char *result = palloc(sizeof(char) * MOBDB_INTERPOLATION_STR_MAXLEN);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT || temp->subtype == TINSTANTSET)
    strcpy(result, "Discrete");
  else if (temp->subtype == TSEQUENCE || temp->subtype == TSEQUENCESET)
  {
    if (MOBDB_FLAGS_GET_LINEAR(temp->flags))
      strcpy(result, "Linear");
    else
      strcpy(result, "Stepwise");
  }
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Set the second argument to the bounding box of a temporal value
 * @see tinstant_set_bbox
 * @see tinstantset_set_bbox
 * @see tsequence_set_bbox
 * @see tsequenceset_set_bbox
 * @note For temporal instants the bounding box must be computed.
 * For the other subtypes a copy of the precomputed bounding box is made.
 * @sqlfunc period(), tbox(), stbox()
 * @sqlop @p ::
 */
void
temporal_set_bbox(const Temporal *temp, void *box)
{
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    tinstant_set_bbox((TInstant *) temp, box);
  else if (temp->subtype == TINSTANTSET)
    tinstantset_set_bbox((TInstantSet *) temp, box);
  else if (temp->subtype == TSEQUENCE)
    tsequence_set_bbox((TSequence *) temp, box);
  else /* temp->subtype == TSEQUENCESET */
    tsequenceset_set_bbox((TSequenceSet *) temp, box);
  return;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of base values of a temporal value.
 * @see tinstant_values
 * @see tinstantset_values
 * @see tsequence_values
 * @see tsequenceset_values
 * @sqlfunc values()
 */
Datum *
temporal_values(const Temporal *temp, int *count)
{
  Datum *result;  /* make the compiler quiet */
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_values((TInstant *) temp, count);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_values((TInstantSet *) temp, count);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_values((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_values((TSequenceSet *) temp, count);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the the array of base values of a temporal boolean
 * @sqlfunc values()
 * @pymeosfunc getValues()
 */
bool *
tbool_values(Temporal *temp)
{
  int count;
  Datum *datumarr = temporal_values(temp, &count);
  bool *result = palloc(sizeof(bool) * count);
  for (int i = 0; i < count; i++)
    result[i] = DatumGetBool(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the the array of base values of a temporal integer
 * @sqlfunc values()
 * @pymeosfunc getValues()
 */
int *
tint_values(Temporal *temp)
{
  int count;
  Datum *datumarr = temporal_values(temp, &count);
  int *result = palloc(sizeof(int) * count);
  for (int i = 0; i < count; i++)
    result[i] = DatumGetInt32(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the the array of base values of a temporal float
 * @sqlfunc values()
 * @pymeosfunc getValues()
 */
double *
tfloat_values(Temporal *temp)
{
  int count;
  Datum *datumarr = temporal_values(temp, &count);
  double *result = palloc(sizeof(double) * count);
  for (int i = 0; i < count; i++)
    result[i] = DatumGetFloat8(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the the array of base values of a temporal text
 * @sqlfunc values()
 * @pymeosfunc getValues()
 */
text **
ttext_values(Temporal *temp)
{
  int count;
  Datum *datumarr = temporal_values(temp, &count);
  text **result = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    result[i] = DatumGetTextP(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the the array of base values of a temporal geometric point
 * @sqlfunc values()
 * @pymeosfunc getValues()
 */
GSERIALIZED **
tpoint_values(Temporal *temp)
{
  int count;
  Datum *datumarr = temporal_values(temp, &count);
  GSERIALIZED **result = palloc(sizeof(GSERIALIZED *) * count);
  for (int i = 0; i < count; i++)
    result[i] = DatumGetGserializedP(datumarr[i]);
  pfree(datumarr);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the base values of a temporal float as an array of spans.
 * @see tfloatinst_spans
 * @see tfloatinstset_spans
 * @see tfloatseq_spans
 * @see tfloatseqset_spans
 * @sqlfunc getValues()
 * @pymeosfunc TFloat.getValues()
 */
Span **
tfloat_spans(const Temporal *temp, int *count)
{
  Span **result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tfloatinst_spans((TInstant *) temp, count);
  else if (temp->subtype == TINSTANTSET)
    result = tfloatinstset_spans((TInstantSet *) temp, count);
  else if (temp->subtype == TSEQUENCE)
    result = tfloatseq_spans((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tfloatseqset_spans((TSequenceSet *) temp, count);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the time frame of a temporal value as a period set.
 * @see tinstant_time
 * @see tinstantset_time
 * @see tsequence_time
 * @see tsequenceset_time
 * @sqlfunc getTime()
 * @pymeosfunc getTime()
 */
PeriodSet *
temporal_time(const Temporal *temp)
{
  PeriodSet *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_time((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_time((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_time((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_time((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the start base value of a temporal value
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
Datum
temporal_start_value(Temporal *temp)
{
  Datum result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = tinstant_value_copy(tinstantset_inst_n((TInstantSet *) temp, 0));
  else if (temp->subtype == TSEQUENCE)
    result = tinstant_value_copy(tsequence_inst_n((TSequence *) temp, 0));
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp, 0);
    result = tinstant_value_copy(tsequence_inst_n(seq, 0));
  }
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal boolean
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
bool
tbool_start_value(Temporal *temp)
{
  return DatumGetBool(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal integer
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
int
tint_start_value(Temporal *temp)
{
  return DatumGetInt32(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal float
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
double
tfloat_start_value(Temporal *temp)
{
  return DatumGetFloat8(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal text
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
text *
ttext_start_value(Temporal *temp)
{
  return DatumGetTextP(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal geometric point
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
GSERIALIZED *
tpoint_start_value(Temporal *temp)
{
  return DatumGetGserializedP(temporal_start_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the end base value of a temporal value
 */
Datum
temporal_end_value(Temporal *temp)
{
  Datum result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = tinstant_value_copy(tinstantset_inst_n((TInstantSet *) temp,
      ((TInstantSet *) temp)->count - 1));
  else if (temp->subtype == TSEQUENCE)
    result = tinstant_value_copy(tsequence_inst_n((TSequence *) temp,
      ((TSequence *) temp)->count - 1));
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp,
      ((TSequenceSet *) temp)->count - 1);
    result = tinstant_value_copy(tsequence_inst_n(seq, seq->count - 1));
  }
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal boolean
 * @sqlfunc endValue()
 * @pymeosfunc endValue()
 */
bool
tbool_end_value(Temporal *temp)
{
  return DatumGetBool(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal integer
 * @sqlfunc endValue()
 * @pymeosfunc endValue()
 */
int
tint_end_value(Temporal *temp)
{
  return DatumGetInt32(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal float
 * @sqlfunc endValue()
 * @pymeosfunc endValue()
 */
double
tfloat_end_value(Temporal *temp)
{
  return DatumGetFloat8(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal text
 * @sqlfunc endValue()
 * @pymeosfunc endValue()
 */
text *
ttext_end_value(Temporal *temp)
{
  return DatumGetTextP(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal point
 * @sqlfunc endValue()
 * @pymeosfunc endValue()
 */
GSERIALIZED *
tpoint_end_value(Temporal *temp)
{
  return DatumGetGserializedP(temporal_end_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return a copy of the minimum base value of a temporal value
 */
Datum
temporal_min_value(const Temporal *temp)
{
  Datum result;
  mobdbType basetype = temptype_basetype(temp->temptype);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = datum_copy(tinstantset_min_value((TInstantSet *) temp), basetype);
  else if (temp->subtype == TSEQUENCE)
    result = datum_copy(tsequence_min_value((TSequence *) temp), basetype);
  else /* temp->subtype == TSEQUENCESET */
    result = datum_copy(tsequenceset_min_value((TSequenceSet *) temp), basetype);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the minimum value of a temporal integer
 * @sqlfunc minValue()
 * @pymeosfunc minValue()
 */
int
tint_min_value(Temporal *temp)
{
  return DatumGetInt32(temporal_min_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the minimum value of a temporal float
 * @sqlfunc minValue()
 * @pymeosfunc minValue()
 */
double
tfloat_min_value(Temporal *temp)
{
  return DatumGetFloat8(temporal_min_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the minimum value of a temporal text
 * @sqlfunc minValue()
 * @pymeosfunc minValue()
 */
text *
ttext_min_value(Temporal *temp)
{
  return DatumGetTextP(temporal_min_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return a copy of the maximum base value of a temporal value.
 * @sqlfunc maxValue()
 * @pymeosfunc maxValue()
 */
Datum
temporal_max_value(const Temporal *temp)
{
  Datum result;
  mobdbType basetype = temptype_basetype(temp->temptype);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = datum_copy(tinstantset_max_value((TInstantSet *) temp), basetype);
  else if (temp->subtype == TSEQUENCE)
    result = datum_copy(tsequence_max_value((TSequence *) temp), basetype);
  else /* temp->subtype == TSEQUENCESET */
    result = datum_copy(tsequenceset_max_value((TSequenceSet *) temp), basetype);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum value of a temporal integer
 * @sqlfunc maxValue()
 * @pymeosfunc maxValue()
 */
int
tint_max_value(Temporal *temp)
{
  return DatumGetInt32(temporal_max_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum value of a temporal float
 * @sqlfunc maxValue()
 * @pymeosfunc maxValue()
 */
double
tfloat_max_value(Temporal *temp)
{
  return DatumGetFloat8(temporal_max_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum value of a temporal text
 * @sqlfunc maxValue()
 * @pymeosfunc maxValue()
 */
text *
ttext_max_value(Temporal *temp)
{
  return DatumGetTextP(temporal_max_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of a
 * temporal value.
 *
 * @note The function does not take into account whether the instant is at
 * an exclusive bound or not.
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance
 * @see tinstantset_min_instant
 * @see tsequence_min_instant
 * @see tsequenceset_min_instant
 * @sqlfunc minInstant()
 */
const TInstant *
temporal_min_instant(const Temporal *temp)
{
  const TInstant *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_min_instant((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_min_instant((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_min_instant((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with maximum base value of a
 * temporal value.
 * @see tinstantset_max_instant
 * @see tsequence_min_instant
 * @see tsequenceset_max_instant
 * @sqlfunc maxInstant()
 */
const TInstant *
temporal_max_instant(const Temporal *temp)
{
  const TInstant *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_max_instant((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_max_instant((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_max_instant((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the timespan of a temporal value.
 * @see tinstantset_timespan
 * @see tsequence_duration
 * @see tsequenceset_timespan
 * @sqlfunc timespan()
 * @pymeosfunc timespan()
 */
Interval *
temporal_timespan(const Temporal *temp)
{
  Interval *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = palloc0(sizeof(Interval));
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_timespan((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_duration((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_timespan((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the duration of a temporal value.
 * @see tsequence_duration
 * @see tsequenceset_duration
 * @sqlfunc duration()
 * @pymeosfunc duration()
 */
Interval *
temporal_duration(const Temporal *temp)
{
  Interval *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT || temp->subtype == TINSTANTSET)
    result = palloc0(sizeof(Interval));
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_duration((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_duration((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the number of sequences of a temporal sequence (set).
 * @sqlfunc numSequences()
 */
int
temporal_num_sequences(const Temporal *temp)
{
  ensure_seq_subtypes(temp->subtype);
  int result = 1;
  if (temp->subtype == TSEQUENCESET)
    result = ((TSequenceSet *) temp)->count;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start sequence of a temporal sequence (set).
 * @sqlfunc startSequence()
 */
TSequence *
temporal_start_sequence(const Temporal *temp)
{
  ensure_seq_subtypes(temp->subtype);
  TSequence *result;
  if (temp->subtype == TSEQUENCE)
    result = tsequence_copy((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequenceSet *ts = (const TSequenceSet *) temp;
    result = tsequence_copy(tsequenceset_seq_n(ts, 0));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end sequence of a temporal sequence (set).
 * @sqlfunc endSequence()
 */
TSequence *
temporal_end_sequence(const Temporal *temp)
{
  ensure_seq_subtypes(temp->subtype);
  TSequence *result;
  if (temp->subtype == TSEQUENCE)
    result = tsequence_copy((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequenceSet *ts = (const TSequenceSet *) temp;
    result = tsequence_copy(tsequenceset_seq_n(ts, ts->count - 1));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th sequence of a temporal sequence (set).
 *
 * @note n is assumed to be 1-based.
 * @sqlfunc sequenceN()
 */
TSequence *
temporal_sequence_n(const Temporal *temp, int i)
{
  ensure_seq_subtypes(temp->subtype);
  TSequence *result = NULL;
  if (temp->subtype == TSEQUENCE)
  {
    if (i == 1)
      result = tsequence_copy((TSequence *) temp);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequenceSet *ts = (const TSequenceSet *) temp;
    if (i >= 1 && i <= ts->count)
      result = tsequence_copy(tsequenceset_seq_n(ts, i - 1));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of sequences of a temporal sequence (set).
 * @see tinstant_sequences
 * @see tinstantset_sequences
 * @see tsequence_sequences
 * @see tsequenceset_sequences
 * @sqlfunc sequences()
 */
TSequence **
temporal_sequences(const Temporal *temp, int *count)
{
  TSequence **result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_sequences((TInstant *) temp, count);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_sequences((TInstantSet *) temp, count);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_sequences((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCE */
    result = tsequenceset_sequences((TSequenceSet *) temp, count);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of segments of a temporal value.
 * @see tinstant_sequences
 * @see tinstantset_sequences
 * @see tsequence_segments
 * @see tsequenceset_segments
 * @sqlfunc segments()
 */
TSequence **
temporal_segments(const Temporal *temp, int *count)
{
  TSequence **result;
  if (temp->subtype == TINSTANT)
    result = tinstant_sequences((TInstant *) temp, count);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_sequences((TInstantSet *) temp, count);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_segments((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_segments((TSequenceSet *) temp, count);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the number of distinct instants of a temporal value.
 * @sqlfunc numInstants()
 * @pymeosfunc numInstants()
 */
int
temporal_num_instants(const Temporal *temp)
{
  int result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = 1;
  else if (temp->subtype == TINSTANTSET)
    result = ((TInstantSet *) temp)->count;
  else if (temp->subtype == TSEQUENCE)
    result = ((TSequence *) temp)->count;
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_num_instants((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start instant of a temporal value.
 * @sqlfunc startInstant()
 * @pymeosfunc startInstant()
 */
const TInstant *
temporal_start_instant(const Temporal *temp)
{
  const TInstant *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_inst_n((TInstantSet *) temp, 0);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_inst_n((TSequence *) temp, 0);
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp, 0);
    result = tsequence_inst_n(seq, 0);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end instant of a temporal value.
 * @note This function is used for validity testing.
 * @sqlfunc endInstant()
 * @pymeosfunc endInstant()
 */
const TInstant *
temporal_end_instant(const Temporal *temp)
{
  const TInstant *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_inst_n((TInstantSet *) temp,
      ((TInstantSet *) temp)->count - 1);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_inst_n((TSequence *) temp,
      ((TSequence *) temp)->count - 1);
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequence *seq = tsequenceset_seq_n((TSequenceSet *) temp,
      ((TSequenceSet *) temp)->count - 1);
    result = tsequence_inst_n(seq, seq->count - 1);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th instant of a temporal value.
 * @note n is assumed 1-based
 * @sqlfunc instantN()
 * @pymeosfunc instantN()
 */
const TInstant *
temporal_instant_n(Temporal *temp, int n)
{
  const TInstant *result = NULL;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
  {
    if (n == 1)
      result = (const TInstant *) temp;
  }
  else if (temp->subtype == TINSTANTSET)
  {
    if (n >= 1 && n <= ((TInstantSet *) temp)->count)
      result = tinstantset_inst_n((TInstantSet *) temp, n - 1);
  }
  else if (temp->subtype == TSEQUENCE)
  {
    if (n >= 1 && n <= ((TSequence *) temp)->count)
      result = tsequence_inst_n((TSequence *) temp, n - 1);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    /* This test is necessary since the n-th DISTINCT instant is requested */
    if (n >= 1 && n <= ((TSequenceSet *) temp)->totalcount)
      result = tsequenceset_inst_n((TSequenceSet *) temp, n);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of instants of a temporal value.
 * @see tinstant_instants
 * @see tinstantset_instants
 * @see tsequence_instants
 * @see tsequenceset_instants
 * @sqlfunc instants()
 * @pymeosfunc instants()
 */
const TInstant **
temporal_instants(const Temporal *temp, int *count)
{
  const TInstant **result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_instants((TInstant *) temp, count);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_instants((TInstantSet *) temp, count);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_instants((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_instants((TSequenceSet *) temp, count);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the number of distinct timestamps of a temporal value.
 * @sqlfunc numTimestamps()
 * @pymeosfunc numTimestamps()
 */
int
temporal_num_timestamps(const Temporal *temp)
{
  int result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = 1;
  else if (temp->subtype == TINSTANTSET)
    result = ((TInstantSet *) temp)->count;
  else if (temp->subtype == TSEQUENCE)
    result = ((TSequence *) temp)->count;
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_num_timestamps((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start timestamp of a temporal value.
 * @sqlfunc startTimestamp()
 * @pymeosfunc startTimestamp()
 */
TimestampTz
temporal_start_timestamp(const Temporal *temp)
{
  TimestampTz result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = ((TInstant *) temp)->t;
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_inst_n((TInstantSet *) temp, 0)->t;
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_start_timestamp((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_start_timestamp((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end timestamp of a temporal value.
 * @sqlfunc endTimestamp()
 * @pymeosfunc endTimestamp()
 */
TimestampTz
temporal_end_timestamp(Temporal *temp)
{
  TimestampTz result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = ((TInstant *) temp)->t;
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_inst_n((TInstantSet *) temp, ((TInstantSet *) temp)->count - 1)->t;
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_end_timestamp((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_end_timestamp((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th distinct timestamp of a temporal value.
 * @note n is assumed 1-based
 * @sqlfunc timestampN()
 * @pymeosfunc timestampN()
 */
bool
temporal_timestamp_n(Temporal *temp, int n, TimestampTz *result)
{
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
  {
    if (n == 1)
    {
      *result = ((TInstant *) temp)->t;
      return true;
    }
  }
  else if (temp->subtype == TINSTANTSET)
  {
    if (n >= 1 && n <= ((TInstantSet *) temp)->count)
    {
      *result = (tinstantset_inst_n((TInstantSet *) temp, n - 1))->t;
      return true;
    }
  }
  else if (temp->subtype == TSEQUENCE)
  {
    if (n >= 1 && n <= ((TSequence *) temp)->count)
    {
      *result = (tsequence_inst_n((TSequence *) temp, n - 1))->t;
      return true;
    }
  }
  else /* temp->subtype == TSEQUENCESET */
    return tsequenceset_timestamp_n((TSequenceSet *) temp, n, result);
  return false;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of distinct timestamps of a temporal value.
 * @see tinstant_timestamps
 * @see tinstantset_timestamps
 * @see tsequence_timestamps
 * @sqlfunc timestamps()
 * @pymeosfunc timestamps()
 */
TimestampTz *
temporal_timestamps(const Temporal *temp, int *count)
{
  TimestampTz *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_timestamps((TInstant *) temp, count);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_timestamps((TInstantSet *) temp, count);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_timestamps((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_timestamps((TSequenceSet *) temp, count);
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
 * Return true if the bounding box of a temporal value is ever/always equal
 * to a base value
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
    temporal_set_bbox(temp, &box);
    double d = datum_double(value, temptype_basetype(temp->temptype));
    return (ever && box.xmin <= d && d <= box.xmax) ||
      (!ever && box.xmin == d && d == box.xmax);
  }
  else if (tspatial_type(temp->temptype))
  {
    STBOX box1, box2;
    temporal_set_bbox(temp, &box1);
    if (tgeo_type(temp->temptype))
      geo_set_stbox(DatumGetGserializedP(value), &box2);
#if NPOINT
    else if (temp->temptype == T_TNPOINT)
    {
      GSERIALIZED *geom = npoint_geom(DatumGetNpointP(value));
      geo_set_stbox(geom, &box2);
      pfree(geom);
    }
#endif
    return (ever && contains_stbox_stbox(&box1, &box2)) ||
      (!ever && same_stbox_stbox(&box1, &box2));
  }
  return true;
}

/**
 * Return true if the bounding box of a temporal value is ever/always less
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
    temporal_set_bbox(temp, &box);
    double d = datum_double(value, temptype_basetype(temp->temptype));
    if ((ever && d < box.xmin) || (!ever && d < box.xmax))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal value is ever equal to a base value.
 * @see tinstant_ever_eq
 * @see tinstantset_ever_eq
 * @see tsequence_ever_eq
 * @see tsequenceset_ever_eq
 */
bool
temporal_ever_eq(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_ever_eq((TInstant *) temp, value);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_ever_eq((TInstantSet *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_ever_eq((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_ever_eq((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal boolean is ever equal to a boolean.
 * @sqlop @p ?=
 */
bool tbool_ever_eq(const Temporal *temp, bool b)
{
  return temporal_ever_eq(temp, BoolGetDatum(b));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is ever equal to an integer.
 * @sqlop @p ?=
 */
bool tint_ever_eq(const Temporal *temp, int i)
{
  return temporal_ever_eq(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is ever equal to a float.
 * @sqlop @p ?=
 */
bool tfloat_ever_eq(const Temporal *temp, double d)
{
  return temporal_ever_eq(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is ever equal to a text.
 * @sqlop @p ?=
 */
bool ttext_ever_eq(const Temporal *temp, text *txt)
{
  return temporal_ever_eq(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal value is always equal to a base value.
 * @see tinstant_always_eq
 * @see tinstantset_always_eq
 * @see tsequence_always_eq
 * @see tsequenceset_always_eq
 */
bool
temporal_always_eq(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_always_eq((TInstant *) temp, value);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_always_eq((TInstantSet *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_always_eq((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_always_eq((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal boolean is always equal to a boolean.
 * @sqlop @p %=
 */
bool tbool_always_eq(const Temporal *temp, bool b)
{
  return temporal_always_eq(temp, BoolGetDatum(b));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is always equal to an integer.
 * @sqlop @p %=
 */
bool tint_always_eq(const Temporal *temp, int i)
{
  return temporal_always_eq(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is always equal to a float.
 * @sqlop @p %=
 */
bool tfloat_always_eq(const Temporal *temp, double d)
{
  return temporal_always_eq(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is always equal to a text.
 * @sqlop @p %=
 */
bool ttext_always_eq(const Temporal *temp, text *txt)
{
  return temporal_always_eq(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal value is ever less than a base value.
 * @see tinstant_ever_lt
 * @see tinstantset_ever_lt
 * @see tsequence_ever_lt
 * @see tsequenceset_ever_lt
 */
bool
temporal_ever_lt(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_ever_lt((TInstant *) temp, value);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_ever_lt((TInstantSet *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_ever_lt((TSequence *) temp, value);
  else /* subtype == TSEQUENCESET */
    result = tsequenceset_ever_lt((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is ever less than an integer.
 * @sqlop @p ?<
 */
bool tint_ever_lt(const Temporal *temp, int i)
{
  return temporal_ever_lt(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is ever less than a float.
 * @sqlop @p ?<
 */
bool tfloat_ever_lt(const Temporal *temp, double d)
{
  return temporal_ever_lt(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is ever less than a text.
 * @sqlop @p ?<
 */
bool ttext_ever_lt(const Temporal *temp, text *txt)
{
  return temporal_ever_lt(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal value is always less than a base value.
 * @see tinstant_always_lt
 * @see tinstantset_always_lt
 * @see tsequence_always_lt
 * @see tsequenceset_always_lt
 */
bool
temporal_always_lt(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_always_lt((TInstant *) temp, value);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_always_lt((TInstantSet *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_always_lt((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_always_lt((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is always less than an integer.
 * @sqlop @p %<
 */
bool tint_always_lt(const Temporal *temp, int i)
{
  return temporal_always_lt(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is always less than  a float.
 * @sqlop @p %<
 */
bool tfloat_always_lt(const Temporal *temp, double d)
{
  return temporal_always_lt(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is always less than  a text.
 * @sqlop @p %<
 */
bool ttext_always_lt(const Temporal *temp, text *txt)
{
  return temporal_always_lt(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal value is ever less than or equal to a
 * base value.
 * @see tinstant_ever_le
 * @see tinstantset_ever_le
 * @see tsequence_ever_le
 * @see tsequenceset_ever_le
 */
bool
temporal_ever_le(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_ever_le((TInstant *) temp, value);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_ever_le((TInstantSet *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_ever_le((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_ever_le((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is ever less than or equal to an integer.
 * @sqlop @p ?<=
 */
bool tint_ever_le(const Temporal *temp, int i)
{
  return temporal_ever_le(temp, Int32GetDatum(i));

}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is ever less than or equal to a float.
 * @sqlop @p ?<=
 */
bool tfloat_ever_le(const Temporal *temp, double d)
{
  return temporal_ever_le(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is ever less than or equal to a text.
 * @sqlop @p ?<=
 */
bool ttext_ever_le(const Temporal *temp, text *txt)
{
  return temporal_ever_le(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal value is always less than or equal to a
 * base value.
 * @see tinstant_always_le
 * @see tinstantset_always_le
 * @see tsequence_always_le
 * @see tsequenceset_always_le
 */
bool
temporal_always_le(const Temporal *temp, Datum value)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_always_le((TInstant *) temp, value);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_always_le((TInstantSet *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_always_le((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_always_le((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is always less than or equal to an integer.
 * @sqlop @p %<=
 */
bool tint_always_le(const Temporal *temp, int i)
{
  return temporal_always_le(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is always less than or equal to a float.
 * @sqlop @p %<=
 */
bool tfloat_always_le(const Temporal *temp, double d)
{
  return temporal_always_le(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is always less than or equal to a text.
 * @sqlop @p %<=
 */
bool ttext_always_le(const Temporal *temp, text *txt)
{
  return temporal_always_le(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/*****************************************************************************
 * Bounding box tests for the restriction functions
 *****************************************************************************/

/**
 * Return true if the bounding box of a temporal value contains a base value
 */
bool
temporal_bbox_restrict_value(const Temporal *temp, Datum value)
{
  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    TBOX box1, box2;
    temporal_set_bbox(temp, &box1);
    number_set_tbox(value, temptype_basetype(temp->temptype), &box2);
    return contains_tbox_tbox(&box1, &box2);
  }
  if (tgeo_type(temp->temptype))
  {
    /* Test that the geometry is not empty */
    GSERIALIZED *gs = DatumGetGserializedP(value);
    ensure_point_type(gs);
    ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
    ensure_same_dimensionality_tpoint_gs(temp, gs);
    if (gserialized_is_empty(gs))
      return false;
    if (temp->subtype != TINSTANT)
    {
      STBOX box1, box2;
      temporal_set_bbox(temp, &box1);
      geo_set_stbox(gs, &box2);
      return contains_stbox_stbox(&box1, &box2);
    }
  }
  return true;
}

/**
 * Return the array of base values that are contained in the bounding box
 * of a temporal value.
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
  mobdbType basetype = temptype_basetype(temp->temptype);

  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    TBOX box1;
    temporal_set_bbox(temp, &box1);
    for (int i = 0; i < count; i++)
    {
      TBOX box2;
      number_set_tbox(values[i], basetype, &box2);
      if (contains_tbox_tbox(&box1, &box2))
        newvalues[k++] = values[i];
    }
  }
  if (tgeo_type(temp->temptype))
  {
    STBOX box1;
    temporal_set_bbox(temp, &box1);
    for (int i = 0; i < count; i++)
    {
      /* Test that the geometry is not empty */
      GSERIALIZED *gs = DatumGetGserializedP(values[i]);
      ensure_point_type(gs);
      ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
      ensure_same_dimensionality_tpoint_gs(temp, gs);
      if (! gserialized_is_empty(gs))
      {
        STBOX box2;
        geo_set_stbox(gs, &box2);
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
 * Return true if the bounding box of the temporal number overlaps the span
 * of base values
 */
bool
tnumber_bbox_restrict_span(const Temporal *temp, const Span *span)
{
  /* Bounding box test */
  assert(tnumber_type(temp->temptype));
  TBOX box1, box2;
  temporal_set_bbox(temp, &box1);
  span_set_tbox(span, &box2);
  return overlaps_tbox_tbox(&box1, &box2);
}

/**
 * Return the array of spans of base values that overlap with the bounding box
 * of a temporal value.
 *
 * @param[in] temp Temporal value
 * @param[in] spans Array of spans of base values
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 * @return Filtered array of spans.
 */
Span **
tnumber_bbox_restrict_spans(const Temporal *temp, Span **spans,
  int count, int *newcount)
{
  assert(tnumber_type(temp->temptype));
  Span **newspans = palloc(sizeof(Datum) * count);
  int k = 0;
  TBOX box1;
  temporal_set_bbox(temp, &box1);
  for (int i = 0; i < count; i++)
  {
    TBOX box2;
    span_set_tbox(spans[i], &box2);
    if (overlaps_tbox_tbox(&box1, &box2))
      newspans[k++] = spans[i];
  }
  if (k == 0)
  {
    *newcount = 0;
    pfree(newspans);
    return NULL;
  }
  Span **normspans = spanarr_normalize(newspans, k, newcount);
  pfree(newspans);
  return normspans;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a base value.
 *
 * @note This function does a bounding box test for the temporal types
 * different from instant. The singleton tests are done in the functions for
 * the specific temporal types.
 * @sqlfunc atValue(), minusValue()
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
      return (temp->subtype != TSEQUENCE) ?
        temporal_copy(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_value(
      (TInstant *) temp, value, atfunc);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_restrict_value(
      (TInstantSet *) temp, value, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_restrict_value((TSequence *) temp,
      value, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_value(
      (TSequenceSet *) temp, value, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) an array of base
 * values.
 * @sqlfunc atValues(), minusValues()
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
      return (temp->subtype != TSEQUENCE) ?
        temporal_copy(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_values(
      (TInstant *) temp, newvalues, newcount, atfunc);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_restrict_values(
      (TInstantSet *) temp, newvalues, newcount, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_restrict_values((TSequence *) temp,
      newvalues, newcount, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_values(
      (TSequenceSet *) temp, newvalues, newcount, atfunc);

  pfree(newvalues);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a span of base values.
 * @sqlfunc atSpan(), minusSpan()
 */
Temporal *
tnumber_restrict_span(const Temporal *temp, Span *span, bool atfunc)
{
  /* Bounding box test */
  if (! tnumber_bbox_restrict_span(temp, span))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->subtype != TSEQUENCE) ?
        temporal_copy(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tnumberinst_restrict_span((TInstant *) temp,
      span, atfunc);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tnumberinstset_restrict_span((TInstantSet *) temp,
      span, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tnumberseq_restrict_span((TSequence *) temp,
      span, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tnumberseqset_restrict_span((TSequenceSet *) temp,
      span, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) an array of spans
 * of base values.
 * @sqlfunc atSpans(), minusSpans()
 */
Temporal *
tnumber_restrict_spans(const Temporal *temp, Span **spans, int count,
  bool atfunc)
{
  /* Bounding box test */
  int newcount;
  Span **newspans = tnumber_bbox_restrict_spans(temp, spans, count,
    &newcount);
  if (newcount == 0)
  {
    if (atfunc)
      return NULL;
    else
      return (temp->subtype != TSEQUENCE) ?
        temporal_copy(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }
  if (newcount == 1)
    return tnumber_restrict_span(temp, newspans[0], atfunc);

  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tnumberinst_restrict_spans((TInstant *) temp,
      newspans, newcount, atfunc);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tnumberinstset_restrict_spans((TInstantSet *) temp,
      newspans, newcount, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tnumberseq_restrict_spans((TSequence *) temp,
        newspans, newcount, atfunc, BBOX_TEST_NO);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tnumberseqset_restrict_spans((TSequenceSet *) temp,
      newspans, newcount, atfunc);

  pfree_array((void **) newspans, newcount);

  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a minimum base value
 * @sqlfunc atMin(), atMax(), minusMin(), minusMax()
 */
Temporal *
temporal_restrict_minmax(const Temporal *temp, bool min, bool atfunc)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = atfunc ? (Temporal *) tinstant_copy((TInstant *) temp) : NULL;
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_restrict_minmax((TInstantSet *) temp,
      min, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_restrict_minmax((TSequence *) temp,
      min, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_minmax((TSequenceSet *) temp,
      min, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal value to a timestamp.
 * @sqlfunc atTimestamp(), minusTimestamp()
 */
Temporal *
temporal_restrict_timestamp(const Temporal *temp, TimestampTz t, bool atfunc)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_timestamp((TInstant *) temp, t, atfunc);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_restrict_timestamp((TInstantSet *) temp,
    t, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = atfunc ?
      (Temporal *) tsequence_at_timestamp((TSequence *) temp, t) :
      (Temporal *) tsequence_minus_timestamp((TSequence *) temp, t);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_timestamp((TSequenceSet *) temp,
      t, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Return the base value of a temporal value at the timestamp
 * @sqlfunc valueAtTimestamp()
 * @pymeosfunc valueAtTimestamp()
 */
bool
temporal_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict,
  Datum *result)
{
  bool found = false;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    found = tinstant_value_at_timestamp((TInstant *) temp, t, result);
  else if (temp->subtype == TINSTANTSET)
    found = tinstantset_value_at_timestamp((TInstantSet *) temp, t, result);
  else if (temp->subtype == TSEQUENCE)
    found = tsequence_value_at_timestamp((TSequence *) temp, t, strict, result);
  else /* subtype == TSEQUENCESET */
    found = tsequenceset_value_at_timestamp((TSequenceSet *) temp, t, strict,
      result);
  return found;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a timestamp set
 * @sqlfunc atTimestampSet(), minusTimestampSet()
 */
Temporal *
temporal_restrict_timestampset(const Temporal *temp, const TimestampSet *ts,
  bool atfunc)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_timestampset(
      (TInstant *) temp, ts, atfunc);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_restrict_timestampset(
      (TInstantSet *) temp, ts, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = atfunc ?
      (Temporal *) tsequence_at_timestampset((TSequence *) temp, ts) :
      (Temporal *) tsequence_minus_timestampset((TSequence *) temp, ts);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_timestampset(
      (TSequenceSet *) temp, ts, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a period.
 * @sqlfunc atPeriod(), minusPeriod()
 */
Temporal *
temporal_restrict_period(const Temporal *temp, const Period *p, bool atfunc)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_period(
      (TInstant *) temp, p, atfunc);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_restrict_period(
      (TInstantSet *) temp, p, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = atfunc ?
      (Temporal *) tsequence_at_period((TSequence *) temp, p) :
      (Temporal *) tsequence_minus_period((TSequence *) temp, p);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_period(
      (TSequenceSet *) temp, p, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a period set.
 * @sqlfunc atPeriodSet(), minusPeriodSet()
 */
Temporal *
temporal_restrict_periodset(const Temporal *temp, const PeriodSet *ps,
  bool atfunc)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_periodset(
      (TInstant *) temp, ps, atfunc);
  else if (temp->subtype == TINSTANTSET)
    result = (Temporal *) tinstantset_restrict_periodset(
      (TInstantSet *) temp, ps, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_restrict_periodset(
      (TSequence *) temp, ps, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_periodset(
      (TSequenceSet *) temp, ps, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict a temporal number to a temporal box.
 * @sqlfunc atTbox()
 */
Temporal *
tnumber_at_tbox(const Temporal *temp, const TBOX *box)
{
  /* Bounding box test */
  TBOX box1;
  temporal_set_bbox(temp, &box1);
  if (! overlaps_tbox_tbox(box, &box1))
    return NULL;

  /* At least one of MOBDB_FLAGS_GET_T and MOBDB_FLAGS_GET_X is true */
  Temporal *temp1;
  bool hasx = MOBDB_FLAGS_GET_X(box->flags);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  if (hast)
  {
    Period p;
    span_set(box->tmin, box->tmax, true, true, T_TIMESTAMPTZ, &p);
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
    /* The basetype of a temporal value determines wheter the
     * argument box is converted into an intspan or a floatspan */
    Span *span;
    if (temp->temptype == T_TINT)
      span = span_make(Int32GetDatum((int) box->xmin),
        Int32GetDatum((int) box->xmax), true, true, T_INT4);
    else /* temp->temptype == T_TFLOAT */
      span = span_make(Float8GetDatum(box->xmin),
        Float8GetDatum(box->xmax), true, true, T_FLOAT8);
    result = tnumber_restrict_span(temp1, span, true);
    pfree(span);
  }
  else
    result = temp1;
  if (hasx && hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict a temporal number to the complement of a temporal box.
 *
 * We cannot make the difference from each dimension separately, i.e.,
 * restrict at the period and then restrict to the span. Therefore, we
 * compute the atTbox and then compute the complement of the value obtained.
 * @sqlfunc minusTbox()
 */
Temporal *
tnumber_minus_tbox(const Temporal *temp, const TBOX *box)
{
  /* Bounding box test */
  TBOX box1;
  temporal_set_bbox(temp, &box1);
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
 * @brief Return true if a temporal value intersects a timestamp
 * @see tinstant_intersects_timestamp
 * @see tinstantset_intersects_timestamp
 * @see tsequence_intersects_timestamp
 * @see tsequenceset_intersects_timestamp
 * @sqlfunc intersectsTimestamp()
 * @pymeosfunc intersectsTimestamp()
 */
bool
temporal_intersects_timestamp(const Temporal *temp, TimestampTz t)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_intersects_timestamp((TInstant *) temp, t);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_intersects_timestamp((TInstantSet *) temp, t);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_intersects_timestamp((TSequence *) temp, t);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_intersects_timestamp((TSequenceSet *) temp, t);
  return result;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if a temporal value intersects a timestamp set
 * @see tinstant_intersects_timestampset
 * @see tinstantset_intersects_timestampset
 * @see tsequence_intersects_timestampset
 * @see tsequenceset_intersects_timestampset
 * @sqlfunc intersectsTimestampSet()
 * @pymeosfunc intersectsTimestampSet()
 */
bool
temporal_intersects_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_intersects_timestampset((TInstant *) temp, ts);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_intersects_timestampset((TInstantSet *) temp, ts);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_intersects_timestampset((TSequence *) temp, ts);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_intersects_timestampset((TSequenceSet *) temp, ts);
  return result;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if a temporal value intersects a period
 * @see tinstant_intersects_period
 * @see tinstantset_intersects_period
 * @see tsequence_intersects_period
 * @see tsequenceset_intersects_period
 * @sqlfunc intersectsPeriod()
 * @pymeosfunc intersectsPeriod()
 */
bool
temporal_intersects_period(const Temporal *temp, const Period *p)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_intersects_period((TInstant *) temp, p);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_intersects_period((TInstantSet *) temp, p);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_intersects_period((TSequence *) temp, p);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_intersects_period((TSequenceSet *) temp, p);
  return result;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if a temporal value intersects a period set
 * @see tinstant_intersects_periodset
 * @see tinstantset_intersects_periodset
 * @see tsequence_intersects_periodset
 * @see tsequenceset_intersects_periodset
 * @sqlfunc intersectsPeriodSet()
 * @pymeosfunc intersectsPeriodSet()
 */
bool
temporal_intersects_periodset(const Temporal *temp, const PeriodSet *ps)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_intersects_periodset((TInstant *) temp, ps);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_intersects_periodset((TInstantSet *) temp, ps);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_intersects_periodset((TSequence *) temp, ps);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_intersects_periodset((TSequenceSet *) temp, ps);
  return result;
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_agg
 * @brief Return the integral (area under the curve) of a temporal number
 * @see tnumberseq_integral
 * @see tnumberseqset_integral
 */
double
tnumber_integral(const Temporal *temp)
{
  double result = 0.0;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT || temp->subtype == TINSTANTSET)
    ;
  else if (temp->subtype == TSEQUENCE)
    result = tnumberseq_integral((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tnumberseqset_integral((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Return the time-weighted average of a temporal number
 * @see tnumberinstset_twavg
 * @see tnumberseq_twavg
 * @see tnumberseqset_twavg
 * @sqlfunc twAvg()
 */
double
tnumber_twavg(const Temporal *temp)
{
  double result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tnumberinst_double((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = tnumberinstset_twavg((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tnumberseq_twavg((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tnumberseqset_twavg((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the temporal values are equal.
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 * @see tinstant_eq
 * @see tinstantset_eq
 * @see tsequence_eq
 * @see tsequenceset_eq
 * @sqlop @p =
 * @pymeosfunc __eq__()
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
    if (temp1->subtype == TINSTANT)
      return tinstant_eq((TInstant *) temp1, (TInstant *) temp2);
    else if (temp1->subtype == TINSTANTSET)
      return tinstantset_eq((TInstantSet *) temp1, (TInstantSet *) temp2);
    else if (temp1->subtype == TSEQUENCE)
      return tsequence_eq((TSequence *) temp1, (TSequence *) temp2);
    else /* temp1->subtype == TSEQUENCESET */
      return tsequenceset_eq((TSequenceSet *) temp1, (TSequenceSet *) temp2);
  }

  /* Different temporal type */
  if (temp1->subtype > temp2->subtype)
  {
    const Temporal *temp = (Temporal *) temp1;
    temp1 = temp2;
    temp2 = temp;
  }
  if (temp1->subtype == TINSTANT)
  {
    const TInstant *inst = (TInstant *) temp1;
    if (temp2->subtype == TINSTANTSET)
    {
      ti = (TInstantSet *) temp2;
      if (ti->count != 1)
        return false;
      inst1 = tinstantset_inst_n(ti, 0);
      return tinstant_eq(inst, inst1);
    }
    if (temp2->subtype == TSEQUENCE)
    {
      seq = (TSequence *) temp2;
      if (seq->count != 1)
        return false;
      inst1 = tsequence_inst_n(seq, 0);
      return tinstant_eq(inst, inst1);
    }
    if (temp2->subtype == TSEQUENCESET)
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
  else if (temp1->subtype == TINSTANTSET)
  {
    ti = (TInstantSet *) temp1;
    if (temp2->subtype == TSEQUENCE)
    {
      seq = (TSequence *) temp2;
      if (ti->count != 1 || seq->count != 1)
        return false;
      inst1 = tinstantset_inst_n(ti, 0);
      inst2 = tsequence_inst_n(seq, 0);
      return tinstant_eq(inst1, inst2);
    }
    if (temp2->subtype == TSEQUENCESET)
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
  /* temp1->subtype == TSEQUENCE && temp2->subtype == TSEQUENCESET */
  seq = (TSequence *) temp1;
  ts = (TSequenceSet *) temp2;
  if (ts->count != 1)
    return false;
  const TSequence *seq1 = tsequenceset_seq_n(ts, 0);
  return tsequence_eq(seq, seq1);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the temporal values are different
 * @sqlop @p <>
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
 * @see tinstant_cmp
 * @see tinstantset_cmp
 * @see tsequence_cmp
 * @see tsequenceset_cmp
 * @sqlfunc tbool_cmp(), tint_cmp(), tfloat_cmp(), ttext_cmp(), etc.
 */
int
temporal_cmp(const Temporal *temp1, const Temporal *temp2)
{
  assert(temp1->temptype == temp2->temptype);

  /* Compare bounding period
   * We need to compare periods AND bounding boxes since the bounding boxes
   * do not distinguish between inclusive and exclusive bounds */
  Period p1, p2;
  temporal_set_period(temp1, &p1);
  temporal_set_period(temp2, &p2);
  int result = span_cmp(&p1, &p2);
  if (result)
    return result;

  /* Compare bounding box */
  bboxunion box1, box2;
  temporal_set_bbox(temp1, &box1);
  temporal_set_bbox(temp2, &box2);
  result = temporal_bbox_cmp(&box1, &box2, temp1->temptype);
  if (result)
    return result;

  /* If both are of the same temporal type use the specific comparison */
  if (temp1->subtype == temp2->subtype)
  {
    ensure_valid_tempsubtype(temp1->subtype);
    if (temp1->subtype == TINSTANT)
      return tinstant_cmp((TInstant *) temp1, (TInstant *) temp2);
    else if (temp1->subtype == TINSTANTSET)
      return tinstantset_cmp((TInstantSet *) temp1, (TInstantSet *) temp2);
    else if (temp1->subtype == TSEQUENCE)
      return tsequence_cmp((TSequence *) temp1, (TSequence *) temp2);
    else /* temp1->subtype == TSEQUENCESET */
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
 * @sqlop @p <
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
 * @sqlop @p <=
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
 * @sqlop @p >
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
 * @sqlop @p >=
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
 * @brief Return the 32-bit hash value of a temporal value.
 */
uint32
temporal_hash(const Temporal *temp)
{
  uint32 result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tinstant_hash((TInstant *) temp);
  else if (temp->subtype == TINSTANTSET)
    result = tinstantset_hash((TInstantSet *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_hash((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_hash((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************/
