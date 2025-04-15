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

#ifndef __MEOS_NPOINT_H__
#define __MEOS_NPOINT_H__

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

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/* Structure to represent network-based points */

typedef struct Npoint Npoint;

/* Structure to represent network-based segments */

typedef struct Nsegment Nsegment;

/*****************************************************************************
 * Initialization of the MEOS library
 *****************************************************************************/

extern void meos_initialize_npoint(const char *file_name);

/*****************************************************************************
 * Validity macros and functions
 *****************************************************************************/

/**
 * @brief Macro for ensuring that the set passed as argument is a network
 * point set
 */
#if MEOS
  #define VALIDATE_NPOINTSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
            ensure_set_isof_type((set), T_NPOINTSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_NPOINTSET(set, ret) \
    do { \
      assert(set); \
      assert(set_isof_type((set), T_NPOINTSET)); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro for ensuring that the temporal value passed as argument is a
 * temporal network point
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TNPOINT(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
            ensure_temporal_isof_type((Temporal *) (temp), T_TNPOINT) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TNPOINT(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TNPOINT); \
    } while (0)
#endif /* MEOS */

/******************************************************************************
 * Functions for network points
 ******************************************************************************/

extern Npoint *geom_npoint(const GSERIALIZED *gs);
extern Nsegment *geom_nsegment(const GSERIALIZED *gs);
extern int32_t get_srid_ways();
extern GSERIALIZED *npoint_geom(const Npoint *np);
extern bool npoint_eq(const Npoint *np1, const Npoint *np2);
extern int npoint_cmp(const Npoint *np1, const Npoint *np2);
extern bool npoint_ge(const Npoint *np1, const Npoint *np2);
extern bool npoint_gt(const Npoint *np1, const Npoint *np2);
extern uint32 npoint_hash(const Npoint *np);
extern uint64 npoint_hash_extended(const Npoint *np, uint64 seed);
extern Npoint *npoint_in(const char *str);
extern bool npoint_le(const Npoint *np1, const Npoint *np2);
extern bool npoint_lt(const Npoint *np1, const Npoint *np2);
extern Npoint *npoint_make(int64 rid, double pos);
extern bool npoint_ne(const Npoint *np1, const Npoint *np2);
extern char *npoint_out(const Npoint *np, int maxdd);
extern double npoint_position(const Npoint *np);
extern int64 npoint_route(const Npoint *np);
extern bool npoint_same(const Npoint *np1, const Npoint *np2);
extern int32_t npoint_srid(const Npoint *np);
extern Nsegment *npoint_nsegment(const Npoint *np);
extern int nsegment_cmp(const Nsegment *ns1, const Nsegment *ns2);
extern double nsegment_end_position(const Nsegment *ns);
extern bool nsegment_eq(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_ge(const Nsegment *ns1, const Nsegment *ns2);
extern GSERIALIZED *nsegment_geom(const Nsegment *ns);
extern bool nsegment_gt(const Nsegment *ns1, const Nsegment *ns2);
extern Nsegment *nsegment_in(const char *str);
extern bool nsegment_le(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_lt(const Nsegment *ns1, const Nsegment *ns2);
extern Nsegment *nsegment_make(int64 rid, double pos1, double pos2);
extern bool nsegment_ne(const Nsegment *ns1, const Nsegment *ns2);
extern char *nsegment_out(const Nsegment *ns, int maxdd);
extern int64 nsegment_route(const Nsegment *ns);
extern double nsegment_start_position(const Nsegment *ns);
extern int32_t nsegment_srid(const Nsegment *ns);
extern bool route_exists(int64 rid);
extern GSERIALIZED *route_geom(int64 rid);
extern double route_length(int64 rid);

/******************************************************************************
 * Functions for network point sets
 ******************************************************************************/

extern bool contained_npoint_set(const Npoint *np, const Set *s);
extern bool contains_set_npoint(const Set *s, Npoint *np);
extern Set *intersection_npoint_set(const Npoint *np, const Set *s);
extern Set *intersection_set_npoint(const Set *s, const Npoint *np);
extern Set *minus_npoint_set(const Npoint *np, const Set *s);
extern Set *minus_set_npoint(const Set *s, const Npoint *np);
extern Set *npoint_to_set(const Npoint *np);
extern Set *npoint_union_transfn(Set *state, const Npoint *np);
extern Npoint *npointset_end_value(const Set *s);
extern Set *npointset_in(const char *str);
extern Set *npointset_make(const Npoint **values, int count);
extern char *npointset_out(const Set *s, int maxdd);
extern Npoint *npointset_start_value(const Set *s);
extern bool npointset_value_n(const Set *s, int n, Npoint **result);
extern Npoint **npointset_values(const Set *s);
extern Set *union_npoint_set(const Npoint *np, const Set *s);
extern Set *union_set_npoint(const Set *s, const Npoint *np);

/*===========================================================================*
 * Functions for box types
 *===========================================================================*/

/*****************************************************************************
 * Constructor functions for box types
 *****************************************************************************/

extern STBox *npoint_tstzspan_to_stbox(const Npoint *np, const Span *s);
extern STBox *npoint_timestamptz_to_stbox(const Npoint *np, TimestampTz t);

/*****************************************************************************
 * Conversion functions for box types
 *****************************************************************************/

extern STBox *npoint_stbox(const Npoint *np);
extern STBox *nsegment_stbox(const Nsegment *np);
extern Temporal *tgeompoint_tnpoint(const Temporal *temp);
extern Temporal *tnpoint_tgeompoint(const Temporal *temp);

/*===========================================================================*
 * Functions for temporal network points
 *===========================================================================*/

/*****************************************************************************
 * Restriction functions for temporal network points
 *****************************************************************************/

extern Temporal *tnpoint_at_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
extern Temporal *tnpoint_at_npoint(const Temporal *temp, const Npoint *np, const Span *zspan);
extern Temporal *tnpoint_at_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *tnpoint_minus_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
extern Temporal *tnpoint_minus_npoint(const Temporal *temp, const Npoint *np, const Span *zspan);
extern Temporal *tnpoint_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc);

/*****************************************************************************
 * Distance functions for temporal network points
 *****************************************************************************/

extern Temporal *distance_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern Temporal *distance_tnpoint_point(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *distance_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern double nad_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern double nad_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern TInstant *nai_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern GSERIALIZED *shortestline_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern GSERIALIZED *shortestline_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern GSERIALIZED *shortestline_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************
 * Spatial functions for temporal points
 *****************************************************************************/

/* Spatial accessor functions for temporal points */

extern Temporal *tnpoint_cumulative_length(const Temporal *temp);
extern double tnpoint_length(const Temporal *temp);
extern Nsegment **tnpoint_positions(const Temporal *temp, int *count);
extern int64 tnpoint_route(const Temporal *temp);
extern Set *tnpoint_routes(const Temporal *temp);
extern Temporal *tnpoint_speed(const Temporal *temp);
extern GSERIALIZED *tnpoint_trajectory(const Temporal *temp);
extern GSERIALIZED *tnpoint_twcentroid(const Temporal *temp);

/*****************************************************************************/

/* Spatial transformation functions for temporal points */

extern Temporal *tnpoint_to_tgeompoint(const Temporal *temp);

/*****************************************************************************/

/* Ever and always spatial relationship functions for temporal points */

extern int acontains_geo_tnpoint(const GSERIALIZED *gs, const Temporal *temp);
extern int acontains_geo_tpoint(const GSERIALIZED *gs, const Temporal *temp);
extern int adisjoint_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int adisjoint_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern int adisjoint_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern int adwithin_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern int adwithin_tnpoint_npoint(const Temporal *temp, const Npoint *np, double dist);
extern int adwithin_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2, double dist);
extern int aintersects_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int aintersects_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern int aintersects_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern int atouches_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int atouches_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern int atouches_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern int econtains_geo_tnpoint(const GSERIALIZED *gs, const Temporal *temp);
extern int edisjoint_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int edisjoint_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern int edisjoint_tnpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern int edwithin_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern int edwithin_tnpoint_npoint(const Temporal *temp, const Npoint *np, double dist);
extern int edwithin_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2, double dist);
extern int eintersects_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int eintersects_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern int eintersects_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern int etouches_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int etouches_tnpoint_npoint(const Temporal *temp, const Npoint *np);

/*****************************************************************************/

/* Temporal spatial relationship functions for temporal points */

extern Temporal *tcontains_geo_tnpoint(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue);
extern Temporal *tdwithin_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tnpoint_npoint(const Temporal *temp, const Npoint *np, double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2, double dist, bool restr, bool atvalue);
extern Temporal *ttouches_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);
extern Temporal *ttouches_tnpoint_npoint(const Temporal *temp, const Npoint *np, bool restr, bool atvalue);

/*****************************************************************************
 * Aggregate functions for temporal network points
 *****************************************************************************/

extern SkipList *tnpoint_tcentroid_transfn(SkipList *state, Temporal *temp);

/*****************************************************************************/

#endif
