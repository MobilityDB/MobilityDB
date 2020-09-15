/*****************************************************************************
 *
 * tpoint_in.sql
 *    Input of temporal points in WKT, EWKT and MF-JSON format
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*
CREATE FUNCTION fromText(text)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_from_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION fromText(text)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'tpoint_from_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION fromEWKT(tgeompoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'tpoint_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION fromEWKT(tgeogpoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'tpoint_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

CREATE FUNCTION fromMFJSON(text)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
/*
CREATE FUNCTION fromMFJSON(text)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'tpoint_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

CREATE FUNCTION fromEWKB(bytea)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_from_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
/*
CREATE FUNCTION fromEWKB(bytea)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'tpoint_from_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

/*
CREATE FUNCTION fromHexEWKB(bytea)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_from_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION fromEWKB(bytea)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'tpoint_from_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

/*****************************************************************************/
