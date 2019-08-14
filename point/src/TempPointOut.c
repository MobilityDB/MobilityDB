/*****************************************************************************
 *
 * TempPointOut.c
 *	  Output of temporal points in WKT, EWKT and MF-JSON format
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TempPointOut.h"

#include <assert.h>
#include <float.h>
#include <executor/spi.h>
#include <utils/builtins.h>

#include "TemporalTypes.h"
#include "TemporalUtil.h"
#include "TemporalPoint.h"
#include "SpatialFuncs.h"

/* The following definitions are taken from PostGIS */

#define FP_TOLERANCE 1e-12

#define OUT_MAX_DOUBLE 1E15
#define OUT_SHOW_DIGS_DOUBLE 20
#define OUT_MAX_DOUBLE_PRECISION 15
#define OUT_MAX_DIGS_DOUBLE (OUT_SHOW_DIGS_DOUBLE + 2) /* +2 mean add dot and sign */
#define OUT_DOUBLE_BUFFER_SIZE \
	OUT_MAX_DIGS_DOUBLE + OUT_MAX_DOUBLE_PRECISION + 1

/*****************************************************************************
 * Output in WKT and EWKT format 
 *****************************************************************************/

/* 
 * Output a geometry in Well-Known Text (WKT) and Extended Well-Known Text 
 * (EWKT) format.
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

/* Output a temporal point in WKT format */

static text *
tpoint_as_text_internal(Temporal *temp)
{
	char *str = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		str = temporalinst_to_string((TemporalInst *)temp, &wkt_out);
	else if (temp->duration == TEMPORALI) 
		str = temporali_to_string((TemporalI *)temp, &wkt_out);
	else if (temp->duration == TEMPORALSEQ) 
		str = temporalseq_to_string((TemporalSeq *)temp, &wkt_out);
	else if (temp->duration == TEMPORALS) 
		str = temporals_to_string((TemporalS *)temp, &wkt_out);
	text *result = cstring_to_text(str);
	pfree(str);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_as_text);

PGDLLEXPORT Datum
tpoint_as_text(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	text *result = tpoint_as_text_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TEXT_P(result);
}

/* Output a temporal point in WKT format */

static text *
tpoint_as_ewkt_internal(Temporal *temp)
{
	int srid = tpoint_srid_internal(temp);
	char str1[20];
	if (srid > 0)
		sprintf(str1, "SRID=%d;", srid);
	else
		str1[0] = '\0';
	char *str2 = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		str2 = temporalinst_to_string((TemporalInst *)temp, &wkt_out);
	else if (temp->duration == TEMPORALI) 
		str2 = temporali_to_string((TemporalI *)temp, &wkt_out);
	else if (temp->duration == TEMPORALSEQ) 
		str2 = temporalseq_to_string((TemporalSeq *)temp, &wkt_out);
	else if (temp->duration == TEMPORALS) 
		str2 = temporals_to_string((TemporalS *)temp, &wkt_out);
	char *str = (char *)palloc(strlen(str1) + strlen(str2) + 1);
	strcpy(str, str1);
	strcat(str, str2);
	text *result = cstring_to_text(str);
	pfree(str2); pfree(str);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_as_ewkt);

