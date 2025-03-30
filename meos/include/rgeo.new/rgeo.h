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
 * documentation for any purrgeo, without fee, and without a written
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
 * AND FITNESS FOR A PARTICULAR PURRGEO. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @brief Basic functions for static rgeo objects.
 */

#ifndef __RGEO_H__
#define __RGEO_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/**
 * Structure to represent rgeo values
 *
 * flags (8 bits, x = unused): xxZXxxxx
 * data: 2D: [x, y, theta]
 *       3D: [x, y, z, W, X, Z, Y]
 *
 */
struct Rgeo
{
  int32         vl_len_;       /**< varlena header (do not touch directly!) */
  int8          flags;         /**< flags */
  uint8_t       srid[3];       /**< srid */
  double        data[];        /**< position and orientation values */
};

/*****************************************************************************
 * rgeo.c
 *****************************************************************************/

/* Input/output functions */

extern char *rgeo_wkt_out(Datum value, bool extended, int maxdd);

extern Rgeo *rgeo_parse(const char **str, bool end);

extern Datum datum_rgeo_point(Datum rgeo);
extern Datum datum_rgeo_rotation(Datum rgeo);

/* Transformation functions */

extern Datum datum_rgeo_round(Datum rgeo, Datum size);
extern Rgeo **rgeoarr_round(const Rgeo **rgeoarr, int count, int maxdd);
extern Temporal *trgeo_round(const Temporal *temp, int maxdd);

/* Distance */

extern Datum rgeo_distance(Datum rgeo1, Datum rgeo2);

/* Interpolation */

extern Rgeo *rgeo_interpolate(const Rgeo *rgeo1, const Rgeo *rgeo2, double ratio);
extern bool rgeo_collinear(const Rgeo *rgeo1, const Rgeo *rgeo2, const Rgeo *rgeo3, double ratio);

/* Interpolation */

extern bool rgeo_set_stbox(const Rgeo *rgeo, STBox *box);
extern void rgeoarr_set_stbox(const Datum *values, int count, STBox *box);
extern bool rgeo_timestamptz_set_stbox(const Rgeo *rgeo, TimestampTz t,
  STBox *box);
extern bool rgeo_tstzspan_set_stbox(const Rgeo *rgeo, const Span *p,
  STBox *box);

/* Comparison functions */

extern bool rgeo_eq(const Rgeo *rgeo1, const Rgeo *rgeo2);
extern bool rgeo_ne(const Rgeo *rgeo1, const Rgeo *rgeo2);

extern bool rgeo_same(const Rgeo *rgeo1, const Rgeo *rgeo2);
extern bool rgeo_nsame(const Rgeo *rgeo1, const Rgeo *rgeo2);

extern int rgeo_cmp(const Rgeo *rgeo1, const Rgeo *rgeo2);
extern bool rgeo_lt(const Rgeo *rgeo1, const Rgeo *rgeo2);
extern bool rgeo_le(const Rgeo *rgeo1, const Rgeo *rgeo2);
extern bool rgeo_gt(const Rgeo *rgeo1, const Rgeo *rgeo2);
extern bool rgeo_ge(const Rgeo *rgeo1, const Rgeo *rgeo2);

/* Hash functions */

extern uint32 rgeo_hash(const Rgeo *rgeo);
extern uint64 rgeo_hash_extended(const Rgeo *rgeo, uint64 seed);

/*****************************************************************************/

#endif /* __RGEO_H__ */
