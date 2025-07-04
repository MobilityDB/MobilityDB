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
 * @file
 * @brief Route identifier operators for temporal network points.
 */

#ifndef __TNPOINT_ROUTEOPS_H__
#define __TNPOINT_ROUTEOPS_H__

/* PostgreSQL */
#include <postgres.h>
#if ! MEOS 
  #include <lib/stringinfo.h>
#endif /* ! MEOS */
/* MEOS */
#include "npoint/tnpoint.h"

/*****************************************************************************/

extern bool contains_rid_tnpoint_bigint(const Temporal *temp, int64 rid,
  bool invert UNUSED);
extern bool contained_rid_tnpoint_bigint(const Temporal *temp, int64 rid,
  bool invert);
extern bool same_rid_tnpoint_bigint(const Temporal *temp, int64 rid,
  bool invert UNUSED);
extern bool overlaps_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert UNUSED);
extern bool contains_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert);
extern bool contained_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert);
extern bool same_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert UNUSED);
extern bool contains_rid_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  bool invert UNUSED);
extern bool contained_rid_npoint_tnpoint(const Temporal *temp, const Npoint *np,
  bool invert);
extern bool same_rid_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  bool invert UNUSED);
extern bool overlaps_rid_tnpoint_tnpoint(const Temporal *temp1,
  const Temporal *temp2);
extern bool contains_rid_tnpoint_tnpoint(const Temporal *temp1,
  const Temporal *temp2);
extern bool contained_rid_tnpoint_tnpoint(const Temporal *temp1,
  const Temporal *temp2);
extern bool same_rid_tnpoint_tnpoint(const Temporal *temp1,
  const Temporal *temp2);

/*****************************************************************************/

#endif /* __TNPOINT_ROUTEOPS_H__ */
