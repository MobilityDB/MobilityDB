/*****************************************************************************
 *
 * TemporalGeo.c
 *	  Geospatial functions for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/* 
 * Output a geometry in Well-Known Text (WKT) format.
 * The Oid argument is not used but is needed since the second argument of 
 * the functions temporal*_to_string is of type char *(*value_out)(Oid, Datum) 
 */
static char *
wkt_out(Oid type, Datum value)
{
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
	LWGEOM *geom = lwgeom_from_gserialized(gs);
	size_t len;
	char *wkt = lwgeom_to_wkt(geom, WKT_ISO, DBL_DIG, &len);
	char *result = palloc(len);
	strcpy(result, wkt);
	lwgeom_free(geom);
	pfree(wkt);
	return result;
}

/* 
 * Output a geometry in Well-Known Text (WKT) format prefixed with the SRID.
 * The Oid argument is not used but is needed since the second argument of 
 * the functions temporal*_to_string is of type char *(*value_out)(Oid, Datum) 
 */
static char *
ewkt_out(Oid type, Datum value)
{
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
	LWGEOM *geom = lwgeom_from_gserialized(gs);
	size_t len;
	char *wkt = lwgeom_to_wkt(geom, WKT_EXTENDED, DBL_DIG, &len);
	char *result = palloc(len);
	strcpy(result, wkt);
	lwgeom_free(geom);
	pfree(wkt);
	return result;
}

/* Get 2D point from a serialized geometry */

POINT2D
gs_get_point2d(GSERIALIZED *gs)
{
	POINT2D *point = (POINT2D *)((uint8_t*)gs->data + 8);
	return *point;
}

/* Get 2D point from a datum
 * The function supposes that the GSERIALIZED has been already detoasted.
 * This is typically the case when the datum is within a Temporal* that 
 * has been already detoasted with PG_GETARG_TEMPORAL*  */
POINT2D
datum_get_point2d(Datum geom)
{
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(geom);
	POINT2D *point = (POINT2D *)((uint8_t*)gs->data + 8);
	return *point;
}

/* Get 3D point from a serialized geometry */

POINT3DZ
gs_get_point3dz(GSERIALIZED *gs)
{
	POINT3DZ *point = (POINT3DZ *)((uint8_t*)gs->data + 8);
	return *point;
}

/* Get 3D point from a datum 
 * The function supposes that the GSERIALIZED has been already detoasted.
 * This is typically the case when the datum is within a Temporal* that 
 * has been already detoasted with PG_GETARG_TEMPORAL* */

POINT3DZ
datum_get_point3dz(Datum geom)
{
	GSERIALIZED *gs = (GSERIALIZED *)PointerGetDatum(geom);
	POINT3DZ *point = (POINT3DZ *)((uint8_t*)gs->data + 8);
	return *point;
}

/* Serialize a geometry */
 
GSERIALIZED *
geometry_serialize(LWGEOM *geom)
{
	size_t size;
	GSERIALIZED *result = gserialized_from_lwgeom(geom, &size);
	SET_VARSIZE(result, size);
	return result;
}

/*****************************************************************************
 * Functions for output in WKT and EWKT format 
 *****************************************************************************/

/* Output a temporal point in WKT format */

static text *
tpoint_astext_internal(Temporal *temp)
{
	char *str = NULL;
	if (temp->type == TEMPORALINST) 
		str = temporalinst_to_string((TemporalInst *)temp, &wkt_out);
	else if (temp->type == TEMPORALI) 
		str = temporali_to_string((TemporalI *)temp, &wkt_out);
	else if (temp->type == TEMPORALSEQ) 
		str = temporalseq_to_string((TemporalSeq *)temp, &wkt_out);
	else if (temp->type == TEMPORALS) 
		str = temporals_to_string((TemporalS *)temp, &wkt_out);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	text *result = cstring_to_text(str);
	pfree(str);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_astext);

PGDLLEXPORT Datum
tpoint_astext(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	text *result = tpoint_astext_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TEXT_P(result);
}

/* Output a temporal point in WKT format */

static text *
tpoint_asewkt_internal(Temporal *temp)
{
	int srid = tpoint_srid_internal(temp);
	char str1[20];
	if (srid > 0)
		sprintf(str1, "SRID=%d;", srid);
	else
		str1[0] = '\0';
	char *str2 = NULL;
	if (temp->type == TEMPORALINST) 
		str2 = temporalinst_to_string((TemporalInst *)temp, &wkt_out);
	else if (temp->type == TEMPORALI) 
		str2 = temporali_to_string((TemporalI *)temp, &wkt_out);
	else if (temp->type == TEMPORALSEQ) 
		str2 = temporalseq_to_string((TemporalSeq *)temp, &wkt_out);
	else if (temp->type == TEMPORALS) 
		str2 = temporals_to_string((TemporalS *)temp, &wkt_out);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	char *str = (char *)palloc(strlen(str1) + strlen(str2) + 1);
	strcpy(str, str1);
	strcat(str, str2);
	text *result = cstring_to_text(str);
	pfree(str2); pfree(str);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_asewkt);

PGDLLEXPORT Datum
tpoint_asewkt(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	text *result = tpoint_asewkt_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

/* Output a geometry/geography array in WKT format */

PG_FUNCTION_INFO_V1(geoarr_astext);

PGDLLEXPORT Datum
geoarr_astext(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count;
	Datum *geoarr = datumarr_extract(array, &count);
	text **textarr = palloc(sizeof(text *) * count);
	for (int i = 0; i < count; i++)
	{
		/* The wkt_out function does not use the first argument */
		char *str = wkt_out(ANYOID, geoarr[i]);
		textarr[i] = cstring_to_text(str);
		pfree(str);
	}
	ArrayType *result = textarr_to_array(textarr, count);

	pfree(geoarr);
	for (int i = 0; i < count; i++)
		pfree(textarr[i]);
	pfree(textarr);
	PG_FREE_IF_COPY(array, 0);

	PG_RETURN_ARRAYTYPE_P(result);
}

/* Output a geometry/geography array in WKT format prefixed with the SRID */

PG_FUNCTION_INFO_V1(geoarr_asewkt);

PGDLLEXPORT Datum
geoarr_asewkt(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count;
	Datum *geoarr = datumarr_extract(array, &count);
	text **textarr = palloc(sizeof(text *) * count);
	for (int i = 0; i < count; i++)
	{
		/* The wkt_out function does not use the first argument */
		char *str = ewkt_out(ANYOID, geoarr[i]);
		textarr[i] = cstring_to_text(str);
		pfree(str);
	}
	ArrayType *result = textarr_to_array(textarr, count);

	pfree(geoarr);
	for (int i = 0; i < count; i++)
		pfree(textarr[i]);
	pfree(textarr);
	PG_FREE_IF_COPY(array, 0);

	PG_RETURN_ARRAYTYPE_P(result);
}

/* Output a temporal point array in WKT format */

PG_FUNCTION_INFO_V1(tpointarr_astext);

PGDLLEXPORT Datum
tpointarr_astext(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count;
	Temporal **temparr = temporalarr_extract(array, &count);
	text **textarr = palloc(sizeof(text *) * count);
	for (int i = 0; i < count; i++)
		textarr[i] = tpoint_astext_internal(temparr[i]);
	ArrayType *result = textarr_to_array(textarr, count);

	pfree(temparr);
	for (int i = 0; i < count; i++)
		pfree(textarr[i]);
	pfree(textarr);
	PG_FREE_IF_COPY(array, 0);

	PG_RETURN_ARRAYTYPE_P(result);
}

/* Output a temporal point array in WKT format prefixed with the SRID */

PG_FUNCTION_INFO_V1(tpointarr_asewkt);

PGDLLEXPORT Datum
tpointarr_asewkt(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count;
	Temporal **temparr = temporalarr_extract(array, &count);
	text **textarr = palloc(sizeof(text *) * count);
	for (int i = 0; i < count; i++)
		textarr[i] = tpoint_asewkt_internal(temparr[i]);
	ArrayType *result = textarr_to_array(textarr, count);

	pfree(temparr);
	for (int i = 0; i < count; i++)
		pfree(textarr[i]);
	pfree(textarr);
	PG_FREE_IF_COPY(array, 0);

	PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

/* Get the spatial reference system identifier (SRID) of a temporal point */

static int
tpointinst_srid_internal(TemporalInst *inst)
{
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value(inst));
	return gserialized_get_srid(gs);
}

static int
tpointi_srid_internal(TemporalI *ti)
{
	TemporalInst *inst = temporali_inst_n(ti, 0);
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value(inst));
	return gserialized_get_srid(gs);
}

static int
tpointseq_srid_internal(TemporalSeq *seq)
{
	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value(inst));
	return gserialized_get_srid(gs);
}

static int
tpoints_srid_internal(TemporalS *ts)
{
	TemporalSeq *seq = temporals_seq_n(ts, 0);
	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value(inst));
	return gserialized_get_srid(gs);
}

int
tpoint_srid_internal(Temporal *temp)
{
	int result;
	if (temp->type == TEMPORALINST) 
		result = tpointinst_srid_internal((TemporalInst *)temp);
	else if (temp->type == TEMPORALI) 
		result = tpointi_srid_internal((TemporalI *)temp);
	else if (temp->type == TEMPORALSEQ) 
		result = tpointseq_srid_internal((TemporalSeq *)temp);
	else if (temp->type == TEMPORALS) 
		result = tpoints_srid_internal((TemporalS *)temp);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_srid);

PGDLLEXPORT Datum
tpoint_srid(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int result = tpoint_srid_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_INT32(result);
}

/*****************************************************************************/

/* Set the spatial reference system identifier (SRID) of a temporal point */

/* TemporalInst */

static TemporalInst *
tpointinst_set_srid_internal(TemporalInst *inst, int32 srid)
{
	TemporalInst *result = temporalinst_copy(inst); 
	GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value(result));
	gserialized_set_srid(gs, srid);
	return result;
}

static TemporalI *
tpointi_set_srid_internal(TemporalI *ti, int32 srid)
{
	TemporalI *result = temporali_copy(ti);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(result, i);
		GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value(inst));
		gserialized_set_srid(gs, srid);
	}
	return result;
}

static TemporalSeq *
tpointseq_set_srid_internal(TemporalSeq *seq, int32 srid)
{
	TemporalSeq *result = temporalseq_copy(seq);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(result, i);
		GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value(inst));
		gserialized_set_srid(gs, srid);
	}
	return result;
}

