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
 * @brief Functions for temporal network points.
 */

#ifndef __TNPOINT_H__
#define __TNPOINT_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_npoint.h>

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

/* Npoint */
#define DatumGetNpointP(X)         ((Npoint *) DatumGetPointer(X))
#define NpointPGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_NPOINT_P(X)      DatumGetNpointP(PG_GETARG_DATUM(X))
#define PG_RETURN_NPOINT_P(X)      PG_RETURN_POINTER(X)

/* Nsegment */
#define DatumGetNsegmentP(X)       ((Nsegment *) DatumGetPointer(X))
#define NsegmentPGetDatum(X)       PointerGetDatum(X)
#define PG_GETARG_NSEGMENT_P(X)    DatumGetNsegmentP(PG_GETARG_DATUM(X))
#define PG_RETURN_NSEGMENT_P(X)    PG_RETURN_POINTER(X)

/*****************************************************************************
 * Cbuffer functions
 *****************************************************************************/

/* General functions */

extern int32_t get_srid_ways(void);
extern GSERIALIZED *npointarr_geom(Npoint **points, int nelems);
extern GSERIALIZED *nsegmentarr_geom(Nsegment **segments, int nelems);
extern Nsegment **nsegmentarr_normalize(Nsegment **segments, int *nelems);

/* Input/output functions */

extern Npoint *npoint_in(const char *str);
extern char *npoint_out(const Npoint *np, int maxdd);

extern Nsegment *nsegment_in(const char *str);
extern char *nsegment_out(const Nsegment *ns, int maxdd);

/* Constructor functions */

extern void npoint_set(int64 rid, double pos, Npoint *np);
extern void nsegment_set(int64 rid, double pos1, double pos2, Nsegment *ns);

/*****************************************************************************
 * Tcbuffer functions
 *****************************************************************************/

/* Input/output functions */

/* Conversion functions */

/** Symbolic constants for transforming tnpoint <-> tgeompoint */
#define NPOINT_TO_GEOM        true
#define GEOM_TO_NPOINT        false

extern TInstant *tnpointinst_tgeompointinst(const TInstant *inst);
extern TSequence *tnpointseq_tgeompointseq_disc(const TSequence *is);
extern TSequence *tnpointseq_tgeompointseq_cont(const TSequence *seq);
extern TSequenceSet *tnpointseqset_tgeompointseqset(const TSequenceSet *ss);

extern TInstant *tgeompointinst_tnpointinst(const TInstant *inst);
extern TSequence *tgeompointseq_tnpointseq(const TSequence *seq);
extern TSequenceSet *tgeompointseqset_tnpointseqset(const TSequenceSet *ss);

/* Accessor functions */

extern Nsegment **tnpointinst_positions(const TInstant *inst);
extern Nsegment **tnpointseq_positions(const TSequence *seq, int *count);
extern Nsegment **tnpointseqset_positions(const TSequenceSet *ss, int *count);
extern int64 tnpointinst_route(const TInstant *inst);
extern Set *tnpointinst_routes(const TInstant *inst);
extern Set *tnpointseq_disc_routes(const TSequence *is);
extern Set *tnpointseq_cont_routes(const TSequence *seq);
extern Set *tnpointseqset_routes(const TSequenceSet *ss);

extern Nsegment *tnpointseq_linear_positions(const TSequence *seq);

/*****************************************************************************/

#endif /* __TNPOINT_H__ */
