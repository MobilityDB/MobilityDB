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
 * arguments is a gbox.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"

/*****************************************************************************/
/* GBOX op GBOX */

/* strictly left of? */

bool
left_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->xmax == infinity || box2->xmax == infinity )
		return false;
	return (box1->xmax < box2->xmin);
}

PG_FUNCTION_INFO_V1(left_gbox_gbox);

PGDLLEXPORT Datum
left_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = left_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to right of? */

bool
overleft_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->xmax == infinity || box2->xmax == infinity )
		return false;
	return (box1->xmax <= box2->xmax);
}

PG_FUNCTION_INFO_V1(overleft_gbox_gbox);

PGDLLEXPORT Datum
overleft_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = overleft_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly right of? */

bool
right_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->xmax == infinity || box2->xmax == infinity )
		return false;
	return (box1->xmin > box2->xmax);
}

PG_FUNCTION_INFO_V1(right_gbox_gbox);

PGDLLEXPORT Datum
right_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = right_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to left of? */

bool
overright_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->xmax == infinity || box2->xmax == infinity )
		return false;
	return (box1->xmin >= box2->xmin);
}

PG_FUNCTION_INFO_V1(overright_gbox_gbox);

PGDLLEXPORT Datum
overright_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = overright_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly below of? */

bool
below_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->ymax == infinity || box2->ymax == infinity )
		return false;
	return (box1->ymax < box2->ymin);
}

PG_FUNCTION_INFO_V1(below_gbox_gbox);

PGDLLEXPORT Datum
below_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = below_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend above of? */

bool
overbelow_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->ymax == infinity || box2->ymax == infinity )
		return false;
	return (box1->ymax <= box2->ymax);
}

PG_FUNCTION_INFO_V1(overbelow_gbox_gbox);

PGDLLEXPORT Datum
overbelow_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = overbelow_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly above of? */

bool
above_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->ymax == infinity || box2->ymax == infinity )
		return false;
	return (box1->ymin > box2->ymax);
}

PG_FUNCTION_INFO_V1(above_gbox_gbox);

PGDLLEXPORT Datum
above_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = above_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend below of? */

bool
overabove_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->ymax == infinity || box2->ymax == infinity )
		return false;
	return (box1->ymin >= box2->ymin);
}

PG_FUNCTION_INFO_V1(overabove_gbox_gbox);

PGDLLEXPORT Datum
overabove_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = overabove_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly front of? */

bool
front_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	if ( ! FLAGS_GET_Z(box1->flags) || ! FLAGS_GET_Z(box2->flags))
		return false;
	return (box1->zmax < box2->zmin);
}

PG_FUNCTION_INFO_V1(front_gbox_gbox);

PGDLLEXPORT Datum
front_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = front_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to the back of? */

bool
overfront_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	if (! FLAGS_GET_Z(box1->flags) || ! FLAGS_GET_Z(box2->flags))
		return false;
	return (box1->zmax <= box2->zmax);
}

PG_FUNCTION_INFO_V1(overfront_gbox_gbox);

PGDLLEXPORT Datum
overfront_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = overfront_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly back of? */

bool
back_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	if (! FLAGS_GET_Z(box1->flags) || ! FLAGS_GET_Z(box2->flags))
		return false;
	return (box1->zmin > box2->zmax);
}

PG_FUNCTION_INFO_V1(back_gbox_gbox);

PGDLLEXPORT Datum
back_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = back_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to the front of? */

bool
overback_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	if (! FLAGS_GET_Z(box1->flags) || ! FLAGS_GET_Z(box2->flags))
		return false;
	return (box1->zmin >= box2->zmin);
}

PG_FUNCTION_INFO_V1(overback_gbox_gbox);

PGDLLEXPORT Datum
overback_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = overback_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly before of? */

bool
before_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	if (! FLAGS_GET_M(box1->flags) || ! FLAGS_GET_M(box2->flags))
		return false;
	return (box1->mmax < box2->mmin);
}

PG_FUNCTION_INFO_V1(before_gbox_gbox);

PGDLLEXPORT Datum
before_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = before_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to the after of? */

bool
overbefore_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	if (! FLAGS_GET_M(box1->flags) || ! FLAGS_GET_M(box2->flags))
		return false;
	return (box1->mmax <= box2->mmax);
}

PG_FUNCTION_INFO_V1(overbefore_gbox_gbox);