static TemporalS *
tpoints_set_srid_internal(TemporalS *ts, int32 srid)
{
	TemporalS *result = temporals_copy(ts);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(result, i);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(temporalinst_value(inst));
			gserialized_set_srid(gs, srid);
		}
	}
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_set_srid);

PGDLLEXPORT Datum
tpoint_set_srid(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	int32 srid = PG_GETARG_INT32(1);
	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)tpointinst_set_srid_internal((TemporalInst *)temp, srid);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)tpointi_set_srid_internal((TemporalI *)temp, srid);
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)tpointseq_set_srid_internal((TemporalSeq *)temp, srid);
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)tpoints_set_srid_internal((TemporalS *)temp, srid);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/* Transform a temporal geometry point into another spatial reference system */

TemporalInst *
tgeompointinst_transform(TemporalInst *inst, Datum srid)
{
	Datum geom = call_function2(transform, temporalinst_value(inst), srid);
	TemporalInst *result = temporalinst_make(geom, inst->t, 
		type_oid(T_GEOMETRY));
	pfree(DatumGetPointer(geom)); 
	return result;
}

static TemporalI *
tgeompointi_transform_internal(TemporalI *ti, Datum srid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = tgeompointinst_transform(inst, srid);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);

	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);

	return result;
}

static TemporalSeq *
tgeompointseq_transform_internal(TemporalSeq *seq, Datum srid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tgeompointinst_transform(inst, srid);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, true);

	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);

	return result;
}

static TemporalS *
tgeompoints_transform_internal(TemporalS *ts, Datum srid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			instants[j] = tgeompointinst_transform(inst, srid);
		}
		sequences[i] = temporalseq_from_temporalinstarr(instants, 
			seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
		for (int j = 0; j < seq->count; j++)
			pfree(instants[j]);
		pfree(instants);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, 
		ts->count, false);

	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

PG_FUNCTION_INFO_V1(tpoint_transform);

PGDLLEXPORT Datum
tpoint_transform(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum srid = PG_GETARG_DATUM(1);
	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)tgeompointinst_transform((TemporalInst *)temp, srid);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)tgeompointi_transform_internal((TemporalI *)temp, srid);
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)tgeompointseq_transform_internal((TemporalSeq *)temp, srid);
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)tgeompoints_transform_internal((TemporalS *)temp, srid);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 * Notice that a geometry point and a geography point are of different size
 * since the geography point keeps a bounding box 
 *****************************************************************************/

/* Geometry to Geography */
 
static TemporalInst *
tgeompointinst_as_tgeogpointinst_internal(TemporalInst *inst)
{
	Datum geog = call_function1(geography_from_geometry, temporalinst_value(inst));
	return temporalinst_make(geog, inst->t, type_oid(T_GEOGRAPHY));
}

static TemporalI *
tgeompointi_as_tgeogpointi_internal(TemporalI *ti)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = tgeompointinst_as_tgeogpointinst_internal(inst);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);

	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);

	return result;
}

static TemporalSeq *
tgeompointseq_as_tgeogpointseq_internal(TemporalSeq *seq)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tgeompointinst_as_tgeogpointinst_internal(inst);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, false);
	
	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	
	return result;
}

static TemporalS *
tgeompoints_as_tgeogpoints_internal(TemporalS *ts)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tgeompointseq_as_tgeogpointseq_internal(seq);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, 
		ts->count, false);
	
	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

PG_FUNCTION_INFO_V1(tgeompoint_as_tgeogpoint);

PGDLLEXPORT Datum
tgeompoint_as_tgeogpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)tgeompointinst_as_tgeogpointinst_internal((TemporalInst *)temp);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)tgeompointi_as_tgeogpointi_internal((TemporalI *)temp);
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)tgeompointseq_as_tgeogpointseq_internal((TemporalSeq *)temp);
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)tgeompoints_as_tgeogpoints_internal((TemporalS *)temp);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/* Geography to Geometry */

TemporalInst *
tgeogpointinst_as_tgeompointinst(TemporalInst *inst)
{
	Datum geom = call_function1(geometry_from_geography, temporalinst_value(inst));
	return temporalinst_make(geom, inst->t, type_oid(T_GEOMETRY));
}

TemporalI *
tgeogpointi_as_tgeompointi(TemporalI *ti)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = tgeogpointinst_as_tgeompointinst(inst);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);

	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);

	return result;
}

TemporalSeq *
tgeogpointseq_as_tgeompointseq(TemporalSeq *seq)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tgeogpointinst_as_tgeompointinst(inst);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, false);
	
	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	
	return result;
}

TemporalS *
tgeogpoints_as_tgeompoints(TemporalS *ts)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tgeogpointseq_as_tgeompointseq(seq);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, 
		ts->count, false);
	
	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

PG_FUNCTION_INFO_V1(tgeogpoint_as_tgeompoint);

PGDLLEXPORT Datum
tgeogpoint_as_tgeompoint(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)tgeogpointinst_as_tgeompointinst((TemporalInst *)temp);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)tgeogpointi_as_tgeompointi((TemporalI *)temp);
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)tgeogpointseq_as_tgeompointseq((TemporalSeq *)temp);
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)tgeogpoints_as_tgeompoints((TemporalS *)temp);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Trajectory functions.
 *****************************************************************************/
 
/* Compute the trajectory of a couple of subsequent instants.
 * The functions are called during normalization for determining whether 
 * three subsequent points are collinear, for computing the temporal 
 * distance, the temporal spatial relationships, etc. */

/* Make a line from two geometry points */

Datum
geompoint_trajectory(Datum value1, Datum value2) 
{
	GSERIALIZED *gstart = (GSERIALIZED *)DatumGetPointer(value1);
	GSERIALIZED *gend = (GSERIALIZED *)DatumGetPointer(value2);
	LWGEOM *geoms[2];
	geoms[0] = lwgeom_from_gserialized(gstart);
	geoms[1] = lwgeom_from_gserialized(gend);
	LWGEOM *traj = (LWGEOM *)lwline_from_lwgeom_array(geoms[0]->srid, 2, geoms);
	GSERIALIZED *result = geometry_serialize(traj);
	lwgeom_free(geoms[0]); lwgeom_free(geoms[1]);
	lwgeom_free(traj);
	return PointerGetDatum(result);
}

/* Trajectory of two subsequent temporal geometry points */

Datum
tgeompointseq_trajectory1(TemporalInst *inst1, TemporalInst *inst2) 
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	if (datum_eq(value1, value2, inst1->valuetypid))
	{
		GSERIALIZED *gstart = (GSERIALIZED *)DatumGetPointer(value1);
		Datum result = PointerGetDatum(gserialized_copy(gstart)); 
		return result; 
	}
	return geompoint_trajectory(value1, value2);
}

/* Trajectory of two subsequent temporal geography points */

Datum
tgeogpointseq_trajectory1(TemporalInst *inst1, TemporalInst *inst2) 
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	if (datum_eq(value1, value2, inst1->valuetypid))
	{
		GSERIALIZED *gsvalue = (GSERIALIZED *)DatumGetPointer(value1);
		Datum result = PointerGetDatum(gserialized_copy(gsvalue)); 
		return result; 
	}

	Datum geom1 = call_function1(geometry_from_geography, value1);
	Datum geom2 = call_function1(geometry_from_geography, value2);
	GSERIALIZED *gsgeom1 = (GSERIALIZED *)PG_DETOAST_DATUM(geom1);
	GSERIALIZED *gsgeom2 = (GSERIALIZED *)PG_DETOAST_DATUM(geom2);
	LWGEOM *geoms[2];
	geoms[0] = lwgeom_from_gserialized(gsgeom1);
	geoms[1] = lwgeom_from_gserialized(gsgeom2);
	LWGEOM *traj = (LWGEOM *)lwline_from_lwgeom_array(geoms[0]->srid, 2, geoms);
	GSERIALIZED *resultgeom = geometry_serialize(traj);
	Datum result = call_function1(geography_from_geometry, PointerGetDatum(resultgeom));
	POSTGIS_FREE_IF_COPY_P(gsgeom1, DatumGetPointer(geom1));
	POSTGIS_FREE_IF_COPY_P(gsgeom2, DatumGetPointer(geom2));
	pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
	lwgeom_free(geoms[0]); lwgeom_free(geoms[1]); 
	lwgeom_free(traj); pfree(resultgeom); 
	return result;
}

/*****************************************************************************/

/* Compute the trajectory of an array of instants.
 * This function is called by the constructor of a temporal sequence and 
 * returns a single Datum which is a geometry */
static Datum 
tgeompointseq_make_trajectory(TemporalInst **instants, int count)
{
	Datum *points = palloc(sizeof(Datum) * count);
	TemporalInst *inst1 = instants[0];
	Datum value1 = temporalinst_value(inst1);
	points[0] = value1;
	Oid valuetypid = inst1->valuetypid;
	int k = 1;
	for (int i = 1; i < count; i++)
	{
		TemporalInst *inst2 = instants[i];
		Datum value2 = temporalinst_value(inst2);
		if (datum_ne(value1, value2, valuetypid))
			points[k++] = value2;
		inst1 = inst2;
		value1 = value2;
	}
	Datum result;
	if (k == 1)
		result = PointerGetDatum(gserialized_copy(
			(GSERIALIZED *)PG_DETOAST_DATUM(points[0])));
	else
	{
		ArrayType *array = datumarr_to_array(points, k, type_oid(T_GEOMETRY));
		result = call_function1(LWGEOM_makeline_garray, PointerGetDatum(array));
		pfree(array);
	}
	pfree(points);
	return result;	
}

static Datum
tgeogpointseq_make_trajectory(TemporalInst **instants, int count)
{
	TemporalInst **geominstants = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++)
		geominstants[i] = tgeogpointinst_as_tgeompointinst(instants[i]);
	Datum geomresult = tgeompointseq_make_trajectory(geominstants, count);
	Datum result = call_function1(geography_from_geometry, geomresult);
	for (int i = 0; i < count; i++)
		pfree(geominstants[i]);
	pfree(geominstants); pfree(DatumGetPointer(geomresult));
	return result;
}

Datum
tpointseq_make_trajectory(TemporalInst **instants, int count)
{
	Oid valuetypid = instants[0]->valuetypid;
	if (valuetypid == type_oid(T_GEOMETRY))
		return tgeompointseq_make_trajectory(instants, count);
	else if (valuetypid == type_oid(T_GEOGRAPHY))
		return tgeogpointseq_make_trajectory(instants, count);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
}

/* Get the precomputed trajectory of a tgeompointseq */

