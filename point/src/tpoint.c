/*****************************************************************************
 *
 * tpoint.c
 *	  Basic functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint.h"

#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "lifting.h"
#include "temporal_compops.h"
#include "stbox.h"
#include "tpoint_parser.h"
#include "tpoint_boxops.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

#define PGC_ERRMSG_MAXLEN 2048

/**
 * Output an error message
 */
static void
pg_error(const char *fmt, va_list ap)
{
	char errmsg[PGC_ERRMSG_MAXLEN + 1];

	vsnprintf (errmsg, PGC_ERRMSG_MAXLEN, fmt, ap);

	errmsg[PGC_ERRMSG_MAXLEN]='\0';
	ereport(ERROR, (errmsg_internal("%s", errmsg)));
}

/**
 * Output a notice message
 */
static void
pg_notice(const char *fmt, va_list ap)
{
	char errmsg[PGC_ERRMSG_MAXLEN + 1];

	vsnprintf (errmsg, PGC_ERRMSG_MAXLEN, fmt, ap);

	errmsg[PGC_ERRMSG_MAXLEN]='\0';
	ereport(NOTICE, (errmsg_internal("%s", errmsg)));
}

/**
 * Set the handlers for initializing the liblwgeom library
 */
void temporalgeom_init()
{
	lwgeom_set_handlers(palloc, repalloc, pfree, pg_error, pg_notice);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * Check the consistency of the metadata we want to enforce in the typmod:
 * SRID, type and dimensionality. If things are inconsistent, shut down the query.
 */
static Temporal *
tpoint_valid_typmod(Temporal *temp, int32_t typmod)
{
	int32 tpoint_srid = tpoint_srid_internal(temp);
	TDuration tpoint_duration = temp->duration;
	TDuration typmod_duration = TYPMOD_GET_DURATION(typmod);
	TYPMOD_DEL_DURATION(typmod);
	/* If there is no geometry type */
	if (typmod == 0)
		typmod = -1;
	int32 tpoint_z = MOBDB_FLAGS_GET_Z(temp->flags);
	int32 typmod_srid = TYPMOD_GET_SRID(typmod);
	int32 typmod_type = TYPMOD_GET_TYPE(typmod);
	int32 typmod_z = TYPMOD_GET_Z(typmod);

	/* No typmod (-1) */
	if (typmod < 0 && typmod_duration == ANYDURATION)
		return temp;
	/* Typmod has a preference for SRID? Geometry SRID had better match */
	if (typmod_srid > 0 && typmod_srid != tpoint_srid)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Temporal point SRID (%d) does not match column SRID (%d)",
					tpoint_srid, typmod_srid) ));
	/* Typmod has a preference for temporal type */
	if (typmod_type > 0 && typmod_duration != ANYDURATION && typmod_duration != tpoint_duration)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Temporal type (%s) does not match column type (%s)",
					temporal_duration_name(tpoint_duration), temporal_duration_name(typmod_duration)) ));
	/* Mismatched Z dimensionality.  */
	if (typmod > 0 && typmod_z && ! tpoint_z)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Column has Z dimension but temporal point does not" )));
	/* Mismatched Z dimensionality (other way) */
	if (typmod > 0 && tpoint_z && ! typmod_z)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Temporal point has Z dimension but column does not" )));

	return temp;
}

PG_FUNCTION_INFO_V1(tpoint_in);
/**
 * Generic input function for temporal points
 *
 * @note Examples of input for the various durations:
 * - Instant
 * @code
 * Point(0 0) @ 2012-01-01 08:00:00
 * @endcode
 * - Instant set
 * @code
 * { Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 }
 * @endcode
 * - Sequence
 * @code
 * [ Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 )
 * @endcode
 * - Sequence set
 * @code
 * { [ Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 ) ,
 * [ Point(1 1) @ 2012-01-01 08:20:00 , Point(0 0) @ 2012-01-01 08:30:00 ] }
 * @endcode
 */
PGDLLEXPORT Datum
tpoint_in(PG_FUNCTION_ARGS) 
{
	char *input = PG_GETARG_CSTRING(0);
	Oid temptypid = PG_GETARG_OID(1);
	Oid valuetypid = temporal_valuetypid(temptypid);
	Temporal *result = tpoint_parse(&input, valuetypid);
	PG_RETURN_POINTER(result);
}

/**
 * Input typmod information for temporal points
 */
