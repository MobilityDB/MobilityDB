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
-- distance
-------------------------------------------------------------------------------

SELECT round(distance(geometry 'Point(1 0)', cbuffer 'Cbuffer(Point(4 0),0.5)'), 6);
SELECT round(distance(stbox 'STBOX X((1,-1),(3,1))', cbuffer 'Cbuffer(Point(4 0),0.5)'), 6);
SELECT round(distance(cbuffer 'Cbuffer(Point(4 0),0.5)', geometry 'Point(1 0)'), 6);
SELECT round(distance(cbuffer 'Cbuffer(Point(4 0),0.5)', stbox 'STBOX X((1,-1),(3,1))'), 6);
SELECT round(distance(cbuffer 'Cbuffer(Point(0 0),0.5)', cbuffer 'Cbuffer(Point(3 4),0.5)'), 6);

SELECT round(geometry 'Point(1 0)' <-> cbuffer 'Cbuffer(Point(4 0),0.5)', 6);
SELECT round(stbox 'STBOX X((1,-1),(3,1))' <-> cbuffer 'Cbuffer(Point(4 0),0.5)', 6);
SELECT round(cbuffer 'Cbuffer(Point(4 0),0.5)' <-> geometry 'Point(1 0)', 6);
SELECT round(cbuffer 'Cbuffer(Point(4 0),0.5)' <-> stbox 'STBOX X((1,-1),(3,1))', 6);
SELECT round(cbuffer 'Cbuffer(Point(0 0),0.5)' <-> cbuffer 'Cbuffer(Point(3 4),0.5)', 6);

-- The radii cover the gap between the centres, so the distance is zero
SELECT round(cbuffer 'Cbuffer(Point(0 0),3)' <-> cbuffer 'Cbuffer(Point(3 4),3)', 6);
SELECT round(cbuffer 'Cbuffer(Point(0 0),0)' <-> cbuffer 'Cbuffer(Point(0 0),0)', 6);
SELECT round(cbuffer 'SRID=5676;Cbuffer(Point(0 0),0.5)' <->
  cbuffer 'SRID=5676;Cbuffer(Point(3 4),0.5)', 6);
SELECT round(geometry 'Point empty' <-> cbuffer 'Cbuffer(Point(4 0),0.5)', 6);

-------------------------------------------------------------------------------
-- tDistance
-------------------------------------------------------------------------------

