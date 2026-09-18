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

-- Static-geometry → H3 cell / cell set: latLngToCell, geoToH3IndexSet,
-- and the eEqual cell-set prefilter.  Covers every WKT/GSERIALIZED
-- geometry type the kernel supports.

-------------------------------------------------------------------------------
-- POINT → single cell  (latLngToCell)
-------------------------------------------------------------------------------

-- Brussels city center (lat 50.85, lng 4.35), resolution 7
SELECT latLngToCell(geometry 'SRID=4326;POINT(4.35 50.85)', 7);

-- Same point at resolution 0 (coarse) gives a base cell
SELECT latLngToCell(geometry 'SRID=4326;POINT(4.35 50.85)', 0);

-- A geometry other than a point, an empty point and a resolution outside 0 to
-- 15 are refused (geoToH3IndexSet covers any geometry)
/* Errors */
SELECT latLngToCell(geometry 'SRID=4326;LINESTRING(4.35 50.85, 4.36 50.86)', 7);
SELECT latLngToCell(geometry 'SRID=4326;POINT EMPTY', 7);
SELECT latLngToCell(geometry 'SRID=4326;POINT(4.35 50.85)', 16);
SELECT latLngToCell(geometry 'SRID=4326;POINT(4.35 50.85)', -1);

-------------------------------------------------------------------------------
-- POINT → single-element set  (geoToH3IndexSet)
-------------------------------------------------------------------------------

-- Singleton set
SELECT geoToH3IndexSet(geometry 'SRID=4326;POINT(4.35 50.85)', 7);

-- Cardinality 1 verification
SELECT numvalues(geoToH3IndexSet(geometry 'SRID=4326;POINT(4.35 50.85)', 7));

-------------------------------------------------------------------------------
-- LINESTRING → cells along the path
-------------------------------------------------------------------------------

-- ~10 km segment across Brussels at resolution 7 (cell edge ~ 1.2 km).
-- Expect roughly 8-12 cells covering the line.
SELECT numvalues(
  geoToH3IndexSet(
    geometry 'SRID=4326;LINESTRING(4.30 50.80, 4.45 50.90)', 7)) > 1;

-- Same line at resolution 5 (cell edge ~ 9 km) — single cell expected
SELECT numvalues(
  geoToH3IndexSet(
    geometry 'SRID=4326;LINESTRING(4.30 50.80, 4.45 50.90)', 5)) >= 1;

-- THE COVER HOLDS THE CELL OF EVERY POINT ON THE LINE, the cell latLngToCell
-- assigns it, since the segments are walked from cell to cell. Read at a
-- resolution fine enough for a cell to be crossed between two samples.
WITH line(g) AS (
  VALUES (geometry 'SRID=4326;LINESTRING(4.30 50.80, 4.45 50.90)')),
samples AS (
  SELECT latLngToCell(ST_LineInterpolatePoint(g, i / 500.0), 11) AS cell,
    geoToH3IndexSet(g, 11) AS cover
  FROM line, generate_series(0, 500) AS i)
SELECT bool_and(cell <@ cover) FROM samples;

-------------------------------------------------------------------------------
-- POLYGON → cells covering the area
-------------------------------------------------------------------------------

-- Small Brussels-area square (~1 km × 1 km) at resolution 8 (cell edge ~ 460 m).
-- Expect a handful of cells covering the polygon.
SELECT numvalues(
  geoToH3IndexSet(
    geometry 'SRID=4326;POLYGON((4.34 50.84, 4.36 50.84,
                                  4.36 50.86, 4.34 50.86, 4.34 50.84))',
    8)) > 0;

-- POLYGON with a hole — outer ring covers area, inner ring excluded.
SELECT numvalues(
  geoToH3IndexSet(
    geometry 'SRID=4326;POLYGON((4.30 50.80, 4.40 50.80,
                                  4.40 50.90, 4.30 50.90, 4.30 50.80),
                                 (4.34 50.84, 4.36 50.84,
                                  4.36 50.86, 4.34 50.86, 4.34 50.84))',
    7)) > 0;

-------------------------------------------------------------------------------
-- The cover against the cells of the points of the geometry
-------------------------------------------------------------------------------