PGDLLEXPORT Datum
tpoint_as_ewkt(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	text *result = tpoint_as_ewkt_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

/* Output a geometry/geography array in WKT format */

PG_FUNCTION_INFO_V1(geoarr_as_text);

PGDLLEXPORT Datum
geoarr_as_text(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(geoarr_as_ewkt);

PGDLLEXPORT Datum
geoarr_as_ewkt(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(tpointarr_as_text);

PGDLLEXPORT Datum
tpointarr_as_text(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count;
	Temporal **temparr = temporalarr_extract(array, &count);
	if (count == 0)
	{
		PG_FREE_IF_COPY(array, 0);
		PG_RETURN_NULL();
	}
	text **textarr = palloc(sizeof(text *) * count);
	for (int i = 0; i < count; i++)
		textarr[i] = tpoint_as_text_internal(temparr[i]);
	ArrayType *result = textarr_to_array(textarr, count);

	pfree(temparr);
	for (int i = 0; i < count; i++)
		pfree(textarr[i]);
	pfree(textarr);
	PG_FREE_IF_COPY(array, 0);

	PG_RETURN_ARRAYTYPE_P(result);
}

/* Output a temporal point array in WKT format prefixed with the SRID */

PG_FUNCTION_INFO_V1(tpointarr_as_ewkt);

PGDLLEXPORT Datum
tpointarr_as_ewkt(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count;
	Temporal **temparr = temporalarr_extract(array, &count);
	if (count == 0)
	{
		PG_FREE_IF_COPY(array, 0);
		PG_RETURN_NULL();
	}
	text **textarr = palloc(sizeof(text *) * count);
	for (int i = 0; i < count; i++)
		textarr[i] = tpoint_as_ewkt_internal(temparr[i]);
	ArrayType *result = textarr_to_array(textarr, count);

	pfree(temparr);
	for (int i = 0; i < count; i++)
		pfree(textarr[i]);
	pfree(textarr);
	PG_FREE_IF_COPY(array, 0);

	PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Output in MFJSON format 
 *****************************************************************************/

/*
 * Removes trailing zeros and dot for a %f formatted number.
 * Modifies input.
 * 
 * This function is taken from PostGIS file lwprint.c
 */
static void
trim_trailing_zeros(char *str)
{
	char *ptr, *totrim = NULL;
	int len;
	ptr = strchr(str, '.');
	if (!ptr) return; /* no dot, no decimal digits */
	len = strlen(ptr);
	for (int i = len - 1; i; i--)
	{
		if (ptr[i] != '0') break;
		totrim = &ptr[i];
	}
	if (totrim)
	{
		if (ptr == totrim - 1)
			*ptr = '\0';
		else
			*totrim = '\0';
	}
}

/*
 * Print an ordinate value using at most the given number of decimal digits
 *
 * The actual number of printed decimal digits may be less than the
 * requested ones if out of significant digits.
 *
 * The function will not write more than maxsize bytes, including the
 * terminating NULL. Returns the number of bytes that would have been
 * written if there was enough space (excluding terminating NULL).
 * So a return of ``bufsize'' or more means that the string was
 * truncated and misses a terminating NULL.
 * 
 * This function is taken from PostGIS file lwprint.c
 */
static int
lwprint_double(double d, int maxdd, char* buf, size_t bufsize)
{
	double ad = fabs(d);
	int ndd;
	int length = 0;
	if (ad <= FP_TOLERANCE)
	{
		d = 0;
		ad = 0;
	}
	if (ad < OUT_MAX_DOUBLE)
	{
		ndd = ad < 1 ? 0 : floor(log10(ad)) + 1; /* non-decimal digits */
		if (maxdd > (OUT_MAX_DOUBLE_PRECISION - ndd)) maxdd -= ndd;
		length = snprintf(buf, bufsize, "%.*f", maxdd, d);
	}
	else
	{
		length = snprintf(buf, bufsize, "%g", d);
	}
	trim_trailing_zeros(buf);
	return length;
}

/*
 * Retrieve an SRS from a given SRID
 * Require valid spatial_ref_sys table entry
 * Could return SRS as short one (i.e EPSG:4326)
 * or as long one: (i.e urn:ogc:def:crs:EPSG::4326).
 * 
 * This function is taken from PostGIS file lwgeom_export.c
 */
static char *
getSRSbySRID(int32_t srid, bool short_crs)
{
	char query[256];
	char *srs, *srscopy;
	int size, err;

	if (SPI_OK_CONNECT != SPI_connect ())
	{
		elog(NOTICE, "getSRSbySRID: could not connect to SPI manager");
		SPI_finish();
		return NULL;
	}

	if (short_crs)
		sprintf(query, "SELECT auth_name||':'||auth_srid \
				FROM spatial_ref_sys WHERE srid='%d'", srid);
	else
		sprintf(query, "SELECT 'urn:ogc:def:crs:'||auth_name||'::'||auth_srid \
				FROM spatial_ref_sys WHERE srid='%d'", srid);

	err = SPI_exec(query, 1);
	if (err < 0)
	{
		elog(NOTICE, "getSRSbySRID: error executing query %d", err);
		SPI_finish();
		return NULL;
	}

	/* No entry in spatial_ref_sys */
	if (SPI_processed <= 0)
	{
		SPI_finish();
		return NULL;
	}

	/* Get result */
	srs = SPI_getvalue(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1);

	/* NULL result */
	if (! srs)
	{
		SPI_finish();
		return NULL;
	}

	/* Copy result to upper executor context */
	size = strlen(srs) + 1;
	srscopy = SPI_palloc(size);
	memcpy(srscopy, srs, size);

	/* Disconnect from SPI */
	SPI_finish();

	return srscopy;
}

/*
 * Handle coordinate array
 * Returns maximum size of rendered coordinate array in bytes.
 */
static size_t
coordinates_mfjson_size(int npoints, bool hasz, int precision)
{
	assert(precision <= OUT_MAX_DOUBLE_PRECISION);
	if (hasz)
		return (OUT_MAX_DIGS_DOUBLE + precision + sizeof(",,"))
		   * 3 * npoints + sizeof(",[]");
	else
		return (OUT_MAX_DIGS_DOUBLE + precision + sizeof(","))
			   * 2 * npoints + sizeof(",[]");
}

static size_t
coordinates_mfjson_buf(char *output, TemporalInst *inst, int precision)
{
	char *ptr;
	char x[OUT_DOUBLE_BUFFER_SIZE];
	char y[OUT_DOUBLE_BUFFER_SIZE];
	char z[OUT_DOUBLE_BUFFER_SIZE];

	assert (precision <= OUT_MAX_DOUBLE_PRECISION);
	ptr = output;

	if (!MOBDB_FLAGS_GET_Z(inst->flags))
	{
		POINT2D pt = datum_get_point2d(temporalinst_value(inst));
		lwprint_double(pt.x, precision, x, OUT_DOUBLE_BUFFER_SIZE);
		lwprint_double(pt.y, precision, y, OUT_DOUBLE_BUFFER_SIZE);
		ptr += sprintf(ptr, "[%s,%s]", x, y);
	}
	else
	{
		POINT3DZ pt = datum_get_point3dz(temporalinst_value(inst));
		lwprint_double(pt.x, precision, x, OUT_DOUBLE_BUFFER_SIZE);
		lwprint_double(pt.y, precision, y, OUT_DOUBLE_BUFFER_SIZE);
		lwprint_double(pt.z, precision, z, OUT_DOUBLE_BUFFER_SIZE);
		ptr += sprintf(ptr, "[%s,%s,%s]", x, y, z);
	}
	return (ptr - output);
}

/*
 * Handle datetimes array
 * Returns maximum size of datetimes array in bytes.
 * Example: "datetimes":["2019-08-06T18:35:48.021455+02:30","2019-08-06T18:45:18.476983+02:30"],
 * 32 characters for the timestamptz + 2 double quotes + 1 comma
 */
static size_t
datetimes_mfjson_size(int npoints)
{
	return 35 * npoints + sizeof("[],");
}

static size_t
datetimes_mfjson_buf(char *output, TemporalInst *inst)
{
	char *ptr = output;
	char *t = call_output(TIMESTAMPTZOID, inst->t);
	/* Replace ' ' by 'T' as separator between date and time parts */
	t[10] = 'T';
	ptr += sprintf(ptr, "\"%s\"", t);
	pfree(t);
	return (ptr - output);
}

/*
 * Handle SRS
 */
static size_t
asmfjson_srs_size(char *srs)
{
	int size = sizeof("'crs':{'type':'name',");
	size += sizeof("'properties':{'name':''}},");
	size += strlen(srs) * sizeof(char);
	return size;
}

static size_t
asmfjson_srs_buf(char *output, char *srs)
{
	char *ptr = output;
	ptr += sprintf(ptr, "\"crs\":{\"type\":\"name\",");
	ptr += sprintf(ptr, "\"properties\":{\"name\":\"%s\"}},", srs);
	return (ptr - output);
}

/*
 * Handle Bbox
 */
static size_t
asmfjson_bbox_size(int hasz, int precision)
{
	/* The maximum size of a timestamptz is 35, e.g., "2019-08-06 23:18:16.195062-09:30" */
	int size = sizeof("'stBoundedBy':{'period':{'begin':,'end':}},") + 70;
	if (!hasz)
	{
		size += sizeof("'bbox':[,,,],");
		size +=	2 * 2 * (OUT_MAX_DIGS_DOUBLE + precision);
	}
	else
	{
		size += sizeof("\"bbox\":[,,,,,],");
		size +=	2 * 3 * (OUT_MAX_DIGS_DOUBLE + precision);
	}
	return size;
}

static size_t
asmfjson_bbox_buf(char *output, STBOX *bbox, int hasz, int precision)
{
	char *ptr = output;
	ptr += sprintf(ptr, "\"stBoundedBy\":{");
	if (!hasz)
		ptr += sprintf(ptr, "\"bbox\":[%.*f,%.*f,%.*f,%.*f],",
				   precision, bbox->xmin, precision, bbox->ymin,
				   precision, bbox->xmax, precision, bbox->ymax);
	else
		ptr += sprintf(ptr, "\"bbox\":[%.*f,%.*f,%.*f,%.*f,%.*f,%.*f],",
					precision, bbox->xmin, precision, bbox->ymin, precision, bbox->zmin,
					precision, bbox->xmax, precision, bbox->ymax, precision, bbox->zmax);
	char *begin = call_output(TIMESTAMPTZOID, bbox->tmin);
	char *end = call_output(TIMESTAMPTZOID, bbox->tmax);
	ptr += sprintf(ptr, "\"period\":{\"begin\":\"%s\",\"end\":\"%s\"}},", begin, end);
	pfree(begin); pfree(end); 
	return (ptr - output);
}

/*****************************************************************************/

static size_t
tpointinst_asmfjson_size(const TemporalInst *inst, int precision, STBOX *bbox, char *srs)
{
	int size = coordinates_mfjson_size(1, MOBDB_FLAGS_GET_Z(inst->flags), precision);
	size += datetimes_mfjson_size(1);
	size += sizeof("{'type':'MovingPoint',");
	size += sizeof("'coordinates':,'datetimes':,'interpolations':['Discrete']}");
	if (srs) size += asmfjson_srs_size(srs);
	if (bbox) size += asmfjson_bbox_size(MOBDB_FLAGS_GET_Z(inst->flags), precision);
	return size;
}

static size_t
tpointinst_asmfjson_buf(TemporalInst *inst, int precision, STBOX *bbox, char *srs, char *output)
{
	char *ptr = output;
	ptr += sprintf(ptr, "{\"type\":\"MovingPoint\",");
	if (srs) ptr += asmfjson_srs_buf(ptr, srs);
	if (bbox) ptr += asmfjson_bbox_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(inst->flags), precision);
	ptr += sprintf(ptr, "\"coordinates\":");
	ptr += coordinates_mfjson_buf(ptr, inst, precision);
	ptr += sprintf(ptr, ",\"datetimes\":");
	ptr += datetimes_mfjson_buf(ptr, inst);
	ptr += sprintf(ptr, ",\"interpolations\":[\"Discrete\"]}");
	return (ptr - output);
}

static char *
tpointinst_asmfjson(TemporalInst *inst, int precision, STBOX *bbox, char *srs)
{
	int size = tpointinst_asmfjson_size(inst, precision, bbox, srs);
	char *output = palloc(size);
	tpointinst_asmfjson_buf(inst, precision, bbox, srs, output);
	return output;
}

/*****************************************************************************/

static size_t
tpointi_asmfjson_size(const TemporalI *ti, int precision, STBOX *bbox, char *srs)
{
	int size = coordinates_mfjson_size(ti->count, MOBDB_FLAGS_GET_Z(ti->flags), precision);
	size += datetimes_mfjson_size(ti->count);
	size += sizeof("{'type':'MovingPoint',");
	size += sizeof("'coordinates':[],'datetimes':[],'interpolations':['Discrete']}");
	if (srs) size += asmfjson_srs_size(srs);
	if (bbox) size += asmfjson_bbox_size(MOBDB_FLAGS_GET_Z(ti->flags), precision);
	return size;
}

static size_t
tpointi_asmfjson_buf(TemporalI *ti, int precision, STBOX *bbox, char *srs, char *output)
{
	char *ptr = output;
	ptr += sprintf(ptr, "{\"type\":\"MovingPoint\",");
	if (srs) ptr += asmfjson_srs_buf(ptr, srs);
	if (bbox) ptr += asmfjson_bbox_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(ti->flags), precision);
	ptr += sprintf(ptr, "\"coordinates\":[");
	for (int i = 0; i < ti->count; i++)
	{
		if (i) ptr += sprintf(ptr, ",");
		ptr += coordinates_mfjson_buf(ptr, temporali_inst_n(ti, i), precision);
	}
	ptr += sprintf(ptr, "],\"datetimes\":[");
	for (int i = 0; i < ti->count; i++)
	{
		if (i) ptr += sprintf(ptr, ",");
		ptr += datetimes_mfjson_buf(ptr, temporali_inst_n(ti, i));
	}
	ptr += sprintf(ptr, "],\"interpolations\":[\"Discrete\"]}");
	return (ptr - output);
}

static char *
tpointi_asmfjson(TemporalI *ti, int precision, STBOX *bbox, char *srs)
{
	int size = tpointi_asmfjson_size(ti, precision, bbox, srs);
	char *output = palloc(size);
	tpointi_asmfjson_buf(ti, precision, bbox, srs, output);
	return output;
}

/*****************************************************************************/

static size_t
tpointseq_asmfjson_size(const TemporalSeq *seq, int precision, STBOX *bbox, char *srs)
{
	int size = coordinates_mfjson_size(seq->count, MOBDB_FLAGS_GET_Z(seq->flags), precision);
	size += datetimes_mfjson_size(seq->count);
	size += sizeof("{'type':'MovingPoint',");
	size += sizeof("'coordinates':[],'datetimes':[],'lower_inc':false,'upper_inc':false,interpolations':['Linear']}");
	if (srs) size += asmfjson_srs_size(srs);
	if (bbox) size += asmfjson_bbox_size(MOBDB_FLAGS_GET_Z(seq->flags), precision);
	return size;
}

static size_t
tpointseq_asmfjson_buf(TemporalSeq *seq, int precision, STBOX *bbox, char *srs, char *output)
{
	char *ptr = output;
	ptr += sprintf(ptr, "{\"type\":\"MovingPoint\",");
	if (srs) ptr += asmfjson_srs_buf(ptr, srs);
	if (bbox) ptr += asmfjson_bbox_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(seq->flags), precision);
	ptr += sprintf(ptr, "\"coordinates\":[");
	for (int i = 0; i < seq->count; i++)
	{
		if (i) ptr += sprintf(ptr, ",");
		ptr += coordinates_mfjson_buf(ptr, temporalseq_inst_n(seq, i), precision);
	}
	ptr += sprintf(ptr, "],\"datetimes\":[");
	for (int i = 0; i < seq->count; i++)
	{
		if (i) ptr += sprintf(ptr, ",");
		ptr += datetimes_mfjson_buf(ptr, temporalseq_inst_n(seq, i));
	}
	ptr += sprintf(ptr, "],\"lower_inc\":%s,\"upper_inc\":%s,\"interpolations\":[\"Linear\"]}", 
		seq->period.lower_inc ? "true" : "false", seq->period.upper_inc ? "true" : "false");
	return (ptr - output);
}

static char *
tpointseq_asmfjson(TemporalSeq *seq, int precision, STBOX *bbox, char *srs)
{
	int size = tpointseq_asmfjson_size(seq, precision, bbox, srs);
	char *output = palloc(size);
	tpointseq_asmfjson_buf(seq, precision, bbox, srs, output);
	return output;
}

/*****************************************************************************/

static size_t
tpoints_asmfjson_size(TemporalS *ts, int precision, STBOX *bbox, char *srs)
{
	bool hasz = MOBDB_FLAGS_GET_Z(ts->flags);
	int size = sizeof("{'type':'MovingPoint','sequences':[],");
	size += sizeof("{'coordinates':[],'datetimes':[],'lower_inc':false,'upper_inc':false},") * ts->count;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		coordinates_mfjson_size(seq->count, hasz, precision);
		size += datetimes_mfjson_size(seq->count);
	}
	size += sizeof(",interpolations':['Linear']}");
	if (srs) size += asmfjson_srs_size(srs);
	if (bbox) size += asmfjson_bbox_size(hasz, precision);
	return size;
}

