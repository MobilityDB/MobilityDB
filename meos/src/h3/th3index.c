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
 * @brief Type-inheritance boilerplate for `th3index`.
 *
 * This file is the analogue of `meos/src/cbuffer/tcbuffer.c`. Every
 * temporal type carries this layer to specialise the generic
 * `Temporal` machinery for its own value type:
 *
 *   * argument validators for every supported operand pair,
 *   * type-specific input parsers that delegate to the generic
 *     int-8 parser but tag the result with `T_TH3INDEX`,
 *   * type-specific constructors (`th3index_make`, `th3indexinst_make`,
 *     `th3indexseq_make`, `th3indexseqset_make`),
 *   * type-specific accessors (`th3index_start_value`,
 *     `th3index_end_value`, `th3index_value_n`, `th3index_values`,
 *     `th3index_value_at_timestamptz`) that hide the Datum-packing
 *     convention from callers,
 *   * MEOS-level conversions to and from `tbigint` for callers that
 *     want the bit-identical representation without a SQL round trip.
 *
 * The lifting-specific Datum wrappers (`datum_h3_*`) live in
 * `th3index_lifting.c`; the comparison-operator wrappers live in
 * `th3index_compops.c`.
 */

#include "h3/th3index.h"

#include <assert.h>
#include <string.h>

#include <meos.h>
#include <meos_internal.h>
#include <meos_h3.h>

#include "temporal/temporal.h"
#include "temporal/lifting.h"
#include "temporal/meos_catalog.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "h3/h3index.h"

/*****************************************************************************
 * Validators
 *
 * Every binary_synced / mixed-type MEOS function calls one of these
 * before doing any real work; failure returns false and leaves an
 * error on the thread-local stack, matching the TCBUFFER pattern.
 *****************************************************************************/

/**
 * @brief Ensure that a (th3index, th3index) pair is valid — both are
 * the right temptype and share a meaningful time axis. The sync
 * check itself happens later in `tfunc_temporal_temporal`; we only
 * handle the null / temptype fences here.
 */
bool
ensure_valid_th3index_th3index(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TH3INDEX(temp1, false);
  VALIDATE_TH3INDEX(temp2, false);
  return true;
}

/**
 * @brief Ensure that a (th3index, H3Index) pair is valid.
 *
 * The cell argument carries h3 semantics: a value of 0 is the
 * conventional "invalid" sentinel (what `h3_is_valid_cell(0)`
 * returns `false` for). We reject it up front so callers cannot
 * e.g. test a temporal trajectory for "ever equal to the invalid
 * sentinel" without noticing.
 */
