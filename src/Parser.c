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

#include "TemporalTypes.h"
#ifdef WITH_POSTGIS
#include "TemporalPoint.h"
#endif

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
p_basetype(char **str, Oid basetype)
{ // ugly
	p_whitespace(str);
	int delim = 0;
	bool isttext = false;
	/* ttext values must be enclosed between double quotes */
	if (**str == '"')
	{
		isttext = true;
		/* Consume the double quote */
		*str += 1;
		while ( ( (*str)[delim] != '"' || (*str)[delim-1] == '\\' )  && 
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
	*str += delim + 1; // since we know there's an @ here, let's take it with us
	return result;
}

/*****************************************************************************/
/* Time Types */

TimestampTz 
timestamp_parse(char **str) 
{ // ugly
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
period_parse(char **str) 
{
	bool lower_inc = false, upper_inc = false;
	if (p_obracket(str))
		lower_inc = true;
	else if (p_oparen(str))
		lower_inc = false;
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));

	TimestampTz lower = timestamp_parse(str);
	p_comma(str);
	TimestampTz upper = timestamp_parse(str);

	if (p_cbracket(str))
		upper_inc = true;
	else if (p_cparen(str))
		upper_inc = false;
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));

	Period *result = period_make(lower, upper, lower_inc, upper_inc);

	return result;
}

TimestampSet *
timestampset_parse(char **str) 
{
	if (!p_obrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse timestamp set")));

	//FIXME: parsing twice
	char *bak = *str;
	TimestampTz t = timestamp_parse(str);
	/* keep compiler quiet */
	if (t == 0) 
	{}
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		t = timestamp_parse(str);
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
	TimestampSet *result = timestampset_from_timestamparr_internal(times, count);

	pfree(times);

	return result;
}

PeriodSet *
periodset_parse(char **str) 
{
	if (!p_obrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse period set")));

	//FIXME: parsing twice
	char *bak = *str;
	Period *per = period_parse(str);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		pfree(per);
		per = period_parse(str);
	}
	pfree(per);
	if (!p_cbrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse period set")));

	*str = bak;
	Period **periods = palloc(sizeof(Period *) * count);
	for (int i = 0; i < count; i++) 
	{
		p_comma(str);
		periods[i] = period_parse(str);
	}
	p_cbrace(str);
	PeriodSet *result = periodset_from_periodarr_internal(periods, count, true);

	for (int i = 0; i < count; i++)
		pfree(periods[i]);
	pfree(periods);

	return result;
}

/*****************************************************************************/
/* Temporal Types */

TemporalInst *
temporalinst_parse(char **str, Oid basetype, bool end) 
{
	p_whitespace(str);
	/* The next two instructions will throw an exception if they fail */
	Datum elem = p_basetype(str, basetype);
	TimestampTz t = timestamp_parse(str);
	if (end)
	{
		/* Ensure there is no more input */
		p_whitespace(str);
		if (**str != 0)
			ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
				errmsg("Could not parse temporal value")));
	}

	return temporalinst_make(elem, t, basetype);
}

TemporalI *
temporali_parse(char **str, Oid basetype) 
{
	p_whitespace(str);
	if (!p_obrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));

	//FIXME: parsing twice
	char *bak = *str;
	TemporalInst *inst = temporalinst_parse(str, basetype, false);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		pfree(inst);
		inst = temporalinst_parse(str, basetype, false);
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
		insts[i] = temporalinst_parse(str, basetype, false);
	}
	p_cbrace(str);
	TemporalI *result = temporali_from_temporalinstarr(insts, count);

	for (int i = 0; i < count; i++)
		pfree(insts[i]);
	pfree(insts);

	return result;
}

static TemporalSeq *
temporalseq_parse(char **str, Oid basetype, bool end) 
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
	TemporalInst *inst = temporalinst_parse(str, basetype, false);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		pfree(inst);
		inst = temporalinst_parse(str, basetype, false);
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
		insts[i] = temporalinst_parse(str, basetype, false);
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
temporals_parse(char **str, Oid basetype) 
{
	p_whitespace(str);
	if (!p_obrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse temporal value")));

	//FIXME: parsing twice
	char *bak = *str;
	TemporalSeq *seq = temporalseq_parse(str, basetype, false);
	int count = 1;
	while (p_comma(str)) 
	{
		count++;
		pfree(seq);
		seq = temporalseq_parse(str, basetype, false);
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
		seqs[i] = temporalseq_parse(str, basetype, false);
	}
	p_cbrace(str);
	TemporalS *result = temporals_from_temporalseqarr(seqs, count, true);

	for (int i = 0; i < count; i++)
		pfree(seqs[i]);
	pfree(seqs);

	return result;
}

Temporal *
temporal_parse(char **str, Oid basetype) 
{
	p_whitespace(str);
	if (**str != '{' && **str != '[' && **str != '(')
		return (Temporal *)temporalinst_parse(str, basetype, true);
	else if (**str == '[' || **str == '(')
		return (Temporal *)temporalseq_parse(str, basetype, true);		
	else if (**str == '{')
	{
		char *bak = *str;
		p_obrace(str);
		p_whitespace(str);
		if (**str == '[' || **str == '(')
		{
			*str = bak;
			return (Temporal *)temporals_parse(str, basetype);
		}
		else
		{
			*str = bak;
			return (Temporal *)temporali_parse(str, basetype);		
		}
	}
	return NULL; /* keep compiler quiet */
}

/*****************************************************************************/
