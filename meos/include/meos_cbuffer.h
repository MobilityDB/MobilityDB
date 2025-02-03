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
 * @brief API of the Mobility Engine Open Source (MEOS) library.
 */

#ifndef __MEOS_CBUFFER_H__
#define __MEOS_CBUFFER_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/* PostgreSQL */
#if MEOS
#include "postgres_int_defs.h"
#else
#include <postgres.h>
#include <utils/date.h>
#include <utils/timestamp.h>
#endif
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/* Structure to represent buffers */

typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  double radius;        /**< radius */
  Datum point;          /**< First 8 bytes of the point which is passed by 
                             reference. The extra bytes needed are added upon 
                             creation. */
  /* variable-length data follows */
} Cbuffer;

/******************************************************************************
 * Functions for buffer types
 ******************************************************************************/


extern char *cbuffer_as_text(const Cbuffer *cbuf, int maxdd);
extern char *cbuffer_as_ewkt(const Cbuffer *cbuf, int maxdd);
extern int cbuffer_cmp(const Cbuffer *cbuf1, const Cbuffer *cbuf2);
extern bool cbuffer_eq(const Cbuffer *cbuf1, const Cbuffer *cbuf2);
extern bool cbuffer_ge(const Cbuffer *cbuf1, const Cbuffer *cbuf2);
extern bool cbuffer_gt(const Cbuffer *cbuf1, const Cbuffer *cbuf2);
extern uint32 cbuffer_hash(const Cbuffer *cbuf);
extern Cbuffer *cbuffer_in(const char *str);
extern bool cbuffer_le(const Cbuffer *cbuf1, const Cbuffer *cbuf2);
extern bool cbuffer_lt(const Cbuffer *cbuf1, const Cbuffer *cbuf2);
extern Cbuffer *cbuffer_make(const GSERIALIZED *point, double radius);
extern bool cbuffer_ne(const Cbuffer *cbuf1, const Cbuffer *cbuf2);
extern char *cbuffer_out(const Cbuffer *cbuf, int maxdd);
extern double cbuffer_radius(const Cbuffer *cbuf);
extern const GSERIALIZED *cbuffer_point(const Cbuffer *cbuf);
extern int32_t cbuffer_srid(const Cbuffer *cbuf);
extern GSERIALIZED *cbuffer_to_geom(const Cbuffer *cbuf);
extern char **cbufferarr_as_text(const Datum *cbufarr, int count, int maxdd, bool extended);
extern Cbuffer *geom_to_cbuffer(const GSERIALIZED *gs);

/*===========================================================================*
 * Functions for box types
 *===========================================================================*/

/*****************************************************************************
 * Constructor functions for box types
 *****************************************************************************/

extern STBox *cbuffer_tstzspan_to_stbox(const Cbuffer *cbuf, const Span *s);
extern STBox *cbuffer_timestamptz_to_stbox(const Cbuffer *cbuf, TimestampTz t);

/*****************************************************************************
 * Conversion functions for box types
 *****************************************************************************/

extern STBox *cbuffer_to_stbox(const Cbuffer *cbuf);

/*===========================================================================*
 * Functions for temporal types
 *===========================================================================*/

/*****************************************************************************
 * Input/output functions for temporal types
 *****************************************************************************/

extern char *tcbuffer_as_text(const Temporal *temp, int maxdd);
extern char *tcbuffer_as_ewkt(const Temporal *temp, int maxdd);
extern char *tcbuffer_out(const Temporal *temp, int maxdd);
extern int32_t tcbuffer_srid(const Temporal *temp);
extern char **tcbufferarr_as_text(const Temporal **temparr, int count, int maxdd, bool extended);

/*****************************************************************************
 * Constructor functions for temporal types
 *****************************************************************************/

extern Temporal *tcbuffer_constructor(const Temporal *tpoint, const Temporal *tfloat);

/*****************************************************************************
 * Restriction functions for temporal types
 *****************************************************************************/

extern Temporal *tcbuffer_at_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
extern Temporal *tcbuffer_at_cbuffer(const Temporal *temp, const Cbuffer *cbuf, const Span *zspan);
extern Temporal *tcbuffer_at_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *tcbuffer_minus_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
extern Temporal *tcbuffer_minus_cbuffer(const Temporal *temp, const Cbuffer *cbuf, const Span *zspan);
extern Temporal *tcbuffer_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc);

