/*****************************************************************************
 *
 * Parser.h
 *	  Functions for parsing temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __PARSER_H__
#define __PARSER_H__

/*****************************************************************************/

extern void p_whitespace(char **str);
extern bool p_obrace(char **str);
extern bool p_cbrace(char **str);
extern bool p_obracket(char **str);
extern bool p_cbracket(char **str);
extern bool p_oparen(char **str);
extern bool p_cparen(char **str);
extern bool p_comma(char **str);

extern TBOX *tbox_parse(char **str);
extern Datum basetype_parse(char **str, Oid basetype);
extern TimestampTz timestamp_parse(char **str);
extern TimestampSet *timestampset_parse(char **str);
extern Period *period_parse(char **str);
extern PeriodSet *periodset_parse(char **str);
extern TemporalInst *temporalinst_parse(char **str, Oid basetype, bool end);
extern TemporalI *temporali_parse(char **str, Oid basetype);
extern Temporal *temporal_parse(char **str, Oid basetype);

/*****************************************************************************/

#endif
