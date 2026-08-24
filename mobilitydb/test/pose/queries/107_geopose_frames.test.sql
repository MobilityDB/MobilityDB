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
SELECT geoPoseFrameSRID(1), geoPoseFrameIsGeographic(1);

-- WGS-84 ECEF (frame_id 2) is Cartesian.
SELECT geoPoseFrameName(2), geoPoseFrameIsGeographic(2);

-- The parametric frames (LTP, BODY) have no static SRID.
SELECT geoPoseFrameSRID(3) IS NULL AS ltp_srid_null,
       geoPoseFrameSRID(4) IS NULL AS body_srid_null;

-- Lookup of an unknown frame_id returns NULL (helper functions are STRICT but
-- the missing-row case yields NULL via the SQL body's empty result).
SELECT geoPoseFrameSRID(999) AS unknown_srid;

-- A user can register a custom frame.
INSERT INTO geopose_frames(frame_id, authority, code, name, srid, is_geographic, description)
VALUES (1000, 'CUSTOM', 'EXAMPLE', 'Example custom outer frame', 3857, false,
        'Web Mercator projection used for tile-aligned visualisation.');
SELECT geoPoseFrameSRID(1000), geoPoseFrameName(1000);

-- Cleanup the user-added row so the test doesn't leave state behind.
DELETE FROM geopose_frames WHERE frame_id = 1000;
SELECT count(*) FROM geopose_frames;

-- A frame is stated from one. (103_pose_geopose asserts that the registry
-- states every authority and id pair the encoder emits.)
INSERT INTO geopose_frames(frame_id, authority, name) VALUES (0, 'CUSTOM', 'Zero');
