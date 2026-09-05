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

-- Static-geometry → H3 cell / cell set: geoToH3Cell, geoToH3IndexSet,
-- and the eEq cell-set prefilter.  Covers every WKT/GSERIALIZED
-- geometry type the kernel supports.

-------------------------------------------------------------------------------
-- POINT → single cell  (geoToH3Cell)
-------------------------------------------------------------------------------

-- Brussels city center (lat 50.85, lng 4.35), resolution 7
SELECT geoToH3Cell(geometry 'SRID=4326;POINT(4.35 50.85)', 7);

-- Same point at resolution 0 (coarse) gives a base cell
SELECT geoToH3Cell(geometry 'SRID=4326;POINT(4.35 50.85)', 0);

-- Non-POINT input: returns first point's cell (use geoToH3IndexSet for the full set)
SELECT geoToH3Cell(geometry 'SRID=4326;LINESTRING(4.35 50.85, 4.36 50.86)', 7);

-------------------------------------------------------------------------------
-- POINT → single-element set  (geoToH3IndexSet)
-------------------------------------------------------------------------------

-- Singleton set
SELECT geoToH3IndexSet(geometry 'SRID=4326;POINT(4.35 50.85)', 7);

-- Cardinality 1 verification
SELECT numvalues(geoToH3IndexSet(geometry 'SRID=4326;POINT(4.35 50.85)', 7));

-------------------------------------------------------------------------------
-- LINESTRING → cells along the path (sampling, each sample ringed)
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

-- THE COVER HOLDS THE CELL OF EVERY POINT ON THE LINE, and sampling alone
-- does not give that: a cell the line enters and leaves between two
-- consecutive samples is named by neither of them. Each sample therefore
-- contributes the ring of its own neighbours, which is what makes the cover
-- conservative. Read at a resolution fine enough for the gap to open.
WITH line(g) AS (
  VALUES (geometry 'SRID=4326;LINESTRING(4.30 50.80, 4.45 50.90)')),
samples AS (
  SELECT geoToH3Cell(ST_LineInterpolatePoint(g, i / 500.0), 11) AS cell,
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

-------------------------------------------------------------------------------
-- eEq / ?= — set vs th3index prefilter
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
SELECT eEq(
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
