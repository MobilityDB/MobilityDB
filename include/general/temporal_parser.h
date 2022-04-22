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
 * @file temporal_parser.h
 * Functions for parsing temporal types.
 */

#ifndef __TEMPORAL_PARSER_H__
#define __TEMPORAL_PARSER_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
/* MobilityDB */
#include "general/tempcache.h"
#include "general/timetypes.h"
#include "general/temporal.h"

/*****************************************************************************/

extern void ensure_end_input(char **str, bool end);

extern void p_whitespace(char **str);
extern bool p_obrace(char **str);
extern bool p_cbrace(char **str);
extern bool p_obracket(char **str);
extern bool p_cbracket(char **str);
extern bool p_oparen(char **str);
extern bool p_cparen(char **str);
extern bool p_comma(char **str);
extern Datum basetype_parse(char **str, Oid basetypid);
extern double double_parse(char **str);
extern TimestampTz timestamp_parse(char **str);

extern TBOX *tbox_parse(char **str);
extern TimestampSet *timestampset_parse(char **str);
extern Period *period_parse(char **str, bool make);
extern PeriodSet *periodset_parse(char **str);
extern TInstant *tinstant_parse(char **str, CachedType temptype, bool end,
  bool make);
extern TInstantSet *tinstantset_parse(char **str, CachedType temptype);
extern TSequence *tsequence_parse(char **str, CachedType temptype, bool linear,
  bool end, bool make);
extern TSequenceSet *tsequenceset_parse(char **str, CachedType temptype,
  bool linear);
extern Temporal *temporal_parse(char **str, CachedType temptype);

/*****************************************************************************/

#endif
