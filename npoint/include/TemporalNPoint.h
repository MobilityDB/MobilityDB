#ifndef __TEMPORALNPOINT_H__
#define __TEMPORALNPOINT_H__

#include "TemporalPoint.h"
#include <executor/spi.h>

/* Network-based point */

typedef struct
{
	int64		rid;			 /* route identifier */
	double		pos;			 /* position */
} npoint;

/* Network-based segment */

typedef struct
{
	int64		rid;			/* route identifier */
	double		pos1;			/* position1 */
	double		pos2;			/* position2 */
} nsegment;

/* Network-based region */

typedef struct
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int32 		count;			/* number of nsegment elements */
	nsegment	nsegs[1];
	/* variable-length data follows */
} nregion;

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

/* npoint */
#define DatumGetNpoint(X)		((npoint *) DatumGetPointer(X))
#define NpointGetDatum(X)		PointerGetDatum(X)
#define PG_GETARG_NPOINT(i)		((npoint *) PG_GETARG_POINTER(i))

/* nregion */
#define DatumGetNregion(X)		((nregion *) PG_DETOAST_DATUM(X))
#define NregionGetDatum(X)		PointerGetDatum(X)
#define PG_GETARG_NREGION(i)	((nregion *) PG_GETARG_VARLENA_P(i))

/*****************************************************************************
 * Parser.c
 *****************************************************************************/

extern nsegment nsegment_parse(char **str);
extern nregion *nregion_parse(char **str);
extern TemporalSeq *tnpointseq_parse(char **str, Oid basetype);
extern TemporalS *tnpoints_parse(char **str, Oid basetype);

/*****************************************************************************
 * StaticObjects.c
 *****************************************************************************/

extern Datum npoint_in(PG_FUNCTION_ARGS);
extern Datum npoint_out(PG_FUNCTION_ARGS);
extern Datum npoint_recv(PG_FUNCTION_ARGS);
extern Datum npoint_send(PG_FUNCTION_ARGS);
extern Datum nregion_in(PG_FUNCTION_ARGS);
extern Datum nregion_recv(PG_FUNCTION_ARGS);
extern Datum nregion_send(PG_FUNCTION_ARGS);
extern Datum npoint_constructor(PG_FUNCTION_ARGS);
extern Datum nregion_from_segment(PG_FUNCTION_ARGS);
extern Datum nregion_from_route(PG_FUNCTION_ARGS);
extern Datum nregion_from_route_pos(PG_FUNCTION_ARGS);
extern Datum nregion_from_npoint(PG_FUNCTION_ARGS);
extern Datum nregion_from_nregionarr(PG_FUNCTION_ARGS);
extern Datum npoint_route(PG_FUNCTION_ARGS);
extern Datum npoint_pos(PG_FUNCTION_ARGS);
extern Datum nregion_segments(PG_FUNCTION_ARGS);
extern Datum npoint_geom(PG_FUNCTION_ARGS);
extern Datum nregion_geom(PG_FUNCTION_ARGS);

extern nregion *nregion_from_nsegmentarr_internal(nsegment *nsegs, int count);

extern npoint *npoint_constructor_internal(int64 rid, double pos);
extern nregion *nregion_from_segment_internal(int64 rid, double pos1, double pos2);
extern nregion *nregion_from_nregionarr_internal(nregion **nregs, int count);

extern double route_length_with_rid(int64 rid);
extern Datum route_geom_with_rid(int64 rid);

extern Datum npoint_geom_internal(npoint *np);
extern Datum nregion_geom_internal(nregion *nreg);

extern bool eq_npoint_npoint_internal(npoint *np1, npoint *np2);
extern bool lt_npoint_npoint(npoint *np1, npoint *np2);

/*****************************************************************************
 * TemporalNPoint.c
 *****************************************************************************/

extern nregion *tnpointseq_positions(TemporalSeq *seq);
extern nregion *tnpoints_positions(TemporalS *ts);

/*****************************************************************************
 * TemporalGeo.c
 *****************************************************************************/

extern Datum tnpointseq_trajectory(TemporalSeq *seq);
extern Datum tnpointseq_trajectory1(TemporalInst *inst1, TemporalInst *inst2);
extern Datum tnpoints_trajectory(TemporalS *ts);

extern Datum tnpointinst_geom(TemporalInst *inst);
extern Datum tnpointi_geom(TemporalI *ti);
extern Datum tnpointseq_geom(TemporalSeq *seq);
extern Datum tnpoints_geom(TemporalS *ts);

/*****************************************************************************
 * TempDistance.c
 *****************************************************************************/

extern TemporalSeq *distance_geo_tnpointseq(Datum geo, TemporalSeq *seq);
extern TemporalS *distance_geo_tnpoints(Datum geo, TemporalS *ts);
extern TemporalSeq *distance_tnpointseq_geo(TemporalSeq *seq, Datum geo);
extern TemporalSeq *distance_tnpointseq_tnpointseq(TemporalSeq *seq1, TemporalSeq *seq2);
extern TemporalS *distance_tnpointseq_tnpoints(TemporalSeq *seq, TemporalS *ts);
extern TemporalS *distance_tnpoints_geo(TemporalS *ts, Datum geo);
extern TemporalS *distance_tnpoints_tnpointseq(TemporalS *ts, TemporalSeq *seq);
extern TemporalS *distance_tnpoints_tnpoints(TemporalS *ts1, TemporalS *ts2);

/*****************************************************************************
 * BoundBoxOps.c
 *****************************************************************************/

extern Datum tnpoint_to_gbox(PG_FUNCTION_ARGS);

extern void tnpointinst_make_gbox(GBOX *box, Datum value, TimestampTz t);
extern void tnpointinstarr_disc_to_gbox(GBOX *box, TemporalInst **inst, int count);
extern void tnpointinstarr_cont_to_gbox(GBOX *box, TemporalInst **inst, int count);
extern void tnpointseqarr_to_gbox(GBOX *box, TemporalSeq **seq, int count);

/*****************************************************************************
 * TempSpatialRels.c
 *****************************************************************************/

extern bool tnpointseq_intersect_at_timestamp(TemporalInst *start1, 
	TemporalInst *end1,	TemporalInst *start2, TemporalInst *end2, 
	bool lower_inc, bool upper_inc, TimestampTz *inter);

extern TemporalInst *tspatialrel_tnpointinst_geo(TemporalInst *inst, Datum geo,
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert);
extern TemporalI *tspatialrel_tnpointi_geo(TemporalI *ti, Datum geo,
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert);

extern TemporalInst *tspatialrel_tnpointinst_tnpointinst(
	TemporalInst *inst1, TemporalInst *inst2,
	Datum (*operator)(Datum, Datum), Oid valuetypid);
extern TemporalI *tspatialrel_tnpointi_tnpointi(TemporalI *ti1, TemporalI *ti2,
	Datum (*operator)(Datum, Datum), Oid valuetypid);


/*****************************************************************************/

#endif /* __TEMPORALPOINT_H__ */
