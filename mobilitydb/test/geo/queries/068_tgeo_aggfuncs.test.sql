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

SELECT extent(temp) FROM (VALUES
(NULL::tgeometry),('Point(1 1)@2000-01-01'::tgeometry),(NULL::tgeometry)) t(temp);

SELECT extent(temp) FROM (VALUES
  (tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]'),
  (tgeometry '[Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]'),
  (tgeometry '[Point(2 2)@2000-01-02, Point(3 3)@2000-01-03]')) t(temp);
SELECT round(extent(temp), 13) FROM (VALUES
  (tgeography '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02]'),
  (tgeography '[Point(3 3 3)@2000-01-03, Point(4 4 4)@2000-01-04]'),
  (tgeography '[Point(2 2 2)@2000-01-02, Point(3 3 3)@2000-01-03]')) t(temp);

/* Errors */
SELECT extent(temp) FROM (VALUES
  (tgeometry 'Point(1 1 1)@2000-01-01'),
  (tgeometry 'Point(1 1)@2000-01-01')) t(temp);

-------------------------------------------------------------------------------

WITH temp(inst) AS (
  SELECT tgeometry 'Point(1 1)@2000-01-01' UNION
  SELECT tgeometry 'Point(2 2)@2000-01-02' UNION
  SELECT tgeometry 'Point(3 3)@2000-01-03' UNION
  SELECT tgeometry 'Point(4 4)@2000-01-04' UNION
  SELECT tgeometry 'Point(5 5)@2000-01-05'  )
SELECT asText(appendInstant(inst ORDER BY inst)) FROM temp;

WITH temp(inst) AS (
  SELECT tgeometry 'Point(1 1)@2000-01-01' UNION
  SELECT tgeometry 'Point(1 1)@2000-01-01' UNION
  SELECT tgeometry 'Point(2 2)@2000-01-02' UNION
  SELECT tgeometry 'Point(2 2)@2000-01-02' UNION
  SELECT tgeometry 'Point(3 3)@2000-01-03' UNION
  SELECT tgeometry 'Point(4 4)@2000-01-04' UNION
  SELECT tgeometry 'Point(5 5)@2000-01-05'  )
SELECT asText(appendInstant(inst ORDER BY inst)) FROM temp;

WITH temp(inst) AS (
  SELECT tgeography 'Point(1 1)@2000-01-01' UNION
  SELECT tgeography 'Point(2 2)@2000-01-02' UNION
  SELECT tgeography 'Point(3 3)@2000-01-03' UNION
  SELECT tgeography 'Point(4 4)@2000-01-04' UNION
  SELECT tgeography 'Point(5 5)@2000-01-05' )
SELECT asText(appendInstant(inst ORDER BY inst)) FROM temp;

WITH temp(inst) AS (
  SELECT tgeometry(ST_Point(extract(day from d)::int % 2, extract(day from d)::int % 2), d)
  FROM generate_series(timestamptz '1900-01-01', '2000-01-10', interval '1 day') AS d )
SELECT numInstants(appendInstant(inst ORDER BY inst)) FROM temp;

/* Errors */
WITH temp(inst) AS (
  SELECT tgeometry 'Point(1 1)@2000-01-01' UNION
  SELECT tgeometry 'Point(2 2)@2000-01-01' UNION
  SELECT tgeometry 'Point(2 2)@2000-01-02' UNION
  SELECT tgeometry 'Point(2 2)@2000-01-02' UNION
  SELECT tgeometry 'Point(3 3)@2000-01-03' UNION
  SELECT tgeometry 'Point(4 4)@2000-01-04' UNION
  SELECT tgeometry 'Point(5 5)@2000-01-05'  )
SELECT asText(appendInstant(inst ORDER BY inst)) FROM temp;

-------------------------------------------------------------------------------

WITH temp(inst) AS (
  SELECT tgeometry 'Point(1 1)@2000-01-01' UNION
  SELECT tgeometry 'Point(2 2)@2000-01-02' UNION
  SELECT tgeometry 'Point(3 3)@2000-01-03' UNION
  SELECT tgeometry 'Point(4 4)@2000-01-04' UNION
  SELECT tgeometry 'Point(5 5)@2000-01-05' )
SELECT asText(appendInstant(inst ORDER BY inst, 'discrete')) FROM temp;

WITH temp(inst) AS (
  SELECT tgeometry 'Point(1 1)@2000-01-01' UNION
  SELECT tgeometry 'Point(1 1)@2000-01-01' UNION
  SELECT tgeometry 'Point(2 2)@2000-01-02' UNION
  SELECT tgeometry 'Point(2 2)@2000-01-02' UNION
  SELECT tgeometry 'Point(3 3)@2000-01-03' UNION
  SELECT tgeometry 'Point(4 4)@2000-01-04' UNION
  SELECT tgeometry 'Point(5 5)@2000-01-05' )
