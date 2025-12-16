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
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/* Structure to represent network-based points */

typedef struct
{
  int64 rid;        /**< route identifier */
  double pos;       /**< position */
} Npoint;

/* Structure to represent network-based segments */

typedef struct
{
  int64 rid;       /**< route identifier */
  double pos1;     /**< position1 */
  double pos2;     /**< position2 */
} Nsegment;

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
              ! ensure_set_isof_type((set), T_NPOINTSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_NPOINTSET(set, ret) \
    do { \
      assert(set); \
      assert(set->settype == T_NPOINTSET); \
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
              ! ensure_temporal_isof_type((Temporal *) (temp), T_TNPOINT) ) \
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

/* Input and output functions */

extern char *npoint_as_ewkt(const Npoint *np, int maxdd);
extern char *npoint_as_hexwkb(const Npoint *np, uint8_t variant, size_t *size_out);
extern char *npoint_as_text(const Npoint *np, int maxdd);
extern uint8_t *npoint_as_wkb(const Npoint *np, uint8_t variant, size_t *size_out);
extern Npoint *npoint_from_hexwkb(const char *hexwkb);
extern Npoint *npoint_from_wkb(const uint8_t *wkb, size_t size);
extern Npoint *npoint_in(const char *str);
extern char *npoint_out(const Npoint *np, int maxdd);
extern Nsegment *nsegment_in(const char *str);
extern char *nsegment_out(const Nsegment *ns, int maxdd);

/* Constructor functions */

extern Npoint *npoint_make(int64 rid, double pos);
extern Nsegment *nsegment_make(int64 rid, double pos1, double pos2);

/* Conversion functions */

extern Npoint *geompoint_to_npoint(const GSERIALIZED *gs);
extern Nsegment *geom_to_nsegment(const GSERIALIZED *gs);
extern GSERIALIZED *npoint_to_geompoint(const Npoint *np);
extern Nsegment *npoint_to_nsegment(const Npoint *np);
extern STBox *npoint_to_stbox(const Npoint *np);
extern GSERIALIZED *nsegment_to_geom(const Nsegment *ns);
extern STBox *nsegment_to_stbox(const Nsegment *ns);

/* Accessor functions */

extern uint32 npoint_hash(const Npoint *np);
extern uint64 npoint_hash_extended(const Npoint *np, uint64 seed);
extern double npoint_position(const Npoint *np);
extern int64 npoint_route(const Npoint *np);
extern double nsegment_end_position(const Nsegment *ns);
extern int64 nsegment_route(const Nsegment *ns);
extern double nsegment_start_position(const Nsegment *ns);

/* Route functions */

extern bool route_exists(int64 rid);
extern const GSERIALIZED *route_geom(int64 rid);
extern double route_length(int64 rid);

/* Transformation functions */

extern Npoint *npoint_round(const Npoint *np, int maxdd);
extern Nsegment *nsegment_round(const Nsegment *ns, int maxdd);

/* Spatial reference system functions */

extern int32_t get_srid_ways(void);
extern int32_t npoint_srid(const Npoint *np);
extern int32_t nsegment_srid(const Nsegment *ns);

/* Bounding box functions */

extern STBox *npoint_timestamptz_to_stbox(const Npoint *np, TimestampTz t);
extern STBox *npoint_tstzspan_to_stbox(const Npoint *np, const Span *s);

/* Comparison functions */

extern int npoint_cmp(const Npoint *np1, const Npoint *np2);
extern bool npoint_eq(const Npoint *np1, const Npoint *np2);
extern bool npoint_ge(const Npoint *np1, const Npoint *np2);
extern bool npoint_gt(const Npoint *np1, const Npoint *np2);
extern bool npoint_le(const Npoint *np1, const Npoint *np2);
extern bool npoint_lt(const Npoint *np1, const Npoint *np2);
extern bool npoint_ne(const Npoint *np1, const Npoint *np2);
extern bool npoint_same(const Npoint *np1, const Npoint *np2);
extern int nsegment_cmp(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_eq(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_ge(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_gt(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_le(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_lt(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_ne(const Nsegment *ns1, const Nsegment *ns2);

/******************************************************************************
 * Functions for network point sets
 ******************************************************************************/

/* Input and output functions */

extern Set *npointset_in(const char *str);
extern char *npointset_out(const Set *s, int maxdd);

/* Constructor functions */

extern Set *npointset_make(Npoint **values, int count);

/* Conversion functions */

extern Set *npoint_to_set(const Npoint *np);

/* Accessor functions */

extern Npoint *npointset_end_value(const Set *s);
extern Set *npointset_routes(const Set *s);
extern Npoint *npointset_start_value(const Set *s);
extern bool npointset_value_n(const Set *s, int n, Npoint **result);
extern Npoint **npointset_values(const Set *s);

/* Set operations */

extern bool contained_npoint_set(const Npoint *np, const Set *s);
extern bool contains_set_npoint(const Set *s, const Npoint *np);
extern Set *intersection_npoint_set(const Npoint *np, const Set *s);
extern Set *intersection_set_npoint(const Set *s, const Npoint *np);
extern Set *minus_npoint_set(const Npoint *np, const Set *s);
extern Set *minus_set_npoint(const Set *s, const Npoint *np);
extern Set *npoint_union_transfn(Set *state, const Npoint *np);
extern Set *union_npoint_set(const Npoint *np, const Set *s);
extern Set *union_set_npoint(const Set *s, const Npoint *np);

/*===========================================================================*
 * Functions for temporal network points
 *===========================================================================*/

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

extern Temporal *tnpoint_in(const char *str);
extern char *tnpoint_out(const Temporal *temp, int maxdd);

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

extern TInstant *tnpointinst_make(const Npoint *np, TimestampTz t);

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

extern Temporal *tgeompoint_to_tnpoint(const Temporal *temp);
extern Temporal *tnpoint_to_tgeompoint(const Temporal *temp);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

extern Temporal *tnpoint_cumulative_length(const Temporal *temp);
extern double tnpoint_length(const Temporal *temp);
extern Nsegment **tnpoint_positions(const Temporal *temp, int *count);
extern int64 tnpoint_route(const Temporal *temp);
extern Set *tnpoint_routes(const Temporal *temp);
extern Temporal *tnpoint_speed(const Temporal *temp);
extern GSERIALIZED *tnpoint_trajectory(const Temporal *temp);
extern GSERIALIZED *tnpoint_twcentroid(const Temporal *temp);

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

extern Temporal *tnpoint_at_geom(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tnpoint_at_npoint(const Temporal *temp, const Npoint *np);
extern Temporal *tnpoint_at_npointset(const Temporal *temp, const Set *s);
extern Temporal *tnpoint_at_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *tnpoint_minus_geom(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tnpoint_minus_npoint(const Temporal *temp, const Npoint *np);
extern Temporal *tnpoint_minus_npointset(const Temporal *temp, const Set *s);
extern Temporal *tnpoint_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc);

/*****************************************************************************
 * Distance functions
 *****************************************************************************/

extern Temporal *tdistance_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern Temporal *tdistance_tnpoint_point(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tdistance_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern double nad_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern double nad_tnpoint_stbox(const Temporal *temp, const STBox *box);
extern double nad_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern TInstant *nai_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern GSERIALIZED *shortestline_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern GSERIALIZED *shortestline_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern GSERIALIZED *shortestline_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************
 * Aggregate functions
 *****************************************************************************/

extern SkipList *tnpoint_tcentroid_transfn(SkipList *state, Temporal *temp);

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/* Ever/always comparisons */

extern int always_eq_npoint_tnpoint(const Npoint *np, const Temporal *temp);
extern int always_eq_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern int always_eq_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern int always_ne_npoint_tnpoint(const Npoint *np, const Temporal *temp);
extern int always_ne_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern int always_ne_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern int ever_eq_npoint_tnpoint(const Npoint *np, const Temporal *temp);
extern int ever_eq_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern int ever_eq_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);
extern int ever_ne_npoint_tnpoint(const Npoint *np, const Temporal *temp);
extern int ever_ne_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern int ever_ne_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2);

/* Temporal comparisons */

extern Temporal *teq_tnpoint_npoint(const Temporal *temp, const Npoint *np);
extern Temporal *tne_tnpoint_npoint(const Temporal *temp, const Npoint *np);

/*****************************************************************************/

#endif
