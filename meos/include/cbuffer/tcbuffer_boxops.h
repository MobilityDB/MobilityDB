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
 * @brief Bounding box operators for temporal circular buffers.
 */

#ifndef __TCBUFFER_BOXOPS_H__
#define __TCBUFFER_BOXOPS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/temporal.h"
#include <meos_cbuffer.h>

/*****************************************************************************/

extern bool cbuffer_set_stbox(const Cbuffer *cbuf, STBox *box);
extern void cbufferarr_set_stbox(const Datum *values, int count, STBox *box);
extern bool cbuffer_timestamptz_set_stbox(const Cbuffer *cbuf, TimestampTz t,
  STBox *box);
extern bool cbuffer_tstzspan_set_stbox(const Cbuffer *cbuf, const Span *s,
  STBox *box);

extern void tcbufferinst_set_stbox(const TInstant *inst, STBox *box);
extern void tcbufferinstarr_set_stbox(const TInstant **instants, int count,
  STBox *box);
extern void tcbufferseq_expand_stbox(const TSequence *seq, const TInstant *inst);

/*****************************************************************************/

extern int boxop_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *geo,
  bool (*func)(const STBox *, const STBox *), bool invert);
extern int boxop_tcbuffer_stbox(const Temporal *temp, const STBox *box,
  bool (*func)(const STBox *, const STBox *), bool spatial, bool invert);
extern bool boxop_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf,
  bool (*func)(const STBox *, const STBox *), bool invert);
extern bool boxop_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const STBox *, const STBox *));

/*****************************************************************************/

#endif /* __TCBUFFER_BOXOPS_H__ */
