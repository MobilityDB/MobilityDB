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
\set VERBOSITY terse
SELECT pose(geography 'Point(1 1)',1.5);
\set VERBOSITY default
SELECT pose('Point(1 1)',-1.5);

-- The Basic-YPR constructor: the orientation given as yaw, pitch and roll
-- in radians rather than as a quaternion.
SELECT asText(pose(ST_PointZ(1,1,1), 0, 0, 0));
SELECT asText(pose(ST_PointZ(1,1,1), radians(90), 0, 0), 6);
SELECT asText(pose(ST_PointZ(1,1,1), radians(30), radians(45), radians(60)), 6);
-- It round-trips against the ypr accessor.
SELECT round(degrees(yaw(pose(ST_PointZ(0,0,0), radians(30), radians(45),
  radians(60))))::numeric, 6);
\set VERBOSITY terse
-- A 2D point has no 3D orientation to carry.
SELECT asText(pose(ST_Point(1,1), 0, 0, 0));
SELECT asText(pose(ST_PointZ(1,1,1), 'NaN', 0, 0));
\set VERBOSITY default

-------------------------------------------------------------------------------
-- Accessing values
-------------------------------------------------------------------------------

SELECT ST_AsText(point(pose 'Pose(Point(1 1),0.5)'));
SELECT yaw(pose 'Pose(Point(1 1),0.5)');
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

-- The orientation quaternion of a pose, in the (W, X, Y, Z) Hamilton order a
-- 3D pose stores. Defined for both dimensions: the orientation of a 2D pose
-- is a turn about the local vertical by its stored angle, which is the
-- quaternion the GeoPose Basic-Quaternion encoder writes for it.
SELECT quaternion(pose 'Pose(Point(0 0 0), 1, 0, 0, 0)');
SELECT quaternion(pose 'Pose(Point Z(1 1 1), 0, 0, 0, 1)');
-- A 120-degree turn about (1 1 1). Its components are exactly
-- representable, so the unit-norm renormalization leaves them alone and
-- the output carries no libm-dependent trailing digits.
SELECT quaternion(pose 'Pose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)');
-- An unturned 2D pose carries the identity quaternion exactly.
SELECT quaternion(pose 'Pose(Point(1 1), 0)');
-- A turned 2D pose puts half its angle in W and Z and leaves X and Y at
-- zero, since it neither pitches nor rolls. The two turning components are
-- bounded because the cosine and sine of a general angle drift across libm.
SELECT round((quaternion(pose 'Pose(Point(1 1),0.5)')).W::numeric, 6),
  (quaternion(pose 'Pose(Point(1 1),0.5)')).X,
  (quaternion(pose 'Pose(Point(1 1),0.5)')).Y,
  round((quaternion(pose 'Pose(Point(1 1),0.5)')).Z::numeric, 6);
-- The two encodings answer one orientation: the yaw read back from the
-- quaternion of a 2D pose is the angle that pose stores.
SELECT round(yaw(pose(ST_MakePoint(1, 1, 0),
  (quaternion(pose 'Pose(Point(1 1),0.5)')).W,
  (quaternion(pose 'Pose(Point(1 1),0.5)')).X,
  (quaternion(pose 'Pose(Point(1 1),0.5)')).Y,
  (quaternion(pose 'Pose(Point(1 1),0.5)')).Z))::numeric, 6) = 0.5;

-- The same orientation in the other GeoPose encoding. Defined for both
-- dimensions: a 2D pose yaws by its stored angle and neither pitches nor
-- rolls.
SELECT ypr(pose 'Pose(Point(1 1),0.5)');
SELECT ypr(pose 'Pose(Point Z(1 1 1), 1, 0, 0, 0)');
-- Bounded in degrees: an unbounded pi/2 would drift across libm.
SELECT round(degrees((ypr(pose 'Pose(Point Z(1 1 1), 0.5, 0.5, 0.5, 0.5)')).yaw)::numeric, 6),
  round(degrees((ypr(pose 'Pose(Point Z(1 1 1), 0.5, 0.5, 0.5, 0.5)')).pitch)::numeric, 6),
  round(degrees((ypr(pose 'Pose(Point Z(1 1 1), 0.5, 0.5, 0.5, 0.5)')).roll)::numeric, 6);

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT asText(round(pose 'Pose(Point(1.123456789 1.123456789), 0.123456789)', 6));

-- A 2D pose is returned unchanged (it stores no quaternion to renormalize).
SELECT asText(poseNormalize(pose 'Pose(Point(1 1), 0.5)'));
-- A unit-norm 3D pose round-trips identically.
SELECT asText(poseNormalize(pose 'Pose(Point(1 1 1), 1, 0, 0, 0)'));
SELECT asText(poseNormalize(pose 'Pose(Point(1 1 1), 0.5, 0.5, 0.5, 0.5)'));

