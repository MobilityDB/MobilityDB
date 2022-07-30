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
 * @brief Temporal mathematical operators (+, -, *, /) and functions (round,
 * degrees).
 */

#ifndef __TEMPORAL_MATHFUNCS_H__
#define __TEMPORAL_MATHFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* PostgreSQL */
#include "general/temporal.h"

/*****************************************************************************/

/** Enumeration for the arithmetic functions */

typedef enum
{
  ADD,
  SUB,
  MULT,
  DIV,
  DIST,
} TArithmetic;

/*****************************************************************************/

extern bool tnumber_mult_tp_at_timestamp(const TInstant *start1,
  const TInstant *end1, const TInstant *start2, const TInstant *end2,
  Datum *value, TimestampTz *t);
extern bool tnumber_div_tp_at_timestamp(const TInstant *start1,
  const TInstant *end1, const TInstant *start2, const TInstant *end2,
  Datum *value, TimestampTz *t);

extern Temporal *arithop_tnumber_number(const Temporal *temp, Datum value,
  mobdbType basetype, TArithmetic oper,
  Datum (*func)(Datum, Datum, mobdbType, mobdbType), bool invert);
extern Temporal *arithop_tnumber_tnumber(const Temporal *temp1,
  const Temporal *temp2, TArithmetic oper,
  Datum (*func)(Datum, Datum, mobdbType, mobdbType),
  bool (*tpfunc)(const TInstant *, const TInstant *, const TInstant *,
    const TInstant *, Datum *, TimestampTz *));

extern Temporal *tnumber_degrees(const Temporal *temp);

extern TSequence *tnumberseq_derivative(const TSequence *seq);
extern TSequenceSet *tnumberseqset_derivative(const TSequenceSet *ts);
extern Temporal *tnumber_derivative(const Temporal *temp);

/*****************************************************************************/

#endif