SELECT round(geometry 'Point(1 1)' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(geometry 'Point(1 1)' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(geometry 'Point(1 1)' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(geometry 'Point(1 1)' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

SELECT round(geometry 'Point empty' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(geometry 'Point empty' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(geometry 'Point empty' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(geometry 'Point empty' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(cbuffer 'Cbuffer(Point(1 1), 0.2)' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> cbuffer 'Cbuffer(Point(1 1), 0.2)', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> geometry 'Point(1 1)', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> geometry 'Point(1 1)', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> geometry 'Point(1 1)', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> geometry 'Point(1 1)', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> geometry 'Point empty', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> geometry 'Point empty', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> geometry 'Point empty', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> geometry 'Point empty', 6);

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 6);
SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 6);
SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 6);
SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <-> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 6);

-------------------------------------------------------------------------------
-- Temporal distance to a non-point geometry: the full geometry is decomposed
-- into its boundary edges, not collapsed to its minimum bounding circle. Every
-- assertion cross-checks the kernel against an INDEPENDENT oracle -- either
-- GEOS ST_Distance from the sampled centre to the geometry minus the sampled
-- radius, or the already-native nearestApproachDistance/minDistance kernel.
-------------------------------------------------------------------------------

-- A long thin rectangle: its minimum bounding circle has radius about 50
-- centred near (50, 0.5), which encloses the point below even though it is
-- far from the polygon's own boundary (true distance 8.5)
SELECT round(startValue(tcbuffer 'Cbuffer(Point(2 10), 0.5)@2000-01-01' <-> geometry 'Polygon((0 0,100 0,100 1,0 1,0 0))')::numeric, 6);
SELECT round((ST_Distance('Point(2 10)'::geometry, 'Polygon((0 0,100 0,100 1,0 1,0 0))'::geometry) - 0.5)::numeric, 6);

-- A concave L-shaped polygon: the centre sits in the missing corner, close to
-- the notch but far from the bounding circle boundary
SELECT round(startValue(tcbuffer 'Cbuffer(Point(2 7), 0.3)@2000-01-01' <-> geometry 'Polygon((0 0,10 0,10 10,5 10,5 5,0 5,0 0))')::numeric, 6);
SELECT round(GREATEST(ST_Distance('Point(2 7)'::geometry, 'Polygon((0 0,10 0,10 10,5 10,5 5,0 5,0 0))'::geometry) - 0.3, 0)::numeric, 6);

-- A linestring: the bounding-circle composition would measure to the chord's
-- circle rather than the line itself
SELECT round(startValue(tcbuffer 'Cbuffer(Point(50 20), 1)@2000-01-01' <-> geometry 'Linestring(0 0,100 0)')::numeric, 6);
SELECT round((ST_Distance('Point(50 20)'::geometry, 'Linestring(0 0,100 0)'::geometry) - 1)::numeric, 6);

-- A polygon with a hole: the buffer sits inside the hole, closer to the
-- inner ring than the outer bounding circle would suggest
SELECT round(startValue(tcbuffer 'Cbuffer(Point(5 5), 0.2)@2000-01-01' <-> geometry 'Polygon((-10 -10,-10 20,20 20,20 -10,-10 -10),(0 0,10 0,10 10,0 10,0 0))')::numeric, 6);
SELECT round((ST_Distance('Point(5 5)'::geometry, 'Polygon((-10 -10,-10 20,20 20,20 -10,-10 -10),(0 0,10 0,10 10,0 10,0 0))'::geometry) - 0.2)::numeric, 6);

-- A multipolygon: the centre sits in the gap between the two components
SELECT round(startValue(tcbuffer 'Cbuffer(Point(5 15), 0.4)@2000-01-01' <-> geometry 'Multipolygon(((0 0,10 0,10 10,0 10,0 0)),((0 20,10 20,10 30,0 30,0 20)))')::numeric, 6);
SELECT round(GREATEST(ST_Distance('Point(5 15)'::geometry, 'Multipolygon(((0 0,10 0,10 10,0 10,0 0)),((0 20,10 20,10 30,0 30,0 20)))'::geometry) - 0.4, 0)::numeric, 6);

-- A moving buffer with a growing radius sweeping through a polygon interior,
-- exercising the interior-crossing (zero-clamp) turning points. The minimum
-- of the temporal distance must equal the analytic nearest-approach distance,
-- computed by the independent nad_tcbuffer_geo kernel.
SELECT round(minValue(tcbuffer '[Cbuffer(Point(-10 5), 0.2)@2000-01-01, Cbuffer(Point(20 5), 1.5)@2000-01-02]' <-> geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))')::numeric, 6);
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(-10 5), 0.2)@2000-01-01, Cbuffer(Point(20 5), 1.5)@2000-01-02]', geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))')::numeric, 6);

-- An interior extremum away from the segment's own endpoints: a buffer that
-- passes near, but not through, a polygon corner
SELECT round(minValue(tcbuffer '[Cbuffer(Point(-5 -5), 0.3)@2000-01-01, Cbuffer(Point(15 15), 0.3)@2000-01-02]' <-> geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))')::numeric, 6);
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(-5 -5), 0.3)@2000-01-01, Cbuffer(Point(15 15), 0.3)@2000-01-02]', geometry 'Polygon((0 0,10 0,10 10,0 10,0 0))')::numeric, 6);

-- A circular-arc geometry stays arc-exact, sharing the edge decomposition of
-- the already arc-exact nad_tcbuffer_geo kernel
SELECT round(minValue(tcbuffer 'Cbuffer(Point(0 8), 1)@2000-01-01' <-> geometry 'CircularString(5 0, 0 5, -5 0)')::numeric, 6);
SELECT round(nearestApproachDistance(tcbuffer 'Cbuffer(Point(0 8), 1)@2000-01-01', geometry 'CircularString(5 0, 0 5, -5 0)')::numeric, 6);