SELECT asText(appendInstant(inst ORDER BY inst, 'discrete')) FROM temp;

WITH temp(inst) AS (
  SELECT tgeography 'Point(1 1)@2000-01-01' UNION
  SELECT tgeography 'Point(2 2)@2000-01-02' UNION
  SELECT tgeography 'Point(3 3)@2000-01-03' UNION
  SELECT tgeography 'Point(4 4)@2000-01-04' UNION
  SELECT tgeography 'Point(5 5)@2000-01-05' )
SELECT asText(appendInstant(inst ORDER BY inst, 'discrete')) FROM temp;

WITH temp(inst) AS (
  SELECT tgeometry(ST_Point(extract(day from d)::int % 2, extract(day from d)::int % 2), d)
  FROM generate_series(timestamptz '1900-01-01', '2000-01-10', interval '1 day') AS d )
SELECT numInstants(appendInstant(inst ORDER BY inst, 'discrete')) FROM temp;

-------------------------------------------------------------------------------

WITH temp(inst) AS (
  SELECT tgeometry 'Point(1 1)@2000-01-01' UNION
  SELECT tgeometry 'Point(2 2)@2000-01-02' UNION
  SELECT tgeometry 'Point(4 4)@2000-01-04' UNION
  SELECT tgeometry 'Point(5 5)@2000-01-05' UNION
  SELECT tgeometry 'Point(7 7)@2000-01-07' )
SELECT asText(appendInstant(inst, NULL, sqrt(2), NULL ORDER BY inst)) FROM temp;

WITH temp(inst) AS (
  SELECT tgeometry 'Point(1 1 1)@2000-01-01' UNION
  SELECT tgeometry 'Point(2 2 2)@2000-01-02' UNION
  SELECT tgeometry 'Point(2 2 2)@2000-01-04' UNION
  SELECT tgeometry 'Point(5 5 5)@2000-01-05' UNION
  SELECT tgeometry 'Point(7 7 7)@2000-01-07' )
SELECT asText(appendInstant(inst, NULL, sqrt(3), '1 day' ORDER BY inst)) FROM temp;

-------------------------------------------------------------------------------

WITH temp1(k, inst) AS (
  SELECT 1, tgeometry 'Point(1 1)@2000-01-01' UNION
  SELECT 2, tgeometry 'Point(2 2)@2000-01-02' UNION
  SELECT 3, tgeometry 'Point(3 3)@2000-01-03' UNION
  SELECT 4, tgeometry 'Point(4 4)@2000-01-04' UNION
  SELECT 5, tgeometry 'Point(5 5)@2000-01-05' UNION
  SELECT 6, tgeometry 'Point(6 6)@2000-01-06' UNION
  SELECT 7, tgeometry 'Point(7 7)@2000-01-07' UNION
  SELECT 8, tgeometry 'Point(8 8)@2000-01-08'  ),
temp2(k, seq) AS (
  SELECT k / 3, appendInstant(inst ORDER BY inst)
  FROM temp1
  GROUP BY k / 3)
SELECT astext(appendSequence(seq ORDER BY seq)) FROM temp2;

WITH temp1(k, inst) AS (
  SELECT 1, tgeography 'Point(1 1)@2000-01-01' UNION
  SELECT 2, tgeography 'Point(2 2)@2000-01-02' UNION
  SELECT 3, tgeography 'Point(3 3)@2000-01-03' UNION
  SELECT 4, tgeography 'Point(4 4)@2000-01-04' UNION
  SELECT 5, tgeography 'Point(5 5)@2000-01-05' UNION
  SELECT 6, tgeography 'Point(6 6)@2000-01-06' UNION
  SELECT 7, tgeography 'Point(7 7)@2000-01-07' UNION
  SELECT 8, tgeography 'Point(8 8)@2000-01-08'  ),
temp2(seq) AS (
  SELECT appendInstant(inst ORDER BY inst)
  FROM temp1
  GROUP BY k / 3)
SELECT astext(appendSequence(seq ORDER BY seq)) FROM temp2;

WITH temp1(k, inst) AS (
  SELECT extract(day from d)::int % 2,
    tgeometry(ST_Point(extract(day from d)::int % 2,extract(day from d)::int % 2), d)
  FROM generate_series(timestamptz '1900-01-01', '2000-01-10', interval '1 day') AS d ),
temp2(seq) AS (
  SELECT appendInstant(inst ORDER BY inst)
  FROM temp1
  GROUP BY k / 3)
SELECT numInstants(appendSequence(seq ORDER BY seq)) FROM temp2;

-------------------------------------------------------------------------------