static uint32 
tpoint_typmod_in(ArrayType *arr, int is_geography)
{
	uint32 typmod = 0;
	Datum *elem_values;
	int n = 0;

	if (ARR_ELEMTYPE(arr) != CSTRINGOID)
		ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
			errmsg("typmod array must be type cstring[]")));
	if (ARR_NDIM(arr) != 1)
		ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
			errmsg("typmod array must be one-dimensional")));
	if (ARR_HASNULL(arr))
		ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
			errmsg("typmod array must not contain nulls")));

	/*
	 * There are several ways to define a column wrt type modifiers:
	 *   column_type(Duration, Geometry, SRID) => All modifiers are determined.
	 * 	 column_type(Duration, Geometry) => The SRID is generic.
	 * 	 column_type(Geometry, SRID) => The duration type is generic.
	 * 	 column_type(Geometry) => The duration type and SRID are generic.
	 *	 column_type(Duration) => The geometry type and SRID are generic.
	 *	 column_type => The duration type, geometry type, and SRID are generic.
	 *
	 * For example, if the user did not set the duration type, we can use all 
	 * duration types in the same column. Similarly for all generic modifiers.
	 */
	deconstruct_array(arr, CSTRINGOID, -2, false, 'c', &elem_values, NULL, &n);
	TDuration duration = ANYDURATION;
	uint8_t geometry_type = 0;
	int hasZ = 0, hasM = 0;
	char *s;
	
	switch(n)
	{
		case 3: 
		{
			/* Type_modifier is (Duration, Geometry, SRID) */
			/* Duration type */
			s = DatumGetCString(elem_values[0]);
			if (temporal_duration_from_string(s, &duration) == false) 
				ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
						errmsg("Invalid duration type modifier: %s", s)));

			/* Shift to remove the 4 bits of the duration */
			TYPMOD_DEL_DURATION(typmod);
			/* Set default values */
			if (is_geography)
				TYPMOD_SET_SRID(typmod, SRID_DEFAULT);
			else
				TYPMOD_SET_SRID(typmod, SRID_UNKNOWN);
						
			/* Geometry type */
			s = DatumGetCString(elem_values[1]);
			if (geometry_type_from_string(s, &geometry_type, &hasZ, &hasM) == LW_FAILURE) 
				ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
						errmsg("Invalid geometry type modifier: %s", s)));
			if (geometry_type != POINTTYPE || hasM)
				ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					errmsg("Only point geometries without M dimension accepted")));

			TYPMOD_SET_TYPE(typmod, geometry_type);
			if (hasZ)
				TYPMOD_SET_Z(typmod);
		
			/* SRID */
			s = DatumGetCString(elem_values[2]);
			int srid = pg_atoi(s, sizeof(int32), '\0');
			srid = clamp_srid(srid);
			if (srid != SRID_UNKNOWN)
				TYPMOD_SET_SRID(typmod, srid);
			/* Shift to restore the 4 bits of the duration */
			TYPMOD_SET_DURATION(typmod, duration);
			break;
		}
		case 2:
		{
			/* Type modifier is either (Duration, Geometry) or (Geometry, SRID) */
			s = DatumGetCString(elem_values[0]);
			if (temporal_duration_from_string(s, &duration)) 
			{
				/* Type modifier is (Duration, Geometry) */
				/* Shift to remove the 4 bits of the duration */
				TYPMOD_DEL_DURATION(typmod);
				/* Set default values */
				if (is_geography)
					TYPMOD_SET_SRID(typmod, SRID_DEFAULT);
				else
					TYPMOD_SET_SRID(typmod, SRID_UNKNOWN);
				/* Geometry type */
				s = DatumGetCString(elem_values[1]);
				if (geometry_type_from_string(s, &geometry_type, &hasZ, &hasM) == LW_FAILURE)
					ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
							errmsg("Invalid geometry type modifier: %s", s)));
				if (geometry_type != POINTTYPE || hasM)
					ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
						errmsg("Only point geometries without M dimension accepted")));

				TYPMOD_SET_TYPE(typmod, geometry_type);
				if (hasZ)
					TYPMOD_SET_Z(typmod);
				/* Shift to restore the 4 bits of the duration */
				TYPMOD_SET_DURATION(typmod, duration);
			}
			else if (geometry_type_from_string(s, &geometry_type, &hasZ, &hasM))
			{
				/* Type modifier is (Geometry, SRID) */
				if (geometry_type != POINTTYPE || hasM)
					ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
						errmsg("Only point geometries without M dimension accepted")));

				/* Shift to remove the 4 bits of the duration */
				TYPMOD_DEL_DURATION(typmod);
				/* Set default values */
				if (is_geography)
					TYPMOD_SET_SRID(typmod, SRID_DEFAULT);
				else
					TYPMOD_SET_SRID(typmod, SRID_UNKNOWN);
				
				TYPMOD_SET_TYPE(typmod, geometry_type);
				if (hasZ)
					TYPMOD_SET_Z(typmod);
				/* SRID */
				s = DatumGetCString(elem_values[1]);
				int srid = pg_atoi(s, sizeof(int32), '\0');
				srid = clamp_srid(srid);
				if (srid != SRID_UNKNOWN)
					TYPMOD_SET_SRID(typmod, srid);
				/* Shift to restore the 4 bits of the duration */
				TYPMOD_SET_DURATION(typmod, duration);
			}
			else
				ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					errmsg("Invalid temporal point type modifier: %s", s)));
			break;
		}
		case 1:
		{
			/* Type modifier: either (Duration) or (Geometry) */
			s = DatumGetCString(elem_values[0]);
			if (temporal_duration_from_string(s, &duration))
			{
				TYPMOD_SET_DURATION(typmod, duration);
			}
			else if (geometry_type_from_string(s, &geometry_type, &hasZ, &hasM)) 
			{
				if (geometry_type != POINTTYPE)
					ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
						errmsg("Only point geometries accepted")));

				/* Shift to remove the 4 bits of the duration */
				TYPMOD_DEL_DURATION(typmod);
				/* Set default values */
				if (is_geography)
					TYPMOD_SET_SRID(typmod, SRID_DEFAULT);
				else
					TYPMOD_SET_SRID(typmod, SRID_UNKNOWN);

				TYPMOD_SET_TYPE(typmod, geometry_type);
				if (hasZ)
					TYPMOD_SET_Z(typmod);

				/* Shift to restore the 4 bits of the duration */
				TYPMOD_SET_DURATION(typmod, duration);
			}
			else
				ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					errmsg("Invalid temporal point type modifier: %s", s)));
			break;
		}
		default:
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					errmsg("Invalid temporal point type modifier:")));
	}
	pfree(elem_values);
	return typmod;
}

