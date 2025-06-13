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
 * @brief Spatiotemporal relationships for temporal points.
 */

#ifndef __TGEO_TEMPSPATIALRELS_H__
#define __TGEO_TEMPSPATIALRELS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"

/* Compute either the tintersects or the tdisjoint relationship */
#define TINTERSECTS true
#define TDISJOINT   false

/*****************************************************************************/

extern Temporal *tspatialrel_tspatial_base(const Temporal *temp,
  Datum base, Datum param, varfunc func, int numparam, bool invert);
extern Temporal *tspatialrel_tspatial_tspatial_int(const Temporal *temp1,
  const Temporal *temp2, Datum param, varfunc func, int numparam, bool invert);

extern Temporal *tinterrel_tgeo_geo(const Temporal *temp,
  const GSERIALIZED *gs, bool tinter, bool restr, bool atvalue);
extern Temporal *tinterrel_tspatial_base(const Temporal *temp, Datum base,
  bool tinter, bool restr, bool atvalue, datum_func2 func);

extern Temporal *tinterrel_tspatial_base(const Temporal *temp,
  Datum base, bool tinter, bool restr, bool atvalue, datum_func2 func);
extern Temporal *tinterrel_tspatial_tspatial(const Temporal *temp1,
  const Temporal *temp2, bool tinter, bool restr, bool atvalue);

extern Temporal *tdwithin_tspatial_tspatial(const Temporal *sync1,
  const Temporal *sync2, Datum dist, bool restr, bool atvalue,
  datum_func3 func, tpfunc_temp tpfn);

extern int tdwithin_add_solutions(int solutions, TimestampTz lower,
  TimestampTz upper, bool lower_inc, bool upper_inc, bool upper_inc1,
  TimestampTz t1, TimestampTz t2, TInstant **instants, TSequence **result);
extern Temporal *tdwithin_tspatial_spatial(const Temporal *temp, Datum base,
  Datum dist, bool restr, bool atvalue, datum_func3 func, tpfunc_temp tpfn);
extern Temporal *tdwithin_tspatial_tspatial(const Temporal *sync1,
  const Temporal *sync2, Datum dist, bool restr, bool atvalue,
  datum_func3 func, tpfunc_temp tpfn);

/*****************************************************************************/

#endif /* __TGEO_TEMPSPATIALRELS_H__ */
