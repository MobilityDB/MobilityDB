/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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

/*
 * GeoPose frame metadata registry.
 *
 * The OGC GeoPose v1.0 standard distinguishes the *outer frame* (the global
 * reference frame in which the pose's position lives) from the *inner frame*
 * (the body frame of the object being posed). The Basic conformance classes
 * mandate WGS-84 geographic for the outer frame and an implicit body-axes
 * inner frame; the Advanced conformance class supports stacks of frames
 * with explicit identifiers.
 *
 * In MobilityDB v1 the Pose type encodes the outer frame implicitly via its
 * SRID and the inner frame by convention (right-handed body axes). This
 * table documents the mapping and seeds the registry with the well-known
 * frames so that a future Advanced-class lift only needs to extend it,
 * not re-design it.
 *
 * The shape mirrors pgPointCloud's `pointcloud_formats(pcid, …, schema)`
 * registry: a small primary-key-indexed catalog of named formats, with a
 * description column that's free-form for human readers.
 */

CREATE TABLE geopose_frames (
  frame_id      integer PRIMARY KEY,
  authority     text NOT NULL,
  code          text,
  name          text NOT NULL,
  srid          integer,
  is_geographic boolean NOT NULL DEFAULT false,
  description   text
);

COMMENT ON TABLE  geopose_frames IS
  'OGC GeoPose v1.0 frame metadata registry. v1 is informational only; '
  'the Pose type encodes the outer frame implicitly via SRID and uses '
  'a conventional right-handed body-axes inner frame.';
COMMENT ON COLUMN geopose_frames.frame_id   IS 'Stable integer key for cross-references.';
COMMENT ON COLUMN geopose_frames.authority  IS 'Naming authority: ''EPSG'', ''OGC'', ''CUSTOM''.';
COMMENT ON COLUMN geopose_frames.code       IS 'Authority-specific code (e.g., ''4326'').';
COMMENT ON COLUMN geopose_frames.name       IS 'Human-readable frame name.';
COMMENT ON COLUMN geopose_frames.srid       IS 'PostGIS SRID, or NULL if the frame is parametric (e.g., LTP at runtime).';
COMMENT ON COLUMN geopose_frames.is_geographic IS 'TRUE for lat/lon/h frames, FALSE for Cartesian / projected.';

INSERT INTO geopose_frames(frame_id, authority, code, name, srid, is_geographic, description) VALUES
  (1, 'EPSG', '4326', 'WGS-84 geographic (lat/lon/h)', 4326, true,
     'OGC GeoPose Basic-class default outer frame. Position parsed as {lat, lon, h} in degrees / metres.'),
  (2, 'EPSG', '4978', 'WGS-84 ECEF (Earth-Centred Earth-Fixed)', 4978, false,
     'Cartesian X/Y/Z geocentric. Used as an intermediate by frame transforms; rotation between this frame and WGS-84 geographic at point P is given by the East-North-Up basis at P.'),
  (3, 'OGC',  'LTP',  'Local Tangent Plane (East-North-Up)', NULL, false,
     'Parameterised at runtime by an anchor (lat, lon, h). The outer-frame conversion to ECEF is the standard ENU rotation matrix at the anchor.'),
  (4, 'OGC',  'BODY', 'Right-handed body axes (default inner frame)', NULL, false,
     'Conventional inner frame: X forward, Y left, Z up. The pose''s quaternion takes vectors from this body frame to the outer frame.');

/* Helper SQL functions to query the registry without exposing the schema. */

CREATE FUNCTION geopose_frame_srid(frame_id integer) RETURNS integer
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT srid FROM geopose_frames WHERE geopose_frames.frame_id = $1 $$;

CREATE FUNCTION geopose_frame_name(frame_id integer) RETURNS text
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT name FROM geopose_frames WHERE geopose_frames.frame_id = $1 $$;

CREATE FUNCTION geopose_frame_is_geographic(frame_id integer) RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT is_geographic FROM geopose_frames WHERE geopose_frames.frame_id = $1 $$;

/* Mark the catalog as a configuration table so pg_dump preserves the row
 * data (the seed) but lets users edit it in place. */
SELECT pg_catalog.pg_extension_config_dump('geopose_frames', '');

/*****************************************************************************/
