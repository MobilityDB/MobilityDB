/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * tnpoint_boxops.sql
 * Bounding box operators for temporal network points.
 */

/*****************************************************************************/

CREATE FUNCTION tnpoint_overlaps_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tnpoint_overlaps_sel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnpoint_overlaps_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tnpoint_overlaps_joinsel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnpoint_contains_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tnpoint_contains_sel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnpoint_contains_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tnpoint_contains_joinsel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnpoint_same_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tnpoint_same_sel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnpoint_same_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tnpoint_same_joinsel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnpoint_adjacent_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tnpoint_adjacent_sel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnpoint_adjacent_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tnpoint_adjacent_joinsel'
  LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * Temporal npoint to stbox
 *****************************************************************************/

CREATE FUNCTION stbox(npoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'npoint_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(nsegment)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'nsegment_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(npoint, timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'npoint_timestamp_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(npoint, period)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'npoint_period_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(tnpoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'tnpoint_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (npoint AS stbox) WITH FUNCTION stbox(npoint) AS IMPLICIT;
CREATE CAST (nsegment AS stbox) WITH FUNCTION stbox(nsegment) AS IMPLICIT;
CREATE CAST (tnpoint AS stbox) WITH FUNCTION stbox(tnpoint) AS IMPLICIT;

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION contains_bbox(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = geometry, RIGHTARG = tnpoint,
  COMMUTATOR = <@,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = stbox, RIGHTARG = tnpoint,
  COMMUTATOR = <@,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = npoint, RIGHTARG = tnpoint,
  COMMUTATOR = <@,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);

CREATE FUNCTION contains_bbox(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tnpoint, RIGHTARG = geometry,
  COMMUTATOR = <@,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tnpoint, RIGHTARG = stbox,
  COMMUTATOR = <@,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tnpoint, RIGHTARG = npoint,
  COMMUTATOR = <@,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = <@,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION contained_bbox(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = geometry, RIGHTARG = tnpoint,
  COMMUTATOR = @>,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = stbox, RIGHTARG = tnpoint,
  COMMUTATOR = @>,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = npoint, RIGHTARG = tnpoint,
  COMMUTATOR = @>,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);

CREATE FUNCTION contained_bbox(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tnpoint, RIGHTARG = geometry,
  COMMUTATOR = @>,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tnpoint, RIGHTARG = stbox,
  COMMUTATOR = @>,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tnpoint, RIGHTARG = npoint,
  COMMUTATOR = @>,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = @>,
  RESTRICT = tnpoint_contains_sel, JOIN = tnpoint_contains_joinsel
);

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = geometry, RIGHTARG = tnpoint,
  COMMUTATOR = &&,
  RESTRICT = tnpoint_overlaps_sel, JOIN = tnpoint_overlaps_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = stbox, RIGHTARG = tnpoint,
  COMMUTATOR = &&,
  RESTRICT = tnpoint_overlaps_sel, JOIN = tnpoint_overlaps_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = npoint, RIGHTARG = tnpoint,
  COMMUTATOR = &&,
  RESTRICT = tnpoint_overlaps_sel, JOIN = tnpoint_overlaps_joinsel
);

CREATE FUNCTION overlaps_bbox(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tnpoint, RIGHTARG = geometry,
  COMMUTATOR = &&,
  RESTRICT = tnpoint_overlaps_sel, JOIN = tnpoint_overlaps_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tnpoint, RIGHTARG = stbox,
  COMMUTATOR = &&,
  RESTRICT = tnpoint_overlaps_sel, JOIN = tnpoint_overlaps_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tnpoint, RIGHTARG = npoint,
  COMMUTATOR = &&,
  RESTRICT = tnpoint_overlaps_sel, JOIN = tnpoint_overlaps_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = &&,
  RESTRICT = tnpoint_overlaps_sel, JOIN = tnpoint_overlaps_joinsel
);

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION same_bbox(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = geometry, RIGHTARG = tnpoint,
  COMMUTATOR = ~=,
  RESTRICT = tnpoint_same_sel, JOIN = tnpoint_same_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = stbox, RIGHTARG = tnpoint,
  COMMUTATOR = ~=,
  RESTRICT = tnpoint_same_sel, JOIN = tnpoint_same_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = npoint, RIGHTARG = tnpoint,
  COMMUTATOR = ~=,
  RESTRICT = tnpoint_same_sel, JOIN = tnpoint_same_joinsel
);

CREATE FUNCTION same_bbox(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tnpoint, RIGHTARG = geometry,
  COMMUTATOR = ~=,
  RESTRICT = tnpoint_same_sel, JOIN = tnpoint_same_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tnpoint, RIGHTARG = stbox,
  COMMUTATOR = ~=,
  RESTRICT = tnpoint_same_sel, JOIN = tnpoint_same_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tnpoint, RIGHTARG = npoint,
  COMMUTATOR = ~=,
  RESTRICT = tnpoint_same_sel, JOIN = tnpoint_same_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = ~=,
  RESTRICT = tnpoint_same_sel, JOIN = tnpoint_same_joinsel
);

/*****************************************************************************
 * adjacent
 *****************************************************************************/

CREATE FUNCTION adjacent_bbox(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = geometry, RIGHTARG = tnpoint,
  COMMUTATOR = -|-,
  RESTRICT = tnpoint_adjacent_sel, JOIN = tnpoint_adjacent_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = stbox, RIGHTARG = tnpoint,
  COMMUTATOR = -|-,
  RESTRICT = tnpoint_adjacent_sel, JOIN = tnpoint_adjacent_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = npoint, RIGHTARG = tnpoint,
  COMMUTATOR = -|-,
  RESTRICT = tnpoint_adjacent_sel, JOIN = tnpoint_adjacent_joinsel
);

CREATE FUNCTION adjacent_bbox(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tnpoint, RIGHTARG = geometry,
  COMMUTATOR = -|-,
  RESTRICT = tnpoint_adjacent_sel, JOIN = tnpoint_adjacent_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tnpoint, RIGHTARG = stbox,
  COMMUTATOR = -|-,
  RESTRICT = tnpoint_adjacent_sel, JOIN = tnpoint_adjacent_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tnpoint, RIGHTARG = npoint,
  COMMUTATOR = -|-,
  RESTRICT = tnpoint_adjacent_sel, JOIN = tnpoint_adjacent_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = -|-,
  RESTRICT = tnpoint_adjacent_sel, JOIN = tnpoint_adjacent_joinsel
);

/*****************************************************************************/
