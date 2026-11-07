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

-------------------------------------------------------------------------------
-- The trip generator of BerlinMOD
-------------------------------------------------------------------------------

-- A road of 50 km/h, 13.889 m/s, going straight on through 19 inner vertices
WITH x AS (
  SELECT createTrip(ARRAY[ROW(geometry 'SRID=3857;LINESTRING(0 0,50 0,100 0,150 0,
    200 0,250 0,300 0,350 0,400 0,450 0,500 0,550 0,600 0,650 0,700 0,750 0,
    800 0,850 0,900 0,950 0,1000 0)', 50.0::float, 1)]::record[],
    '2020-06-01 08:00', false, 'minimal') AS trip )
SELECT numInstants(trip), duration(trip),
  maxValue(speed(trip)) <= 50.0 / 3.6 + 1e-6 AS keeps_limit,
  length(trip) / extract(epoch FROM duration(trip)) > 0.75 * 50.0 / 3.6 AS keeps_pace
FROM x;

-- A road of 36 km/h, 10 m/s, turning back at its inner vertex: the object
-- slows down to the vertex and passes it
WITH x AS (
  SELECT createTrip(ARRAY[ROW(geometry 'SRID=3857;LINESTRING(0 0,100 0,0 1)',
    36.0::float, 1)]::record[], '2020-06-01 08:00', false, 'minimal') AS trip )
SELECT numInstants(trip), duration(trip),
  maxValue(speed(trip)) <= 36.0 / 3.6 + 1e-6 AS keeps_limit,
  EXISTS (SELECT 1 FROM unnest(instants(trip)) i
    WHERE ST_Equals(getValue(i), geometry 'SRID=3857;POINT(100 0)')) AS passes_vertex
FROM x;

-- Two edges of 90 and 30 km/h, 25 and 8.333 m/s, joined at a right angle
WITH x AS (
  SELECT createTrip(ARRAY[
    ROW(geometry 'SRID=3857;LINESTRING(0 0,400 0)', 90.0::float, 1),
    ROW(geometry 'SRID=3857;LINESTRING(400 0,400 -400)', 30.0::float, 1)]::record[],
    '2020-06-01 08:00', false, 'minimal') AS trip )
SELECT numInstants(trip), duration(trip),
  maxValue(speed(atGeometry(trip, geometry 'SRID=3857;LINESTRING(0 0,399 0)')))
    <= 90.0 / 3.6 + 1e-6 AS keeps_limit_1,
  maxValue(speed(atGeometry(trip, geometry 'SRID=3857;LINESTRING(400 -1,400 -400)')))
    <= 30.0 / 3.6 + 1e-6 AS keeps_limit_2
FROM x;

-------------------------------------------------------------------------------
