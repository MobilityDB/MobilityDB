/*****************************************************************************
 *
 * GeoRelativePosOps.c
 *	  Relative position operators for temporal geometry points.
 *
 * The following operators are defined for the spatial dimension:
 * - left, overleft, right, overright, below, overbelow, above, overabove,
 *   front, overfront, back, overback
 * There are no equivalent operators for the temporal geography points since
 * PostGIS does not currently provide such functionality for geography.
 * The following operators for the temporal dimension:
 * - before, overbefore, after, overafter
 * for both temporal geometry and geography points are "inherited" from the
 * basic temporal types. In this file they are defined when one of the
 * arguments is a stbox.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include <postgres.h>
#include <assert.h>
#include <catalog/pg_type.h>
#include <utils/rangetypes.h>

#include "TemporalPoint.h"

/*****************************************************************************/
/* STBOX op STBOX */

/* strictly left of? */

bool
left_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the XY dimension  */
	assert(MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->xmax < box2->xmin);
}

PG_FUNCTION_INFO_V1(left_stbox_stbox);

PGDLLEXPORT Datum
left_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		PG_RETURN_NULL();
	bool result = left_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to right of? */

bool
overleft_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the XY dimension  */
	assert(MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->xmax <= box2->xmax);
}

PG_FUNCTION_INFO_V1(overleft_stbox_stbox);

PGDLLEXPORT Datum
overleft_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		PG_RETURN_NULL();
	bool result = overleft_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly right of? */

bool
right_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the XY dimension  */
	assert(MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->xmin > box2->xmax);
}

PG_FUNCTION_INFO_V1(right_stbox_stbox);

PGDLLEXPORT Datum
right_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		PG_RETURN_NULL();
	bool result = right_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to left of? */

bool
overright_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the XY dimension  */
	assert(MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->xmin >= box2->xmin);
}

PG_FUNCTION_INFO_V1(overright_stbox_stbox);

PGDLLEXPORT Datum
overright_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		PG_RETURN_NULL();
	bool result = overright_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly below of? */

bool
below_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the XY dimension  */
	assert(MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->ymax < box2->ymin);
}

PG_FUNCTION_INFO_V1(below_stbox_stbox);

PGDLLEXPORT Datum
below_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		PG_RETURN_NULL();
	bool result = below_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend above of? */

bool
overbelow_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the XY dimension  */
	assert(MOBDB_FLAGS_GET_X(box1->flags) || MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->ymax <= box2->ymax);
}

PG_FUNCTION_INFO_V1(overbelow_stbox_stbox);

PGDLLEXPORT Datum
overbelow_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		PG_RETURN_NULL();
	bool result = overbelow_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly above of? */

bool
above_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the XY dimension  */
	assert(MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->ymin > box2->ymax);
}

PG_FUNCTION_INFO_V1(above_stbox_stbox);

PGDLLEXPORT Datum
above_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		PG_RETURN_NULL();
	bool result = above_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend below of? */

bool
overabove_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the XY dimension  */
	assert(MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->ymin >= box2->ymin);
}

PG_FUNCTION_INFO_V1(overabove_stbox_stbox);

PGDLLEXPORT Datum
overabove_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		PG_RETURN_NULL();
	bool result = overabove_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly front of? */

bool
front_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the Z dimension  */
	assert(MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags));
	return (box1->zmax < box2->zmin);
}

PG_FUNCTION_INFO_V1(front_stbox_stbox);

PGDLLEXPORT Datum
front_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_Z(box1->flags) || ! MOBDB_FLAGS_GET_Z(box2->flags))
		PG_RETURN_NULL();
	bool result = front_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to the back of? */

bool
overfront_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the Z dimension  */
	assert(MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags));
	return (box1->zmax <= box2->zmax);
}

PG_FUNCTION_INFO_V1(overfront_stbox_stbox);

PGDLLEXPORT Datum
overfront_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_Z(box1->flags) || ! MOBDB_FLAGS_GET_Z(box2->flags))
		PG_RETURN_NULL();
	bool result = overfront_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly back of? */

bool
back_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the Z dimension  */
	assert(MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags));
	return (box1->zmin > box2->zmax);
}

PG_FUNCTION_INFO_V1(back_stbox_stbox);

