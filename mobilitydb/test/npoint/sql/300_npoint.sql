-------------------------------------------------------------------------------
-- Input
-------------------------------------------------------------------------------

SELECT npoint 'npoint(1,0.5)';
SELECT npoint ' npoint   (   1   ,	0.5   )   ';
-- An SRID is read when it is the one of the network
SELECT asEWKT(npoint 'SRID=5676;npoint(1,0.5)');
/* Errors */
SELECT npoint 'point(1,0.5)';
SELECT npoint 'npoint 1,0.5)';
SELECT npoint 'npoint(1,0.5';
SELECT npoint 'npoint(1 0.5)';
SELECT npoint 'npoint(1000,0.5)';
SELECT npoint 'npoint(1,1.5)';
SELECT npoint 'npoint(1,0.5)xxx';
-- An SRID other than the one of the network is refused
SELECT npoint 'SRID=4326;npoint(1,0.5)';
SELECT npointFromEWKT('SRID=4326;Npoint(1,0.5)');

SELECT nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment '  nsegment  (  1  ,  0.5  ,  0.7 ) ';
-- An SRID is read when it is the one of the network
SELECT nsegment 'SRID=5676;nsegment(1,0.5,0.7)';
/* Errors */
SELECT nsegment 'Nsegment 1, 1.1)';
SELECT nsegment(1, 1.1);
SELECT nsegment 'segment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.5,0.7';
SELECT nsegment 'nsegment(1 0.5 0.7)';
SELECT nsegment 'nsegment(1000,0.5,0.7)';
SELECT nsegment 'nsegment(1,1.5,0.7)';
SELECT nsegment 'nsegment(1,0.5,0.7)xxx';
SELECT nsegment 'NSegment(1, 1, 1.5)';
-- An SRID other than the one of the network is refused
SELECT nsegment 'SRID=4326;nsegment(1,0.5,0.7)';

-------------------------------------------------------------------------------
-- Input/output in (E)WKT, (E)WKB, and HexWKB representation
-------------------------------------------------------------------------------

SELECT asText(npoint 'SRID=5676;Npoint(1,0.5)');
SELECT asEWKT(npoint 'SRID=5676;Npoint(1,0.5)');
SELECT asBinary(npoint 'SRID=5676;Npoint(1,0.5)');
SELECT asEWKB(npoint 'SRID=5676;Npoint(1,0.5)');
SELECT asHexWKB(npoint 'SRID=5676;Npoint(1,0.5)');
SELECT asHexEWKB(npoint 'SRID=5676;Npoint(1,0.5)');

SELECT npointFromText(asText(npoint 'SRID=5676;Npoint(1,0.5)'));
SELECT npointFromEWKT(asEWKT(npoint 'SRID=5676;Npoint(1,0.5)'));
SELECT npointFromBinary(asBinary(npoint 'SRID=5676;Npoint(1,0.5)'));
SELECT npointFromEWKB(asEWKB(npoint 'SRID=5676;Npoint(1,0.5)'));
SELECT npointFromHexEWKB(asHexWKB(npoint 'SRID=5676;Npoint(1,0.5)'));
SELECT npointFromHexWKB(asHexWKB(npoint 'SRID=5676;Npoint(1,0.5)'));
SELECT npointFromHexEWKB(asHexEWKB(npoint 'SRID=5676;Npoint(1,0.5)'));
-- An SRID other than the one of the network is refused
SELECT npointFromHexEWKB(replace(asHexEWKB(npoint 'SRID=5676;Npoint(1,0.5)', 'XDR'), '0000162C', '000010E6'));

SELECT asBinary(nsegment 'NSegment(1,0.5,0.7)');
SELECT asEWKB(nsegment 'NSegment(1,0.5,0.7)');
SELECT asHexWKB(nsegment 'NSegment(1,0.5,0.7)');
SELECT asHexEWKB(nsegment 'NSegment(1,0.5,0.7)');
SELECT asHexEWKB(nsegment 'NSegment(1,0.5,0.7)', 'XDR');

SELECT nsegmentFromBinary(asBinary(nsegment 'NSegment(1,0.5,0.7)'));
SELECT nsegmentFromEWKB(asEWKB(nsegment 'NSegment(1,0.5,0.7)', 'XDR'));
SELECT nsegmentFromHexWKB(asHexWKB(nsegment 'NSegment(1,0.5,0.7)'));
SELECT nsegmentFromHexEWKB(asHexEWKB(nsegment 'NSegment(1,0.5,0.7)'));
-- An SRID other than the one of the network is refused, and so is a route the
-- network lacks
SELECT nsegmentFromHexEWKB(replace(asHexEWKB(nsegment 'NSegment(1,0.5,0.7)', 'XDR'), '0000162C', '000010E6'));
SELECT nsegmentFromHexWKB(replace(asHexWKB(nsegment 'NSegment(1,0.5,0.7)', 'XDR'), '0000000000000001', '00000000000003E8'));
-- The container states the SRID once, for its components too
SELECT asHexEWKB(npointset '{"NPoint(1,0.5)"}');
-- A component that states the SRID of its container again reads back
SELECT npointsetFromHexEWKB('013100412C16000001000000412C1600000100000000000000000000000000E03F');
-- A component that states an SRID other than the one of the network is refused
SELECT npointsetFromHexEWKB('01310041E61000000100000041E61000000100000000000000000000000000E03F');