-- Discrete, step, and sequence-set dispatch against a non-point geometry:
-- every instant is a stationary disc, so there is no interior turning point
SELECT round(tcbuffer '{Cbuffer(Point(2 10), 0.5)@2000-01-01, Cbuffer(Point(50 20), 1)@2000-01-02}' <-> geometry 'Polygon((0 0,100 0,100 1,0 1,0 0))', 6);
SELECT round(tcbuffer 'Interp=Step;[Cbuffer(Point(2 10), 0.5)@2000-01-01, Cbuffer(Point(50 20), 1)@2000-01-02]' <-> geometry 'Polygon((0 0,100 0,100 1,0 1,0 0))', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(2 10), 0.5)@2000-01-01, Cbuffer(Point(3 10), 0.5)@2000-01-02], [Cbuffer(Point(50 20), 1)@2000-01-03, Cbuffer(Point(51 20), 1)@2000-01-04]}' <-> geometry 'Polygon((0 0,100 0,100 1,0 1,0 0))', 6);

-------------------------------------------------------------------------------
-- minDistance(tcbuffer, geometry) / (geometry, tcbuffer) -- scalar, reduces
-- to NAD when one side has no time dimension
-------------------------------------------------------------------------------

SELECT round(minDistance(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]', geometry 'Point(5 5)')::numeric, 6);
SELECT round(minDistance(geometry 'Point(5 5)', tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]')::numeric, 6);
-- Trajectory disc sweeps over the static point: distance 0
SELECT minDistance(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 1)@2000-01-02]', geometry 'Point(5 0)');
-- Static polygon
SELECT round(minDistance(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(2 2), 0.5)@2000-01-02]', geometry 'POLYGON((10 10, 20 10, 20 20, 10 20, 10 10))')::numeric, 6);

-------------------------------------------------------------------------------
-- minDistance(tcbuffer, cbuffer) / (cbuffer, tcbuffer) -- scalar
-------------------------------------------------------------------------------

SELECT round(minDistance(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]', cbuffer 'Cbuffer(Point(5 5), 1)')::numeric, 6);
SELECT round(minDistance(cbuffer 'Cbuffer(Point(5 5), 1)', tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]')::numeric, 6);
-- Moving disc closest to the static disc at (5,0): 5 - 1 (moving) - 2 (static) = 2
SELECT round(cbuffer 'Cbuffer(Point(5 5), 2)' |=| tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 1)@2000-01-02]', 6);

-------------------------------------------------------------------------------
-- minDistance(tcbuffer, tcbuffer) -- 2-ary aggregate, time-agnostic spatial
-- min equivalent to ST_Distance(traversedArea, traversedArea)
-------------------------------------------------------------------------------

-- Aggregate over one row equals the per-pair value
SELECT round(minDistance(t1, t2)::numeric, 6) FROM (
  SELECT tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]' AS t1,
         tcbuffer '[Cbuffer(Point(0 5), 0.5)@2000-01-01, Cbuffer(Point(10 5), 0.5)@2000-01-02]' AS t2) v;

-- Parallel segments, constant radius: gap is centerline gap minus both radii
SELECT round(minDistance(t1, t2)::numeric, 6) FROM (
  SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 1)@2000-01-02]' AS t1,
         tcbuffer '[Cbuffer(Point(0 6), 1)@2000-01-01, Cbuffer(Point(10 6), 1)@2000-01-02]' AS t2) v;

-- Crossing centerlines, tapered radius: exercises the interior critical point
SELECT round(minDistance(t1, t2)::numeric, 6) FROM (
  SELECT tcbuffer '[Cbuffer(Point(0 0), 0.2)@2000-01-01, Cbuffer(Point(10 10), 2.0)@2000-01-02]' AS t1,
         tcbuffer '[Cbuffer(Point(0 10), 2.0)@2000-01-01, Cbuffer(Point(10 0), 0.2)@2000-01-02]' AS t2) v;

-- Overlapping swept discs: distance 0
SELECT minDistance(t1, t2) FROM (
  SELECT tcbuffer '[Cbuffer(Point(0 0), 2)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02]' AS t1,
         tcbuffer '[Cbuffer(Point(5 0), 2)@2000-01-01, Cbuffer(Point(5 5), 2)@2000-01-02]' AS t2) v;

