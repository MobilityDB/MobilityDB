-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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

-- Temporal spatial relationships between a ts2cell and a geometry. Each
-- delegates through the cell boundary, so the answer must equal the same
-- relationship asked of cellToBoundary directly.

-------------------------------------------------------------------------------
-- The delegation identity
-------------------------------------------------------------------------------

SELECT asText(tIntersects(ts2cell '[47c3c3@2001-01-01]', geometry 'SRID=4326;Point(4.35 50.85)'))
  = asText(tIntersects(cellToBoundary(ts2cell '[47c3c3@2001-01-01]')::tgeometry,
      geometry 'SRID=4326;Point(4.35 50.85)'));
SELECT asText(tDisjoint(geometry 'SRID=4326;Point(-122.4 37.8)', ts2cell '[47c3c3@2001-01-01]'))
  = asText(tDisjoint(geometry 'SRID=4326;Point(-122.4 37.8)',
      cellToBoundary(ts2cell '[47c3c3@2001-01-01]')::tgeometry));

-------------------------------------------------------------------------------
-- The answers themselves
-------------------------------------------------------------------------------

SELECT asText(tIntersects(ts2cell '[47c3c3@2001-01-01]', geometry 'SRID=4326;Point(4.35 50.85)'));
SELECT asText(tDisjoint(ts2cell '[47c3c3@2001-01-01]', geometry 'SRID=4326;Point(-122.4 37.8)'));
SELECT asText(tContains(geometry 'SRID=4326;Polygon((0 45,0 55,10 55,10 45,0 45))',
  ts2cell '[47c3c3@2001-01-01]'));
SELECT asText(tDwithin(ts2cell '[47c3c3@2001-01-01]', geometry 'SRID=4326;Point(4.35 50.85)', 0.1));

-------------------------------------------------------------------------------
