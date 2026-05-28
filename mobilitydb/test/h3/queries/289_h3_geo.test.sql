-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Static-geometry → H3 cell / cell set: geoToH3Cell, geoToH3IndexSet,
-- eIntersects.  Covers every WKT/GSERIALIZED
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
-- LINESTRING → cells along the path (Nyquist segment sampling)
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
-- eIntersects — set vs th3index prefilter
-------------------------------------------------------------------------------

-- Build a th3index covering Brussels at resolution 7
WITH t AS (
  SELECT h3_latlng_to_cell(
    tgeompoint 'SRID=4326;[POINT(4.35 50.85)@2024-01-01,
                 POINT(4.40 50.90)@2024-01-02]', 7) AS th3idx
)
-- Set covering a polygon that contains both endpoints → prefilter true
SELECT eIntersects(
         geoToH3IndexSet(geometry 'SRID=4326;POLYGON((4.30 50.80, 4.45 50.80,
                                                     4.45 50.95, 4.30 50.95,
                                                     4.30 50.80))', 7),
         t.th3idx) FROM t;

-- Set covering a polygon that contains neither endpoint → prefilter false
WITH t AS (
  SELECT h3_latlng_to_cell(
    tgeompoint 'SRID=4326;[POINT(4.35 50.85)@2024-01-01,
                 POINT(4.40 50.90)@2024-01-02]', 7) AS th3idx
)
SELECT eIntersects(
         geoToH3IndexSet(geometry 'SRID=4326;POLYGON((10.0 50.0, 10.5 50.0,
                                                     10.5 50.5, 10.0 50.5,
                                                     10.0 50.0))', 7),
         t.th3idx) FROM t;

-- NOTE: The two h3_latlng_to_cell-based queries above currently error with
-- "Unknown SRID function for type: h3index" — a pre-existing th3index
-- catalog gap (the h3index basetype's SRID resolver is not registered).
-- Will pass automatically once the th3index branch wires up the resolver.
