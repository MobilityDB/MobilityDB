/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @file
 * @brief Multi-entry support of the temporal types: the keys of a value are
 * the boxes MEOS splits it into
 */

#ifndef __PG_TEMPORAL_MEST_H__
#define __PG_TEMPORAL_MEST_H__

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <access/gist.h>
#include <access/spgist.h>
#include <access/stratnum.h>
/* MEOS */
#include <meos.h>

/*****************************************************************************/

extern int mest_count(FunctionCallInfo fcinfo);
extern Datum *mest_keys(void *boxes, size_t size, int count);
extern Datum *mest_keys_whole(const Temporal *temp, void *boxes, size_t size,
  int32 *count);
extern bool mest_key_answers(int16 flags, StrategyNumber strategy);
extern StrategyNumber mest_pieces_strategy(StrategyNumber strategy);
extern ScanKey mest_pieces_scankeys(ScanKey keys, int nkeys);
extern void mest_duration_validate(const char *value);
extern Size mest_duration_fill(const char *value, void *ptr);

/*****************************************************************************/

#endif /* __PG_TEMPORAL_MEST_H__ */
