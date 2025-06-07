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
 * @brief Functions for temporal buffers.
 */

#ifndef __CBUFFER_H__
#define __CBUFFER_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_cbuffer.h>

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/* Structure to represent circular buffers */

struct Cbuffer
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  double radius;        /**< radius */
  Datum point;          /**< First 8 bytes of the point which is passed by 
                             reference. The extra bytes needed are added upon 
                             creation. */
  /* variable-length data follows */
};

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetCbufferP(X)         ((Cbuffer *) DatumGetPointer(X))
#define CbufferPGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_CBUFFER_P(X)      DatumGetCbufferP(PG_GETARG_DATUM(X))
#define PG_RETURN_CBUFFER_P(X)      PG_RETURN_POINTER(X)

/*****************************************************************************/

/* Validity functions */

extern bool ensure_valid_cbuffer_cbuffer(const Cbuffer *cb1,
  const Cbuffer *cb2);
extern bool ensure_valid_cbuffer_geo(const Cbuffer *cb,
  const GSERIALIZED *gs);
extern bool ensure_valid_cbuffer_stbox(const Cbuffer *cb, const STBox *box);
extern bool ensure_valid_cbufferset_cbuffer(const Set *s, const Cbuffer *cb);

/* Collinear and interpolation functions */

extern bool cbuffer_collinear(const Cbuffer *cb1, const Cbuffer *cb2,
  const Cbuffer *cbuf3, double ratio);
extern Cbuffer *cbuffersegm_interpolate(const Cbuffer *start,
  const Cbuffer *end, long double ratio);
extern long double cbuffersegm_locate(const Cbuffer *start, const Cbuffer *end,
  const Cbuffer *value);

/* Input/output functions */

extern Cbuffer *cbuffer_parse(const char **str, bool end);
extern char *cbuffer_wkt_out(Datum value, int maxdd, bool extended);

/* Accessor functions */

extern const GSERIALIZED *cbuffer_point_p(const Cbuffer *cb);

extern Datum datum_cbuffer_round(Datum buffer, Datum size);

/* Transformation functions */

extern Cbuffer *cbuffer_transf_pj(const Cbuffer *cb, int32_t srid_to, const LWPROJ *pj);

/* Distance function */

extern double cbuffer_distance(const Cbuffer *cb1, const Cbuffer *cb2);
extern Datum datum_cbuffer_distance(Datum cb1, Datum cb2);
extern int cbuffersegm_distance_turnpt(const Cbuffer *start1,
  const Cbuffer *end1, const Cbuffer *start2, const Cbuffer *end2,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2);

/* Spatial relationship functions */

extern int cbuffer_contains(const Cbuffer *cb1, const Cbuffer *cb2);
extern int cbuffer_covers(const Cbuffer *cb1, const Cbuffer *cb2);
extern int cbuffer_disjoint(const Cbuffer *cb1, const Cbuffer *cb2);
extern int cbuffer_intersects(const Cbuffer *cb1, const Cbuffer *cb2);
extern int cbuffer_dwithin(const Cbuffer *cb1, const Cbuffer *cb2, double dist);
extern int cbuffer_touches(const Cbuffer *cb1, const Cbuffer *cb2);

extern int contains_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2);
extern int covers_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2);
extern int disjoint_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2);
extern int intersects_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2);
extern int dwithin_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2, double dist);
extern int touches_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2);

extern Datum datum_cbuffer_contains(Datum cb1, Datum cb2);
extern Datum datum_cbuffer_covers(Datum cb1, Datum cb2);
extern Datum datum_cbuffer_disjoint(Datum cb1, Datum cb2);
extern Datum datum_cbuffer_intersects(Datum cb1, Datum cb2);
extern Datum datum_cbuffer_dwithin(Datum cb1, Datum cb2, Datum dist);
extern Datum datum_cbuffer_touches(Datum cb1, Datum cb2);

/*****************************************************************************/

#endif /* __CBUFFER_H__ */
