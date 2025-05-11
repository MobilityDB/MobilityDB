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
 * @brief API of the Mobility Engine Open Source (MEOS) library.
 */

#ifndef __MEOS_CBUFFER_H__
#define __MEOS_CBUFFER_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
// #include "geo/stbox.h"

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/* Opaque structure to represent circular buffers */

typedef struct Cbuffer Cbuffer;

/*****************************************************************************
 * Validity macros and functions
 *****************************************************************************/

/**
 * @brief Macro for ensuring that the set passed as argument is a circular
 * buffer set
 */
#if MEOS
  #define VALIDATE_CBUFFERSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_set_isof_type((set), T_CBUFFERSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_CBUFFERSET(set, ret) \
    do { \
      assert(set); \
      assert((set)->settype == T_CBUFFERSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that the temporal value passed as argument is a
 * temporal circular buffer
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TCBUFFER(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_temporal_isof_type((Temporal *) (temp), T_TCBUFFER) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TCBUFFER(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TCBUFFER); \
    } while (0)
#endif /* MEOS */

/******************************************************************************
 * Functions for circular buffers
 ******************************************************************************/

extern char *cbuffer_as_text(const Cbuffer *cb, int maxdd);
extern char *cbuffer_as_ewkt(const Cbuffer *cb, int maxdd);
extern uint8_t *cbuffer_as_wkb(const Cbuffer *cb, uint8_t variant, size_t *size_out);
extern char *cbuffer_as_hexwkb(const Cbuffer *cb, uint8_t variant, size_t *size);
extern int cbuffer_cmp(const Cbuffer *cb1, const Cbuffer *cb2);
extern Cbuffer *cbuffer_copy(const Cbuffer *cb);
extern bool cbuffer_eq(const Cbuffer *cb1, const Cbuffer *cb2);
extern Cbuffer *cbuffer_from_wkb(const uint8_t *wkb, size_t size);
extern Cbuffer *cbuffer_from_hexwkb(const char *hexwkb);
extern bool cbuffer_ge(const Cbuffer *cb1, const Cbuffer *cb2);
extern GSERIALIZED *cbuffer_geom(const Cbuffer *cb);
extern bool cbuffer_gt(const Cbuffer *cb1, const Cbuffer *cb2);
extern uint32 cbuffer_hash(const Cbuffer *cb);
extern uint64 cbuffer_hash_extended(const Cbuffer *cb, uint64 seed);
extern Cbuffer *cbuffer_in(const char *str);
extern bool cbuffer_le(const Cbuffer *cb1, const Cbuffer *cb2);
extern bool cbuffer_lt(const Cbuffer *cb1, const Cbuffer *cb2);
extern Cbuffer *cbuffer_make(const GSERIALIZED *point, double radius);
extern bool cbuffer_ne(const Cbuffer *cb1, const Cbuffer *cb2);
extern bool cbuffer_nsame(const Cbuffer *cb1, const Cbuffer *cb2);
extern char *cbuffer_out(const Cbuffer *cb, int maxdd);
extern GSERIALIZED *cbuffer_point(const Cbuffer *cb);
extern double cbuffer_radius(const Cbuffer *cb);
extern bool cbuffer_same(const Cbuffer *cb1, const Cbuffer *cb2);
extern int32_t cbuffer_srid(const Cbuffer *cb);
extern void cbuffer_set_srid(Cbuffer *cb, int32_t srid);
extern Cbuffer *cbuffer_transform(const Cbuffer *cb, int32 srid);
extern Cbuffer *cbuffer_transform_pipeline(const Cbuffer *cb, const char *pipelinestr, int32 srid, bool is_forward);
extern Cbuffer *geom_cbuffer(const GSERIALIZED *gs);

/******************************************************************************
 * Functions for circular buffer sets
 ******************************************************************************/

extern Set *cbuffer_to_set(const Cbuffer *cb);
extern Set *cbuffer_union_transfn(Set *state, const Cbuffer *cb);
extern Cbuffer *cbufferset_end_value(const Set *s);
extern Set *cbufferset_in(const char *str);
extern Set *cbufferset_make(const Cbuffer **values, int count);
extern char *cbufferset_out(const Set *s, int maxdd);
extern Cbuffer *cbufferset_start_value(const Set *s);
extern bool cbufferset_value_n(const Set *s, int n, Cbuffer **result);
extern Cbuffer **cbufferset_values(const Set *s);
extern bool contained_cbuffer_set(const Cbuffer *cb, const Set *s);
extern bool contains_set_cbuffer(const Set *s, Cbuffer *cb);
extern Set *minus_cbuffer_set(const Cbuffer *cb, const Set *s);
extern Set *minus_set_cbuffer(const Set *s, const Cbuffer *cb);
extern Set *intersection_cbuffer_set(const Cbuffer *cb, const Set *s);
extern Set *intersection_set_cbuffer(const Set *s, const Cbuffer *cb);
extern Set *union_cbuffer_set(const Cbuffer *cb, const Set *s);
extern Set *union_set_cbuffer(const Set *s, const Cbuffer *cb);

/*===========================================================================*
 * Functions for box types
 *===========================================================================*/

/*****************************************************************************
 * Constructor functions for box types
 *****************************************************************************/

extern STBox *cbuffer_tstzspan_to_stbox(const Cbuffer *cb, const Span *s);
extern STBox *cbuffer_timestamptz_to_stbox(const Cbuffer *cb, TimestampTz t);

/*****************************************************************************
 * Conversion functions for box types
 *****************************************************************************/

extern STBox *cbuffer_stbox(const Cbuffer *cb);

/*===========================================================================*
 * Functions for temporal types
 *===========================================================================*/

/*****************************************************************************
 * Input/output functions for temporal types
 *****************************************************************************/

extern char *tcbuffer_out(const Temporal *temp, int maxdd);
extern char **tcbufferarr_as_text(const Temporal **temparr, int count, int maxdd);

/*****************************************************************************
 * Constructor functions for temporal types
 *****************************************************************************/

extern Temporal *tcbuffer_make(const Temporal *tpoint, const Temporal *tfloat);

/*****************************************************************************
 * Restriction functions for temporal types
 *****************************************************************************/

extern Temporal *tcbuffer_at_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
extern Temporal *tcbuffer_at_cbuffer(const Temporal *temp, const Cbuffer *cb, const Span *zspan);
extern Temporal *tcbuffer_at_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *tcbuffer_minus_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
extern Temporal *tcbuffer_minus_cbuffer(const Temporal *temp, const Cbuffer *cb, const Span *zspan);
extern Temporal *tcbuffer_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc);

/*****************************************************************************
 * Distance functions for temporal types
 *****************************************************************************/

extern double distance_cbuffer_geo(const Cbuffer *cb, const GSERIALIZED *gs);
extern double distance_cbuffer_stbox(const Cbuffer *cb, const STBox *box);
extern double distance_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2);
extern Temporal *distance_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern Temporal *distance_tcbuffer_point(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *distance_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);
extern double nad_stbox_tcbuffer(const STBox *box, const Temporal *temp);
extern double nad_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern double nad_tcbuffer_stbox(const Temporal *temp, const STBox *box);
extern double nad_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern TInstant *nai_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);
extern GSERIALIZED *shortestline_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern GSERIALIZED *shortestline_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern GSERIALIZED *shortestline_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************
 * Spatial functions for temporal points
 *****************************************************************************/

