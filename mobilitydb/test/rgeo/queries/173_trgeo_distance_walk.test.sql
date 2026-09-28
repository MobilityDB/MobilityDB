-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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
-- Temporal distance computed by the closest-feature walk.
--
-- While the bodies overlap, the distance is zero.  The comparisons at the end
-- check the nearest approach distance against the minimum of the distance at
-- many sampled times, for rotating bodies.  This finds an algorithm that
-- misses an extremum, which the other tests do not.
-------------------------------------------------------------------------------

-- A square sweeps over a point: the distance is zero from entry to exit
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),0)@2001-01-02]',
  geometry 'Point(0 0.5)'), 6);
-- The same body rotating a quarter turn as it passes
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),1.5707963267948966)@2001-01-02]',
  geometry 'Point(0 0.5)'), 6);
-- A point passing exactly through a corner of the body
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(2 -2),0)@2001-01-01, Pose(Point(-2 2),0)@2001-01-02]',
  geometry 'Point(0 0)'), 6);
-- Starting inside, and inside for the whole sequence
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-02]',
  geometry 'Point(0 0)'), 6);
SELECT tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0.2 0.1),0.3)@2001-01-02]',
  geometry 'Point(0 0)');

-- Step interpolation: the body holds its pose, and so does the distance
SELECT round(tDistance(
  trgeometry 'Interp=Step;Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0.5)@2001-01-02]',
  geometry 'Point(5 3)'), 6);

-- A body that is not convex is refused, also with step interpolation, so that
-- the distance functions accept the same values in all cases
SELECT tDistance(
  trgeometry 'Polygon((0 0,4 0,4 4,2 1,0 4,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Point(7 3)');
SELECT round(tDistance(
  trgeometry 'Interp=Step;Polygon((0 0,4 0,4 4,2 1,0 4,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Point(7 3)'), 6);
SELECT nearestApproachInstant(
  trgeometry 'Polygon((0 0,4 0,4 4,2 1,0 4,0 0));Pose(Point(0 0),0)@2001-01-01',
  geometry 'Point(7 3)');
-- A ring with a spike is not convex: the end of the spike changes the distance
SELECT tDistance(
  trgeometry 'Polygon((0 0,3 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),0)@2001-01-02]',
  geometry 'Point(4 0)');

-- A polygon with thousands of vertices that turns.  The walk takes about one
-- step for each vertex that it passes.  A vertex points to the point during
-- the turn, thus the nearest approach distance is sqrt(1.5^2 + 0.2^2) - 1.
-- The result is exact, although the normalization of the temporal distance
-- removes most of its instants, which are a few seconds apart.
SELECT abs(nearestApproachDistance(
  trgeometry(ST_Buffer(geometry 'Point(0 0)', 1, 1250),
    tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.1)@2001-01-02]'),
  geometry 'Point(1.5 0.2)') - (sqrt(2.29) - 1)) < 1e-12;

-- A sequence set is the distance of each of its sequences
SELECT round(tDistance(
  trgeometry 'Polygon((0 0,2 0,2 1,0 1,0 0));{[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),1)@2001-01-02], [Pose(Point(10 5),1)@2001-01-03, Pose(Point(0 5),-1)@2001-01-04]}',
  geometry 'Point(5 0.5)'), 6);

-- Static polygons: a body crossing one, and a body turning past one
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-8 0),0)@2001-01-01, Pose(Point(8 0),0)@2001-01-02]',
  geometry 'Polygon((-0.5 -3,0.5 -3,0.5 3,-0.5 3,-0.5 -3))'), 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),3.141592653589793)@2001-01-02]',
  geometry 'Polygon((-1 3,1 3,0 4,-1 3))')::numeric, 6);
-- Coordinates in the millions, as projected data has
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((-2 -0.5,2 -0.5,2 0.5,-2 0.5,-2 -0.5));[Pose(Point(4000000 5000000),0)@2001-01-01, Pose(Point(4000010 5000000),1.5)@2001-01-02]',
  geometry 'Polygon((4000004 5000003,4000006 5000003,4000005 5000004,4000004 5000003))')::numeric, 6);

-- Two moving bodies: crossing each other, and rotating in step with two faces
-- parallel for the whole sequence
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),0)@2001-01-02]',
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(6 0.5),0)@2001-01-01, Pose(Point(-6 0.5),0)@2001-01-02]'), 6);
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0.3)@2001-01-01, Pose(Point(6 0),1.3)@2001-01-02]',
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 4),0.3)@2001-01-01, Pose(Point(0 4),1.3)@2001-01-02]'), 6);

-- Lines, multilines and multipolygons: the minimum of the distances to the
-- segments or polygons
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),1.5)@2001-01-02]',
  geometry 'Linestring(-5 4,0 2.5,5 4)'), 6);
SELECT round(nearestApproachDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),1.5)@2001-01-02]',
  geometry 'MultiLinestring((-5 4,0 2.5,5 4),(0 -3,0 -8))')::numeric, 6);
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),0)@2001-01-02]',
  geometry 'MultiPolygon(((-3 3,-2 3,-2 4,-3 4,-3 3)),((2 -0.5,3 -0.5,3 0.5,2 0.5,2 -0.5)))'), 6);
