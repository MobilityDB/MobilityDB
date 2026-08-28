/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Type-inheritance boilerplate for `ts2cell`.
 *
 * The analogue of `meos/src/quadbin/tquadbin.c`. Every temporal type carries
 * this layer to specialise the generic `Temporal` machinery for its own value
 * type:
 *
 *   * argument validators for every supported operand pair,
 *   * type-specific input parsers that delegate to the generic int-8 parser
 *     but tag the result with `T_TS2CELL`,
 *   * type-specific constructors (`ts2cell_make`, `ts2cellinst_make`,
 *     `ts2cellseq_make`, `ts2cellseqset_make`),
 *   * type-specific accessors (`ts2cell_start_value`, `ts2cell_end_value`,
 *     `ts2cell_value_n`, `ts2cell_values`, `ts2cell_value_at_timestamptz`)
 *     that hide the Datum-packing convention from callers,
 *   * MEOS-level conversions to and from `tbigint` for callers that want the
 *     bit-identical representation without a SQL round trip.
 *
 * S2 cells are geodetic, so the temporal point bridge uses `tgeogpoint`.
 */

#include "s2cell/ts2cell.h"

/* C */
#include <assert.h>
#include <string.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_s2cell.h>
#include "temporal/temporal.h"
#include "temporal/lifting.h"
#include "temporal/meos_catalog.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "s2cell/s2cell.h"

/*****************************************************************************
 * Validators
 *
 * Every binary_synced / mixed-type MEOS function calls one of these before
 * doing any real work; failure returns false and leaves an error on the
 * thread-local stack, matching the TQUADBIN and TH3INDEX pattern.
 *****************************************************************************/

/**
 * @brief Ensure that a (ts2cell, ts2cell) pair is valid — both are the right
 * temptype and share a meaningful time axis
 * @details The synchronisation check itself happens later in
 * `tfunc_temporal_temporal`; only the null and temptype fences are here.
 */
bool
ensure_valid_ts2cell_ts2cell(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TS2CELL(temp1, false); VALIDATE_TS2CELL(temp2, false);
  return true;
}

/**
 * @brief Ensure that a (ts2cell, S2CellId) pair is valid
 * @details A value of 0 is the conventional invalid sentinel, what
 * `s2cell_is_valid_cell(0)` answers false for. It is rejected up front so a
 * caller cannot test a trajectory for "ever equal to the invalid sentinel"
 * without noticing.
 */