/* Spatial accessor functions for temporal points */

extern Set *tcbuffer_points(const Temporal *temp);
extern Set *tcbuffer_radius(const Temporal *temp);
extern GSERIALIZED *tcbuffer_traversed_area(const Temporal *temp);

/*****************************************************************************/

/* Spatial transformation functions for temporal points */

extern Temporal *tcbuffer_tgeompoint(const Temporal *temp);
extern TInstant *tcbufferinst_tgeompointinst(const TInstant *inst);

extern Temporal *tcbuffer_tfloat(const Temporal *temp);
extern TInstant *tcbufferinst_tfloatinst(const TInstant *inst);

/*****************************************************************************/

/* Ever/always and temporal comparison functions for temporal circular buffers */

extern int ever_eq_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp);
extern int ever_eq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int ever_ne_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp);
extern int ever_ne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int always_eq_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp);
extern int always_eq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int always_ne_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp);
extern int always_ne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int ever_eq_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);
extern int ever_ne_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);
extern int always_eq_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);
extern int always_ne_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);

extern Temporal *teq_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp);
extern Temporal *tne_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp);
extern Temporal *teq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern Temporal *tne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);

/*****************************************************************************/

/* Ever and always spatial relationship functions for temporal circular buffers */

extern int acontains_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp);
extern int acontains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp);
extern int acontains_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int acontains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int acontains_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);
extern int adisjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int adisjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int adisjoint_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);
extern int adwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern int adwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, double dist);
extern int adwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, double dist);
extern int aintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int aintersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int aintersects_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);
extern int atouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int atouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int atouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);

extern int econtains_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp);
extern int econtains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp);
extern int econtains_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int econtains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int econtains_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);
extern int edisjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int edisjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int edisjoint_tcbuffer_tpoint(const Temporal *temp1, const Temporal *temp2);
extern int edwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern int edwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, double dist);
extern int edwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, double dist);
extern int eintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int eintersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int eintersects_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);
extern int etouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int etouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb);
extern int etouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************/

/* Temporal spatial relationship functions for temporal points */

extern Temporal *tcontains_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp, bool restr, bool atvalue);
extern Temporal *tcontains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue);
extern Temporal *tcontains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);
extern Temporal *tcontains_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, bool restr, bool atvalue);
extern Temporal *tcontains_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue);
extern Temporal *tdwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, double dist, bool restr, bool atvalue);
extern Temporal *ttouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);
extern Temporal *ttouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, bool restr, bool atvalue);
extern Temporal *ttouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue);

/*****************************************************************************/

#endif /* __MEOS_CBUFFER_H__ */