-- A static polygon that is not convex, or has holes, is refused
SELECT tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),0)@2001-01-02]',
  geometry 'Polygon((0 3,4 3,4 7,2 4,0 7,0 3))');
SELECT tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),0)@2001-01-02]',
  geometry 'Polygon((0 3,4 3,4 7,0 7,0 3),(1 4,2 4,2 5,1 5,1 4))');
SELECT tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),0)@2001-01-02]',
  geometry 'Polygon((0 3,3 3,2 3,2 5,0 5,0 3))');

-- Temporal points: a moving point, and a point with step interpolation,
-- whose distance jumps at each instant
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),1.5)@2001-01-02]',
  tgeompoint '[Point(-4 3)@2001-01-01, Point(4 3)@2001-01-02]'), 6);
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),0)@2001-01-02, Pose(Point(6 6),0)@2001-01-03]',
  tgeompoint 'Interp=Step;[Point(0 3)@2001-01-01, Point(0 5)@2001-01-02, Point(0 5)@2001-01-03]'), 6);

-- Two rigid geometries, one with step interpolation
SELECT round(tDistance(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),1)@2001-01-02, Pose(Point(6 6),1)@2001-01-03]',
  trgeometry 'Interp=Step;Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 4),0)@2001-01-01, Pose(Point(8 4),0)@2001-01-02, Pose(Point(8 4),0)@2001-01-03]'), 6);

-- Ever, always and temporal dwithin
SELECT eDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Point(5 3)', 2.5),
  aDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  geometry 'Point(5 3)', 2.5);
SELECT eDwithin(
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(0 0),0)@2001-01-02]',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1.5 0),0)@2001-01-01, Pose(Point(1.5 0),0)@2001-01-02]', 1),
  eDwithin(
  trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(5 3),0)@2001-01-01, Pose(Point(5 3),0)@2001-01-02]', 2.5);
SELECT tDwithin(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),1.5)@2001-01-02]',
  geometry 'Point(0 3)', 2.5);
SELECT tDwithin(
  geometry 'Linestring(-5 4,5 4)',
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),1.5)@2001-01-02]', 2.5);
SELECT tDwithin(
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(-6 0),0)@2001-01-01, Pose(Point(6 0),1.5)@2001-01-02]',
  trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 4),0)@2001-01-01, Pose(Point(0 4),-1)@2001-01-02]', 1);

-- A threshold equal to the nearest approach distance.  The distance reaches
-- the threshold at one time but does not cross it, thus tDwithin is true at
-- that time.
WITH c AS MATERIALIZED (
  SELECT format('Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 %s),0.3)@2001-01-01, Pose(Point(0.5 %s),2.1)@2001-01-02]',
    k * 0.37, -k * 0.21)::trgeometry AS trg,
    geometry 'Point(4 0.3)' AS pt,
    geometry 'Polygon((4 -1,6 -1,6 1,4 1,4 -1))' AS poly,
    trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(5 0),0)@2001-01-01, Pose(Point(5 0),-1)@2001-01-02]' AS trg2
  FROM generate_series(1, 20) k
)
SELECT count(*),
  count(*) FILTER (WHERE tDwithin(trg, pt, nearestApproachDistance(trg, pt)) ?= true) AS point,
  count(*) FILTER (WHERE tDwithin(trg, poly, nearestApproachDistance(trg, poly)) ?= true) AS polygon,
  count(*) FILTER (WHERE tDwithin(trg, trg2, nearestApproachDistance(trg, trg2)) ?= true) AS trgeometry
FROM c;

-- tDwithin agrees with the distance at sampled times, for rotating bodies
-- against lines
WITH trg AS MATERIALIZED (
  SELECT k, (format('Polygon((-2 -1,2 -1,2 1,-2 1,-2 -1));[Pose(Point(-8 %s),%s)@2001-01-01, Pose(Point(8 %s),%s)@2001-01-02]',
    (k % 5) - 2, round(atan2(sin(k), cos(k))::numeric, 3),
    2 - (k % 3), round(atan2(sin(2.5 * k), cos(2.5 * k))::numeric, 3)))::trgeometry AS trg,
    format('Linestring(%s 3,0 %s,%s 3)', -6 + k % 4, 1 + (k % 3) * 0.5, 5 - k % 3)::geometry AS line
  FROM generate_series(1, 10) k
),
r AS (
  SELECT k, trg, line, tDwithin(trg, line, 1.0) AS tdw FROM trg
)
SELECT count(*) AS bodies,
  bool_and((SELECT bool_and(valueAtTimestamp(tdw, t) =
      (ST_Distance(valueAtTimestamp(trg, t), line) <= 1.0) OR
      abs(ST_Distance(valueAtTimestamp(trg, t), line) - 1.0) < 1e-6)
    FROM generate_series(timestamptz '2001-01-01', '2001-01-02', interval '5 minutes') t))
    AS tdwithin_ok,
  count(*) FILTER (WHERE eDwithin(trg, line, 1.0)) AS ever_within