Datum 
tpointseq_trajectory(TemporalSeq *seq)
{
	size_t *offsets = temporalseq_offsets_ptr(seq);
	void *traj = temporalseq_data_ptr(seq) + offsets[seq->count+1];
	return PointerGetDatum(traj);
}

/* Copy the precomputed trajectory of a tgeompointseq */

Datum 
tpointseq_trajectory_copy(TemporalSeq *seq)
{
	size_t *offsets = temporalseq_offsets_ptr(seq);
	void *traj = temporalseq_data_ptr(seq) + offsets[seq->count+1];
	return PointerGetDatum(gserialized_copy(traj));
}

/*****************************************************************************/

/* Compute the trajectory of a tgeompoints from the precomputed trajectories
   of its composing segments. The resulting trajectory must be freed by the
   calling function */

static Datum
tgeompoints_trajectory(TemporalS *ts)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tpointseq_trajectory_copy(temporals_seq_n(ts, 0)); 
	
	Datum *points = palloc(sizeof(Datum) * ts->count);
	Datum *segments = palloc(sizeof(Datum) * ts->count);
	int j = 0, k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		Datum traj = tpointseq_trajectory(temporals_seq_n(ts, i));
		GSERIALIZED *gstraj = (GSERIALIZED *)DatumGetPointer(traj);
		if (gserialized_get_type(gstraj) == POINTTYPE)
			points[j++] = traj;
		else
			segments[k++] = traj;
	}
	Datum multipoint = 0, multilinestring = 0; /* keep compiler quiet */
	if (j > 0)
	{
		if (j == 1)
			multipoint = PointerGetDatum(gserialized_copy(
				(GSERIALIZED *)(DatumGetPointer(points[0]))));
		else
		{
			ArrayType *array = datumarr_to_array(points, j, type_oid(T_GEOMETRY));
			multipoint = call_function1(pgis_union_geometry_array, 
				PointerGetDatum(array));
			pfree(array);
		}			
	}
	if (k > 0)
	{
		if (k == 1)
			multilinestring = PointerGetDatum(gserialized_copy(
				(GSERIALIZED *)(DatumGetPointer(segments[0]))));
		else
		{
			ArrayType *array = datumarr_to_array(segments, k, type_oid(T_GEOMETRY));
			Datum lines = call_function1(LWGEOM_collect_garray, 
				PointerGetDatum(array));
			multilinestring = call_function1(linemerge, lines);
			pfree(DatumGetPointer(lines));
			pfree(array);
		}			
	}
	Datum result;
 	if (j > 0 && k > 0)
	{
		result = call_function2(geomunion, multipoint, multilinestring);
		pfree(DatumGetPointer(multipoint)); pfree(DatumGetPointer(multilinestring));
	}
 	else if (j > 0)
 		result = multipoint;
	else 
		result = multilinestring;	

	pfree(points); pfree(segments);
	return result;
}

static Datum
tgeogpoints_trajectory(TemporalS *ts)
{
	TemporalS *tsgeom = tgeogpoints_as_tgeompoints(ts);
	Datum geomresult = tgeompoints_trajectory(tsgeom);
	Datum result = call_function1(geography_from_geometry, geomresult);
	pfree(tsgeom); pfree(DatumGetPointer(geomresult));
	return result;
}

Datum
tpoints_trajectory(TemporalS *ts) 
{
	if (ts->valuetypid == type_oid(T_GEOMETRY))
		return tgeompoints_trajectory(ts);
	else if (ts->valuetypid == type_oid(T_GEOGRAPHY))
		return tgeogpoints_trajectory(ts);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
}

PG_FUNCTION_INFO_V1(tgeompoint_trajectory);

PGDLLEXPORT Datum
tgeompoint_trajectory(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result;
	if (temp->type == TEMPORALINST) 
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->type == TEMPORALI) 
		result = tgeompointi_values((TemporalI *)temp);
	else if (temp->type == TEMPORALSEQ) 
		result = tpointseq_trajectory_copy((TemporalSeq *)temp);
	else if (temp->type == TEMPORALS)
		result = tgeompoints_trajectory((TemporalS *)temp);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(tgeogpoint_trajectory);

PGDLLEXPORT Datum
tgeogpoint_trajectory(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result;
	if (temp->type == TEMPORALINST) 
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->type == TEMPORALI) 
		result = tgeogpointi_values((TemporalI *)temp);
	else if (temp->type == TEMPORALSEQ)
		result = tpointseq_trajectory_copy((TemporalSeq *)temp);	
	else if (temp->type == TEMPORALS)
		result = tgeogpoints_trajectory((TemporalS *)temp);	
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Length functions
 *****************************************************************************/

/* Length traversed by the temporal point */

static double
tpointseq_length(TemporalSeq *seq)
{
	Datum traj = tpointseq_trajectory(seq);
	GSERIALIZED *gstraj = (GSERIALIZED *) PG_DETOAST_DATUM(traj);
	bool ispoint = (gserialized_get_type(gstraj) == POINTTYPE);
	POSTGIS_FREE_IF_COPY_P(gstraj, DatumGetPointer(traj));
	if (ispoint)
		return 0;
	
	/* We are sure that the trajectory is a line */
	double result;
	if (seq->valuetypid == type_oid(T_GEOMETRY))
		/* The next function call works for 2D and 3D */
		result = DatumGetFloat8(call_function1(LWGEOM_length_linestring, traj));
	else if (seq->valuetypid == type_oid(T_GEOGRAPHY))
		result = DatumGetFloat8(call_function1(geography_length, traj));
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	return result;
}

static double
tpoints_length(TemporalS *ts)
{
	double result = 0;
	for (int i = 0; i < ts->count; i++)
		result += tpointseq_length(temporals_seq_n(ts, i));
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_length);

PGDLLEXPORT Datum
tpoint_length(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	double result = 0.0; 
	if (temp->type == TEMPORALSEQ)
		result = tpointseq_length((TemporalSeq *)temp);	
	else if (temp->type == TEMPORALS)
		result = tpoints_length((TemporalS *)temp);	
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_FLOAT8(result);
}

/* Cumulative length traversed by the temporal point */

static TemporalSeq *
tpointseq_cumulative_length(TemporalSeq *seq, double prevlength)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = temporalinst_make(Float8GetDatum(0), inst->t,
			FLOAT8OID);
		TemporalSeq *result = temporalseq_from_temporalinstarr(&inst1, 1,
			true, true, false);
		pfree(inst1);
		return result;
	}

	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	Datum value1 = temporalinst_value(inst1);
	double length = prevlength;
	instants[0] = temporalinst_make(Float8GetDatum(length), inst1->t,
			FLOAT8OID);
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		Datum value2 = temporalinst_value(inst2);
		if (datum_ne(value1, value2, inst1->valuetypid))
		{
			Datum line = tgeompointseq_trajectory1(inst1, inst2);
			/* The next function works for 2D and 3D */
			length += DatumGetFloat8(call_function1(LWGEOM_length_linestring, line));	
			pfree(DatumGetPointer(line)); 
		}
		instants[i] = temporalinst_make(Float8GetDatum(length), inst2->t,
			FLOAT8OID);
		inst1 = inst2;
		value1 = value2;
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, false);
		
	for (int i = 1; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	
	return result;
}

static TemporalS *
tpoints_cumulative_length(TemporalS *ts)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	double length = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tpointseq_cumulative_length(seq, length);
		TemporalInst *end = temporalseq_inst_n(sequences[i], seq->count - 1);
		length += DatumGetFloat8(temporalinst_value(end));
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, 
		ts->count, false);
		
	for (int i = 1; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

PG_FUNCTION_INFO_V1(tpoint_cumulative_length);

PGDLLEXPORT Datum
tpoint_cumulative_length(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->type != TEMPORALSEQ && temp->type != TEMPORALS)
	{
		PG_FREE_IF_COPY(temp, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));
	}

	Temporal *result; 
	if (temp->type == TEMPORALSEQ)
		result = (Temporal *)tpointseq_cumulative_length((TemporalSeq *)temp, 0);	
	else
		result = (Temporal *)tpoints_cumulative_length((TemporalS *)temp);	
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

static TemporalSeq **
tpointseq_speed1(TemporalSeq *seq)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * (seq->count-1));
	TemporalInst *instants[2];
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	Datum value1 = temporalinst_value(inst1);
	double speed = 0; /* To make the compiler quiet */
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count-1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i+1);
		Datum value2 = temporalinst_value(inst2);
		if (datum_eq(value1, value2, seq->valuetypid))
			speed = 0;
		else
		{
			Datum traj = tgeompointseq_trajectory1(inst1, inst2);
			double length;
			if (seq->valuetypid == type_oid(T_GEOMETRY))
				/* The next function works for 2D and 3D */
				length = DatumGetFloat8(call_function1(LWGEOM_length_linestring, traj));
			else if (seq->valuetypid == type_oid(T_GEOGRAPHY))
				length = DatumGetFloat8(call_function1(geography_length, traj));
			else
				ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
					errmsg("Operation not supported")));
			
			pfree(DatumGetPointer(traj)); 
			speed = length / (((double)(inst2->t) - (double)(inst1->t))/ 1000000);
		}
		instants[0] = temporalinst_make(Float8GetDatum(speed),
			inst1->t, FLOAT8OID);
		instants[1] = temporalinst_make(Float8GetDatum(speed),
			inst2->t, FLOAT8OID);
		bool upper_inc = (i == seq->count-2) ? seq->period.upper_inc : false;
		result[i] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		inst1 = inst2;
		value1 = value2;
		lower_inc = true;
	}			
	return result;
}

static TemporalS *
tpointseq_speed(TemporalSeq *seq)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
		return NULL;
	
	TemporalSeq **sequences = tpointseq_speed1(seq);
	TemporalS *result = temporals_from_temporalseqarr(sequences, 
		seq->count-1, true);
	
	for (int i = 0; i < seq->count-1; i++)
		pfree(sequences[i]);
	pfree(sequences);
		
	return result;
}

static TemporalS *
tpoints_speed(TemporalS *ts)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		if (seq->count > 1)
		{
			sequences[i] = tpointseq_speed1(seq);
			/* The number of sequences in the result is always seq->count-1 */
			totalseqs += seq->count-1;
		}
	}
	if (totalseqs == 0)
	{
		pfree(sequences); 
		return NULL;
	}
	
	TemporalSeq **allseqs = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		for (int j = 0; j < seq->count - 1; j++)
			allseqs[k++] = sequences[i][j];
		if (seq->count > 1)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allseqs, 
		totalseqs, true);	

	pfree(sequences);
	for (int i = 0; i < totalseqs; i++)
		pfree(allseqs[i]);
	 pfree(allseqs); 

	return result;
}

