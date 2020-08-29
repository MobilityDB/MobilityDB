/*****************************************************************************
 *
 * stbox.c
 *	  Functions for spatiotemporal bounding boxes.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "stbox.h"

#include <assert.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "period.h"
#include "timestampset.h"
#include "periodset.h"
#include "temporal_util.h"
#include "tnumber_mathfuncs.h"
#include "tpoint.h"
#include "tpoint_parser.h"
#include "tpoint_spatialfuncs.h"

/* Buffer size for input and output of STBOX */
#define MAXSTBOXLEN		256

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

/**
 * Constructs a newly allocated spatiotemporal box
 */
STBOX *
stbox_make(bool hasx, bool hasz, bool hast, bool geodetic, int32 srid, 
	double xmin, double xmax, double ymin, double ymax, double zmin, 
	double zmax, TimestampTz tmin, TimestampTz tmax)
{
	STBOX *result = palloc0(sizeof(STBOX));
	MOBDB_FLAGS_SET_X(result->flags, hasx);
	MOBDB_FLAGS_SET_Z(result->flags, hasz);
	MOBDB_FLAGS_SET_T(result->flags, hast);
	MOBDB_FLAGS_SET_GEODETIC(result->flags, geodetic);
	result->srid = srid;

	/* Process X min/max */
	if (hasx)
	{
		double tmp;
		if (xmin > xmax)
		{
			tmp = xmin;
			xmin = xmax;
			xmax = tmp;
		}
		result->xmin = xmin;
		result->xmax = xmax;

		/* Process Y min/max */
		if (ymin > ymax)
		{
			tmp = ymin;
			ymin = ymax;
			ymax = tmp;
		}
		result->ymin = ymin;
		result->ymax = ymax;

		if (hasz || geodetic)
		{
			/* Process Z min/max */
			if (zmin > zmax)
			{
				tmp = zmin;
				zmin = zmax;
				zmax = tmp;
			}
			result->zmin = zmin;
			result->zmax = zmax;
		}
	}

	if (hast)
	{
		TimestampTz ttmp;
		/* Process T min/max */
		if (tmin > tmax)
		{
			ttmp = tmin;
			tmin = tmax;
			tmax = ttmp;
		}
		result->tmin = tmin;
		result->tmax = tmax;
	}
	return result;
}

/**
 * Returns a copy of the spatiotemporal box
 */
STBOX *
stbox_copy(const STBOX *box)
{
	STBOX *result = palloc0(sizeof(STBOX));
	memcpy(result, box, sizeof(STBOX));
	return result;
}

/**
 * Expand the first spatiotemporal box with the second one
 *
 * @pre No tests are made concerning the srid, dimensionality, etc.
 * This should be ensured by the calling function.
 */
void
stbox_expand(STBOX *box1, const STBOX *box2)
{
	bool hasx = MOBDB_FLAGS_GET_X(box1->flags);
	bool hasz = MOBDB_FLAGS_GET_Z(box1->flags);
	bool hast = MOBDB_FLAGS_GET_T(box1->flags);
	bool geodetic = MOBDB_FLAGS_GET_GEODETIC(box1->flags);
	if (hasx)
	{
		box1->xmin = Min(box1->xmin, box2->xmin);
		box1->xmax = Max(box1->xmax, box2->xmax);
		box1->ymin = Min(box1->ymin, box2->ymin);
		box1->ymax = Max(box1->ymax, box2->ymax);
		if (hasz || geodetic)
		{
			box1->zmin = Min(box1->zmin, box2->zmin);
			box1->zmax = Max(box1->zmax, box2->zmax);
		}
	}
	if (hast)
	{
		box1->tmin = Min(box1->tmin, box2->tmin);
		box1->tmax = Max(box1->tmax, box2->tmax);
	}
}

/**
 * Shift the spatiotemporal box by the interval 
 */
void
stbox_shift(STBOX *box, const Interval *interval)
{
	box->tmin = DatumGetTimestampTz(
		DirectFunctionCall2(timestamptz_pl_interval,
		TimestampTzGetDatum(box->tmin), PointerGetDatum(interval)));
	box->tmax = DatumGetTimestampTz(
		DirectFunctionCall2(timestamptz_pl_interval,
		TimestampTzGetDatum(box->tmax), PointerGetDatum(interval)));
}

/*****************************************************************************
 * Input/Ouput functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_in);
/**
 * Input function for spatiotemporal boxes.
 *
 * Examples of input:
 * @code
 * STBOX((1.0, 2.0), (3.0, 4.0)) -> only spatial
 * STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0)) -> only spatial
 * STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02)) -> spatiotemporal
 * STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02)) -> spatiotemporal
 * STBOX T(( , , 2001-01-01), ( , , 2001-01-02)) -> only temporal
 * SRID=xxxx;STBOX... (any of the above)
 * GEODSTBOX((1.0, 2.0, 3.0), (4.0, 5.0, 6.0)) -> only spatial
 * GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02)) -> spatiotemporal
 * GEODSTBOX T(( , , 2001-01-01), ( , , 2001-01-02)) -> only temporal
 * SRID=xxxx;GEODSTBOX... (any of the above)
 * @endcode
 * where the commas are optional and the SRID is optional. If the SRID is not
 * stated it is by default 0 for non geodetic boxes and 4326 for geodetic boxes
 */
PGDLLEXPORT Datum
stbox_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	STBOX *result = stbox_parse(&input);
	PG_RETURN_POINTER(result);
}

/**
 * Returns the string representation of the spatiotemporal box
 */
