-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Input
-------------------------------------------------------------------------------

SELECT asText(pose 'Pose(Point(1 1),0.5)');
SELECT asText(pose ' pose   (  Point  ( 1  1  ) ,	0.5   )   ');
/* Errors */
SELECT pose 'point(1,0.5)';
SELECT pose 'pose 1,0.5)';
SELECT pose 'Pose(Point(1 1),0.5';
SELECT pose 'Pose(Point(1 1) 0.5)';
SELECT pose 'Pose(Point(1 1)000,0.5)';
SELECT pose 'Pose(Point(1 1),-1.5)';
SELECT pose 'Pose(Point(1 1),0.5)xxx';

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT asText(pose('Point(1 1)', 0.5));
SELECT asText(pose(ST_Point(1,1), 0.5));
/* Errors */
SELECT pose('Linestring(1 1,2 2)',1.5);
SELECT pose('Point Z(1 1 1)',1.5);
SELECT pose('Point M(1 1 1)',1.5);
SELECT pose(geography 'Point(1 1)',1.5);
SELECT pose('Point(1 1)',-1.5);

-------------------------------------------------------------------------------
-- Accessing values
-------------------------------------------------------------------------------

SELECT ST_AsText(point(pose 'Pose(Point(1 1),0.5)'));
SELECT rotation(pose 'Pose(Point(1 1),0.5)');
SELECT srid(pose 'Pose(SRID=5676;Point(1 1),0.5)');

-- (yaw, pitch, roll) accessors. For a 2D pose the rotation theta is yaw;
-- pitch and roll are zero by definition. For a 3D pose these are the
-- ZYX intrinsic Tait-Bryan decomposition (the OGC GeoPose convention).
SELECT yaw(pose 'Pose(Point(1 1),0.5)');
SELECT pitch(pose 'Pose(Point(1 1),0.5)');
SELECT roll(pose 'Pose(Point(1 1),0.5)');
-- 3D identity quaternion: zero yaw / pitch / roll.
SELECT yaw(pose 'Pose(Point(0 0 0), 1, 0, 0, 0)');
SELECT pitch(pose 'Pose(Point(0 0 0), 1, 0, 0, 0)');
SELECT roll(pose 'Pose(Point(0 0 0), 1, 0, 0, 0)');

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT asText(round(pose 'Pose(Point(1.123456789 1.123456789), 0.123456789)', 6));

-- A 2D pose is returned unchanged (no quaternion to renormalise).
SELECT asText(normalise(pose 'Pose(Point(1 1), 0.5)'));
-- A unit-norm 3D pose round-trips identically.
SELECT asText(normalise(pose 'Pose(Point(1 1 1), 1, 0, 0, 0)'));
SELECT asText(normalise(pose 'Pose(Point(1 1 1), 0.5, 0.5, 0.5, 0.5)'));

-------------------------------------------------------------------------------
-- Cast functions 
-------------------------------------------------------------------------------

SELECT ST_AsText(round(pose 'Pose(Point(1 1),0.2)'::geometry, 6));

-- SELECT geometry 'SRID=5676;Point(610.455019399524 528.508247341961)'::pose;

-- NULL

-- /* Errors */

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

-- true
SELECT pose 'Pose(Point(1.000001 1),0.5)' ~= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1.000001),0.5)' ~= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5000001)' ~= pose 'Pose(Point(1 1),0.5)';
-- false
SELECT pose 'Pose(Point(1.00001 1),0.5)' ~= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1.00001),0.5)' ~= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.500001)' ~= pose 'Pose(Point(1 1),0.5)';

-------------------------------------------------------------------------------

SELECT pose 'Pose(Point(1 1),0.5)' = pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' = pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' = pose 'Pose(Point(2 2),0.5)';

SELECT pose 'Pose(Point(1 1),0.5)' != pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' != pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' != pose 'Pose(Point(2 2),0.5)';

SELECT pose 'Pose(Point(1 1),0.5)' < pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' < pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' < pose 'Pose(Point(2 2),0.5)';

SELECT pose 'Pose(Point(1 1),0.5)' <= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' <= pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' <= pose 'Pose(Point(2 2),0.5)';

SELECT pose 'Pose(Point(1 1),0.5)' > pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' > pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' > pose 'Pose(Point(2 2),0.5)';

SELECT pose 'Pose(Point(1 1),0.5)' >= pose 'Pose(Point(1 1),0.5)';
SELECT pose 'Pose(Point(1 1),0.5)' >= pose 'Pose(Point(1 1),0.7)';
SELECT pose 'Pose(Point(1 1),0.5)' >= pose 'Pose(Point(2 2),0.5)';

