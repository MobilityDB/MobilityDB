/*****************************************************************************
 *
 * TemporalPoint.c
 *	  Basic functions for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

/* Initialize the extension */

void temporalgeom_init()
{
	lwgeom_set_handlers(palloc, repalloc, pfree, NULL, NULL);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/*
 * Check the consistency of the metadata we want to enforce in the typmod:
 * srid, type and dimensionality. If things are inconsistent, shut down the query.
 */
Temporal*
tpoint_valid_typmod(Temporal *temp, int32_t typmod)
{
	int32 tpoint_srid = tpoint_srid_internal(temp);
	int32 tpoint_type = temp->type;
	int32 duration_type = TYPMOD_GET_DURATION(typmod);
	TYPMOD_DEL_DURATION(typmod);
	/* If there is no geometry type */
	if (typmod == 0)
		typmod = -1;
	int32 tpoint_z = MOBDB_FLAGS_GET_Z(temp->flags);
	int32 typmod_srid = TYPMOD_GET_SRID(typmod);
	int32 typmod_type = TYPMOD_GET_TYPE(typmod);
	int32 typmod_z = TYPMOD_GET_Z(typmod);

	/* No typmod (-1) */
	if (typmod < 0 && duration_type == 0)
		return temp;
	/* Typmod has a preference for SRID? Geometry SRID had better match.  */
	if ( typmod_srid > 0 && typmod_srid != tpoint_srid )
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Temporal point SRID (%d) does not match column SRID (%d)",
					tpoint_srid, typmod_srid) ));
	/* Typmod has a preference for temporal type.  */
	if (typmod_type > 0 && duration_type != 0 && duration_type != tpoint_type)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Temporal type (%s) does not match column type (%s)",
					temporal_type_name(tpoint_type), temporal_type_name(duration_type)) ));
	/* Mismatched Z dimensionality.  */
	if (typmod > 0 && typmod_z && ! tpoint_z)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Column has Z dimension but temporal point does not" )));
	/* Mismatched Z dimensionality (other way).  */
	if (typmod > 0 && tpoint_z && ! typmod_z)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Temporal type has Z dimension but column does not" )));

	return temp;
}

/* 
 * Input function. 
 * Examples of input:
 * - tpointinst
 *	  Point(0 0) @ 2012-01-01 08:00:00
 * - tpointi
 * 		{ Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 }
 * - tpointseq
 * 		[ Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 )
 * - tpoints
 * 		{ [ Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 ) ,
 *  	  [ Point(1 1) @ 2012-01-01 08:20:00 , Point(0 0) @ 2012-01-01 08:30:00 ] }
 */
PG_FUNCTION_INFO_V1(tpoint_in);

