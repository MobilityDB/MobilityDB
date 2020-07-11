/*****************************************************************************
 *
 * tpoint_posops.c
 *	  Relative position operators for temporal geometry points.
 *
 * The following operators are defined for the spatial dimension:
 * - left, overleft, right, overright, below, overbelow, above, overabove,
 *   front, overfront, back, overback
 * There are no equivalent operators for the temporal geography points since
 * PostGIS does not currently provide such functionality for geography.
 * The following operators for the time dimension:
 * - before, overbefore, after, overafter
 * for both temporal geometry and geography points are "inherited" from the
 * basic temporal types. In this file they are defined when one of the
 * arguments is a stbox.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_posops.h"

#include <assert.h>

#include "postgis.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"
#include "tpoint_boxops.h"

/*****************************************************************************
 * Generic functions 
 *****************************************************************************/

Datum
posop_geom_tpoint(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = func(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

Datum
posop_geom_tpoint_zdim(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_Z_tpoint(temp);
	ensure_has_Z_gs(gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = func(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

Datum
posop_tpoint_geom(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = func(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

Datum
posop_tpoint_geom_zdim(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_Z_tpoint(temp);
	ensure_has_Z_gs(gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = func(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

Datum
posop_stbox_tpoint(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_geodetic_tpoint_stbox(temp, box);
	ensure_same_srid_tpoint_stbox(temp, box);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = func(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

Datum
posop_stbox_tpoint_zdim(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_geodetic_tpoint_stbox(temp, box);
	ensure_same_srid_tpoint_stbox(temp, box);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool hasz = MOBDB_FLAGS_GET_Z(box->flags) && MOBDB_FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = func(&box1, box);
	PG_FREE_IF_COPY(temp, 1);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

Datum
posop_stbox_tpoint_tdim(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hast = MOBDB_FLAGS_GET_T(box->flags);
	bool result = false;
	if (hast)
	{
		STBOX box1;
		memset(&box1, 0, sizeof(STBOX));
		temporal_bbox(&box1, temp);
		result = func(box, &box1);
	}
	PG_FREE_IF_COPY(temp, 1);
	if (!hast)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

Datum
posop_tpoint_stbox(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	STBOX *box = PG_GETARG_STBOX_P(1);
	if (! MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ensure_same_geodetic_tpoint_stbox(temp, box);
	ensure_same_srid_tpoint_stbox(temp, box);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = func(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

Datum
posop_tpoint_stbox_zdim(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	ensure_same_geodetic_tpoint_stbox(temp, box);
	ensure_same_srid_tpoint_stbox(temp, box);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool hasz = MOBDB_FLAGS_GET_Z(box->flags) && MOBDB_FLAGS_GET_Z(box1.flags);
	bool result = false;
	if (hasz)
		result = func(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	if (!hasz)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

Datum
posop_tpoint_stbox_tdim(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	bool hast = MOBDB_FLAGS_GET_T(box->flags);
	bool result = false;
	if (hast)
	{
		STBOX box1;
		memset(&box1, 0, sizeof(STBOX));
		temporal_bbox(&box1, temp);
		result = func(&box1, box);
	}
	PG_FREE_IF_COPY(temp, 0);
	if (!hast)
		PG_RETURN_NULL();
	PG_RETURN_BOOL(result);
}

Datum
posop_tpoint_tpoint(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = func(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

Datum
posop_tpoint_tpoint_zdim(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_has_Z_tpoint(temp1);
	ensure_has_Z_tpoint(temp2);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overfront_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* geom op Temporal */

PG_FUNCTION_INFO_V1(left_geom_tpoint);

PGDLLEXPORT Datum
left_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_geom_tpoint);

PGDLLEXPORT Datum
overleft_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_geom_tpoint);

PGDLLEXPORT Datum
right_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_geom_tpoint);

PGDLLEXPORT Datum
overright_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_geom_tpoint);

PGDLLEXPORT Datum
below_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_geom_tpoint);

PGDLLEXPORT Datum
overbelow_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_geom_tpoint);

PGDLLEXPORT Datum
above_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_geom_tpoint);

PGDLLEXPORT Datum
overabove_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_geom_tpoint);

PGDLLEXPORT Datum
front_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint_zdim(fcinfo, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_geom_tpoint);

PGDLLEXPORT Datum
overfront_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint_zdim(fcinfo, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_geom_tpoint);

PGDLLEXPORT Datum
back_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint_zdim(fcinfo, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_geom_tpoint);

PGDLLEXPORT Datum
overback_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint_zdim(fcinfo, &overback_stbox_stbox_internal);
}

/*****************************************************************************/
/* Temporal op geom */

PG_FUNCTION_INFO_V1(left_tpoint_geom);

PGDLLEXPORT Datum
left_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_geom);

PGDLLEXPORT Datum
overleft_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_tpoint_geom);

PGDLLEXPORT Datum
right_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tpoint_geom);

PGDLLEXPORT Datum
overright_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_tpoint_geom);

PGDLLEXPORT Datum
below_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_geom);

PGDLLEXPORT Datum
overbelow_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_tpoint_geom);

PGDLLEXPORT Datum
above_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_geom);

PGDLLEXPORT Datum
overabove_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_tpoint_geom);

PGDLLEXPORT Datum
front_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom_zdim(fcinfo, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_geom);

PGDLLEXPORT Datum
overfront_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom_zdim(fcinfo, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_tpoint_geom);

PGDLLEXPORT Datum
back_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom_zdim(fcinfo, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_tpoint_geom);

PGDLLEXPORT Datum
overback_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom_zdim(fcinfo, &overback_stbox_stbox_internal);
}

/*****************************************************************************/
/* stbox op Temporal */

PG_FUNCTION_INFO_V1(left_stbox_tpoint);

PGDLLEXPORT Datum
left_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_stbox_tpoint);

PGDLLEXPORT Datum
overleft_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_stbox_tpoint);

PGDLLEXPORT Datum
right_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_stbox_tpoint);

PGDLLEXPORT Datum
overright_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_stbox_tpoint);