FROM r;

-- Dense comparisons.  The bodies come from a fixed rule, so that the test is
-- deterministic: convex polygons of five to seven vertices on an ellipse,
-- that move and turn up to almost a half turn in each segment.  The CTEs are
-- materialized so that each rigid geometry is parsed once, and not once for
-- each sample.
WITH body AS MATERIALIZED (
  SELECT k,
    ('Polygon((' || string_agg(format('%s %s',
       round((2 + (k % 3)) * cos(2 * pi() * i / (5 + k % 3) + k)::numeric, 6),
       round((1 + (k % 2)) * sin(2 * pi() * i / (5 + k % 3) + k)::numeric, 6)),
       ',' ORDER BY i) || ',' ||
     format('%s %s',
       round((2 + (k % 3)) * cos(k)::numeric, 6),
       round((1 + (k % 2)) * sin(k)::numeric, 6)) || '))')::geometry AS geom
  FROM generate_series(1, 20) k, LATERAL generate_series(0, 4 + k % 3) i
  GROUP BY k
),
trg AS MATERIALIZED (
  SELECT k, (ST_AsText(geom) || ';[' ||
    format('Pose(Point(%s %s),%s)@2001-01-01, ', -10 + k % 5, (k % 7) - 3,
      round(atan2(sin(k * 0.7), cos(k * 0.7))::numeric, 3)) ||
    format('Pose(Point(%s %s),%s)@2001-01-02, ', k % 4, 1 - (k % 5),
      round(atan2(sin(k * 0.7 + 2.9 * sin(k)), cos(k * 0.7 + 2.9 * sin(k)))::numeric, 3)) ||
    format('Pose(Point(%s %s),%s)@2001-01-03', 9 - k % 6, (k % 3) - 1,
      round(atan2(sin(k * 0.7 + 2.9 * sin(k) - 2.5 * cos(k)), cos(k * 0.7 + 2.9 * sin(k) - 2.5 * cos(k)))::numeric, 3)) ||
    ']')::trgeometry AS trg
  FROM body
),
trg2 AS MATERIALIZED (
  -- A second body for each, on a path that crosses the first one for odd k
  -- and stays clear of it for even k
  SELECT k, (ST_AsText(geom) || ';[' ||
    format('Pose(Point(%s %s),%s)@2001-01-01, ', 8 - k % 4,
      CASE WHEN k % 2 = 1 THEN 0 ELSE 9 END,
      round(atan2(sin(k * 1.3), cos(k * 1.3))::numeric, 3)) ||
    format('Pose(Point(%s %s),%s)@2001-01-03', -8 + k % 3,
      CASE WHEN k % 2 = 1 THEN 1 ELSE 10 END,
      round(atan2(sin(k * 1.3 - 2.8), cos(k * 1.3 - 2.8))::numeric, 3)) ||
    ']')::trgeometry AS trg2
  FROM body
),
pts AS MATERIALIZED (
  SELECT trg.k, trg, trg2, ST_Point(((trg.k * 37) % 11) - 5, ((trg.k * 23) % 9) - 4) AS pt,
    -- A polygon on the path of the body for odd k and off it for even k
    ST_Buffer(ST_Point(((trg.k * 29) % 13) - 6,
      CASE WHEN trg.k % 2 = 1 THEN ((trg.k * 31) % 7) - 3 ELSE 9 END), 1.5, 2) AS poly
  FROM trg JOIN trg2 ON trg.k = trg2.k
),
r AS (
  SELECT k,
    nearestApproachDistance(trg, pt) AS nadpt,
    (SELECT min(ST_Distance(valueAtTimestamp(trg, t), pt))
     FROM generate_series(timestamptz '2001-01-01', '2001-01-03', interval '1 minute') t) AS densept,
    nearestApproachDistance(trg, poly) AS nadpoly,
    (SELECT min(ST_Distance(valueAtTimestamp(trg, t), poly))
     FROM generate_series(timestamptz '2001-01-01', '2001-01-03', interval '1 minute') t) AS densepoly,
    nearestApproachDistance(trg, trg2) AS nadtrg,
    (SELECT min(ST_Distance(valueAtTimestamp(trg, t), valueAtTimestamp(trg2, t)))
     FROM generate_series(timestamptz '2001-01-01', '2001-01-03', interval '1 minute') t) AS densetrg
  FROM pts
)
SELECT count(*),
  bool_and(nadpt <= densept + 1e-9 AND densept - nadpt < 1e-2) AS point_ok,
  bool_and(nadpoly <= densepoly + 1e-9 AND densepoly - nadpoly < 1e-2) AS polygon_ok,
  bool_and(nadtrg <= densetrg + 1e-9 AND densetrg - nadtrg < 1e-2) AS trgeometry_ok,
  count(*) FILTER (WHERE densept = 0) AS point_overlaps,
  count(*) FILTER (WHERE densepoly = 0) AS polygon_overlaps,
  count(*) FILTER (WHERE densetrg = 0) AS trgeometry_overlaps
FROM r;

-------------------------------------------------------------------------------
