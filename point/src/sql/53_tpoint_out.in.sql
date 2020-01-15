/*****************************************************************************
 *
 * tpoint_out.sql
 *	  Output of temporal points in WKT, EWKT and MF-JSON format
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION asText(tgeompoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_as_text'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tgeompoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_as_text'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(tgeogpoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_as_text'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tgeogpoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_as_text'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION asText(geometry[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_as_text'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	
CREATE FUNCTION asText(geography[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_as_text'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	

CREATE FUNCTION asEWKT(tgeompoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_as_ewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(tgeompoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_as_ewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(tgeogpoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_as_ewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(tgeogpoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_as_ewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	

CREATE FUNCTION asEWKT(geometry[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_as_ewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	
CREATE FUNCTION asEWKT(geography[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_as_ewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asMFJSON(point tgeompoint, maxdecimaldigits int4 DEFAULT 15, options int4 DEFAULT 0)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_as_mfjson'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asMFJSON(point tgeogpoint, maxdecimaldigits int4 DEFAULT 15, options int4 DEFAULT 0)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_as_mfjson'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(tgeompoint, endianenconding text DEFAULT 'NDR')
	RETURNS bytea
	AS 'MODULE_PATHNAME', 'tpoint_as_binary'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tgeogpoint, endianenconding text DEFAULT 'NDR')
	RETURNS bytea
	AS 'MODULE_PATHNAME', 'tpoint_as_binary'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKB(tgeompoint, endianenconding text DEFAULT 'NDR')
	RETURNS bytea
	AS 'MODULE_PATHNAME', 'tpoint_as_ewkb'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKB(tgeogpoint, endianenconding text DEFAULT 'NDR')
	RETURNS bytea
	AS 'MODULE_PATHNAME', 'tpoint_as_ewkb'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexEWKB(tgeompoint, endianenconding text DEFAULT 'NDR')
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_as_hexewkb'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexEWKB(tgeogpoint, endianenconding text DEFAULT 'NDR')
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_as_hexewkb'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