static size_t
tpoints_asmfjson_buf(TemporalS *ts, int precision, STBOX *bbox, char *srs, char *output)
{
	char *ptr = output;
	ptr += sprintf(ptr, "{\"type\":\"MovingPoint\",");
	if (srs) ptr += asmfjson_srs_buf(ptr, srs);
	if (bbox) ptr += asmfjson_bbox_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(ts->flags), precision);
	ptr += sprintf(ptr, "\"sequences\":[");
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		if (i) ptr += sprintf(ptr, ",");
		ptr += sprintf(ptr, "{\"coordinates\":[");
		for (int j = 0; j < seq->count; j++)
		{
			if (j) ptr += sprintf(ptr, ",");
			ptr += coordinates_mfjson_buf(ptr, temporalseq_inst_n(seq, j), precision);
		}
		ptr += sprintf(ptr, "],\"datetimes\":[");
		for (int j = 0; j < seq->count; j++)
		{
			if (j) ptr += sprintf(ptr, ",");
			ptr += datetimes_mfjson_buf(ptr, temporalseq_inst_n(seq, j));
		}
		ptr += sprintf(ptr, "],\"lower_inc\":%s,\"upper_inc\":%s}", 
			seq->period.lower_inc ? "true" : "false", seq->period.upper_inc ? "true" : "false");
	}
	ptr += sprintf(ptr, "],\"interpolations\":[\"Linear\"]}");
	return (ptr - output);
}

