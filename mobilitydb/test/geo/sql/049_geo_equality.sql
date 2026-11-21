-- Equality of two lines with the same points, the second with one more vertex,
-- in the base values, the temporal values, the sets, the hash, the B-tree order
-- and the normalization of a sequence

SELECT geometry 'Linestring(0 0,2 0)' = geometry 'Linestring(0 0,1 0,2 0)';
SELECT geography 'Linestring(0 0,2 0)' = geography 'Linestring(0 0,1 0,2 0)';

SELECT tgeometry '[Linestring(0 0,2 0)@2001-01-01]' = tgeometry '[Linestring(0 0,1 0,2 0)@2001-01-01]';
SELECT cmp(tgeometry '[Linestring(0 0,2 0)@2001-01-01]', tgeometry '[Linestring(0 0,1 0,2 0)@2001-01-01]');
SELECT hash(tgeometry '[Linestring(0 0,2 0)@2001-01-01]') = hash(tgeometry '[Linestring(0 0,1 0,2 0)@2001-01-01]');
SELECT tgeography '[Linestring(0 0,2 0)@2001-01-01]' = tgeography '[Linestring(0 0,1 0,2 0)@2001-01-01]';
SELECT cmp(tgeography '[Linestring(0 0,2 0)@2001-01-01]', tgeography '[Linestring(0 0,1 0,2 0)@2001-01-01]');
SELECT hash(tgeography '[Linestring(0 0,2 0)@2001-01-01]') = hash(tgeography '[Linestring(0 0,1 0,2 0)@2001-01-01]');

SELECT eEqual(tgeometry '[Linestring(0 0,2 0)@2001-01-01]', geometry 'Linestring(0 0,1 0,2 0)');
SELECT eEqual(tgeography '[Linestring(0 0,2 0)@2001-01-01]', geography 'Linestring(0 0,1 0,2 0)');

SELECT geomset '{"Linestring(0 0,2 0)"}' = geomset '{"Linestring(0 0,1 0,2 0)"}';
SELECT numValues(geomset '{"Linestring(0 0,2 0)", "Linestring(0 0,1 0,2 0)"}');

SELECT numInstants(tgeometry 'Interp=Step;[Linestring(0 0,2 0)@2001-01-01, Linestring(0 0,1 0,2 0)@2001-01-02, Linestring(0 0,2 0)@2001-01-03]');
SELECT ST_AsText(valueAtTimestamp(tgeometry 'Interp=Step;[Linestring(0 0,2 0)@2001-01-01, Linestring(0 0,1 0,2 0)@2001-01-02, Linestring(0 0,2 0)@2001-01-03]', '2001-01-02'));

CREATE TEMP TABLE tbl_geo_eq(t tgeometry);
INSERT INTO tbl_geo_eq VALUES (tgeometry '[Linestring(0 0,2 0)@2001-01-01]'), (tgeometry '[Linestring(0 0,1 0,2 0)@2001-01-01]');
SET enable_hashagg = on; SET enable_sort = off;
SELECT COUNT(*) FROM (SELECT DISTINCT t FROM tbl_geo_eq) s;
SET enable_hashagg = off; SET enable_sort = on;
SELECT COUNT(*) FROM (SELECT DISTINCT t FROM tbl_geo_eq) s;
RESET enable_hashagg; RESET enable_sort;
DROP TABLE tbl_geo_eq;

-- The extended hash, which a hash partition reads, of a set and of a temporal
-- value of geometries and of points: equal values have equal hashes under one
-- seed, a zero of either sign and a longitude of 180 or -180 being one value
SELECT hashExtended(geomset '{"Linestring(0 0,2 0)", "Point(1 1)"}', 0) IS NOT NULL;
SELECT hashExtended(geogset '{"Linestring(0 0,2 0)", "Point(1 1)"}', 1) IS NOT NULL;
SELECT hashExtended(tgeometry '[Linestring(0 0,2 0)@2001-01-01]', 0) =
  hashExtended(tgeometry '[Linestring(0 0,2 0)@2001-01-01]', 0);
SELECT hashExtended(tgeography '[Linestring(0 0,2 0)@2001-01-01]', 7) IS NOT NULL;
SELECT hashExtended(tgeompoint '[Point(0 0)@2001-01-01]', 0) =
  hashExtended(tgeompoint '[Point(-0 0)@2001-01-01]', 0);
SELECT hashExtended(tgeogpoint '[Point(180 10)@2001-01-01]', 3) =
  hashExtended(tgeogpoint '[Point(-180 10)@2001-01-01]', 3);
SELECT hashExtended(tgeompoint '[Point(0 0)@2001-01-01]', 0) <>
  hashExtended(tgeompoint '[Point(1 0)@2001-01-01]', 0);

CREATE TEMP TABLE tbl_geo_part(t tgeompoint) PARTITION BY HASH (t);
CREATE TEMP TABLE tbl_geo_part0 PARTITION OF tbl_geo_part FOR VALUES WITH (MODULUS 2, REMAINDER 0);
CREATE TEMP TABLE tbl_geo_part1 PARTITION OF tbl_geo_part FOR VALUES WITH (MODULUS 2, REMAINDER 1);
INSERT INTO tbl_geo_part VALUES (tgeompoint '[Point(0 0)@2001-01-01]'), (tgeompoint '[Point(1 1)@2001-01-01]');
SELECT COUNT(*) FROM tbl_geo_part;
DROP TABLE tbl_geo_part;

-- Geometries are ordered and equated by value: the type, so that a point
-- precedes a line and a line a polygon; the counts before the coordinates;
-- the coordinates as numbers, a zero of either sign being 0; a geography
-- read on the sphere
SELECT asText(geomset '{"Polygon((0 0,1 0,1 1,0 0))", "Linestring(9 9,8 8)", "Point(5 5)"}');
SELECT asText(geomset '{"Linestring(0 0,0 1,0 2)", "Linestring(5 5,6 6)"}');
SELECT numValues(geomset '{"Linestring(0 0,1 1)", "Linestring(-0 0,1 1)"}');
SELECT geomset '{"Polygon((0 0,1 0,1 1,0 0))"}' = geomset '{"Polygon((-0 0,1 0,1 1,-0 0))"}';
SELECT tgeometry '[Linestring(0 0,1 1)@2001-01-01]' = tgeometry '[Linestring(-0 0,1 1)@2001-01-01]';
SELECT hash(tgeometry '[Linestring(0 0,1 1)@2001-01-01]') = hash(tgeometry '[Linestring(-0 0,1 1)@2001-01-01]');
SELECT hashExtended(tgeometry '[Linestring(0 0,1 1)@2001-01-01]', 5) =
  hashExtended(tgeometry '[Linestring(-0 0,1 1)@2001-01-01]', 5);
SELECT numValues(geogset '{"Linestring(180 10,0 0)", "Linestring(-180 10,0 0)"}');
SELECT tgeography '[Point(10 90)@2001-01-01]' = tgeography '[Point(20 90)@2001-01-01]';

-------------------------------------------------------------------------------