PG_FUNCTION_INFO_V1(tpoint_speed);

PGDLLEXPORT Datum
tpoint_speed(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->type != TEMPORALSEQ && temp->type != TEMPORALS)
	{
		PG_FREE_IF_COPY(temp, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));
	}

	Temporal *result; 
	if (temp->type == TEMPORALSEQ)
		result = (Temporal *)tpointseq_speed((TemporalSeq *)temp);	
	else
		result = (Temporal *)tpoints_speed((TemporalS *)temp);	
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Time-weighed centroid for temporal geometry points
 *****************************************************************************/

static Datum
tgeompointi_centroid(TemporalI *ti)
{
	TemporalInst **instantsx = palloc(sizeof(TemporalInst *) * ti->count);
	TemporalInst **instantsy = palloc(sizeof(TemporalInst *) * ti->count);
	TemporalInst **instantsz = NULL ; /* keep compiler quiet */
	bool hasz = MOBDB_FLAGS_GET_Z(ti->flags);
	if (hasz)
		instantsz = palloc(sizeof(TemporalInst *) * ti->count);
		
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		POINT2D point2d;
		POINT3DZ point;
		if (hasz)
			point = datum_get_point3dz(temporalinst_value(inst));
		else
		{
			point2d = datum_get_point2d(temporalinst_value(inst));
			point.x = point2d.x;
			point.y = point2d.y;
		}
		
		instantsx[i] = temporalinst_make(Float8GetDatum(point.x), 
			inst->t, FLOAT8OID);		
		instantsy[i] = temporalinst_make(Float8GetDatum(point.y), 
			inst->t, FLOAT8OID);
		if (hasz)
			instantsz[i] = temporalinst_make(Float8GetDatum(point.z), 
				inst->t, FLOAT8OID);

	}
	TemporalI *tix = temporali_from_temporalinstarr(instantsx, ti->count);
	TemporalI *tiy = temporali_from_temporalinstarr(instantsy, ti->count);
	TemporalI *tiz = NULL; /* keep compiler quiet */
	if (hasz)
		tiz = temporali_from_temporalinstarr(instantsz, ti->count);
	double avgx = temporali_lavg(tix);
	double avgy = temporali_lavg(tiy);
	double avgz;
	if (hasz)
		avgz = temporali_lavg(tiz);
	Datum result;
	if (hasz)
		result = call_function3(LWGEOM_makepoint, Float8GetDatum(avgx), 
			Float8GetDatum(avgy), Float8GetDatum(avgz));
	else
		result = call_function2(LWGEOM_makepoint, Float8GetDatum(avgx), 
			Float8GetDatum(avgy));
		
	for (int i = 0; i < ti->count; i++)
	{
		pfree(instantsx[i]);
		pfree(instantsy[i]);
		if (hasz)
			pfree(instantsz[i]);
	}
	pfree(instantsx); pfree(instantsy);
	pfree(tix); pfree(tiy);
	if (hasz)
	{
		pfree(instantsz); pfree(tiz);
	}

	return result;
}

static Datum
tgeompointseq_twcentroid(TemporalSeq *seq)
{
	TemporalInst **instantsx = palloc(sizeof(TemporalInst *) * seq->count);
	TemporalInst **instantsy = palloc(sizeof(TemporalInst *) * seq->count);
	TemporalInst **instantsz;
	bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
	if (hasz)
		instantsz = palloc(sizeof(TemporalInst *) * seq->count);
		
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		POINT2D point2d;
		POINT3DZ point;
		if (hasz)
			point = datum_get_point3dz(temporalinst_value(inst));
		else
		{
			point2d = datum_get_point2d(temporalinst_value(inst));
			point.x = point2d.x;
			point.y = point2d.y;
		}
		instantsx[i] = temporalinst_make(Float8GetDatum(point.x), 
			inst->t, FLOAT8OID);		
		instantsy[i] = temporalinst_make(Float8GetDatum(point.y), 
			inst->t, FLOAT8OID);
		if (hasz)
			instantsz[i] = temporalinst_make(Float8GetDatum(point.z), 
				inst->t, FLOAT8OID);
	}
	TemporalSeq *seqx = temporalseq_from_temporalinstarr(instantsx, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
	TemporalSeq *seqy = temporalseq_from_temporalinstarr(instantsy, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
	TemporalSeq *seqz;
	if (hasz)
		seqz = temporalseq_from_temporalinstarr(instantsz, 
			seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
	double twavgx = tfloatseq_twavg(seqx);
	double twavgy = tfloatseq_twavg(seqy);
	double twavgz;
	if (hasz)
		twavgz = tfloatseq_twavg(seqz);
	Datum result;
	if (hasz)
		result = call_function3(LWGEOM_makepoint, Float8GetDatum(twavgx), 
			Float8GetDatum(twavgy), Float8GetDatum(twavgz));
	else
		result = call_function2(LWGEOM_makepoint, Float8GetDatum(twavgx), 
			Float8GetDatum(twavgy));
		
	for (int i = 0; i < seq->count; i++)
	{
		pfree(instantsx[i]);
		pfree(instantsy[i]);
		if (hasz)
			pfree(instantsz[i]);
	}
	pfree(instantsx); pfree(instantsy);
	pfree(seqx); pfree(seqy);
	if (hasz)
	{
		pfree(seqz); pfree(instantsz);
	}

	return result;
}

static Datum
tgeompoints_twcentroid(TemporalS *ts)
{
	TemporalSeq **sequencesx = palloc(sizeof(TemporalSeq *) * ts->count);
	TemporalSeq **sequencesy = palloc(sizeof(TemporalSeq *) * ts->count);
	TemporalSeq **sequencesz = NULL; /* keep compiler quiet */
	bool hasz = MOBDB_FLAGS_GET_Z(ts->flags);
	if (hasz)
		sequencesz = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalInst **instantsx = palloc(sizeof(TemporalInst *) * seq->count);
		TemporalInst **instantsy = palloc(sizeof(TemporalInst *) * seq->count);
		TemporalInst **instantsz;
		if (hasz)
			instantsz = palloc(sizeof(TemporalInst *) * seq->count);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			POINT2D point2d;
			POINT3DZ point;
			if (hasz)
				point = datum_get_point3dz(temporalinst_value(inst));
			else
			{
				point2d = datum_get_point2d(temporalinst_value(inst));
				point.x = point2d.x;
				point.y = point2d.y;
			}
			instantsx[j] = temporalinst_make(Float8GetDatum(point.x), 
				inst->t, FLOAT8OID);		
			instantsy[j] = temporalinst_make(Float8GetDatum(point.y), 
				inst->t, FLOAT8OID);
			if (hasz)
				instantsz[j] = temporalinst_make(Float8GetDatum(point.z), 
					inst->t, FLOAT8OID);
		}
		sequencesx[i] = temporalseq_from_temporalinstarr(instantsx,
			seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
		sequencesy[i] = temporalseq_from_temporalinstarr(instantsy,
			seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
		if (hasz)
			sequencesz[i] = temporalseq_from_temporalinstarr(instantsz,
				seq->count, seq->period.lower_inc, seq->period.upper_inc, true);

		for (int j = 0; j < seq->count; j++)
		{
			pfree(instantsx[j]); pfree(instantsy[j]);
			if (hasz)
				pfree(instantsz[j]);
		}
		pfree(instantsx); pfree(instantsy);
		if (hasz)
			pfree(instantsz);
	}
	TemporalS *tsx = temporals_from_temporalseqarr(sequencesx, 
		ts->count, true);
	TemporalS *tsy = temporals_from_temporalseqarr(sequencesy, 
		ts->count, true);
	TemporalS *tsz = NULL; /* keep compiler quiet */
	if (hasz)
		tsz = temporals_from_temporalseqarr(sequencesz, ts->count, true);

	double twavgx = tfloats_twavg(tsx);
	double twavgy = tfloats_twavg(tsy);
	double twavgz;
	if (hasz)
		twavgz = tfloats_twavg(tsz);
	Datum result;
	if (hasz)
		result = call_function3(LWGEOM_makepoint, Float8GetDatum(twavgx), 
			Float8GetDatum(twavgy), Float8GetDatum(twavgz));
	else
		result = call_function2(LWGEOM_makepoint, Float8GetDatum(twavgx), 
			Float8GetDatum(twavgy));
	
	for (int i = 0; i < ts->count; i++)
	{
		pfree(sequencesx[i]); pfree(sequencesy[i]);
		if (hasz)
			pfree(sequencesz[i]);
	}
	pfree(sequencesx); pfree(sequencesy);
	pfree(tsx); pfree(tsy);
	if (hasz)
	{
		pfree(tsz); pfree(sequencesz);
	}
	
	return result;
}

PG_FUNCTION_INFO_V1(tgeompoint_twcentroid);

PGDLLEXPORT Datum
tgeompoint_twcentroid(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result = 0; 
	if (temp->type == TEMPORALINST) 
		result = temporalinst_value_copy((TemporalInst *)temp);
	else if (temp->type == TEMPORALI) 
		result = tgeompointi_centroid((TemporalI *)temp);
	else if (temp->type == TEMPORALSEQ) 
		result = tgeompointseq_twcentroid((TemporalSeq *)temp);
	else if (temp->type == TEMPORALS) 
		result = tgeompoints_twcentroid((TemporalS *)temp);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}
	
/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

/* This function assumes that the instant values are not equal 
 * This should be ensured by the calling function */
static Datum
tpointseq_azimuth1(TemporalInst *inst1, TemporalInst *inst2)
{
	if (inst1->valuetypid == type_oid(T_GEOMETRY))
		return call_function2(LWGEOM_azimuth, temporalinst_value(inst1), 
			temporalinst_value(inst2));
	else if (inst1->valuetypid == type_oid(T_GEOGRAPHY))
		return call_function2(geography_azimuth, temporalinst_value(inst1), 
			temporalinst_value(inst2));
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
}

static int
tpointseq_azimuth2(TemporalSeq **result, TemporalSeq *seq)
{
	if (seq->count == 1)
		return 0;
	
	/* We are sure that there are at least 2 instants */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	Datum value1 = temporalinst_value(inst1);
	int k = 0, l = 0;
	Datum azimuth = 0; /* To make the compiler quiet */
	bool lower_inc = seq->period.lower_inc, upper_inc;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		Datum value2 = temporalinst_value(inst2);
		upper_inc = (i == seq->count-1) ? seq->period.upper_inc : false;
		if (datum_ne(value1, value2, seq->valuetypid))
		{
			azimuth = tpointseq_azimuth1(inst1, inst2);
			instants[k++] = temporalinst_make(azimuth,
				inst1->t, FLOAT8OID);
		}
		else 
		{
			if (k != 0) 
			{
				instants[k++] = temporalinst_make(azimuth, inst1->t, FLOAT8OID);
				upper_inc = true;
				result[l++] = temporalseq_from_temporalinstarr(instants, 
					k, lower_inc, upper_inc, true);
				for (int j = 0; j < k; j++)
					pfree(instants[j]);
				k = 0;
			}
			lower_inc = true;
		}
		inst1 = inst2;
		value1 = value2;
	}
	if (k != 0)
	{
		instants[k++] = temporalinst_make(azimuth, inst1->t, FLOAT8OID);
		result[l++] = temporalseq_from_temporalinstarr(instants, 
			k, lower_inc, upper_inc, true);
	}

	pfree(instants);

	return l;
}

TemporalS *
tpointseq_azimuth(TemporalSeq *seq)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int count = tpointseq_azimuth2(sequences, seq);
	if (count == 0)
	{
		pfree(sequences);
		return NULL;
	}
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

TemporalS *
tpoints_azimuth(TemporalS *ts)
{
	if (ts->count == 1)
		return tpointseq_azimuth(temporals_seq_n(ts, 0));

	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount);
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = tpointseq_azimuth2(&sequences[k], seq);
		k += countstep;
	}
	if (k == 0)
		return NULL;

	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_azimuth);

