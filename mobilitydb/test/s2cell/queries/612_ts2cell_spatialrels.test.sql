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

-- Ever and always spatial relationships between a ts2cell and a geography,
-- or two ts2cell values.
--
-- An S2 cell is a region of the sphere bounded by great-circle arcs, so each
-- relationship is the one of the temporal geography of the cell boundary,
-- answered on the sphere; the answer must equal the same relationship asked
-- of cellToBoundary directly, and a distance is in metres.

-------------------------------------------------------------------------------
-- The delegation identity
-------------------------------------------------------------------------------

SELECT eIntersects(ts2cell '[47c3c3@2001-01-01]', geography 'Point(4.35 50.85)')
  = eIntersects(cellToBoundary(ts2cell '[47c3c3@2001-01-01]'),
      geography 'Point(4.35 50.85)');
SELECT aIntersects(geography 'Point(4.35 50.85)', ts2cell '[47c3c3@2001-01-01]')
  = aIntersects(geography 'Point(4.35 50.85)',
      cellToBoundary(ts2cell '[47c3c3@2001-01-01]'));

-------------------------------------------------------------------------------
-- A point inside its own cell, and one far away
-------------------------------------------------------------------------------

SELECT eIntersects(ts2cell '[47c3c3@2001-01-01]', geography 'Point(4.35 50.85)');
SELECT eIntersects(ts2cell '[47c3c3@2001-01-01]', geography 'Point(-122.4 37.8)');
SELECT eDisjoint(ts2cell '[47c3c3@2001-01-01]', geography 'Point(-122.4 37.8)');
SELECT eDwithin(ts2cell '[47c3c3@2001-01-01]', geography 'Point(4.35 50.85)', 1000.0);
SELECT aDwithin(ts2cell '[47c3c3@2001-01-01]', ts2cell '[47c3c3@2001-01-01]', 1.0);

-------------------------------------------------------------------------------
-- The cell is the region S2 assigns its points to. The first point lies in
-- the level-2 cell of the queries below and 1,220 km outside the polygon
-- joining its vertices with straight lines in longitude and latitude; the
-- second lies 397 km inside that polygon and outside the cell.
-------------------------------------------------------------------------------

SELECT geoToS2Cell(geography 'Point(88.8843 79.0319)', 2) = s2cell '45',
  eIntersects(ts2cell '[45@2001-01-01]', geography 'Point(88.8843 79.0319)');
SELECT geoToS2Cell(geography 'Point(-0.3719 71.2156)', 2) = s2cell '45',
  eIntersects(ts2cell '[45@2001-01-01]', geography 'Point(-0.3719 71.2156)');

-------------------------------------------------------------------------------
