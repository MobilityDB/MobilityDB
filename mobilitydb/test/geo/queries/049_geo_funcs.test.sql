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

-------------------------------------------------------------------------------
-- Buffer
-- A buffer is bounded by the offsets of the geometry and by the joins and caps
-- between them, and a circular arc is kept as an arc rather than sampled
-------------------------------------------------------------------------------

-- The buffer of a point is a disk, of a negative or zero distance nothing
SELECT ST_AsText(round(Buffer(geometry 'Point(0 0)', 1), 6));
SELECT ST_AsText(round(Buffer(geometry 'Point(0 0)', 0), 6));
SELECT ST_AsText(round(Buffer(geometry 'Point(0 0)', -1), 6));

-- The two offsets of a line, closed by a cap at each end
SELECT ST_AsText(round(Buffer(geometry 'Linestring(0 0,10 0)', 1), 6));
SELECT ST_AsText(round(Buffer(geometry 'Linestring(0 0,10 0)', 1, 'endcap=flat'), 6));
SELECT ST_AsText(round(Buffer(geometry 'Linestring(0 0,10 0)', 1, 'endcap=square'), 6));

-- The outer side of a turn is filled by the join the style names
SELECT ST_AsText(round(Buffer(geometry 'Linestring(0 0,10 0,10 10)', 1, 'join=bevel'), 6));
SELECT ST_AsText(round(Buffer(geometry 'Linestring(0 0,10 0,10 10)', 1, 'join=mitre'), 6));

-- A polygon grows outwards and its holes inwards
SELECT ST_AsText(round(Buffer(geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))', 1, 'join=mitre'), 6));
SELECT ST_AsText(round(Buffer(geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))', -1, 'join=mitre'), 6));

-- A circular string keeps its arcs, and so does the buffer of one
SELECT ST_AsText(round(Buffer(geometry 'Circularstring(0 0,2 2,4 0)', 1), 6));
SELECT ST_AsText(round(Buffer(geometry 'Curvepolygon(Circularstring(0 0,2 2,4 0,2 -2,0 0))', 1), 6));

-- The buffer of a geometry of several components is their union
SELECT ST_GeometryType(Buffer(geometry 'Multipoint(0 0,10 0)', 1));
SELECT ST_GeometryType(Buffer(geometry 'Multilinestring((0 0,10 0),(0 20,10 20))', 1));

-------------------------------------------------------------------------------
