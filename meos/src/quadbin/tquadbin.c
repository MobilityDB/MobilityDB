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
 * @brief Type-inheritance boilerplate for `tquadbin`.
 *
 * This file is the analogue of `meos/src/h3/th3index.c`. Every
 * temporal type carries this layer to specialise the generic
 * `Temporal` machinery for its own value type:
 *
 *   * argument validators for every supported operand pair,
 *   * type-specific input parsers that delegate to the generic
 *     int-8 parser but tag the result with `T_TQUADBIN`,
 *   * type-specific constructors (`tquadbin_make`, `tquadbininst_make`,
 *     `tquadbinseq_make`, `tquadbinseqset_make`),
 *   * type-specific accessors (`tquadbin_start_value`,
 *     `tquadbin_end_value`, `tquadbin_value_n`, `tquadbin_values`,
 *     `tquadbin_value_at_timestamptz`) that hide the Datum-packing
 *     convention from callers,
 *   * MEOS-level conversions to and from `tbigint` for callers that
 *     want the bit-identical representation without a SQL round trip.
 *
 * Quadbin cells are square Web-Mercator (planar) cells; the temporal
 * point bridge therefore uses `tgeompoint` rather than `tgeogpoint`.
 */

#include "quadbin/tquadbin.h"

#include <assert.h>
#include <string.h>

#include <meos.h>
#include <meos_internal.h>
#include <meos_quadbin.h>

#include "temporal/temporal.h"
#include "temporal/lifting.h"
#include "temporal/meos_catalog.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "quadbin/quadbin.h"
#include "quadbin/quadbin_meos.h"

/*****************************************************************************
 * Validators
 *
 * Every binary_synced / mixed-type MEOS function calls one of these
 * before doing any real work; failure returns false and leaves an
 * error on the thread-local stack, matching the TH3INDEX pattern.
 *****************************************************************************/

/**
 * @brief Ensure that a (tquadbin, tquadbin) pair is valid — both are
 * the right temptype and share a meaningful time axis. The sync
 * check itself happens later in `tfunc_temporal_temporal`; we only
 * handle the null / temptype fences here.
 */
bool
ensure_valid_tquadbin_tquadbin(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TQUADBIN(temp1, false);
  VALIDATE_TQUADBIN(temp2, false);
  return true;
}

/**
 * @brief Ensure that a (tquadbin, Quadbin) pair is valid.
 *
 * The cell argument carries quadbin semantics: a value of 0 is the
 * conventional "invalid" sentinel (what `quadbin_is_valid_cell(0)`
 * returns `false` for). We reject it up front so callers cannot
 * e.g. test a temporal trajectory for "ever equal to the invalid
 * sentinel" without noticing.
 */
