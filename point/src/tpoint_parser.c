/*****************************************************************************
 *
 * tpoint_parser.c
 *	  Functions for parsing temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_parser.h"

#include "temporaltypes.h"
#include "oidcache.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"
#include "temporal_parser.h"
#include "stbox.h"

/*****************************************************************************/

/**
 * Parse a spatiotemporal box value from the buffer
 */
STBOX *
stbox_parse(char **str) 
{
	double xmin, xmax, ymin, ymax, 
		zmin = 0, zmax = 0; /* make Codacy quiet */
	TimestampTz tmin, tmax, ttmp;
	bool hasx = false, hasz = false, hast = false, geodetic = false;
	char *nextstr;
	int srid = 0;
	bool hassrid = false;

	p_whitespace(str);
	if (strncasecmp(*str,"SRID=",5) == 0)
	{
		/* Move str to the start of the numeric part */
		*str += 5;
		int delim = 0;
		/* Delimiter will be either ',' or ';' depending on whether interpolation
		   is given after */
		while ((*str)[delim] != ',' && (*str)[delim] != ';' && (*str)[delim] != '\0')
		{
			srid = srid * 10 + (*str)[delim] - '0';
			delim++;
		}
		/* Set str to the start of the temporal point */
		*str += delim + 1;
		hassrid = true;
	}
	if (strncasecmp(*str, "STBOX", 5) == 0)
	{
		*str += 5;
		p_whitespace(str);
		if (strncasecmp(*str, "ZT", 2) == 0)
		{
			hasz = hast = true;
			*str += 2;
		}
		else if (strncasecmp(*str, "Z", 1) == 0)
		{
			*str += 1;
			hasz = true;
		}
		else if (strncasecmp(*str, "T", 1) == 0)
		{
			*str += 1;
			hast = true;
		}
		p_whitespace(str);
	}
	else if (strncasecmp(*str, "GEODSTBOX", 9) == 0)
	{
		*str += 9;
		hasz = geodetic = true;
		p_whitespace(str);
		if (strncasecmp(*str, "T", 1) == 0)
		{
			*str += 1;
			hast = true;
		}
		p_whitespace(str);
		if (!hassrid)
			srid = 4326;
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse STBOX")));

	/* Parse double opening parenthesis */
	if (!p_oparen(str) || !p_oparen(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse STBOX: Missing opening parenthesis")));

	/* Determine whether there is an XY(Z) dimension */
	p_whitespace(str);
	if (((*str)[0]) != ',')
		hasx = true;

	if (!hasx && !hast)
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse STBOX")));
	if (!hasx && hassrid)
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
			errmsg("An SRID is specified but not coordinates are given")));

	if (hasx)
	{
		/* xmin */
		nextstr = *str;
		xmin = strtod(*str, &nextstr);
		if (*str == nextstr)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse STBOX: Invalid input syntax for type double")));
		*str = nextstr; 
		/* ymin */
		p_whitespace(str);
		p_comma(str);
		p_whitespace(str);
		ymin = strtod(*str, &nextstr);
		if (*str == nextstr)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse STBOX: Invalid input syntax for type double")));
		*str = nextstr; 
		if (hasz)
		{	
			p_whitespace(str);
			p_comma(str);
			p_whitespace(str);
			zmin = strtod(*str, &nextstr);
			if (*str == nextstr)
				ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
					errmsg("Could not parse STBOX: Invalid input syntax for type double")));
			*str = nextstr; 
		}
	}
	else
	{
		/* Empty XY(Z) dimension */
		p_whitespace(str);
		p_comma(str);
		p_whitespace(str);
		p_comma(str);
	}
	if (hast)
	{	
		p_whitespace(str);
		p_comma(str);
		p_whitespace(str);
		nextstr = *str;
		tmin = timestamp_parse(&nextstr);
		if (*str == nextstr)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse STBOX")));
		*str = nextstr; 
	}
	p_whitespace(str);
	if (!p_cparen(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse STBOX: Missing closing parenthesis")));
	p_whitespace(str);
	p_comma(str);
	p_whitespace(str);

	/* Parse upper bounds */
	if (!p_oparen(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse STBOX: Missing opening parenthesis")));

	if (hasx)
	{
		xmax = strtod(*str, &nextstr);
		if (*str == nextstr)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse STBOX: Invalid input syntax for type double")));
		*str = nextstr; 
		p_whitespace(str);
		p_comma(str);
		p_whitespace(str);
		ymax = strtod(*str, &nextstr);
		if (*str == nextstr)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse STBOX: Invalid input syntax for type double")));
		*str = nextstr; 
		if (hasz)
		{	
			p_whitespace(str);
			p_comma(str);
			p_whitespace(str);
			zmax = strtod(*str, &nextstr);
			if (*str == nextstr)
				ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
					errmsg("Could not parse STBOX: Invalid input syntax for type double")));
			*str = nextstr; 
		}
	}
	else
	{
		/* Empty XY dimensions */
		p_whitespace(str);
		p_comma(str);
		p_whitespace(str);
		p_comma(str);
	}
	if (hast)
	{	
		p_whitespace(str);
		p_comma(str);
		nextstr = *str;
		tmax = timestamp_parse(&nextstr);
		if (*str == nextstr)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse STBOX")));
		*str = nextstr; 
	}
	p_whitespace(str);
	if (!p_cparen(str) || !p_cparen(str) )
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse STBOX: Missing closing parenthesis")));
	
	STBOX *result = stbox_new(hasx, hasz, hast, geodetic, srid);
	if (hasx)
	{
		double tmp;
		if (xmin > xmax)
		{
			tmp = xmin;
			xmin = xmax;
			xmax = tmp;
		}
		if (ymin > ymax)
		{
			tmp = ymin;
			ymin = ymax;
			ymax = tmp;
		}
		result->xmin = xmin;
		result->xmax = xmax;
		result->ymin = ymin;
		result->ymax = ymax;
		if (hasz)
		{
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

/*****************************************************************************/

/**
 * Parse a temporal point value of instant duration from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in] tpoint_srid SRID of the temporal point
 */
static TInstant *
tpointinst_parse(char **str, Oid basetype, bool end, int *tpoint_srid) 
{
	p_whitespace(str);
	/* The next instruction will throw an exception if it fails */
	Datum geo = basetype_parse(str, basetype); 
	GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geo);
	int geo_srid = gserialized_get_srid(gs);
	ensure_point_type(gs);
	ensure_non_empty(gs);
	ensure_has_not_M_gs(gs);
	if (*tpoint_srid != SRID_UNKNOWN && geo_srid != SRID_UNKNOWN && *tpoint_srid != geo_srid)
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Geometry SRID (%d) does not match temporal type SRID (%d)", 
			geo_srid, *tpoint_srid)));
	if (basetype == type_oid(T_GEOMETRY))
	{
		if (*tpoint_srid != SRID_UNKNOWN && geo_srid == SRID_UNKNOWN)
			gserialized_set_srid(gs, *tpoint_srid);
		if (*tpoint_srid == SRID_UNKNOWN && geo_srid != SRID_UNKNOWN)
			*tpoint_srid = geo_srid;
	}
	else
	{
		if (*tpoint_srid != SRID_UNKNOWN && geo_srid == SRID_DEFAULT)
			gserialized_set_srid(gs, *tpoint_srid);
		if (*tpoint_srid == SRID_UNKNOWN && geo_srid != SRID_DEFAULT)
			*tpoint_srid = geo_srid;
	}
	/* The next instruction will throw an exception if it fails */
	TimestampTz t = timestamp_parse(str);
	if (end)
	{
		/* Ensure there is no more input */
		p_whitespace(str);
		if (**str != 0)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse temporal value")));
	}
	TInstant *result = tinstant_make(PointerGetDatum(gs), t, basetype);
	pfree(gs);
	return result;
}