static char *
tpoints_asmfjson(TemporalS *ts, int precision, STBOX *bbox, char *srs)
{
	int size = tpoints_asmfjson_size(ts, precision, bbox, srs);
	char *output = palloc(size);
	tpoints_asmfjson_buf(ts, precision, bbox, srs, output);
	return output;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_as_mfjson);

PGDLLEXPORT Datum
tpoint_as_mfjson(PG_FUNCTION_ARGS)
{
	int has_bbox = 0;
	int precision = DBL_DIG;
	char *srs = NULL;
	
	/* Get the temporal point */
	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);

	/* Retrieve precision if any (default is max) */
	if (PG_NARGS() > 1 && !PG_ARGISNULL(1))
	{
		precision = PG_GETARG_INT32(1);
		if (precision > DBL_DIG)
			precision = DBL_DIG;
		else if (precision < 0)
			precision = 0;
	}

	/* Retrieve output option
	 * 0 = without option (default)
	 * 1 = bbox
	 * 2 = short crs
	 * 4 = long crs
	 */
	if (PG_NARGS() > 2 && !PG_ARGISNULL(2))
	{
		int option = PG_GETARG_INT32(2);

		if (option & 2 || option & 4)
		{
			int32_t srid = tpoint_srid_internal(temp);
			if (srid != SRID_UNKNOWN)
			{
				if (option & 2)
					srs = getSRSbySRID(srid, true);

				if (option & 4)
					srs = getSRSbySRID(srid, false);

				if (!srs)
				{
					elog(ERROR, "SRID %i unknown in spatial_ref_sys table",
						  srid);
					PG_RETURN_NULL();
				}
			}
		}

		if (option & 1)
			has_bbox = 1;
	}

	/* Get bounding box if needed */
	STBOX *bbox = NULL;
	STBOX tmp = {0,0,0,0,0,0,0,0,0};
	if (has_bbox)
	{
		temporal_bbox(&tmp, temp);
		bbox = &tmp;
	}

	char *mfjson = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST)
		mfjson = tpointinst_asmfjson((TemporalInst *)temp, precision, bbox, srs);
	else if (temp->duration == TEMPORALI)
		mfjson = tpointi_asmfjson((TemporalI *)temp, precision, bbox, srs);
	else if (temp->duration == TEMPORALSEQ)
		mfjson = tpointseq_asmfjson((TemporalSeq *)temp, precision, bbox, srs);
	else if (temp->duration == TEMPORALS)
		mfjson = tpoints_asmfjson((TemporalS *)temp, precision, bbox, srs);
	text *result = cstring_to_text(mfjson);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Output in WKB format 
 *****************************************************************************/