static char *
stbox_to_string(const STBOX *box)
{
	static size_t size = MAXSTBOXLEN + 1;
	char *str, *xmin = NULL, *xmax = NULL, *ymin = NULL, *ymax = NULL,
		*zmin = NULL, *zmax = NULL, *tmin = NULL, *tmax = NULL;
	bool hasx = MOBDB_FLAGS_GET_X(box->flags);
	bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
	bool hast = MOBDB_FLAGS_GET_T(box->flags);
	bool geodetic = MOBDB_FLAGS_GET_GEODETIC(box->flags);

	str = (char *) palloc(size);
	char srid[20];
	if (hasx && box->srid > 0)
		sprintf(srid, "SRID=%d;", box->srid);
	else
		srid[0] = '\0';
	char *boxtype = geodetic ? "GEODSTBOX" : "STBOX";
	assert(hasx || hast);
	if (hasx)
	{
		xmin = call_output(FLOAT8OID, Float8GetDatum(box->xmin));
		xmax = call_output(FLOAT8OID, Float8GetDatum(box->xmax));
		ymin = call_output(FLOAT8OID, Float8GetDatum(box->ymin));
		ymax = call_output(FLOAT8OID, Float8GetDatum(box->ymax));
		if (geodetic || hasz)
		{
			zmin = call_output(FLOAT8OID, Float8GetDatum(box->zmin));
			zmax = call_output(FLOAT8OID, Float8GetDatum(box->zmax));
		}
	}
	if (hast)
	{
		tmin = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmin));
		tmax = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmax));
	}
	if (hasx)
	{
		if (geodetic)
		{
			char *Z;
			if (hast)
			{
				Z = hasz ? "Z" : "";
				snprintf(str, size, "%s%s %sT((%s,%s,%s,%s),(%s,%s,%s,%s))",
					srid, boxtype, Z, xmin, ymin, zmin, tmin, xmax, ymax, zmax, tmax);
			}
			else
			{
				Z = hasz ? " Z" : "";
				snprintf(str, size, "%s%s%s((%s,%s,%s),(%s,%s,%s))",
					srid, boxtype, Z, xmin, ymin, zmin, xmax, ymax, zmax);
			}
		}
		else if (hasz && hast)
			snprintf(str, size, "%s%s ZT((%s,%s,%s,%s),(%s,%s,%s,%s))",
				srid, boxtype, xmin, ymin, zmin, tmin, xmax, ymax, zmax, tmax);
		else if (hasz)
			snprintf(str, size, "%s%s Z((%s,%s,%s),(%s,%s,%s))",
				srid, boxtype, xmin, ymin, zmin, xmax, ymax, zmax);
		else if (hast)
			snprintf(str, size, "%s%s T((%s,%s,%s),(%s,%s,%s))",
				srid, boxtype, xmin, ymin, tmin, xmax, ymax, tmax);
		else 
			snprintf(str, size, "%s%s((%s,%s),(%s,%s))",
				srid, boxtype, xmin, ymin, xmax, ymax);
	}
	else
		/* Missing spatial dimension */
		snprintf(str, size, "%s%s T((,,%s),(,,%s))", srid, boxtype, tmin, tmax);
	if (hasx)
	{
		pfree(xmin); pfree(xmax);
		pfree(ymin); pfree(ymax);
		if (hasz)
		{
			pfree(zmin); pfree(zmax);
		}
	}
	if (hast)
	{
		pfree(tmin); pfree(tmax);
	}
	return str;
}

PG_FUNCTION_INFO_V1(stbox_out);
/**
 * Output function for spatiotemporal boxes.
 */
