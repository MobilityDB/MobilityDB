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
 * @file
 * @brief Ever comparison operators (?=, ?<>, ?<, ?>, ?<=, ?>=),
 * always comparison operators (%=, %<>, %<, %>, %<=, %>=), and
 * temporal comparison operators (#=, #<>, #<, #>, #<=, #>=).
 */

#ifndef __TEMPORAL_COMPOPS_H__
#define __TEMPORAL_COMPOPS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h"

/*****************************************************************************/

extern int eacomp_base_temporal(Datum value, const Temporal *temp,
  Datum (*func)(Datum, Datum, meosType), bool ever);
extern int eacomp_temporal_base(const Temporal *temp, Datum value,
  Datum (*func)(Datum, Datum, meosType), bool ever);
extern int eacomp_temporal_temporal(const Temporal *temp1,
  const Temporal *temp2, Datum (*func)(Datum, Datum, meosType), bool ever);

extern Temporal *tcomp_base_temporal(Datum value, const Temporal *temp,
  Datum (*func)(Datum, Datum, meosType));
extern Temporal *tcomp_temporal_base(const Temporal *temp, Datum value,
  Datum (*func)(Datum, Datum, meosType));
extern Temporal *tcomp_temporal_temporal(const Temporal *temp1,
  const Temporal *temp2, Datum (*func)(Datum, Datum, meosType));

/*****************************************************************************/

#endif /* __TEMPORAL_COMPOPS_H__ */
