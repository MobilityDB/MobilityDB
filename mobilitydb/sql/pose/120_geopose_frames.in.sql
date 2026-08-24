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
 * The shape is the one `pointcloud_schemas` carries: a small
 * primary-key-indexed catalog stated in SQL, marked as a configuration table
 * so that pg_dump preserves the rows a user registers, with a description
 * column that is free-form for human readers.
 */

CREATE TABLE geopose_frames (
  frame_id      integer PRIMARY KEY CHECK (frame_id > 0),
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
COMMENT ON COLUMN geopose_frames.authority  IS
  'Naming authority: ''EPSG'' or ''OGC'' for a frame those bodies name, '
  '''/geopose/1.0'' for one the standard names in a document, and '
  '''CUSTOM'' for one a user registers.';
COMMENT ON COLUMN geopose_frames.code       IS 'Authority-specific code (e.g., ''4326'').';
COMMENT ON COLUMN geopose_frames.name       IS 'Human-readable frame name.';
COMMENT ON COLUMN geopose_frames.srid       IS 'PostGIS SRID, or NULL if the frame is parametric (e.g., LTP at runtime).';
COMMENT ON COLUMN geopose_frames.is_geographic IS 'TRUE for lat/lon/h frames, FALSE for Cartesian / projected.';

/* The frames the standard and the encoders name come from MEOS, which builds
 * them from the very macros pose_geopose.c writes, so a frame this build can
 * emit cannot be absent from the registry. A user registers further frames by
 * inserting into the table. */

CREATE FUNCTION geoPoseFrames()
  RETURNS TABLE (frame_id integer, authority text, code text, name text,
                 srid integer, is_geographic boolean, description text)
  AS 'MODULE_PATHNAME', 'Geopose_frames'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

INSERT INTO geopose_frames(frame_id, authority, code, name, srid,
  is_geographic, description)
SELECT frame_id, authority, code, name, srid, is_geographic, description
FROM geoPoseFrames();

/* Helper SQL functions to query the registry without exposing the schema. */

CREATE FUNCTION geoPoseFrameSRID(frame_id integer) RETURNS integer
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT srid FROM geopose_frames WHERE geopose_frames.frame_id = $1 $$;

CREATE FUNCTION geoPoseFrameName(frame_id integer) RETURNS text
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT name FROM geopose_frames WHERE geopose_frames.frame_id = $1 $$;

CREATE FUNCTION geoPoseFrameIsGeographic(frame_id integer) RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT is_geographic FROM geopose_frames WHERE geopose_frames.frame_id = $1 $$;

/* Mark the catalog as a configuration table so pg_dump preserves the row
 * data (the seed) but lets users edit it in place. */
SELECT pg_catalog.pg_extension_config_dump('geopose_frames', '');

/*****************************************************************************/
