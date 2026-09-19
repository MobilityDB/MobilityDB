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

-- tquadbin accessors and lifted cell operations: value accessors, resolution,
-- validity, parent, centroid point, boundary, area, quadkey, and the
-- tquadbin -> tgeompoint cast. Each lifts its static kernel over the time axis.

-------------------------------------------------------------------------------
-- Value accessors
-------------------------------------------------------------------------------

SELECT startValue(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT endValue(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT valueN(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}', 2);
SELECT getValues(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT valueAtTimestamp(tquadbin '[480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-03]', '2001-01-01');

-------------------------------------------------------------------------------
-- Resolution + validity (lifted)
-------------------------------------------------------------------------------

SELECT getResolution(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT isValidCell(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');

-------------------------------------------------------------------------------
-- Hierarchy (lifted parent at a constant resolution)
-------------------------------------------------------------------------------

SELECT cellToParent(tquadbin '{48427fffffffffff@2001-01-01, 48a6227affffffff@2001-01-02}', 0);

-- Every instant's parent at res 0 is the z0 world cell
SELECT getValues(cellToParent(
  tquadbin '{48427fffffffffff@2001-01-01, 48a6227affffffff@2001-01-02}', 0))
  = quadbinset '{480fffffffffffff}';

-------------------------------------------------------------------------------
-- Centroid point / boundary (lifted)
-------------------------------------------------------------------------------

SELECT cellToPoint(tquadbin '{48a6227affffffff@2001-01-01}');
SELECT asText(cellToPoint(tquadbin '{48a6227affffffff@2001-01-01}'), 6);
SELECT cellToBoundary(tquadbin '{48a6227affffffff@2001-01-01}') IS NOT NULL;

-------------------------------------------------------------------------------
-- Area (lifted, tfloat)
-------------------------------------------------------------------------------

SELECT round(startValue(cellArea(tquadbin '{480fffffffffffff@2001-01-01}'))::numeric, 1);

-------------------------------------------------------------------------------
-- Quadkey (lifted, ttext)
-------------------------------------------------------------------------------

SELECT tquadbinCellToQuadkey(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');

-------------------------------------------------------------------------------
-- Convenience cast: tquadbin :: tgeompoint  (reuses cell_to_point centroid)
-------------------------------------------------------------------------------

SELECT (tquadbin '{48a6227affffffff@2001-01-01}')::tgeompoint
  = cellToPoint(tquadbin '{48a6227affffffff@2001-01-01}');
SELECT asText((tquadbin '{48a6227affffffff@2001-01-01}')::tgeompoint, 6);

-------------------------------------------------------------------------------
-- Conversion from a temporal point: tquadbin(tgeompoint, integer)
--
-- The cell-entry timestamps are interpolated from floating-point crossings
-- whose low digits vary across build configurations, so the cases below state
-- the build-stable invariants and print whole values only where every
-- timestamp is exact.
-------------------------------------------------------------------------------

-- An instant yields the cell holding its position
SELECT tquadbin(tgeompoint 'SRID=4326;Point(4.35 50.85)@2001-01-01', 10);
SELECT startValue(tquadbin(tgeompoint 'SRID=4326;Point(4.35 50.85)@2001-01-01', 10))
  = geoToQuadbinCell(geometry 'SRID=4326;Point(4.35 50.85)', 10);

-- A trajectory staying inside one cell holds it over its whole period
SELECT tquadbin(tgeompoint
  'SRID=4326;[Point(4.3500 50.8500)@2001-01-01, Point(4.3501 50.8501)@2001-01-02]', 10);

-- A trajectory that states nothing between its instants holds the cells of
-- those instants, whether discrete or stepwise
SELECT tquadbin(tgeompoint
  'SRID=4326;{Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02}', 4);
SELECT tquadbin(setSRID(tgeompoint
  'Interp=Step;[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02]', 4326), 4);

-- A linear trajectory holds every cell it crosses. The segment below starts in
-- the zoom-4 tile of column 8 and row 7 and ends in the tile of column 10 and
-- row 9, so it enters two more columns and two more rows: five cells, each
-- entered once, and the closing instant holding the last one to the end.
SELECT numValues(getValues(t)), numInstants(t),
  startValue(t) = geoToQuadbinCell(geometry 'SRID=4326;Point(1 1)', 4),
  endValue(t) = geoToQuadbinCell(geometry 'SRID=4326;Point(60 -40)', 4)
FROM (SELECT tquadbin(tgeompoint
  'SRID=4326;[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02]', 4) AS t) AS q;

-- At every sampled timestamp the value is the cell holding the position of the
-- trajectory then
SELECT count(*) AS instants,
  count(*) FILTER (WHERE valueAtTimestamp(c, t) <>
    geoToQuadbinCell(valueAtTimestamp(p, t), 4)) AS other_cell
FROM (SELECT p, tquadbin(p, 4) AS c FROM (VALUES
  (tgeompoint 'SRID=4326;[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02]'),
  (tgeompoint 'SRID=4326;[Point(-170 70)@2001-01-01, Point(170 -70)@2001-01-03]'))
  AS v(p)) AS q,
  generate_series(timestamptz '2001-01-01 00:17', timestamptz '2001-01-02 23:00',
    interval '37 minutes') AS t
WHERE valueAtTimestamp(p, t) IS NOT NULL;

-- The last cell holds to the end of the trajectory, so the temporal cell spans
-- the period of the trajectory under an inclusive and an exclusive upper bound
SELECT getTime(tquadbin(p, 10)) = getTime(p) FROM (VALUES
  (tgeompoint 'SRID=4326;[Point(4.30 50.80)@2001-01-01, Point(4.40 50.90)@2001-01-02]'),
  (tgeompoint 'SRID=4326;[Point(4.30 50.80)@2001-01-01, Point(4.40 50.90)@2001-01-02)'))
  AS v(p);
SELECT endValue(tquadbin(tgeompoint
  'SRID=4326;[Point(4.30 50.80)@2001-01-01, Point(4.40 50.90)@2001-01-02)', 10))
  = geoToQuadbinCell(geometry 'SRID=4326;Point(4.40 50.90)', 10);

-- A tile holds its west and north boundaries, so the point where four tiles
-- meet belongs to the one east of the meridian and south of the parallel
-- meeting there. A path through that point from the south-west leaves its
-- tile for the one diagonally across, which it holds from the crossing on;
-- the tile holding the corner it holds for no time, so the cover states the
-- cell the path enters, as a tile of the space grid does not hold its upper
-- border.
SELECT tquadbin(tgeompoint
  'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]', 2);
SELECT valueAtTimestamp(tquadbin(tgeompoint
  'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]', 2),
  timestamptz '2001-01-02') = geoToQuadbinCell(valueAtTimestamp(tgeompoint
  'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]',
  timestamptz '2001-01-02' + interval '1 microsecond'), 2) AS states_the_cell_entered;
SELECT valueAtTimestamp(tquadbin(tgeompoint
  'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]', 2),
  timestamptz '2001-01-02') = geoToQuadbinCell(geometry 'SRID=4326;Point(-90 0)', 2)
  AS states_the_corner_owner;
SELECT tquadbin(tgeompoint
  'SRID=4326;[Point(-45 -10)@2001-01-01, Point(-135 10)@2001-01-03]', 2);

-- The cover of a trajectory cut at half-open periods merges into the cover of
-- the whole trajectory, one period ending where the path crosses the corner
WITH trip(tp) AS (
  SELECT tgeompoint 'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]'
), periods(period) AS (VALUES
  (tstzspan '[2001-01-01, 2001-01-02)'),
  (tstzspan '[2001-01-02, 2001-01-02 12:00:00)'),
  (tstzspan '[2001-01-02 12:00:00, 2001-01-03]'))
SELECT merge(array_agg(tquadbin(atTime(tp, period), 2) ORDER BY period))
  = tquadbin(tp, 2) AS periods_as_whole
FROM trip, periods GROUP BY tp;

-- Every cell of a cover is held for some time, but for the cell reached at the
-- last instant, which is stated there as the space split states the tile of
-- the last corner under its default borderInc
WITH trip(tp) AS (
  SELECT tgeompoint 'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]'
)
SELECT count(*) FILTER (WHERE duration((u).time) = interval '0') AS cells_of_an_instant,
  count(*) FILTER (WHERE duration((u).time) > interval '0') AS cells_holding_time
FROM trip, unnest(tquadbin(tp, 2)) u;

-- A trajectory ending exactly where four tiles meet reaches the cell of that
-- corner at its last instant and holds it there, as the space split gives the
-- tile of a last corner a fragment of one instant
SELECT tquadbin(tgeompoint
  'SRID=4326;[Point(-45 -33.3)@2001-01-01, Point(0 0)@2001-01-03]', 2);
WITH trip(tp) AS (
  SELECT tgeompoint 'SRID=4326;[Point(-45 -33.3)@2001-01-01, Point(0 0)@2001-01-03]'
)
SELECT count(*) FILTER (WHERE duration((u).time) = interval '0') AS cells_of_an_instant,
  count(*) FILTER (WHERE duration((u).time) > interval '0') AS cells_holding_time
FROM trip, unnest(tquadbin(tp, 2)) u;

-- A trajectory starting where four tiles meet states the cell it travels into
SELECT tquadbin(tgeompoint
  'SRID=4326;[Point(0 0)@2001-01-01, Point(45 33.3)@2001-01-03]', 2);

-- The cover and the restriction of the trajectory to a cell's own box state
-- the same period for every cell. A cell held over an interval is left where
-- the path crosses out of the box, which is the exclusive upper border; a cell
-- reached at the last instant is held there, which is the inclusive border the
-- space split gives the tile of a last corner
WITH trips(tp) AS (VALUES
  (tgeompoint 'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]'),
  (tgeompoint 'SRID=4326;[Point(1 1)@2001-01-01, Point(5 5)@2001-01-05]'),
  (tgeompoint 'SRID=4326;[Point(-170 -60)@2001-01-01, Point(170 60)@2001-01-04]'),
  (tgeompoint 'SRID=4326;[Point(-45 -33.3)@2001-01-01, Point(0 0)@2001-01-03]')
), zooms(z) AS (VALUES (2), (4), (6))
SELECT count(*) AS cells,
  count(*) FILTER (WHERE (u).time <> getTime(atStbox(tp,
    stbox(cellToBoundary((u).value)), duration((u).time) = interval '0')))
  AS cells_stating_another_period
FROM trips, zooms, unnest(tquadbin(tp, z)) u;

-- A sequence set yields one sequence per sequence
SELECT numSequences(tquadbin(tgeompoint
  'SRID=4326;{[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02],
  [Point(4.30 50.80)@2001-01-03, Point(4.40 50.90)@2001-01-04]}', 4));

-------------------------------------------------------------------------------
-- Conversion from a temporal point: tquadbin(tgeogpoint, integer)
-------------------------------------------------------------------------------

-- An instant yields the cell holding its position, as for a planar point
SELECT tquadbin(tgeogpoint 'Point(4.35 50.85)@2001-01-01', 10) =
  tquadbin(tgeompoint 'SRID=4326;Point(4.35 50.85)@2001-01-01', 10);

-- A geodetic segment follows its great circle. The arc from longitude -170 to
-- -100 along latitude 65 rises to latitude 69.1, north of the parallel 66.51
-- bounding the zoom-2 tiles it starts and ends in, so it leaves its tile for
-- the one north of it and comes back: three instants over two cells, where
-- the straight line in longitude and latitude stays in one tile
SELECT numInstants(t), numValues(getValues(t)), startValue(t) = endValue(t),
  numValues(getValues(tquadbin(tgeompoint
    'SRID=4326;[Point(-170 65)@2001-01-01, Point(-100 65)@2001-01-02]', 2)))
FROM (SELECT tquadbin(tgeogpoint
  '[Point(-170 65)@2001-01-01, Point(-100 65)@2001-01-02]', 2) AS t) AS q;

-- An arc across the antimeridian takes its short way, through the tiles of
-- the last and the first columns, where the straight line in longitude and
-- latitude sweeps every column of the grid
SELECT numValues(getValues(tquadbin(tgeogpoint
    '[Point(170 10)@2001-01-01, Point(-170 -10)@2001-01-02]', 3))),
  numValues(getValues(tquadbin(tgeompoint
    'SRID=4326;[Point(170 10)@2001-01-01, Point(-170 -10)@2001-01-02]', 3)));

-- At every sampled timestamp the value is the cell holding the position of the
-- geodetic trajectory then, for an arc crossing rows and columns, the arc
-- bulging north of its tile, an arc across the antimeridian and one over the
-- pole. The samples lie between the minutes, so none falls on the pole or the
-- antimeridian, where a position has more than one longitude
SELECT count(*) AS instants,
  count(*) FILTER (WHERE valueAtTimestamp(c, t) <>
    geoToQuadbinCell(valueAtTimestamp(p, t)::geometry, z)) AS other_cell,
  count(DISTINCT p::text) FILTER (WHERE c <> tquadbin(p::tgeompoint, z))
    AS other_than_planar
FROM (SELECT p, z, tquadbin(p, z) AS c FROM (VALUES
  (tgeogpoint '[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02]', 4),
  (tgeogpoint '[Point(-170 65)@2001-01-01, Point(-100 65)@2001-01-02]', 2),
  (tgeogpoint '[Point(170 10)@2001-01-01, Point(-170 -10)@2001-01-02]', 6),
  (tgeogpoint '[Point(10 80)@2001-01-01, Point(-170 75)@2001-01-02]', 5))
  AS v(p, z)) AS q,
  generate_series(timestamptz '2001-01-01 00:00:30',
    timestamptz '2001-01-01 23:59:30', interval '1 minute') AS t;

-- A sequence set yields one sequence per sequence, spanning its period
SELECT numSequences(t), getTime(t) = getTime(p)
FROM (SELECT p, tquadbin(p, 4) AS t FROM (VALUES (tgeogpoint
  '{[Point(1 1)@2001-01-01, Point(60 -40)@2001-01-02],
  [Point(4.30 50.80)@2001-01-03, Point(4.40 50.90)@2001-01-04)}')) AS v(p))
  AS q;

-- A reference system other than lon/lat, and a resolution outside 0 to 26
/* Errors */
SELECT tquadbin(tgeogpoint 'Point(4.35 50.85)@2001-01-01', 27);
SELECT tquadbin(tgeompoint 'SRID=3857;Point(4.35 50.85)@2001-01-01', 10);
SELECT tquadbin(tgeompoint
  'SRID=3857;[Point(4.35 50.85)@2001-01-01, Point(4.36 50.86)@2001-01-02]', 10);
SELECT tquadbin(tgeompoint 'SRID=4326;Point(4.35 50.85)@2001-01-01', 27);
SELECT tquadbin(tgeompoint 'SRID=4326;Point(4.35 50.85)@2001-01-01', -1);

-------------------------------------------------------------------------------
-- eEqual / ?= -- cell set vs tquadbin prefilter
-------------------------------------------------------------------------------

-- A trip read against the cover of a region it crosses, and of one it never
-- reaches, in both argument orders
WITH t(trip) AS (VALUES (tquadbin(tgeompoint
  'SRID=4326;[Point(4.35 50.85)@2001-01-01, Point(4.40 50.90)@2001-01-02]',
  10)))
SELECT geoToQuadbinSet(geometry 'SRID=4326;POLYGON((4.30 50.80, 4.45 50.80,
    4.45 50.95, 4.30 50.95, 4.30 50.80))', 10) ?= trip,
  trip ?= geoToQuadbinSet(geometry 'SRID=4326;POLYGON((4.30 50.80,
    4.45 50.80, 4.45 50.95, 4.30 50.95, 4.30 50.80))', 10),
  eEqual(geoToQuadbinSet(geometry 'SRID=4326;POLYGON((10.0 50.0, 10.5 50.0,
    10.5 50.5, 10.0 50.5, 10.0 50.0))', 10), trip),
  eEqual(trip, geoToQuadbinSet(geometry 'SRID=4326;POLYGON((10.0 50.0,
    10.5 50.0, 10.5 50.5, 10.0 50.5, 10.0 50.0))', 10))
FROM t;

-- The prefilter answers from the instants of every subtype, and stops at the
-- first instant the set contains. Each subtype is asked for a hit and a miss,
-- and the sequence forms are asked where the hit sits, so a walk that stopped
-- at the wrong place or skipped a composing sequence is refused.

-- Temporal instant
SELECT quadbinset '{48a6227affffffff, 480fffffffffffff}' ?=
       tquadbin '48a6227affffffff@2001-01-01';
SELECT quadbinset '{48a6227affffffff, 480fffffffffffff}' ?=
       tquadbin '48a6227bffffffff@2001-01-01';

-- Discrete sequence: the hit at the first instant, at the last, and absent
SELECT quadbinset '{48a6227affffffff, 480fffffffffffff}' ?=
       tquadbin '{48a6227affffffff@2001-01-01, 48a6227bffffffff@2001-01-02}';
SELECT quadbinset '{48a6227affffffff, 480fffffffffffff}' ?=
       tquadbin '{48a6227bffffffff@2001-01-01, 480fffffffffffff@2001-01-02}';
SELECT quadbinset '{48a6227affffffff, 480fffffffffffff}' ?=
       tquadbin '{48a6227bffffffff@2001-01-01, 48a62278ffffffff@2001-01-02}';

-- Step sequence
SELECT quadbinset '{48a6227affffffff, 480fffffffffffff}' ?=
       tquadbin '[48a6227bffffffff@2001-01-01, 48a6227affffffff@2001-01-02]';
SELECT quadbinset '{48a6227affffffff, 480fffffffffffff}' ?=
       tquadbin '[48a6227bffffffff@2001-01-01, 48a62278ffffffff@2001-01-02]';

-- Sequence set: the hit in the first composing sequence, in the last, absent
SELECT quadbinset '{48a6227affffffff, 480fffffffffffff}' ?=
       tquadbin '{[48a6227affffffff@2001-01-01], [48a6227bffffffff@2001-01-02]}';
SELECT quadbinset '{48a6227affffffff, 480fffffffffffff}' ?=
       tquadbin '{[48a6227bffffffff@2001-01-01], [480fffffffffffff@2001-01-02]}';
SELECT quadbinset '{48a6227affffffff, 480fffffffffffff}' ?=
       tquadbin '{[48a6227bffffffff@2001-01-01], [48a62278ffffffff@2001-01-02]}';

-------------------------------------------------------------------------------
-- Split by the cells of the grid: quadbinSplit(tgeompoint|tgeogpoint, integer)
-------------------------------------------------------------------------------

-- The split states the cells of the cover and, beside each, the trajectory
-- over the periods the cover states for that cell
WITH trip(tp) AS (
  SELECT tgeompoint 'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]'
)
SELECT count(*) AS fragments,
  count(*) FILTER (WHERE s.tpoint <> atTime(tp, getTime(s.tpoint)))
  AS fragments_stating_another_value,
  count(*) FILTER (WHERE getTime(s.tpoint) <>
    (SELECT u.time FROM unnest(tquadbin(tp, 2)) u WHERE u.value = s.cell))
  AS fragments_stating_another_period
FROM trip, LATERAL quadbinSplit(tp, 2) s;

-- Every cell of the split is a cell of the cover
WITH trip(tp) AS (
  SELECT tgeompoint 'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]'
)
SELECT set(array_agg(s.cell)) = getValues(tquadbin(tp, 2)) AS same_cells
FROM trip, LATERAL quadbinSplit(tp, 2) s GROUP BY tp;

-- A trajectory reaching a cell at its last instant holds it there for an
-- instant, so the cover states that cell and the split states its fragment
-- over that instant
WITH trip(tp) AS (
  SELECT tgeompoint 'SRID=4326;[Point(-45 -33.3)@2001-01-01, Point(0 0)@2001-01-03]'
)
SELECT numValues(getValues(tquadbin(tp, 2))) AS cells_of_the_cover,
  (SELECT count(*) FROM quadbinSplit(tp, 2)) AS fragments_of_the_split
FROM trip;

-- The fragments of a split merge into the trajectory
WITH trip(tp) AS (
  SELECT tgeompoint 'SRID=4326;[Point(-135 -10)@2001-01-01, Point(-45 10)@2001-01-03]'
)
SELECT merge(array_agg(s.tpoint ORDER BY getTime(s.tpoint))) = tp AS merges_back
FROM trip, LATERAL quadbinSplit(tp, 2) s GROUP BY tp;

-- A geodetic trajectory is split along its great circles, so it holds a cell
-- the straight line in longitude and latitude never enters
SELECT count(*) AS fragments
FROM quadbinSplit(tgeogpoint '[Point(-170 65)@2001-01-01, Point(-100 65)@2001-01-02]',
  2);
SELECT count(*) AS fragments
FROM quadbinSplit(tgeompoint
  'SRID=4326;[Point(-170 65)@2001-01-01, Point(-100 65)@2001-01-02]', 2);

-------------------------------------------------------------------------------