/**
* Well-Known Binary (WKB) Output Variant Types
*/

#define WKB_TIMESTAMP_SIZE 8 /* Internal use only */
#define WKB_DOUBLE_SIZE 8 /* Internal use only */
#define WKB_INT_SIZE 4 /* Internal use only */
#define WKB_BYTE_SIZE 1 /* Internal use only */

/* Machine endianness */
#define XDR 0 /* big endian */
#define NDR 1 /* little endian */

/*
* Look-up table for hex writer
*/
static char *hexchr = "0123456789ABCDEF";

static char
getMachineEndian(void)
{
	static int endian_check_int = 1; /* don't modify this!!! */

	return *((char *) &endian_check_int); /* 0 = big endian | xdr,
	                                       * 1 = little endian | ndr
	                                       */
}

/*
* SwapBytes?
*/
static inline bool
wkb_swap_bytes(uint8_t variant)
{
	/* If requested variant matches machine arch, we don't have to swap! */
	if (((variant & WKB_NDR) && (getMachineEndian() == NDR)) ||
	     ((! (variant & WKB_NDR)) && (getMachineEndian() == XDR)))
	{
		return false;
	}
	return true;
}

/*
* Integer32
*/
static uint8_t*
integer_to_wkb_buf(const int ival, uint8_t *buf, uint8_t variant)
{
	char *iptr = (char*)(&ival);
	int i = 0;

	if (sizeof(int) != WKB_INT_SIZE)
		elog(ERROR, "Machine int size is not %d bytes!", WKB_INT_SIZE);

	if (variant & WKB_HEX)
	{
		int swap = wkb_swap_bytes(variant);
		/* Machine/request arch mismatch, so flip byte order */
		for (i = 0; i < WKB_INT_SIZE; i++)
		{
			int j = (swap ? WKB_INT_SIZE - 1 - i : i);
			uint8_t b = iptr[j];
			/* Top four bits to 0-F */
			buf[2*i] = hexchr[b >> 4];
			/* Bottom four bits to 0-F */
			buf[2*i+1] = hexchr[b & 0x0F];
		}
		return buf + (2 * WKB_INT_SIZE);
	}
	else
	{
		/* Machine/request arch mismatch, so flip byte order */
		if (wkb_swap_bytes(variant))
		{
			for (i = 0; i < WKB_INT_SIZE; i++)
			{
				buf[i] = iptr[WKB_INT_SIZE - 1 - i];
			}
		}
		/* If machine arch and requested arch match, don't flip byte order */
		else
		{
			memcpy(buf, iptr, WKB_INT_SIZE);
		}
		return buf + WKB_INT_SIZE;
	}
}

