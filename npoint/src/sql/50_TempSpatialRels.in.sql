/*****************************************************************************
 *
 * TempSpatialRels.sql
 *	  Spatial relationships for temporal network-constrained points.
 *
 * These relationships are applied at each instant and result in a temporal 
 * Boolean. The following relationships are supported:
 *		tcontains, tcovers, tcoveredby, tdisjoint, equals, tintersects, 
 *		ttouches, twithin, tdwithin, and trelate (with 2 and 3 arguments)
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * tcontains
 *****************************************************************************/

CREATE FUNCTION tcontains(geometry, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcontains_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcontains(tnpoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcontains_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcontains(tnpoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcontains_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * tcovers
 *****************************************************************************/

CREATE FUNCTION tcovers(geometry, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcovers(tnpoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcovers(tnpoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tcoveredby
 *****************************************************************************/

CREATE FUNCTION tcoveredby(geometry, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcoveredby(tnpoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcoveredby(tnpoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * tdisjoint
 *****************************************************************************/

CREATE FUNCTION tdisjoint(geometry, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tnpoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tnpoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * tequals
 *****************************************************************************/

CREATE FUNCTION tequals(geometry, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tnpoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tnpoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * tintersects
 *****************************************************************************/

CREATE FUNCTION tintersects(geometry, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tnpoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tnpoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * ttouches
 *****************************************************************************/

CREATE FUNCTION ttouches(geometry, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'ttouches_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tnpoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'ttouches_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tnpoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'ttouches_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * twithin
 *****************************************************************************/

CREATE FUNCTION twithin(geometry, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'twithin_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION twithin(tnpoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'twithin_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION twithin(tnpoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'twithin_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * tdwithin
 *****************************************************************************/

CREATE FUNCTION tdwithin(geometry, tnpoint, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tnpoint, geometry, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tnpoint, tnpoint, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * trelate (2 arguments)
 *****************************************************************************/

CREATE FUNCTION trelate(geometry, tnpoint)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'trelate_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tnpoint, geometry)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'trelate_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tnpoint, tnpoint)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'trelate_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * trelate (3 arguments)
 *****************************************************************************/

CREATE FUNCTION trelate(geometry, tnpoint, pattern text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'trelate_pattern_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tnpoint, geometry, pattern text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'trelate_pattern_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tnpoint, tnpoint, pattern text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'trelate_pattern_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************/
