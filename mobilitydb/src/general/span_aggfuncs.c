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
 * @brief Aggregate function for span types.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MobilityDB */
#include <meos.h>
#include "general/span.h"

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Span_extent_transfn);
/**
 * Transition function for extent aggregation of span values
 */
PGDLLEXPORT Datum
Span_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  Span *s2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_SPAN_P(1);
  Span *result;

  /* Can't do anything with null inputs */
  if (! s1 && ! s2)
    PG_RETURN_NULL();
  /* Null span and non-null span, return the span */
  else if (! s1)
    result = span_copy(s2);
  /* Non-null span and null span, return the span */
  else if (! s2)
    result = span_copy(s1);
  else
    /* Non-strict union */
    result = union_span_span(s1, s2, false);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_extent_combinefn);
/**
 * Combine function for temporal extent aggregation
 */
PGDLLEXPORT Datum
Span_extent_combinefn(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  Span *s2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_SPAN_P(1);
  if (! s2 && ! s1)
    PG_RETURN_NULL();
  if (s1 && ! s2)
    PG_RETURN_POINTER(s1);
  if (s2 && ! s1)
    PG_RETURN_POINTER(s2);
  /* Non-strict union */
  Span *result = union_span_span(s1, s2, false);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