/*
* Float64
*/
static uint8_t*
double_to_wkb_buf(const double d, uint8_t *buf, uint8_t variant)
{
	char *dptr = (char*)(&d);
	int i = 0;

	if (sizeof(double) != WKB_DOUBLE_SIZE)
	{
		elog(ERROR, "Machine double size is not %d bytes!", WKB_DOUBLE_SIZE);
	}

	if (variant & WKB_HEX)
	{
		int swap =  wkb_swap_bytes(variant);
		/* Machine/request arch mismatch, so flip byte order */
		for (i = 0; i < WKB_DOUBLE_SIZE; i++)
		{
			int j = (swap ? WKB_DOUBLE_SIZE - 1 - i : i);
			uint8_t b = dptr[j];
			/* Top four bits to 0-F */
			buf[2*i] = hexchr[b >> 4];
			/* Bottom four bits to 0-F */
			buf[2*i+1] = hexchr[b & 0x0F];
		}
		return buf + (2 * WKB_DOUBLE_SIZE);
	}
	else
	{
		/* Machine/request arch mismatch, so flip byte order */
		if (wkb_swap_bytes(variant))
		{
			for (i = 0; i < WKB_DOUBLE_SIZE; i++)
			{
				buf[i] = dptr[WKB_DOUBLE_SIZE - 1 - i];
			}
		}
		/* If machine arch and requested arch match, don't flip byte order */
		else
		{
			memcpy(buf, dptr, WKB_DOUBLE_SIZE);
		}
		return buf + WKB_DOUBLE_SIZE;
	}
}

