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

/**
 * Generic position function for a geometry and a temporal point
 *
 * @param[in] func Function
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] hasz True when the values must have Z coordinates
 * @param[in] invert True when the function is called with inverted arguments
 */
bool
posop_tpoint_geom1(bool (*func)(const STBOX *, const STBOX *),
	Temporal *temp, GSERIALIZED *gs, bool hasz, bool invert)
{
	ensure_same_srid_tpoint_gs(temp, gs);
	if (hasz)
	{
		ensure_has_Z_tpoint(temp);
		ensure_has_Z_gs(gs);
	}
	else
		ensure_same_dimensionality_tpoint_gs(temp, gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	/* The test that the geometry is not empty should be ensured 
	   by the calling function */
	temporal_bbox(&box1, temp);
	geo_to_stbox_internal(&box2, gs);
	bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
	return result;
}

/**
 * Generic position function for a geometry and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] hasz True when the values must have Z coordinates
 * @param[in] func Function
 */
Datum
posop_geom_tpoint(FunctionCallInfo fcinfo, bool hasz,
	bool (*func)(const STBOX *, const STBOX *))
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = posop_tpoint_geom1(func, temp, gs, hasz, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/**
 * Generic position function for a temporal point and a geometry
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] hasz True when the values must have Z coordinates
 * @param[in] func Function
 */
Datum
posop_tpoint_geom(FunctionCallInfo fcinfo, bool hasz,
	bool (*func)(const STBOX *, const STBOX *))
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	bool result = posop_tpoint_geom1(func, temp, gs, hasz, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

/**
 * Generic position function for a temporal point and a spatiotemporal box
 *
 * @param[in] func Function
 * @param[in] temp Temporal value
 * @param[in] box Spatiotemporal box
 * @param[in] hasz True when the values must have Z coordinates
 * @param[in] invert True when the function is called with inverted arguments
 */
bool
posop_tpoint_stbox1(bool (*func)(const STBOX *, const STBOX *),
	Temporal *temp, STBOX *box, bool hasz, bool invert)
{
	ensure_same_geodetic_tpoint_stbox(temp, box);
	ensure_same_srid_tpoint_stbox(temp, box);
	if (hasz)
	{
		ensure_has_Z_tpoint(temp);
		ensure_has_Z_stbox(box);
	}
	else
		ensure_same_spatial_dimensionality_tpoint_stbox(temp, box);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = invert ? func(box, &box1) : func(&box1, box);
	return result;
}

/**
 * Generic position function for a temporal point and a spatiotemporal box
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] hasz True when the values must have Z coordinates
 * @param[in] func Function
 */
Datum
posop_stbox_tpoint(FunctionCallInfo fcinfo, bool hasz,
	bool (*func)(const STBOX *, const STBOX *))
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = posop_tpoint_stbox1(func, temp, box, hasz, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/**
 * Generic position function for a temporal point and a spatiotemporal box
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] hasz True when the values must have Z coordinates
 * @param[in] func Function
 */
Datum
posop_tpoint_stbox(FunctionCallInfo fcinfo, bool hasz,
	bool (*func)(const STBOX *, const STBOX *))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	bool result = posop_tpoint_stbox1(func, temp, box, hasz, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/**
 * Generic position function for a temporal point and a spatiotemporal box
 * regarding the temporal dimension
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
posop_stbox_tpoint_tdim(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_has_T_stbox(box);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = func(box, &box1);
	PG_RETURN_BOOL(result);
}

/**
 * Generic position function for a temporal point and a spatiotemporal box
 * regarding the temporal dimension
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
posop_tpoint_stbox_tdim(FunctionCallInfo fcinfo, 
	bool (*func)(const STBOX *, const STBOX *))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	ensure_has_T_stbox(box);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = func(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/**
 * Generic position function for temporal points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] hasz True when the values must have Z coordinates
 * @param[in] func Function
 */
Datum
posop_tpoint_tpoint(FunctionCallInfo fcinfo, bool hasz,
	bool (*func)(const STBOX *, const STBOX *))
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	if (hasz)
	{
		ensure_has_Z_tpoint(temp1);
		ensure_has_Z_tpoint(temp2);
	}
	else
		ensure_same_dimensionality_tpoint(temp1, temp2);
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

/*****************************************************************************/
/* geom op Temporal */

PG_FUNCTION_INFO_V1(left_geom_tpoint);

PGDLLEXPORT Datum
left_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, false, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_geom_tpoint);

PGDLLEXPORT Datum
overleft_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, false, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_geom_tpoint);

PGDLLEXPORT Datum
right_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, false, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_geom_tpoint);

PGDLLEXPORT Datum
overright_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, false, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_geom_tpoint);

PGDLLEXPORT Datum
below_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, false, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_geom_tpoint);

PGDLLEXPORT Datum
overbelow_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, false, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_geom_tpoint);

PGDLLEXPORT Datum
above_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, false, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_geom_tpoint);

PGDLLEXPORT Datum
overabove_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, false, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_geom_tpoint);

PGDLLEXPORT Datum
front_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, true, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_geom_tpoint);

PGDLLEXPORT Datum
overfront_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, true, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_geom_tpoint);