-- THE COVER IS THE SET OF THE CELLS THAT HOLD A POINT OF THE GEOMETRY, and it
-- is read against two bounds taken from the cell outlines of cellToBoundary.
-- Every cell whose interior shares a point with the interior of the geometry
-- holds a point of it, so the cover omits none (missed), and every cell of the
-- cover shares a point with the geometry (overclaim). A cell sharing only a
-- point of its outline with the geometry (outline) holds a point of it only
-- where the grid assigns that point to it. The candidates are the cells of a
-- grid of points over the box of the geometry widened by one cell, together
-- with their neighbours. The cases are a 20 km sliver 5 m and 20 m wide, whose
-- ring crosses cells no centre of which the sliver holds, the line along its
-- axis, a polygon with a hole, and a 13 km by 16 km and a 300 m by 300 m
-- rectangle.
WITH geo(name, g, resmin, resmax) AS (VALUES
  ('sliver 5 m', geometry 'SRID=4326;POLYGON((10.872725 54.9479166,
    11.1276484 55.0519101, 11.1276032 55.0519468, 10.8726797 54.9479532,
    10.872725 54.9479166))', 7, 9),
  ('sliver 20 m', geometry 'SRID=4326;POLYGON((10.8727929 54.9478617,
    11.1277161 55.0518551, 11.1275355 55.0520018, 10.8726118 54.948008,
    10.8727929 54.9478617))', 7, 9),
  ('line', geometry 'SRID=4326;LINESTRING(10.8727024 54.9479349,
    11.1276258 55.0519285)', 7, 9),
  ('hole', geometry 'SRID=4326;POLYGON((11.24 54.59, 11.27 54.59,
    11.27 54.607, 11.24 54.607, 11.24 54.59), (11.25 54.595, 11.26 54.595,
    11.26 54.6, 11.25 54.6, 11.25 54.595))', 8, 10),
  ('belt', geometry 'SRID=4326;POLYGON((11.2 54.55, 11.2 54.6937298,
    11.4016945 54.6937298, 11.4016945 54.55, 11.2 54.55))', 7, 9),
  ('port', geometry 'SRID=4326;POLYGON((11.2031 54.5517, 11.2031 54.5543949,
    11.2077467 54.5543949, 11.2077467 54.5517, 11.2031 54.5517))', 7, 12)),
cases AS (
  SELECT name, g, res, ST_XMax(b) - ST_XMin(b) AS dx, ST_YMax(b) - ST_YMin(b) AS dy
  FROM geo, generate_series(resmin, resmax) AS res,
    LATERAL (SELECT cellToBoundary(latLngToCell(ST_Centroid(g), res)) AS b) AS c),
cand AS (
  SELECT DISTINCT name, res, n.c
  FROM cases,
    generate_series(0, ceil((ST_XMax(g) - ST_XMin(g) + 2 * dx) / (dx / 4))::int) AS i,
    generate_series(0, ceil((ST_YMax(g) - ST_YMin(g) + 2 * dy) / (dy / 4))::int) AS j,
    LATERAL unnest(gridDisk(latLngToCell(ST_SetSRID(ST_MakePoint(
      ST_XMin(g) - dx + i * dx / 4, ST_YMin(g) - dy + j * dy / 4), 4326), res), 1))
      AS n(c)),
shared AS (
  SELECT k.name, k.res, k.c,
    ST_Relate(cellToBoundary(k.c), cases.g, 'T********') AS interior
  FROM cand k JOIN cases USING (name, res)
  WHERE ST_Intersects(cellToBoundary(k.c), cases.g)),
cover AS (
  SELECT name, res, unnest(geoToH3IndexSet(g, res)) AS c FROM cases)
SELECT name, res, count(*) FILTER (WHERE o.interior) AS interior,
  count(v.c) AS cover,
  count(*) FILTER (WHERE o.interior AND v.c IS NULL) AS missed,
  count(*) FILTER (WHERE v.c IS NOT NULL AND o.c IS NULL) AS overclaim,
  count(*) FILTER (WHERE v.c IS NOT NULL AND NOT o.interior) AS outline
FROM shared o FULL JOIN cover v USING (name, res, c)
GROUP BY name, res ORDER BY name, res;

-------------------------------------------------------------------------------
-- MULTIPOINT → union of per-point cells
-------------------------------------------------------------------------------

SELECT numvalues(
  geoToH3IndexSet(
    geometry 'SRID=4326;MULTIPOINT((4.35 50.85), (4.40 50.90))', 7));

-- Same point twice — dedup to 1
SELECT numvalues(
  geoToH3IndexSet(
    geometry 'SRID=4326;MULTIPOINT((4.35 50.85), (4.35 50.85))', 7));

-------------------------------------------------------------------------------
-- MULTILINESTRING → union of per-line cells
-------------------------------------------------------------------------------

SELECT numvalues(
  geoToH3IndexSet(
    geometry 'SRID=4326;MULTILINESTRING((4.30 50.80, 4.32 50.82),
                                         (4.40 50.88, 4.42 50.90))', 7)) > 1;

-------------------------------------------------------------------------------
-- MULTIPOLYGON → union of per-polygon cells
-------------------------------------------------------------------------------

SELECT numvalues(
  geoToH3IndexSet(
    geometry 'SRID=4326;MULTIPOLYGON(((4.30 50.80, 4.32 50.80,
                                        4.32 50.82, 4.30 50.82, 4.30 50.80)),
                                      ((4.40 50.88, 4.42 50.88,
                                        4.42 50.90, 4.40 50.90, 4.40 50.88)))',
    7)) > 0;

-------------------------------------------------------------------------------
-- GEOMETRYCOLLECTION → recursive union
-------------------------------------------------------------------------------