PG_FUNCTION_INFO_V1(tgeompoint_typmod_in);
/**
 * Input typmod information for temporal geometric points
 */
PGDLLEXPORT Datum 
tgeompoint_typmod_in(PG_FUNCTION_ARGS)
{
	ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
	uint32 typmod = tpoint_typmod_in(array, false); /* Not a geography  */;
	PG_RETURN_INT32(typmod);
}

PG_FUNCTION_INFO_V1(tgeogpoint_typmod_in);
/**
 * Input typmod information for temporal geographic points
 */
PGDLLEXPORT Datum 
tgeogpoint_typmod_in(PG_FUNCTION_ARGS)
{
	ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
	int32 typmod = tpoint_typmod_in(array, true);
	// int srid = TYPMOD_GET_SRID(typmod);
	// /* Check the SRID is legal (geographic coordinates) */
	// srid_is_latlong(fcinfo, srid);
	PG_RETURN_INT32(typmod);
}

PG_FUNCTION_INFO_V1(tpoint_typmod_out);
/**
 * Output typmod information for temporal points
 */
PGDLLEXPORT Datum 
tpoint_typmod_out(PG_FUNCTION_ARGS)
{
	char *s = (char *) palloc(64);
	char *str = s;
	int32 typmod = PG_GETARG_INT32(0);
	TDuration duration = TYPMOD_GET_DURATION(typmod);
	TYPMOD_DEL_DURATION(typmod);
	int32 srid = TYPMOD_GET_SRID(typmod);
	uint8_t geometry_type = (uint8_t) TYPMOD_GET_TYPE(typmod);
	int32 hasz = TYPMOD_GET_Z(typmod);

	/* No duration type or geometry type? Then no typmod at all. 
	  Return empty string. */
	if (typmod < 0 || (duration == ANYDURATION && !geometry_type))
	{
		*str = '\0';
		PG_RETURN_CSTRING(str);
	}
	/* Opening bracket */
	str += sprintf(str, "(");
	/* Has duration type?  */
	if (duration != ANYDURATION)
		str += sprintf(str, "%s", temporal_duration_name(duration));
	if (geometry_type)
	{
		if (duration != ANYDURATION) str += sprintf(str, ",");
		str += sprintf(str, "%s", lwtype_name(geometry_type));
		/* Has Z?  */
		if (hasz) str += sprintf(str, "Z");
		/* Has SRID?  */
		if (srid) str += sprintf(str, ",%d", srid);
	}
	/* Closing bracket.  */
	sprintf(str, ")");

	PG_RETURN_CSTRING(s);
}

