/*****************************************************************************
 *
 * tpoint_out.h
 *	  Output of temporal points in WKT, EWKT and MF-JSON format
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_OUT_H__
#define __TPOINT_OUT_H__

#include <postgres.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum tpoint_as_text(PG_FUNCTION_ARGS);
extern Datum tpoint_as_ewkt(PG_FUNCTION_ARGS);
extern Datum geoarr_as_text(PG_FUNCTION_ARGS);
extern Datum geoarr_as_ewkt(PG_FUNCTION_ARGS);
extern Datum tpointarr_as_text(PG_FUNCTION_ARGS);
extern Datum tpointarr_as_ewkt(PG_FUNCTION_ARGS);
extern Datum tpoint_as_mfjson(PG_FUNCTION_ARGS);
extern Datum tpoint_as_binary(PG_FUNCTION_ARGS);
extern Datum tpoint_as_ewkb(PG_FUNCTION_ARGS);
extern Datum tpoint_as_hexewkb(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