bool
ensure_valid_tquadbin_quadbin(const Temporal *temp, Quadbin cell)
{
  VALIDATE_TQUADBIN(temp, false);
  if (cell == (Quadbin) 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "quadbin cell argument must not be the invalid sentinel 0");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a (tquadbin, tgeompoint) pair is valid.
 */
bool
ensure_valid_tquadbin_tgeompoint(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TQUADBIN(temp1, false);
  if (! ensure_not_null((void *) temp2) ||
      ! ensure_temporal_isof_type((Temporal *) temp2, T_TGEOMPOINT))
    return false;
  return true;
}

/*****************************************************************************
 * Input / output
 *
 * tquadbin's on-disk representation is identical to tbigint's — the
 * only distinction is the MeosType tag. The type-specific parsers
 * delegate to the generic temporal parser and rely on the enclosing
 * SQL `CREATE FUNCTION tquadbin_in(...)` to pick the right oid.
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_inout
 * @brief Parse a temporal quadbin cell index from its Well-Known Text
 * representation.
 */
Temporal *
tquadbin_in(const char *str)
{
  if (! ensure_not_null((void *) str))
    return NULL;
  return temporal_parse(&str, T_TQUADBIN);
}

/**
 * @ingroup meos_quadbin_inout
 * @brief Parse a temporal quadbin cell instant from its Well-Known Text
 * representation.
 */
TInstant *
tquadbininst_in(const char *str)
{
  Temporal *temp = tquadbin_in(str);
  if (! temp)
    return NULL;
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_quadbin_inout
 * @brief Parse a temporal quadbin cell sequence from its Well-Known Text
 * representation.
 *
 * tquadbin sequences always carry step interpolation (quadbin cells are
 * discrete); the `interp` argument is accepted for signature
 * parity with the generic API and discarded.
 */
TSequence *
tquadbinseq_in(const char *str, interpType interp)
{
  (void) interp;
  Temporal *temp = tquadbin_in(str);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_quadbin_inout
 * @brief Parse a temporal quadbin cell sequence set from its Well-Known Text
 * representation.
 */
TSequenceSet *
tquadbinseqset_in(const char *str)
{
  Temporal *temp = tquadbin_in(str);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}

/*****************************************************************************
 * Constructors
 *
 * These pack a `Quadbin` (uint64) into an `Int64` Datum; the
 * storage is identical to tbigint.
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_constructor
 * @brief Build a temporal quadbin cell instant.
 * @param[in] value The quadbin cell value
 * @param[in] t The instant's timestamp
 */
TInstant *
tquadbininst_make(Quadbin value, TimestampTz t)
{
  return tinstant_make(QuadbinGetDatum(value), T_TQUADBIN, t);
}

/**
 * @ingroup meos_quadbin_constructor
 * @brief Build a temporal quadbin cell sequence (step interpolation only
 * — quadbin cells are discrete, never interpolated).
 * @param[in] values Array of quadbin cell values
 * @param[in] times Array of timestamps, same length as `values`
 * @param[in] count Number of elements in the arrays
 * @param[in] lower_inc Lower bound inclusivity
 * @param[in] upper_inc Upper bound inclusivity
 */
TSequence *
tquadbinseq_make(const Quadbin *values, const TimestampTz *times,
  int count, bool lower_inc, bool upper_inc)
{
  if (! ensure_not_null((void *) values) ||
      ! ensure_not_null((void *) times) ||
      ! ensure_positive(count))
    return NULL;

  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; ++i)
    instants[i] = tinstant_make(QuadbinGetDatum(values[i]),
      T_TQUADBIN, times[i]);
  TSequence *result = tsequence_make(instants, count,
    lower_inc, upper_inc, STEP, NORMALIZE);
  for (int i = 0; i < count; ++i)
    pfree(instants[i]);
  pfree(instants);
  return result;
}

/**
 * @ingroup meos_quadbin_constructor
 * @brief Build a temporal quadbin cell sequence set from an array of sequences.
 * @param[in] sequences Array of sequences (step interpolation)
 * @param[in] count Number of sequences
 */
TSequenceSet *
tquadbinseqset_make(const TSequence **sequences, int count)
{
  return tsequenceset_make((TSequence **) sequences, count, NORMALIZE);
}

/**
 * @ingroup meos_quadbin_constructor
 * @brief Shorthand constructor: the single-instant temporal quadbin cell.
 * Alias for `tquadbininst_make`.
 */
Temporal *
tquadbin_make(Quadbin value, TimestampTz t)
{
  return (Temporal *) tquadbininst_make(value, t);
}

/*****************************************************************************
 * Accessors — hide the Datum-packing convention from the public API.
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_accessor
 * @brief Return the quadbin cell value at the first instant of `temp`.
 */
Quadbin
tquadbin_start_value(const Temporal *temp)
{
  VALIDATE_TQUADBIN(temp, (Quadbin) 0);
  return DatumGetQuadbin(temporal_start_value(temp));
}

/**
 * @ingroup meos_quadbin_accessor
 * @brief Return the quadbin cell value at the last instant of `temp`.
 */
Quadbin
tquadbin_end_value(const Temporal *temp)
{
  VALIDATE_TQUADBIN(temp, (Quadbin) 0);
  return DatumGetQuadbin(temporal_end_value(temp));
}

/**
 * @ingroup meos_quadbin_accessor
 * @brief Return the quadbin cell value at the `n`-th distinct value of `temp`.
 * 1-indexed.
 * @return `true` on success, `false` if `n` is out of range.
 */
bool
tquadbin_value_n(const Temporal *temp, int n, Quadbin *result)
{
  VALIDATE_TQUADBIN(temp, false);
  if (! ensure_not_null((void *) result))
    return false;
  Datum d;
  if (! temporal_value_n(temp, n, &d))
    return false;
  *result = DatumGetQuadbin(d);
  return true;
}

/**
 * @ingroup meos_quadbin_accessor
 * @brief Return the distinct quadbin cell values that `temp` takes, in
 * ascending order.
 * @param[in] temp Temporal quadbin cell
 * @param[out] count Number of distinct values
 * @return Allocated array of `count` Quadbin values. Caller owns.
 */
Quadbin *
tquadbin_values(const Temporal *temp, int *count)
{
  VALIDATE_TQUADBIN(temp, NULL);
  if (! ensure_not_null((void *) count))
    return NULL;
  Datum *datums = temporal_values(temp, count);
  if (datums == NULL)
    return NULL;
  Quadbin *result = palloc(sizeof(Quadbin) * (*count));
  for (int i = 0; i < *count; ++i)
    result[i] = DatumGetQuadbin(datums[i]);
  pfree(datums);
  return result;
}

/**
 * @ingroup meos_quadbin_accessor
 * @brief Return the quadbin cell value of `temp` at timestamp `t`.
 * @param[in] temp Temporal quadbin cell
 * @param[in] t Timestamp
 * @param[in] strict True to require an exact instant match; false
 * to return the value at the containing sequence under step
 * interpolation.
 * @param[out] result The quadbin cell value at `t` (only written on success)
 * @return `true` on success, `false` if `t` lies outside `temp`.
 */
bool
tquadbin_value_at_timestamptz(const Temporal *temp, TimestampTz t,
  bool strict, Quadbin *result)
{
  VALIDATE_TQUADBIN(temp, false);
  if (! ensure_not_null((void *) result))
    return false;
  Datum d;
  if (! temporal_value_at_timestamptz(temp, t, strict, &d))
    return false;
  *result = DatumGetQuadbin(d);
  return true;
}

/*****************************************************************************
 * MEOS-level conversions between tquadbin and tbigint
 *
 * The int64 payload is identical between tbigint and quadbin basetypes,
 * but the embedded bounding box differs: tbigint sequences carry a TBox
 * while tquadbin sequences carry an STBox. The conversion is therefore
 * implemented by lifting an identity Datum function with the target
 * restype so that tinstant/tsequence/tsequenceset are rebuilt at the
 * correct shape and the bbox is recomputed from the new basetype.
 *****************************************************************************/

static Datum
datum_quadbin_identity(Datum d)
{
  return d;
}

/**
 * @ingroup meos_quadbin_conversion
 * @brief Convert a `tbigint` to a `tquadbin`. Caller owns the result.
 */
Temporal *
tbigint_to_tquadbin(const Temporal *temp)
{
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type((Temporal *) temp, T_TBIGINT))
    return NULL;
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_quadbin_identity;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TBIGINT;
  lfinfo.restype = T_TQUADBIN;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_quadbin_conversion
 * @brief Convert a `tquadbin` to a `tbigint`. Caller owns the result.
 */
Temporal *
tquadbin_to_tbigint(const Temporal *temp)
{
  VALIDATE_TQUADBIN(temp, NULL);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_quadbin_identity;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TQUADBIN;
  lfinfo.restype = T_TBIGINT;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * Comparison-primitive wrappers
 *
 * Equality / inequality for quadbin cells is bit equality at the int64
 * level — the `datum2_eq` / `datum2_ne` used by the generic tbigint
 * machinery already covers that. We only need thin type-correct
 * symbols so the compops dispatcher can pass them through.
 *****************************************************************************/

Datum
datum2_quadbin_eq(Datum d1, Datum d2, MeosType type)
{
  (void) type;
  return BoolGetDatum(DatumGetQuadbin(d1) == DatumGetQuadbin(d2));
}

Datum
datum2_quadbin_ne(Datum d1, Datum d2, MeosType type)
{
  (void) type;
  return BoolGetDatum(DatumGetQuadbin(d1) != DatumGetQuadbin(d2));
}

/*****************************************************************************/
