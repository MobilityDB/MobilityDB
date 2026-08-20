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
-- Tests for the pose chain set type
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Input/output
-- A set holds its values ordered and without duplicates, and the frame of the
-- set is the frame its chains agree on.
-------------------------------------------------------------------------------

SELECT asEWKT(posechainset '{"PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))"}');
SELECT asEWKT(posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}');
-- Duplicates collapse and the values come out ordered
SELECT asEWKT(posechainset '{"PoseChain(Pose(Point(1 0), 0))", "PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}');
SELECT asEWKT(posechainset '{"SRID=3812;PoseChain(Pose(Point(1 2 3), 1, 0, 0, 0), Pose(Point(0 0 1), 1, 0, 0, 0))"}');
SELECT asEWKT(posechainset '{"SRID=4326;PoseChain(GeodPose(Point(8 47 0), 1, 0, 0, 0))"}');

-- Text and binary representations round trip
SELECT asEWKT(posechainsetFromEWKT(asEWKT(posechainset '{"PoseChain(Pose(Point(1 2), 0.5), Pose(Point(3 0), 0.25))"}')));
SELECT posechainsetFromBinary(asBinary(posechainset '{"PoseChain(Pose(Point(1 2), 0.5))", "PoseChain(Pose(Point(3 0), 0.25))"}')) =
  posechainset '{"PoseChain(Pose(Point(1 2), 0.5))", "PoseChain(Pose(Point(3 0), 0.25))"}';
-- EWKB is the representation that carries the frame; plain WKB does not
SELECT posechainsetFromEWKB(asEWKB(posechainset '{"SRID=3812;PoseChain(Pose(Point(1 2 3), 1, 0, 0, 0))"}')) =
  posechainset '{"SRID=3812;PoseChain(Pose(Point(1 2 3), 1, 0, 0, 0))"}';
SELECT posechainsetFromHexWKB(asHexWKB(posechainset '{"PoseChain(Pose(Point(1 2), 0.5))"}')) =
  posechainset '{"PoseChain(Pose(Point(1 2), 0.5))"}';

-------------------------------------------------------------------------------
-- Errors
-- The chains of a set share one frame and one dimension, as the links of a
-- chain do.
-------------------------------------------------------------------------------

SELECT posechainset '{"SRID=3812;PoseChain(Pose(Point(0 0), 0))", "SRID=4326;PoseChain(Pose(Point(1 1), 0))"}';
SELECT posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 2 3), 1, 0, 0, 0))"}';
SELECT posechainset '{}';

-------------------------------------------------------------------------------
-- Constructors and conversions
-------------------------------------------------------------------------------

SELECT asEWKT(set(posechain 'PoseChain(Pose(Point(1 2), 0.5), Pose(Point(3 0), 0.25))'));
SELECT asEWKT((posechain 'PoseChain(Pose(Point(1 2), 0.5))')::posechainset);
SELECT asEWKT(set(ARRAY[posechain 'PoseChain(Pose(Point(0 0), 0))',
  posechain 'PoseChain(Pose(Point(1 0), 0))']));

-- The box of a set spans every joint of every chain it holds
SELECT round(stbox(posechainset '{"PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))", "PoseChain(Pose(Point(1 5), 0))"}'), 6);
SELECT round((posechainset '{"PoseChain(Pose(Point(0 0), 0))"}')::stbox, 6);

-------------------------------------------------------------------------------
-- Transformations
-------------------------------------------------------------------------------

SELECT asEWKT(round(posechainset '{"PoseChain(Pose(Point(1.123456789 2.123456789), 0.5))"}', 3));

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT memSize(posechainset '{"PoseChain(Pose(Point(0 0), 0))"}') > 0;
SELECT numValues(posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))", "PoseChain(Pose(Point(2 0), 0))"}');
SELECT asEWKT(startValue(posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}'));
SELECT asEWKT(endValue(posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}'));
SELECT asEWKT(valueN(posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}', 2));

-- Out of range yields NULL
SELECT valueN(posechainset '{"PoseChain(Pose(Point(0 0), 0))"}', 99);

SELECT asEWKT(getValues(posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}'));

-------------------------------------------------------------------------------
-- Spatial reference system
-------------------------------------------------------------------------------

SELECT SRID(posechainset '{"SRID=3812;PoseChain(Pose(Point(1 2), 0.5))"}');
SELECT SRID(setSRID(posechainset '{"PoseChain(Pose(Point(1 2), 0.5))"}', 3812));
-- Transforming and transforming back returns the values it started from
SELECT asEWKT(round(transform(transform(posechainset '{"SRID=4326;PoseChain(Pose(Point(8 47), 0.5))"}', 3812), 4326), 6));

-------------------------------------------------------------------------------
-- unnest: SETOF expansion
-------------------------------------------------------------------------------

SELECT COUNT(*) = numValues(posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))", "PoseChain(Pose(Point(2 0), 0))"}')
FROM unnest(posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))", "PoseChain(Pose(Point(2 0), 0))"}');

-------------------------------------------------------------------------------
-- setUnion aggregate
-------------------------------------------------------------------------------

SELECT setUnion(v) = posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}'
FROM (VALUES (posechain 'PoseChain(Pose(Point(0 0), 0))'),
             (posechain 'PoseChain(Pose(Point(1 0), 0))'),
             (posechain 'PoseChain(Pose(Point(0 0), 0))')) AS t(v);

SELECT setUnion(s) = posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))", "PoseChain(Pose(Point(2 0), 0))"}'
FROM (VALUES (posechainset '{"PoseChain(Pose(Point(0 0), 0))"}'),
             (posechainset '{"PoseChain(Pose(Point(1 0), 0))", "PoseChain(Pose(Point(2 0), 0))"}'),
             (posechainset '{"PoseChain(Pose(Point(0 0), 0))"}')) AS t(s);