PGDLLEXPORT Datum
stbox_out(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	char *result = stbox_to_string(box);
	PG_RETURN_CSTRING(result);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * Construct a spatiotemporal box from the arguments
 */
static Datum
stbox_constructor1(FunctionCallInfo fcinfo, bool hasx, bool hasz, bool hast, 
	bool geodetic)
{
	double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
	TimestampTz tmin = 0, tmax = 0;
	int srid = 0; /* make Codacy quiet */
	
	if (!hasx && hast)
	{
		tmin = PG_GETARG_TIMESTAMPTZ(0);
		tmax = PG_GETARG_TIMESTAMPTZ(1);
		srid = PG_GETARG_INT32(2);
	}
	else if (hasx && !hasz && !geodetic && !hast)
	{
		xmin = PG_GETARG_FLOAT8(0);
		ymin = PG_GETARG_FLOAT8(1);
		xmax = PG_GETARG_FLOAT8(2);
		ymax = PG_GETARG_FLOAT8(3);
		srid = PG_GETARG_INT32(4);
	}
	else if (hasx && (hasz || geodetic) && !hast)
	{
		xmin = PG_GETARG_FLOAT8(0);
		ymin = PG_GETARG_FLOAT8(1);
		zmin = PG_GETARG_FLOAT8(2);
		xmax = PG_GETARG_FLOAT8(3);
		ymax = PG_GETARG_FLOAT8(4);
		zmax = PG_GETARG_FLOAT8(5);
		srid = PG_GETARG_INT32(6);
	}
	else if (hasx && !hasz && !geodetic && hast)
	{
		xmin = PG_GETARG_FLOAT8(0);
		ymin = PG_GETARG_FLOAT8(1);
		tmin = PG_GETARG_TIMESTAMPTZ(2);
		xmax = PG_GETARG_FLOAT8(3);
		ymax = PG_GETARG_FLOAT8(4);
		tmax = PG_GETARG_TIMESTAMPTZ(5);
		srid = PG_GETARG_INT32(6);
	}
	else if(hasx && (hasz || geodetic) && hast)
	{
		xmin = PG_GETARG_FLOAT8(0);
		ymin = PG_GETARG_FLOAT8(1);
		zmin = PG_GETARG_FLOAT8(2);
		tmin = PG_GETARG_TIMESTAMPTZ(3);
		xmax = PG_GETARG_FLOAT8(4);
		ymax = PG_GETARG_FLOAT8(5);
		zmax = PG_GETARG_FLOAT8(6);
		tmax = PG_GETARG_TIMESTAMPTZ(7);
		srid = PG_GETARG_INT32(8);
	}
	else 
		/* Should never arrive here */
		elog(ERROR, "Invalid arguments for stbox constructor");

	/* Construct the box */
	STBOX *result = stbox_make(hasx, hasz, hast, geodetic, srid,
		xmin, xmax, ymin, ymax, zmin, zmax, tmin, tmax);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_constructor_t);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
stbox_constructor_t(PG_FUNCTION_ARGS)
{
	if (PG_NARGS() > 3)
		return stbox_constructor1(fcinfo, true, false, true, false);
	return stbox_constructor1(fcinfo, false, false, true, false);
}

PG_FUNCTION_INFO_V1(stbox_constructor);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
stbox_constructor(PG_FUNCTION_ARGS)
{
	return stbox_constructor1(fcinfo, true, false, false, false);
}

PG_FUNCTION_INFO_V1(stbox_constructor_z);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
stbox_constructor_z(PG_FUNCTION_ARGS)
{
	return stbox_constructor1(fcinfo, true, true, false, false);
}

PG_FUNCTION_INFO_V1(stbox_constructor_zt);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
stbox_constructor_zt(PG_FUNCTION_ARGS)
{
	return stbox_constructor1(fcinfo, true, true, true, false);
}

/* The names of the SQL and C functions are different, otherwise there is
 * ambiguity and explicit casting of the arguments to ::timestamptz is needed */

PG_FUNCTION_INFO_V1(geodstbox_constructor_t);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
geodstbox_constructor_t(PG_FUNCTION_ARGS)
{
	if (PG_NARGS() > 3)
		return stbox_constructor1(fcinfo, true, false, true, true);
	return stbox_constructor1(fcinfo, false, false, true, true);
}

PG_FUNCTION_INFO_V1(geodstbox_constructor);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
geodstbox_constructor(PG_FUNCTION_ARGS)
{
	return stbox_constructor1(fcinfo, true, false, false, true);
}

PG_FUNCTION_INFO_V1(geodstbox_constructor_z);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
geodstbox_constructor_z(PG_FUNCTION_ARGS)
{
	return stbox_constructor1(fcinfo, true, true, false, true);
}

PG_FUNCTION_INFO_V1(geodstbox_constructor_zt);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
geodstbox_constructor_zt(PG_FUNCTION_ARGS)
{
	return stbox_constructor1(fcinfo, true, true, true, true);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

/**
 * Cast the spatiotemporal box as a GBOX value for PostGIS
 */
GBOX *
stbox_to_gbox(const STBOX *box)
{
	assert(MOBDB_FLAGS_GET_X(box->flags));
	/* Initialize existing dimensions */
	GBOX *result = palloc0(sizeof(GBOX));
	result->xmin = box->xmin;
	result->xmax = box->xmax;
	result->ymin = box->ymin;
	result->ymax = box->ymax;
	result->zmin = box->zmin;
	result->zmax = box->zmax;
	FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(box->flags));
	FLAGS_SET_M(result->flags, 0);
	FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(box->flags));
	return result;
}

PG_FUNCTION_INFO_V1(stbox_to_period);
/**
 * Cast the spatiotemporal box as a period
 */
PGDLLEXPORT Datum
stbox_to_period(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (!MOBDB_FLAGS_GET_T(box->flags))
		elog(ERROR, "The box does not have time dimension");

	Period *result = period_make(box->tmin, box->tmax, true, true);
	PG_RETURN_POINTER(result);
}

/* Cast an STBOX as a box2d */

PG_FUNCTION_INFO_V1(stbox_to_box2d);
/**
 * Cast the spatiotemporal box as a GBOX value for PostGIS
 */
