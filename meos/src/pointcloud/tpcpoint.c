/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Temporal pgpointcloud point value surface — the Temporal<T> value bridge
 *   (constructors, accessors, restrictions), generated from the tjsonb reference
 *   by tools/codegen/temporal_basetype/generate.py; DO NOT EDIT BY HAND.
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/meos_catalog.h"
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "pointcloud/pcpoint.h"

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_constructor
 * @brief Return a temporal pgpointcloud point instant from a pgpointcloud
 * point and a timestamptz
 * @param[in] pt Value
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
tpcpointinst_make(const Pcpoint *pt, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL);
  return tinstant_make(PointerGetDatum(pt), T_TPCPOINT, t);
}

/**
 * @ingroup meos_pointcloud_constructor
 * @brief Return a temporal pgpointcloud point discrete sequence from a
 * pgpointcloud point value and a timestamptz set
 * @param[in] pt Value
 * @param[in] s Set
 * @csqlfn #Tsequence_from_base_tstzset()
 */
TSequence *
tpcpointseq_from_base_tstzset(const Pcpoint *pt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL); VALIDATE_TSTZSET(s, NULL);
  return tsequence_from_base_tstzset(PointerGetDatum(pt), T_TPCPOINT, s);
}

/**
 * @ingroup meos_pointcloud_constructor
 * @brief Return a temporal pgpointcloud point sequence from a pgpointcloud
 * point value and a timestamptz span
 * @param[in] pt Value
 * @param[in] sp Span
 * @csqlfn #Tsequence_from_base_tstzspan()
 */
TSequence *
tpcpointseq_from_base_tstzspan(const Pcpoint *pt, const Span *sp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL); VALIDATE_TSTZSPAN(sp, NULL);
  return tsequence_from_base_tstzspan(PointerGetDatum(pt), T_TPCPOINT, sp, STEP);
}

/**
 * @ingroup meos_pointcloud_constructor
 * @brief Return a temporal pgpointcloud point sequence set from a pgpointcloud
 * point value and a timestamptz span set
 * @param[in] pt Value
 * @param[in] ss Span set
 * @csqlfn #Tsequenceset_from_base_tstzspanset()
 */
TSequenceSet *
tpcpointseqset_from_base_tstzspanset(const Pcpoint *pt, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL); VALIDATE_TSTZSPANSET(ss, NULL);
  /* Delegate to the generic tsequenceset constructor, with STEP interpolation */
  return tsequenceset_from_base_tstzspanset(PointerGetDatum(pt),T_TPCPOINT, ss,
    STEP);
}

/**
 * @ingroup meos_pointcloud_constructor
 * @brief Return a temporal pgpointcloud point from a pgpointcloud point and
 * the time frame of another temporal value
 * @param[in] pt Value
 * @param[in] temp Temporal value
 */
Temporal *
tpcpoint_from_base_temp(const Pcpoint *pt, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pt, NULL); VALIDATE_NOT_NULL(temp, NULL);
  return temporal_from_base_temp(PointerGetDatum(pt), T_TPCPOINT, temp);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return the start value of a temporal pgpointcloud point
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_value()
 */
Pcpoint *
tpcpoint_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, NULL);
  return DatumGetPcpointP(temporal_start_value(temp));
}

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return the end value of a temporal pgpointcloud point
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_value()
 */
Pcpoint *
tpcpoint_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, NULL);
  return DatumGetPcpointP(temporal_end_value(temp));
}

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return the n-th value of a temporal pgpointcloud point
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @csqlfn #Temporal_value_n()
 */
bool
tpcpoint_value_n(const Temporal *temp, int n, Pcpoint **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, false); VALIDATE_NOT_NULL(result, false);
  Datum dresult;
  if (! temporal_value_n(temp, n, &dresult))
    return false;
  *result = DatumGetPcpointP(dresult);
  return true;
}

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return an array of copies of base values of a temporal pgpointcloud
 * point
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
Pcpoint **
tpcpoint_values(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  Datum *datumarr = temporal_values_p(temp, count);
  Pcpoint **result = palloc(sizeof(Pcpoint *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = pcpoint_copy(DatumGetPcpointP(datumarr[i]));
  pfree(datumarr);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return the value of a temporal pgpointcloud point at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] value Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tpcpoint_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  Pcpoint **value)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, false); VALIDATE_NOT_NULL(value, false);
  Datum res;
  bool result = temporal_value_at_timestamptz(temp, t, strict, &res);
  *value = DatumGetPcpointP(res);
  return result;
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_restrict
 * @brief Return a temporal pgpointcloud point restricted to a pgpointcloud
 * point value
 * @param[in] temp Temporal value
 * @param[in] pt pgpointcloud point value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tpcpoint_at_value(const Temporal *temp, const Pcpoint *pt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, NULL); VALIDATE_NOT_NULL(pt, NULL);
  /* Restrict the temporal pgpointcloud point to the instants where it equals
   * the given pt */
  return temporal_restrict_value(temp, PointerGetDatum(pt), REST_AT);
}

/**
 * @ingroup meos_pointcloud_restrict
 * @brief Return a temporal pgpointcloud point restricted to the complement of
 * a pgpointcloud point value
 * @param[in] temp Temporal value
 * @param[in] pt pgpointcloud point value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
tpcpoint_minus_value(const Temporal *temp, const Pcpoint *pt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, NULL); VALIDATE_NOT_NULL(pt, NULL);
  /* Restrict the temporal pgpointcloud point to the instants where it does
   * not equal the given pt */
  return temporal_restrict_value(temp, PointerGetDatum(pt), REST_MINUS);
}

/*****************************************************************************/