/**
 * Parse a temporal point value of instant set duration from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 * @param[in] tpoint_srid SRID of the temporal point
 */
static TInstantSet *
tpointinstset_parse(char **str, Oid basetype, int *tpoint_srid) 
{
	p_whitespace(str);
	/* We are sure to find an opening brace because that was the condition 
	 * to call this function in the dispatch function tpoint_parse */
	p_obrace(str);

	/* First parsing */
	char *bak = *str;
	TInstant *inst = tpointinst_parse(str, basetype, false, tpoint_srid);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		pfree(inst);
		inst = tpointinst_parse(str, basetype, false, tpoint_srid);
	}
	pfree(inst);
	if (!p_cbrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));
	/* Ensure there is no more input */
	p_whitespace(str);
	if (**str != 0)
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));
	/* Second parsing */
	*str = bak;
	TInstant **insts = palloc(sizeof(TInstant *) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		insts[i] = tpointinst_parse(str, basetype, false, tpoint_srid);
	}
	p_cbrace(str);
	TInstantSet *result = tinstantset_make(insts, count);

	for (int i = 0; i < count; i++)
		pfree(insts[i]);
	pfree(insts);

	return result;
}

/**
 * Parse a temporal point value of sequence duration from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 * @param[in] linear Set to true when the sequence set has linear interpolation
 * @param[in] end Set to true when reading a single instant to ensure there is
 * no moreinput after the sequence
 * @param[in] tpoint_srid SRID of the temporal point
*/
static TSequence *
tpointseq_parse(char **str, Oid basetype, bool linear, bool end, int *tpoint_srid) 
{
	p_whitespace(str);
	bool lower_inc = false, upper_inc = false;
	/* We are sure to find an opening bracket or parenthesis because that was 
	 * the condition to call this function in the dispatch function tpoint_parse */
	if (p_obracket(str))
		lower_inc = true;
	else if (p_oparen(str))
		lower_inc = false;

	/* First parsing */
	char *bak = *str;
	TInstant *inst = tpointinst_parse(str, basetype, false, tpoint_srid);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		pfree(inst);
		inst = tpointinst_parse(str, basetype, false, tpoint_srid);
	}
	pfree(inst);
	if (p_cbracket(str))
		upper_inc = true;
	else if (p_cparen(str))
		upper_inc = false;
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));
	if (end)
	{
		/* Ensure there is no more input */
		p_whitespace(str);
		if (**str != 0)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse temporal value")));
	}
	/* Second parsing */
	*str = bak; 
	TInstant **insts = palloc(sizeof(TInstant *) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		insts[i] = tpointinst_parse(str, basetype, false, tpoint_srid);
	}

	p_cbracket(str);
	p_cparen(str);

	TSequence *result = tsequence_make(insts, count, lower_inc, upper_inc,
		linear, true);

	for (int i = 0; i < count; i++)
		pfree(insts[i]);
	pfree(insts);

	return result;
}