SELECT asText(nsegment 'NSegment(1,0.5,0.7)');
SELECT asText(nsegment 'NSegment(1,0.123456,0.7)', 2);
SELECT asEWKT(nsegment 'NSegment(1,0.5,0.7)');
SELECT nsegmentFromText(asText(nsegment 'NSegment(1,0.5,0.7)'))
       = nsegment 'NSegment(1,0.5,0.7)';
SELECT nsegmentFromEWKT(asEWKT(nsegment 'NSegment(1,0.5,0.7)'))
       = nsegment 'NSegment(1,0.5,0.7)';
/* Errors */
SELECT nsegmentFromEWKT('SRID=4326;NSegment(1,0.5,0.7)');

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT npoint(1, 0.5);
/* Errors */
SELECT npoint(1000,0.5);
SELECT npoint(1,1.5);

SELECT nsegment(1, 0.2, 0.6);
SELECT nsegment(1);
SELECT nsegment(1, 0.2);
SELECT nsegment(npoint(1, 0.5));
/* Errors */
SELECT nsegment(1000,0.5,0.7);
SELECT nsegment(1,1.5,0.7);

-------------------------------------------------------------------------------
-- Accessing values
-------------------------------------------------------------------------------

SELECT route(npoint 'npoint(1,0.5)');
SELECT getPosition(npoint 'npoint(1,0.5)');
SELECT srid(npoint 'npoint(1,0.5)');

SELECT route(nsegment 'nsegment(1,0.5,0.7)');
SELECT startPosition(nsegment 'nsegment(1,0.5,0.7)');
SELECT endPosition(nsegment 'nsegment(1,0.5,0.7)');
SELECT srid(nsegment 'nsegment(1,0.5,0.7)');

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT round(npoint 'NPoint(1, 0.123456789)', 6);
SELECT round(nsegment 'NSegment(1, 0.123456789, 0.223456789)', 6);

-------------------------------------------------------------------------------
-- Conversion functions between network and space
-------------------------------------------------------------------------------

SELECT ST_AsText(round(npoint 'npoint(1,0.2)'::geometry, 6));

SELECT ST_AsText(round(nsegment 'nsegment(1,0.5,0.7)'::geometry, 6));

SELECT round((npoint 'npoint(1,0.2)'::geometry)::npoint, 6);

SELECT round((nsegment 'nsegment(1,0.5,0.7)'::geometry)::nsegment, 6);
SELECT round((nsegment 'nsegment(1,0.5,0.5)'::geometry)::nsegment, 6);

SELECT geometry 'SRID=5676;Point(610.455019399524 528.508247341961)'::npoint;
-- A point off the network is mapped only within a tolerance, onto the
-- nearest route: route 29 is at 0.398 of this point and route 1 at 0.473
SELECT ST_Translate(npoint 'npoint(1,0.2)'::geometry, 0, 0.5)::npoint;
SELECT npoint(ST_Translate(npoint 'npoint(1,0.2)'::geometry, 0, 0.5), 0.3);
SELECT round(npoint(ST_Translate(npoint 'npoint(1,0.2)'::geometry, 0, 0.5), 0.45), 6);
SELECT round(npoint(ST_Translate(npoint 'npoint(1,0.2)'::geometry, 0, 0.5), 10), 6);
-- The cast maps back every point a route constructs, and no point lying off
-- the route by more than the rounding of that construction: 0, then NULL
SELECT COUNT(*) FROM generate_series(1, 50) AS r, generate_series(1, 99) AS i
WHERE (npoint(r, i / 100.0)::geometry)::npoint IS NULL;
SELECT ST_Translate(npoint 'npoint(1,0.2)'::geometry, 0, 1e-12)::npoint;

