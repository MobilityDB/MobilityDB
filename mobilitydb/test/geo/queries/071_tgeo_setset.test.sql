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

-------------------------------------------------------------------------------
-- Set-set minimum distance
-------------------------------------------------------------------------------

SELECT round(minDistance(
  ARRAY[tgeometry '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-02]'],
  ARRAY[tgeometry '[Point(0 5)@2000-01-01, Point(1 5)@2000-01-02]'])::numeric, 6);
SELECT round(minDistance(
  tgeometry '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-02]', geometry 'Point(0 5)')::numeric, 6);
SELECT round(minDistance(t1, t2)::numeric, 6) FROM (VALUES
  (tgeometry '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
   tgeometry '[Point(0 3)@2000-01-01, Point(1 3)@2000-01-02]'),
  (tgeometry '[Point(0 0)@2000-01-01, Point(1 1)@2000-01-02]',
   tgeometry '[Point(0 2)@2000-01-01, Point(1 2)@2000-01-02]')) v(t1, t2);
-- Geodetic array and aggregate
SELECT round(minDistance(
  ARRAY[tgeography '[Point(0 0)@2000-01-01, Point(0 1)@2000-01-02]'],
  ARRAY[tgeography '[Point(0 2)@2000-01-01, Point(0 3)@2000-01-02]'])::numeric, 0);

-------------------------------------------------------------------------------
-- Set-set spatial join: ever / always
-------------------------------------------------------------------------------

SELECT i, j FROM eDwithinPairs(
  ARRAY[tgeometry '[Point(0 0)@2000-01-01, Point(0 10)@2000-01-02]',
        tgeometry '[Point(50 50)@2000-01-01, Point(50 60)@2000-01-02]'],
  ARRAY[tgeometry '[Point(1 0)@2000-01-01, Point(1 10)@2000-01-02]',
        tgeometry '[Point(0 0)@2000-01-01, Point(0 10)@2000-01-02]'],
  2.0) ORDER BY i, j;
SELECT i, j FROM aDwithinPairs(
  ARRAY[tgeometry '[Point(0 0)@2000-01-01, Point(0 10)@2000-01-02]'],
  ARRAY[tgeometry '[Point(1 0)@2000-01-01, Point(1 10)@2000-01-02]'],
  2.0) ORDER BY i, j;
SELECT i, j FROM eIntersectsPairs(
  ARRAY[tgeometry '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-02]'],
  ARRAY[tgeometry '[Point(0 2)@2000-01-01, Point(2 0)@2000-01-02]']) ORDER BY i, j;
SELECT i, j FROM aIntersectsPairs(
  ARRAY[tgeometry '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-02]'],
  ARRAY[tgeometry '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-02]']) ORDER BY i, j;
SELECT i, j FROM eDisjointPairs(
  ARRAY[tgeometry '[Point(0 0)@2000-01-01, Point(0 10)@2000-01-02]'],
  ARRAY[tgeometry '[Point(5 0)@2000-01-01, Point(5 10)@2000-01-02]']) ORDER BY i, j;
SELECT i, j FROM aDisjointPairs(
  ARRAY[tgeometry '[Point(0 0)@2000-01-01, Point(0 10)@2000-01-02]',
        tgeometry '[Point(50 50)@2000-01-01, Point(50 60)@2000-01-02]'],
  ARRAY[tgeometry '[Point(1 0)@2000-01-01, Point(1 10)@2000-01-02]',
        tgeometry '[Point(0 0)@2000-01-01, Point(0 10)@2000-01-02]']) ORDER BY i, j;
-- Touches: geometry only (geodetic touches is undefined)
SELECT i, j FROM eTouchesPairs(
  ARRAY[tgeometry '[Polygon((0 0,2 0,2 2,0 2,0 0))@2000-01-01]'],
  ARRAY[tgeometry '[Polygon((2 0,4 0,4 2,2 2,2 0))@2000-01-01]']) ORDER BY i, j;
SELECT i, j FROM aTouchesPairs(
  ARRAY[tgeometry '[Polygon((0 0,2 0,2 2,0 2,0 0))@2000-01-01]'],
  ARRAY[tgeometry '[Polygon((2 0,4 0,4 2,2 2,2 0))@2000-01-01]']) ORDER BY i, j;
-- Geodetic intersects
SELECT i, j FROM eIntersectsPairs(
  ARRAY[tgeography '[Point(0 0)@2000-01-01, Point(0 1)@2000-01-02]'],
  ARRAY[tgeography '[Point(0 0)@2000-01-01, Point(0 1)@2000-01-02]']) ORDER BY i, j;

-------------------------------------------------------------------------------
-- Set-set spatial join: temporal
-------------------------------------------------------------------------------

SELECT i, j FROM tDwithinPairs(
  ARRAY[tgeometry '[Point(0 0)@2000-01-01, Point(0 10)@2000-01-02]'],
  ARRAY[tgeometry '[Point(1 0)@2000-01-01, Point(1 10)@2000-01-02]'],
  2.0) ORDER BY i, j;
SELECT i, j FROM tIntersectsPairs(
  ARRAY[tgeometry '[Point(0 0)@2000-01-01, Point(2 2)@2000-01-02]'],
  ARRAY[tgeometry '[Point(0 2)@2000-01-01, Point(2 0)@2000-01-02]']) ORDER BY i, j;
SELECT i, j FROM tTouchesPairs(
  ARRAY[tgeometry '[Polygon((0 0,2 0,2 2,0 2,0 0))@2000-01-01]'],
  ARRAY[tgeometry '[Polygon((2 0,4 0,4 2,2 2,2 0))@2000-01-01]']) ORDER BY i, j;
SELECT i, j FROM tDisjointPairs(
  ARRAY[tgeography '[Point(0 0)@2000-01-01, Point(0 1)@2000-01-02]'],
  ARRAY[tgeography '[Point(0 0)@2000-01-01, Point(0 1)@2000-01-02]']) ORDER BY i, j;

-------------------------------------------------------------------------------
-- Edge cases
-------------------------------------------------------------------------------

SELECT i, j FROM eDwithinPairs(ARRAY[]::tgeometry[],
  ARRAY[tgeometry 'Point(0 0)@2000-01-01'], 2.0) ORDER BY i, j;
SELECT i, j FROM aDisjointPairs(
  ARRAY[tgeometry '[Point(0 0)@2000-01-01, Point(0 10)@2000-01-02]'],
  ARRAY[tgeometry '[Point(5 5)@2000-03-01, Point(5 10)@2000-03-02]']) ORDER BY i, j;

-------------------------------------------------------------------------------