-------------------------------------------------------------------------------
-- Comparisons, btree and hash
-------------------------------------------------------------------------------

SELECT posechainset '{"PoseChain(Pose(Point(0 0), 0))"}' = posechainset '{"PoseChain(Pose(Point(0 0), 0))"}';
SELECT posechainset '{"PoseChain(Pose(Point(0 0), 0))"}' <> posechainset '{"PoseChain(Pose(Point(1 0), 0))"}';
SELECT posechainset '{"PoseChain(Pose(Point(0 0), 0))"}' < posechainset '{"PoseChain(Pose(Point(1 0), 0))"}';
SELECT cmp(posechainset '{"PoseChain(Pose(Point(0 0), 0))"}', posechainset '{"PoseChain(Pose(Point(0 0), 0))"}');
SELECT hash(posechainset '{"PoseChain(Pose(Point(0 0), 0))"}') =
  hash(posechainset '{"PoseChain(Pose(Point(0 0), 0))"}');
SELECT hashExtended(posechainset '{"PoseChain(Pose(Point(0 0), 0))"}', 1) =
  hashExtended(posechainset '{"PoseChain(Pose(Point(0 0), 0))"}', 1);

DROP TABLE IF EXISTS tbl_posechainset_test;
CREATE TABLE tbl_posechainset_test(k int, s posechainset);
INSERT INTO tbl_posechainset_test VALUES
  (1, posechainset '{"PoseChain(Pose(Point(0 0), 0))"}'),
  (2, posechainset '{"PoseChain(Pose(Point(1 0), 0))"}'),
  (3, posechainset '{"PoseChain(Pose(Point(0 0), 0))"}'),
  (4, posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}');

SELECT COUNT(DISTINCT s) FROM tbl_posechainset_test;
DROP TABLE tbl_posechainset_test;

-------------------------------------------------------------------------------
-- Set operations
-------------------------------------------------------------------------------

SELECT posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}' @> posechain 'PoseChain(Pose(Point(1 0), 0))';
SELECT posechain 'PoseChain(Pose(Point(1 0), 0))' <@ posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}';
SELECT posechainset '{"PoseChain(Pose(Point(0 0), 0))"}' @> posechainset '{"PoseChain(Pose(Point(0 0), 0))"}';
SELECT posechainset '{"PoseChain(Pose(Point(0 0), 0))"}' && posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}';

SELECT asEWKT(posechainset '{"PoseChain(Pose(Point(0 0), 0))"}' + posechain 'PoseChain(Pose(Point(1 0), 0))');
SELECT asEWKT(posechainset '{"PoseChain(Pose(Point(0 0), 0))"}' + posechainset '{"PoseChain(Pose(Point(1 0), 0))"}');
SELECT asEWKT(posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}' - posechain 'PoseChain(Pose(Point(1 0), 0))');
SELECT asEWKT(posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}' * posechainset '{"PoseChain(Pose(Point(1 0), 0))"}');
SELECT asEWKT(posechain 'PoseChain(Pose(Point(1 0), 0))' * posechainset '{"PoseChain(Pose(Point(0 0), 0))", "PoseChain(Pose(Point(1 0), 0))"}');

-- Mixing frames in a set operation is an error, as it is between two chains
SELECT posechainset '{"SRID=3812;PoseChain(Pose(Point(0 0), 0))"}' @> posechain 'SRID=4326;PoseChain(Pose(Point(0 0), 0))';

-------------------------------------------------------------------------------
