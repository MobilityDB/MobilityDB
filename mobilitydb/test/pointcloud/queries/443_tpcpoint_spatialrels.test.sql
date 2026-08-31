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

-- Value-level tests for the spatial-relationship functions on
-- tpcpoint — eIntersects / aIntersects / eDisjoint / aDisjoint /
-- eDwithin / aDwithin.

\set p1 'tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), ''2024-01-01''::timestamptz)'
\set p2 'tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), ''2024-01-02''::timestamptz)'
\set inst1 ':p1'
\set seq 'tpcpointSeq(ARRAY[:p1, :p2])'

-------------------------------------------------------------------------------
-- eIntersects
-------------------------------------------------------------------------------

SELECT eIntersects(:p1, geometry 'SRID=0;POINT(0 0)');
SELECT eIntersects(geometry 'SRID=0;POINT(0 0)', :p1);
SELECT eIntersects(:p1, geometry 'SRID=0;POINT(99 99)');
SELECT eIntersects(:seq, geometry 'SRID=0;POINT(0 0)');
SELECT eIntersects(:p1, :p1);

-------------------------------------------------------------------------------
-- aIntersects
-------------------------------------------------------------------------------

SELECT aIntersects(:p1, geometry 'SRID=0;POINT(0 0)');
SELECT aIntersects(:seq, geometry 'SRID=0;POINT(0 0)');

-------------------------------------------------------------------------------
-- eDisjoint
-------------------------------------------------------------------------------

SELECT eDisjoint(:p1, geometry 'SRID=0;POINT(99 99)');
SELECT eDisjoint(:p1, geometry 'SRID=0;POINT(0 0)');
SELECT eDisjoint(:seq, geometry 'SRID=0;POINT(99 99)');

-------------------------------------------------------------------------------
-- aDisjoint
-------------------------------------------------------------------------------

SELECT aDisjoint(:p1, geometry 'SRID=0;POINT(99 99)');
SELECT aDisjoint(:seq, geometry 'SRID=0;POINT(99 99)');

-------------------------------------------------------------------------------
-- eDwithin / aDwithin
-------------------------------------------------------------------------------

SELECT eDwithin(:p1, geometry 'SRID=0;POINT(1 1)', 5.0);
SELECT eDwithin(:p1, geometry 'SRID=0;POINT(99 99)', 5.0);
SELECT aDwithin(:p1, geometry 'SRID=0;POINT(0 0)', 1.0);
SELECT eDwithin(:p1, :p1, 1e-9);
SELECT eDwithin(:seq, geometry 'SRID=0;POINT(0 0)', 1.0);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The matrix of the cast target
--
-- A point cloud schema states its dimensions by name, so Z is one of them and
-- not a property of the type: a schema declaring X and Y alone answers every
-- planar relationship, while one carrying Z meets the engine's refusal of a Z
-- value in a planar relationship, as a temporal geometry point does.
-------------------------------------------------------------------------------

INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (7, 0,
'<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <pc:dimension>
    <pc:position>1</pc:position>
    <pc:size>4</pc:size>
    <pc:description>X coordinate</pc:description>
    <pc:name>X</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>2</pc:position>
    <pc:size>4</pc:size>
    <pc:description>Y coordinate</pc:description>
    <pc:name>Y</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:metadata>
    <Metadata name="srid">0</Metadata>
  </pc:metadata>
</pc:PointCloudSchema>')
  ON CONFLICT (pcid) DO NOTHING;

\set flat 'tpcpoint(PC_MakePoint(7, ARRAY[1.0, 1.0]::float[]), ''2024-01-01''::timestamptz)'
\set square 'geometry ''SRID=0;Polygon((0 0,0 4,4 4,4 0,0 0))'''

SELECT eContains(:square, :flat);
SELECT aContains(:square, :flat);
SELECT eCovers(:square, :flat);
SELECT aCovers(:square, :flat);
SELECT eTouches(:square, :flat);
SELECT eTouches(:flat, :square);
SELECT aTouches(:flat, :square);

-- Every new cell agrees with the cast written out by hand
SELECT eContains(:square, :flat) = eContains(:square, (:flat)::tgeompoint) AS ec_agrees,
       eCovers(:square, :flat) = eCovers(:square, (:flat)::tgeompoint) AS ecv_agrees,
       eTouches(:flat, :square) = eTouches((:flat)::tgeompoint, :square) AS etc_agrees,
       aContains(:square, :flat) = aContains(:square, (:flat)::tgeompoint) AS ac_agrees;

/* Errors: a schema carrying Z in a planar relationship */
SELECT eContains(:square, :p1);
SELECT eTouches(:p1, :square);

-------------------------------------------------------------------------------
