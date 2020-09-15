/*****************************************************************************
 *
 * temporal_posops.h
 *    Relative position operators for temporal types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_POSOPS_H__
#define __TEMPORAL_POSOPS_H__

#include <postgres.h>
#include <catalog/pg_type.h>

#include "temporal.h"

/*****************************************************************************/

extern Datum before_period_temporal(PG_FUNCTION_ARGS);
extern Datum overbefore_period_temporal(PG_FUNCTION_ARGS);
extern Datum after_period_temporal(PG_FUNCTION_ARGS);
extern Datum overafter_period_temporal(PG_FUNCTION_ARGS);

extern Datum before_temporal_period(PG_FUNCTION_ARGS);
extern Datum overbefore_temporal_period(PG_FUNCTION_ARGS);
extern Datum after_temporal_period(PG_FUNCTION_ARGS);
extern Datum overafter_temporal_period(PG_FUNCTION_ARGS);

extern Datum before_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum overbefore_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum after_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum overafter_temporal_temporal(PG_FUNCTION_ARGS);

/*****************************************************************************/

extern Datum left_range_tnumber(PG_FUNCTION_ARGS);
extern Datum overleft_range_tnumber(PG_FUNCTION_ARGS);
extern Datum right_range_tnumber(PG_FUNCTION_ARGS);
extern Datum overright_range_tnumber(PG_FUNCTION_ARGS);

extern Datum left_tnumber_range(PG_FUNCTION_ARGS);
extern Datum overleft_tnumber_range(PG_FUNCTION_ARGS);
extern Datum overright_tnumber_range(PG_FUNCTION_ARGS);
extern Datum right_tnumber_range(PG_FUNCTION_ARGS);

extern Datum left_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum overleft_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum right_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum overright_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum before_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum overbefore_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum after_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum overafter_tbox_tnumber(PG_FUNCTION_ARGS);

extern Datum left_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum overleft_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum right_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum overright_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum before_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum overbefore_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum after_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum overafter_tnumber_tbox(PG_FUNCTION_ARGS);

extern Datum left_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum overleft_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum right_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum overright_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum before_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum overbefore_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum after_tnumber_tnumber(PG_FUNCTION_ARGS);
extern Datum overafter_tnumber_tnumber(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