SELECT geometry 'SRID=5676;LINESTRING(83.28320090658963 86.09033222310245,69.08071548677981 81.20815036818385,13.62569909542799 97.53460139036179)'::nsegment;
-- NULL
SELECT geometry 'SRID=5676;LINESTRING(416.346567736997 528.335344322874,610.455019399524 528.508247341961,476.989195102204 642.550969672973)'::nsegment;
/* Errors */
SELECT geometry 'Point empty'::npoint;
SELECT geometry 'Polygon((0 0,0 1,1 1,1 0,0 0))'::nsegment;
SELECT npoint(npoint 'npoint(1,0.2)'::geometry, -1);
SELECT npoint(npoint 'npoint(1,0.2)'::geometry, 'NaN');
SELECT npoint(npoint 'npoint(1,0.2)'::geometry, 'Infinity');

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT npoint 'npoint(1,0.5)' = npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' = npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' = npoint 'npoint(2,0.5)';
-- Equality is exact, as the order is: positions closer than any tolerance differ
SELECT npoint 'npoint(1,0.5)' = npoint 'npoint(1,0.5000006)';
SELECT npoint 'npoint(1,0.5)' < npoint 'npoint(1,0.5000006)';

SELECT npoint 'npoint(1,0.5)' != npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' != npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' != npoint 'npoint(2,0.5)';

SELECT npoint 'npoint(1,0.5)' < npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' < npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' < npoint 'npoint(2,0.5)';

SELECT npoint 'npoint(1,0.5)' <= npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' <= npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' <= npoint 'npoint(2,0.5)';

SELECT npoint 'npoint(1,0.5)' > npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' > npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' > npoint 'npoint(2,0.5)';

SELECT npoint 'npoint(1,0.5)' >= npoint 'npoint(1,0.5)';
SELECT npoint 'npoint(1,0.5)' >= npoint 'npoint(1,0.7)';
SELECT npoint 'npoint(1,0.5)' >= npoint 'npoint(2,0.5)';

SELECT cmp(nsegment 'nsegment(1,0.3,0.5)', nsegment 'nsegment(1,0.3,0.4)');

SELECT nsegment 'nsegment(1,0.3,0.5)' = nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' = nsegment 'nsegment(1,0.3,0.7)';
-- Equality is exact, as the order is: positions closer than any tolerance differ
SELECT nsegment 'nsegment(1,0.3,0.5)' = nsegment 'nsegment(1,0.3,0.5000006)';
SELECT nsegment 'nsegment(1,0.3,0.5)' < nsegment 'nsegment(1,0.3,0.5000006)';
SELECT nsegment 'nsegment(1,0.3,0.5)' = nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' = nsegment 'nsegment(2,0.3,0.5)';

SELECT nsegment 'nsegment(1,0.3,0.5)' != nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' != nsegment 'nsegment(1,0.3,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' != nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' != nsegment 'nsegment(2,0.3,0.5)';

SELECT nsegment 'nsegment(1,0.3,0.5)' < nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' < nsegment 'nsegment(1,0.3,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' < nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' < nsegment 'nsegment(2,0.3,0.5)';

SELECT nsegment 'nsegment(1,0.3,0.5)' <= nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' <= nsegment 'nsegment(1,0.3,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' <= nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' <= nsegment 'nsegment(2,0.3,0.5)';

SELECT nsegment 'nsegment(1,0.3,0.5)' > nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' > nsegment 'nsegment(1,0.3,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' > nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' > nsegment 'nsegment(2,0.3,0.5)';

SELECT nsegment 'nsegment(1,0.3,0.5)' >= nsegment 'nsegment(1,0.3,0.5)';
SELECT nsegment 'nsegment(1,0.3,0.5)' >= nsegment 'nsegment(1,0.3,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' >= nsegment 'nsegment(1,0.5,0.7)';
SELECT nsegment 'nsegment(1,0.3,0.5)' >= nsegment 'nsegment(2,0.3,0.5)';

-------------------------------------------------------------------------------/

-------------------------------------------------------------------------------
-- The conversions between geometry and geography
-------------------------------------------------------------------------------

SELECT npoint 'Npoint(1, 0.5)'::geography;
SELECT nsegment 'Nsegment(1, 0.5, 0.7)'::geography;

-------------------------------------------------------------------------------
-- A route of the ways table is read as a planar line in the SRID of the network
-------------------------------------------------------------------------------

INSERT INTO ways(gid, the_geom, length) VALUES
  (9001, 'SRID=5676;LINESTRING Z(0 0 0,3 4 0)', 5),
  (9002, 'SRID=5676;LINESTRING M(0 0 0,3 4 0)', 5),
  (9003, 'SRID=5676;MULTILINESTRING((0 0,3 4))', 5),
  (9004, 'SRID=4326;LINESTRING(0 0,3 4)', 5);
/* Errors */
SELECT npoint(9001, 0.5);
SELECT npoint(9002, 0.5);
SELECT npoint(9003, 0.5);
SELECT npoint(9004, 0.5);
SELECT npoint(geometry 'SRID=5676;POINT(1 1)', 10);
DELETE FROM ways WHERE gid > 9000;