PGDLLEXPORT Datum
back_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_Z(box1->flags) || ! MOBDB_FLAGS_GET_Z(box2->flags))
		PG_RETURN_NULL();
	bool result = back_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to the front of? */

bool
overback_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the Z dimension  */
	assert(MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags));
	return (box1->zmin >= box2->zmin);
}

PG_FUNCTION_INFO_V1(overback_stbox_stbox);

PGDLLEXPORT Datum
overback_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	if (! MOBDB_FLAGS_GET_Z(box1->flags) || ! MOBDB_FLAGS_GET_Z(box2->flags))
		PG_RETURN_NULL();
	bool result = overback_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly before of? */

bool
before_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the T dimension  */
	assert(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags));
	return (box1->tmax < box2->tmin);
}

PG_FUNCTION_INFO_V1(before_stbox_stbox);

PGDLLEXPORT Datum
before_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_T(box1->flags) || ! MOBDB_FLAGS_GET_T(box2->flags))
		PG_RETURN_NULL();
	bool result = before_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to the after of? */

bool
overbefore_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the T dimension  */
	assert(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags));
	return (box1->tmax <= box2->tmax);
}

PG_FUNCTION_INFO_V1(overbefore_stbox_stbox);

PGDLLEXPORT Datum
overbefore_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_T(box1->flags) || ! MOBDB_FLAGS_GET_T(box2->flags))
		PG_RETURN_NULL();
	bool result = overbefore_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly after of? */

bool
after_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the T dimension  */
	assert(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags));
	return (box1->tmin > box2->tmax);
}

PG_FUNCTION_INFO_V1(after_stbox_stbox);

PGDLLEXPORT Datum
after_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_T(box1->flags) || ! MOBDB_FLAGS_GET_T(box2->flags))
		PG_RETURN_NULL();
	bool result = after_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to the before of? */

bool
overafter_stbox_stbox_internal(STBOX *box1, STBOX *box2)
{
	/* Both boxes should have the T dimension  */
	assert(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags));
	return (box1->tmin >= box2->tmin);
}

PG_FUNCTION_INFO_V1(overafter_stbox_stbox);

PGDLLEXPORT Datum
overafter_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_T(box1->flags) || ! MOBDB_FLAGS_GET_T(box2->flags))
		PG_RETURN_NULL();
	bool result = overafter_stbox_stbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* geom op Temporal */

PG_FUNCTION_INFO_V1(left_geom_tpoint);

PGDLLEXPORT Datum
left_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = left_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_geom_tpoint);

PGDLLEXPORT Datum
overleft_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overleft_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_geom_tpoint);

PGDLLEXPORT Datum
right_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = right_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_geom_tpoint);

PGDLLEXPORT Datum
overright_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overright_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_geom_tpoint);

PGDLLEXPORT Datum
below_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = below_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_geom_tpoint);

PGDLLEXPORT Datum
overbelow_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overbelow_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_geom_tpoint);

PGDLLEXPORT Datum
above_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = above_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_geom_tpoint);

PGDLLEXPORT Datum
overabove_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overabove_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(front_geom_tpoint);

PGDLLEXPORT Datum
front_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_check_Z_dimension(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = front_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overfront_geom_tpoint);

PGDLLEXPORT Datum
overfront_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_check_Z_dimension(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overfront_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(back_geom_tpoint);

PGDLLEXPORT Datum
back_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_check_Z_dimension(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = back_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overback_geom_tpoint);

PGDLLEXPORT Datum
overback_geom_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_check_Z_dimension(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overback_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op geom */

PG_FUNCTION_INFO_V1(left_tpoint_geom);

PGDLLEXPORT Datum
left_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = left_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_geom);

PGDLLEXPORT Datum
overleft_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overleft_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tpoint_geom);

PGDLLEXPORT Datum
right_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = right_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tpoint_geom);

PGDLLEXPORT Datum
overright_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overright_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_tpoint_geom);

PGDLLEXPORT Datum
below_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = below_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_geom);

PGDLLEXPORT Datum
overbelow_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overbelow_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_tpoint_geom);

PGDLLEXPORT Datum
above_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = above_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_geom);

PGDLLEXPORT Datum
overabove_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overabove_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(front_tpoint_geom);

PGDLLEXPORT Datum
front_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_check_Z_dimension(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = front_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_geom);

