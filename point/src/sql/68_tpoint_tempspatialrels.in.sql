/*****************************************************************************
 *
 * tpoint_tempspatialrels.sql
 *	  Spatial relationships for temporal points.
 *
 * These relationships are applied at each instant and result in a temporal 
 * Boolean. 
 * The following relationships are supported for a temporal geometry point
 * and a geometry:
 *		tcontains, tcovers, tcoveredby, tdisjoint, tequals, tintersects,
 *		ttouches, twithin, tdwithin, and trelate (with 2 and 3 arguments)
 * The following relationships are supported for two temporal geometry points:
 *		tdisjoint, tequals, tintersects, tdwithin, and trelate (with 2 and 3
 *    arguments)
 * The following relationships are supported for a temporal geography point
 * and a geography:
 *		tcovers, tcoveredby, tdisjoint, tintersects, tdwithin
 * The following relationships are supported for two temporal geography points:
 *		tdisjoint, tequals, tintersects, tdwithin
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * tcontains
 *****************************************************************************/

CREATE FUNCTION tcontains(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcontains_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcontains(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcontains_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tcovers
 *****************************************************************************/

CREATE FUNCTION tcovers(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcovers(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tcovers(geography, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcovers(tgeogpoint, geography)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tcoveredby
 *****************************************************************************/

CREATE FUNCTION tcoveredby(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcoveredby(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tcoveredby(geography, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcoveredby(tgeogpoint, geography)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tdisjoint
 *****************************************************************************/

CREATE FUNCTION tdisjoint(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tdisjoint(geography, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeogpoint, geography)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tequals
 *****************************************************************************/

CREATE FUNCTION tequals(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tequals(geography, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tgeogpoint, geography)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tintersects
 *****************************************************************************/

CREATE FUNCTION tintersects(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tintersects(geography, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeogpoint, geography)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * ttouches
 *****************************************************************************/

CREATE FUNCTION ttouches(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'ttouches_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'ttouches_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * twithin
 *****************************************************************************/

CREATE FUNCTION twithin(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'twithin_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION twithin(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'twithin_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tdwithin
 *****************************************************************************/

CREATE FUNCTION tdwithin(geometry, tgeompoint, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, geometry, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, tgeompoint, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tdwithin(geography, tgeogpoint, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeogpoint, geography, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	
CREATE FUNCTION tdwithin(tgeogpoint, tgeogpoint, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * trelate (2 arguments)
 *****************************************************************************/

CREATE FUNCTION trelate(geometry, tgeompoint)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'trelate_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, geometry)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'trelate_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, tgeompoint)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'trelate_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * trelate (3 arguments)
 *****************************************************************************/

CREATE FUNCTION trelate(geometry, tgeompoint, pattern text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'trelate_pattern_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, geometry, pattern text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'trelate_pattern_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, tgeompoint, pattern text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'trelate_pattern_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
