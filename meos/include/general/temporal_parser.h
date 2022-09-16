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
 * @brief Functions for parsing temporal types.
 */

#ifndef __TEMPORAL_PARSER_H__
#define __TEMPORAL_PARSER_H__

/* MobilityDB */
#include "general/temporal_catalog.h"
#include "general/timetypes.h"
#include "general/span.h"
#include "general/temporal.h"

/*****************************************************************************/

extern void ensure_end_input(const char **str, bool end, const char *type);

extern void p_whitespace(const char **str);
extern bool p_obrace(const char **str);
extern bool p_cbrace(const char **str);
extern bool p_obracket(const char **str);
extern bool p_cbracket(const char **str);
extern bool p_oparen(const char **str);
extern bool p_cparen(const char **str);
extern bool p_comma(const char **str);
extern Datum basetype_parse(const char **str, mobdbType basetypid);
extern double double_parse(const char **str);
extern TimestampTz timestamp_parse(const char **str);

extern TBOX *tbox_parse(const char **str);
extern TimestampSet *timestampset_parse(const char **str);
extern Period *period_parse(const char **str, bool make);
extern PeriodSet *periodset_parse(const char **str);
extern Datum elem_parse(const char **str, mobdbType basetype);
extern Span *span_parse(const char **str, mobdbType spantype, bool end, bool make);

extern TInstant *tinstant_parse(const char **str, mobdbType temptype, bool end,
  bool make);
extern TSequence *tdiscseq_parse(const char **str, mobdbType temptype);
extern TSequence *tcontseq_parse(const char **str, mobdbType temptype,
  interpType interp, bool end, bool make);
extern TSequenceSet *tsequenceset_parse(const char **str, mobdbType temptype,
  interpType interp);
extern Temporal *temporal_parse(const char **str, mobdbType temptype);

/*****************************************************************************/

#endif