-- Instant vs sequence (TINSTANT dispatch)
SELECT round(minDistance(t1, t2)::numeric, 6) FROM (
  SELECT tcbuffer 'Cbuffer(Point(5 5), 0.5)@2000-01-01' AS t1,
         tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(10 0), 0.5)@2000-01-02]' AS t2) v;

-- Sequence set on both sides (TSEQUENCESET dispatch)
SELECT round(minDistance(t1, t2)::numeric, 6) FROM (
  SELECT tcbuffer '{[Cbuffer(Point(0 0), 0.3)@2000-01-01, Cbuffer(Point(2 0), 0.3)@2000-01-02], [Cbuffer(Point(20 20), 0.3)@2000-01-04, Cbuffer(Point(22 20), 0.3)@2000-01-05]}' AS t1,
         tcbuffer '{[Cbuffer(Point(0 4), 0.3)@2000-01-01, Cbuffer(Point(2 4), 0.3)@2000-01-02], [Cbuffer(Point(40 40), 0.3)@2000-01-04, Cbuffer(Point(42 40), 0.3)@2000-01-05]}' AS t2) v;

-- Aggregate over multiple rows: minimum across all per-pair distances
WITH pairs(t1, t2) AS (VALUES
  (tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
   tcbuffer '[Cbuffer(Point(10 10), 0.5)@2000-01-01, Cbuffer(Point(11 11), 0.5)@2000-01-02]'),
  (tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
   tcbuffer '[Cbuffer(Point(0 5), 0.5)@2000-01-01, Cbuffer(Point(1 5), 0.5)@2000-01-02]'),
  (tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
   tcbuffer '[Cbuffer(Point(2 1), 0.5)@2000-01-01, Cbuffer(Point(2 2), 0.5)@2000-01-02]'))
SELECT round(minDistance(t1, t2)::numeric, 6) FROM pairs;

-- Grouped: per-group minimum, exercises the running-threshold tightening
WITH src(g, t1, t2) AS (VALUES
  (1, tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
      tcbuffer '[Cbuffer(Point(0 10), 0.5)@2000-01-01, Cbuffer(Point(1 10), 0.5)@2000-01-02]'),
  (1, tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
      tcbuffer '[Cbuffer(Point(0 5), 0.5)@2000-01-01, Cbuffer(Point(1 5), 0.5)@2000-01-02]'),
  (2, tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]',
      tcbuffer '[Cbuffer(Point(0 2), 0.5)@2000-01-01, Cbuffer(Point(1 2), 0.5)@2000-01-02]'))
SELECT g, round(minDistance(t1, t2)::numeric, 6) FROM src GROUP BY g ORDER BY g;

-- Empty group returns NULL
SELECT minDistance(t1, t2) FROM (
  SELECT tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]' AS t1,
         tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02]' AS t2 WHERE false) v;

-- Agreement with ST_Distance over the traversed areas.  The kernel is the
-- exact circular-geometry minimum; traversedArea facets each arc into a
-- finite polygon, so the two agree only up to that polygonisation error
-- (here under 1e-3 for these small radii).
WITH d(a, b) AS (
  SELECT minDistance(t1, t2),
         min(ST_Distance(traversedArea(t1), traversedArea(t2)))
  FROM (
    SELECT tcbuffer '[Cbuffer(Point(0 0), 0.4)@2000-01-01, Cbuffer(Point(8 3), 0.7)@2000-01-02, Cbuffer(Point(12 1), 0.5)@2000-01-03]' AS t1,
           tcbuffer '[Cbuffer(Point(2 9), 0.6)@2000-01-01, Cbuffer(Point(9 7), 0.3)@2000-01-02, Cbuffer(Point(13 8), 0.5)@2000-01-03]' AS t2) v)
SELECT abs(a - b) < 1e-3 FROM d;

-------------------------------------------------------------------------------
-- nearestApproachInstant
-------------------------------------------------------------------------------

