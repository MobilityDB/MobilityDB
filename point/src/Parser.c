/*****************************************************************************
 *
 * Parser.c
 *	  Functions for parsing temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"

/*****************************************************************************/

GBOX *
gbox_parse(char **str) 
{
	int hasz = 0, hasm = 0, geodetic = 0;
	p_whitespace(str);
	if (strncasecmp(*str, "GBOX", 4) == 0) 
	{
		*str += 4;
		p_whitespace(str);
		if (strncasecmp(*str, "ZM", 2) == 0)
		{
			hasz = hasm = 1;
			*str += 2;
		}
		else if (strncasecmp(*str, "Z", 1) == 0)
		{
			*str += 1;
			hasz = 1;
		}
		else if (strncasecmp(*str, "M", 1) == 0)
		{
			*str += 1;
			hasm = 1;
		}
		p_whitespace(str);
	}
	else if (strncasecmp(*str, "GEODBOX", 7) == 0) 
	{
		*str += 7;
		hasz = geodetic = 1;
		p_whitespace(str);
	}
	else
		return NULL;

	GBOX *result = gbox_new(gflags(hasz, hasm, geodetic));
	if (!p_oparen(str) || !p_oparen(str))
		return NULL;

	char *nextstr = *str;
	result->xmin = strtod(*str, &nextstr);
	if (*str == nextstr) return NULL; else *str = nextstr; 
	p_whitespace(str);
	p_comma(str);
	p_whitespace(str);
	result->ymin = strtod(*str, &nextstr);
	if (*str == nextstr) return NULL; else *str = nextstr; 
	if (hasz != 0)
	{	
		p_whitespace(str);
		p_comma(str);
		p_whitespace(str);
		result->zmin = strtod(*str, &nextstr);
		if (*str == nextstr) return NULL; else *str = nextstr; 
	}
	if (hasm != 0)
	{	
		p_whitespace(str);
		p_comma(str);
		p_whitespace(str);
		result->mmin = strtod(*str, &nextstr);
		if (*str == nextstr) return NULL; else *str = nextstr; 
	}
	p_whitespace(str);
	if (!p_cparen(str))
		return NULL;
	p_whitespace(str);
	if (!p_comma(str))
		return NULL;
	p_whitespace(str);
	if (!p_oparen(str))
		return NULL;

	result->xmax = strtod(*str, &nextstr);
	if (*str == nextstr) return NULL; else *str = nextstr; 
	p_whitespace(str);
	p_comma(str);
	p_whitespace(str);
	result->ymax = strtod(*str, &nextstr);
	if (*str == nextstr) return NULL; else *str = nextstr; 
	if (hasz != 0)
	{	
		p_whitespace(str);
		p_comma(str);
		p_whitespace(str);
		result->zmax = strtod(*str, &nextstr);
		if (*str == nextstr) return NULL; else *str = nextstr; 
	}
	if (hasm != 0)
	{	
		p_whitespace(str);
		p_comma(str);
		p_whitespace(str);
		result->mmax = strtod(*str, &nextstr);
		if (*str == nextstr) return NULL; else *str = nextstr; 
	}
	p_whitespace(str);
	if (!p_cparen(str) && !p_cparen(str) )
		return NULL;

	return result;
}

/*****************************************************************************/

static TemporalInst *
tpointinst_parse(char **str, Oid basetype, bool end, int *tpoint_srid) 
{
	p_whitespace(str);
	/* The next instruction will throw an exception if it fails */
	Datum geo = p_basetype(str, basetype); 
	GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geo);
	int geom_srid = gserialized_get_srid(gs);
	if ((gserialized_get_type(gs) != POINTTYPE) || 
		gserialized_is_empty(gs))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Only non-empty point geometries accepted")));
	if (*tpoint_srid != 0 && geom_srid != 0 && *tpoint_srid != geom_srid) 
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Geometry SRID (%d) does not match temporal type SRID (%d)", 
			geom_srid, *tpoint_srid)));
	if (*tpoint_srid != 0 && geom_srid == 0)
		gserialized_set_srid(gs, *tpoint_srid);
	if (*tpoint_srid == 0 && geom_srid != 0)
		*tpoint_srid = geom_srid;
	/* The next instruction will throw an exception if it fails */
	TimestampTz t = timestamp_parse(str);
	TemporalInst *result = temporalinst_make(PointerGetDatum(gs), t, basetype);
	pfree(gs);
	return result;
}

