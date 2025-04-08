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
 * @brief Functions for temporal network points.
 */

#ifndef __TNPOINT_H__
#define __TNPOINT_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_npoint.h>
#include "general/meos_catalog.h"

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/* Structure to represent network-based points */

struct Npoint
{
  int64 rid;        /**< route identifier */
  double pos;       /**< position */
};

/* Structure to represent network-based segments */

struct Nsegment
{
  int64 rid;       /**< route identifier */
  double pos1;     /**< position1 */
  double pos2;     /**< position2 */
};

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
 * Npoint functions
 *****************************************************************************/

/* Validity functions */

extern bool ensure_valid_tnpoint_npoint(const Temporal *temp,
  const Npoint *np);
extern bool ensure_valid_tnpoint_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern bool ensure_valid_tnpoint_stbox(const Temporal *temp, const STBox *box);
extern bool ensure_valid_tnpoint_tnpoint(const Temporal *temp1,
  const Temporal *temp2);

/* Collinear functions */

extern bool npoint_collinear(const Npoint *np1, const Npoint *np2, 
  const Npoint *np3, double ratio);

/* Interpolation functions */

extern Npoint *npointsegm_interpolate(const Npoint *start, const Npoint *end,
  long double ratio);
extern long double npointsegm_locate(const Npoint *start, const Npoint *end,
  const Npoint *value);

/* General functions */

extern int32_t get_srid_ways(void);
extern Npoint *npoint_parse(const char **str, bool end);
extern GSERIALIZED *npointarr_geom(Npoint **points, int count);
extern GSERIALIZED *nsegmentarr_geom(Nsegment **segments, int count);
extern Nsegment **nsegmentarr_normalize(Nsegment **segments, int *count);

/* Input/output functions */

extern char *npoint_wkt_out(Datum value, int maxdd);

extern Npoint *npoint_in(const char *str);
extern char *npoint_out(const Npoint *np, int maxdd);

extern Nsegment *nsegment_in(const char *str);
extern char *nsegment_out(const Nsegment *ns, int maxdd);

extern char *npoint_as_text(const Npoint *np, int maxdd);
extern char *npoint_as_ewkt(const Npoint *np, int maxdd);
extern uint8_t *npoint_as_wkb(const Npoint *np, uint8_t variant, size_t *size_out);
extern char *npoint_as_hexwkb(const Npoint *np, uint8_t variant, size_t *size_out);
extern Npoint *npoint_from_wkb(const uint8_t *wkb, size_t size);
extern Npoint *npoint_from_hexwkb(const char *hexwkb);

/* Constructor functions */

extern void npoint_set(int64 rid, double pos, Npoint *np);
extern void nsegment_set(int64 rid, double pos1, double pos2, Nsegment *ns);

/* Transformation functions */

extern Datum datum_npoint_round(Datum npoint, Datum size);
extern Npoint *npoint_round(const Npoint *np, int maxdd);
extern Nsegment *nsegment_round(const Nsegment *ns, int maxdd);

/*****************************************************************************
 * Temporal network point functions
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

extern Temporal *tnpoint_restrict_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan, bool atfunc);
extern Temporal *tnpoint_restrict_stbox(const Temporal *temp, const STBox *box, bool border_inc, bool atfunc);

/*****************************************************************************/

#endif /* __TNPOINT_H__ */