-- Every overload, against a buffer sweeping from (0 0) to (4 0). Each probe is
-- placed so that a single instant attains the minimum.
SELECT asText(nearestApproachInstant(geometry 'Point(2 3)', tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]'));
SELECT asText(nearestApproachInstant(cbuffer 'Cbuffer(Point(4 9), 0.7)', tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]'));
SELECT asText(nearestApproachInstant(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]', geometry 'Point(2 3)'));
SELECT asText(nearestApproachInstant(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]', cbuffer 'Cbuffer(Point(4 9), 0.7)'));
SELECT asText(nearestApproachInstant(
  tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]',
  tcbuffer '[Cbuffer(Point(0 9), 0.5)@2000-01-01, Cbuffer(Point(4 1), 0.5)@2000-01-05]'));

-- The instant answers the same minimisation the distance reports, so measuring
-- the distance at the instant it names gives the distance itself. Taken on the
-- centreline against the circle's defining vertices, the buffer probe would
-- instead name x = 3.3, the probe radius subtracted along x.
WITH d(a, b) AS (
  SELECT nearestApproachDistance(nearestApproachInstant(t, p), p),
         nearestApproachDistance(t, p)
  FROM (SELECT tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]' AS t,
               cbuffer 'Cbuffer(Point(4 9), 0.7)' AS p) v)
SELECT a = b FROM d;
-- The two argument orders name the same instant
SELECT nearestApproachInstant(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]', cbuffer 'Cbuffer(Point(4 9), 0.7)')
     = nearestApproachInstant(cbuffer 'Cbuffer(Point(4 9), 0.7)', tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]');

-------------------------------------------------------------------------------


-- Analytic nearest approach distance (|=| and nearestApproachDistance):
-- polygons, lines, holes, multi, containing polygon (interior ray-cast),
-- and a curved type (exact traversed-area fallback)
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Polygon((-3 -3,-3 3,3 3,3 -3,-3 -3))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Polygon((-10 -10,-10 10,10 10,10 -10,-10 -10))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Linestring(4 -3,4 6)', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'Circularstring(5 0,7 2,9 0)', 6);
-- Circular-arc input: on-span bulge, non-vertex arc interior (discriminates native
-- arc-exact from GEOS stroking), off-span endpoint, inside-circle, and moving
SELECT round(tcbuffer 'Cbuffer(Point(0 8), 1)@2000-01-01' |=| geometry 'CircularString(5 0, 0 5, -5 0)', 6);
SELECT round(tcbuffer 'Cbuffer(Point(3 8), 1)@2000-01-01' |=| geometry 'CircularString(5 0, 0 5, -5 0)', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 -10), 1)@2000-01-01' |=| geometry 'CircularString(5 0, 0 5, -5 0)', 6);
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 5)@2000-01-01' |=| geometry 'CircularString(5 0, 0 5, -5 0)', 6);
SELECT round(tcbuffer '[Cbuffer(Point(0 8), 1)@2000-01-01, Cbuffer(Point(0 5), 1)@2000-01-02]' |=| geometry 'CircularString(5 0, 0 5, -5 0)', 6);
-- Compound curve (arc chained to a line) and multi curve: the arc components are
-- decomposed arc-exactly (a value matching the arc-only case proves native, not
-- GEOS stroking), and the line components are walked too (line nearer than arc)
SELECT round(tcbuffer 'Cbuffer(Point(3 8), 1)@2000-01-01' |=| geometry 'CompoundCurve(CircularString(5 0, 0 5, -5 0),(-5 0, -5 -8))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(-8 -4), 1)@2000-01-01' |=| geometry 'CompoundCurve(CircularString(5 0, 0 5, -5 0),(-5 0, -5 -8))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(8 6), 1)@2000-01-01' |=| geometry 'MultiCurve((6 6, 10 10),CircularString(5 0, 0 5, -5 0))', 6);
-- Curve polygon (arc ring, a disk of radius 5 at the origin): a centre inside
-- the arc ring gives distance 0 (arc-aware interior ray cast), and an outside
-- centre measures to the arc boundary exactly (non-vertex, unreachable by
-- stroking)
SELECT round(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01' |=| geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', 6);
SELECT round(tcbuffer 'Cbuffer(Point(4 4), 0.3)@2000-01-01' |=| geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))', 6);
SELECT round(geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))' |=| tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', 6);
SELECT round(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]' |=| geometry 'Polygon((20 20,20 24,24 24,24 20,20 20))', 6);
SELECT round(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]' |=| geometry 'Linestring(20 -5,20 20)', 6);
SELECT round(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]' |=| geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))', 6);
SELECT round(tcbuffer 'Interp=Step;[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(5 5), 3)@2000-01-02]' |=| geometry 'Polygon((11 -1,11 3,14 3,14 -1,11 -1))', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(4 0), 1)@2000-01-02], [Cbuffer(Point(20 20), 2)@2000-01-03, Cbuffer(Point(25 20), 1)@2000-01-04]}' |=| geometry 'Polygon((-5 -5,-5 15,15 15,15 -5,-5 -5),(0 0,4 0,4 4,0 4,0 0))', 6);
SELECT round(tcbuffer '{[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(4 0), 1)@2000-01-02], [Cbuffer(Point(20 20), 2)@2000-01-03, Cbuffer(Point(25 20), 1)@2000-01-04]}' |=| geometry 'Multilinestring((50 50,60 60),(2 -3,2 9))', 6);
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]', geometry 'Polygon((11 -1,11 3,14 3,14 -1,11 -1))'), 6);
SELECT round(nearestApproachDistance(geometry 'Linestring(4 -3,4 6)', tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01'), 6);
SELECT round(nearestApproachDistance(tcbuffer '{Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(8 3), 2)@2000-01-02}', geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))'), 6);

-- Perpendicular perfect-square crossing: the buffer centre sweeps across the
-- edge's supporting line at an interior projection, so the squared
-- perpendicular distance is a perfect square whose stationarity discriminant
-- rounds to a small negative; the crossing (overlap) instant must still be
-- taken, giving 0 rather than the edge-endpoint distance.
SELECT round(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(141.4213562373095 173.20508075688772), 1)@2000-01-02]' |=| geometry 'Linestring(173.20508075688772 0, 0 141.4213562373095)', 6);

-- A circular buffer as the other argument, in both orders. The distance between
-- two buffers is the distance between their centres less both radii, so centres
-- sqrt(2) apart with radii 0.5 and 0.7 give sqrt(2) - 0.5 - 0.7 = 0.214214
-- exactly, the two orders of a commutative operation agree, and minDistance --
-- which names the same kernel -- agrees with both.
SELECT round(nearestApproachDistance(tcbuffer 'Cbuffer(Point(0 0), 0.5)@2000-01-01', cbuffer 'Cbuffer(Point(1 1), 0.7)')::numeric, 6);
SELECT round(nearestApproachDistance(cbuffer 'Cbuffer(Point(1 1), 0.7)', tcbuffer 'Cbuffer(Point(0 0), 0.5)@2000-01-01')::numeric, 6);
SELECT nearestApproachDistance(tcbuffer 'Cbuffer(Point(0 0), 0.5)@2000-01-01', cbuffer 'Cbuffer(Point(1 1), 0.7)')
     = nearestApproachDistance(cbuffer 'Cbuffer(Point(1 1), 0.7)', tcbuffer 'Cbuffer(Point(0 0), 0.5)@2000-01-01');
SELECT nearestApproachDistance(tcbuffer 'Cbuffer(Point(0 0), 0.5)@2000-01-01', cbuffer 'Cbuffer(Point(1 1), 0.7)')
     = minDistance(tcbuffer 'Cbuffer(Point(0 0), 0.5)@2000-01-01', cbuffer 'Cbuffer(Point(1 1), 0.7)');
-- A buffer beside the path: the distance is the centre distance less both radii
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]', cbuffer 'Cbuffer(Point(4 9), 0.7)')::numeric, 6);

