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
 * @brief Functions for parsing temporal types.
 */

#ifndef __TEMPORAL_PARSER_H__
#define __TEMPORAL_PARSER_H__

/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h"

/*****************************************************************************/

extern bool ensure_end_input(const char **str, const char *type);
extern void p_whitespace(const char **str);
extern bool p_obrace(const char **str);
extern bool ensure_obrace(const char **str, const char *type);
extern bool p_cbrace(const char **str);
extern bool ensure_cbrace(const char **str, const char *type);
extern bool p_obracket(const char **str);
extern bool p_cbracket(const char **str);
extern bool p_oparen(const char **str);
extern bool ensure_oparen(const char **str, const char *type);
extern bool p_cparen(const char **str);
extern bool ensure_cparen(const char **str, const char *type);
extern bool p_comma(const char **str);
extern bool temporal_basetype_parse(const char **str, meosType basetypid, Datum *result);
extern bool double_parse(const char **str, double *result);
extern bool elem_parse(const char **str, meosType basetype, Datum *result);
extern Set *set_parse(const char **str, meosType basetype);
extern bool span_parse(const char **str, meosType spantype, bool end, Span *span);
extern SpanSet *spanset_parse(const char **str, meosType spantype);
extern TBox *tbox_parse(const char **str);
extern TimestampTz timestamp_parse(const char **str);
extern bool tinstant_parse(const char **str, meosType temptype, bool end,
  TInstant **result);
extern TSequence *tdiscseq_parse(const char **str, meosType temptype);
extern bool tcontseq_parse(const char **str, meosType temptype, interpType interp,
  bool end, TSequence **result);
extern TSequenceSet *tsequenceset_parse(const char **str, meosType temptype,
  interpType interp);
extern Temporal *temporal_parse(const char **str, meosType temptype);

/*****************************************************************************/

#endif