PG_FUNCTION_INFO_V1(tpoint_enforce_typmod);
/**
 * Enforce typmod information for temporal points with respect to
 * duration, dimensions, and SRID
 */
PGDLLEXPORT Datum
tpoint_enforce_typmod(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int32 typmod = PG_GETARG_INT32(1);
	/* Check if typmod of temporal point is consistent with the supplied one */
	temp = tpoint_valid_typmod(temp, typmod);
	PG_RETURN_POINTER(temp);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpointinst_constructor);
/**
 * Construct a temporal instant point value from the arguments
 */
PGDLLEXPORT Datum
tpointinst_constructor(PG_FUNCTION_ARGS) 
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	ensure_point_type(gs);
	ensure_non_empty(gs);
	ensure_has_not_M_gs(gs);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Temporal *result = (Temporal *)tinstant_make(PointerGetDatum(gs),
		t, valuetypid);
	PG_FREE_IF_COPY(gs, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_stbox);
/**
 * Returns the bounding box of the temporal point value
 */
PGDLLEXPORT Datum
tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *result = palloc0(sizeof(STBOX));
	temporal_bbox(result, temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Ever/always comparison operators
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_ever_eq);
/**
 * Returns true if the temporal value is ever equal to the base value
 */
PGDLLEXPORT Datum
tpoint_ever_eq(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	/* A temporal point is never equal to a empty geometry */
	if (gserialized_is_empty(gs))
		PG_RETURN_BOOL(false);
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ensure_point_type(gs);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	/* Bounding box test */
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	geo_to_stbox_internal(&box2, gs);
	if (!contains_stbox_stbox_internal(&box1, &box2))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_BOOL(false);
	}
	bool result = temporal_ever_eq_internal(temp, PointerGetDatum(gs));
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tpoint_always_eq);
/**
 * Returns true if the temporal value is always equal to the base value
 */
PGDLLEXPORT Datum
tpoint_always_eq(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	/* A temporal point is never equal to a empty geometry */
	if (gserialized_is_empty(gs))
		PG_RETURN_BOOL(false);
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ensure_point_type(gs);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	/* The bounding box test is enough to test the predicate */
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	geo_to_stbox_internal(&box2, gs);
	if (!same_stbox_stbox_internal(&box1, &box2))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_BOOL(false);
	}

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(true);
}

PG_FUNCTION_INFO_V1(tpoint_ever_ne);
/**
 * Returns true if the temporal value is ever different to the base value
 */
PGDLLEXPORT Datum
tpoint_ever_ne(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(! tpoint_always_eq(fcinfo));
}

PG_FUNCTION_INFO_V1(tpoint_always_ne);
/**
 * Returns true if the temporal value is always different to the base value
 */
PGDLLEXPORT Datum
tpoint_always_ne(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(! tpoint_ever_eq(fcinfo));
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * Returns the temporal comparison of the the base value and temporal value 
 */
Datum
tcomp_geo_tpoint(FunctionCallInfo fcinfo, 
	Datum (*func)(Datum, Datum, Oid, Oid))
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	ensure_point_type(gs);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Temporal *result = tcomp_temporal_base1(temp, PointerGetDatum(gs), 
		datumtypid, func, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

/**
 * Returns the temporal comparison of the temporal value and the base value
 */
Datum
tcomp_tpoint_geo(FunctionCallInfo fcinfo, 
	Datum (*func)(Datum, Datum, Oid, Oid))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_point_type(gs);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Temporal *result = tcomp_temporal_base1(temp, PointerGetDatum(gs), 
		datumtypid, func, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

/**
 * Returns the temporal comparison of the temporal values
 */
Datum
tcomp_tpoint_tpoint(FunctionCallInfo fcinfo, 
	Datum (*func)(Datum, Datum, Oid, Oid))
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2,
		(Datum) NULL, (varfunc) func, 4, BOOLOID, false, true, NULL);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(teq_geo_tpoint);
/**
 * Returns the temporal equality of the base value and the temporal value
 */
PGDLLEXPORT Datum
teq_geo_tpoint(PG_FUNCTION_ARGS)
{
	return tcomp_geo_tpoint(fcinfo, &datum2_eq2);
}

PG_FUNCTION_INFO_V1(teq_tpoint_geo);
/**
 * Returns the temporal equality of the temporal value and base value
 */
PGDLLEXPORT Datum
teq_tpoint_geo(PG_FUNCTION_ARGS)
{
	return tcomp_tpoint_geo(fcinfo, &datum2_eq2);
}

PG_FUNCTION_INFO_V1(teq_tpoint_tpoint);
/**
 * Returns the temporal equality of the temporal values
 */
PGDLLEXPORT Datum
teq_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return tcomp_tpoint_tpoint(fcinfo, &datum2_eq2);
}

PG_FUNCTION_INFO_V1(tne_geo_tpoint);
/**
 * Returns the temporal difference of the base value and the temporal value
 */
PGDLLEXPORT Datum
tne_geo_tpoint(PG_FUNCTION_ARGS)
{
	return tcomp_geo_tpoint(fcinfo, &datum2_ne2);
}

PG_FUNCTION_INFO_V1(tne_tpoint_geo);
/**
 * Returns the temporal difference of the temporal value and base value
 */
PGDLLEXPORT Datum
tne_tpoint_geo(PG_FUNCTION_ARGS)
{
	return tcomp_tpoint_geo(fcinfo, &datum2_ne2);
}

PG_FUNCTION_INFO_V1(tne_tpoint_tpoint);
/**
 * Returns the temporal difference of the temporal values
 */
PGDLLEXPORT Datum
tne_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return tcomp_tpoint_tpoint(fcinfo, &datum2_ne2);
}