-- A box carrying a period measures the part of the temporal buffer inside it
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]', stbox 'STBOX XT(((10,-1),(12,1)),[2000-01-01, 2000-01-03])')::numeric, 6);
SELECT round(nearestApproachDistance(stbox 'STBOX XT(((10,-1),(12,1)),[2000-01-01, 2000-01-03])', tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]')::numeric, 6);
-- The same box with no period measures the whole temporal buffer
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]', stbox 'STBOX X((10,-1),(12,1))')::numeric, 6);
-- No part of the temporal buffer is inside the period
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0), 0.5)@2000-01-01, Cbuffer(Point(4 0), 0.5)@2000-01-05]', stbox 'STBOX XT(((10,-1),(12,1)),[2000-02-01, 2000-02-02])')::numeric, 6);

-- The shortest line against a static circular buffer runs between the two disk
-- boundaries, so its length is the nearest approach distance of the same pair.
-- Reading the disc and the traversed area as polygons measures between two
-- polygonal approximations and answers neither. A disk of radius 1 passing a
-- disk of radius 2 six units away has a nearest approach of 3 at the midpoint,
-- where the line leaves (5 1) and reaches (5 4)
SELECT ST_AsText(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 1)@2000-01-02]', cbuffer 'Cbuffer(Point(5 6), 2)'));
SELECT round(ST_Length(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 1)@2000-01-02]', cbuffer 'Cbuffer(Point(5 6), 2)'))::numeric, 6);
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 1)@2000-01-02]', cbuffer 'Cbuffer(Point(5 6), 2)')::numeric, 6);
-- The two argument orders name the same line
SELECT ST_AsText(shortestLine(cbuffer 'Cbuffer(Point(5 6), 2)', tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 1)@2000-01-02]'));
-- Disks that meet: the line degenerates where the boundary of one reaches the other
SELECT ST_AsText(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 3)@2000-01-01, Cbuffer(Point(10 0), 3)@2000-01-02]', cbuffer 'Cbuffer(Point(5 4), 2)'));
-- A radius that varies over the segment carries the witness off the midpoint,
-- as it does for the nearest approach distance the line must realise
SELECT round(ST_Length(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 5)@2000-01-05]', cbuffer 'Cbuffer(Point(5 6), 0)'))::numeric, 6);
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 5)@2000-01-05]', cbuffer 'Cbuffer(Point(5 6), 0)')::numeric, 6);

