/*****************************************************************************
 *
 * temporal_parser.c
 *	  Functions for parsing time types and temporal types.
 *
 * Many functions make two passes for parsing, the first one to obtain the 
 * number of elements in order to do memory allocation with palloc, the second
 * one to create the type. This is the only approach we can see at the moment
 * which is both correct and simple.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_parser.h"

#include "periodset.h"
#include "period.h"
#include "timestampset.h"
#include "temporaltypes.h"
#include "temporal_util.h"


/*****************************************************************************/

void 
p_whitespace(char **str) 
{
	while (**str == ' ' || **str == '\n' || **str == '\r' || **str == '\t')
		*str += 1;
}

bool 
p_obrace(char **str)
{
	p_whitespace(str);
	if (**str == '{') 
	{
		*str += 1;
		return true;
	}
	return false;
}

bool 
p_cbrace(char **str)
{
	p_whitespace(str);
	if (**str == '}') 
	{
		*str += 1;
		return true;
	}
	return false;
}

bool 
p_obracket(char **str)
{
	p_whitespace(str);
	if (**str == '[') 
	{
		*str += 1;
		return true;
	}
	return false;
}

bool 
p_cbracket(char **str)
{
	p_whitespace(str);
	if (**str == ']') 
	{
		*str += 1;
		return true;
	}
	return false;
}

bool 
p_oparen(char **str)
{
	p_whitespace(str);
	if (**str == '(') 
	{
		*str += 1;
		return true;
	}
	return false;
}

bool 
p_cparen(char **str)
{
	p_whitespace(str);
	if (**str == ')') 
	{
		*str += 1;
		return true;
	}
	return false;
}

bool 
p_comma(char **str)
{
	p_whitespace(str);
	if (**str == ',') 
	{
		*str += 1;
		return true;
	}
	return false;
}

Datum 
basetype_parse(char **str, Oid basetype)
{
	p_whitespace(str);
	int delim = 0;
	bool isttext = false;
	/* ttext values must be enclosed between double quotes */
	if (**str == '"')
	{
		isttext = true;
		/* Consume the double quote */
		*str += 1;
		while ( ( (*str)[delim] != '"' || (*str)[delim - 1] == '\\' )  && 
			(*str)[delim] != '\0' )
			delim++;
	}
	else
	{
		while ((*str)[delim] != '@' && (*str)[delim] != '\0')
			delim++;
	}
	if ((*str)[delim] == '\0')
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse element value")));
	(*str)[delim] = '\0';
	Datum result = call_input(basetype, *str);
	if (isttext)
		/* Replace the double quote */
		(*str)[delim++] = '"';
	else
		/* Replace the at */
		(*str)[delim] = '@';
	/* since we know there's an @ here, let's take it with us */
	*str += delim + 1; 
	return result;
}

/*****************************************************************************/

