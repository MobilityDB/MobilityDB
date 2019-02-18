/*****************************************************************************
 *
 * BoundBoxOps.c
 *	  Bounding box operators for temporal network-constrained points.
 *
 * These operators test the bounding boxes of temporal npoints, which are
 * 3D boxes, where the x and y coordinates are for the space (value)
 * dimension and the z coordinate is for the time dimension.
 * The following operators are defined:
 *	  overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalNPoint.h"

/*****************************************************************************
 * Transform a temporal npoint to a GBOX
 *****************************************************************************/

void
tnpointinst_make_gbox(GBOX *box, Datum value, TimestampTz t)
{
	double infinity = get_float8_infinity();
    Datum geom = npoint_geom_internal(DatumGetNpoint(value));
	GBOX gbox;
	GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
	if (gserialized_get_gbox_p(gs, &gbox) == LW_FAILURE)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Error while computing the bounding box of the temporal point")));

    memcpy(box, &gbox, sizeof(GBOX));
    box->zmin = -infinity;
    box->zmax = infinity;
    box->mmin = box->mmax = t;
    POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
    pfree(DatumGetPointer(geom));
    return;
}

void
tnpointinstarr_disc_to_gbox(GBOX *box, TemporalInst **instants, int count)
{
	Datum value = temporalinst_value(instants[0]);
	tnpointinst_make_gbox(box, value, instants[0]->t);
	for (int i = 1; i < count; i++)
    {
		GBOX box1;
		value = temporalinst_value(instants[i]);
		tnpointinst_make_gbox(&box1, value, instants[i]->t);
        gbox_merge(&box1, box);
    }
    return;
}

void
tnpointinstarr_cont_to_gbox(GBOX *box, TemporalInst **instants, int count)
{
    npoint *np = DatumGetNpoint(temporalinst_value(instants[0]));
    int64 rid = np->rid;
    double posmin = np->pos, posmax = np->pos, mmin = instants[0]->t, mmax = instants[0]->t;
    for (int i = 1; i < count; i++)
    {
        np = DatumGetNpoint(temporalinst_value(instants[i]));
        posmin = Min(posmin, np->pos);
        posmax = Max(posmax, np->pos);
        mmin = Min(mmin, instants[i]->t);
        mmax = Max(mmax, instants[i]->t);
    }

    Datum line = route_geom_with_rid(rid);
    Datum geom;
    if (posmin == 0 && posmax == 1)
        geom = PointerGetDatum(gserialized_copy((GSERIALIZED *)PG_DETOAST_DATUM(line)));
    else
        geom = call_function3(LWGEOM_line_substring, line,
            Float8GetDatum(posmin), Float8GetDatum(posmax));
								 
	GBOX gbox;
	GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
	if (gserialized_get_gbox_p(gs, &gbox) == LW_FAILURE)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Error while computing the bounding box of the temporal point")));
								 
	memcpy(box, &gbox, sizeof(GBOX));
    box->mmin = mmin;
    box->mmax = mmax;

    pfree(DatumGetPointer(line));
    pfree(DatumGetPointer(geom));
    return;
}

void
tnpointseqarr_to_gbox(GBOX *box, TemporalSeq **sequences, int count)
{
	temporalseq_bbox(box, sequences[0]);
    for (int i = 1; i < count; i++)
    {
        GBOX box1;
		temporalseq_bbox(&box1, sequences[i]);
        gbox_merge(&box1, box);
    }
    return;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tnpoint_to_gbox);

PGDLLEXPORT Datum
tnpoint_to_gbox(PG_FUNCTION_ARGS)
{
    Temporal *temp = PG_GETARG_TEMPORAL(0);
    GBOX *result = palloc0(sizeof(GBOX));
	temporal_bbox(result, temp);
    PG_FREE_IF_COPY(temp, 0);
    PG_RETURN_POINTER(result);
}


/*****************************************************************************/