/*****************************************************************************
 * Distance functions for temporal types
 *****************************************************************************/

extern Temporal *distance_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf);
extern Temporal *distance_tcbuffer_point(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *distance_tcbuffer_tbuffer(const Temporal *temp1, const Temporal *temp2);
extern double nad_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf);
extern double nad_tcbuffer_tbuffer(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf);
extern TInstant *nai_tcbuffer_tbuffer(const Temporal *temp1, const Temporal *temp2);
extern GSERIALIZED *shortestline_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern GSERIALIZED *shortestline_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf);
extern GSERIALIZED *shortestline_tcbuffer_tbuffer(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************
 * Spatial functions for temporal points
 *****************************************************************************/

/* Spatial accessor functions for temporal points */

extern Temporal *tcbuffer_azimuth(const Temporal *temp);
extern Temporal *tcbuffer_cumulative_length(const Temporal *temp);
extern double tcbuffer_length(const Temporal *temp);
extern Set *tcbuffer_points(const Temporal *temp);
extern Temporal *tcbuffer_speed(const Temporal *temp);
extern int32_t tcbuffer_srid(const Temporal *temp);
extern GSERIALIZED *tcbuffer_trajectory(const Temporal *temp);
extern GSERIALIZED *tcbuffer_twcentroid(const Temporal *temp);

/*****************************************************************************/

/* Spatial transformation functions for temporal points */

extern Temporal *tcbuffer_to_tgeompoint(const Temporal *temp);
extern Temporal *tcbuffer_tgeompoint(const Temporal *temp);
extern TInstant *tcbufferinst_tgeompointinst(const TInstant *inst);

extern Temporal *tcbuffer_to_tfloat(const Temporal *temp);
extern Temporal *tcbuffer_tfloat(const Temporal *temp);
extern TInstant *tcbufferinst_tfloatinst(const TInstant *inst);

/*****************************************************************************/

/* Ever and always spatial relationship functions for temporal points */

extern int acontains_geo_tbuffer(const GSERIALIZED *gs, const Temporal *temp);
extern int acontains_geo_tpoint(const GSERIALIZED *gs, const Temporal *temp);
extern int adisjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int adisjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf);
extern int adisjoint_tcbuffer_tbuffer(const Temporal *temp1, const Temporal *temp2);
extern int adwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern int adwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf, double dist);
extern int adwithin_tcbuffer_tbuffer(const Temporal *temp1, const Temporal *temp2, double dist);
extern int aintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int aintersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf);
extern int aintersects_tcbuffer_tbuffer(const Temporal *temp1, const Temporal *temp2);
extern int atouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int atouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf);
extern int atouches_tcbuffer_tbuffer(const Temporal *temp1, const Temporal *temp2);
extern int econtains_geo_tbuffer(const GSERIALIZED *gs, const Temporal *temp);
extern int edisjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int edisjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf);
extern int edisjoint_tcbuffer_tpoint(const Temporal *temp1, const Temporal *temp2);
extern int edwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern int edwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf, double dist);
extern int edwithin_tcbuffer_tbuffer(const Temporal *temp1, const Temporal *temp2, double dist);
extern int eintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int eintersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf);
extern int eintersects_tcbuffer_tbuffer(const Temporal *temp1, const Temporal *temp2);
extern int etouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int etouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf);

/*****************************************************************************/

/* Temporal spatial relationship functions for temporal points */

extern Temporal *tcontains_geo_tbuffer(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue);
extern Temporal *tdwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf, double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tcbuffer_tbuffer(const Temporal *temp1, const Temporal *temp2, double dist, bool restr, bool atvalue);
extern Temporal *ttouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);
extern Temporal *ttouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf, bool restr, bool atvalue);

/*****************************************************************************
 * Aggregate functions for temporal types
 *****************************************************************************/

extern SkipList *tcbuffer_tcentroid_transfn(SkipList *state, Temporal *temp);

/*****************************************************************************/

#endif /* __MEOS_CBUFFER_H__ */