PGDLLEXPORT Datum
overfront_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_check_Z_dimension(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overfront_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(back_tpoint_geom);

PGDLLEXPORT Datum
back_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_check_Z_dimension(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = back_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overback_tpoint_geom);

PGDLLEXPORT Datum
overback_tpoint_geom(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_check_Z_dimension(temp, gs);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overback_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* stbox op Temporal */

PG_FUNCTION_INFO_V1(left_stbox_tpoint);

PGDLLEXPORT Datum
left_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = left_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_stbox_tpoint);

PGDLLEXPORT Datum
overleft_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = overleft_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_stbox_tpoint);

PGDLLEXPORT Datum
right_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = right_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_stbox_tpoint);

PGDLLEXPORT Datum
overright_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = overright_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_stbox_tpoint);

PGDLLEXPORT Datum
below_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = below_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_stbox_tpoint);

PGDLLEXPORT Datum
overbelow_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = overbelow_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_stbox_tpoint);

PGDLLEXPORT Datum
above_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = above_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_stbox_tpoint);

PGDLLEXPORT Datum
overabove_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = overabove_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(front_stbox_tpoint);

PGDLLEXPORT Datum
front_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool hasz = MOBDB_FLAGS_GET_Z(box->flags) && MOBDB_FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = front_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overfront_stbox_tpoint);

PGDLLEXPORT Datum
overfront_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool hasz = MOBDB_FLAGS_GET_Z(box->flags) && MOBDB_FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = overfront_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(back_stbox_tpoint);

PGDLLEXPORT Datum
back_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool hasz = MOBDB_FLAGS_GET_Z(box->flags) && MOBDB_FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = back_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overback_stbox_tpoint);

PGDLLEXPORT Datum
overback_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool hasz = MOBDB_FLAGS_GET_Z(box->flags) && MOBDB_FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = overback_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_stbox_tpoint);

PGDLLEXPORT Datum
before_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hast = MOBDB_FLAGS_GET_T(box->flags);
	bool result = false;
	if (hast)
	{
		STBOX box1 = {0,0,0,0,0,0,0,0,0};
		temporal_bbox(&box1, temp);
		result = before_stbox_stbox_internal(box, &box1);
	}
	PG_FREE_IF_COPY(temp, 1);
	if (!hast)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_stbox_tpoint);

PGDLLEXPORT Datum
overbefore_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hast = MOBDB_FLAGS_GET_T(box->flags);
	bool result = false;
	if (hast)
	{
		STBOX box1 = {0,0,0,0,0,0,0,0,0};
		temporal_bbox(&box1, temp);
		result = overbefore_stbox_stbox_internal(box, &box1);
	}
	PG_FREE_IF_COPY(temp, 1);
	if (!hast)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_stbox_tpoint);

PGDLLEXPORT Datum
after_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hast = MOBDB_FLAGS_GET_T(box->flags);
	bool result = false;
	if (hast)
	{
		STBOX box1 = {0,0,0,0,0,0,0,0,0};
		temporal_bbox(&box1, temp);
		result = after_stbox_stbox_internal(box, &box1);
	}
	PG_FREE_IF_COPY(temp, 1);
	if (!hast)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_stbox_tpoint);

PGDLLEXPORT Datum
overafter_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hast = MOBDB_FLAGS_GET_T(box->flags);
	bool result = false;
	if (hast)
	{
		STBOX box1 = {0,0,0,0,0,0,0,0,0};
		temporal_bbox(&box1, temp);
		result = overafter_stbox_stbox_internal(box, &box1);
	}
	PG_FREE_IF_COPY(temp, 1);
	if (!hast)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op stbox */

PG_FUNCTION_INFO_V1(left_tpoint_stbox);

PGDLLEXPORT Datum
left_tpoint_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = left_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_stbox);

PGDLLEXPORT Datum
overleft_tpoint_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = overleft_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tpoint_stbox);

PGDLLEXPORT Datum
right_tpoint_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = right_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tpoint_stbox);

PGDLLEXPORT Datum
overright_tpoint_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = overright_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_tpoint_stbox);

PGDLLEXPORT Datum
below_tpoint_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = below_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_stbox);

PGDLLEXPORT Datum
overbelow_tpoint_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = overbelow_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_tpoint_stbox);

