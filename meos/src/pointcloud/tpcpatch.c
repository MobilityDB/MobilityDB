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
 * @brief Temporal pgpointcloud patch value surface — the Temporal<T> value bridge
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
#include "pointcloud/pcpatch.h"

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_constructor
 * @brief Return a temporal pgpointcloud patch instant from a pgpointcloud
 * patch and a timestamptz
 * @param[in] pa Value
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
tpcpatchinst_make(const Pcpatch *pa, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, NULL);
  return tinstant_make(PointerGetDatum(pa), T_TPCPATCH, t);
}

/**
 * @ingroup meos_pointcloud_constructor
 * @brief Return a temporal pgpointcloud patch discrete sequence from a
 * pgpointcloud patch value and a timestamptz set
 * @param[in] pa Value
 * @param[in] s Set
 * @csqlfn #Tsequence_from_base_tstzset()
 */
TSequence *
tpcpatchseq_from_base_tstzset(const Pcpatch *pa, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, NULL); VALIDATE_TSTZSET(s, NULL);
  return tsequence_from_base_tstzset(PointerGetDatum(pa), T_TPCPATCH, s);
}

/**
 * @ingroup meos_pointcloud_constructor
 * @brief Return a temporal pgpointcloud patch sequence from a pgpointcloud
 * patch value and a timestamptz span
 * @param[in] pa Value
 * @param[in] sp Span
 * @csqlfn #Tsequence_from_base_tstzspan()
 */
TSequence *
tpcpatchseq_from_base_tstzspan(const Pcpatch *pa, const Span *sp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, NULL); VALIDATE_TSTZSPAN(sp, NULL);
  return tsequence_from_base_tstzspan(PointerGetDatum(pa), T_TPCPATCH, sp,
    STEP);
}

/**
 * @ingroup meos_pointcloud_constructor
 * @brief Return a temporal pgpointcloud patch sequence set from a pgpointcloud
 * patch value and a timestamptz span set
 * @param[in] pa Value
 * @param[in] ss Span set
 * @csqlfn #Tsequenceset_from_base_tstzspanset()
 */
TSequenceSet *
tpcpatchseqset_from_base_tstzspanset(const Pcpatch *pa, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, NULL);
  VALIDATE_TSTZSPANSET(ss, NULL);
  /* Delegate to the generic tsequenceset constructor, with STEP interpolation */
  return tsequenceset_from_base_tstzspanset(PointerGetDatum(pa),T_TPCPATCH, ss,
    STEP);
}

/**
 * @ingroup meos_pointcloud_constructor
 * @brief Return a temporal pgpointcloud patch from a pgpointcloud patch and
 * the time frame of another temporal value
 * @param[in] pa Value
 * @param[in] temp Temporal value
 */
Temporal *
tpcpatch_from_base_temp(const Pcpatch *pa, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, NULL); VALIDATE_NOT_NULL(temp, NULL);
  return temporal_from_base_temp(PointerGetDatum(pa), T_TPCPATCH, temp);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return the start value of a temporal pgpointcloud patch
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_value()
 */
Pcpatch *
tpcpatch_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPATCH(temp, NULL);
  return DatumGetPcpatchP(temporal_start_value(temp));
}

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return the end value of a temporal pgpointcloud patch
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_value()
 */
Pcpatch *
tpcpatch_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPATCH(temp, NULL);
  return DatumGetPcpatchP(temporal_end_value(temp));
}

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return the n-th value of a temporal pgpointcloud patch
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @csqlfn #Temporal_value_n()
 */
bool
tpcpatch_value_n(const Temporal *temp, int n, Pcpatch **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPATCH(temp, false); VALIDATE_NOT_NULL(result, false);
  Datum dresult;
  if (! temporal_value_n(temp, n, &dresult))
    return false;
  *result = DatumGetPcpatchP(dresult);
  return true;
}

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return an array of copies of base values of a temporal pgpointcloud
 * patch
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
Pcpatch **
tpcpatch_values(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPATCH(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  Datum *datumarr = temporal_values_p(temp, count);
  Pcpatch **result = palloc(sizeof(Pcpatch *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = pcpatch_copy(DatumGetPcpatchP(datumarr[i]));
  pfree(datumarr);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return the value of a temporal pgpointcloud patch at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] value Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tpcpatch_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  Pcpatch **value)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPATCH(temp, false); VALIDATE_NOT_NULL(value, false);
  Datum res;
  bool result = temporal_value_at_timestamptz(temp, t, strict, &res);
  *value = DatumGetPcpatchP(res);
  return result;
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_restrict
 * @brief Return a temporal pgpointcloud patch restricted to a pgpointcloud
 * patch value
 * @param[in] temp Temporal value
 * @param[in] pa pgpointcloud patch value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tpcpatch_at_value(const Temporal *temp, const Pcpatch *pa)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPATCH(temp, NULL); VALIDATE_NOT_NULL(pa, NULL);
  /* Restrict the temporal pgpointcloud patch to the instants where it equals
   * the given pa */
  return temporal_restrict_value(temp, PointerGetDatum(pa), REST_AT);
}

/**
 * @ingroup meos_pointcloud_restrict
 * @brief Return a temporal pgpointcloud patch restricted to the complement of
 * a pgpointcloud patch value
 * @param[in] temp Temporal value
 * @param[in] pa pgpointcloud patch value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
tpcpatch_minus_value(const Temporal *temp, const Pcpatch *pa)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPATCH(temp, NULL); VALIDATE_NOT_NULL(pa, NULL);
  /* Restrict the temporal pgpointcloud patch to the instants where it does not
   * equal the given pa */
  return temporal_restrict_value(temp, PointerGetDatum(pa), REST_MINUS);
}

/*****************************************************************************/
