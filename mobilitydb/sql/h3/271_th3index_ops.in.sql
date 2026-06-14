/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Bounding-box and temporal-position operators for `th3index`.
 *
 * H3 cells are geographic hexagons on the WGS84 sphere — always geodetic,
 * always lat/lon. The bounding box of a `th3index` value is therefore a
 * geodetic `stbox` (X/Y set, GEODETIC flag set, T set from the time span).
 * This matches the `tgeogpoint` / `tcbuffer` `tspatial_type` pattern.
 *
 * All C implementations behind these operators are type-generic — they
 * dispatch on the Temporal struct's MeosType via `oid_meostype()` and the
 * catalog entry `{T_TH3INDEX, T_H3INDEX}`. No new C symbols are introduced
 * beyond the bounding box conversion function.
 *
 * The grid-distance `<->(th3index, th3index)` built on
 * `h3_grid_distance` is declared in `285_th3index_traversal.in.sql` —
 * it is not an ORDER BY operator and is not part of the GiST / SP-GiST
 * opclasses.
 */

/******************************************************************************
 * th3index to stbox conversion
 ******************************************************************************/

CREATE FUNCTION stbox(th3index)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (th3index AS stbox) WITH FUNCTION stbox(th3index);

/******************************************************************************
 * Bounding-box operators (&&, ~=, @>, <@, -|-)
 *
 * Mirroring the tcbuffer section from
 * mobilitydb/sql/cbuffer/158_tcbuffer_topops.in.sql.
 ******************************************************************************/

/* tstzspan op th3index */

CREATE FUNCTION temporal_overlaps(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
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

/*****************************************************************************/

/* stbox op th3index */

CREATE FUNCTION temporal_overlaps(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = stbox, RIGHTARG = th3index,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = th3index, RIGHTARG = stbox,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tstzspan op th3index */

CREATE FUNCTION temporal_contains(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
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

/*****************************************************************************/

/* stbox op th3index */

CREATE FUNCTION temporal_contains(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = stbox, RIGHTARG = th3index,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = th3index, RIGHTARG = stbox,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = <@,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tstzspan op th3index */

CREATE FUNCTION temporal_contained(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
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

/*****************************************************************************/

/* stbox op th3index */

CREATE FUNCTION temporal_contained(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = stbox, RIGHTARG = th3index,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = th3index, RIGHTARG = stbox,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tstzspan op th3index */

CREATE FUNCTION temporal_same(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
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

/*****************************************************************************/

/* stbox op th3index */

CREATE FUNCTION temporal_same(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_same(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = stbox, RIGHTARG = th3index,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = th3index, RIGHTARG = stbox,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = temporal_same,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tstzspan op th3index */

CREATE FUNCTION temporal_adjacent(tstzspan, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(th3index, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
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

/*****************************************************************************/

/* stbox op th3index */

CREATE FUNCTION temporal_adjacent(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = stbox, RIGHTARG = th3index,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = th3index, RIGHTARG = stbox,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = th3index, RIGHTARG = th3index,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/******************************************************************************
 * Temporal-position operators (<<#, &<#, #>>, #&>)
 *
 * Mirroring the tcbuffer section from
 * mobilitydb/sql/cbuffer/158_tcbuffer_topops.in.sql.
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

/* stbox op th3index */

CREATE FUNCTION temporal_before(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* th3index op stbox */

CREATE FUNCTION temporal_before(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* th3index op th3index */

CREATE FUNCTION temporal_before(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/******************************************************************************
 * Spatial relative-position operators (<<, &<, >>, &>, <<|, &<|, |>>, |&>)
 *
 * Required by the GiST and SP-GiST opclasses. H3 cell bounding boxes are
 * geodetic stboxes, so the generic tspatial C symbols handle dispatch.
 ******************************************************************************/

/* stbox op th3index */

CREATE FUNCTION temporal_left(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(stbox, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_overleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_overright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = stbox, RIGHTARG = th3index,
  PROCEDURE = temporal_overabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* th3index op stbox */

CREATE FUNCTION temporal_left(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(th3index, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_overleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_overright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = th3index, RIGHTARG = stbox,
  PROCEDURE = temporal_overabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* th3index op th3index */

CREATE FUNCTION temporal_left(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(th3index, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_overleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_overright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_overabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/