PGDLLEXPORT Datum
tpoint_in(PG_FUNCTION_ARGS) 
{
	char *input = PG_GETARG_CSTRING(0);
	Oid temptypid = PG_GETARG_OID(1);
	Oid valuetypid;
	temporal_typinfo(temptypid, &valuetypid);
	Temporal *result = tpoint_parse(&input, valuetypid);
	if (result == 0)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

static uint32 
tpoint_typmod_in(ArrayType *arr, int is_geography)
{
	int32 typmod = 0;
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
	uint8_t duration_type = 0, geometry_type = 0;
	int z = 0, m = 0;
	char *s;
	
	switch(n)
	{
		case 3: 
		{
			/* Type_modifier is (Duration, Geometry, SRID) */
			/* Duration type */
			s = DatumGetCString(elem_values[0]);
			if (temporal_type_from_string(s, &duration_type) == false) 
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
			if (geometry_type_from_string(s, &geometry_type, &z, &m) == LW_FAILURE) 
				ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
						errmsg("Invalid geometry type modifier: %s", s)));

			TYPMOD_SET_TYPE(typmod, geometry_type);
			if (z)
				TYPMOD_SET_Z(typmod);
			if (m)
				TYPMOD_SET_M(typmod);
			
			/* SRID */
			s = DatumGetCString(elem_values[2]);
			int srid = pg_atoi(s, sizeof(int32), '\0');
			srid = clamp_srid(srid);
			if (srid != SRID_UNKNOWN)
				TYPMOD_SET_SRID(typmod, srid);
			/* Shift to restore the 4 bits of the duration */
			TYPMOD_SET_DURATION(typmod, duration_type);
			break;
		}
		case 2:
		{
			/* Type modifier is either (Duration, Geometry) or (Geometry, SRID) */
			s = DatumGetCString(elem_values[0]);
			if (temporal_type_from_string(s, &duration_type)) 
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
				if (geometry_type_from_string(s, &geometry_type, &z, &m) == LW_FAILURE)
					ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
							errmsg("Invalid geometry type modifier: %s", s)));

				TYPMOD_SET_TYPE(typmod, geometry_type);
				if (z)
					TYPMOD_SET_Z(typmod);
				if (m)
					TYPMOD_SET_M(typmod);
				/* Shift to restore the 4 bits of the duration */
				TYPMOD_SET_DURATION(typmod, duration_type);
			}
			else if (geometry_type_from_string(s, &geometry_type, &z, &m))
			{
				/* Type modifier is (Geometry, SRID) */
				/* Shift to remove the 4 bits of the duration */
				TYPMOD_DEL_DURATION(typmod);
				/* Set default values */
				if (is_geography)
					TYPMOD_SET_SRID(typmod, SRID_DEFAULT);
				else
					TYPMOD_SET_SRID(typmod, SRID_UNKNOWN);
				
				TYPMOD_SET_TYPE(typmod, geometry_type);
				if (z)
					TYPMOD_SET_Z(typmod);
				if (m)
					TYPMOD_SET_M(typmod);

				/* SRID */
				s = DatumGetCString(elem_values[1]);
				int srid = pg_atoi(s, sizeof(int32), '\0');
				srid = clamp_srid(srid);
				if (srid != SRID_UNKNOWN)
					TYPMOD_SET_SRID(typmod, srid);
				/* Shift to restore the 4 bits of the duration */
				TYPMOD_SET_DURATION(typmod, duration_type);
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
			if (temporal_type_from_string(s, &duration_type))
			{
				TYPMOD_SET_DURATION(typmod, duration_type);
			}
			else if (geometry_type_from_string(s, &geometry_type, &z, &m)) 
			{
				/* Shift to remove the 4 bits of the duration */
				TYPMOD_DEL_DURATION(typmod);
				/* Set default values */
				if (is_geography)
					TYPMOD_SET_SRID(typmod, SRID_DEFAULT);
				else
					TYPMOD_SET_SRID(typmod, SRID_UNKNOWN);

				TYPMOD_SET_TYPE(typmod, geometry_type);
				if (z)
					TYPMOD_SET_Z(typmod);
				if (m)
					TYPMOD_SET_M(typmod);

				/* Shift to restore the 4 bits of the duration */
				TYPMOD_SET_DURATION(typmod, duration_type);
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

/*
 * typmod input for tgeompoint
 */
PG_FUNCTION_INFO_V1(tgeompoint_typmod_in);

PGDLLEXPORT Datum 
tgeompoint_typmod_in(PG_FUNCTION_ARGS)
{
	ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
	uint32 typmod = tpoint_typmod_in(array, false); /* Not a geography  */;
	PG_RETURN_INT32(typmod);
}

/*
 * typmod input for tgeogpoint
 */
PG_FUNCTION_INFO_V1(tgeogpoint_typmod_in);

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

/*
 * typmod input for tgeompoint and tgeogpoint
 */
PG_FUNCTION_INFO_V1(tpoint_typmod_out);

PGDLLEXPORT Datum 
tpoint_typmod_out(PG_FUNCTION_ARGS)
{
	char *s = (char *)palloc(64);
	char *str = s;
	int32 typmod = PG_GETARG_INT32(0);
	int32 duration_type = TYPMOD_GET_DURATION(typmod);
	TYPMOD_DEL_DURATION(typmod);
	int32 srid = TYPMOD_GET_SRID(typmod);
	int32 geometry_type = TYPMOD_GET_TYPE(typmod);
	int32 hasz = TYPMOD_GET_Z(typmod);

	/* No duration type or geometry type? Then no typmod at all. 
	  Return empty string. */
	if (typmod < 0 || (!duration_type && !geometry_type))
	{
		*str = '\0';
		PG_RETURN_CSTRING(str);
	}
	/* Opening bracket.  */
	str += sprintf(str, "(");
	/* Has duration type?  */
	if (duration_type)
		str += sprintf(str, "%s", temporal_type_name(duration_type));
	if (geometry_type)
	{
		if (duration_type) str += sprintf(str, ", ");
		str += sprintf(str, "%s", lwtype_name(geometry_type));
		/* Has Z?  */
		if (hasz) str += sprintf(str, "Z");
		/* Has SRID?  */
		if (srid) str += sprintf(str, ",%d", srid);
	}
	/* Closing bracket.  */
	str += sprintf(str, ")");

	PG_RETURN_CSTRING(s);
}

/*
 * Ensure that an incoming geometry conforms to typmod restrictions on
 * type, dims and srid.
 */
PG_FUNCTION_INFO_V1(tpoint_enforce_typmod);
PGDLLEXPORT Datum tpoint_enforce_typmod(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int32 typmod = PG_GETARG_INT32(1);
	/* Check if geometry typmod is consistent with the supplied one.  */
	temp = tpoint_valid_typmod(temp, typmod);
	PG_RETURN_POINTER(temp);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/* Construct a temporal instant point from two arguments */

PG_FUNCTION_INFO_V1(tpoint_make_temporalinst);
 
PGDLLEXPORT Datum
tpoint_make_temporalinst(PG_FUNCTION_ARGS) 
{
	GSERIALIZED *value = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_get_type(value) != POINTTYPE ||
		gserialized_is_empty(value))
	{
		PG_FREE_IF_COPY(value, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Only non empty point geometries accepted")));		
	}

	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Temporal *result = (Temporal *)temporalinst_make(PointerGetDatum(value),
		t, valuetypid);
	PG_FREE_IF_COPY(value, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* Get the precomputed bounding box of a Temporal */

PG_FUNCTION_INFO_V1(tpoint_gbox);

PGDLLEXPORT Datum
tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *result = palloc0(sizeof(GBOX));
	temporal_bbox(result, temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/* Is the temporal value ever equal to the value? */

PG_FUNCTION_INFO_V1(tpoint_ever_equals);

PGDLLEXPORT Datum
tpoint_ever_equals(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *value = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_get_type(value) != POINTTYPE) 
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Only point geometries accepted")));
	}
	if (gserialized_get_srid(value) != tpoint_srid_internal(temp)) 
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(value->flags) != MOBDB_FLAGS_GET_Z(temp->flags)) 
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}
	
	/* Bounding box test */
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, value))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		PG_RETURN_BOOL(false);
	}
	temporal_bbox(&box1, temp);
	if (!contains_gbox_gbox_internal(&box1, &box2))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		PG_RETURN_BOOL(false);
	}

	bool result;
	if (temp->type == TEMPORALINST) 
		result = temporalinst_ever_equals((TemporalInst *)temp, 
			PointerGetDatum(value));
	else if (temp->type == TEMPORALI) 
		result = temporali_ever_equals((TemporalI *)temp, 
			PointerGetDatum(value));
	else if (temp->type == TEMPORALSEQ) 
		result = temporalseq_ever_equals((TemporalSeq *)temp, 
			PointerGetDatum(value));
	else if (temp->type == TEMPORALS) 
		result = temporals_ever_equals((TemporalS *)temp, 
			PointerGetDatum(value));
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(value, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tpoint_always_equals);

