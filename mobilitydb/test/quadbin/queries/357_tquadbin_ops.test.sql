-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
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

-- tquadbin accessors and lifted cell operations: value accessors, resolution,
-- validity, parent, centroid point, boundary, area, quadkey, and the
-- tquadbin -> tgeompoint cast. Each lifts its static kernel over the time axis.

-------------------------------------------------------------------------------
-- Value accessors
-------------------------------------------------------------------------------

SELECT startValue(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT endValue(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT valueN(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}', 2);
SELECT getValues(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT valueAtTimestamp(tquadbin '[480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-03]', '2001-01-01');

-------------------------------------------------------------------------------
-- Resolution + validity (lifted)
-------------------------------------------------------------------------------

SELECT tquadbinGetResolution(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');
SELECT isValidCell(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');

-------------------------------------------------------------------------------
-- Hierarchy (lifted parent at a constant resolution)
-------------------------------------------------------------------------------

SELECT tquadbinCellToParent(tquadbin '{48427fffffffffff@2001-01-01, 48a6227affffffff@2001-01-02}', 0);

-- Every instant's parent at res 0 is the z0 world cell
SELECT getValues(tquadbinCellToParent(
  tquadbin '{48427fffffffffff@2001-01-01, 48a6227affffffff@2001-01-02}', 0))
  = quadbinset '{480fffffffffffff}';

-------------------------------------------------------------------------------
-- Centroid point / boundary (lifted)
-------------------------------------------------------------------------------

SELECT tquadbinCellToPoint(tquadbin '{48a6227affffffff@2001-01-01}');
SELECT asText(tquadbinCellToPoint(tquadbin '{48a6227affffffff@2001-01-01}'), 6);
SELECT tquadbinCellToBoundary(tquadbin '{48a6227affffffff@2001-01-01}') IS NOT NULL;

-------------------------------------------------------------------------------
-- Area (lifted, tfloat)
-------------------------------------------------------------------------------

SELECT round(startValue(tquadbinCellArea(tquadbin '{480fffffffffffff@2001-01-01}'))::numeric, 1);

-------------------------------------------------------------------------------
-- Quadkey (lifted, ttext)
-------------------------------------------------------------------------------

SELECT tquadbinCellToQuadkey(tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}');

-------------------------------------------------------------------------------
-- Convenience cast: tquadbin :: tgeompoint  (reuses cell_to_point centroid)
-------------------------------------------------------------------------------

SELECT (tquadbin '{48a6227affffffff@2001-01-01}')::tgeompoint
  = tquadbinCellToPoint(tquadbin '{48a6227affffffff@2001-01-01}');
SELECT asText((tquadbin '{48a6227affffffff@2001-01-01}')::tgeompoint, 6);

-------------------------------------------------------------------------------
