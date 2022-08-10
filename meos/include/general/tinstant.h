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
 * @brief Basic functions for temporal instants.
 */

#ifndef __TINSTANT_H__
#define __TINSTANT_H__

/* PostgreSQL */
#include <postgres.h>
/* MobilityDB */
#include "general/temporal.h"
#include "general/span.h"
#include "general/temporal_catalog.h"

/*****************************************************************************/

/* General functions */

extern void tinstant_set(TInstant *inst, Datum value, TimestampTz t);
extern double tnumberinst_double(const TInstant *inst);

/* Input/output functions */

extern char *tinstant_to_string(const TInstant *inst, Datum arg,
  char *(*value_out)(mobdbType, Datum, Datum));

/* Restriction Functions */

extern bool tinstant_restrict_values_test(const TInstant *inst,
  const Datum *values, int count, bool atfunc);
extern bool tnumberinst_restrict_span_test(const TInstant *inst,
  const Span *span, bool atfunc);
extern bool tnumberinst_restrict_spans_test(const TInstant *inst,
  Span **normspans, int count, bool atfunc);
extern bool tinstant_restrict_timestampset_test(const TInstant *inst,
  const TimestampSet *ts, bool atfunc);
extern bool tinstant_restrict_periodset_test(const TInstant *inst,
  const PeriodSet *ps, bool atfunc);

/* Intersection function */

extern bool intersection_tinstant_tinstant(const TInstant *inst1,
  const TInstant *inst2, TInstant **inter1, TInstant **inter2);

/*****************************************************************************/

#endif
