-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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

-------------------------------------------------------------------------------
-- Traversed area
-------------------------------------------------------------------------------

SELECT traversedArea(NULL::tcbuffer);
SELECT ST_AsText(traversedArea(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01'));
SELECT ST_AsText(traversedArea(tcbuffer '[Cbuffer(Point(1 1),0.3)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02]'));

-------------------------------------------------------------------------------
-- Restriction to a geometry (atGeometry / minusGeometry)
-- The restriction is by the circular disk footprint, not the centre path: the
-- buffer meets the geometry when the moving disk intersects it.
-------------------------------------------------------------------------------

SELECT atGeometry(NULL::tcbuffer, geometry 'Point(1 1)');
SELECT minusGeometry(NULL::tcbuffer, geometry 'Point(1 1)');
SELECT atGeometry(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', NULL::geometry);

-- Instant: the disk covers the geometry -> unchanged; disjoint -> NULL
SELECT asText(atGeometry(tcbuffer 'Cbuffer(Point(1 1),1)@2000-01-01', geometry 'Point(1.5 1)'));
SELECT atGeometry(tcbuffer 'Cbuffer(Point(1 1),1)@2000-01-01', geometry 'Point(5 5)');

-- Sequence: a box lies ABOVE the centre path (y in [0.5,1.5]); the centre (y=0)
-- never enters it but the radius-1 disk does, so the disk-footprint restriction
-- is non-empty where a centre-only restriction would be empty
SELECT getTime(atGeometry(
  tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(4 0),1)@2000-01-05]',
  geometry 'Polygon((1.5 0.5,2.5 0.5,2.5 1.5,1.5 1.5,1.5 0.5))'));
SELECT getTime(minusGeometry(
  tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(4 0),1)@2000-01-05]',
  geometry 'Polygon((1.5 0.5,2.5 0.5,2.5 1.5,1.5 1.5,1.5 0.5))'));

-- Disjoint over the whole sequence -> at NULL, minus unchanged
SELECT atGeometry(
  tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(4 0),1)@2000-01-05]',
  geometry 'Point(2 50)');
SELECT getTime(minusGeometry(
  tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(4 0),1)@2000-01-05]',
  geometry 'Point(2 50)'));

-------------------------------------------------------------------------------
-- Centroid
-------------------------------------------------------------------------------

SELECT centroid(NULL::tcbuffer);
SELECT asText(centroid(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01'));
SELECT asText(centroid(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}'));
SELECT asText(centroid(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(3 3),0.5)@2000-01-03]'));

-------------------------------------------------------------------------------
-- Convex hull
-------------------------------------------------------------------------------

SELECT convexHull(NULL::tcbuffer);
SELECT ST_GeometryType(convexHull(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01'));
SELECT ST_GeometryType(convexHull(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(3 3),0.5)@2000-01-03]'));

-------------------------------------------------------------------------------
