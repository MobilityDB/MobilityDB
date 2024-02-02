/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
#include <meos.h>

/*****************************************************************************/

extern Datum Boxop_timestamp_temporal(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum Boxop_temporal_timestamptz(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum Boxop_tstzset_temporal(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum Boxop_temporal_tstzset(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum Boxop_tstzspan_temporal(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum Boxop_temporal_tstzspan(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum Boxop_tstzspanset_temporal(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum Boxop_temporal_tstzspanset(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum Boxop_temporal_temporal(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));

extern Datum Boxop_number_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));
extern Datum Boxop_tnumber_number(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));
extern Datum Boxop_numspan_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum Boxop_tnumber_numspan(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum Boxop_numspanset_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *));
extern Datum Boxop_tnumber_numspanset(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));
extern Datum Boxop_tbox_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));
extern Datum Boxop_tnumber_tbox(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));
extern Datum Boxop_tnumber_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *));

/*****************************************************************************/

#endif /* __PG_TEMPORAL_BOXOPS_H__ */