TBOX *
tbox_parse(char **str) 
{
	double xmin, xmax, tmp;
	TimestampTz tmin, tmax, ttmp;
	bool hasx = false, hast = false;
	char *nextstr;

	p_whitespace(str);
	if (strncasecmp(*str, "TBOX", 4) == 0) 
	{
		*str += 4;
		p_whitespace(str);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse TBOX")));

	/* Parse double opening parenthesis */
	if (!p_oparen(str) || !p_oparen(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse TBOX: Missing opening parenthesis")));

	/* Determine whether there is an X dimension */
	p_whitespace(str);
	if (((*str)[0]) != ',')
	{
		nextstr = *str;
		xmin = strtod(*str, &nextstr);
		if (*str == nextstr)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse TBOX: Invalid input syntax for type double")));
		hasx = true;
		*str = nextstr; 
	}

	p_whitespace(str);
	p_comma(str);
	p_whitespace(str);		

	/* Determine whether there is a T dimension */
	if (((*str)[0]) != ')')
	{
		nextstr = *str;
		tmin = timestamp_parse(&nextstr);
		if (*str != nextstr)
		{
			hast = true;
			*str = nextstr; 
		}
	}

	if (! hasx && ! hast)
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse TBOX")));

	p_whitespace(str);
	if (!p_cparen(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse TBOX: Missing closing parenthesis")));
	p_whitespace(str);
	p_comma(str);
	p_whitespace(str);		

	/* Parse upper bounds */
	if (!p_oparen(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse TBOX: Missing opening parenthesis")));

	if (hasx)
	{
		xmax = strtod(*str, &nextstr);
		if (*str == nextstr)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse TBOX: Invalid input syntax for type double")));
		*str = nextstr; 
	}
	p_whitespace(str);
	p_comma(str);
	p_whitespace(str);
	if (hast)
	{	
		nextstr = *str;
		tmax = timestamp_parse(&nextstr);
		if (*str == nextstr)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse TBOX")));
		*str = nextstr; 
	}
	p_whitespace(str);
	if (!p_cparen(str) || !p_cparen(str) )
	ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse TBOX: Missing closing parenthesis")));

	TBOX *result = palloc0(sizeof(TBOX));
	MOBDB_FLAGS_SET_X(result->flags, hasx);
	MOBDB_FLAGS_SET_T(result->flags, hast);
	if (hasx)
	{
		if (xmin > xmax)
		{
			tmp = xmin;
			xmin = xmax;
			xmax = tmp;
		}
		result->xmin = xmin;
		result->xmax = xmax;
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
/* Time Types */

TimestampTz 
timestamp_parse(char **str) 
{
	p_whitespace(str);
	int delim = 0;
	while ((*str)[delim] != ',' && (*str)[delim] != ']' && (*str)[delim] != ')' && 
		(*str)[delim] != '}' && (*str)[delim] != '\0')
		delim++;
	char bak = (*str)[delim];
	(*str)[delim] = '\0';
	Datum result = call_input(TIMESTAMPTZOID, *str);
	(*str)[delim] = bak;
	*str += delim;
	return result;
}

Period *
period_parse(char **str, bool make) 
{
	bool lower_inc = false, upper_inc = false;
	if (p_obracket(str))
		lower_inc = true;
	else if (p_oparen(str))
		lower_inc = false;
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse period")));

	TimestampTz lower = timestamp_parse(str);
	p_comma(str);
	TimestampTz upper = timestamp_parse(str);

	if (p_cbracket(str))
		upper_inc = true;
	else if (p_cparen(str))
		upper_inc = false;
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse period")));

	if (! make)
		return NULL;
	return period_make(lower, upper, lower_inc, upper_inc);
}

TimestampSet *
timestampset_parse(char **str) 
{
	if (!p_obrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse timestamp set")));

	/* First parsing */
	char *bak = *str;
	timestamp_parse(str);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		timestamp_parse(str);
	}
	if (!p_cbrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse timestamp set")));

	*str = bak;
	TimestampTz *times = palloc(sizeof(TimestampTz) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		times[i] = timestamp_parse(str);
	}
	p_cbrace(str);
	TimestampSet *result = timestampset_make_internal(times, count);

	pfree(times);

	return result;
}

PeriodSet *
periodset_parse(char **str) 
{
	if (!p_obrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse period set")));

	/* First parsing */
	char *bak = *str;
	period_parse(str, false);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		period_parse(str, false);
	}
	if (!p_cbrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse period set")));

	/* Second parsing */
	*str = bak;
	Period **periods = palloc(sizeof(Period *) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		periods[i] = period_parse(str, true);
	}
	p_cbrace(str);
	PeriodSet *result = periodset_make_internal(periods, count, true);

	for (int i = 0; i < count; i++)
		pfree(periods[i]);
	pfree(periods);

	return result;
}

/*****************************************************************************/
/* Temporal Types */

/* Arguments:
 * str: input string
 * basetype: Oid of the base type
 * end: set to true when reading a single instant to ensure there is no more
 * 		input after the instant
 * make: set to false for the first pass to do not create the instant */
TemporalInst *
temporalinst_parse(char **str, Oid basetype, bool end, bool make) 
{
	p_whitespace(str);
	/* The next two instructions will throw an exception if they fail */
	Datum elem = basetype_parse(str, basetype);
	TimestampTz t = timestamp_parse(str);
	if (end)
	{
		/* Ensure there is no more input */
		p_whitespace(str);
		if (**str != 0)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse temporal value")));
	}
	if (! make)
		return NULL;
	return temporalinst_make(elem, t, basetype);
}

/* Arguments:
 * str: input string
 * basetype: Oid of the base type */
static TemporalI *
temporali_parse(char **str, Oid basetype) 
{
	p_whitespace(str);
	/* We are sure to find an opening brace because that was the condition 
	 * to call this function in the dispatch function temporal_parse */
	p_obrace(str);

	/* First parsing */
	char *bak = *str;
	temporalinst_parse(str, basetype, false, false);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		temporalinst_parse(str, basetype, false, false);
	}
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
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		instants[i] = temporalinst_parse(str, basetype, false, true);
	}
	p_cbrace(str);
	TemporalI *result = temporali_make(instants, count);

	for (int i = 0; i < count; i++)
		pfree(instants[i]);
	pfree(instants);

	return result;
}

/* Arguments:
 * str: input string
 * basetype: Oid of the base type
 * linear: set to true when the sequence has linear interpolation
 * end: set to true when reading a single instant to ensure there is no more
 * 		input after the sequence
 * make: set to false for the first pass to do not create the instant */
static TemporalSeq *
temporalseq_parse(char **str, Oid basetype, bool linear, bool end, bool make) 
{
	p_whitespace(str);
	bool lower_inc = false, upper_inc = false;
	/* We are sure to find an opening bracket or parenthesis because that was the
	 * condition to call this function in the dispatch function temporal_parse */
	if (p_obracket(str))
		lower_inc = true;
	else if (p_oparen(str))
		lower_inc = false;

	/* First parsing */
	char *bak = *str;
	temporalinst_parse(str, basetype, false, false);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		temporalinst_parse(str, basetype, false, false);
	}
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
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		instants[i] = temporalinst_parse(str, basetype, false, true);
	}
	p_cbracket(str);
	p_cparen(str);

	if (! make)
		return NULL;

	TemporalSeq *result = temporalseq_make(instants, 
		count, lower_inc, upper_inc, linear, true);

	for (int i = 0; i < count; i++)
		pfree(instants[i]);
	pfree(instants);

	return result;
}

/* Arguments:
 * str: input string
 * basetype: Oid of the base type
 * linear: set to true when the sequence set has linear interpolation */
static TemporalS *
temporals_parse(char **str, Oid basetype, bool linear) 
{
	p_whitespace(str);
	/* We are sure to find an opening brace because that was the condition 
	 * to call this function in the dispatch function temporal_parse */
	p_obrace(str);

	/* First parsing */
	char *bak = *str;
	temporalseq_parse(str, basetype, linear, false, false);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		temporalseq_parse(str, basetype, linear, false, false);
	}
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
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		sequences[i] = temporalseq_parse(str, basetype, linear, false, true);
	}
	p_cbrace(str);
	TemporalS *result = temporals_make(sequences, count,
		linear, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

/* Arguments:
 * str: input string
 * basetype: Oid of the base type */
Temporal *
temporal_parse(char **str, Oid basetype) 
{
	p_whitespace(str);
	Temporal *result = NULL;  /* keep compiler quiet */
	bool linear = linear_interpolation(basetype);
	/* Starts with "Interp=Stepwise" */
	if (strncasecmp(*str,"Interp=Stepwise;", 16) == 0)
	{
		/* Move str after the semicolon */
		*str += 16;
		linear = false;
	}
	if (**str != '{' && **str != '[' && **str != '(')
		result = (Temporal *)temporalinst_parse(str, basetype, true, true);
	else if (**str == '[' || **str == '(')
		result = (Temporal *)temporalseq_parse(str, basetype, linear, true, true);		
	else if (**str == '{')
	{
		char *bak = *str;
		p_obrace(str);
		p_whitespace(str);
		if (**str == '[' || **str == '(')
		{
			*str = bak;
			result = (Temporal *)temporals_parse(str, basetype, linear);
		}
		else
		{
			*str = bak;
			result = (Temporal *)temporali_parse(str, basetype);		
		}
	}
	return result;
}

/*****************************************************************************/