bool
ensure_valid_th3index_h3index(const Temporal *temp, H3Index cell)
{
  VALIDATE_TH3INDEX(temp, false);
  if (cell == (H3Index) 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3 cell argument must not be the invalid sentinel 0");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a (th3index, tgeogpoint) pair is valid.
 */
bool
ensure_valid_th3index_tgeogpoint(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TH3INDEX(temp1, false);
  if (! ensure_not_null((void *) temp2) ||
      ! ensure_temporal_isof_type((Temporal *) temp2, T_TGEOGPOINT))
    return false;
  return true;
}

/*****************************************************************************
 * Input / output
 *
 * th3index's on-disk representation is identical to tbigint's — the
 * only distinction is the MeosType tag. The type-specific parsers
 * delegate to the generic temporal parser and rely on the enclosing
 * SQL `CREATE FUNCTION th3index_in(...)` to pick the right oid.
 *****************************************************************************/

/**
 * @ingroup meos_h3_inout
 * @brief Parse a temporal H3 cell index from its Well-Known Text
 * representation.
 */
Temporal *
th3index_in(const char *str)
{
  if (! ensure_not_null((void *) str))
    return NULL;
  return temporal_parse(&str, T_TH3INDEX);
}

/**
 * @ingroup meos_h3_inout
 * @brief Parse a temporal H3 cell instant from its Well-Known Text
 * representation.
 */
TInstant *
th3indexinst_in(const char *str)
{
  Temporal *temp = th3index_in(str);
  if (! temp)
    return NULL;
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_h3_inout
 * @brief Parse a temporal H3 cell sequence from its Well-Known Text
 * representation.
 *
 * th3index sequences always carry step interpolation (h3 cells are
 * discrete); the `interp` argument is accepted for signature
 * parity with the generic API and discarded.
 */
TSequence *
th3indexseq_in(const char *str, interpType interp)
{
  (void) interp;
  Temporal *temp = th3index_in(str);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_h3_inout
 * @brief Parse a temporal H3 cell sequence set from its Well-Known Text
 * representation.
 */
TSequenceSet *
th3indexseqset_in(const char *str)
{
  Temporal *temp = th3index_in(str);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}

/*****************************************************************************
 * Constructors
 *
 * These pack an `H3Index` (uint64) into an `Int64` Datum; the
 * storage is identical to tbigint.
 *****************************************************************************/

/**
 * @ingroup meos_h3_constructor
 * @brief Build a temporal H3 cell instant.
 * @param[in] value The H3 cell value
 * @param[in] t The instant's timestamp
 */
TInstant *
th3indexinst_make(H3Index value, TimestampTz t)
{
  return tinstant_make(H3IndexGetDatum(value), T_TH3INDEX, t);
}

/**
 * @ingroup meos_h3_constructor
 * @brief Build a temporal H3 cell sequence (step interpolation only
 * — h3 cells are discrete, never interpolated).
 * @param[in] values Array of H3 cell values
 * @param[in] times Array of timestamps, same length as `values`
 * @param[in] count Number of elements in the arrays
 * @param[in] lower_inc Lower bound inclusivity
 * @param[in] upper_inc Upper bound inclusivity
 */
TSequence *
th3indexseq_make(const H3Index *values, const TimestampTz *times,
  int count, bool lower_inc, bool upper_inc)
{
  if (! ensure_not_null((void *) values) ||
      ! ensure_not_null((void *) times) ||
      ! ensure_positive(count))
    return NULL;

  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; ++i)
    instants[i] = tinstant_make(H3IndexGetDatum(values[i]),
      T_TH3INDEX, times[i]);
  TSequence *result = tsequence_make(instants, count,
    lower_inc, upper_inc, STEP, NORMALIZE);
  for (int i = 0; i < count; ++i)
    pfree(instants[i]);
  pfree(instants);
  return result;
}

/**
 * @ingroup meos_h3_constructor
 * @brief Build a temporal H3 cell sequence set from an array of sequences.
 * @param[in] sequences Array of sequences (step interpolation)
 * @param[in] count Number of sequences
 */
TSequenceSet *
th3indexseqset_make(const TSequence **sequences, int count)
{
  return tsequenceset_make((TSequence **) sequences, count, NORMALIZE);
}

/**
 * @ingroup meos_h3_constructor
 * @brief Shorthand constructor: the single-instant temporal H3 cell.
 * Alias for `th3indexinst_make` — matches the cbuffer convention.
 */
Temporal *
th3index_make(H3Index value, TimestampTz t)
{
  return (Temporal *) th3indexinst_make(value, t);
}

/*****************************************************************************
 * Accessors — hide the Datum-packing convention from the public API.
 *****************************************************************************/

/**
 * @ingroup meos_h3_accessor
 * @brief Return the H3 cell value at the first instant of `temp`.
 */
H3Index
th3index_start_value(const Temporal *temp)
{
  VALIDATE_TH3INDEX(temp, (H3Index) 0);
  return DatumGetH3Index(temporal_start_value(temp));
}

/**
 * @ingroup meos_h3_accessor
 * @brief Return the H3 cell value at the last instant of `temp`.
 */
H3Index
th3index_end_value(const Temporal *temp)
{
  VALIDATE_TH3INDEX(temp, (H3Index) 0);
  return DatumGetH3Index(temporal_end_value(temp));
}

/**
 * @ingroup meos_h3_accessor
 * @brief Return the H3 cell value at the `n`-th distinct value of `temp`.
 * 1-indexed, following the cbuffer convention.
 * @return `true` on success, `false` if `n` is out of range.
 */
bool
th3index_value_n(const Temporal *temp, int n, H3Index *result)
{
  VALIDATE_TH3INDEX(temp, false);
  if (! ensure_not_null((void *) result))
    return false;
  Datum d;
  if (! temporal_value_n(temp, n, &d))
    return false;
  *result = DatumGetH3Index(d);
  return true;
}

/**
 * @ingroup meos_h3_accessor
 * @brief Return the distinct H3 cell values that `temp` takes, in
 * ascending order.
 * @param[in] temp Temporal H3 cell
 * @param[out] count Number of distinct values
 * @return Allocated array of `count` H3Index values. Caller owns.
 */
H3Index *
th3index_values(const Temporal *temp, int *count)
{
  VALIDATE_TH3INDEX(temp, NULL);
  if (! ensure_not_null((void *) count))
    return NULL;
  Datum *datums = temporal_values(temp, count);
  if (datums == NULL)
    return NULL;
  H3Index *result = palloc(sizeof(H3Index) * (*count));
  for (int i = 0; i < *count; ++i)
    result[i] = DatumGetH3Index(datums[i]);
  pfree(datums);
  return result;
}

/**
 * @ingroup meos_h3_accessor
 * @brief Return the H3 cell value of `temp` at timestamp `t`.
 * @param[in] temp Temporal H3 cell
 * @param[in] t Timestamp
 * @param[in] strict True to require an exact instant match; false
 * to return the value at the containing sequence under step
 * interpolation.
 * @param[out] result The H3 cell value at `t` (only written on success)
 * @return `true` on success, `false` if `t` lies outside `temp`.
 */
bool
th3index_value_at_timestamptz(const Temporal *temp, TimestampTz t,
  bool strict, H3Index *result)
{
  VALIDATE_TH3INDEX(temp, false);
  if (! ensure_not_null((void *) result))
    return false;
  Datum d;
  if (! temporal_value_at_timestamptz(temp, t, strict, &d))
    return false;
  *result = DatumGetH3Index(d);
  return true;
}

/*****************************************************************************
 * MEOS-level conversions between th3index and tbigint
 *
 * The int64 payload is identical between tbigint and h3index basetypes,
 * but the embedded bounding box differs: tbigint sequences carry a TBox
 * while th3index sequences carry an STBox. The conversion is therefore
 * implemented by lifting an identity Datum function with the target
 * restype so that tinstant/tsequence/tsequenceset are rebuilt at the
 * correct shape and the bbox is recomputed from the new basetype.
 *****************************************************************************/

static Datum
datum_h3index_identity(Datum d)
{
  return d;
}

/**
 * @ingroup meos_h3_conversion
 * @brief Convert a `tbigint` to a `th3index`. Caller owns the result.
 */
Temporal *
tbigint_to_th3index(const Temporal *temp)
{
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type((Temporal *) temp, T_TBIGINT))
    return NULL;
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_h3index_identity;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TBIGINT;
  lfinfo.restype = T_TH3INDEX;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_h3_conversion
 * @brief Convert a `th3index` to a `tbigint`. Caller owns the result.
 */
Temporal *
th3index_to_tbigint(const Temporal *temp)
{
  VALIDATE_TH3INDEX(temp, NULL);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_h3index_identity;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TBIGINT;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * Comparison-primitive wrappers used by th3index_compops.c
 *
 * Equality / inequality for h3 cells is bit equality at the int64
 * level — the `datum2_eq` / `datum2_ne` used by the generic tbigint
 * machinery already covers that. We only need thin type-correct
 * symbols so the compops dispatcher can pass them through.
 *****************************************************************************/

Datum
datum2_h3index_eq(Datum d1, Datum d2, MeosType type)
{
  (void) type;
  return BoolGetDatum(DatumGetH3Index(d1) == DatumGetH3Index(d2));
}

Datum
datum2_h3index_ne(Datum d1, Datum d2, MeosType type)
{
  (void) type;
  return BoolGetDatum(DatumGetH3Index(d1) != DatumGetH3Index(d2));
}

/*****************************************************************************/
