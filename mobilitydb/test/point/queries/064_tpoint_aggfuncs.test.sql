-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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

SELECT asText(tcentroid(temp)) FROM (VALUES
(NULL::tgeompoint),('Point(1 1)@2000-01-01'::tgeompoint),(NULL::tgeompoint)) t(temp);

SELECT asText(tcentroid(temp)) FROM (VALUES
  (tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02)'),
  (tgeompoint '[Point(3 3)@2000-01-03, Point(4 4)@2000-01-04)'),
  (tgeompoint '[Point(2 2)@2000-01-02, Point(3 3)@2000-01-03)')) t(temp);
SELECT asText(tcentroid(temp)) FROM (VALUES
  (tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02)'),
  (tgeompoint '[Point(3 3 3)@2000-01-03, Point(4 4 4)@2000-01-04)'),
  (tgeompoint '[Point(2 2 2)@2000-01-02, Point(3 3 3)@2000-01-03)')) t(temp);

/* Errors */
SELECT asText(tcentroid(temp)) FROM (VALUES
  (tgeompoint 'Point(0 0)@2000-01-01'),
  (tgeompoint 'srid=5676;Point(1 1)@2000-01-01'),
  ('Point(2 2)@2000-01-01')) t(temp);
SELECT asText(tcentroid(temp)) FROM (VALUES
  (tgeompoint 'Point(0 0)@2000-01-01'),
  (tgeompoint 'Point(1 1)@2000-01-01'),
  ('Point(2 2 2)@2000-01-01')) t(temp);
SELECT asText(tcentroid(temp)) FROM (VALUES
  (tgeompoint 'Point(0 0)@2000-01-01'),
  (tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02}'),
  ('Point(2 2 2)@2000-01-01')) t(temp);
SELECT asText(tcentroid(temp)) FROM (VALUES
  (tgeompoint 'Point(0 0)@2000-01-01'),
  (tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]'),
  ('Point(2 2 2)@2000-01-01')) t(temp);
SELECT asText(tcentroid(temp)) FROM (VALUES
  (tgeompoint '[Point(0 0)@2000-01-01]'),
  (tgeompoint 'Interp=Stepwise;[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]'),
  ('Point(2 2 2)@2000-01-01')) t(temp);

-------------------------------------------------------------------------------

SELECT extent(temp) FROM (VALUES
(NULL::tgeompoint),('Point(1 1)@2000-01-01'::tgeompoint),(NULL::tgeompoint)) t(temp);

SELECT extent(temp) FROM (VALUES
  (tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02)'),
  (tgeompoint '[Point(3 3)@2000-01-03, Point(4 4)@2000-01-04)'),
  (tgeompoint '[Point(2 2)@2000-01-02, Point(3 3)@2000-01-03)')) t(temp);
SELECT round(extent(temp), 13) FROM (VALUES
  (tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02)'),
  (tgeogpoint '[Point(3 3 3)@2000-01-03, Point(4 4 4)@2000-01-04)'),
  (tgeogpoint '[Point(2 2 2)@2000-01-02, Point(3 3 3)@2000-01-03)')) t(temp);

/* Errors */
SELECT extent(temp) FROM (VALUES
  (tgeompoint 'Point(1 1 1)@2000-01-01'),
  (tgeompoint 'Point(1 1)@2000-01-01')) t(temp);

-------------------------------------------------------------------------------
