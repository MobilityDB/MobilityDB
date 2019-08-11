/*****************************************************************************
 *
 * GeoRelativePosOps.h
 *	  Relative position operators for temporal geometry points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __GEORELATIVEPOSOPS_H__
#define __GEORELATIVEPOSOPS_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "Temporal.h"

/*****************************************************************************/

extern Datum left_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overleft_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum right_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overright_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum below_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overbelow_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum above_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overabove_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum front_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overfront_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum back_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overback_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum before_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overbefore_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum after_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overafter_stbox_stbox(PG_FUNCTION_ARGS);

extern bool left_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool overleft_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool right_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool overright_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool below_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool overbelow_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool above_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool overabove_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool front_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool overfront_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool back_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool overback_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool before_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool overbefore_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool after_stbox_stbox_internal(STBOX *box1, STBOX *box2);
extern bool overafter_stbox_stbox_internal(STBOX *box1, STBOX *box2);

extern Datum left_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overleft_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum right_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overright_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum below_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overbelow_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum above_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overabove_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum front_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overfront_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum back_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overback_geom_tpoint(PG_FUNCTION_ARGS);

extern Datum left_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overleft_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum right_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overright_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum above_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overabove_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum below_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overbelow_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum front_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overfront_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum back_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overback_tpoint_geom(PG_FUNCTION_ARGS);

extern Datum left_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overleft_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum right_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overright_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum below_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overbelow_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum above_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overabove_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum front_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overfront_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum back_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overback_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum before_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overbefore_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum after_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overafter_stbox_tpoint(PG_FUNCTION_ARGS);

extern Datum left_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overleft_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum right_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overright_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum above_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overabove_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum below_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overbelow_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum front_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overfront_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum back_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overback_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum before_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overbefore_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum after_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overafter_tpoint_stbox(PG_FUNCTION_ARGS);

extern Datum left_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overleft_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum right_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overright_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum above_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overabove_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum below_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overbelow_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum front_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overfront_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum back_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overback_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum before_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overbefore_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum after_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overafter_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
