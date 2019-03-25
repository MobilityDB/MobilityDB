/*****************************************************************************
 *
 * TempSpatialRelsG.sql
 *	  Temporal spatial relationships for temporal geography points.
 *
 * These relationships are applied at each instant and result in a temporal 
 * Boolean. The following relationships are supported for geographies:
 *		tcovers, tcoveredby, tintersects, tdwithin
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * tcovers
 *****************************************************************************/

CREATE FUNCTION tcovers(geography, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcovers(tgeogpoint, geography)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcovers(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tcoveredby
 *****************************************************************************/

CREATE FUNCTION tcoveredby(geography, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcoveredby(tgeogpoint, geography)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcoveredby(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tintersects
 *****************************************************************************/

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
 * tdwithin
 *****************************************************************************/

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

/******************************************************************************/