PGDLLEXPORT Datum
tpoint_azimuth(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->type != TEMPORALSEQ && temp->type != TEMPORALS)
	{
		PG_FREE_IF_COPY(temp, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));
	}

	Temporal *result; 
	if (temp->type == TEMPORALSEQ)
		result = (Temporal *)tpointseq_azimuth((TemporalSeq *)temp);	
	else
		result = (Temporal *)tpoints_azimuth((TemporalS *)temp);	
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 * N.B. In the current version of PostGIS (2.5) there is no true ST_Intersection 
 * function for geography
 *****************************************************************************/

/* Restrict a temporal point to a geometry */

static TemporalInst *
tpointinst_at_geometry(TemporalInst *inst, Datum geom)
{
	if (!DatumGetBool(call_function2(intersects, temporalinst_value(inst), geom)))
		return NULL;
	return temporalinst_copy(inst);
}

static TemporalI *
tpointi_at_geometry(TemporalI *ti, Datum geom)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		Datum value = temporalinst_value(inst);
		if (DatumGetBool(call_function2(intersects, value, geom)))
			instants[k++] = inst;
	}
	TemporalI *result = NULL;
	if (k != 0)
		result = temporali_from_temporalinstarr(instants, k);
	pfree(instants);
	return result;
}

/*
 * This function assumes that inst1 and inst2 have same SRID and that the
 * points and the geometry are in 2D 
 */
static TemporalSeq **
tpointseq_at_geometry1(TemporalInst *inst1, TemporalInst *inst2,
	bool lower_inc, bool upper_inc, Datum geom, int *count)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);

	/* Constant sequence */
	if (datum_eq(value1, value2, inst1->valuetypid))
	{
		if (!DatumGetBool(call_function2(intersects, value1, geom)))
		{
			*count = 0;
			return NULL;
		}

		TemporalInst *instants[2];
		instants[0] = inst1;
		instants[1] = inst2;
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, upper_inc, false);
		*count = 1;
		return result;
	}

	/* Look for intersections */
	Datum line = geompoint_trajectory(value1, value2);
	Datum intersections = call_function2(intersection, line, geom);
	if (DatumGetBool(call_function1(LWGEOM_isempty, intersections)))
	{
		pfree(DatumGetPointer(line));
		pfree(DatumGetPointer(intersections));
		*count = 0;
		return NULL;
	}

	int countinter = DatumGetInt32(call_function1(
		LWGEOM_numgeometries_collection, intersections));
	TemporalInst *instants[2];
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * countinter);
	double duration = (double)(inst2->t) - (double)(inst1->t);
	int k = 0;
	for (int i = 1; i <= countinter; i++)
	{
		/* Find the i-th intersection */
		Datum inter = call_function2(LWGEOM_geometryn_collection, 
			intersections, Int32GetDatum(i));
		GSERIALIZED *gsinter = (GSERIALIZED *) PG_DETOAST_DATUM(inter);

		/* Each intersection is either a point or a linestring with two points */
		if (gserialized_get_type(gsinter) == POINTTYPE)
		{
			double fraction = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, inter));
			TimestampTz t = (double)(inst1->t) + duration * fraction;
			/* If the intersection is not at an exclusive bound */
			if ((lower_inc || t > inst1->t) && (upper_inc || t < inst2->t))
			{
				/* Restriction at timestamp done to avoid floating point imprecision */
				instants[0] = temporalseq_at_timestamp1(inst1, inst2, t);
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false);
				pfree(instants[0]);
			}
		}
		else
		{
			Datum point1 = call_function2(LWGEOM_pointn_linestring, 
				inter, Int32GetDatum(1));
			Datum point2 = call_function2(LWGEOM_pointn_linestring, 
				inter, Int32GetDatum(2));
			double fraction1 = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, point1));
			double fraction2 = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, point2));
			TimestampTz t1 = (double)(inst1->t) + duration * fraction1;
			TimestampTz t2 = (double)(inst1->t) + duration * fraction2;					
			TimestampTz lower1 = Min(t1, t2);
			TimestampTz upper1 = Max(t1, t2);
			/* Restriction at timestamp done to avoid floating point imprecision */
			instants[0] = temporalseq_at_timestamp1(inst1, inst2, lower1);
			instants[1] = temporalseq_at_timestamp1(inst1, inst2, upper1);
			bool lower_inc1 = timestamp_cmp_internal(lower1, inst1->t) == 0 ? 
				lower_inc : true;
			bool upper_inc1 = timestamp_cmp_internal(upper1, inst2->t) == 0 ? 
				upper_inc : true;
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					lower_inc1, upper_inc1, false);
			pfree(DatumGetPointer(point1)); pfree(DatumGetPointer(point2));
		}
		POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(inter));
	}

	pfree(DatumGetPointer(line));
	pfree(DatumGetPointer(intersections));

	if (k == 0)
	{
		pfree(result);
		*count = 0;
		return NULL;
	}

	temporalseqarr_sort(result, k);
	*count = k;
	return result;
}

TemporalSeq **
tpointseq_at_geometry2(TemporalSeq *seq, Datum geom, int *count)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		Datum value = temporalinst_value(inst);
		if (!DatumGetBool(call_function2(intersects, value, geom)))
		{
			*count = 0;
			return NULL;
		}
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_from_temporalinstarr(&inst, 1, 
			true, true, false);
		*count = 1;
		return result;
	}

	/* Temporal sequence has at least 2 instants */
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * (seq->count - 1));
	int *countseqs = palloc0(sizeof(int) * (seq->count - 1));
	int totalseqs = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
		sequences[i] = tpointseq_at_geometry1(inst1, inst2, 
			lower_inc, upper_inc, geom, &countseqs[i]);
		totalseqs += countseqs[i];
		inst1 = inst2;
		lower_inc = true;
	}
	if (totalseqs == 0)
	{
		pfree(countseqs);
		pfree(sequences);
		*count = 0;
		return NULL;
	}

	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < seq->count - 1; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			result[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}

	pfree(countseqs);
	pfree(sequences);
	*count = totalseqs;
	return result;
}


static TemporalS *
tpointseq_at_geometry(TemporalSeq *seq, Datum geom)
{
	int count;
	TemporalSeq **sequences = tpointseq_at_geometry2(seq, geom, &count);
	if (sequences == NULL)
		return NULL;

	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

static TemporalS *
tpoints_at_geometry(TemporalS *ts, GSERIALIZED *gs, GBOX *box2)
{
	/* palloc0 used due to the bounding box test in the for loop below */
	TemporalSeq ***sequences = palloc0(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		/* Bounding box test */
		GBOX *box1 = temporalseq_bbox_ptr(seq);
		if (overlaps_gbox_gbox_internal(box1, box2))
		{
			sequences[i] = tpointseq_at_geometry2(seq, PointerGetDatum(gs), 
				&countseqs[i]);
			totalseqs += countseqs[i];
		}
	}
	if (totalseqs == 0)
	{
		pfree(sequences);
		pfree(countseqs);
		return NULL;
	}

	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences, totalseqs, true);

	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences);
	pfree(sequences);
	pfree(countseqs);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_at_geometry);

PGDLLEXPORT Datum
tpoint_at_geometry(PG_FUNCTION_ARGS)
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
	
	/* Bounding box test */
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	temporal_bbox(&box1, temp);
	if (!overlaps_gbox_gbox_internal(&box1, &box2))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}

	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)tpointinst_at_geometry((TemporalInst *)temp, 
			PointerGetDatum(gs));
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)tpointi_at_geometry((TemporalI *)temp, 
			PointerGetDatum(gs));
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)tpointseq_at_geometry((TemporalSeq *)temp,
			PointerGetDatum(gs));
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)tpoints_at_geometry((TemporalS *)temp, gs, &box2);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	if (result == NULL) 
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/* Restrict a temporal point to the complement of a geometry */

static TemporalInst *
tpointinst_minus_geometry(TemporalInst *inst, Datum geom)
{
	if (DatumGetBool(call_function2(intersects, temporalinst_value(inst), geom)))
		return NULL;
	return temporalinst_copy(inst);
}

static TemporalI *
tpointi_minus_geometry(TemporalI *ti, Datum geom)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		Datum value = temporalinst_value(inst);
		if (!DatumGetBool(call_function2(intersects, value, geom)))
			instants[k++] = inst;
	}
	TemporalI *result = NULL;
	if (k != 0)
		result = temporali_from_temporalinstarr(instants, k);
	pfree(instants);
	return result;
}

/* 
 * It is not possible to use a similar approach as for tpointseq_at_geometry1
 * where instead of computing the intersections we compute the difference since
 * in PostGIS the following query
 *  	select st_astext(st_difference(geometry 'Linestring(0 0,3 3)',
 *  		geometry 'MultiPoint((1 1),(2 2),(3 3))'))
 * returns "LINESTRING(0 0,3 3)". Therefore we compute tpointseq_at_geometry1
 * and then compute the complement of the value obtained.
 */
