-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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
-- Discrete Frechet distance
-------------------------------------------------------------------------------

SELECT frechetDistance(tint '1@2000-01-01', tint '1@2000-01-01');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '1@2000-01-01');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '1@2000-01-01');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '1@2000-01-01');
SELECT frechetDistance(tint '1@2000-01-01', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT frechetDistance(tint '1@2000-01-01', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT frechetDistance(tint '1@2000-01-01', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');

SELECT frechetDistance(tint '1@2000-01-01', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tint '1@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tint '1@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tint '1@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

SELECT frechetDistance(tint '1@2000-01-01', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tint '1@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tint '1@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tint '1@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT frechetDistance(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT frechetDistance(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT frechetDistance(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

SELECT frechetDistance(tfloat '1.5@2000-01-01', tint '1@2000-01-01');
SELECT frechetDistance(tfloat '1.5@2000-01-01', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT frechetDistance(tfloat '1.5@2000-01-01', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT frechetDistance(tfloat '1.5@2000-01-01', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT frechetDistance(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '1@2000-01-01');
SELECT frechetDistance(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT frechetDistance(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT frechetDistance(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT frechetDistance(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '1@2000-01-01');
SELECT frechetDistance(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT frechetDistance(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT frechetDistance(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT frechetDistance(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '1@2000-01-01');
SELECT frechetDistance(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT frechetDistance(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT frechetDistance(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');

SELECT frechetDistance(tfloat '1.5@2000-01-01', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '1.5@2000-01-01');
SELECT frechetDistance(tfloat '1.5@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT frechetDistance(tfloat '1.5@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT frechetDistance(tfloat '1.5@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT frechetDistance(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT frechetDistance(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT frechetDistance(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

-------------------------------------------------------------------------------
-- Discrete Frechet distance path
-------------------------------------------------------------------------------

WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '1@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2000-01-01', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2000-01-01', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2000-01-01', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2000-01-01', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2000-01-01', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '1.5@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT frechetDistancePath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;

-------------------------------------------------------------------------------
-- Dynamic Time Warp (DTW) distance
-------------------------------------------------------------------------------

SELECT dynamicTimeWarp(tint '1@2000-01-01', tint '1@2000-01-01');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '1@2000-01-01');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '1@2000-01-01');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '1@2000-01-01');
SELECT dynamicTimeWarp(tint '1@2000-01-01', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT dynamicTimeWarp(tint '1@2000-01-01', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT dynamicTimeWarp(tint '1@2000-01-01', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');

SELECT dynamicTimeWarp(tint '1@2000-01-01', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tint '1@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tint '1@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tint '1@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

SELECT dynamicTimeWarp(tint '1@2000-01-01', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tint '1@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tint '1@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tint '1@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT dynamicTimeWarp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT dynamicTimeWarp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT dynamicTimeWarp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

SELECT dynamicTimeWarp(tfloat '1.5@2000-01-01', tint '1@2000-01-01');
SELECT dynamicTimeWarp(tfloat '1.5@2000-01-01', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT dynamicTimeWarp(tfloat '1.5@2000-01-01', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT dynamicTimeWarp(tfloat '1.5@2000-01-01', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT dynamicTimeWarp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '1@2000-01-01');
SELECT dynamicTimeWarp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT dynamicTimeWarp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT dynamicTimeWarp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT dynamicTimeWarp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '1@2000-01-01');
SELECT dynamicTimeWarp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT dynamicTimeWarp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT dynamicTimeWarp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT dynamicTimeWarp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '1@2000-01-01');
SELECT dynamicTimeWarp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT dynamicTimeWarp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT dynamicTimeWarp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');

SELECT dynamicTimeWarp(tfloat '1.5@2000-01-01', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '1.5@2000-01-01');
SELECT dynamicTimeWarp(tfloat '1.5@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT dynamicTimeWarp(tfloat '1.5@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT dynamicTimeWarp(tfloat '1.5@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT dynamicTimeWarp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT dynamicTimeWarp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT dynamicTimeWarp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

-------------------------------------------------------------------------------
-- Dynamic Time Warp (DTW) path
-------------------------------------------------------------------------------

WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '1@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '1.5@2000-01-01', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '1.5@2000-01-01', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '1.5@2000-01-01', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '1.5@2000-01-01', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '1@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;

WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '1.5@2000-01-01', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '1.5@2000-01-01') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '1.5@2000-01-01', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '1.5@2000-01-01', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '1.5@2000-01-01', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;
WITH Temp AS (
  SELECT dynamicTimeWarpPath(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}') )
SELECT COUNT(*) FROM Temp;

-------------------------------------------------------------------------------