-- Both 3D constructors hold the unit-norm invariant: a quaternion within
-- the accepted drift is renormalized before it is stored, whether the
-- position arrives as coordinates or as a point geometry.
SELECT asText(pose(1, 1, 1, 1.0005, 0, 0, 0), 12);
SELECT asText(pose(ST_PointZ(1,1,1), 1.0005, 0, 0, 0), 12);

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

-- Cross-SRID frame transforms. The orientation correction
-- between WGS-84 geographic (4326) and WGS-84 ECEF (4978) is the local
-- East-North-Up basis change at the point. Round-trip lands at the input.
SELECT asEWKT(round(transform(transform(pose 'SRID=4326;Pose(Point(8 47 0), 1, 0, 0, 0)', 4978), 4326), 6));
-- A round trip cannot see the direction of the orientation correction, since an
-- inverse rotation composed with itself restores the input either way. These
-- one-way transforms can: at the equator-meridian the East, North and Up axes
-- point along geocentric +Y, +Z and +X, so a body at rest in the local frame
-- carries that basis change and no other.
SELECT asEWKT(round(transform(pose 'SRID=4326;Pose(Point(0 0 0), 1, 0, 0, 0)', 4978), 6));
-- Away from the equator the rotation and its inverse share a yaw and a pitch
-- and differ in the sign of the roll, which is 90 degrees less the latitude.
-- The angles are read rather than the quaternion because they bound their own
-- precision, which round(pose) cannot do for a quaternion: it renormalizes the
-- rounded components back to unit length.
SELECT round(degrees(yaw(transform(pose 'SRID=4326;Pose(Point(0 45 0), 1, 0, 0, 0)', 4978)))::numeric, 6) AS yaw,
  round(degrees(pitch(transform(pose 'SRID=4326;Pose(Point(0 45 0), 1, 0, 0, 0)', 4978)))::numeric, 6) AS pitch,
  round(degrees(roll(transform(pose 'SRID=4326;Pose(Point(0 45 0), 1, 0, 0, 0)', 4978)))::numeric, 6) AS roll;
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
-- A body geometry of any type is carried into the pose's frame with its
-- shape unchanged: the transform reaches every coordinate.
SELECT ST_AsText(applyPose(ST_MakeEnvelope(-2,-1,2,1), pose 'Pose(Point(10 20), 0)'));
SELECT ST_AsText(ST_SnapToGrid(applyPose(ST_MakeEnvelope(0,0,2,1),
  pose(ST_Point(0,0), pi()/2)), 1e-9));
SELECT ST_AsText(applyPose(geometry 'Linestring(0 0,1 0,1 1)', pose 'Pose(Point(5 5), 0)'));
SELECT ST_AsText(applyPose(geometry 'Multipoint(0 0,1 1)', pose 'Pose(Point(10 20), 0)'));
-- The transform reads the body geometry, it does not move it
WITH test(g) AS (SELECT geometry 'Point(1 2)')
SELECT ST_AsText(applyPose(g, pose 'Pose(Point(10 20), 0)')), ST_AsText(g) FROM test;
-- A pose and its body geometry must agree on dimensionality, and an empty
-- body geometry names no point to carry
SELECT applyPose(geometry 'Polygon Empty', pose 'Pose(Point(1 1), 0)');
SELECT applyPose(geometry 'Point(1 2)', pose 'Pose(Point(1 1 1), 1, 0, 0, 0)');
-- The temporal form answers a tgeompoint, so its body geometry is a point
SELECT asText(applyPose(geometry 'Polygon((0 0,1 0,1 1,0 0))',
  tpose '[Pose(Point(0 0), 0)@2026-01-01, Pose(Point(10 20), 1)@2026-01-02]'));
-- The pose names the frame the result is expressed in, so the two SRIDs must
-- agree; an unknown one adopts the other
SELECT ST_SRID(applyPose(ST_SetSRID(geometry 'Point(1 2)', 4326),
  pose 'Pose(Point(0 0), 0)'));
SELECT ST_SRID(applyPose(geometry 'Point(1 2)', pose 'SRID=5676;Pose(Point(0 0), 0)'));
SELECT ST_SRID(applyPose(geometry 'Point(1 2)', pose 'Pose(Point(0 0), 0)'));
SELECT applyPose(ST_SetSRID(geometry 'Point(1 2)', 4326),
  pose 'SRID=5676;Pose(Point(0 0), 0)');
SELECT applyPose(ST_SetSRID(geometry 'Point(1 2)', 4326),
  tpose 'SRID=5676;Pose(Point(0 0), 0)@2001-01-01');