-------------------------------------------------------------------------------/

-- Cross-SRID frame transforms (workstream #6). The orientation correction
-- between WGS-84 geographic (4326) and WGS-84 ECEF (4978) is the local
-- East-North-Up basis change at the point. Round-trip lands at the input.
SELECT asEWKT(round(transform(transform(pose 'SRID=4326;Pose(Point(8 47 0), 1, 0, 0, 0)', 4978), 4326), 6));
-- At lat=lon=0 the ENU->ECEF rotation maps body identity to (0.5,-0.5,-0.5,-0.5).
SELECT asEWKT(round(transform(pose 'SRID=4326;Pose(Point(0 0 0), 1, 0, 0, 0)', 4978), 6));
-- Same-SRID transform is a no-op.
SELECT asEWKT(transform(pose 'SRID=4326;Pose(Point(8 47 0), 1, 0, 0, 0)', 4326));

-- applyPose — body↔world rigid transform via the pose's (R, p).
-- Identity pose: body geometry passes through unchanged.
SELECT ST_AsText(applyPose(ST_Point(1,2), pose 'Pose(Point(0 0), 0)'));
-- 2D pose with theta=pi/2 at (10,20): body (1,0) rotates to (0,1), then translates.
SELECT ST_AsText(applyPose(ST_Point(1,0), pose 'Pose(Point(10 20), 1.5707963267948966)'));
-- 3D identity quaternion + translate: body (1,0,0) shifts to (11,20,30).
SELECT ST_AsText(applyPose(ST_MakePoint(1,0,0), pose 'Pose(Point(10 20 30), 1, 0, 0, 0)'));
-- 3D 90deg yaw: body X axis rotates to world Y axis (epsilon-clean).
SELECT ST_AsText(applyPose(ST_MakePoint(1,0,0), pose 'Pose(Point(0 0 0), 0.7071067811865476, 0, 0, 0.7071067811865475)'));

-- Quaternion drift tolerance. Real sensor-fusion clients (IMUs, AR/VR
-- runtimes, physics engines) deliver quaternions with |q|=1+e where e
-- is up to ~1e-6 because they don't renormalise on every frame. These
-- are accepted within a 1e-3 tolerance and auto-renormalised on
-- construction, so the on-disk representation is always exactly unit
-- norm and downstream cmp/hash/SLERP code is independent of input
-- quality.
SELECT asEWKT(pose(0::float, 0::float, 0::float, 1.0000005::float, 0::float, 0::float, 0::float, 0));
SELECT asEWKT(pose(0::float, 0::float, 0::float, 0.5005::float, 0.5005::float, 0.5005::float, 0.5005::float, 0));
-- Way-off norms (|q|=2 here) are rejected as obvious bugs.
SELECT asEWKT(pose(0::float, 0::float, 0::float, 1::float, 1::float, 1::float, 1::float, 0));
-- Zero / NaN / Inf components are rejected up front (regardless of norm).
SELECT asEWKT(pose(0::float, 0::float, 0::float, 0::float, 0::float, 0::float, 0::float, 0));
SELECT asEWKT(pose(0::float, 0::float, 0::float, 'NaN'::float, 0::float, 0::float, 0::float, 0));

-- Quaternion double-cover canonicalization audit: q and -q represent the
-- same orientation, so every construction path must canonicalize to a
-- single representative (chosen as W >= 0). Without this invariant the
-- byte-level B-tree opclass and hash opclass would treat q and -q as
-- distinct values and break distinct-set / GROUP BY semantics on poseset.
-- Exercises the four entry points (WKT parser, constructor, WKB recv,
-- approximate equality) plus pose_hash to confirm they all agree.
SELECT pose 'Pose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)' = pose 'Pose(Point(0 0 0), -0.5, -0.5, -0.5, -0.5)' AS wkt_canonical;
SELECT pose(0::float, 0::float, 0::float, 0.5::float, 0.5::float, 0.5::float, 0.5::float, 0)
     = pose(0::float, 0::float, 0::float, -0.5::float, -0.5::float, -0.5::float, -0.5::float, 0) AS ctor_canonical;
SELECT poseFromBinary(asBinary(pose 'Pose(Point(0 0 0), -0.5, -0.5, -0.5, -0.5)'))
     = pose 'Pose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)' AS wkb_canonical;
SELECT pose_hash(pose 'Pose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)')
     = pose_hash(pose 'Pose(Point(0 0 0), -0.5, -0.5, -0.5, -0.5)') AS hash_canonical;
SELECT pose 'Pose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)' ~= pose 'Pose(Point(0 0 0), -0.5, -0.5, -0.5, -0.5)' AS approx_canonical;
