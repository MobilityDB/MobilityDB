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
 * @brief Mathematical operators (+, -, *, /) and functions (round, degrees, ...)
 * for temporal numbers.
 */

#ifndef __TEMPORAL_MATHFUNCS_H__
#define __TEMPORAL_MATHFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"
#include "temporal/meos_catalog.h"

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

extern int tfloat_arithop_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum param UNUSED, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);

extern Temporal *arithop_tnumber_number(const Temporal *temp, Datum value,
  TArithmetic oper, Datum (*func)(Datum, Datum, meosType), bool invert);
extern Temporal *arithop_tnumber_tnumber(const Temporal *temp1,
  const Temporal *temp2, TArithmetic oper,
  Datum (*func)(Datum, Datum, meosType), tpfunc_temp tpfunc);

/*****************************************************************************/

#endif