-------------------------------------------------------------------------------

-- Analytic shortestLine: its length equals the nearest-approach distance;
-- exercises the witness path over polygons, lines, hole, multi, a
-- containing polygon (degenerate) and a curved type (exact fallback)
SELECT geometrytype(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))'));
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Linestring(4 -3,4 6)'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Polygon((-2 -2,-2 30,30 30,30 -2,-2 -2))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'Circularstring(5 0,7 2,9 0)'))::numeric, 6);
SELECT round(ST_Length(shortestLine(geometry 'Polygon((5 5,5 8,8 8,8 5,5 5))', tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]', geometry 'Polygon((20 20,20 24,24 24,24 20,20 20))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 2)@2000-01-02, Cbuffer(Point(10 10), 1)@2000-01-03]', geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Interp=Step;[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(5 5), 3)@2000-01-02]', geometry 'Polygon((11 -1,11 3,14 3,14 -1,11 -1))'))::numeric, 6);
-- Curved input: the analytic shortest line is arc-exact, so its length equals
-- the arc-exact nearest-approach distance (2.544004, 0, 0.356854 respectively)
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(3 8), 1)@2000-01-01', geometry 'CircularString(5 0, 0 5, -5 0)'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(0 0), 1)@2000-01-01', geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))'))::numeric, 6);