PGDLLEXPORT Datum
above_tpoint_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = above_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_stbox);

PGDLLEXPORT Datum
overabove_tpoint_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool result = overabove_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(front_tpoint_stbox);

PGDLLEXPORT Datum
front_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool hasz = MOBDB_FLAGS_GET_Z(box->flags) && MOBDB_FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = front_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_stbox);

PGDLLEXPORT Datum
overfront_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool hasz = MOBDB_FLAGS_GET_Z(box->flags) && MOBDB_FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = overfront_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(back_tpoint_stbox);

PGDLLEXPORT Datum
back_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool hasz = MOBDB_FLAGS_GET_Z(box->flags) && MOBDB_FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = back_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overback_tpoint_stbox);

PGDLLEXPORT Datum
overback_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp);
	bool hasz = MOBDB_FLAGS_GET_Z(box->flags) && MOBDB_FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = overback_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_tpoint_stbox);

PGDLLEXPORT Datum
before_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	bool hast = MOBDB_FLAGS_GET_T(box->flags);
	bool result = false;
	if (hast)
	{
		STBOX box1 = {0,0,0,0,0,0,0,0,0};
		temporal_bbox(&box1, temp);
		result = before_stbox_stbox_internal(&box1, box);
	}
	PG_FREE_IF_COPY(temp, 0);
	if (!hast)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_tpoint_stbox);

PGDLLEXPORT Datum
overbefore_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	bool hast = MOBDB_FLAGS_GET_T(box->flags);
	bool result = false;
	if (hast)
	{
		STBOX box1 = {0,0,0,0,0,0,0,0,0};
		temporal_bbox(&box1, temp);
		result = overbefore_stbox_stbox_internal(&box1, box);
	}
	PG_FREE_IF_COPY(temp, 0);
	if (!hast)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_tpoint_stbox);

PGDLLEXPORT Datum
after_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	bool hast = MOBDB_FLAGS_GET_T(box->flags);
	bool result = false;
	if (hast)
	{
		STBOX box1 = {0,0,0,0,0,0,0,0,0};
		temporal_bbox(&box1, temp);
		result = after_stbox_stbox_internal(&box1, box);
	}
	PG_FREE_IF_COPY(temp, 0);
	if (!hast)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_tpoint_stbox);

PGDLLEXPORT Datum
overafter_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	bool hast = MOBDB_FLAGS_GET_T(box->flags);
	bool result = false;
	if (hast)
	{
		STBOX box1 = {0,0,0,0,0,0,0,0,0};
		temporal_bbox(&box1, temp);
		result = overafter_stbox_stbox_internal(&box1, box);
	}
	PG_FREE_IF_COPY(temp, 0);
	if (!hast)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(left_tpoint_tpoint);

PGDLLEXPORT Datum
left_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = left_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_tpoint);

PGDLLEXPORT Datum
overleft_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overleft_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tpoint_tpoint);

PGDLLEXPORT Datum
right_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = right_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tpoint_tpoint);

PGDLLEXPORT Datum
overright_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overright_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_tpoint_tpoint);

PGDLLEXPORT Datum
below_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = below_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_tpoint);

PGDLLEXPORT Datum
overbelow_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overbelow_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_tpoint_tpoint);

PGDLLEXPORT Datum
above_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = above_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_tpoint);

PGDLLEXPORT Datum
overabove_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overabove_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(front_tpoint_tpoint);

PGDLLEXPORT Datum
front_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_check_Z_dimension(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = front_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_tpoint);

PGDLLEXPORT Datum
overfront_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_check_Z_dimension(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overfront_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(back_tpoint_tpoint);

PGDLLEXPORT Datum
back_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_check_Z_dimension(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = back_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overback_tpoint_tpoint);

PGDLLEXPORT Datum
overback_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_check_Z_dimension(temp1, temp2);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overback_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_tpoint_tpoint);

PGDLLEXPORT Datum
before_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = before_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_tpoint_tpoint);

PGDLLEXPORT Datum
overbefore_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overbefore_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_tpoint_tpoint);

PGDLLEXPORT Datum
after_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = after_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_tpoint_tpoint);

PGDLLEXPORT Datum
overafter_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	STBOX box1 = {0,0,0,0,0,0,0,0,0}, box2 = {0,0,0,0,0,0,0,0,0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overafter_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
