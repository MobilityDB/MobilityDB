-- Bounding box operators on a ts2cell, and the stbox extraction the generated
-- topops band carries with them.
--
-- An S2 cell is a region of the WGS84 sphere, so every box here is GEODETIC in
-- SRID 4326 — that is what separates this family from the Web-Mercator
-- tquadbin, whose boxes are planar.

-------------------------------------------------------------------------------
-- Box extraction
-------------------------------------------------------------------------------

SELECT round(stbox(s2cell '47c3c3'), 6);
SELECT round(stbox(s2cell '47c3c3', timestamptz '2001-01-01'), 6);
SELECT round(stbox(s2cell '47c3c3', tstzspan '[2001-01-01, 2001-01-02]'), 6);
SELECT round(stbox(ts2cell '47c3c3@2001-01-01'), 6);
SELECT round(stbox(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]'), 6);

-------------------------------------------------------------------------------
-- The box is geodetic, and it bounds the cell boundary it comes from
-------------------------------------------------------------------------------

SELECT SRID(stbox(s2cell '47c3c3'));
SELECT ST_Covers(
  ST_SetSRID(stbox(s2cell '47c3c3')::geometry, 4326),
  cellToBoundary(s2cell '47c3c3')::geometry);

-------------------------------------------------------------------------------
-- Topological operators
--
-- A cell overlaps and is contained by its own box, and a value spanning two
-- faces contains the box of either one.
-------------------------------------------------------------------------------

SELECT ts2cell '47c3c3@2001-01-01' && stbox(s2cell '47c3c3');
SELECT ts2cell '47c3c3@2001-01-01' <@ stbox(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]');
SELECT stbox(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]') @> ts2cell '54b5c9@2001-01-02';
SELECT ts2cell '47c3c3@2001-01-01' ~= ts2cell '47c3c3@2001-01-01';
SELECT ts2cell '47c3c3@2001-01-01' && ts2cell '54b5c9@2001-01-01';
SELECT ts2cell '47c3c3@2001-01-01' -|- ts2cell '54b5c9@2001-01-02';

-------------------------------------------------------------------------------
-- expandSpace
-------------------------------------------------------------------------------

SELECT round(expandSpace(ts2cell '47c3c3@2001-01-01', 1), 6);

-------------------------------------------------------------------------------