/*****************************************************************************
 * Assemble the Trajectory/values of a temporal point as a single
 * geometry/geography.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_values);
/**
 * Returns the base values (that is, the trajectory) of the temporal point 
 * value as a geometry/geography
 */
PGDLLEXPORT Datum
tpoint_values(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = tpoint_trajectory_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_at_value);
/**
 * Restricts the temporal point value to the base point value
 */
PGDLLEXPORT Datum
tpoint_at_value(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	/* A temporal point is never equal to a empty geometry */
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ensure_point_type(gs);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	/* Bounding box test */
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	geo_to_stbox_internal(&box2, gs);
	if (!contains_stbox_stbox_internal(&box1, &box2))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = temporal_at_value_internal(temp, PointerGetDatum(gs));

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tpoint_minus_value);
/**
 * Restricts the temporal point value to the complement of the base point value
 */
PGDLLEXPORT Datum
tpoint_minus_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_point_type(gs);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	/* Bounding box test */
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box2, gs))
	{
		Temporal *result;
		if (temp->duration == SEQUENCE)
			result = (Temporal *)tsequenceset_make((TSequence **)&temp, 1, NORMALIZE_NO);
		else
			result = temporal_copy(temp);
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_POINTER(result);
	}
	temporal_bbox(&box1, temp);
	if (!contains_stbox_stbox_internal(&box1, &box2))
	{
		Temporal *result;
		if (temp->duration == SEQUENCE)
			result = (Temporal *)tsequenceset_make((TSequence **)&temp, 1, NORMALIZE_NO);
		else
			result = temporal_copy(temp);
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_POINTER(result);
	}

	Temporal *result = temporal_minus_value_internal(temp, PointerGetDatum(gs));

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

Datum
tpoint_restrict_values(FunctionCallInfo fcinfo, bool atfunc)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
	/* Return NULL on empty array */
	int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	if (count == 0)
	{
		if (atfunc)
		{
			PG_FREE_IF_COPY(temp, 0);
			PG_FREE_IF_COPY(array, 1);
			PG_RETURN_NULL();
		}
		else
		{
			Temporal *result = temporal_copy(temp);
			PG_FREE_IF_COPY(temp, 0);
			PG_FREE_IF_COPY(array, 1);
			PG_RETURN_POINTER(result);
		}
	}
	Datum *values = datumarr_extract(array, &count);
	for (int i = 0; i < count; i++)
	{
		GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(values[i]);
		ensure_point_type(gs);
		ensure_same_srid_tpoint_gs(temp, gs);
		ensure_same_dimensionality_tpoint_gs(temp, gs);
	}
	
	Temporal *result = temporal_restrict_values_internal(temp, values, count,  atfunc);

	pfree(values);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(array, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}


PG_FUNCTION_INFO_V1(tpoint_at_values);
/**
 * Restricts the temporal point value to the array of base point values
 */
PGDLLEXPORT Datum
tpoint_at_values(PG_FUNCTION_ARGS)
{
	return tpoint_restrict_values(fcinfo, true);
}

PG_FUNCTION_INFO_V1(tpoint_minus_values);
/**
 * Restricts the temporal point value to the complement of the array
 * of base point values
 */
PGDLLEXPORT Datum
tpoint_minus_values(PG_FUNCTION_ARGS)
{
	return tpoint_restrict_values(fcinfo, false);
}

/*****************************************************************************/