static TemporalSeq **
tpointseq_minus_geometry1(TemporalSeq *seq, Datum geom, int *count)
{
	int countinter;
	TemporalSeq **sequences = tpointseq_at_geometry2(seq, geom, &countinter);
	if (countinter == 0)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_copy(seq);
		*count = 1;
		return result;
	}
		
	Period **periods = palloc(sizeof(Period) * countinter);
	for (int i = 0; i < countinter; i++)
		periods[i] = &sequences[i]->period;
	PeriodSet *ps1 = periodset_from_periodarr_internal(periods, countinter, false);
	PeriodSet *ps2 = minus_period_periodset_internal(&seq->period, ps1);
	pfree(ps1); pfree(periods);
	if (ps2 == NULL)
	{
		*count = 0;
		return NULL;
	}
	TemporalSeq **result = temporalseq_at_periodset2(seq, ps2, count);
	pfree(ps2);
	return result;
}

static TemporalS *
tpointseq_minus_geometry(TemporalSeq *seq, Datum geom)
{
	int count;
	TemporalSeq **sequences = tpointseq_minus_geometry1(seq, geom, &count);
	if (sequences == NULL)
		return NULL;

	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

static TemporalS *
tpoints_minus_geometry(TemporalS *ts, GSERIALIZED *gs, GBOX *box2)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tpointseq_minus_geometry(temporals_seq_n(ts, 0), 
			PointerGetDatum(gs));

	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		/* Bounding box test */
		GBOX *box1 = temporalseq_bbox_ptr(seq);
		if (!overlaps_gbox_gbox_internal(box1, box2))
		{
			sequences[i] = palloc(sizeof(TemporalSeq *));
			sequences[i][0] = temporalseq_copy(seq);
			countseqs[i] = 1;
			totalseqs ++;
		}
		else
		{
			sequences[i] = tpointseq_minus_geometry1(seq, PointerGetDatum(gs), 
				&countseqs[i]);
			totalseqs += countseqs[i];
		}
	}
	if (totalseqs == 0)
	{
		pfree(sequences); pfree(countseqs);
		return NULL;
	}

	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (countseqs[i] != 0)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences, totalseqs, true);

	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); pfree(sequences); pfree(countseqs);

	return result;
}

PG_FUNCTION_INFO_V1(tpoint_minus_geometry);

PGDLLEXPORT Datum
tpoint_minus_geometry(PG_FUNCTION_ARGS)
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

	/* Bounding box test */
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		Temporal* copy = temporal_copy(temp) ;
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_POINTER(copy);
	}
	temporal_bbox(&box1, temp);
	if (!overlaps_gbox_gbox_internal(&box1, &box2))
	{
		Temporal* copy = temporal_copy(temp) ;
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_POINTER(copy);
	}

	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)tpointinst_minus_geometry((TemporalInst *)temp, 
			PointerGetDatum(gs));
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)tpointi_minus_geometry((TemporalI *)temp, 
			PointerGetDatum(gs));
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)tpointseq_minus_geometry((TemporalSeq *)temp, 
			PointerGetDatum(gs));
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)tpoints_minus_geometry((TemporalS *)temp, gs, &box2);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach instant
 *****************************************************************************/

static TemporalInst *
NAI_tpointi_geometry(TemporalI *ti, Datum geom, bool hasz)
{
	TemporalInst *inst = temporali_inst_n(ti, 0);
	double mindist = DBL_MAX;
	int number = 0; /* keep compiler quiet */ 
	for (int i = 0; i < ti->count; i++)
	{
		inst = temporali_inst_n(ti, i);
		Datum value = temporalinst_value(inst);
		double dist = hasz ?
			DatumGetFloat8(call_function2(distance3d, value, geom)) : 
			DatumGetFloat8(call_function2(distance, value, geom));	
		if (dist < mindist)
		{
			mindist = dist;
			number = i;
		}
	}
	return temporali_inst_n(ti, number);
}

/*****************************************************************************/

static TemporalInst *
NAI_tpointseq_geometry(TemporalSeq *seq, Datum geom, bool hasz)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
		return temporalinst_copy(temporalseq_inst_n(seq, 0));

	double mindist = DBL_MAX;
	Datum minpoint = 0; /* keep compiler quiet */
	TimestampTz t = 0; /* keep compiler quiet */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	for (int i = 0; i < seq->count-1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i+1);
		Datum traj = tgeompointseq_trajectory1(inst1, inst2);
		Datum point;
		double dist;
		if (hasz)
		{
			point = call_function2(LWGEOM_closestpoint3d, traj, geom);
			dist = DatumGetFloat8(call_function2(distance3d,
				point, geom));
		}
		else
		{
			point = call_function2(LWGEOM_closestpoint, traj, geom);
			dist = DatumGetFloat8(call_function2(distance,
				point, geom));
		}
		if (dist < mindist)
		{
			mindist = dist;
			minpoint = point;
			GSERIALIZED *gstraj = (GSERIALIZED *)DatumGetPointer(traj);
			if (gserialized_get_type(gstraj) == POINTTYPE)
				t = inst1->t;
			else
			{
				double fraction = DatumGetFloat8(call_function2(
					LWGEOM_line_locate_point, traj, minpoint));
				t = inst1->t + (inst2->t - inst1->t) * fraction;
			}
		}
		else
			pfree(DatumGetPointer(point)); 			
		inst1 = inst2;
		pfree(DatumGetPointer(traj)); 
	}
	TemporalInst *result = temporalinst_make(minpoint, t, 
		seq->valuetypid);
	pfree(DatumGetPointer(minpoint)); 
	return result;
}

/*****************************************************************************/

static TemporalInst *
NAI_tpoints_geometry(TemporalS *ts, Datum geom, bool hasz)
{
	Datum minpoint = 0; /* keep compiler quiet */
	TimestampTz t = 0; /* keep compiler quiet */
	double mindist = DBL_MAX;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalInst *inst = NAI_tpointseq_geometry(seq, geom, hasz);
		Datum value = temporalinst_value(inst);
		double dist = hasz ?
			DatumGetFloat8(call_function2(distance3d, value, geom)) :
			DatumGetFloat8(call_function2(distance, value, geom));
		if (dist < mindist)
		{
			if (mindist != DBL_MAX)
				pfree(DatumGetPointer(minpoint));
			mindist = dist;
			minpoint = temporalinst_value_copy(inst);
			t = inst->t;
		}
		pfree(inst);
	}
	TemporalInst *result = temporalinst_make(minpoint, t, 
		ts->valuetypid);
	return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(NAI_geometry_tpoint);

PGDLLEXPORT Datum
NAI_geometry_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hasz = MOBDB_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->flags); 

	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)NAI_tpointi_geometry((TemporalI *)temp, 
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)NAI_tpointseq_geometry((TemporalSeq *)temp, 
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)NAI_tpoints_geometry((TemporalS *)temp,
			PointerGetDatum(gs), hasz);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tpoint_geometry);

PGDLLEXPORT Datum
NAI_tpoint_geometry(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	bool hasz = MOBDB_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->flags); 

	Temporal *result;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)temporalinst_copy((TemporalInst *)temp);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)NAI_tpointi_geometry((TemporalI *)temp, 
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)NAI_tpointseq_geometry((TemporalSeq *)temp, 
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALS) 
		result = (Temporal *)NAI_tpoints_geometry((TemporalS *)temp,
			PointerGetDatum(gs), hasz);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tpoint_tpoint);

PGDLLEXPORT Datum
NAI_tpoint_tpoint(PG_FUNCTION_ARGS)
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

	Datum (*operator)(Datum, Datum);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp1->flags))
			operator = &geom_distance3d;
		else
			operator = &geom_distance2d;
	}
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_distance;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	TemporalInst *result = NULL;
	Temporal *dist = sync_oper2_temporal_temporal(temp1, temp2,
		operator, FLOAT8OID, &tpointseq_min_dist_at_timestamp);
	if (dist != NULL)
	{
		Temporal *mindist = temporal_at_min_internal(dist);
		TimestampTz t = temporal_start_timestamp_internal(mindist);
		result = temporal_at_timestamp_internal(temp1, t);
		pfree(dist); pfree(mindist);		
	}
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

static double
NAD_tpointinst_geometry(TemporalInst *inst, Datum geom, bool hasz)
{
	Datum value = temporalinst_value(inst);
	Datum result = hasz ?
		DatumGetFloat8(call_function2(distance3d, value, geom)) :
		DatumGetFloat8(call_function2(distance, value, geom));
	return result;
}

static double
NAD_tpointi_geometry(TemporalI *ti, Datum geom, bool hasz)
{
	Datum traj = tgeompointi_values(ti);
	double result = hasz ?
		DatumGetFloat8(call_function2(distance3d, traj, geom)) :
		DatumGetFloat8(call_function2(distance, traj, geom));
	pfree(DatumGetPointer(traj));
	return result;
}

static double
NAD_tpointseq_geometry(TemporalSeq *seq, Datum geom, bool hasz)
{
	Datum traj = tpointseq_trajectory(seq);
	double result = hasz ?
		DatumGetFloat8(call_function2(distance3d, traj, geom)) :
		DatumGetFloat8(call_function2(distance, traj, geom));
	return result;
}

static double
NAD_tpoints_geometry(TemporalS *ts, Datum geom, bool hasz)
{
	Datum traj = tgeompoints_trajectory(ts);
	double result = hasz ?
		DatumGetFloat8(call_function2(distance3d, traj, geom)) :
		DatumGetFloat8(call_function2(distance, traj, geom));
	pfree(DatumGetPointer(traj));
	return result;
}

PG_FUNCTION_INFO_V1(NAD_geometry_tpoint);

PGDLLEXPORT Datum
NAD_geometry_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hasz = MOBDB_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->flags); 

	Datum result;
	if (temp->type == TEMPORALINST) 
		result = NAD_tpointinst_geometry((TemporalInst *)temp,
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALI) 
		result = NAD_tpointi_geometry((TemporalI *)temp,
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALSEQ) 
		result = NAD_tpointseq_geometry((TemporalSeq *)temp,
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALS) 
		result = NAD_tpoints_geometry((TemporalS *)temp,
			PointerGetDatum(gs), hasz);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(NAD_tpoint_geometry);

PGDLLEXPORT Datum
NAD_tpoint_geometry(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	bool hasz = MOBDB_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->flags); 

	Datum result;
	if (temp->type == TEMPORALINST) 
		result = NAD_tpointinst_geometry((TemporalInst *)temp,
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALI) 
		result = NAD_tpointi_geometry((TemporalI *)temp,
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALSEQ) 
		result = NAD_tpointseq_geometry((TemporalSeq *)temp,
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALS) 
		result = NAD_tpoints_geometry((TemporalS *)temp,
			PointerGetDatum(gs), hasz);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(NAD_tpoint_tpoint);