bool
ensure_valid_ts2cell_s2cell(const Temporal *temp, S2CellId cell)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TS2CELL(temp, false);
  if (! s2cell_is_valid_cell(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Value %" PRIu64 " does not encode a valid S2 cell", (uint64) cell);
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a (ts2cell, tgeogpoint) pair is valid
 */
bool
ensure_valid_ts2cell_tgeogpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TS2CELL(temp1, false);
  if (! ensure_not_null((void *) temp2) ||
      ! ensure_temporal_isof_type((Temporal *) temp2, T_TGEOGPOINT))
    return false;
  return true;
}

/*****************************************************************************
 * Input and output
 *
 * The on-disk representation of a ts2cell is identical to that of a tbigint —
 * the only distinction is the MeosType tag. The type-specific parsers delegate
 * to the generic temporal parser.
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_inout
 * @brief Return a temporal S2 cell from its Well-Known Text representation
 * @csqlfn #Temporal_in()
 */
Temporal *
ts2cell_in(const char *str)
{
  if (! ensure_not_null((void *) str))
    return NULL;
  return temporal_parse(&str, T_TS2CELL);
}

/**
 * @ingroup meos_internal_s2cell_inout
 * @brief Return a temporal S2 cell instant from its Well-Known Text
 * representation
 */
TInstant *
ts2cellinst_in(const char *str)
{
  Temporal *temp = ts2cell_in(str);
  if (! temp)
    return NULL;
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_s2cell_inout
 * @brief Return a temporal S2 cell sequence from its Well-Known Text
 * representation
 * @details A ts2cell sequence always carries step interpolation, S2 cells
 * being discrete; the @p interp argument is accepted for signature parity with
 * the generic API and discarded.
 */
TSequence *
ts2cellseq_in(const char *str, interpType interp)
{
  (void) interp;
  Temporal *temp = ts2cell_in(str);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_s2cell_inout
 * @brief Return a temporal S2 cell sequence set from its Well-Known Text
 * representation
 */
TSequenceSet *
ts2cellseqset_in(const char *str)
{
  Temporal *temp = ts2cell_in(str);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}

/*****************************************************************************
 * Constructors
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_constructor
 * @brief Return a temporal S2 cell instant from a cell and a timestamptz
 * @param[in] value S2 cell value
 * @param[in] t Timestamp
 */
TInstant *
ts2cellinst_make(S2CellId value, TimestampTz t)
{
  return tinstant_make(S2CellGetDatum(value), T_TS2CELL, t);
}

/**
 * @ingroup meos_s2cell_constructor
 * @brief Return a temporal S2 cell sequence from arrays of cells and
 * timestamps, with step interpolation, S2 cells being discrete
 * @param[in] values Array of S2 cell values
 * @param[in] times Array of timestamps, of the same length as @p values
 * @param[in] count Number of elements in the arrays
 * @param[in] lower_inc Lower bound inclusivity
 * @param[in] upper_inc Upper bound inclusivity
 */
TSequence *
ts2cellseq_make(const S2CellId *values, const TimestampTz *times, int count,
  bool lower_inc, bool upper_inc)
{
  if (! ensure_not_null((void *) values) ||
      ! ensure_not_null((void *) times) ||
      ! ensure_positive(count))
    return NULL;

  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; ++i)
    instants[i] = tinstant_make(S2CellGetDatum(values[i]), T_TS2CELL, times[i]);
  TSequence *result = tsequence_make(instants, count, lower_inc, upper_inc,
    STEP, NORMALIZE);
  for (int i = 0; i < count; ++i)
    pfree(instants[i]);
  pfree(instants);
  return result;
}

/**
 * @ingroup meos_s2cell_constructor
 * @brief Return a temporal S2 cell sequence set from an array of sequences
 * @param[in] sequences Array of sequences, with step interpolation
 * @param[in] count Number of sequences
 */
TSequenceSet *
ts2cellseqset_make(const TSequence **sequences, int count)
{
  return tsequenceset_make((TSequence **) sequences, count, NORMALIZE);
}

/**
 * @ingroup meos_s2cell_constructor
 * @brief Return the single-instant temporal S2 cell of a cell and a
 * timestamptz
 */
Temporal *
ts2cell_make(S2CellId value, TimestampTz t)
{
  return (Temporal *) ts2cellinst_make(value, t);
}

/*****************************************************************************
 * Accessors
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_accessor
 * @brief Return the S2 cell at the first instant of a temporal S2 cell
 * @csqlfn #Temporal_start_value()
 */
S2CellId
ts2cell_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TS2CELL(temp, (S2CellId) 0);
  return DatumGetS2Cell(temporal_start_value(temp));
}

/**
 * @ingroup meos_s2cell_accessor
 * @brief Return the S2 cell at the last instant of a temporal S2 cell
 * @csqlfn #Temporal_end_value()
 */
S2CellId
ts2cell_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TS2CELL(temp, (S2CellId) 0);
  return DatumGetS2Cell(temporal_end_value(temp));
}

/**
 * @ingroup meos_s2cell_accessor
 * @brief Return the nth distinct S2 cell of a temporal S2 cell, counting
 * from one
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @return True on success, false when @p n is out of range
 * @csqlfn #Temporal_value_n()
 */
