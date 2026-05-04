-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------
-- GeoPose v1.0 frame metadata registry — exercise the seeded rows and the
-- helper SQL functions that query the registry.
-------------------------------------------------------------------------------

-- The registry is seeded with the four well-known frames.
SELECT count(*) FROM geopose_frames;

-- WGS-84 geographic is frame_id 1; SRID maps to PostGIS 4326.
SELECT geopose_frame_srid(1), geopose_frame_is_geographic(1);

-- WGS-84 ECEF (frame_id 2) is Cartesian.
SELECT geopose_frame_name(2), geopose_frame_is_geographic(2);

-- The parametric frames (LTP, BODY) have no static SRID.
SELECT geopose_frame_srid(3) IS NULL AS ltp_srid_null,
       geopose_frame_srid(4) IS NULL AS body_srid_null;

-- Lookup of an unknown frame_id returns NULL (helper functions are STRICT but
-- the missing-row case yields NULL via the SQL body's empty result).
SELECT geopose_frame_srid(999) AS unknown_srid;

-- A user can register a custom frame.
INSERT INTO geopose_frames(frame_id, authority, code, name, srid, is_geographic, description)
VALUES (1000, 'CUSTOM', 'EXAMPLE', 'Example custom outer frame', 3857, false,
        'Web Mercator projection used for tile-aligned visualisation.');
SELECT geopose_frame_srid(1000), geopose_frame_name(1000);

-- Cleanup the user-added row so the test doesn't leave state behind.
DELETE FROM geopose_frames WHERE frame_id = 1000;
SELECT count(*) FROM geopose_frames;