PGDLLEXPORT Datum
NAD_tpoint_tpoint(PG_FUNCTION_ARGS)
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

	Datum (*operator)(Datum, Datum);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp1->flags) && MOBDB_FLAGS_GET_Z(temp2->flags))
			operator = &geom_distance3d;
		else
			operator = &geom_distance2d;
	}
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_distance;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	Temporal *dist = sync_oper2_temporal_temporal_crossdisc(temp1, temp2, 
		operator, FLOAT8OID);
	if (dist == NULL)
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	double result = DatumGetFloat8(temporal_min_value_internal(dist));
	pfree(dist);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

static Datum
shortestline_tpointinst_geometry(TemporalInst *inst, Datum geom, bool hasz)
{
	Datum value = temporalinst_value(inst);
	Datum result = hasz ?
		call_function2(LWGEOM_shortestline3d, value, geom) :
		call_function2(LWGEOM_shortestline2d, value, geom);
	return result;
}

static Datum
shortestline_tpointi_geometry(TemporalI *ti, Datum geom, bool hasz)
{
	Datum traj = tgeompointi_values(ti);
	Datum result = hasz ?
		call_function2(LWGEOM_shortestline3d, traj, geom) :
		call_function2(LWGEOM_shortestline2d, traj, geom);
	pfree(DatumGetPointer(traj));
	return result;
}

static Datum
shortestline_tpointseq_geometry(TemporalSeq *seq, Datum geom, bool hasz)
{
	Datum traj = tpointseq_trajectory(seq);
	Datum result = hasz ?
		call_function2(LWGEOM_shortestline3d, traj, geom) :
		call_function2(LWGEOM_shortestline2d, traj, geom);
	return result;
}

static Datum
shortestline_tpoints_geometry(TemporalS *ts, Datum geom, bool hasz)
{
	Datum traj = tpoints_trajectory(ts);
	Datum result = hasz ?
		call_function2(LWGEOM_shortestline3d, traj, geom) :
		call_function2(LWGEOM_shortestline2d, traj, geom);
	pfree(DatumGetPointer(traj));
	return result;
}

PG_FUNCTION_INFO_V1(shortestline_geometry_tpoint);

PGDLLEXPORT Datum
shortestline_geometry_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool hasz = MOBDB_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->flags); 
	
	Datum result;
	if (temp->type == TEMPORALINST) 
		result = shortestline_tpointinst_geometry((TemporalInst *)temp, 
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALI) 
		result = shortestline_tpointi_geometry((TemporalI *)temp, 
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALSEQ) 
		result = shortestline_tpointseq_geometry((TemporalSeq *)temp, 
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALS) 
		result = shortestline_tpoints_geometry((TemporalS *)temp, 
			PointerGetDatum(gs), hasz);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(shortestline_tpoint_geometry);

PGDLLEXPORT Datum
shortestline_tpoint_geometry(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	bool hasz = MOBDB_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->flags); 

	Datum result;
	if (temp->type == TEMPORALINST) 
		result = shortestline_tpointinst_geometry((TemporalInst *)temp, 
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALI) 
		result = shortestline_tpointi_geometry((TemporalI *)temp, 
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALSEQ) 
		result = shortestline_tpointseq_geometry((TemporalSeq *)temp, 
			PointerGetDatum(gs), hasz);
	else if (temp->type == TEMPORALS) 
		result = shortestline_tpoints_geometry((TemporalS *)temp, 
			PointerGetDatum(gs), hasz);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************/
/* These functions suppose that the temporal values overlap in time */

static Datum
shortestline_tpointinst_tpointinst(TemporalInst *inst1, TemporalInst *inst2)
{
	return call_function2(LWGEOM_makeline, temporalinst_value(inst1), 
		temporalinst_value(inst2));
}

static Datum
shortestline_tpointi_tpointi(TemporalI *ti1, TemporalI *ti2,
	Datum (*operator)(Datum, Datum))
{
	/* Compute the distance */
	TemporalI *dist = sync_oper2_temporali_temporali(ti1, ti2, operator, 
		FLOAT8OID);
	Datum minvalue = temporali_min_value(dist);
	TemporalI *mindistance = temporali_at_value(dist, minvalue);
	TimestampTz t = temporali_start_timestamp(mindistance);
	TemporalInst *inst1 = temporali_at_timestamp(ti1, t);
	TemporalInst *inst2 = temporali_at_timestamp(ti2, t);
	Datum result = call_function2(LWGEOM_makeline, temporalinst_value(inst1), 
		temporalinst_value(inst2));
	pfree(dist); pfree(mindistance); pfree(inst1); pfree(inst2);
	return result;
}

static Datum
shortestline_tpointseq_tpointseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum))
{
	/* Compute the distance */
	TemporalSeq *dist = sync_oper2_temporalseq_temporalseq(seq1, seq2, 
		operator, FLOAT8OID, NULL);
	TemporalS *mindist = temporalseq_at_min(dist);
	TimestampTz t = temporals_start_timestamp(mindist);
	/* Make a copy of the sequences with inclusive bounds */
	TemporalSeq *newseq1 = temporalseq_copy(seq1);
	newseq1->period.lower_inc = true;
	newseq1->period.upper_inc = true;
	TemporalSeq *newseq2 = temporalseq_copy(seq2);
	newseq2->period.lower_inc = true;
	newseq2->period.upper_inc = true;
	TemporalInst *inst1 = temporalseq_at_timestamp(newseq1, t);
	TemporalInst *inst2 = temporalseq_at_timestamp(newseq2, t);
	Datum result = call_function2(LWGEOM_makeline, temporalinst_value(inst1), 
		temporalinst_value(inst2));
	pfree(dist); pfree(mindist); pfree(inst1); pfree(inst2);
	pfree(newseq1); pfree(newseq2);
	return result;
}