PGDLLEXPORT Datum
tpoint_always_equals(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *value = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_get_type(value) != POINTTYPE) 
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Only point geometries accepted")));
	}
	if (gserialized_get_srid(value) != tpoint_srid_internal(temp)) 
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(value->flags) != MOBDB_FLAGS_GET_Z(temp->flags)) 
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}
	
	bool result;
	if (temp->type == TEMPORALINST) 
		result = temporalinst_always_equals((TemporalInst *)temp, 
			PointerGetDatum(value));
	else if (temp->type == TEMPORALI) 
		result = temporali_always_equals((TemporalI *)temp, 
			PointerGetDatum(value));
	else if (temp->type == TEMPORALSEQ) 
		result = temporalseq_always_equals((TemporalSeq *)temp, 
			PointerGetDatum(value));
	else if (temp->type == TEMPORALS) 
		result = temporals_always_equals((TemporalS *)temp, 
			PointerGetDatum(value));
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(value, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Assemble the set of points of a temporal instant point as a single 
 * geometry/geography. Duplicate points are removed.
 *****************************************************************************/

Datum
tgeompointi_values(TemporalI *ti)
{
	if (ti->count == 1)
		return temporalinst_value_copy(temporali_inst_n(ti, 0));

	Datum *values = palloc(sizeof(Datum *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		Datum value = temporalinst_value(temporali_inst_n(ti, i));
		bool found = false;
		for (int j = 0; j < k; j++)
		{
			if (datum_eq(value, values[j], ti->valuetypid))
			{
				found = true;
				break;
			}
		}
		if (!found)
			values[k++] = value;
	}
	if (k == 1)
	{
		pfree(values);
		return temporalinst_value_copy(temporali_inst_n(ti, 0));
	}

	ArrayType *array = datumarr_to_array(values, k, type_oid(T_GEOMETRY));
	Datum result = call_function1(LWGEOM_collect_garray, PointerGetDatum(array));
	pfree(values); pfree(array);
	return result;
}

Datum
tgeogpointi_values(TemporalI *ti)
{
	if (ti->count == 1)
		return temporalinst_value_copy(temporali_inst_n(ti, 0));

	Datum *values = palloc(sizeof(Datum *) * ti->count);
	int k = 0;
	Oid geomoid = type_oid(T_GEOMETRY);
	for (int i = 0; i < ti->count; i++)
	{
		Datum value = temporalinst_value(temporali_inst_n(ti, i));
		Datum geomvalue = call_function1(geometry_from_geography, value);
		bool found = false;
		for (int j = 0; j < k; j++)
		{
			if (datum_eq(geomvalue, values[j], geomoid))
			{
				found = true;
				break;
			}
		}
		if (!found)
			values[k++] = geomvalue;
		else
			pfree(DatumGetPointer(geomvalue));	
	}
	if (k == 1)
	{
		pfree(DatumGetPointer(values[0])); pfree(values);
		return temporalinst_value_copy(temporali_inst_n(ti, 0));
	}

	ArrayType *array = datumarr_to_array(values, k, type_oid(T_GEOMETRY));
	Datum geomresult = call_function1(LWGEOM_collect_garray, PointerGetDatum(array));
	Datum result = call_function1(geography_from_geometry, geomresult);
	for (int i = 0; i < k; i++)
		pfree(DatumGetPointer(values[i]));
	pfree(values); pfree(array); pfree(DatumGetPointer(geomresult));
	return result;
}

Datum
tpointi_values(TemporalI *ti)
{
	if (ti->valuetypid == type_oid(T_GEOMETRY))
		return tgeompointi_values(ti);
	else if (ti->valuetypid == type_oid(T_GEOGRAPHY))
		return tgeogpointi_values(ti);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
}

Datum
tpoint_values_internal(Temporal *temp)
{
	if (temp->type == TEMPORALINST) 
		return temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->type == TEMPORALI) 
		return tpointi_values((TemporalI *)temp);
	else if (temp->type == TEMPORALSEQ)
		return tpointseq_trajectory_copy((TemporalSeq *)temp);
	else if (temp->type == TEMPORALS)
		return tpoints_trajectory((TemporalS *)temp);	
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
}

PG_FUNCTION_INFO_V1(tpoint_values);

PGDLLEXPORT Datum
tpoint_values(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = tpoint_values_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

 /* Restriction to the value */

PG_FUNCTION_INFO_V1(tpoint_at_value);

PGDLLEXPORT Datum
tpoint_at_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *value = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_get_type(value) != POINTTYPE) 
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Only point geometries accepted")));
	}
	if (gserialized_get_srid(value) != tpoint_srid_internal(temp)) 
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(value->flags) != MOBDB_FLAGS_GET_Z(temp->flags)) 
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	/* Bounding box test */
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, value))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		PG_RETURN_NULL();
	}
	temporal_bbox(&box1, temp);
	if (!contains_gbox_gbox_internal(&box1, &box2))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		PG_RETURN_NULL();
	}

	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)temporalinst_at_value(
			(TemporalInst *)temp, PointerGetDatum(value));
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)temporali_at_value(
			(TemporalI *)temp, PointerGetDatum(value));
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_at_value(
			(TemporalSeq *)temp, PointerGetDatum(value));
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)temporals_at_value(
			(TemporalS *)temp, PointerGetDatum(value));
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(value, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/* Restriction to the complement of a value */

PG_FUNCTION_INFO_V1(tpoint_minus_value);

PGDLLEXPORT Datum
tpoint_minus_value(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *value = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_get_type(value) != POINTTYPE) 
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Only point geometries accepted")));
	}
	if (tpoint_srid_internal(temp) != gserialized_get_srid(value))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(value->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}
	
	/* Bounding box test */
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, value))
	{
		Temporal *result;
		if (temp->type == TEMPORALSEQ)
			result = (Temporal *)temporals_from_temporalseqarr(
				(TemporalSeq **)&temp, 1, false);
		else
			result = temporal_copy(temp);
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		PG_RETURN_POINTER(result);
	}
	temporal_bbox(&box1, temp);
	if (!contains_gbox_gbox_internal(&box1, &box2))
	{
		Temporal *result;
		if (temp->type == TEMPORALSEQ)
			result = (Temporal *)temporals_from_temporalseqarr(
				(TemporalSeq **)&temp, 1, false);
		else
			result = temporal_copy(temp);
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(value, 1);
		PG_RETURN_POINTER(result);
	}

	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)temporalinst_minus_value(
			(TemporalInst *)temp, PointerGetDatum(value));
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)temporali_minus_value(
			(TemporalI *)temp, PointerGetDatum(value));
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_minus_value(
			(TemporalSeq *)temp, PointerGetDatum(value));
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)temporals_minus_value(
			(TemporalS *)temp, PointerGetDatum(value));
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(value, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/* Restriction to the values */

PG_FUNCTION_INFO_V1(tpoint_at_values);

PGDLLEXPORT Datum
tpoint_at_values(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
	Oid valuetypid = temp->valuetypid;
	int count;
	Datum *values = datumarr_extract(array, &count);
	for (int i = 0; i < count; i++)
	{
		GSERIALIZED *gsvalue = (GSERIALIZED *) DatumGetPointer(values[i]);
		if (gserialized_get_type(gsvalue) != POINTTYPE) 
		{
			PG_FREE_IF_COPY(temp, 0);
			PG_FREE_IF_COPY(array, 1);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Only point geometries accepted")));
		}
		if (tpoint_srid_internal(temp) != gserialized_get_srid(gsvalue))
		{
			PG_FREE_IF_COPY(temp, 0);
			PG_FREE_IF_COPY(array, 1);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("The geometries must be in the same SRID")));
		}
		if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gsvalue->flags))
		{
			PG_FREE_IF_COPY(temp, 0);
			PG_FREE_IF_COPY(array, 1);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("The geometries must be of the same dimensionality")));
		}
	}
	
	datum_sort(values, count, valuetypid);
	int count1 = datum_remove_duplicates(values, count, valuetypid);
	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)temporalinst_at_values(
			(TemporalInst *)temp, values, count1);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)temporali_at_values(
			(TemporalI *)temp, values, count1);
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_at_values(
			(TemporalSeq *)temp, values, count1);
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)temporals_at_values(
			(TemporalS *)temp, values, count1);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	pfree(values);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(array, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/************************************************************************/

/* Restriction to the complement of values */

PG_FUNCTION_INFO_V1(tpoint_minus_values);

PGDLLEXPORT Datum
tpoint_minus_values(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
	Oid valuetypid = temp->valuetypid;
	int count;
	Datum *values = datumarr_extract(array, &count);
	for (int i = 0; i < count; i++)
	{
		GSERIALIZED *gsvalue = (GSERIALIZED *) DatumGetPointer(values[i]);
		if (gserialized_get_type(gsvalue) != POINTTYPE) 
		{
			PG_FREE_IF_COPY(temp, 0);
			PG_FREE_IF_COPY(array, 1);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Only point geometries accepted")));
		}
		if (tpoint_srid_internal(temp) != gserialized_get_srid(gsvalue))
		{
			PG_FREE_IF_COPY(temp, 0);
			PG_FREE_IF_COPY(array, 1);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("The geometries must be in the same SRID")));
		}
		if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gsvalue->flags))
		{
			PG_FREE_IF_COPY(temp, 0);
			PG_FREE_IF_COPY(array, 1);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("The geometries must be of the same dimensionality")));
		}
	}
	
	datum_sort(values, count, valuetypid);
	int count1 = datum_remove_duplicates(values, count, valuetypid);
	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)temporalinst_minus_values(
			(TemporalInst *)temp, values, count1);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)temporali_minus_values(
			(TemporalI *)temp, values, count1);
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)temporalseq_minus_values(
			(TemporalSeq *)temp, values, count1);
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)temporals_minus_values(
			(TemporalS *)temp, values, count1);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	pfree(values);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(array, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
