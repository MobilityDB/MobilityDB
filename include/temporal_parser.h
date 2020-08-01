/*****************************************************************************
 *
 * temporal_parser.h
 *	  Functions for parsing temporal types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_PARSER_H__
#define __TEMPORAL_PARSER_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "timetypes.h"
#include "temporal.h"

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
extern Period *period_parse(char **str, bool make);
extern PeriodSet *periodset_parse(char **str);
extern TInstant *tinstant_parse(char **str, Oid basetype, bool end, bool make);
extern Temporal *temporal_parse(char **str, Oid basetype);

/*****************************************************************************/

#endif
