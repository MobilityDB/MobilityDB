/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Bounding box operators for temporal types.
 */

#ifndef __PG_TEMPORAL_BOXOPS_H__
#define __PG_TEMPORAL_BOXOPS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/meos_catalog.h"
#include "general/temporal.h"
#include "general/span.h"
#include "general/tbox.h"
#include "point/stbox.h"

/*****************************************************************************/

extern Datum boxop_timestamp_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum boxop_temporal_timestamp_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum boxop_timestampset_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum boxop_temporal_timestampset_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum boxop_period_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum boxop_temporal_period_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum boxop_periodset_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum boxop_temporal_periodset_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum boxop_temporal_temporal_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));

extern Datum boxop_number_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));
extern Datum boxop_tnumber_number_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));
extern Datum boxop_numspan_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum boxop_tnumber_numspan_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum boxop_numspanset_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum boxop_tnumber_numspanset_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));
extern Datum boxop_tbox_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));
extern Datum boxop_tnumber_tbox_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));
extern Datum boxop_tnumber_tnumber_ext(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));

/*****************************************************************************/

#endif /* __PG_TEMPORAL_BOXOPS_H__ */
