/*****************************************************************************
 *
 * mfjson.c
 *	  MF-JSON input/output
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"
#include "executor/spi.h"

#define FP_TOLERANCE 1e-12

#define OUT_MAX_DOUBLE 1E15
#define OUT_SHOW_DIGS_DOUBLE 20
#define OUT_MAX_DOUBLE_PRECISION 15
#define OUT_MAX_DIGS_DOUBLE (OUT_SHOW_DIGS_DOUBLE + 2) /* +2 mean add dot and sign */
#define OUT_DOUBLE_BUFFER_SIZE \
	OUT_MAX_DIGS_DOUBLE + OUT_MAX_DOUBLE_PRECISION + 1

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/*
 * Removes trailing zeros and dot for a %f formatted number.
 * Modifies input.
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
 */
int
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
 *
 * Could return SRS as short one (i.e EPSG:4326)
 * or as long one: (i.e urn:ogc:def:crs:EPSG::4326)
 */
char *
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
	if ( err < 0 )
	{
		elog(NOTICE, "getSRSbySRID: error executing query %d", err);
		SPI_finish();
		return NULL;
	}

	/* no entry in spatial_ref_sys */
	if (SPI_processed <= 0)
	{
		SPI_finish();
		return NULL;
	}

	/* get result  */
	srs = SPI_getvalue(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1);

	/* NULL result */
	if ( ! srs )
	{
		SPI_finish();
		return NULL;
	}

	/* copy result to upper executor context */
	size = strlen(srs)+1;
	srscopy = SPI_palloc(size);
	memcpy(srscopy, srs, size);

	/* disconnect from SPI */
	SPI_finish();

	return srscopy;
}

/*
 * Handle coordinate array
 */
/*
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
coordinates_mfjson_buf(char *output, TemporalInst **instants, int count, int precision)
{
	char *ptr;
	char x[OUT_DOUBLE_BUFFER_SIZE];
	char y[OUT_DOUBLE_BUFFER_SIZE];
	char z[OUT_DOUBLE_BUFFER_SIZE];

	assert (precision <= OUT_MAX_DOUBLE_PRECISION);
	ptr = output;

	/* TODO: rewrite this loop to be simpler and possibly quicker */
	if (!MOBDB_FLAGS_GET_Z(instants[0]->flags))
	{
		for (int i = 0; i < count; i++)
		{
			POINT2D pt = datum_get_point2d(temporalinst_value(instants[i]));

			lwprint_double(
			    pt.x, precision, x, OUT_DOUBLE_BUFFER_SIZE);
			lwprint_double(
			    pt.y, precision, y, OUT_DOUBLE_BUFFER_SIZE);

			if ( i ) ptr += sprintf(ptr, ",");
			ptr += sprintf(ptr, "[%s,%s]", x, y);
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			POINT3DZ pt = datum_get_point3dz(temporalinst_value(instants[i]));

			lwprint_double(
			    pt.x, precision, x, OUT_DOUBLE_BUFFER_SIZE);
			lwprint_double(
			    pt.y, precision, y, OUT_DOUBLE_BUFFER_SIZE);
			lwprint_double(
			    pt.z, precision, z, OUT_DOUBLE_BUFFER_SIZE);

			if (i) ptr += sprintf(ptr, ",");
			ptr += sprintf(ptr, "[%s,%s,%s]", x, y, z);
		}
	}

	return (ptr - output);
}

/*
 * Handle datetimes array
 */
/*
 * Returns maximum size of datetimes array in bytes.
 * Example:   "datetimes":["2019-08-06T18:35:48.021455+02","2019-08-06 18:45:18.476983+02"],
 */
static size_t
datetimes_mfjson_size(int npoints)
{
	return 32 * npoints + sizeof("[]");
}

static size_t
datetimes_mfjson_buf(char *output, TemporalInst **instants, int count)
{
	char *ptr = output;

	/* TODO: rewrite this loop to be simpler and possibly quicker */
    for (int i = 0; i < count; i++)
    {
		char *t = call_output(TIMESTAMPTZOID, instants[i]->t);
        if (i) ptr += sprintf(ptr, ",");
        ptr += sprintf(ptr, "\"%s\"", t);
    }

	return (ptr - output);
}

/*
 * Handle SRS
 */
static size_t
asmfjson_srs_size(char *srs)
{
	int size;

	size = sizeof("'crs':{'type':'name',");
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

	return (ptr-output);
}

/*
 * Handle Bbox
 */