PGDLLEXPORT Datum
overbefore_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = overbefore_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* strictly after of? */

bool
after_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	if (! FLAGS_GET_M(box1->flags) || ! FLAGS_GET_M(box2->flags))
		return false;
	return (box1->mmin > box2->mmax);
}

PG_FUNCTION_INFO_V1(after_gbox_gbox);

PGDLLEXPORT Datum
after_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = after_gbox_gbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/* does not extend to the before of? */

bool
overafter_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	if (! FLAGS_GET_M(box1->flags) || ! FLAGS_GET_M(box2->flags))
		return false;
	return (box1->mmin >= box2->mmin);
}

PG_FUNCTION_INFO_V1(overafter_gbox_gbox);

PGDLLEXPORT Datum
overafter_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	bool result = overafter_gbox_gbox_internal(box1, box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = left_gbox_gbox_internal(&box1, &box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overleft_gbox_gbox_internal(&box1, &box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = right_gbox_gbox_internal(&box1, &box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overright_gbox_gbox_internal(&box1, &box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = below_gbox_gbox_internal(&box1, &box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overbelow_gbox_gbox_internal(&box1, &box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = above_gbox_gbox_internal(&box1, &box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overabove_gbox_gbox_internal(&box1, &box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (! FLAGS_GET_Z(gs->flags) || ! MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must have Z dimension")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = front_gbox_gbox_internal(&box1, &box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (! FLAGS_GET_Z(gs->flags) || ! MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must have Z dimension")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overfront_gbox_gbox_internal(&box1, &box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (! FLAGS_GET_Z(gs->flags) || ! MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must have Z dimension")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = back_gbox_gbox_internal(&box1, &box2);
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
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (! FLAGS_GET_Z(gs->flags) || ! MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must have Z dimension")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overback_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = left_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overleft_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = right_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overright_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = below_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overbelow_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = above_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overabove_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (! MOBDB_FLAGS_GET_Z(temp->flags) || ! FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must have Z dimension")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = front_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (! MOBDB_FLAGS_GET_Z(temp->flags) || ! FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must have Z dimension")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overfront_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (! MOBDB_FLAGS_GET_Z(temp->flags) || ! FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must have Z dimension")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = back_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (! MOBDB_FLAGS_GET_Z(temp->flags) || ! FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must have Z dimension")));
	}

	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = overback_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* gbox op Temporal */

PG_FUNCTION_INFO_V1(left_gbox_tpoint);

PGDLLEXPORT Datum
left_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = left_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_gbox_tpoint);

PGDLLEXPORT Datum
overleft_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = overleft_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_gbox_tpoint);

PGDLLEXPORT Datum
right_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = right_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_gbox_tpoint);

PGDLLEXPORT Datum
overright_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = overright_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_gbox_tpoint);

PGDLLEXPORT Datum
below_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = below_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_gbox_tpoint);

PGDLLEXPORT Datum
overbelow_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = overbelow_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_gbox_tpoint);

PGDLLEXPORT Datum
above_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = above_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_gbox_tpoint);

PGDLLEXPORT Datum
overabove_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = overabove_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(front_gbox_tpoint);

PGDLLEXPORT Datum
front_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool hasz = FLAGS_GET_Z(box->flags) && FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = front_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overfront_gbox_tpoint);

PGDLLEXPORT Datum
overfront_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool hasz = FLAGS_GET_Z(box->flags) && FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = overfront_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(back_gbox_tpoint);

PGDLLEXPORT Datum
back_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool hasz = FLAGS_GET_Z(box->flags) && FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = back_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overback_gbox_tpoint);

PGDLLEXPORT Datum
overback_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool hasz = FLAGS_GET_Z(box->flags) && FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = overback_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_gbox_tpoint);

PGDLLEXPORT Datum
before_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hasm = FLAGS_GET_M(box->flags);
	bool result = false;
	if (hasm)
	{
		GBOX box1;
		temporal_bbox(&box1, temp);
		result = before_gbox_gbox_internal(box, &box1);
	}
	PG_FREE_IF_COPY(temp, 1);
	if (!hasm)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_gbox_tpoint);

PGDLLEXPORT Datum
overbefore_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hasm = FLAGS_GET_M(box->flags);
	bool result = false;
	if (hasm)
	{
		GBOX box1;
		temporal_bbox(&box1, temp);
		result = overbefore_gbox_gbox_internal(box, &box1);
	}
	PG_FREE_IF_COPY(temp, 1);
	if (!hasm)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_gbox_tpoint);

