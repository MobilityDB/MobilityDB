/*****************************************************************************
 *
 * tpoint_tempspatialrels.h
 *	  Temporal spatial relationships for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_TEMPSPATIALRELS_H__
#define __TPOINT_TEMPSPATIALRELS_H__

#include <postgres.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum tcontains_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tcontains_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tcontains_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tcontainsproperly_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tcontainsproperly_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tcontainsproperly_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tcovers_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tcovers_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tcovers_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tcoveredby_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tcoveredby_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tcoveredby_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tdisjoint_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tdisjoint_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tdisjoint_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tequals_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tequals_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tequals_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tintersects_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tintersects_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tintersects_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum ttouches_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum ttouches_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum ttouches_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum twithin_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum twithin_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum twithin_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tdwithin_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tdwithin_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tdwithin_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum trelate_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum trelate_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum trelate_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum trelate_pattern_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum trelate_pattern_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum trelate_pattern_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
