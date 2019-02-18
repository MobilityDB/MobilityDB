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

#include "TemporalNPoint.h"

/*****************************************************************************/

nsegment
nsegment_parse(char **str)
{
	p_whitespace(str);

	int delim = 0;
	while ((*str)[delim] != ')' && (*str)[delim] != '\0')
		delim++;
	if ((*str)[delim] == '\0')
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						errmsg("Could not parse network region")));

	int64		rid;
	double	  pos1;
	double	  pos2;
	if (sscanf(*str, "(%ld,%lf,%lf)", &rid, &pos1, &pos2) != 3)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						errmsg("Could not parse network region")));
	if (pos1 < 0 || pos1 > 1 || pos2 < 0 || pos2 > 1)
		ereport(ERROR,
				(errcode(ERRCODE_INTERNAL_ERROR),
						errmsg("the relative position must be a real number between 0 and 1")));

	*str += delim + 1;

	nsegment nseg;
	nseg.rid = rid;
	nseg.pos1 = Min(pos1, pos2);
	nseg.pos2 = Max(pos1, pos2);
	return nseg;
}

nregion *
nregion_parse(char **str)
{
	if (!p_obrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				errmsg("Could not parse network region")));

	//FIXME: parsing twice
	char *bak = *str;
	nsegment_parse(str);
	int count = 1;
	while (p_comma(str))
	{
		nsegment_parse(str);
		count++;
	}
	if (!p_cbrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				errmsg("Could not parse network region")));

	*str = bak;
	nsegment *nsegs = palloc(sizeof(nsegment) * count);
	for (int i = 0; i < count; i++)
	{
		p_comma(str);
		nsegs[i] = nsegment_parse(str);
	}
	p_cbrace(str);

	nregion *result = nregion_from_nsegmentarr_internal(nsegs, count);
	pfree(nsegs);
	return result;
}

TemporalSeq *
tnpointseq_parse(char **str, Oid basetype)
{
	bool lower_inc = false, upper_inc = false;
	if (p_obracket(str))
		lower_inc = true;
	else if (p_oparen(str))
		lower_inc = false;
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				errmsg("Could not parse temporal value")));

	//FIXME: parsing twice
	char *bak = *str;
	TemporalInst *inst = temporalinst_parse1(str, basetype, false);
	int64 rid0 = DatumGetNpoint(temporalinst_value(inst))->rid;
	int count = 1;
	while (p_comma(str))
	{
		count++;
		pfree(inst);
		inst = temporalinst_parse1(str, basetype, false);
		int64 rid = DatumGetNpoint(temporalinst_value(inst))->rid;
		if (rid != rid0)
		{
			pfree(inst);
			ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
					errmsg("Temporal sequence must have same rid")));
		}
	}
	pfree(inst);
	if (p_cbracket(str))
		upper_inc = true;
	else if (p_cparen(str))
		upper_inc = false;
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				errmsg("Could not parse temporal value")));

	*str = bak; // unparse what we parsed...
	TemporalInst **insts = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++)
	{
		p_comma(str);
		insts[i] = temporalinst_parse1(str, basetype, false);
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

TemporalS *
tnpoints_parse(char **str, Oid basetype)
{
	if (!p_obrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				errmsg("Could not parse temporal value")));

	//FIXME: parsing twice
	char *bak = *str;
	TemporalSeq *seq = tnpointseq_parse(str, basetype);
	int count = 1;
	while (p_comma(str))
	{
		count++;
		pfree(seq);
		seq = tnpointseq_parse(str, basetype);
	}
	pfree(seq);
	if (!p_cbrace(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				errmsg("Could not parse temporal value")));

	*str = bak;
	TemporalSeq **seqs = palloc(sizeof(TemporalSeq *) * count);
	for (int i = 0; i < count; i++)
	{
		p_comma(str);
		seqs[i] = tnpointseq_parse(str, basetype);
	}
	p_cbrace(str);
	TemporalS *result = temporals_from_temporalseqarr(seqs, count, true);

	for (int i = 0; i < count; i++)
		pfree(seqs[i]);
	pfree(seqs);

	return result;
}

/*****************************************************************************/