-- The moving shortest line runs between the two disk boundaries, so its length
-- is the nearest approach distance of the same pair rather than the distance of
-- the two centres, which exceeds it by the two radii. The two disks hold a
-- constant separation, so the length is the same wherever the witness is taken
-- and the test does not depend on how equal minima are broken
SELECT round(ST_Length(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 1)@2000-01-05]', tcbuffer '[Cbuffer(Point(0 6), 2)@2000-01-01, Cbuffer(Point(10 6), 2)@2000-01-05]'))::numeric, 6);
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(10 0), 1)@2000-01-05]', tcbuffer '[Cbuffer(Point(0 6), 2)@2000-01-01, Cbuffer(Point(10 6), 2)@2000-01-05]')::numeric, 6);
-- Disks that meet: the line degenerates where the boundary of one reaches the other
SELECT round(ST_Length(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 3)@2000-01-01, Cbuffer(Point(10 0), 3)@2000-01-05]', tcbuffer '[Cbuffer(Point(0 4), 2)@2000-01-01, Cbuffer(Point(10 4), 2)@2000-01-05]'))::numeric, 6);
-- Operands whose time frames do not meet have no nearest approach
SELECT ST_AsText(shortestLine(tcbuffer '[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(1 0), 1)@2000-01-02]', tcbuffer '[Cbuffer(Point(0 6), 2)@2000-01-05, Cbuffer(Point(1 6), 2)@2000-01-06]'));
-- Two moving disks whose closest approach falls INSIDE a segment. One crosses
-- in front of a stationary one six units away, so the gap dips to 6-1-2 = 3 at
-- the midpoint while both endpoints stand sqrt(61)-3 = 4.810250 apart. Reading
-- the endpoints alone reports the larger value and leaves the temporal
-- distance constant
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(10 0),1)@2000-01-05]', tcbuffer '[Cbuffer(Point(5 6),2)@2000-01-01, Cbuffer(Point(5 6),2)@2000-01-05]')::numeric, 6);
SELECT asText(tdistance(tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(10 0),1)@2000-01-05]', tcbuffer '[Cbuffer(Point(5 6),2)@2000-01-01, Cbuffer(Point(5 6),2)@2000-01-05]'), 6);
SELECT asText(nearestApproachInstant(tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(10 0),1)@2000-01-05]', tcbuffer '[Cbuffer(Point(5 6),2)@2000-01-01, Cbuffer(Point(5 6),2)@2000-01-05]'));
-- A radius growing over the segment moves the minimiser off the midpoint: with
-- the second radius running 0 to 4 the gap is least at f = 0.761861, where it
-- measures 2.499091 against 2.810250 read at the endpoint
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(10 0),1)@2000-01-05]', tcbuffer '[Cbuffer(Point(5 6),0)@2000-01-01, Cbuffer(Point(5 6),4)@2000-01-05]')::numeric, 6);
-- Disks that keep their separation have no interior minimum to find
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(10 0),1)@2000-01-05]', tcbuffer '[Cbuffer(Point(0 6),2)@2000-01-01, Cbuffer(Point(10 6),2)@2000-01-05]')::numeric, 6);
-- A gap that dips below zero is clamped there by the distance, so the value is
-- flat between the two instants at which the disks meet and those instants, not
-- the minimum, are its breakpoints. One disk passes straight through another,
-- meeting it at f = 0.2 and f = 0.8 of the segment
SELECT asText(tdistance(tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(10 0),1)@2000-01-05]', tcbuffer '[Cbuffer(Point(5 0),2)@2000-01-01, Cbuffer(Point(5 0),2)@2000-01-05]'), 6);
-- A static circular buffer asks the same question as a segment that does not
-- move, so the temporal distance against one carries the same turning point and
-- agrees with the nearest approach distance of the pair
SELECT asText(tdistance(tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(10 0),5)@2000-01-05]', cbuffer 'Cbuffer(Point(5 6),0)'), 6);
SELECT round(nearestApproachDistance(tcbuffer '[Cbuffer(Point(0 0),1)@2000-01-01, Cbuffer(Point(10 0),5)@2000-01-05]', cbuffer 'Cbuffer(Point(5 6),0)')::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer 'Cbuffer(Point(4 4), 0.3)@2000-01-01', geometry 'CurvePolygon(CircularString(5 0, 0 5, -5 0, 0 -5, 5 0))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '{[Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(4 0), 1)@2000-01-02], [Cbuffer(Point(20 20), 2)@2000-01-03, Cbuffer(Point(25 20), 1)@2000-01-04]}', geometry 'Polygon((-5 -5,-5 15,15 15,15 -5,-5 -5),(0 0,4 0,4 4,0 4,0 0))'))::numeric, 6);
SELECT round(ST_Length(shortestLine(tcbuffer '{Cbuffer(Point(0 0), 1)@2000-01-01, Cbuffer(Point(8 3), 2)@2000-01-02}', geometry 'Multipolygon(((200 200,200 210,210 210,210 200,200 200)),((9 -1,9 1,12 1,12 -1,9 -1)))'))::numeric, 6);

-------------------------------------------------------------------------------