static Datum
shortestline_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2,
	Datum (*operator)(Datum, Datum))
{
	/* Compute the distance */
	TemporalS *dist = sync_oper2_temporals_temporals(ts1, ts2, operator, 
		FLOAT8OID, NULL);
	TemporalS *mindist = temporals_at_min(dist);
	TimestampTz t = temporals_start_timestamp(mindist);
	TemporalInst *inst1 = temporals_at_timestamp(ts1, t);
	TemporalInst *inst2 = temporals_at_timestamp(ts2, t);
	
	/* If t is at an exclusive bound */
	bool freeinst1 = (inst1 != NULL);
	if (inst1 == NULL)
	{
		int pos;
		temporals_find_timestamp(ts1, t, &pos);
		if (pos == 0)
		{
			TemporalSeq *seq = temporals_seq_n(ts1, 0);
			inst1 = temporalseq_inst_n(seq, 0);
		}
		else if (pos == ts1->count)
		{
			TemporalSeq *seq = temporals_seq_n(ts1, ts1->count-1);
			inst1 = temporalseq_inst_n(seq, seq->count-1);
		}
		else
		{
			TemporalSeq *seq1 = temporals_seq_n(ts1, pos-1);
			TemporalSeq *seq2 = temporals_seq_n(ts1, pos);
			if (timestamp_cmp_internal(temporalseq_end_timestamp(seq1), t) == 0)
				inst1 = temporalseq_inst_n(seq1, seq1->count-1);
			else
				inst1 = temporalseq_inst_n(seq2, 0);
			}		
	}
	
	/* If t is at an exclusive bound */
	bool freeinst2 = (inst2 != NULL);
	if (inst2 == NULL)
	{
		int pos;
		temporals_find_timestamp(ts2, t, &pos);
		if (pos == 0)
		{
			TemporalSeq *seq = temporals_seq_n(ts2, 0);
			inst2 = temporalseq_inst_n(seq, 0);
		}
		else if (pos == ts2->count)
		{
			TemporalSeq *seq = temporals_seq_n(ts2, ts2->count-1);
			inst2 = temporalseq_inst_n(seq, seq->count-1);
		}
		else
		{
			TemporalSeq *seq1 = temporals_seq_n(ts2, pos-1);
			TemporalSeq *seq2 = temporals_seq_n(ts2, pos);
			if (timestamp_cmp_internal(temporalseq_end_timestamp(seq1), t) == 0)
				inst2 = temporalseq_inst_n(seq1, seq1->count-1);
			else
				inst2 = temporalseq_inst_n(seq2, 0);
			}		
	}
	
	Datum result = call_function2(LWGEOM_makeline, temporalinst_value(inst1), 
		temporalinst_value(inst2));
	pfree(dist); pfree(mindist); 
	if (freeinst1)
		pfree(inst1); 
	if (freeinst2)
		pfree(inst2);
	return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(shortestline_tpoint_tpoint);

PGDLLEXPORT Datum
shortestline_tpoint_tpoint(PG_FUNCTION_ARGS)
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

	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Datum (*operator)(Datum, Datum);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp1->flags) && MOBDB_FLAGS_GET_Z(temp2->flags))
			operator = &geom_distance3d;
		else
			operator = &geom_distance2d;
	}
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_distance;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	Datum result;
	if (sync1->type == TEMPORALINST)
		result = shortestline_tpointinst_tpointinst((TemporalInst *)sync1,
			(TemporalInst *)sync2);
	else if (sync1->type == TEMPORALI)
		result = shortestline_tpointi_tpointi((TemporalI *)sync1,
			(TemporalI *)sync2, operator);
	else if (sync1->type == TEMPORALSEQ)
		result = shortestline_tpointseq_tpointseq((TemporalSeq *)sync1,
			(TemporalSeq *)sync2, operator);
	else if (sync1->type == TEMPORALS)
		result = shortestline_tpoints_tpoints((TemporalS *)sync1,
			(TemporalS *)sync2, operator);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	
	pfree(sync1); pfree(sync2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Convert a temporal point into a trajectory geometry/geography where the M  
 * coordinates encode the timestamps in number of seconds since '1970-01-01'
 * The internal representation of timestamps in PostgreSQL is in microseconds
 * since '2000-01-01'. Therefore we need to compute 
 * select date_part('epoch', timestamp '2000-01-01' - timestamp '1970-01-01')
 * which results in 946684800 
 *****************************************************************************/

static LWPOINT *
point_to_trajpoint(GSERIALIZED *gs, TimestampTz t)
{
	int32 srid = gserialized_get_srid(gs);
	bool hasz = FLAGS_GET_Z(gs->flags);
	bool geodetic = FLAGS_GET_GEODETIC(gs->flags);
	double epoch = ((double)t / 1e6) + 946684800 ;
	LWPOINT *result;
	if (hasz)
	{
		POINT3DZ point = gs_get_point3dz(gs);
		result = lwpoint_make4d(srid, point.x, point.y, point.z, epoch);
	}
	else
	{
		POINT2D point = gs_get_point2d(gs);
		result = lwpoint_make3dm(srid, point.x, point.y, epoch);
	}
	FLAGS_SET_GEODETIC(result->flags, geodetic);
	return result;
}

static Datum
tpointinst_to_geo(TemporalInst *inst)
{
	GSERIALIZED *gs = (GSERIALIZED *)PointerGetDatum(temporalinst_value(inst));
	LWPOINT *point = point_to_trajpoint(gs, inst->t);
	GSERIALIZED *result = geometry_serialize((LWGEOM *)point);
	pfree(point);
	return PointerGetDatum(result);
}

static Datum
tpointi_to_geo(TemporalI *ti)
{
	TemporalInst *inst = temporali_inst_n(ti, 0);
	GSERIALIZED *gs = (GSERIALIZED *)PointerGetDatum(temporalinst_value(inst));
	int32 srid = gserialized_get_srid(gs);
	LWGEOM **points = palloc(sizeof(LWGEOM *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		inst = temporali_inst_n(ti, i);
		gs = (GSERIALIZED *)PointerGetDatum(temporalinst_value(inst));
		points[i] = (LWGEOM *)point_to_trajpoint(gs, inst->t);
	}
	GSERIALIZED *result;
	if (ti->count == 1)
		result = geometry_serialize(points[0]);
	else
	{
		LWGEOM *mpoint = (LWGEOM *)lwcollection_construct(MULTIPOINTTYPE, srid,
			NULL, ti->count, points);
		result = geometry_serialize(mpoint);
		pfree(mpoint);
	}

	for (int i = 0; i < ti->count; i++)
		pfree(points[i]);
	pfree(points);
	return PointerGetDatum(result);
}

static Datum
tpointseq_to_geo(TemporalSeq *seq)
{
	LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	GSERIALIZED *gs = (GSERIALIZED *)PointerGetDatum(temporalinst_value(inst));
	int32 srid = gserialized_get_srid(gs);
	for (int i = 0; i < seq->count; i++)
	{
		inst = temporalseq_inst_n(seq, i);
		gs = (GSERIALIZED *)PointerGetDatum(temporalinst_value(inst));
		points[i] = (LWGEOM *)point_to_trajpoint(gs, inst->t);
	}
	GSERIALIZED *result;
	if (seq->count == 1)
		result = geometry_serialize(points[0]);
	else
	{
		LWGEOM *line = (LWGEOM *)lwline_from_lwgeom_array(srid, seq->count, points);
		result = geometry_serialize(line);
		pfree(line);
	}

	for (int i = 0; i < seq->count; i++)
		pfree(points[i]);
	pfree(points);

	return PointerGetDatum(result);
}

static Datum
tpoints_to_geo(TemporalS *ts)
{
	Datum *geoms = palloc(sizeof(Datum) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		geoms[i] = tpointseq_to_geo(seq);
	}
	Datum result;
	if (ts->count == 1)
		result = geoms[0];
	else
	{
		ArrayType *array = datumarr_to_array(geoms, ts->count, ts->valuetypid);
		result = call_function1(LWGEOM_collect_garray, PointerGetDatum(array));		
		for (int i = 0; i < ts->count; i++)
			pfree(DatumGetPointer(geoms[i]));
		pfree(array);
	}
	pfree(geoms);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_to_geo);

PGDLLEXPORT Datum
tpoint_to_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum result;
	if (temp->type == TEMPORALINST) 
		result = tpointinst_to_geo((TemporalInst *)temp);
	else if (temp->type == TEMPORALI) 
		result = tpointi_to_geo((TemporalI *)temp);
	else if (temp->type == TEMPORALSEQ) 
		result = tpointseq_to_geo((TemporalSeq *)temp);
	else if (temp->type == TEMPORALS) 
		result = tpoints_to_geo((TemporalS *)temp);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Convert trajectory geometry/geography where the M coordinates encode the 
 * timestamps in number of seconds since '1970-01-01' into a temporal point.
 *****************************************************************************/

static TemporalInst *
trajpoint_to_tpointinst(LWPOINT *lwpoint)
{
	bool hasz = FLAGS_GET_Z(lwpoint->flags);
	bool geodetic = FLAGS_GET_GEODETIC(lwpoint->flags);
	LWPOINT *lwpoint1;
	TimestampTz t;
	if (hasz)
	{
		POINT4D point = getPoint4d(lwpoint->point, 0);
		t = (point.m - 946684800) * 1e6;
		lwpoint1 = lwpoint_make3dz(lwpoint->srid, point.x, point.y, point.z);
	}
	else
	{
		POINT3DM point = getPoint3dm(lwpoint->point, 0);
		t = (point.m - 946684800) * 1e6;
		lwpoint1 = lwpoint_make2d(lwpoint->srid, point.x, point.y);
	}
	FLAGS_SET_GEODETIC(lwpoint1->flags, geodetic);
	GSERIALIZED *gs = geometry_serialize((LWGEOM *)lwpoint1);
	Oid valuetypid = geodetic ? type_oid(T_GEOGRAPHY) : type_oid(T_GEOMETRY);
	TemporalInst *result = temporalinst_make(PointerGetDatum(gs), t, 
		valuetypid);
	pfree(gs);
	return result;	
}

static TemporalInst *
geo_to_tpointinst(GSERIALIZED *gs)
{
	/* Geometry is a POINT */
	LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
	TemporalInst *result = trajpoint_to_tpointinst((LWPOINT *)lwgeom);
	lwgeom_free(lwgeom);
	return result;
}

static TemporalI *
geo_to_tpointi(GSERIALIZED *gs)
{
	TemporalI *result;
	/* Geometry is a MULTIPOINT */
	LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
	bool hasz = FLAGS_GET_Z(gs->flags);
	/* Verify that is a valid set of trajectory points */
	LWCOLLECTION *lwcoll = lwgeom_as_lwcollection(lwgeom);
	double m1 = -1 * DBL_MAX, m2;
	int npoints = lwcoll->ngeoms;
	for (int i = 0; i < npoints; i++)
	{
		LWPOINT *lwpoint = (LWPOINT *)lwcoll->geoms[i];
		if (hasz)
		{
			POINT4D point = getPoint4d(lwpoint->point, 0);
			m2 = point.m;
		}
		else
		{
			POINT3DM point = getPoint3dm(lwpoint->point, 0);
			m2 = point.m;
		}
		if (m1 >= m2)
		{
			lwgeom_free(lwgeom);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Trajectory must be valid")));
		}
		m1 = m2;
	}
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * npoints);
	for (int i = 0; i < npoints; i++)
		instants[i] = trajpoint_to_tpointinst((LWPOINT *)lwcoll->geoms[i]);
	result = temporali_from_temporalinstarr(instants, npoints);
	
	lwgeom_free(lwgeom);
	for (int i = 0; i < npoints; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

static TemporalSeq *
geo_to_tpointseq(GSERIALIZED *gs)
{
	TemporalSeq *result;
	/* Geometry is a LINESTRING */
	LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
	if (lwgeom_is_trajectory(lwgeom) != LW_TRUE)
	{
		lwgeom_free(lwgeom);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Trajectory must be valid")));
	}
	LWLINE *lwline = lwgeom_as_lwline(lwgeom);
	int npoints = lwline->points->npoints;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * npoints);
	for (int i = 0; i < npoints; i++)
	{
		/* Returns freshly allocated LWPOINT */
		LWPOINT *lwpoint = lwline_get_lwpoint(lwline, i);
		instants[i] = trajpoint_to_tpointinst(lwpoint);
		lwpoint_free(lwpoint);
	}
	result = temporalseq_from_temporalinstarr(instants, 
		npoints, true, true, true);
	for (int i = 0; i < npoints; i++)
		pfree(instants[i]);
	pfree(instants);
	lwgeom_free(lwgeom);
	return result;
}

static TemporalS *
geo_to_tpoints(GSERIALIZED *gs)
{
	TemporalS *result;
	/* Geometry is a MULTILINESTRING or a COLLECTION */
	LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
	LWCOLLECTION *lwcoll = lwgeom_as_lwcollection(lwgeom);
	int ngeoms = lwcoll->ngeoms;
	for (int i = 0; i < ngeoms; i++)
	{
		LWGEOM *lwgeom1 = lwcoll->geoms[i];
		if (lwgeom1->type != POINTTYPE && lwgeom1->type != LINETYPE)
		{
			lwgeom_free(lwgeom);
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Component geometry/geography must be of type Point(Z)M or Linestring(Z)M")));
		}
	}
	
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ngeoms);
	for (int i = 0; i < ngeoms; i++)
	{
		LWGEOM *lwgeom1 = lwcoll->geoms[i];
		GSERIALIZED *gs1 = geometry_serialize(lwgeom1);
		if (lwgeom1->type == POINTTYPE)
		{
			TemporalInst *inst = geo_to_tpointinst(gs1);
			sequences[i] = temporalseq_from_temporalinstarr(&inst, 1,
				true, true, false); 
			pfree(inst);
		}
		else /* lwgeom1->type == LINETYPE */
			sequences[i] = geo_to_tpointseq(gs1);
		pfree(gs1);
	}
	result = temporals_from_temporalseqarr(sequences, 
		ngeoms, false);
	for (int i = 0; i < ngeoms; i++)
		pfree(sequences[i]);
	pfree(sequences);
	lwgeom_free(lwgeom);
	return result;
}

PG_FUNCTION_INFO_V1(geo_to_tpoint);

PGDLLEXPORT Datum
geo_to_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Input trajectory cannot be empty")));
	}
	if (! FLAGS_GET_M(gs->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Input trajectory do not have M values")));
	}
	
	Temporal *result;
	if (gserialized_get_type(gs) == POINTTYPE)
		result = (Temporal *)geo_to_tpointinst(gs);
	else if (gserialized_get_type(gs) == MULTIPOINTTYPE)
		result = (Temporal *)geo_to_tpointi(gs);
	else if (gserialized_get_type(gs) == LINETYPE)
		result = (Temporal *)geo_to_tpointseq(gs);
	else if (gserialized_get_type(gs) == MULTILINETYPE ||
		gserialized_get_type(gs) == COLLECTIONTYPE)
		result = (Temporal *)geo_to_tpoints(gs);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	
	PG_FREE_IF_COPY(gs, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
