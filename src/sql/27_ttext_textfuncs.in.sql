/*****************************************************************************
 *
 * temporal_textfuncs.sql
 *    Temporal text functions.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal text concatenation
 *****************************************************************************/


CREATE FUNCTION temporal_textcat(text, ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'textcat_base_ttext'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_textcat(ttext, text)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'textcat_ttext_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_textcat(ttext, ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'textcat_ttext_ttext'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR || (
  PROCEDURE = temporal_textcat,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = ||
);
CREATE OPERATOR || (
  PROCEDURE = temporal_textcat,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = ||
);
CREATE OPERATOR || (
  PROCEDURE = temporal_textcat,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = ||
);

/******************************************************************************
 * Temporal upper/lower case
 *****************************************************************************/

CREATE FUNCTION upper(ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'ttext_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lower(ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'ttext_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