/**
 * Parse a temporal point value of sequence set duration from the buffer
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 * @param[in] linear Set to true when the sequence set has linear interpolation
 * @param[in] tpoint_srid SRID of the temporal point
 */
static TSequenceSet *
tpointseqset_parse(char **str, Oid basetype, bool linear, int *tpoint_srid) 
{
	p_whitespace(str);
	/* We are sure to find an opening brace because that was the condition 
	 * to call this function in the dispatch function tpoint_parse */
	p_obrace(str);

	/* First parsing */
	char *bak = *str;
	TSequence *seq = tpointseq_parse(str, basetype, linear, false, tpoint_srid);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		pfree(seq);
		seq = tpointseq_parse(str, basetype, linear, false, tpoint_srid);
	}
	pfree(seq);
	if (!p_cbrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));
	/* Ensure there is no more input */
	p_whitespace(str);
	if (**str != 0)
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));
	/* Second parsing */
	*str = bak;
	TSequence **sequences = palloc(sizeof(TSequence *) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		sequences[i] = tpointseq_parse(str, basetype, linear, false, tpoint_srid);
	}
	p_cbrace(str);
	return tsequenceset_make_free(sequences, count, true);
}

/**
 * Parse a temporal point value from the buffer (dispatch function)
 *
 * @param[in] str Input string
 * @param[in] basetype Oid of the base type
 */
Temporal *
tpoint_parse(char **str, Oid basetype) 
{
	int tpoint_srid = 0;
	p_whitespace(str);
	
	/* Starts with "SRID=". The SRID specification must be gobbled for all 
	 * durations excepted TInstant. We cannot use the atoi() function
	 * because this requires a string terminated by '\0' and we cannot 
	 * modify the string in case it must be passed to the tpointinst_parse
	 * function. */
	char *bak = *str;
	if (strncasecmp(*str,"SRID=",5) == 0)
	{
		/* Move str to the start of the numeric part */
		*str += 5;
		int delim = 0;
		tpoint_srid = 0;
		/* Delimiter will be either ',' or ';' depending on whether interpolation 
		   is given after */
		while ((*str)[delim] != ',' && (*str)[delim] != ';' && (*str)[delim] != '\0')
		{
			tpoint_srid = tpoint_srid * 10 + (*str)[delim] - '0'; 
			delim++;
		}
		/* Set str to the start of the temporal point */
		*str += delim + 1;
	}
	/* We cannot ensure that the SRID is geodetic for geography since
	 * the srid_is_latlong function is not exported by PostGIS
	if (basetype == type_oid(T_GEOGRAPHY))
		srid_is_latlong(fcinfo, tpoint_srid);
	 */	

	bool linear = linear_interpolation(basetype);
	/* Starts with "Interp=Stepwise" */
	if (strncasecmp(*str,"Interp=Stepwise;",16) == 0)
	{
		/* Move str after the semicolon */
		*str += 16;
		linear = false;
	}
	Temporal *result = NULL; /* keep compiler quiet */
	/* Determine the type of the temporal point */
	if (**str != '{' && **str != '[' && **str != '(')
	{
		/* Pass the SRID specification */
		*str = bak;
		result = (Temporal *)tpointinst_parse(str, basetype, true, &tpoint_srid);
	}
	else if (**str == '[' || **str == '(')
		result = (Temporal *)tpointseq_parse(str, basetype, linear, true, &tpoint_srid);		
	else if (**str == '{')
	{
		bak = *str;
		p_obrace(str);
		p_whitespace(str);
		if (**str == '[' || **str == '(')
		{
			*str = bak;
			result = (Temporal *)tpointseqset_parse(str, basetype, linear, &tpoint_srid);
		}
		else
		{
			*str = bak;
			result = (Temporal *)tpointinstset_parse(str, basetype, &tpoint_srid);		
		}
	}
	return result;
}

/*****************************************************************************/
