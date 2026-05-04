/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief Type plumbing for `th3index`, a temporal type carrying H3 cell
 * indices as a function of time.
 *
 * On-disk representation is the same Temporal structure used by
 * every other temporal type; the basetype is the dedicated
 * `h3index`, not `int8`, and the catalog entry
 * `{T_TH3INDEX, T_H3INDEX}` drives dispatch. th3index is
 * classified as a `talpha_type` — its bounding box is a
 * `tstzspan` (time only), like `tbool` / `ttext`, because H3
 * cell ids have no meaningful total order.
 *
 * Casts to / from `tbigint` are ASSIGNMENT-only — the user must
 * spell out the cast, mistakes surface as a clear binder error
 * rather than silently reinterpreting an arbitrary int64
 * trajectory as a stream of H3 cells. The two types are
 * binary-coercible (the int64 payload is identical) so the casts
 * are `WITHOUT FUNCTION` — see below.
 */

CREATE TYPE th3index;

/******************************************************************************
 * Input / Output
 ******************************************************************************/

CREATE FUNCTION th3index_in(cstring, oid, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION th3index_recv(internal, oid, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_out(th3index)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Temporal_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_send(th3index)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE th3index (
  internallength = variable,
  input = th3index_in,
  output = temporal_out,
  send = temporal_send,
  receive = th3index_recv,
  typmod_in = temporal_typmod_in,
  typmod_out = temporal_typmod_out,
  storage = extended,
  alignment = double,
  analyze = temporal_analyze
);

/******************************************************************************
 * Typmod enforcer + self-cast (mirrors tbigint / tint / tfloat / ttext)
 ******************************************************************************/

CREATE FUNCTION th3index(th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_enforce_typmod'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (th3index AS th3index) WITH FUNCTION th3index(th3index, integer)
  AS IMPLICIT;

/******************************************************************************
 * Explicit assignment casts to / from tbigint
 *
 * th3index and tbigint share the same on-disk representation (both are
 * temporal carriers over an int64 payload). A binary-coercion cast is
 * therefore both correct and zero-cost; it is declared `AS ASSIGNMENT`
 * (not IMPLICIT) so the user has to spell out `::th3index` or
 * `::tbigint` — mistakes surface as a clear "function
 * h3_get_resolution(tbigint) does not exist" binder error rather than
 * silently reinterpreting arbitrary int64 trajectories as H3 cells.
 ******************************************************************************/

CREATE CAST (tbigint AS th3index) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (th3index AS tbigint) WITHOUT FUNCTION AS ASSIGNMENT;

/******************************************************************************
 * Comparison functions and B-tree / hash indexing
 *
 * All th3index values share the exact same on-disk layout as every
 * other temporal type, so the generic Temporal_* dispatch in the
 * backend is enough — no th3index-specific wrappers required.
 ******************************************************************************/

CREATE FUNCTION temporal_lt(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_cmp(th3index, th3index)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Temporal_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS th3index_btree_ops
  DEFAULT FOR TYPE th3index USING btree AS
    OPERATOR  1 <,
    OPERATOR  2 <=,
    OPERATOR  3 =,
    OPERATOR  4 >=,
    OPERATOR  5 >,
    FUNCTION  1 temporal_cmp(th3index, th3index);

/******************************************************************************/

CREATE FUNCTION temporal_hash(th3index)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS th3index_hash_ops
  DEFAULT FOR TYPE th3index USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   temporal_hash(th3index);

/******************************************************************************/