PGDLLEXPORT Datum
after_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hasm = FLAGS_GET_M(box->flags);
	bool result = false;
	if (hasm)
	{
		GBOX box1;
		temporal_bbox(&box1, temp);
		result = after_gbox_gbox_internal(box, &box1);
	}
	PG_FREE_IF_COPY(temp, 1);
	if (!hasm)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_gbox_tpoint);

PGDLLEXPORT Datum
overafter_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hasm = FLAGS_GET_M(box->flags);
	bool result = false;
	if (hasm)
	{
		GBOX box1;
		temporal_bbox(&box1, temp);
		result = overafter_gbox_gbox_internal(box, &box1);
	}
	PG_FREE_IF_COPY(temp, 1);
	if (!hasm)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op gbox */

PG_FUNCTION_INFO_V1(left_tpoint_gbox);

PGDLLEXPORT Datum
left_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = left_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_gbox);

PGDLLEXPORT Datum
overleft_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = overleft_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tpoint_gbox);

PGDLLEXPORT Datum
right_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = right_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tpoint_gbox);

PGDLLEXPORT Datum
overright_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = overright_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(below_tpoint_gbox);

PGDLLEXPORT Datum
below_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = below_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_gbox);

PGDLLEXPORT Datum
overbelow_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = overbelow_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(above_tpoint_gbox);

PGDLLEXPORT Datum
above_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = above_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_gbox);

PGDLLEXPORT Datum
overabove_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = overabove_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(front_tpoint_gbox);

PGDLLEXPORT Datum
front_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool hasz = FLAGS_GET_Z(box->flags) && FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = front_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_gbox);

PGDLLEXPORT Datum
overfront_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool hasz = FLAGS_GET_Z(box->flags) && FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = overfront_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(back_tpoint_gbox);

PGDLLEXPORT Datum
back_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool hasz = FLAGS_GET_Z(box->flags) && FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = back_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overback_tpoint_gbox);

PGDLLEXPORT Datum
overback_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool hasz = FLAGS_GET_Z(box->flags) && FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = overback_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_tpoint_gbox);

PGDLLEXPORT Datum
before_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	bool hasm = FLAGS_GET_M(box->flags);
	bool result = false;
	if (hasm)
	{
		GBOX box1;
		temporal_bbox(&box1, temp);
		result = before_gbox_gbox_internal(&box1, box);
	}
	PG_FREE_IF_COPY(temp, 0);
	if (!hasm)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_tpoint_gbox);

PGDLLEXPORT Datum
overbefore_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	bool hasm = FLAGS_GET_M(box->flags);
	bool result = false;
	if (hasm)
	{
		GBOX box1;
		temporal_bbox(&box1, temp);
		result = overbefore_gbox_gbox_internal(&box1, box);
	}
	PG_FREE_IF_COPY(temp, 0);
	if (!hasm)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_tpoint_gbox);

PGDLLEXPORT Datum
after_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	bool hasm = FLAGS_GET_M(box->flags);
	bool result = false;
	if (hasm)
	{
		GBOX box1;
		temporal_bbox(&box1, temp);
		result = after_gbox_gbox_internal(&box1, box);
	}
	PG_FREE_IF_COPY(temp, 0);
	if (!hasm)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_tpoint_gbox);

PGDLLEXPORT Datum
overafter_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	bool hasm = FLAGS_GET_M(box->flags);
	bool result = false;
	if (hasm)
	{
		GBOX box1;
		temporal_bbox(&box1, temp);
		result = overafter_gbox_gbox_internal(&box1, box);
	}
	PG_FREE_IF_COPY(temp, 0);
	if (!hasm)
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = left_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overleft_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = right_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overright_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = below_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overbelow_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = above_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overabove_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (! MOBDB_FLAGS_GET_Z(temp1->flags) || ! MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must have Z dimension")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = front_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (! MOBDB_FLAGS_GET_Z(temp1->flags) || ! MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must have Z dimension")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overfront_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (! MOBDB_FLAGS_GET_Z(temp1->flags) || ! MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must have Z dimension")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = back_gbox_gbox_internal(&box1, &box2);
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
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (! MOBDB_FLAGS_GET_Z(temp1->flags) || ! MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must have Z dimension")));
	}

	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overback_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