/*
* Timestamp aka int64
*/
static uint8_t*
timestamp_to_wkb_buf(const TimestampTz t, uint8_t *buf, uint8_t variant)
{
	char *tptr = (char*)(&t);
	int i = 0;

	if (sizeof(double) != WKB_DOUBLE_SIZE)
	{
		elog(ERROR, "Machine timestamp size is not %d bytes!", WKB_DOUBLE_SIZE);
	}

	if (variant & WKB_HEX)
	{
		int swap =  wkb_swap_bytes(variant);
		/* Machine/request arch mismatch, so flip byte order */
		for (i = 0; i < WKB_DOUBLE_SIZE; i++)
		{
			int j = (swap ? WKB_DOUBLE_SIZE - 1 - i : i);
			uint8_t b = tptr[j];
			/* Top four bits to 0-F */
			buf[2*i] = hexchr[b >> 4];
			/* Bottom four bits to 0-F */
			buf[2*i+1] = hexchr[b & 0x0F];
		}
		return buf + (2 * WKB_DOUBLE_SIZE);
	}
	else
	{
		/* Machine/request arch mismatch, so flip byte order */
		if (wkb_swap_bytes(variant))
		{
			for (i = 0; i < WKB_DOUBLE_SIZE; i++)
			{
				buf[i] = tptr[WKB_DOUBLE_SIZE - 1 - i];
			}
		}
		/* If machine arch and requested arch match, don't flip byte order */
		else
		{
			memcpy(buf, tptr, WKB_DOUBLE_SIZE);
		}
		return buf + WKB_DOUBLE_SIZE;
	}
}

