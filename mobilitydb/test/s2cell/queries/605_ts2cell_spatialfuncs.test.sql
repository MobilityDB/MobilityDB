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

-- The six shared DGGS operations lifted over a temporal value, plus the token.
--
-- Each answer is cross-checked against the static form it lifts: the temporal
-- operation applied to a one-instant value must equal the static operation
-- applied to that instant's cell.

-------------------------------------------------------------------------------
-- Resolution, validity and area
-------------------------------------------------------------------------------

SELECT asText(getResolution(ts2cell '[47c3c3@2001-01-01, 47c3c38705f@2001-01-02]'));
SELECT asText(isValidCell(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]'));
SELECT startValue(getResolution(ts2cell '[47c3c3@2001-01-01]'))
  = getResolution(s2cell '47c3c3');
SELECT round(startValue(cellArea(ts2cell '[47c3c3@2001-01-01]'))::numeric, 6)
  = round(cellArea(s2cell '47c3c3')::numeric, 6);

-------------------------------------------------------------------------------
-- Hierarchy
-------------------------------------------------------------------------------

SELECT asText(cellToParent(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]', 5));
SELECT startValue(cellToParent(ts2cell '[47c3c3@2001-01-01]', 5))
  = cellToParent(s2cell '47c3c3', 5);

-------------------------------------------------------------------------------
-- Centre and boundary
--
-- An S2 cell is a region of the sphere, so the centre is a tgeogpoint and the
-- boundary a tgeography, both in SRID 4326.
-------------------------------------------------------------------------------

SELECT tempSubtype(cellToPoint(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]'));
SELECT tempBasetype(cellToPoint(ts2cell '[47c3c3@2001-01-01]'));
SELECT tempBasetype(cellToBoundary(ts2cell '[47c3c3@2001-01-01]'));
SELECT startValue(cellToPoint(ts2cell '[47c3c3@2001-01-01]'))::geometry
  = cellToPoint(s2cell '47c3c3')::geometry;
SELECT ST_NPoints(startValue(cellToBoundary(ts2cell '[47c3c3@2001-01-01]'))::geometry);
SELECT ST_SRID(startValue(cellToBoundary(ts2cell '[47c3c3@2001-01-01]'))::geometry);

-------------------------------------------------------------------------------
-- The token
-------------------------------------------------------------------------------

SELECT asText(ts2CellToToken(ts2cell '[47c3c3@2001-01-01, 54b5c9@2001-01-02]'));
SELECT startValue(ts2CellToToken(ts2cell '[47c3c3@2001-01-01]'))
  = s2CellToToken(s2cell '47c3c3');

-------------------------------------------------------------------------------