bool
ts2cell_value_n(const Temporal *temp, int n, S2CellId *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TS2CELL(temp, false);
  if (! ensure_not_null((void *) result))
    return false;
  Datum d;
  if (! temporal_value_n(temp, n, &d))
    return false;
  *result = DatumGetS2Cell(d);
  return true;
}

/**
 * @ingroup meos_s2cell_accessor
 * @brief Return the distinct S2 cells a temporal S2 cell takes, in ascending
 * order
 * @param[in] temp Temporal S2 cell
 * @param[out] count Number of distinct values
 * @return Allocated array of @p count cells, owned by the caller
 * @csqlfn #Temporal_valueset()
 */
S2CellId *
ts2cell_values(const Temporal *temp, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TS2CELL(temp, NULL);

  Datum *datums = temporal_values(temp, count);
  if (datums == NULL)
    return NULL;
  S2CellId *result = palloc(sizeof(S2CellId) * (*count));
  for (int i = 0; i < *count; ++i)
    result[i] = DatumGetS2Cell(datums[i]);
  pfree(datums);
  return result;
}

/**
 * @ingroup meos_s2cell_accessor
 * @brief Return the S2 cell of a temporal S2 cell at a timestamptz
 * @param[in] temp Temporal S2 cell
 * @param[in] t Timestamp
 * @param[in] strict True to require an exact instant match, false to answer
 * the value the containing sequence carries under step interpolation
 * @param[out] result The cell at @p t, written only on success
 * @return True on success, false when @p t lies outside @p temp
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
ts2cell_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  S2CellId *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TS2CELL(temp, false);
  if (! ensure_not_null((void *) result))
    return false;
  Datum d;
  if (! temporal_value_at_timestamptz(temp, t, strict, &d))
    return false;
  *result = DatumGetS2Cell(d);
  return true;
}

/*****************************************************************************
 * MEOS-level conversions between ts2cell and tbigint
 *
 * The int64 payload is identical between the tbigint and s2cell base types,
 * but the embedded bounding box differs: a tbigint sequence carries a TBox
 * while a ts2cell sequence carries an STBox. The conversion therefore lifts an
 * identity Datum function with the target restype, so that the instant,
 * sequence and sequence set are rebuilt at the correct shape and the box is
 * recomputed from the new base type.
 *****************************************************************************/

/**
 * @brief Return its argument
 */
static Datum
datum_s2cell_identity(Datum d)
{
  return d;
}

/**
 * @ingroup meos_s2cell_conversion
 * @brief Return a `tbigint` converted to a `ts2cell`, owned by the caller
 * @csqlfn #Tbigint_to_ts2cell()
 */
Temporal *
tbigint_to_ts2cell(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_s2cell_identity;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TBIGINT;
  lfinfo.restype = T_TS2CELL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_s2cell_conversion
 * @brief Return a `ts2cell` converted to a `tbigint`, owned by the caller
 * @csqlfn #Ts2cell_to_tbigint()
 */
Temporal *
ts2cell_to_tbigint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TS2CELL(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_s2cell_identity;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TS2CELL;
  lfinfo.restype = T_TBIGINT;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * Comparison-primitive wrappers
 *
 * Equality and inequality for S2 cells are bit equality at the int64 level,
 * which the `datum2_eq` / `datum2_ne` the generic tbigint machinery uses
 * already covers. These are the thin type-correct symbols the compops
 * dispatcher passes through.
 *****************************************************************************/

/**
 * @brief Return true if two S2 cell Datums are equal
 */
Datum
datum2_s2cell_eq(Datum d1, Datum d2, MeosType type)
{
  (void) type;
  return BoolGetDatum(DatumGetS2Cell(d1) == DatumGetS2Cell(d2));
}

/**
 * @brief Return true if two S2 cell Datums are different
 */
Datum
datum2_s2cell_ne(Datum d1, Datum d2, MeosType type)
{
  (void) type;
  return BoolGetDatum(DatumGetS2Cell(d1) != DatumGetS2Cell(d2));
}

/*****************************************************************************/
