-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "ALTER EXTENSION mobilitydb UPDATE TO '1.2.1'" to load this file. \quit

-- Fully drop all operators and rebuild them
-- with the correct commutators defined

DROP OPERATOR #< (text, ttext);
DROP OPERATOR #< (ttext, text);
DROP OPERATOR #< (ttext, ttext);

DROP OPERATOR #> (text, ttext);
DROP OPERATOR #> (ttext, text);
DROP OPERATOR #> (ttext, ttext);

DROP OPERATOR #<= (text, ttext);
DROP OPERATOR #<= (ttext, text);
DROP OPERATOR #<= (ttext, ttext);

DROP OPERATOR #>= (text, ttext);
DROP OPERATOR #>= (ttext, text);
DROP OPERATOR #>= (ttext, ttext);

CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #>
);
CREATE OPERATOR #< (
  PROCEDURE = temporal_tlt,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #>
);

CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #<
);
CREATE OPERATOR #> (
  PROCEDURE = temporal_tgt,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #<
);

CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
  PROCEDURE = temporal_tle,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #>=
);

CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = text, RIGHTARG = ttext,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = ttext, RIGHTARG = text,
  COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
  PROCEDURE = temporal_tge,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = #<=
);
