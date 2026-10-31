/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @file
 * @brief Bin and tile functions for temporal types
 * @note The time bin functions are inspired from TimescaleDB
 * https://docs.timescale.com/latest/api#time_bucket
 */

#include "temporal/temporal_tile.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/date.h>
#include <utils/datetime.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/temporal_restrict.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * Multidimensional tile functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Generate a multidimensional grid for temporal numbers
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the tiles on the time
 * dimension @nullable
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 */
TBox *
tbox_value_time_tiles(const TBox *box, Datum vsize, const Interval *duration,
  Datum vorigin, TimestampTz torigin, bool border_inc, int *count)
{
  assert(box); assert(count);
  assert(! MEOS_FLAGS_GET_X(box->flags) ||
    not_negative_datum(vsize, box->span.basetype));
  assert(! duration || positive_duration(duration));

  TboxGridState *state = tbox_tile_state_make(NULL, box, vsize, duration,
    vorigin, torigin, border_inc);
  int count1 = state->ntiles;

  /* Compute the tiles */
  TBox *result = palloc0(sizeof(TBox) * count1);
  for (int i = 0; i < count1; i++)
  {
    tbox_tile_state_get(state, &result[i]);
    tbox_tile_state_next(state);
  }
  *count = count1;
  pfree(state);
  return result;
}

/**
 * @brief Ensure the validity of a temporal box and the grid laid over it
 * @details Each caller tests the sizes of the dimensions it takes, so a value
 * size of 0 is a value dimension the caller does not take. A value dimension
 * needs a box with a value span of the given type, as #tintbox_shift_scale
 * needs, and a time dimension needs a box with a period and a positive
 * duration
 * @param[in] box Temporal box
 * @param[in] vsize Value size of the tiles, 0 when the value dimension is not
 * used for tiling
 * @param[in] duration Interval defining the size of the bins @nullable
 * @param[in] spantype Span type of the value dimension
 */