static TemporalI *
tpointi_parse(char **str, Oid basetype, int *tpoint_srid) 
{
	p_whitespace(str);
	if (!p_obrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));

	//FIXME: parsing twice
	char *bak = *str;
	TemporalInst *inst = tpointinst_parse(str, basetype, false, tpoint_srid);
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
	TemporalInst **insts = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		insts[i] = tpointinst_parse(str, basetype, false, tpoint_srid);
	}
	p_cbrace(str);
	TemporalI *result = temporali_from_temporalinstarr(insts, count);

	for (int i = 0; i < count; i++)
		pfree(insts[i]);
	pfree(insts);

	return result;
}

static TemporalSeq *
tpointseq_parse(char **str, Oid basetype, bool end, int *tpoint_srid) 
{
	p_whitespace(str);
	bool lower_inc = false, upper_inc = false;
	if (p_obracket(str))
		lower_inc = true;
	else if (p_oparen(str))
		lower_inc = false;
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));

	// FIXME: I pre-parse to have the count, then re-parse. This is the only
	// approach I see at the moment which is both correct and simple
	char *bak = *str;
	TemporalInst *inst = tpointinst_parse(str, basetype, false, tpoint_srid);
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
	TemporalInst **insts = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		insts[i] = tpointinst_parse(str, basetype, false, tpoint_srid);
	}

	p_cbracket(str);
	p_cparen(str);

	TemporalSeq *result = temporalseq_from_temporalinstarr(insts, 
		count, lower_inc, upper_inc, true);

	for (int i = 0; i < count; i++)
		pfree(insts[i]);
	pfree(insts);

	return result;
}

static TemporalS *
tpoints_parse(char **str, Oid basetype, int *tpoint_srid) 
{
	p_whitespace(str);
	if (!p_obrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));

	//FIXME: parsing twice
	char *bak = *str;
	TemporalSeq *seq = tpointseq_parse(str, basetype, false, tpoint_srid);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		pfree(seq);
		seq = tpointseq_parse(str, basetype, false, tpoint_srid);
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
	TemporalSeq **seqs = palloc(sizeof(TemporalSeq *) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		seqs[i] = tpointseq_parse(str, basetype, false, tpoint_srid);
	}
	p_cbrace(str);
	TemporalS *result = temporals_from_temporalseqarr(seqs, count, true);

	for (int i = 0; i < count; i++)
		pfree(seqs[i]);
	pfree(seqs);

	return result;
}

Temporal *
tpoint_parse(char **str, Oid basetype) 
{
	int tpoint_srid = 0;
	p_whitespace(str);
	
	/* Starts with "SRID=" */
	if (strncasecmp(*str,"SRID=",5) == 0)
	{
		/* Move str to the start of the numeric part */
		*str += 5;
		int delim = 0;
		while ((*str)[delim] != ';' && (*str)[delim] != '\0')
			delim++;
		/* Null terminate the SRID= string */
		(*str)[delim] = '\0';
		/* Parse out the SRID number */
		tpoint_srid = atoi(*str);
		/* Set str to the start of the temporal point */
		*str += delim + 1;
	}
	
	/* Determine the type of the temporal point */
	if (**str != '{' && **str != '[' && **str != '(')
		return (Temporal *)tpointinst_parse(str, basetype, true, &tpoint_srid);
	else if (**str == '[' || **str == '(')
		return (Temporal *)tpointseq_parse(str, basetype, true, &tpoint_srid);		
	else if (**str == '{')
	{
		char *bak = *str;
		p_obrace(str);
		p_whitespace(str);
		if (**str == '[' || **str == '(')
		{
			*str = bak;
			return (Temporal *)tpoints_parse(str, basetype, &tpoint_srid);
		}
		else
		{
			*str = bak;
			return (Temporal *)tpointi_parse(str, basetype, &tpoint_srid);		
		}
	}
	return NULL; /* keep compiler quiet */
}

/*****************************************************************************/