/**
* Convert LWGEOM to a char* in WKB format. Caller is responsible for freeing
* the returned array.
*
* @param variant. Unsigned bitmask value. Accepts one of: WKB_ISO, WKB_EXTENDED, WKB_SFSQL.
* Accepts any of: WKB_NDR, WKB_HEX. For example: Variant = (WKB_ISO | WKB_NDR) would
* return the little-endian ISO form of WKB. For Example: Variant = (WKB_EXTENDED | WKB_HEX)
* would return the big-endian extended form of WKB, as hex-encoded ASCII (the "canonical form").
* @param size_out If supplied, will return the size of the returned memory segment,
* including the null terminator in the case of ASCII.
*/
uint8_t* tpoint_to_wkb(const Temporal *temp, uint8_t variant, size_t *size_out)
{
	size_t buf_size;
	uint8_t *buf = NULL;
	uint8_t *wkb_out = NULL;

	/* Initialize output size */
	if (size_out) *size_out = 0;

	if (temp == NULL)
	{
		elog(ERROR, "Cannot convert NULL into WKB.");
		return NULL;
	}

	/* Calculate the required size of the output buffer */
	buf_size = tpoint_to_wkb_size(temp, variant);

	if (buf_size == 0)
	{
		elog(ERROR, "Error calculating output WKB buffer size.");
		return NULL;
	}

	/* Hex string takes twice as much space as binary + a null character */
	if (variant & WKB_HEX)
	{
		buf_size = 2 * buf_size + 1;
	}

	/* If neither or both variants are specified, choose the native order */
	if (! (variant & WKB_NDR || variant & WKB_XDR) ||
	       (variant & WKB_NDR && variant & WKB_XDR))
	{
		if (getMachineEndian() == NDR)
			variant = variant | WKB_NDR;
		else
			variant = variant | WKB_XDR;
	}

	/* Allocate the buffer */
	buf = palloc(buf_size);

	if (buf == NULL)
	{
		elog(ERROR, "Unable to allocate %d bytes for WKB output buffer.", buf_size);
		return NULL;
	}

	/* Retain a pointer to the front of the buffer for later */
	wkb_out = buf;

	/* Write the WKB into the output buffer */
	buf = tpoint_to_wkb_buf(temp, buf, variant);

	/* Null the last byte if this is a hex output */
	if (variant & WKB_HEX)
	{
		*buf = '\0';
		buf++;
	}

	/* The buffer pointer should now land at the end of the allocated buffer space. Let's check. */
	if (buf_size != (size_t) (buf - wkb_out))
	{
		elog(ERROR, "Output WKB is not the same size as the allocated buffer.");
		pfree(wkb_out);
		return NULL;
	}

	/* Report output size */
	if (size_out) *size_out = buf_size;

	return wkb_out;
}

char *
tpoint_to_hexwkb(const Temporal *temp, uint8_t variant, size_t *size_out)
{
	return (char *)tpoint_to_wkb(temp, variant | WKB_HEX, size_out);
}

/*****************************************************************************/