PGDLLEXPORT Datum
back_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, true, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_geom_tpoint);

PGDLLEXPORT Datum
overback_geom_tpoint(PG_FUNCTION_ARGS)
{
	return posop_geom_tpoint(fcinfo, true, &overback_stbox_stbox_internal);
}

/*****************************************************************************/
/* Temporal op geom */

PG_FUNCTION_INFO_V1(left_tpoint_geom);

PGDLLEXPORT Datum
left_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, false, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_geom);

PGDLLEXPORT Datum
overleft_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, false, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_tpoint_geom);

PGDLLEXPORT Datum
right_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, false, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tpoint_geom);

PGDLLEXPORT Datum
overright_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, false, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_tpoint_geom);

PGDLLEXPORT Datum
below_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, false, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_geom);

PGDLLEXPORT Datum
overbelow_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, false, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_tpoint_geom);

PGDLLEXPORT Datum
above_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, false, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_geom);

PGDLLEXPORT Datum
overabove_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, false, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_tpoint_geom);

PGDLLEXPORT Datum
front_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, true,&front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_geom);

PGDLLEXPORT Datum
overfront_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, true,&overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_tpoint_geom);

PGDLLEXPORT Datum
back_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, true,&back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_tpoint_geom);

PGDLLEXPORT Datum
overback_tpoint_geom(PG_FUNCTION_ARGS)
{
	return posop_tpoint_geom(fcinfo, true,&overback_stbox_stbox_internal);
}

/*****************************************************************************/
/* stbox op Temporal */

PG_FUNCTION_INFO_V1(left_stbox_tpoint);

PGDLLEXPORT Datum
left_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, false, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_stbox_tpoint);

PGDLLEXPORT Datum
overleft_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, false, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_stbox_tpoint);

PGDLLEXPORT Datum
right_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, false, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_stbox_tpoint);

PGDLLEXPORT Datum
overright_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, false, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_stbox_tpoint);

PGDLLEXPORT Datum
below_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, false, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_stbox_tpoint);

PGDLLEXPORT Datum
overbelow_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, false, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_stbox_tpoint);

PGDLLEXPORT Datum
above_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, false, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_stbox_tpoint);

PGDLLEXPORT Datum
overabove_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, false, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_stbox_tpoint);

PGDLLEXPORT Datum
front_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, true, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_stbox_tpoint);

PGDLLEXPORT Datum
overfront_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, true, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_stbox_tpoint);

PGDLLEXPORT Datum
back_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, true, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_stbox_tpoint);

PGDLLEXPORT Datum
overback_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return posop_stbox_tpoint(fcinfo, true, &overback_stbox_stbox_internal);
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
	return posop_tpoint_stbox(fcinfo, false, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_stbox);

PGDLLEXPORT Datum
overleft_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, false, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_tpoint_stbox);

PGDLLEXPORT Datum
right_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, false, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tpoint_stbox);

PGDLLEXPORT Datum
overright_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, false, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_tpoint_stbox);

PGDLLEXPORT Datum
below_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, false, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_stbox);

PGDLLEXPORT Datum
overbelow_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, false, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_tpoint_stbox);

PGDLLEXPORT Datum
above_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, false, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_stbox);

PGDLLEXPORT Datum
overabove_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, false, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_tpoint_stbox);

PGDLLEXPORT Datum
front_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, true, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_stbox);

PGDLLEXPORT Datum
overfront_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, true, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_tpoint_stbox);

PGDLLEXPORT Datum
back_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, true, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_tpoint_stbox);

PGDLLEXPORT Datum
overback_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return posop_tpoint_stbox(fcinfo, true, &overback_stbox_stbox_internal);
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
	return posop_tpoint_tpoint(fcinfo, false, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_tpoint);

PGDLLEXPORT Datum
overleft_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, false, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_tpoint_tpoint);

PGDLLEXPORT Datum
right_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, false, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tpoint_tpoint);

PGDLLEXPORT Datum
overright_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, false, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_tpoint_tpoint);

PGDLLEXPORT Datum
below_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, false, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_tpoint);

PGDLLEXPORT Datum
overbelow_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, false, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_tpoint_tpoint);

PGDLLEXPORT Datum
above_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, false, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_tpoint);

PGDLLEXPORT Datum
overabove_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, false, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_tpoint_tpoint);

PGDLLEXPORT Datum
front_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, true,&front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_tpoint);

PGDLLEXPORT Datum
overfront_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, true,&overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_tpoint_tpoint);

PGDLLEXPORT Datum
back_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, true,&back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_tpoint_tpoint);

PGDLLEXPORT Datum
overback_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, true,&overback_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(before_tpoint_tpoint);

PGDLLEXPORT Datum
before_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, false, &before_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_tpoint_tpoint);

PGDLLEXPORT Datum
overbefore_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, false, &overbefore_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(after_tpoint_tpoint);

PGDLLEXPORT Datum
after_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, false, &after_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_tpoint_tpoint);

PGDLLEXPORT Datum
overafter_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return posop_tpoint_tpoint(fcinfo, false, &overafter_stbox_stbox_internal);
}

/*****************************************************************************/
