/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Network-based static point/segments
 */

#ifndef __TNPOINT_STATIC_H__
#define __TNPOINT_STATIC_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "npoint/tnpoint.h"

/*****************************************************************************/

/* General functions */

extern int32_t get_srid_ways(void);
extern GSERIALIZED *npointarr_geom(Npoint **points, int count);
extern GSERIALIZED *nsegmentarr_geom(Nsegment **segments, int count);
extern Nsegment **nsegmentarr_normalize(Nsegment **segments, int *count);

/* Input/Output functions */

extern Npoint *npoint_in(const char *str, bool end);
extern char *npoint_out(const Npoint *np, int maxdd);

/* Constructor functions */

extern Npoint *npoint_make(int64 rid, double pos);
extern void npoint_set(int64 rid, double pos, Npoint *np);
extern Nsegment *nsegment_make(int64 rid, double pos1, double pos2);
extern void nsegment_set(int64 rid, double pos1, double pos2, Nsegment *ns);

/* Cast functions */

extern Nsegment *npoint_to_nsegment(const Npoint *np);

/* Input/output functions */

extern Nsegment *nsegment_in(const char *str);
extern char *nsegment_out(const Nsegment *ns, int maxdd);

/* Accessor functions */

extern int64 npoint_route(const Npoint *np);
extern double npoint_position(const Npoint *np);
extern int64 nsegment_route(const Nsegment *ns);
extern double nsegment_start_position(const Nsegment *ns);
extern double nsegment_end_position(const Nsegment *ns);

/* Conversions between network and Euclidean space */

extern bool route_exists(int64 rid);
extern double route_length(int64 rid);
extern GSERIALIZED *route_geom(int64 rid);
extern GSERIALIZED *npoint_geom(const Npoint *np);
extern Npoint *geom_npoint(const GSERIALIZED *gs);
extern GSERIALIZED *nsegment_geom(const Nsegment *ns);
extern Nsegment *geom_nsegment(const GSERIALIZED *gs);

/* SRID functions */

extern int npoint_srid(const Npoint *np);
extern int nsegment_srid(const Nsegment *ns);

/* Comparison functions */

extern bool npoint_eq(const Npoint *np1, const Npoint *np2);
extern bool npoint_ne(const Npoint *np1, const Npoint *np2);
extern int npoint_cmp(const Npoint *np1, const Npoint *np2);
extern bool npoint_lt(const Npoint *np1, const Npoint *np2);
extern bool npoint_le(const Npoint *np1, const Npoint *np2);
extern bool npoint_gt(const Npoint *np1, const Npoint *np2);
extern bool npoint_ge(const Npoint *np1, const Npoint *np2);

extern bool nsegment_eq(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_ne(const Nsegment *ns1, const Nsegment *ns2);
extern int nsegment_cmp(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_lt(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_le(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_gt(const Nsegment *ns1, const Nsegment *ns2);
extern bool nsegment_ge(const Nsegment *ns1, const Nsegment *ns2);

/* Hash functions */

extern uint32 npoint_hash(const Npoint *np);
extern uint64 npoint_hash_extended(const Npoint *np, uint64 seed);

/*****************************************************************************/

#endif /* __TNPOINT_STATIC_H__ */
