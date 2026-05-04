/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief Bounding-box and temporal-position operators for `th3index`.
 *
 * `th3index` has a **time-only** bounding box (a `tstzspan`), matching
 * the `tbool` / `ttext` `talpha_type` pattern. There is no
 * `h3indexspan` companion — H3 cell ids have no total order that
 * would make a value-range meaningful (precedent: `geometry` has no
 * `geometryspan`; `text` has no `textspan`). This file therefore
 * mirrors the `tbool` section of
 * `mobilitydb/sql/temporal/032_temporal_boxops.in.sql` (bounding
 * overlap / contains / contained / same / adjacent) and of
 * `mobilitydb/sql/temporal/034_temporal_posops.in.sql`
 * (temporal-direction before / overbefore / after / overafter) —
 * every operator pair is either `(tstzspan, th3index)`, `(th3index,
 * tstzspan)`, or `(th3index, th3index)`. Value-range operators
 * (`<<`, `&<`, `&>`, `>>`) and TBox overloads are intentionally
 * absent.
 *
 * All C implementations behind these operators are already
 * type-generic — they dispatch on the Temporal struct's MeosType
 * via `oid_meostype()` and the catalog entry `{T_TH3INDEX,
 * T_H3INDEX}`. No new C symbols are introduced.
 *
 * The grid-distance `<->(th3index, th3index)` built on
 * `h3_grid_distance` is declared in
 * `285_th3index_traversal.in.sql` — it is not an ORDER BY operator
 * and is not part of the rtree / spgist opclasses.
 */

/******************************************************************************
 * Bounding-box operators (&&, ~=, @>, <@, -|-)
 *
 * Mirror of the tbool section from
 * mobilitydb/sql/temporal/032_temporal_boxops.in.sql.
 ******************************************************************************/

CREATE FUNCTION temporal_overlaps(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = tstzspan, RIGHTARG = th3index,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = th3index, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contains(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = tstzspan, RIGHTARG = th3index,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = th3index, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = <@,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_contained(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = tstzspan, RIGHTARG = th3index,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = th3index, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = @>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_same(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = tstzspan, RIGHTARG = th3index,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = th3index, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_adjacent(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = tstzspan, RIGHTARG = th3index,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = th3index, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/******************************************************************************
 * Temporal-position operators (<<#, &<#, #>>, #&>)
 *
 * Mirror of the tbool section from
 * mobilitydb/sql/temporal/034_temporal_posops.in.sql.
 ******************************************************************************/

/* tstzspan op th3index */

CREATE FUNCTION temporal_before(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = th3index,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = th3index,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = th3index,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = th3index,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* th3index op tstzspan */

CREATE FUNCTION temporal_before(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = th3index, RIGHTARG = tstzspan,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = th3index, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = th3index, RIGHTARG = tstzspan,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = th3index, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* th3index op th3index */

CREATE FUNCTION temporal_before(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
