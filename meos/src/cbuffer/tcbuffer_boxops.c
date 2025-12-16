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
 * @brief Bounding box functions for temporal circular buffer
 * @details These functions are used for computing the bounding boxes upon
 * creation of the temporal circular buffers.
 */

#include "cbuffer/tcbuffer_boxops.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/span.h"
#include "cbuffer/cbuffer.h"

/*****************************************************************************
 * Transform a temporal circular buffer to a STBox
 *****************************************************************************/

/**
 * @brief Return in the last argument the spatiotemporal box of a temporal
 * circular buffer instant
 * @param[in] inst Temporal circular buffer
 * @param[out] box Spatiotemporal box
 */
void
tcbufferinst_set_stbox(const TInstant *inst, STBox *box)
{
  cbuffer_set_stbox(DatumGetCbufferP(tinstant_value_p(inst)), box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Return in the last argument a spatiotemporal box constructed from
 * an array of temporal circular buffer instants
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tcbufferinstarr_set_stbox(TInstant **instants, int count, STBox *box)
{
  tcbufferinst_set_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    tcbufferinst_set_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * @brief Expand the temporal box of a temporal circular buffer sequence with an
 * instant
 * @param[in] seq Temporal sequence
 * @param[in] inst Temporal instant
 */
void
tcbufferseq_expand_stbox(const TSequence *seq, const TInstant *inst)
{
  /* Compute the bounding box of the end point of the sequence and the instant */
  STBox box;
  tcbufferinst_set_stbox(inst, &box);
  /* Expand the bounding box of the sequence with the last edge */
  stbox_expand(&box, (STBox *) TSEQUENCE_BBOX_PTR(seq));
  return;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * circular buffer and a timestamptz
 * @param[in] cb Circular buffer
 * @param[in] t Timestamp
 * @param[out] box Spatiotemporal box
 */
bool
cbuffer_timestamptz_set_stbox(const Cbuffer *cb, TimestampTz t, STBox *box)
{
  cbuffer_set_stbox(cb, box);
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return true;
}

/**
 * @ingroup meos_cbuffer_box
 * @brief Return a spatiotemporal box constructed from a circular buffer and a
 * timestamptz
 * @param[in] cb Circular buffer
 * @param[in] t Timestamp
 * @csqlfn #Cbuffer_timestamptz_to_stbox()
 */
STBox *
cbuffer_timestamptz_to_stbox(const Cbuffer *cb, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  STBox box;
  if (! cbuffer_timestamptz_set_stbox(cb, t, &box))
    return NULL;
  return stbox_copy(&box);
}

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * circular buffer and a timestamptz span
 * @param[in] cb Circular buffer
 * @param[in] s Timestamptz span
 * @param[out] box Spatiotemporal box
 */
bool
cbuffer_tstzspan_set_stbox(const Cbuffer *cb, const Span *s, STBox *box)
{
  cbuffer_set_stbox(cb, box);
  memcpy(&box->period, s, sizeof(Span));
  MEOS_FLAGS_SET_T(box->flags, true);
  return true;
}

/**
 * @ingroup meos_cbuffer_box
 * @brief Return a spatiotemporal box constructed from a circular buffer and a
 * timestamptz
 * @param[in] cb Circular buffer
 * @param[in] s Timestamptz span
 * @csqlfn #Cbuffer_tstzspan_to_stbox()
 */
STBox *
cbuffer_tstzspan_to_stbox(const Cbuffer *cb, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL); VALIDATE_TSTZSPAN(s, NULL);
  STBox box;
  if (! cbuffer_tstzspan_set_stbox(cb, s, &box))
    return NULL;
  return stbox_copy(&box);
}

/*****************************************************************************/
