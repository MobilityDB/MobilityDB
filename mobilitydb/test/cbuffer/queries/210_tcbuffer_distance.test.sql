-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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

SELECT round(geometry 'Point(1 1)' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(geometry 'Point(1 1)' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(geometry 'Point(1 1)' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(geometry 'Point(1 1)' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

SELECT round(geometry 'Point empty' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(geometry 'Point empty' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(geometry 'Point empty' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(geometry 'Point empty' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> geometry 'Point(1 1)', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> geometry 'Point(1 1)', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> geometry 'Point(1 1)', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> geometry 'Point(1 1)', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> geometry 'Point empty', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> geometry 'Point empty', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> geometry 'Point empty', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> geometry 'Point empty', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

-------------------------------------------------------------------------------
-- minDistance(tcbuffer, geometry) / (geometry, tcbuffer) -- scalar, reduces
-- to NAD when one side has no time dimension
-------------------------------------------------------------------------------

SELECT round(minDistance(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]', geometry 'Point(5 5)')::numeric, 6);
SELECT round(minDistance(geometry 'Point(5 5)', tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]')::numeric, 6);
-- Trajectory disc sweeps over the static point: distance 0
SELECT minDistance(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 1)@2000-01-02]', geometry 'Point(5 0)');
-- Static polygon
SELECT round(minDistance(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(2 2), 0.5)@2000-01-02]', geometry 'POLYGON((10 10, 20 10, 20 20, 10 20, 10 10))')::numeric, 6);

-------------------------------------------------------------------------------
-- minDistance(tcbuffer, cbuffer) / (cbuffer, tcbuffer) -- scalar
-------------------------------------------------------------------------------

SELECT round(minDistance(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]', cbuffer 'Cbuffer(Point(5 5), 1)')::numeric, 6);
SELECT round(minDistance(cbuffer 'Cbuffer(Point(5 5), 1)', tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]')::numeric, 6);

-------------------------------------------------------------------------------
-- minDistance(tcbuffer, tcbuffer) -- 2-ary aggregate, time-agnostic spatial
-- min equivalent to ST_Distance(traversedArea, traversedArea)
-------------------------------------------------------------------------------

-- Aggregate over one row equals the per-pair value
SELECT round(minDistance(t1, t2)::numeric, 6) FROM (
  SELECT tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]' AS t1,
         tcbuffer '[Cbuffer(Point(0 5), 0.5)@2000-01-01, Cbuffer(Point(10 5), 0.5)@2000-01-02]' AS t2) v;

-- Parallel segments, constant radius: gap is centerline gap minus both radii
SELECT round(minDistance(t1, t2)::numeric, 6) FROM (
  SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 1)@2000-01-02]' AS t1,
         tcbuffer '[Cbuffer(Point(0 6), 1)@2000-01-01, Cbuffer(Point(10 6), 1)@2000-01-02]' AS t2) v;

-- Crossing centerlines, tapered radius: exercises the interior critical point
SELECT round(minDistance(t1, t2)::numeric, 6) FROM (
  SELECT tcbuffer '[Cbuffer(Point(0 0), 0.2)@2000-01-01, Cbuffer(Point(10 10), 2.0)@2000-01-02]' AS t1,
         tcbuffer '[Cbuffer(Point(0 10), 2.0)@2000-01-01, Cbuffer(Point(10 0), 0.2)@2000-01-02]' AS t2) v;

-- Overlapping swept discs: distance 0
SELECT minDistance(t1, t2) FROM (
  SELECT tcbuffer '[Cbuffer(Point(0 0), 2)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02]' AS t1,
         tcbuffer '[Cbuffer(Point(5 0), 2)@2000-01-01, Cbuffer(Point(5 5), 2)@2000-01-02]' AS t2) v;

-- Instant vs sequence (TINSTANT dispatch)
SELECT round(minDistance(t1, t2)::numeric, 6) FROM (
  SELECT tcbuffer 'Cbuffer(Point(5 5), 0.5)@2000-01-01' AS t1,
         tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]' AS t2) v;

-- Sequence set on both sides (TSEQUENCESET dispatch)
SELECT round(minDistance(t1, t2)::numeric, 6) FROM (
  SELECT tcbuffer '{[Cbuffer(Point(0 0), 0.3)@2000-01-01, Cbuffer(Point(2 0), 0.3)@2000-01-02], [Cbuffer(Point(20 20), 0.3)@2000-01-04, Cbuffer(Point(22 20), 0.3)@2000-01-05]}' AS t1,
         tcbuffer '{[Cbuffer(Point(0 4), 0.3)@2000-01-01, Cbuffer(Point(2 4), 0.3)@2000-01-02], [Cbuffer(Point(40 40), 0.3)@2000-01-04, Cbuffer(Point(42 40), 0.3)@2000-01-05]}' AS t2) v;

-- Aggregate over multiple rows: minimum across all per-pair distances
WITH pairs(t1, t2) AS (VALUES
  (tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
   tcbuffer '[Cbuffer(Point(10 10), 0.5)@2000-01-01, Cbuffer(Point(11 11), 0.5)@2000-01-02]'),
  (tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
   tcbuffer '[Cbuffer(Point(0 5), 0.5)@2000-01-01, Cbuffer(Point(1 5), 0.5)@2000-01-02]'),
  (tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
   tcbuffer '[Cbuffer(Point(2 1), 0.5)@2000-01-01, Cbuffer(Point(2 2), 0.5)@2000-01-02]'))
SELECT round(minDistance(t1, t2)::numeric, 6) FROM pairs;

-- Grouped: per-group minimum, exercises the running-threshold tightening
WITH src(g, t1, t2) AS (VALUES
  (1, tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
      tcbuffer '[Cbuffer(Point(0 10), 0.5)@2000-01-01, Cbuffer(Point(1 10), 0.5)@2000-01-02]'),
  (1, tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
      tcbuffer '[Cbuffer(Point(0 5), 0.5)@2000-01-01, Cbuffer(Point(1 5), 0.5)@2000-01-02]'),
  (2, tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
      tcbuffer '[Cbuffer(Point(0 2), 0.5)@2000-01-01, Cbuffer(Point(1 2), 0.5)@2000-01-02]'))
SELECT g, round(minDistance(t1, t2)::numeric, 6) FROM src GROUP BY g ORDER BY g;

-- Empty group returns NULL
SELECT minDistance(t1, t2) FROM (
  SELECT tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]' AS t1,
         tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]' AS t2 WHERE false) v;

-- Agreement with ST_Distance over the traversed areas.  The kernel is the
-- exact circular-geometry minimum; traversedArea facets each arc into a
-- finite polygon, so the two agree only up to that polygonisation error
-- (here under 1e-3 for these small radii).
WITH d(a, b) AS (
  SELECT minDistance(t1, t2),
         min(ST_Distance(traversedArea(t1), traversedArea(t2)))
  FROM (
    SELECT tcbuffer '[Cbuffer(Point(0 0), 0.4)@2000-01-01, Cbuffer(Point(8 3), 0.7)@2000-01-02, Cbuffer(Point(12 1), 0.5)@2000-01-03]' AS t1,
           tcbuffer '[Cbuffer(Point(2 9), 0.6)@2000-01-01, Cbuffer(Point(9 7), 0.3)@2000-01-02, Cbuffer(Point(13 8), 0.5)@2000-01-03]' AS t2) v)
SELECT abs(a - b) < 1e-3 FROM d;

-------------------------------------------------------------------------------


-- Analytic nearest approach distance (|=| and nearestApproachDistance):
-- polygons, lines, holes, multi, containing polygon (interior ray-cast),
-- and a curved type (exact traversed-area fallback)
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Polygon((-3 -3,-3 3,3 3,3 -3,-3 -3))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Polygon((-10 -10,-10 10,10 10,10 -10,-10 -10))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Linestring(4 -3,4 6)', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Circularstring(5 0,7 2,9 0)', 6);
SELECT round(geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))' |=| tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', 6);
SELECT round(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]' |=| geometry 'Polygon((20 20,20 24,24 24,24 20,20 20))', 6);
SELECT round(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]' |=| geometry 'Linestring(20 -5,20 20)', 6);
SELECT round(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]' |=| geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))', 6);
SELECT round(tcbuffer 'Interp=Step;[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(5 5), 3)@2000-01-02]' |=| geometry 'Polygon((11 -1,11 3,14 3,14 -1,11 -1))', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(4 0), 1)@2000-01-02], [Cbuffer(Point(20 20), 2)@2000-01-03, Cbuffer(Point(25 20), 1)@2000-01-04]}' |=| geometry 'Polygon((-5 -5,-5 15,15 15,15 -5,-5 -5),(0 0,4 0,4 4,0 4,0 0))', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(4 0), 1)@2000-01-02], [Cbuffer(Point(20 20), 2)@2000-01-03, Cbuffer(Point(25 20), 1)@2000-01-04]}' |=| geometry 'Multilinestring((50 50,60 60),(2 -3,2 9))', 6);
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]', geometry 'Polygon((11 -1,11 3,14 3,14 -1,11 -1))'), 6);
SELECT round(nearestApproachDistance(geometry 'Linestring(4 -3,4 6)', tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01'), 6);
SELECT round(nearestApproachDistance(tcbuffer '{Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(8 3), 2)@2000-01-02}', geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))'), 6);

-------------------------------------------------------------------------------

-- Analytic shortestLine: its length equals the nearest-approach distance;
-- exercises the witness path over polygons, lines, hole, multi, a
-- containing polygon (degenerate) and a curved type (exact fallback)
SELECT geometrytype(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))'));
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Linestring(4 -3,4 6)'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Polygon((-2 -2,-2 30,30 30,30 -2,-2 -2))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Circularstring(5 0,7 2,9 0)'))::numeric, 6);
SELECT round(ST_Length(shortestLine(geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))', tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]', geometry 'Polygon((20 20,20 24,24 24,24 20,20 20))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]', geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Interp=Step;[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(5 5), 3)@2000-01-02]', geometry 'Polygon((11 -1,11 3,14 3,14 -1,11 -1))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '{[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(4 0), 1)@2000-01-02], [Cbuffer(Point(20 20), 2)@2000-01-03, Cbuffer(Point(25 20), 1)@2000-01-04]}', geometry 'Polygon((-5 -5,-5 15,15 15,15 -5,-5 -5),(0 0,4 0,4 4,0 4,0 0))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '{Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(8 3), 2)@2000-01-02}', geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))'))::numeric, 6);

-------------------------------------------------------------------------------