SELECT numvalues(
  geoToH3IndexSet(
    geometry 'SRID=4326;GEOMETRYCOLLECTION(
                          POINT(4.35 50.85),
                          LINESTRING(4.40 50.88, 4.42 50.90),
                          POLYGON((4.30 50.80, 4.32 50.80,
                                    4.32 50.82, 4.30 50.82, 4.30 50.80)))',
    7)) > 0;

-------------------------------------------------------------------------------
-- Empty / degenerate inputs
-------------------------------------------------------------------------------

-- Empty geometry → NULL
SELECT geoToH3IndexSet(geometry 'SRID=4326;POINT EMPTY', 7);

-- Resolution out of range → ERROR
SELECT geoToH3IndexSet(geometry 'SRID=4326;POINT(4.35 50.85)', -1);
SELECT geoToH3IndexSet(geometry 'SRID=4326;POINT(4.35 50.85)', 16);

-- A CURVE IS REFUSED, alone or inside a collection, rather than read as meeting
-- no cell: the cells of its linearization are what the cover would have to
-- hold, and a cover without them drops every trajectory crossing the curve
SELECT numValues(geoToH3IndexSet(ST_CurveToLine(geometry 'SRID=4326;
  CURVEPOLYGON(CIRCULARSTRING(4.30 50.80, 4.40 50.90, 4.30 50.80))'), 7)) > 0;
/* Errors */
SELECT geoToH3IndexSet(geometry 'SRID=4326;CURVEPOLYGON(CIRCULARSTRING(
  4.30 50.80, 4.40 50.90, 4.30 50.80))', 7);
SELECT geoToH3IndexSet(geometry 'SRID=4326;GEOMETRYCOLLECTION(
  POINT(4.35 50.85), CIRCULARSTRING(4.30 50.80, 4.50 51.00, 4.70 50.80))', 7);

-------------------------------------------------------------------------------
-- eEqual / ?= — set vs th3index prefilter
-------------------------------------------------------------------------------

-- Build a th3index covering Brussels at resolution 7
WITH t AS (
  SELECT th3index(
    tgeompoint 'SRID=4326;[POINT(4.35 50.85)@2024-01-01,
                 POINT(4.40 50.90)@2024-01-02]', 7) AS th3idx
)
-- Set covering a polygon that contains both endpoints → prefilter true
SELECT geoToH3IndexSet(geometry 'SRID=4326;POLYGON((4.30 50.80, 4.45 50.80,
                                                   4.45 50.95, 4.30 50.95,
                                                   4.30 50.80))', 7) ?= t.th3idx
  FROM t;

-- Set covering a polygon that contains neither endpoint → prefilter false
WITH t AS (
  SELECT th3index(
    tgeompoint 'SRID=4326;[POINT(4.35 50.85)@2024-01-01,
                 POINT(4.40 50.90)@2024-01-02]', 7) AS th3idx
)
SELECT eEqual(
         geoToH3IndexSet(geometry 'SRID=4326;POLYGON((10.0 50.0, 10.5 50.0,
                                                     10.5 50.5, 10.0 50.5,
                                                     10.0 50.0))', 7),
         t.th3idx) FROM t;

-- The prefilter answers from the instants of every subtype, and stops at the
-- first instant the set contains. Each subtype is asked for a hit and a miss,
-- and the sequence forms are asked where the hit sits, so a walk that stopped
-- at the wrong place or skipped a composing sequence is refused.

-- Temporal instant
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index '831c02fffffffff@2001-01-01';
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index '8001fffffffffff@2001-01-01';

-- Discrete sequence: the hit at the first instant, at the last, and absent
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index '{831c02fffffffff@2001-01-01, 8001fffffffffff@2001-01-02}';
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index '{8001fffffffffff@2001-01-01, 831c02fffffffff@2001-01-02}';
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index '{8001fffffffffff@2001-01-01, 807ffffffffffff@2001-01-02}';

-- A value repeated across instants is still found, and still not invented
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index '{8001fffffffffff@2001-01-01, 8001fffffffffff@2001-01-02,
                  831c00fffffffff@2001-01-03}';
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index '{8001fffffffffff@2001-01-01, 8001fffffffffff@2001-01-02}';

-- Step sequence
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index 'Interp=Step;[8001fffffffffff@2001-01-01,
                              831c02fffffffff@2001-01-02]';
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index 'Interp=Step;[8001fffffffffff@2001-01-01,
                              807ffffffffffff@2001-01-02]';

-- Sequence set: the hit in the first composing sequence, in the last, absent
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index '{[831c02fffffffff@2001-01-01], [8001fffffffffff@2001-01-02]}';
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index '{[8001fffffffffff@2001-01-01], [831c00fffffffff@2001-01-02]}';
SELECT h3indexset '{831c02fffffffff, 831c00fffffffff}' ?=
       th3index '{[8001fffffffffff@2001-01-01], [807ffffffffffff@2001-01-02]}';