static size_t
asmfjson_bbox_size(int hasz, int precision)
{
	int size;

	if (!hasz)
	{
		size = sizeof("\"bbox\":[,,,],");
		size +=	2 * 2 * (OUT_MAX_DIGS_DOUBLE + precision);
	}
	else
	{
		size = sizeof("\"bbox\":[,,,,,],");
		size +=	2 * 3 * (OUT_MAX_DIGS_DOUBLE + precision);
	}

	return size;
}

static size_t
asmfjson_bbox_buf(char *output, STBOX *bbox, int hasz, int precision)
{
	char *ptr = output;

	if (!hasz)
		ptr += sprintf(ptr, "\"bbox\":[%.*f,%.*f,%.*f,%.*f],",
		               precision, bbox->xmin, precision, bbox->ymin,
		               precision, bbox->xmax, precision, bbox->ymax);
	else
		ptr += sprintf(ptr, "\"bbox\":[%.*f,%.*f,%.*f,%.*f,%.*f,%.*f],",
		               precision, bbox->xmin, precision, bbox->ymin, precision, bbox->zmin,
		               precision, bbox->xmax, precision, bbox->ymax, precision, bbox->zmax);

	return (ptr - output);
}

static size_t
tpointinst_asmfjson_size(const TemporalInst *inst, int precision, STBOX *bbox, char *srs)
{
	int size = coordinates_mfjson_size(1, MOBDB_FLAGS_GET_Z(inst->flags), precision);
	size += datetimes_mfjson_size(1);
	size += sizeof("{'type':'MovingPoint',");
	size += sizeof("'coordinates':,'datetimes':}");

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
	ptr += coordinates_mfjson_buf(ptr, &inst, 1, precision);
	ptr += sprintf(ptr, ",\"datetimes\":");
	ptr += datetimes_mfjson_buf(ptr, &inst, 1);
	ptr += sprintf(ptr, "}");

	return (ptr - output);
}

static char *
tpointinst_asmfjson(TemporalInst *inst, int precision, int has_bbox, char *srs)
{
	STBOX *bbox = NULL;
	STBOX tmp;
	if (has_bbox)
	{
        tpointinst_make_stbox(&tmp, temporalinst_value(inst), inst->t);
		bbox = &tmp;
	}
    
    int size = tpointinst_asmfjson_size(inst, precision, bbox, srs);
	char *output = palloc(size);
	tpointinst_asmfjson_buf(inst, precision, bbox, srs, output);
	return output;
}

static char *
tpointi_asmfjson(TemporalI *ti, int precision, int has_bbox, char *srs)
{
    return NULL;
/*    
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	Datum length = Float8GetDatum(0.0);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = temporalinst_make(length, inst->t, FLOAT8OID);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);
	for (int i = 1; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
*/
}

static char *
tpointseq_asmfjson(TemporalSeq *seq, int precision, int has_bbox, char *srs)
{
    return NULL;
/*    
	/ * Instantaneous sequence * /
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
			Datum line = geompoint_trajectory(value1, value2);
			/ * The next function works for 2D and 3D * /
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
*/
}

static char *
tpoints_asmfjson(TemporalS *ts, int precision, int has_bbox, char *srs)
{
    return NULL;
/*    
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	double length = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tpointseq_asmfjson(seq, length);
		TemporalInst *end = temporalseq_inst_n(sequences[i], seq->count - 1);
		length += DatumGetFloat8(temporalinst_value(end));
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, 
		ts->count, false);
		
	for (int i = 1; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
*/
}

PG_FUNCTION_INFO_V1(tpoint_asmfjson);

PGDLLEXPORT Datum
tpoint_asmfjson(PG_FUNCTION_ARGS)
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
					elog(ERROR,
					      "SRID %i unknown in spatial_ref_sys table",
					      srid);
					PG_RETURN_NULL();
				}
			}
		}

		if (option & 1)
			has_bbox = 1;
	}

	char *mfjson = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST)
		mfjson = tpointinst_asmfjson((TemporalInst *)temp, precision, has_bbox, srs);
	else if (temp->duration == TEMPORALI)
		mfjson = tpointi_asmfjson((TemporalI *)temp, precision, has_bbox, srs);
	else if (temp->duration == TEMPORALSEQ)
		mfjson = tpointseq_asmfjson((TemporalSeq *)temp, precision, has_bbox, srs);
	else if (temp->duration == TEMPORALS)
		mfjson = tpoints_asmfjson((TemporalS *)temp, precision, has_bbox, srs);
	text *result = cstring_to_text(mfjson);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TEXT_P(result);
}