-- Quaternion drift tolerance. Real sensor-fusion clients (IMUs, AR/VR
-- runtimes, physics engines) deliver quaternions with |q|=1+e where e
-- is up to ~1e-6 because they don't renormalize on every frame. These
-- are accepted within a 1e-3 tolerance and auto-renormalized on
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
SELECT poseFromHexEWKB(asHexWKB(pose 'SRID=3812;Pose(Point(1 2),1)'))
     = pose 'SRID=3812;Pose(Point(1 2),1)' AS hexwkb_roundtrip;
SELECT asHexWKB(pose 'SRID=3812;Pose(Point(1 2),1)')
     = asHexEWKB(pose 'SRID=3812;Pose(Point(1 2),1)') AS hexwkb_eq_hexewkb;
SELECT hash(pose 'Pose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)')
     = hash(pose 'Pose(Point(0 0 0), -0.5, -0.5, -0.5, -0.5)') AS hash_canonical;
SELECT pose 'Pose(Point(0 0 0), 0.5, 0.5, 0.5, 0.5)' ~= pose 'Pose(Point(0 0 0), -0.5, -0.5, -0.5, -0.5)' AS approx_canonical;

-- A pose read back from its binary form is the pose that was written, so the
-- binary form is lossless and COPY BINARY carries a table unchanged. The
-- quaternion is (1, 1, 1, 2) scaled to unit norm, one of those whose scaled
-- components move again when scaled a second time; an exactly representable
-- quaternion such as (0.5, 0.5, 0.5, 0.5) survives any rescaling and so
-- cannot tell a lossless round trip from a lossy one.
SELECT poseFromBinary(asBinary(p)) = p AS wkb_roundtrip_identity
FROM (SELECT pose(ST_MakePoint(1, 2, 3), 1 / n, 1 / n, 1 / n, 2 / n) AS p
  FROM (SELECT sqrt(1 + 1 + 1 + 4) AS n) s) t;

-------------------------------------------------------------------------------
-- Geodetic poses (planar/geodetic support, uniform with the stbox GEODSTBOX form)
-------------------------------------------------------------------------------

SELECT asText(pose 'GeodPose(Point(1 1),0.5)');
SELECT asText(pose 'SRID=4326;GeodPose(Point(1 1),0.5)');
SELECT asText(pose 'GeodPose(Point(1 1 1),1,0,0,0)');
SELECT pose 'GeodPose(Point(1 1),0.5)';
SELECT poseFromBinary(asBinary(pose 'GeodPose(Point(1 1),0.5)'));
SELECT poseFromBinary(asBinary(pose 'GeodPose(Point(1 1 1),1,0,0,0)'));

-------------------------------------------------------------------------------
-- Composition
-- applyPose carries a value expressed in the frame a pose names into the frame
-- that pose is itself expressed in, so applying one pose to another composes
-- the two frame relationships.
-------------------------------------------------------------------------------

-- A sensor one metre ahead of a vehicle that sits at the origin unturned
SELECT asEWKT(round(applyPose(pose 'Pose(Point(1 0), 0)',
  pose 'Pose(Point(0 0), 0)'), 6));
-- The same vehicle turned a quarter turn: the sensor is now one metre North
SELECT asEWKT(round(applyPose(pose 'Pose(Point(1 0), 0)',
  pose(ST_Point(0,0), pi()/2)), 6));
-- The vehicle away from the origin as well
SELECT asEWKT(round(applyPose(pose 'Pose(Point(1 0), 0)',
  pose(ST_Point(10,5), pi()/2)), 6));
-- The orientations add
SELECT asEWKT(round(applyPose(pose(ST_Point(0,0), pi()/4),
  pose(ST_Point(0,0), pi()/4)), 6));
-- Composing agrees with the pose chain of the same two links, which is the
-- same operation folded over a chain
SELECT asEWKT(round(applyPose(pose 'Pose(Point(1 0), 0)',
  pose(ST_Point(10,5), pi()/2)), 6)) =
  asEWKT(round(pose(posechain(ARRAY[pose(ST_Point(10,5), pi()/2),
    pose 'Pose(Point(1 0), 0)'])), 6));
-- Three dimensions
SELECT asEWKT(round(applyPose(pose 'Pose(Point(1 0 0), 1, 0, 0, 0)',
  pose 'Pose(Point(0 0 5), 1, 0, 0, 0)'), 6));
-- Mixing the frames of the two poses is an error
SELECT applyPose(pose 'SRID=3812;Pose(Point(1 0), 0)',
  pose 'SRID=4326;Pose(Point(0 0), 0)');
SELECT applyPose(pose 'Pose(Point(1 0), 0)',
  pose 'Pose(Point(0 0 1), 1, 0, 0, 0)');

-------------------------------------------------------------------------------
-- Inverse
-- The inverse reverses the frame relationship, so composing a pose with its
-- inverse gives the identity.
-------------------------------------------------------------------------------