static bool
ensure_valid_tbox_tiles(const TBox *box, double vsize,
  const Interval *duration, MeosType spantype)
{
  if (vsize > 0 && (! ensure_has_X(T_TBOX, box->flags) ||
      ! ensure_span_isof_type(&box->span, spantype)))
    return false;
  if (duration && (! ensure_has_T(T_TBOX, box->flags) ||
      ! ensure_positive_duration(duration)))
    return false;
  return true;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tiles of a temporal integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles, which must be positive
 * @param[in] duration Interval defining the size of the bins, which must be
 * positive
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_time_tiles()
 */
TBox *
tintbox_value_time_tiles(const TBox *box, int vsize, const Interval *duration,
  int vorigin, TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_datum(Int32GetDatum(vsize), T_INT4) ||
      ! ensure_valid_tbox_tiles(box, (double) vsize, duration, T_INTSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Int32GetDatum(vsize), duration,
    Int32GetDatum(vorigin), torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tiles of a temporal big integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles, which must be positive
 * @param[in] duration Interval defining the size of the bins, which must be
 * positive
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_time_tiles()
 */
TBox *
tbigintbox_value_time_tiles(const TBox *box, int64 vsize,
  const Interval *duration, int64 vorigin, TimestampTz torigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_datum(Int64GetDatum(vsize), T_INT8) ||
      ! ensure_valid_tbox_tiles(box, (double) vsize, duration, T_BIGINTSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Int64GetDatum(vsize), duration,
    Int64GetDatum(vorigin), torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tiles of a temporal float box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles, which must be positive
 * @param[in] duration Interval defining the size of the bins, which must be
 * positive
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_time_tiles()
 */
TBox *
tfloatbox_value_time_tiles(const TBox *box, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8) ||
      ! ensure_valid_tbox_tiles(box, vsize, duration, T_FLOATSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Float8GetDatum(vsize), duration,
    Float8GetDatum(vorigin), torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the value tiles of a temporal integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_tiles()
 */
TBox *
tintbox_value_tiles(const TBox *box, int vsize, int vorigin, bool border_inc,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_positive_datum(Int32GetDatum(vsize), T_INT4) ||
      ! ensure_valid_tbox_tiles(box, (double) vsize, NULL, T_INTSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Int32GetDatum(vsize), NULL,
    Int32GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the value tiles of a temporal big integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_tiles()
 */
TBox *
tbigintbox_value_tiles(const TBox *box, int64 vsize, int64 vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_positive_datum(Int64GetDatum(vsize), T_INT8) ||
      ! ensure_valid_tbox_tiles(box, (double) vsize, NULL, T_BIGINTSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Int64GetDatum(vsize), NULL,
    Int64GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the value tiles of a temporal float box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_tiles()
 */
TBox *
tfloatbox_value_tiles(const TBox *box, double vsize, double vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8) ||
      ! ensure_valid_tbox_tiles(box, vsize, NULL, T_FLOATSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Float8GetDatum(vsize), NULL,
    Float8GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time tiles of a temporal box
 * @param[in] box Input box to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_time_tiles()
 */
TBox *
tbox_time_tiles(const TBox *box, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_valid_tbox_tiles(box, 0.0, duration, box->span.spantype))
    return NULL;
  /* The value size of the tiles is zero in the base type of the box */
  Datum vzero = (box->span.basetype == T_FLOAT8) ? Float8GetDatum(0.0) :
    (box->span.basetype == T_INT8) ? Int64GetDatum(0) : Int32GetDatum(0);
  return tbox_value_time_tiles(box, vzero, duration, vzero, torigin,
    border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time tiles of a temporal integer box
 * @param[in] box Input box to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 */
TBox *
tintbox_time_tiles(const TBox *box, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  return tbox_time_tiles(box, duration, torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time tiles of a temporal big integer box
 * @param[in] box Input box to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 */
TBox *
tbigintbox_time_tiles(const TBox *box, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  return tbox_time_tiles(box, duration, torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time tiles of a temporal float box
 * @param[in] box Input box to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 */
TBox *
tfloatbox_time_tiles(const TBox *box, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  return tbox_time_tiles(box, duration, torigin, border_inc, count);
}

/*****************************************************************************
 * Boxes functions for temporal numbers
 *****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value grid
 * @param[in] temp Temporal value
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_boxes()
 */
TBox *
tint_value_boxes(const Temporal *temp, int vsize, int vorigin, bool border_inc,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tnumber_value_time_boxes(temp, Int32GetDatum(vsize), NULL,
    Int32GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a time grid
 * @param[in] temp Temporal value
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_time_boxes()
 */
TBox *
tint_time_boxes(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  return tnumber_value_time_boxes(temp, Int32GetDatum(0), duration,
    Int32GetDatum(0), torigin, border_inc, count);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal big integer split with
 * respect to a value grid
 * @param[in] temp Temporal value
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_boxes()
 */
TBox *
tbigint_value_boxes(const Temporal *temp, int64 vsize, int64 vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tnumber_value_time_boxes(temp, Int64GetDatum(vsize), NULL,
    Int64GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal big integer split with
 * respect to a time grid
 * @param[in] temp Temporal value
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_time_boxes()
 */
TBox *
tbigint_time_boxes(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  return tnumber_value_time_boxes(temp, Int64GetDatum(0), duration,
    Int64GetDatum(0), torigin, border_inc, count);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value grid
 * @param[in] temp Temporal value
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_boxes()
 */
TBox *
tfloat_value_boxes(const Temporal *temp, double vsize, double vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tnumber_value_time_boxes(temp, Float8GetDatum(vsize), NULL,
    Float8GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a time grid
 * @param[in] temp Temporal value
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_time_boxes()
 */
TBox *
tfloat_time_boxes(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  return tnumber_value_time_boxes(temp, Float8GetDatum(0.0), duration,
    Float8GetDatum(0.0), torigin, border_inc, count);
}

/*****************************************************************************
 * Value split functions for temporal numbers
 *****************************************************************************/

/**
 * @brief Get the bin number in the bin space that contains the value
 * @param[in] value Input value
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @param[in] type Type of the arguments
 */
static int
bin_position(Datum value, Datum size, Datum origin, MeosType type)
{
  assert(tnumber_basetype(type));
  if (type == T_INT4)
    return (DatumGetInt32(value) - DatumGetInt32(origin)) /
      DatumGetInt32(size);
  else /* type == T_FLOAT8 */
    return (int) floor( (DatumGetFloat8(value) - DatumGetFloat8(origin)) /
      DatumGetFloat8(size) );
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] inst Temporal value
 * @param[in] size Size of the value bins
 * @param[in] start_bin Value of the start bin
 * @param[out] bins Start value of the bins containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TInstant **
tnumberinst_value_split(const TInstant *inst, Datum start_bin, Datum size,
  Datum **bins, int *newcount)
{
  assert(inst); assert(bins); assert(newcount);

  Datum value = tinstant_value_p(inst);
  MeosType basetype = temptype_basetype(inst->temptype);
  TInstant **result = palloc(sizeof(TInstant *));
  Datum *values = palloc(sizeof(Datum));
  result[0] = tinstant_copy(inst);
  values[0] = datum_bin(value, size, start_bin, basetype);
  *bins = values;
  *newcount = 1;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] size Size of the value bins
 * @param[in] start_bin Value of the start bin
 * @param[in] count Number of bins
 * @param[out] bins Start value of the bins containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TSequence **
tnumberseq_disc_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, Datum **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);

  MeosType basetype = temptype_basetype(seq->temptype);
  TSequence **result;
  Datum *values, value, bin_value;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result = palloc(sizeof(TSequence *));
    values = palloc(sizeof(Datum));
    result[0] = tsequence_copy(seq);
    value = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bin(value, size, start_bin, basetype);
    *bins = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *ninsts = palloc0(sizeof(int) * count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    value = tinstant_value_p(inst);
    bin_value = datum_bin(value, size, start_bin, basetype);
    int bin_no = bin_position(bin_value, size, start_bin, basetype);
    int inst_no = ninsts[bin_no]++;
    instants[bin_no * seq->count + inst_no] = inst;
  }
  /* Assemble the result for each value bin */
  result = palloc(sizeof(TSequence *) * count);
  values = palloc(sizeof(Datum) * count);
  int nfrags = 0;
  bin_value = start_bin;
  for (int i = 0; i < count; i++)
  {
    if (ninsts[i] > 0)
    {
      result[nfrags] = tsequence_make((TInstant **) &instants[i * seq->count],
        ninsts[i], true, true, DISCRETE, NORMALIZE_NO);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
  }
  pfree(instants);
  pfree(ninsts);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[in,out] result Array containing the fragments of each bin
 * @param[in,out] nseqs Number of fragments for each bin
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 * @p seq->count for sequences or @p ss->totalcount for sequence sets
 */
static void
tnumberseq_step_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, TSequence **result, int *nseqs, int numcols)
{
  assert(seq); assert(result); assert(nseqs);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == STEP);

  MeosType basetype = temptype_basetype(seq->temptype);
  Datum value, bin_value;
  int bin_no, seq_no;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    result[bin_no * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    value = tinstant_value_p(inst1);
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    bool lower_inc1 = (i == 1) ? seq->period.lower_inc : true;
    TInstant *bounds[2];
    bounds[0] = (TInstant *) inst1;
    int nfrags = 1;
    if (i < seq->count)
    {
      tofree[nfree++] = bounds[1] = tinstant_make(value, seq->temptype,
        inst2->t);
      nfrags++;
    }
    result[bin_no * numcols + seq_no] = tsequence_make(bounds, nfrags,
      lower_inc1, false, STEP, NORMALIZE);
    bounds[0] = bounds[1];
    inst1 = inst2;
    lower_inc1 = true;
  }
  /* Last value if upper inclusive */
  if (seq->period.upper_inc)
  {
    inst1 = TSEQUENCE_INST_N(seq, seq->count - 1);
    value = tinstant_value_p(inst1);
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    result[bin_no * numcols + seq_no] = tinstant_as_tsequence(inst1, STEP);
  }
  pfree_array((void **) tofree, nfree);
  return;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[in,out] result Array containing the fragments of each bin
 * @param[in,out] nseqs Number of fragments for each bin
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 * @p seq->count for sequences or @p ss->totalcount for sequence sets
 */
static void
tnumberseq_linear_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, TSequence **result, int *nseqs, int numcols)
{
  assert(seq); assert(result); assert(nseqs);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  MeosType basetype = temptype_basetype(seq->temptype);
  MeosType spantype = basetype_spantype(basetype);
  Datum value1, bin_value1;
  int bin_no1, seq_no;
  Span segspan;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value1 = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    bin_value1 = datum_bin(value1, size, start_bin, basetype);
    bin_no1 = bin_position(bin_value1, size, start_bin, basetype);
    seq_no = nseqs[bin_no1]++;
    result[bin_no1 * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  value1 = tinstant_value_p(inst1);
  bin_value1 = datum_bin(value1, size, start_bin, basetype);
  bin_no1 = bin_position(bin_value1, size, start_bin, basetype);
  /* For each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value_p(inst2);
    Datum bin_value2 = datum_bin(value2, size, start_bin, basetype);
    int bin_no2 = bin_position(bin_value2, size, start_bin, basetype);

    /* Set variables depending on whether the segment is constant, increasing,
     * or decreasing */
    Datum min_value, max_value;
    int first_bin, last_bin, first, last;
    bool lower_inc1, upper_inc1; /* Lower/upper bound inclusion of the segment */
    bool lower_inc_def, upper_inc_def; /* Default lower/upper bound inclusion */
    int cmp = datum_cmp(value1, value2, basetype);
    if (cmp <= 0)
    {
      /* Both for constant and increasing segments */
      min_value = value1;
      max_value = value2;
      first_bin = bin_no1;
      last_bin = bin_no2;
      first = 0;
      last = 1;
      lower_inc_def = true;
      upper_inc_def = false;
      lower_inc1 = (i == 1) ? seq->period.lower_inc : true;
      upper_inc1 = (i == seq->count - 1) ? seq->period.upper_inc : false;
    }
    else
    {
      min_value = value2;
      max_value = value1;
      first_bin = bin_no2;
      last_bin = bin_no1;
      first = 1;
      last = 0;
      lower_inc_def = false;
      upper_inc_def = true;
      lower_inc1 = (i == seq->count - 1) ? seq->period.upper_inc : false;
      upper_inc1 = (i == 1) ? seq->period.lower_inc : true;
    }

    /* Split the segment into bins */
    span_set(min_value, max_value, lower_inc1, (cmp != 0) ? upper_inc1 : true,
      basetype, spantype, &segspan);
    TInstant *bounds[2];
    bounds[first] = (cmp <= 0) ? (TInstant *) inst1 : (TInstant *) inst2;
    Datum bin_lower = (cmp <= 0) ? bin_value1 : bin_value2;
    Datum bin_upper = datum_add(bin_lower, size, basetype);
    for (int j = first_bin; j <= last_bin; j++)
    {
      /* Choose between interpolate or take one of the segment ends */
      if (datum_lt(min_value, bin_upper, basetype) &&
          datum_lt(bin_upper, max_value, basetype))
      {
        TimestampTz t1, t2;
        /* We are sure there is a unique turning point */
        tsegment_intersection_value(value1, value2, bin_upper, inst1->temptype,
          inst1->t, inst2->t, &t1, &t2);
        /* To reduce the roundoff errors we take the value projected to the
         * timestamp instead of the bound value */
        Datum projvalue1 = tsegment_value_at_timestamptz(value1, value2,
          inst1->temptype, inst1->t, inst2->t, t1);
        tofree[nfree++] = bounds[last] = 
          tinstant_make(projvalue1, seq->temptype, t1);
      }
      else
        bounds[last] = (cmp <= 0) ? (TInstant *) inst2 : (TInstant *) inst1;
      /* Determine the bounds of the resulting sequence */
      if (j == first_bin || j == last_bin)
      {
        Span binspan;
        span_set(bin_lower, bin_upper, true, false, basetype, spantype,
          &binspan);
        Span inter;
        bool found = inter_span_span(&segspan, &binspan, &inter);
        if (found)
        {
          /* Do nothing for constant segments */
          if (cmp < 0)
          {
            lower_inc1 = inter.lower_inc;
            upper_inc1 = inter.upper_inc;
          }
          else if (cmp > 0)
          {
            lower_inc1 = inter.upper_inc;
            upper_inc1 = inter.lower_inc;
          }
        }
        else
        {
          lower_inc1 = upper_inc1 = false;
        }
      }
      else
      {
        /* Sequence bounds are the bin bounds */
        lower_inc1 = lower_inc_def;
        upper_inc1 = upper_inc_def;
      }
      /* If last bin contains a single instant */
      int nfrags = (bounds[0]->t == bounds[1]->t) ? 1 : 2;
      /* We cannot add to last bin if last instant has exclusive bound */
      if (nfrags == 1 && ! upper_inc1)
        break;
      seq_no = nseqs[j]++;
      result[j * numcols + seq_no] = tsequence_make(bounds, nfrags,
        (nfrags > 1) ? lower_inc1 : true, (nfrags > 1) ? upper_inc1 : true,
        LINEAR, NORMALIZE_NO);
      bounds[first] = bounds[last];
      bin_lower = bin_upper;
      bin_upper = datum_add(bin_upper, size, basetype);
    }
    inst1 = inst2;
    value1 = value2;
    bin_value1 = bin_value2;
    bin_no1 = bin_no2;
  }
  pfree_array((void **) tofree, nfree);
  return;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[out] bins Start value of the bins containing the fragments
 * @param[out] newcount Number of elements in output arrays
 */
static TSequenceSet **
tnumberseq_cont_value_split(const TSequence *seq, Datum start_bin, Datum size,
  int count, Datum **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);

  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  assert(interp != DISCRETE);
  MeosType basetype = temptype_basetype(seq->temptype);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TSequenceSet **result = palloc(sizeof(TSequenceSet *));
    Datum *values = palloc(sizeof(Datum));
    result[0] = tsequence_as_tsequenceset(seq);
    Datum value = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bin(value, size, start_bin, basetype);
    *bins = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *nseqs = palloc0(sizeof(int) * count);
  if (interp == LINEAR)
    tnumberseq_linear_value_split(seq, start_bin, size, count, sequences,
      nseqs, seq->count);
  else
    tnumberseq_step_value_split(seq, start_bin, size, count, sequences,
      nseqs, seq->count);
  /* Assemble the result for each value bin */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bin_value = start_bin;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (nseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make(&sequences[seq->count * i],
        nseqs[i], NORMALIZE);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
    /* Clean up */
    for (int j = 0; j < nseqs[i]; j++)
      pfree(sequences[seq->count * i + j]);
  }
  pfree(sequences);
  pfree(nseqs);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] ss Temporal value
 * @param[in] start_bin Start value of the first bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[out] bins Array of start values of the bins containing the
 * fragments
 * @param[out] newcount Number of values in the output arrays
 */
static TSequenceSet **
tnumberseqset_value_split(const TSequenceSet *ss, Datum start_bin, Datum size,
  int count, Datum **bins, int *newcount)
{
  assert(ss); assert(bins); assert(newcount);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tnumberseq_cont_value_split(TSEQUENCESET_SEQ_N(ss, 0), start_bin,
      size, count, bins, newcount);

  /* General case */
  MeosType basetype = temptype_basetype(ss->temptype);
  TSequence **binseqs = palloc(sizeof(TSequence *) * ss->totalcount * count);
  /* palloc0 to initialize the counters to 0 */
  int *nseqs = palloc0(sizeof(int) * count);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(ss->flags);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (linear)
      tnumberseq_linear_value_split(seq, start_bin, size, count, binseqs,
        nseqs, ss->totalcount);
    else
      tnumberseq_step_value_split(seq, start_bin, size, count, binseqs,
        nseqs, ss->totalcount);
  }
  /* Assemble the result for each value bin */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bin_value = start_bin;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (nseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make(&binseqs[i * ss->totalcount],
        nseqs[i], NORMALIZE);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
    /* Clean up */
    for (int j = 0; j < nseqs[i]; j++)
      pfree(binseqs[i * ss->totalcount + j]);
  }
  pfree(binseqs);
  pfree(nseqs);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_tile
 * @brief Split a temporal number into an array of fragments according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] vorigin Origin of the value bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] bins Array of start values of the bins containing the fragments
 * @param[out] count Number of values in the output arrays
 */
Temporal **
tnumber_value_split(const Temporal *temp, Datum vsize, Datum vorigin,
  bool border_inc, Datum **bins, int *count)
{
  assert(temp); assert(bins); assert(count);
  assert(tnumber_type(temp->temptype));

  /* Compute the value bounds */
  Span s;
  tnumber_set_span(temp, &s);
  Datum start_bin, end_bin, end_bin_border;
  int nbins = span_num_bins(&s, vsize, vorigin, true, &start_bin, &end_bin);

  /* Split the temporal value over every bin, since the kernels place each
   * piece by the position of its bin */
  Temporal **result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      result = (Temporal **) tnumberinst_value_split((const TInstant *) temp,
        start_bin, vsize, bins, count);
      break;
    case TSEQUENCE:
      result = MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal **) tnumberseq_disc_value_split((const TSequence *) temp,
          start_bin, vsize, nbins, bins, count) :
        (Temporal **) tnumberseq_cont_value_split((const TSequence *) temp,
          start_bin, vsize, nbins, bins, count);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal **) tnumberseqset_value_split(
        (const TSequenceSet *) temp, start_bin, vsize, nbins, bins, count);
  }

  /* A grid not containing the upper border of the values leaves out the bin
   * starting at it, the last one, which holds nothing but that border */
  if (! border_inc && span_num_bins(&s, vsize, vorigin, false, &start_bin,
        &end_bin_border) < nbins && *count > 0 &&
      datum_eq((*bins)[*count - 1], end_bin_border,
        temptype_basetype(temp->temptype)))
  {
    pfree(result[*count - 1]);
    (*count)--;
  }
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a temporal value split according to a base value and possibly
 * a temporal grid
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] value_bins Array of value bins @nullable
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 */
Temporal **
tnumber_value_time_split(const Temporal *temp, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin,
  bool border_inc, Datum **value_bins, TimestampTz **time_bins, int *count)
{
  assert(temp); assert(count); assert(tnumber_type(temp->temptype));
  MeosType basetype = temptype_basetype(temp->temptype);
  assert(positive_datum(vsize, basetype)); assert(positive_duration(duration));

  Datum start_bin, end_bin, start_time_bin, end_time_bin;
  /* Compute the value bounds */
  Span s;
  tnumber_set_span(temp, &s);
  int value_count = span_num_bins(&s, vsize, vorigin, border_inc, &start_bin,
    &end_bin);
  /* Compute the time bounds */
  temporal_set_tstzspan(temp, &s);
  int64 tunits = interval_units(duration);
  int time_count = span_num_bins(&s, Int64GetDatum(tunits),
    TimestampTzGetDatum(torigin), border_inc, &start_time_bin, &end_time_bin);
  TimestampTz start_time = DatumGetTimestampTz(start_time_bin);
  TimestampTz end_time = DatumGetTimestampTz(end_time_bin);
  /* Total number of tiles */
  int ntiles = value_count * time_count;

  /* Split the temporal value */
  Datum *v_bins = NULL;
  TimestampTz *t_bins = NULL;
  Temporal **fragments;
  v_bins = palloc(sizeof(Datum) * ntiles);
  t_bins = palloc(sizeof(TimestampTz) * ntiles);
  fragments = palloc(sizeof(Temporal *) * ntiles);
  int nfrags = 0;
  Datum lower_value = start_bin;
  MeosType spantype = basetype_spantype(basetype);
  while (datum_lt(lower_value, end_bin, basetype))
  {
    Datum upper_value = datum_add(lower_value, vsize, basetype);
    span_set(lower_value, upper_value, true, false, basetype, spantype, &s);
    Temporal *atspan = tnumber_restrict_span(temp, &s, REST_AT);
    if (atspan != NULL)
    {
      int num_time_splits;
      TimestampTz *times;
      Temporal **time_splits = temporal_time_split_intl(atspan, start_time,
        end_time, tunits, torigin, time_count, &times, &num_time_splits);
      for (int i = 0; i < num_time_splits; i++)
      {
        v_bins[i + nfrags] = lower_value;
        t_bins[i + nfrags] = times[i];
        fragments[i + nfrags] = time_splits[i];
      }
      nfrags += num_time_splits;
      pfree(time_splits);
      pfree(times);
      pfree(atspan);
    }
    lower_value = upper_value;
  }
  *count = nfrags;
  if (value_bins)
    *value_bins = v_bins;
  else
    pfree(v_bins);
  if (time_bins)
    *time_bins = t_bins;
  else
    pfree(t_bins);
  return fragments;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] bins Array of bins @nullable
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tint_value_split(const Temporal *temp, int vsize, int vorigin, bool border_inc,
  int **bins,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(bins, NULL);
  if (! ensure_positive(vsize))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_split(temp, Int32GetDatum(vsize),
    Int32GetDatum(vorigin), border_inc, &datum_bins, count);
  /* Transform the datum bins into integer bins and return */
  int *values = palloc(sizeof(int) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt32(datum_bins[i]);
  if (bins)
    *bins = values;
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal big integer split according to
 * value bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] bins Array of bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tbigint_value_split(const Temporal *temp, int64 vsize, int64 vorigin,
  bool border_inc, int64 **bins, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL); VALIDATE_NOT_NULL(bins, NULL);
  if (! ensure_positive_datum(Int64GetDatum(vsize), T_INT8))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_split(temp, Int64GetDatum(vsize),
    Int64GetDatum(vorigin), border_inc, &datum_bins, count);
  /* Transform the datum bins into big integer bins and return */
  int64 *values = palloc(sizeof(int64) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt64(datum_bins[i]);
  *bins = values;
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal float split according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] bins Array of bins @nullable
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tfloat_value_split(const Temporal *temp, double vsize, double vorigin,
  bool border_inc, double **bins, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(bins, NULL);
  if (! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_split(temp, Float8GetDatum(vsize),
    Float8GetDatum(vorigin), border_inc, &datum_bins, count);
  /* Transform the datum bins into float bins and return */
  double *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetFloat8(datum_bins[i]);
  if (bins)
    *bins = values;
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * and time bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] value_bins Array of value bins @nullable
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tint_value_time_split(const Temporal *temp, int vsize, const Interval *duration,
  int vorigin, TimestampTz torigin, bool border_inc, int **value_bins,
  TimestampTz **time_bins, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive(vsize) || ! ensure_positive_duration(duration))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_time_split(temp, Int32GetDatum(vsize),
    duration, Int32GetDatum(vorigin), torigin, border_inc, &datum_bins,
    time_bins,
    count);

  /* Transform the datum bins into integer bins and return */
  int *values = palloc(sizeof(int) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt32(datum_bins[i]);
  if (value_bins)
    *value_bins = values;
  else
    pfree(values);
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal big integer split according to
 * value and time bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] value_bins Array of value bins @nullable
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tbigint_value_time_split(const Temporal *temp, int64 vsize,
  const Interval *duration, int64 vorigin, TimestampTz torigin,
  bool border_inc, int64 **value_bins, TimestampTz **time_bins, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL); VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_datum(Int64GetDatum(vsize), T_INT8) ||
      ! ensure_positive_duration(duration))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_time_split(temp, Int64GetDatum(vsize),
    duration, Int64GetDatum(vorigin), torigin, border_inc, &datum_bins,
    time_bins,
    count);

  /* Transform the datum bins into big integer bins and return */
  int64 *values = palloc(sizeof(int64) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt64(datum_bins[i]);
  if (value_bins)
    *value_bins = values;
  else
    pfree(values);
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * and time bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] value_bins Array of value bins @nullable
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tfloat_value_time_split(const Temporal *temp, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin,
  bool border_inc, double **value_bins, TimestampTz **time_bins, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8) ||
      ! ensure_positive_duration(duration))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_time_split(temp, Float8GetDatum(vsize),
    duration, Float8GetDatum(vorigin), torigin, border_inc, &datum_bins,
    time_bins,
    count);

  /* Transform the datum bins into float bins and return */
  double *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetFloat8(datum_bins[i]);
  if (value_bins)
    *value_bins = values;
  else
    pfree(values);
  pfree(datum_bins);
  return result;
}

/*****************************************************************************/
