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
 * @file tinstantset.h
 * Basic functions for temporal instant sets.
 */

#ifndef __TINSTANTSET_H__
#define __TINSTANTSET_H__

/* PostgreSQL */
#include <postgres.h>
/* MobilityDB */
#include "general/temporal.h"
#include "general/span.h"

/*****************************************************************************/

/* General functions */

extern void *tinstantset_bbox_ptr(const TInstantSet *is);
extern TInstantSet *tinstantset_make1(const TInstant **instants, int count);
extern bool tinstantset_find_timestamp(const TInstantSet *is, TimestampTz t,
  int *pos);

/* Input/output functions */

extern char *tinstantset_to_string(const TInstantSet *is,
  char *(*value_out)(mobdbType, Datum));

/* Intersection functions */

extern bool intersection_tinstantset_tinstant(const TInstantSet *is,
  const TInstant *inst, TInstant **inter1, TInstant **inter2);
extern bool intersection_tinstant_tinstantset(const TInstant *inst,
  const TInstantSet *is, TInstant **inter1, TInstant **inter2);
extern bool intersection_tinstantset_tinstantset(const TInstantSet *is1,
  const TInstantSet *is2, TInstantSet **inter1, TInstantSet **inter2);

/*****************************************************************************/

#if ! MEOS

/* Send/receive functions */

extern TInstantSet *tinstantset_recv(StringInfo buf, mobdbType temptype);
extern void tinstantset_write(const TInstantSet *ti, StringInfo buf);

#endif /* ! MEOS */

/*****************************************************************************/

#endif