PGDLLEXPORT Datum
stbox_to_box2d(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (!MOBDB_FLAGS_GET_X(box->flags))
		elog(ERROR, "The box does not have XY(Z) dimensions");

	/* Initialize existing dimensions */
	GBOX *result = palloc0(sizeof(GBOX));
	result->xmin = box->xmin;
	result->xmax = box->xmax;
	result->ymin = box->ymin;
	result->ymax = box->ymax;
	/* Strip out higher dimensions */
	FLAGS_SET_Z(result->flags, 0);
	FLAGS_SET_M(result->flags, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(stbox_to_box3d);
/**
 * Cast the spatiotemporal box as a BOX3D value for PostGIS
 */
PGDLLEXPORT Datum
stbox_to_box3d(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (!MOBDB_FLAGS_GET_X(box->flags))
		elog(ERROR, "The box does not have XY(Z) dimensions");

	/* Initialize existing dimensions */
	BOX3D *result = palloc0(sizeof(BOX3D));
	result->xmin = box->xmin;
	result->xmax = box->xmax;
	result->ymin = box->ymin;
	result->ymax = box->ymax;
	if (MOBDB_FLAGS_GET_Z(box->flags))
	{
		result->zmin = box->zmin;
		result->zmax = box->zmax;
	}
	else
	{
		result->zmin = result->zmax = 0;
	}
	result->srid = box->srid;
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transform a <Type> to a STBOX
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

PG_FUNCTION_INFO_V1(box2d_to_stbox);
/**
 * Transform a box2d to a spatiotemporal box
 */
PGDLLEXPORT Datum
box2d_to_stbox(PG_FUNCTION_ARGS)
{
	GBOX *box = (GBOX *)PG_GETARG_POINTER(0);
	STBOX *result = palloc0(sizeof(STBOX));
	result->xmin = box->xmin;
	result->xmax = box->xmax;
	result->ymin = box->ymin;
	result->ymax = box->ymax;
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_Z(result->flags, false);
	MOBDB_FLAGS_SET_T(result->flags, false);
	MOBDB_FLAGS_SET_GEODETIC(result->flags, false);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(box3d_to_stbox);
/**
 * Transform a box3d to a spatiotemporal box
 */
PGDLLEXPORT Datum
box3d_to_stbox(PG_FUNCTION_ARGS)
{
	BOX3D *box = (BOX3D *)PG_GETARG_POINTER(0);
	STBOX *result = palloc0(sizeof(STBOX));
	result->xmin = box->xmin;
	result->xmax = box->xmax;
	result->ymin = box->ymin;
	result->ymax = box->ymax;
	result->zmin = box->zmin;
	result->zmax = box->zmax;
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_Z(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, false);
	MOBDB_FLAGS_SET_GEODETIC(result->flags, false);
	result->srid = box->srid;
	PG_RETURN_POINTER(result);
}

/**
 * Transform a geometry/geography to a spatiotemporal box
 * (internal function)
 */
bool
geo_to_stbox_internal(STBOX *box, const GSERIALIZED *gs)
{
	GBOX gbox;
	if (gserialized_get_gbox_p(gs, &gbox) == LW_FAILURE)
	{
		/* Spatial dimensions are set as missing for the SP-GiST index */
		MOBDB_FLAGS_SET_X(box->flags, false);
		MOBDB_FLAGS_SET_Z(box->flags, false);
		MOBDB_FLAGS_SET_T(box->flags, false);
		return false;
	}
	box->xmin = gbox.xmin;
	box->xmax = gbox.xmax;
	box->ymin = gbox.ymin;
	box->ymax = gbox.ymax;
	if (FLAGS_GET_Z(gs->flags) || FLAGS_GET_GEODETIC(gs->flags))
	{
		box->zmin = gbox.zmin;
		box->zmax = gbox.zmax;
	}
	box->srid = gserialized_get_srid(gs);
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_Z(box->flags, FLAGS_GET_Z(gs->flags));
	MOBDB_FLAGS_SET_T(box->flags, false);
	MOBDB_FLAGS_SET_GEODETIC(box->flags, FLAGS_GET_GEODETIC(gs->flags));
	return true;
}

PG_FUNCTION_INFO_V1(geo_to_stbox);
/**
 * Transform a geometry/geography to a spatiotemporal box
 */
PGDLLEXPORT Datum
geo_to_stbox(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	STBOX *result = palloc0(sizeof(STBOX));
	geo_to_stbox_internal(result, gs);
	PG_FREE_IF_COPY(gs, 0);
	PG_RETURN_POINTER(result);
}

/**
 * Transform a timestampt to a spatiotemporal box
 * (internal function)
 */
void
timestamp_to_stbox_internal(STBOX *box, TimestampTz t)
{
	box->tmin = box->tmax = t;
	MOBDB_FLAGS_SET_X(box->flags, false);
	MOBDB_FLAGS_SET_Z(box->flags, false);
	MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(timestamp_to_stbox);
/**
 * Transform a timestampt to a spatiotemporal box
 */
PGDLLEXPORT Datum
timestamp_to_stbox(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	STBOX *result = palloc0(sizeof(STBOX));
	timestamp_to_stbox_internal(result, t);
	PG_RETURN_POINTER(result);
}

/**
 * Transform a timestamp set to a spatiotemporal box
 * (internal function)
 */
void
timestampset_to_stbox_internal(STBOX *box, const TimestampSet *ts)
{
	Period *p = timestampset_bbox(ts);
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(timestampset_to_stbox);
/**
 * Transform a timestamp set to a spatiotemporal box
 */
PGDLLEXPORT Datum
timestampset_to_stbox(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	STBOX *result = palloc0(sizeof(STBOX));
	timestampset_to_stbox_internal(result, ts);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_POINTER(result);
}

/**
 * Transform a period to a spatiotemporal box
 * (internal function)
 */
void
period_to_stbox_internal(STBOX *box, const Period *p)
{
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(period_to_stbox);
/**
 * Transform a period to a spatiotemporal box
 */
PGDLLEXPORT Datum
period_to_stbox(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	STBOX *result = palloc0(sizeof(STBOX));
	period_to_stbox_internal(result, p);
	PG_RETURN_POINTER(result);
}

/**
 * Transform a period set to a spatiotemporal box
 * (internal function)
 */
void
periodset_to_stbox_internal(STBOX *box, const PeriodSet *ps)
{
	Period *p = periodset_bbox(ps);
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(periodset_to_stbox);
/**
 * Transform a period set to a spatiotemporal box
 */
PGDLLEXPORT Datum
periodset_to_stbox(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	STBOX *result = palloc0(sizeof(STBOX));
	periodset_to_stbox_internal(result, ps);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(geo_timestamp_to_stbox);
/**
 * Transform a geometry/geography and a timestamp to a spatiotemporal box
 */
PGDLLEXPORT Datum
geo_timestamp_to_stbox(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	STBOX *result = palloc0(sizeof(STBOX));
	geo_to_stbox_internal(result, gs);
	result->tmin = result->tmax = t;
	MOBDB_FLAGS_SET_T(result->flags, true);	
	PG_FREE_IF_COPY(gs, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(geo_period_to_stbox);
/**
 * Transform a geometry/geography and a period to a spatiotemporal box
 */
PGDLLEXPORT Datum
geo_period_to_stbox(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Period *p = PG_GETARG_PERIOD(1);
	STBOX *result = palloc0(sizeof(STBOX));
	geo_to_stbox_internal(result, gs);
	result->tmin = p->lower;
	result->tmax = p->upper;
	MOBDB_FLAGS_SET_T(result->flags, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_hasx);
/**
 * Returns true if the spatiotemporal box has X dimension
 */
PGDLLEXPORT Datum
stbox_hasx(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	bool result = MOBDB_FLAGS_GET_X(box->flags);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(stbox_hasz);
/**
 * Returns true if the spatiotemporal box has Z dimension
 */
PGDLLEXPORT Datum
stbox_hasz(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	bool result = MOBDB_FLAGS_GET_Z(box->flags);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(stbox_hast);
/**
 * Returns true if the spatiotemporal box has T dimension
 */
PGDLLEXPORT Datum
stbox_hast(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	bool result = MOBDB_FLAGS_GET_T(box->flags);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(stbox_isgeodetic);
/**
 * Returns true if the spatiotemporal box is geodetic
 */
PGDLLEXPORT Datum
stbox_isgeodetic(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	bool result = MOBDB_FLAGS_GET_GEODETIC(box->flags);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(stbox_xmin);
/**
 * Returns the minimum X value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_xmin(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (!MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_FLOAT8(box->xmin);
}

PG_FUNCTION_INFO_V1(stbox_xmax);
/**
 * Returns the maximum X value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_xmax(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (!MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_FLOAT8(box->xmax);
}

PG_FUNCTION_INFO_V1(stbox_ymin);
/**
 * Returns the minimum Y value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_ymin(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (!MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_FLOAT8(box->ymin);
}

PG_FUNCTION_INFO_V1(stbox_ymax);
/**
 * Returns the maximum Y value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_ymax(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (!MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_FLOAT8(box->ymax);
}

PG_FUNCTION_INFO_V1(stbox_zmin);
/**
 * Returns the minimum Z value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_zmin(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (!MOBDB_FLAGS_GET_Z(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_FLOAT8(box->zmin);
}

PG_FUNCTION_INFO_V1(stbox_zmax);
/**
 * Returns the maximum Z value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_zmax(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (!MOBDB_FLAGS_GET_Z(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_FLOAT8(box->zmax);
}

PG_FUNCTION_INFO_V1(stbox_tmin);
/**
 * Returns the minimum timestamp value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_tmin(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (!MOBDB_FLAGS_GET_T(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_TIMESTAMPTZ(box->tmin);
}

PG_FUNCTION_INFO_V1(stbox_tmax);
/**
 * Returns the maximum timestamp value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_tmax(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	if (!MOBDB_FLAGS_GET_T(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_TIMESTAMPTZ(box->tmax);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * Expand the spatial dimension of the spatiotemporal box with the double value
 * (internal function)
 */
STBOX *
stbox_expand_spatial_internal(STBOX *box, double d)
{
	ensure_has_X_stbox(box);
	STBOX *result = stbox_copy(box);
	result->xmin = box->xmin - d;
	result->xmax = box->xmax + d;
	result->ymin = box->ymin - d;
	result->ymax = box->ymax + d;
	if (MOBDB_FLAGS_GET_Z(box->flags) || MOBDB_FLAGS_GET_GEODETIC(box->flags))
	{
		result->zmin = box->zmin - d;
		result->zmax = box->zmax + d;
	}
	return result;
}

PG_FUNCTION_INFO_V1(stbox_expand_spatial);
/**
 * Expand the spatial dimension of the spatiotemporal box with the double value
 */
PGDLLEXPORT Datum
stbox_expand_spatial(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	double d = PG_GETARG_FLOAT8(1);
	PG_RETURN_POINTER(stbox_expand_spatial_internal(box, d));
}

/**
 * Expand the temporal dimension of the spatiotemporal box with the interval value
 * (internal function)
 */
STBOX *
stbox_expand_temporal_internal(STBOX *box, Datum interval)
{
	ensure_has_T_stbox(box);
	STBOX *result = stbox_copy(box);
	result->tmin = DatumGetTimestampTz(call_function2(timestamp_mi_interval, 
		TimestampTzGetDatum(box->tmin), interval));
	result->tmax = DatumGetTimestampTz(call_function2(timestamp_pl_interval, 
		TimestampTzGetDatum(box->tmax), interval));
	return result;
}

PG_FUNCTION_INFO_V1(stbox_expand_temporal);
/**
 * Expand the temporal dimension of the spatiotemporal box with the interval value
 */
PGDLLEXPORT Datum
stbox_expand_temporal(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Datum interval = PG_GETARG_DATUM(1);
	PG_RETURN_POINTER(stbox_expand_temporal_internal(box, interval));
}

PG_FUNCTION_INFO_V1(stbox_set_precision);
/**
 * Sets the precision of the coordinates of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_set_precision(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Datum size = PG_GETARG_DATUM(1);
	ensure_has_X_stbox(box);
	STBOX *result = stbox_copy(box);
	result->xmin = DatumGetFloat8(datum_round(Float8GetDatum(box->xmin), size));
	result->xmax = DatumGetFloat8(datum_round(Float8GetDatum(box->xmax), size));
	result->ymin = DatumGetFloat8(datum_round(Float8GetDatum(box->ymin), size));
	result->ymax = DatumGetFloat8(datum_round(Float8GetDatum(box->ymax), size));
	if (MOBDB_FLAGS_GET_Z(box->flags) || MOBDB_FLAGS_GET_GEODETIC(box->flags))
	{
		result->zmin = DatumGetFloat8(datum_round(Float8GetDatum(box->zmin), size));
		result->zmax = DatumGetFloat8(datum_round(Float8GetDatum(box->zmax), size));
	}
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

/**
 * Set the ouput variables with the values of the flags of the boxes.
 *
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hasz,hast,geodetic Boolean variables
 */
static void
topo_stbox_stbox_flags(const STBOX *box1, const STBOX *box2, bool *hasx,
	bool *hasz, bool *hast, bool *geodetic)
{
	ensure_common_dimension_stbox(box1, box2);
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);

	*hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
	*hasz = MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags);
	*hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
	*geodetic = MOBDB_FLAGS_GET_GEODETIC(box1->flags) && MOBDB_FLAGS_GET_GEODETIC(box2->flags);
}
	
/**
 * Returns true if the first spatiotemporal box contains the second one
 * (internal function)
 */
bool
contains_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	bool hasx, hasz, hast, geodetic;
	topo_stbox_stbox_flags(box1, box2, &hasx, &hasz, &hast, &geodetic);
	if (hasx && (box2->xmin < box1->xmin || box2->xmax > box1->xmax ||
		box2->ymin < box1->ymin || box2->ymax > box1->ymax))
			return false;
	if ((hasz || geodetic) && (box2->zmin < box1->zmin || box2->zmax > box1->zmax))
			return false;
	if (hast && (box2->tmin < box1->tmin || box2->tmax > box1->tmax))
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(contains_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box contains the second one
 */
PGDLLEXPORT Datum
contains_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(contains_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is contained by the second one
 * (internal function)
 */
bool
contained_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	return contains_stbox_stbox_internal(box2, box1);
}

PG_FUNCTION_INFO_V1(contained_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is contained by the second one
 */
PGDLLEXPORT Datum
contained_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(contained_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the spatiotemporal boxes overlap
 * (internal function)
 */
bool
overlaps_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	bool hasx, hasz, hast, geodetic;
	topo_stbox_stbox_flags(box1, box2, &hasx, &hasz, &hast, &geodetic);
	if (hasx && (box1->xmax < box2->xmin || box1->xmin > box2->xmax ||
		box1->ymax < box2->ymin || box1->ymin > box2->ymax))
		return false;
	if ((hasz || geodetic) && (box1->zmax < box2->zmin || box1->zmin > box2->zmax))
		return false;
	if (hast && (box1->tmax < box2->tmin || box1->tmin > box2->tmax))
		return false;
	return true;
}

PG_FUNCTION_INFO_V1(overlaps_stbox_stbox);
/**
 * Returns true if the spatiotemporal boxes overlap
 */
PGDLLEXPORT Datum
overlaps_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(overlaps_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the spatiotemporal boxes are equal on the common dimensions
 * (internal function)
 */
bool
same_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	bool hasx, hasz, hast, geodetic;
	topo_stbox_stbox_flags(box1, box2, &hasx, &hasz, &hast, &geodetic);
	if (hasx && (box1->xmin != box2->xmin || box1->xmax != box2->xmax ||
		box1->ymin != box2->ymin || box1->ymax != box2->ymax))
		return false;
	if ((hasz || geodetic) && (box1->zmin != box2->zmin || box1->zmax != box2->zmax))
		return false;
	if (hast && (box1->tmin != box2->tmin || box1->tmax != box2->tmax))
		return false;
	return true;
}

PG_FUNCTION_INFO_V1(same_stbox_stbox);
/**
 * Returns true if the spatiotemporal boxes are equal on the common dimensions
 */
PGDLLEXPORT Datum
same_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(same_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the spatiotemporal boxes are adjacent
 * (internal function)
 */
bool
adjacent_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	bool hasx, hasz, hast, geodetic;
	topo_stbox_stbox_flags(box1, box2, &hasx, &hasz, &hast, &geodetic);
	STBOX *inter = stbox_intersection_internal(box1, box2);
	if (inter == NULL)
		return false;
	/* Boxes are adjacent if they share n dimensions and their intersection is
	 * at most of n-1 dimensions */
	if (!hasx && hast)
		return inter->tmin == inter->tmax;
	else if (hasx && !hast)
	{
		if (hasz || geodetic)
			return inter->xmin == inter->xmax || inter->ymin == inter->ymax ||
				   inter->zmin == inter->zmax;
		else
			return inter->xmin == inter->xmax || inter->ymin == inter->ymax;
	}
	else
	{
		if (hasz || geodetic)
			return inter->xmin == inter->xmax || inter->ymin == inter->ymax ||
				   inter->zmin == inter->zmax || inter->tmin == inter->tmax;
		else
			return inter->xmin == inter->xmax || inter->ymin == inter->ymax ||
				   inter->tmin == inter->tmax;
	}
}

PG_FUNCTION_INFO_V1(adjacent_stbox_stbox);
/**
 * Returns true if the spatiotemporal boxes are adjacent
 */
PGDLLEXPORT Datum
adjacent_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(adjacent_stbox_stbox_internal(box1, box2));
}

/*****************************************************************************
 * Position operators
 *****************************************************************************/

/**
 * Returns true if the first spatiotemporal box is strictly to the left of the second one
 * (internal function)
 */
bool
left_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_X_stbox(box1);
	ensure_has_X_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->xmax < box2->xmin);
}

PG_FUNCTION_INFO_V1(left_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly to the left of the second one
 */
PGDLLEXPORT Datum
left_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(left_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box does not extend to the right of the second one
 * (internal function)
 */
bool
overleft_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_X_stbox(box1);
	ensure_has_X_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->xmax <= box2->xmax);
}

PG_FUNCTION_INFO_V1(overleft_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box does not extend to the right of the second one
 */
PGDLLEXPORT Datum
overleft_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(overleft_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly to the right of the second one
 * (internal function)
 */
bool
right_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_X_stbox(box1);
	ensure_has_X_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->xmin > box2->xmax);
}

PG_FUNCTION_INFO_V1(right_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly to the right of the second one
 */
PGDLLEXPORT Datum
right_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(right_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatio temporal box does not extend to the left of the second one
 * (internal function)
 */
bool
overright_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_X_stbox(box1);
	ensure_has_X_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->xmin >= box2->xmin);
}

PG_FUNCTION_INFO_V1(overright_stbox_stbox);
/**
 * Returns true if the first spatio temporal box does not extend to the left of the second one
 */
PGDLLEXPORT Datum
overright_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(overright_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly below of the second one
 * (internal function)
 */
bool
below_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_X_stbox(box1);
	ensure_has_X_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->ymax < box2->ymin);
}

PG_FUNCTION_INFO_V1(below_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly below of the second one
 */
PGDLLEXPORT Datum
below_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(below_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box does not extend above of the second one
 * (internal function)
 */
bool
overbelow_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_X_stbox(box1);
	ensure_has_X_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->ymax <= box2->ymax);
}

PG_FUNCTION_INFO_V1(overbelow_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box does not extend above of the second one
 */
PGDLLEXPORT Datum
overbelow_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(overbelow_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly above of the second one
 * (internal function)
 */
bool
above_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_X_stbox(box1);
	ensure_has_X_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->ymin > box2->ymax);
}

PG_FUNCTION_INFO_V1(above_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly above of the second one
 */
PGDLLEXPORT Datum
above_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(above_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box does not extend below of the second one
 * (internal function)
 */
bool
overabove_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_X_stbox(box1);
	ensure_has_X_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->ymin >= box2->ymin);
}

PG_FUNCTION_INFO_V1(overabove_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box does not extend below of the second one
 */
PGDLLEXPORT Datum
overabove_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(overabove_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly in front of the second one
 * (internal function)
 */
bool
front_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_Z_stbox(box1);
	ensure_has_Z_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->zmax < box2->zmin);
}

PG_FUNCTION_INFO_V1(front_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly in front of the second one
 */
PGDLLEXPORT Datum
front_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(front_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box does not extend to the back of the second one
 * (internal function)
 */
bool
overfront_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_Z_stbox(box1);
	ensure_has_Z_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->zmax <= box2->zmax);
}

PG_FUNCTION_INFO_V1(overfront_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box does not extend to the back of the second one
 */
PGDLLEXPORT Datum
overfront_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(overfront_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly back of the second one
 * (internal function)
 */
bool
back_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_Z_stbox(box1);
	ensure_has_Z_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->zmin > box2->zmax);
}

PG_FUNCTION_INFO_V1(back_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly back of the second one
 */
PGDLLEXPORT Datum
back_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(back_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box does not extend to the front of the second one
 * (internal function)
 */
bool
overback_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_has_Z_stbox(box1);
	ensure_has_Z_stbox(box2);
	ensure_same_spatial_dimensionality_stbox(box1, box2);
	return (box1->zmin >= box2->zmin);
}

PG_FUNCTION_INFO_V1(overback_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box does not extend to the front of the second one
 */
PGDLLEXPORT Datum
overback_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(overback_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly before the second one
 * (internal function)
 */
bool
before_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_has_T_stbox(box1);
	ensure_has_T_stbox(box2);
	return (box1->tmax < box2->tmin);
}

PG_FUNCTION_INFO_V1(before_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly before the second one
 */
PGDLLEXPORT Datum
before_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(before_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box does not extend after the second one
 * (internal function)
 */
bool
overbefore_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_has_T_stbox(box1);
	ensure_has_T_stbox(box2);
	return (box1->tmax <= box2->tmax);
}

PG_FUNCTION_INFO_V1(overbefore_stbox_stbox);
/**
 * Returns true if the first temporal box does not extend after the second one
 */
PGDLLEXPORT Datum
overbefore_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(overbefore_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly after the second one
 * (internal function)
 */
bool
after_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_has_T_stbox(box1);
	ensure_has_T_stbox(box2);
	return (box1->tmin > box2->tmax);
}

PG_FUNCTION_INFO_V1(after_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly after the second one
 */
PGDLLEXPORT Datum
after_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(after_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box does not extend before the second one
 * (internal function)
 */
bool
overafter_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_has_T_stbox(box1);
	ensure_has_T_stbox(box2);
	return (box1->tmin >= box2->tmin);
}

PG_FUNCTION_INFO_V1(overafter_stbox_stbox);
/**
 * Returns true if the first temporal box does not extend before the second one
 */
PGDLLEXPORT Datum
overafter_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(overafter_stbox_stbox_internal(box1, box2));
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

/**
 * Returns the union of the spatiotemporal boxes
 * (internal function)
 */
STBOX *
stbox_union_internal(const STBOX *box1, const STBOX *box2, bool strict)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);
	ensure_same_dimensionality_stbox(box1, box2);
	/* If the strict parameter is true, we need to ensure that the boxes
	 * intersect, otherwise their union cannot be represented by a box */
	if (strict && ! overlaps_stbox_stbox_internal(box1, box2))
		elog(ERROR, "Result of box union would not be contiguous");

	STBOX *result = stbox_copy(box1);
	stbox_expand(result, box2);
	return(result);
}

PG_FUNCTION_INFO_V1(stbox_union);
/**
 * Returns the union of the spatiotemporal boxes
 */
PGDLLEXPORT Datum
stbox_union(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	STBOX *result = stbox_union_internal(box1, box2, true);
	PG_RETURN_POINTER(result);
}

/**
 * Returns the intersection of the spatiotemporal boxes
 * (internal function)
 */
STBOX *
stbox_intersection_internal(const STBOX *box1, const STBOX *box2)
{
	ensure_same_geodetic_stbox(box1, box2);
	ensure_same_srid_stbox(box1, box2);

	bool hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
	bool hasz = MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags);
	bool hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
	bool geodetic = MOBDB_FLAGS_GET_GEODETIC(box1->flags) && MOBDB_FLAGS_GET_GEODETIC(box2->flags);
	/* If there is no common dimension */
	if ((! hasx && ! hast) ||
		/* If they do no intersect in one common dimension */
		(hasx && (box1->xmin > box2->xmax || box2->xmin > box1->xmax ||
			box1->ymin > box2->ymax || box2->ymin > box1->ymax)) ||
		((hasz || geodetic) && (box1->zmin > box2->zmax || box2->zmin > box1->zmax)) ||
		(hast && (box1->tmin > box2->tmax || box2->tmin > box1->tmax)))
		return(NULL);

	double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
	TimestampTz tmin = 0, tmax = 0;
	if (hasx)
	{
		xmin = Max(box1->xmin, box2->xmin);
		xmax = Min(box1->xmax, box2->xmax);
		ymin = Max(box1->ymin, box2->ymin);
		ymax = Min(box1->ymax, box2->ymax);
		if (hasz || geodetic)
			{
			zmin = Max(box1->zmin, box2->zmin);
			zmax = Min(box1->zmax, box2->zmax);
			}
	}
	if (hast)
	{
		tmin = Max(box1->tmin, box2->tmin);
		tmax = Min(box1->tmax, box2->tmax);
	}
	return stbox_make(hasx, hasz, hast, geodetic, box1->srid,
		xmin, xmax, ymin, ymax, zmin, zmax, tmin, tmax);
}

PG_FUNCTION_INFO_V1(stbox_intersection);
/**
 * Returns the intersection of the spatiotemporal boxes
 */
PGDLLEXPORT Datum
stbox_intersection(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	STBOX *result = stbox_intersection_internal(box1, box2);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * Returns -1, 0, or 1 depending on whether the first spatiotemporal box
 * is less than, equal, or greater than the second one
 * (internal function)
 *
 * @note Function used for B-tree comparison
 */
int 
stbox_cmp_internal(const STBOX *box1, const STBOX *box2)
{
	/* Compare the SRID */
	if (box1->srid < box2->srid)
		return -1;
	if (box1->srid > box2->srid)
		return 1;

	/* Compare the box minima */
	if (MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags))
	{
		if (box1->tmin < box2->tmin)
			return -1;
		if (box1->tmin > box2->tmin)
			return 1;
	}
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags))
	{
		if (box1->xmin < box2->xmin)
			return -1;
		if (box1->xmin > box2->xmin)
			return 1;
		if (box1->ymin < box2->ymin)
			return -1;
		if (box1->ymin > box2->ymin)
			return 1;
	}
	if (MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags))
	{
		if (box1->zmin < box2->zmin)
			return -1;
		if (box1->zmin > box2->zmin)
			return 1;
	}
	/* Compare the box maxima */
	if (MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags))
	{
		if (box1->tmax < box2->tmax)
			return -1;
		if (box1->tmax > box2->tmax)
			return 1;
	}
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags))
	{
		if (box1->xmax < box2->xmax)
			return -1;
		if (box1->xmax > box2->xmax)
			return 1;
		if (box1->ymax < box2->ymax)
			return -1;
		if (box1->ymax > box2->ymax)
			return 1;
	}
	if (MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags))
	{
		if (box1->zmax < box2->zmax)
			return -1;
		if (box1->zmax > box2->zmax)
			return 1;
	}
	/* Finally compare the flags */
	if (box1->flags < box2->flags)
		return -1;
	if (box1->flags > box2->flags)
		return 1;
	/* The two boxes are equal */
	return 0;
}

PG_FUNCTION_INFO_V1(stbox_cmp);
/**
 * Returns -1, 0, or 1 depending on whether the first spatiotemporal box
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
PGDLLEXPORT Datum
stbox_cmp(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	int	cmp = stbox_cmp_internal(box1, box2);
	PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(stbox_lt);
/**
 * Returns true if the first spatiotemporal box is less than the second one
 */
PGDLLEXPORT Datum
stbox_lt(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	int	cmp = stbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(stbox_le);
/**
 * Returns true if the first spatiotemporal box is less than or equal to
 * the second one
 */
PGDLLEXPORT Datum
stbox_le(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	int	cmp = stbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(stbox_ge);
/**
 * Returns true if the first spatiotemporal box is greater than or equal to
 * the second one
 */
PGDLLEXPORT Datum
stbox_ge(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	int	cmp = stbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(stbox_gt);
/**
 * Returns true if the first spatiotemporal box is greater than the second one
 */
PGDLLEXPORT Datum
stbox_gt(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	int	cmp = stbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp > 0);
}

/**
 * Returns true if the two spatiotemporal boxes are equal 
 * (internal function)
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
stbox_eq_internal(const STBOX *box1, const STBOX *box2)
{
	if (box1->xmin != box2->xmin || box1->ymin != box2->ymin ||
		box1->zmin != box2->zmin || box1->tmin != box2->tmin ||
		box1->xmax != box2->xmax || box1->ymax != box2->ymax ||
		box1->zmax != box2->zmax || box1->tmax != box2->tmax ||
		box1->flags != box2->flags || box1->srid != box2->srid)
		return false;
	/* The two boxes are equal */
	return true;
}

PG_FUNCTION_INFO_V1(stbox_eq);
/**
 * Returns true if the two spatiotemporal boxes are equal 
 */
PGDLLEXPORT Datum
stbox_eq(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(stbox_eq_internal(box1, box2));
}

PG_FUNCTION_INFO_V1(stbox_ne);
/**
 * Returns true if the two spatiotemporal boxes are different 
 */
PGDLLEXPORT Datum
stbox_ne(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	PG_RETURN_BOOL(! stbox_eq_internal(box1, box2));
}

/*****************************************************************************/