SELECT asEWKT(round(poseInverse(pose 'Pose(Point(1 0), 0)'), 6));
SELECT asEWKT(round(poseInverse(pose(ST_Point(0,0), pi()/2)), 6));
SELECT asEWKT(round(poseInverse(pose(ST_Point(10,5), pi()/2)), 6));
-- A pose composed with its inverse is the identity, in both directions
SELECT asEWKT(round(applyPose(poseInverse(pose(ST_Point(10,5), pi()/2)),
  pose(ST_Point(10,5), pi()/2)), 6));
SELECT asEWKT(round(applyPose(pose(ST_Point(10,5), pi()/2),
  poseInverse(pose(ST_Point(10,5), pi()/2))), 6));
-- Three dimensions: a half turn about Z, one metre along X
SELECT asEWKT(round(poseInverse(pose 'Pose(Point(1 0 0), 0, 0, 0, 1)'), 6));
SELECT asEWKT(round(applyPose(poseInverse(pose 'Pose(Point(1 0 0), 0, 0, 0, 1)'),
  pose 'Pose(Point(1 0 0), 0, 0, 0, 1)'), 6));
-- The inverse is its own inverse
SELECT asEWKT(round(poseInverse(poseInverse(pose(ST_Point(10,5), pi()/2))), 6));
-- A pose over a geographic frame names no frame of the ellipsoid to reverse
SELECT poseInverse(pose 'SRID=4326;GeodPose(Point(8 47), 0.5)');

-------------------------------------------------------------------------------
-- Composition and inversion lifted over time
-- A frame that moves carries what it names through the whole of its movement.
-------------------------------------------------------------------------------

-- A fixed sensor one unit ahead of a moving body: the body ends at (10,20)
-- yawed a quarter turn, so the sensor ends one unit north of it.
SELECT asEWKT(round(applyPose(pose 'Pose(Point(1 0), 0)',
  tposeSeq(ARRAY[tpose(pose(ST_Point(0,0), 0), timestamptz '2026-01-01'),
    tpose(pose(ST_Point(10,20), pi()/2), timestamptz '2026-01-02')])), 6));

-- The other order carries the moving body into the fixed frame instead, so
-- the same unit offset lands east rather than north.
SELECT asEWKT(round(applyPose(
  tposeSeq(ARRAY[tpose(pose(ST_Point(0,0), 0), timestamptz '2026-01-01'),
    tpose(pose(ST_Point(10,20), pi()/2), timestamptz '2026-01-02')]),
  pose 'Pose(Point(1 0), 0)'), 6));

-- Two moving frames agree with the static body when the body holds still
SELECT asEWKT(round(applyPose(
  tpose '[Pose(Point(1 0), 0)@2026-01-01, Pose(Point(1 0), 0)@2026-01-02]',
  tposeSeq(ARRAY[tpose(pose(ST_Point(0,0), 0), timestamptz '2026-01-01'),
    tpose(pose(ST_Point(10,20), pi()/2), timestamptz '2026-01-02')])), 6));

-- Two moving frames meet on the time they share
SELECT asEWKT(round(applyPose(
  tpose '[Pose(Point(1 0), 0)@2026-01-01, Pose(Point(1 0), 0)@2026-01-03]',
  tpose '[Pose(Point(0 0), 0)@2026-01-02, Pose(Point(10 20), 0)@2026-01-04]'), 6));

-- The frame seen from the body, for the whole of the movement
SELECT asEWKT(round(poseInverse(
  tposeSeq(ARRAY[tpose(pose(ST_Point(0,0), 0), timestamptz '2026-01-01'),
    tpose(pose(ST_Point(10,20), pi()/2), timestamptz '2026-01-02')])), 6));

-- A temporal pose composed with its own inverse is the identity at every
-- instant
SELECT asEWKT(round(applyPose(
  poseInverse(tpose '[Pose(Point(3 4), 0.5)@2026-01-01, Pose(Point(10 20), 1.2)@2026-01-02]'),
  tpose '[Pose(Point(3 4), 0.5)@2026-01-01, Pose(Point(10 20), 1.2)@2026-01-02]'), 6));

/* Errors */

-- A temporal pose over a geographic frame names no frame of the ellipsoid to
-- reverse
SELECT poseInverse(tpose 'SRID=4326;GeodPose(Point(8 47), 0.5)@2026-01-01');

-- Mixing the dimensions of two frames is an error, lifted as it is static
SELECT applyPose(pose 'Pose(Point(1 0 0), 1, 0, 0, 0)',
  tpose '[Pose(Point(0 0), 0)@2026-01-01, Pose(Point(1 1), 0)@2026-01-02]');

-------------------------------------------------------------------------------