PGDLLEXPORT Datum
below_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_stbox_tpoint);

PGDLLEXPORT Datum
overbelow_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_stbox_tpoint);

PGDLLEXPORT Datum
above_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_stbox_tpoint);

PGDLLEXPORT Datum
overabove_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_stbox_tpoint);

PGDLLEXPORT Datum
front_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint_zdim(fcinfo, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_stbox_tpoint);

PGDLLEXPORT Datum
overfront_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint_zdim(fcinfo, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_stbox_tpoint);

PGDLLEXPORT Datum
back_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint_zdim(fcinfo, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_stbox_tpoint);

PGDLLEXPORT Datum
overback_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint_zdim(fcinfo, &overback_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(before_stbox_tpoint);

PGDLLEXPORT Datum
before_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint_tdim(fcinfo, &before_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_stbox_tpoint);

PGDLLEXPORT Datum
overbefore_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint_tdim(fcinfo, &overbefore_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(after_stbox_tpoint);

PGDLLEXPORT Datum
after_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint_tdim(fcinfo, &after_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_stbox_tpoint);

PGDLLEXPORT Datum
overafter_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint_tdim(fcinfo, &overafter_stbox_stbox_internal);
}

/*****************************************************************************/
/* Temporal op stbox */

PG_FUNCTION_INFO_V1(left_tpoint_stbox);

PGDLLEXPORT Datum
left_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_stbox);

PGDLLEXPORT Datum
overleft_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_tpoint_stbox);

PGDLLEXPORT Datum
right_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tpoint_stbox);

PGDLLEXPORT Datum
overright_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_tpoint_stbox);

PGDLLEXPORT Datum
below_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_stbox);

PGDLLEXPORT Datum
overbelow_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_tpoint_stbox);

PGDLLEXPORT Datum
above_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_stbox);

PGDLLEXPORT Datum
overabove_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_tpoint_stbox);

PGDLLEXPORT Datum
front_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox_zdim(fcinfo, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_stbox);

PGDLLEXPORT Datum
overfront_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox_zdim(fcinfo, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_tpoint_stbox);

PGDLLEXPORT Datum
back_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox_zdim(fcinfo, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_tpoint_stbox);

PGDLLEXPORT Datum
overback_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox_zdim(fcinfo, &overback_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(before_tpoint_stbox);

PGDLLEXPORT Datum
before_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox_tdim(fcinfo, &before_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_tpoint_stbox);

PGDLLEXPORT Datum
overbefore_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox_tdim(fcinfo, &overbefore_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(after_tpoint_stbox);

PGDLLEXPORT Datum
after_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox_tdim(fcinfo, &after_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_tpoint_stbox);

PGDLLEXPORT Datum
overafter_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox_tdim(fcinfo, &overafter_stbox_stbox_internal);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(left_tpoint_tpoint);

PGDLLEXPORT Datum
left_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_tpoint);

PGDLLEXPORT Datum
overleft_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_tpoint_tpoint);

PGDLLEXPORT Datum
right_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tpoint_tpoint);

PGDLLEXPORT Datum
overright_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_tpoint_tpoint);

PGDLLEXPORT Datum
below_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_tpoint);

PGDLLEXPORT Datum
overbelow_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_tpoint_tpoint);

PGDLLEXPORT Datum
above_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_tpoint);

PGDLLEXPORT Datum
overabove_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_tpoint_tpoint);

PGDLLEXPORT Datum
front_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint_zdim(fcinfo, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_tpoint);

PGDLLEXPORT Datum
overfront_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint_zdim(fcinfo, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_tpoint_tpoint);

PGDLLEXPORT Datum
back_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint_zdim(fcinfo, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_tpoint_tpoint);

PGDLLEXPORT Datum
overback_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint_zdim(fcinfo, &overback_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(before_tpoint_tpoint);

PGDLLEXPORT Datum
before_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &before_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_tpoint_tpoint);

PGDLLEXPORT Datum
overbefore_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &overbefore_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(after_tpoint_tpoint);

PGDLLEXPORT Datum
after_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &after_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_tpoint_tpoint);

PGDLLEXPORT Datum
overafter_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, &overafter_stbox_stbox_internal);
}

/*****************************************************************************/
