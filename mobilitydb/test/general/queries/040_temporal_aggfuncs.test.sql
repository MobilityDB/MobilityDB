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

SELECT tand(temp) FROM (VALUES
(NULL::tbool),(NULL::tbool)) t(temp);
SELECT tand(temp) FROM (VALUES
(NULL::tbool),('true@2000-01-01'::tbool)) t(temp);

SELECT tor(temp) FROM (VALUES
(NULL::tbool),(NULL::tbool)) t(temp);
SELECT tor(temp) FROM (VALUES
(NULL::tbool),('true@2000-01-01'::tbool)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::tbool),(NULL::tbool)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::tbool),('true@2000-01-01'::tbool)) t(temp);

-------------------------------------------------------------------------------

SELECT tmin(temp) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT tmin(temp) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT tmax(temp) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT tmax(temp) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT tsum(temp) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT tsum(temp) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT tavg(temp) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT tavg(temp) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

-------------------------------------------------------------------------------

SELECT tmin(temp) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT tmin(temp) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT tmax(temp) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT tmax(temp) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT tsum(temp) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT tsum(temp) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT tavg(temp) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT tavg(temp) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

-------------------------------------------------------------------------------

SELECT tmin(temp) FROM (VALUES
(NULL::ttext),(NULL::ttext)) t(temp);
SELECT tmin(temp) FROM (VALUES
(NULL::ttext),('AAAA@2000-01-01'::ttext)) t(temp);

SELECT tmax(temp) FROM (VALUES
(NULL::ttext),(NULL::ttext)) t(temp);
SELECT tmax(temp) FROM (VALUES
(NULL::ttext),('AAAA@2000-01-01'::ttext)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::ttext),(NULL::ttext)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::ttext),('AAAA@2000-01-01'::ttext)) t(temp);

-------------------------------------------------------------------------------

SELECT extent(temp) FROM (VALUES
('[true@2000-01-01, false@2000-01-03, true@2000-01-05, false@2000-01-07]'::tbool),
('[true@2000-01-02, false@2000-01-06]'::tbool)) t(temp);

SELECT tcount(temp) FROM (VALUES
('[true@2000-01-01, false@2000-01-03, true@2000-01-05, false@2000-01-07]'::tbool),
('[true@2000-01-02, false@2000-01-06]'::tbool)) t(temp);

SELECT tand(temp) FROM (VALUES
('[true@2000-01-01, false@2000-01-03, true@2000-01-05, false@2000-01-07]'::tbool),
('[true@2000-01-02, false@2000-01-06]'::tbool)) t(temp);

SELECT tor(temp) FROM (VALUES
('[true@2000-01-01, false@2000-01-03, true@2000-01-05, false@2000-01-07]'::tbool),
('[true@2000-01-02, false@2000-01-06]'::tbool)) t(temp);

-------------------------------------------------------------------------------

SELECT extent(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint),
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

SELECT tcount(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint),
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

SELECT tmin(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint),
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

SELECT tmax(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint),
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

SELECT tsum(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint),
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

SELECT tavg(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint),
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

-------------------------------------------------------------------------------

SELECT extent(temp) FROM (VALUES
('Interp=Step;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('Interp=Step;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tcount(temp) FROM (VALUES
('Interp=Step;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('Interp=Step;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tmin(temp) FROM (VALUES
('Interp=Step;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('Interp=Step;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tmax(temp) FROM (VALUES
('Interp=Step;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('Interp=Step;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tsum(temp) FROM (VALUES
('Interp=Step;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('Interp=Step;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tavg(temp) FROM (VALUES
('Interp=Step;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('Interp=Step;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

-------------------------------------------------------------------------------

SELECT extent(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tcount(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT round(tmin(temp), 6) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT round(tmax(temp), 6) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tsum(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT round(tavg(temp), 6) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

-------------------------------------------------------------------------------

/* Errors */
SELECT tsum(temp) FROM ( VALUES
(tfloat '[1@2000-01-01, 2@2000-01-02]'),
(tfloat '{3@2000-01-03, 4@2000-01-04}')) t(temp);
SELECT tsum(temp) FROM ( VALUES
(tfloat '{3@2000-01-03, 4@2000-01-04}'),
(tfloat '[1@2000-01-01, 2@2000-01-02]')) t(temp);
SELECT tsum(temp) FROM ( VALUES
(tfloat '{1@2000-01-01, 2@2000-01-02}'),
(tfloat '[3@2000-01-03, 4@2000-01-04]')) t(temp);
SELECT tsum(temp) FROM (VALUES
('Interp=Step;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat),
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);
SELECT tsum(temp) FROM (VALUES
('{1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07}'::tfloat),
('{[3@2000-01-02, 4@2000-01-06]}'::tfloat)) t(temp);
SELECT tsum(temp) FROM (VALUES
('Interp=Step;{[1@2000-01-01, 2@2000-01-03], [1@2000-01-05, 2@2000-01-07]}'::tfloat),
('{[3@2000-01-02, 4@2000-01-06]}'::tfloat)) t(temp);

-------------------------------------------------------------------------------

WITH temp(inst) AS (
  SELECT tint '1@2000-01-01' UNION
  SELECT tint '2@2000-01-02' UNION
  SELECT tint '3@2000-01-03' UNION
  SELECT tint '4@2000-01-04' UNION
  SELECT tint '5@2000-01-05' )
SELECT appendInstant(inst ORDER BY inst) FROM temp;

WITH temp(inst) AS (
  SELECT tint '1@2000-01-01' UNION
  SELECT tint '1@2000-01-01' UNION
  SELECT tint '2@2000-01-02' UNION
  SELECT tint '2@2000-01-02' UNION
  SELECT tint '3@2000-01-03' UNION
  SELECT tint '4@2000-01-04' UNION
  SELECT tint '5@2000-01-05' )
SELECT appendInstant(inst ORDER BY inst) FROM temp;

WITH temp(inst) AS (
  SELECT NULL UNION
  SELECT tfloat '1@2000-01-01' UNION
  SELECT tfloat '2@2000-01-02' UNION
  SELECT tfloat '3@2000-01-03' UNION
  SELECT tfloat '4@2000-01-04' UNION
  SELECT tfloat '5@2000-01-05' )
SELECT appendInstant(inst ORDER BY inst) FROM temp;

WITH temp(inst) AS (
  SELECT ttext 'AA@2000-01-01' UNION
  SELECT ttext 'BB@2000-01-02' UNION
  SELECT ttext 'CC@2000-01-03' UNION
  SELECT ttext 'DD@2000-01-04' UNION
  SELECT ttext 'EE@2000-01-05' )
SELECT appendInstant(inst ORDER BY inst) FROM temp;

WITH temp(inst) AS (
  SELECT tint_inst(extract(day from d)::int % 2, d)
  FROM generate_series(timestamptz '1900-01-01', '2000-01-10', interval '1 day') AS d )
SELECT numInstants(appendInstant(inst ORDER BY inst)) FROM temp;

WITH temp(inst) AS (
  SELECT tint_seq(tint_inst(extract(day from d)::int % 2, d))
  FROM generate_series(timestamptz '1900-01-01', '2000-01-10', interval '1 day') AS d )
SELECT numInstants(appendSequence(inst ORDER BY inst)) FROM temp;

/* Errors */
WITH temp(inst) AS (
  SELECT tint '1@2000-01-01' UNION
  SELECT tint '2@2000-01-01' UNION
  SELECT tint '2@2000-01-02' UNION
  SELECT tint '2@2000-01-02' UNION
  SELECT tint '3@2000-01-03' UNION
  SELECT tint '4@2000-01-04' UNION
  SELECT tint '5@2000-01-05' )
SELECT appendInstant(inst ORDER BY inst) FROM temp;

-------------------------------------------------------------------------------

WITH temp(inst) AS (
  SELECT tint '1@2000-01-01' UNION
  SELECT tint '2@2000-01-02' UNION
  SELECT tint '4@2000-01-04' UNION
  SELECT tint '5@2000-01-05' UNION
  SELECT tint '7@2000-01-07' )
SELECT appendInstant(inst, 1, NULL ORDER BY inst) FROM temp;

WITH temp(inst) AS (
  SELECT tint '1@2000-01-01' UNION
  SELECT tint '2@2000-01-02' UNION
  SELECT tint '4@2000-01-04' UNION
  SELECT tint '5@2000-01-05' UNION
  SELECT tint '7@2000-01-07' )
SELECT appendInstant(inst, NULL, interval '1 day' ORDER BY inst) FROM temp;

WITH temp(inst) AS (
  SELECT tfloat '1@2000-01-01' UNION
  SELECT tfloat '2@2000-01-02' UNION
  SELECT tfloat '4@2000-01-04' UNION
  SELECT tfloat '5@2000-01-05' UNION
  SELECT tfloat '7@2000-01-07' )
SELECT appendInstant(inst, 1, NULL ORDER BY inst) FROM temp;

WITH temp(inst) AS (
  SELECT tfloat '1@2000-01-01' UNION
  SELECT tfloat '2@2000-01-02' UNION
  SELECT tfloat '4@2000-01-04' UNION
  SELECT tfloat '5@2000-01-05' UNION
  SELECT tfloat '7@2000-01-07' )
SELECT appendInstant(inst, NULL, interval '1 day' ORDER BY inst) FROM temp;

WITH temp(inst) AS (
  SELECT ttext 'AA@2000-01-01' UNION
  SELECT ttext 'BB@2000-01-02' UNION
  SELECT ttext 'CC@2000-01-04' UNION
  SELECT ttext 'DD@2000-01-05' UNION
  SELECT ttext 'EE@2000-01-07' )
SELECT appendInstant(inst, interval '1 day' ORDER BY inst) FROM temp;

-------------------------------------------------------------------------------

WITH temp(k, seq) AS (
  SELECT 1, tint '[1@2000-01-01, 2@2000-01-02]' UNION
  SELECT 2, tint '[2@2000-01-02, 3@2000-01-03]' UNION
  SELECT 3, tint '[3@2000-01-03, 4@2000-01-04]' UNION
  SELECT 4, tint '[4@2000-01-04, 5@2000-01-05]' UNION
  SELECT 5, tint '[5@2000-01-05, 6@2000-01-06]' )
SELECT appendSequence(seq ORDER BY k) FROM temp;

WITH temp(k, seq) AS (
  SELECT 1, tint '[1@2000-01-01, 2@2000-01-02]' UNION
  SELECT 2, tint '[3@2000-01-03, 4@2000-01-04]' UNION
  SELECT 3, tint '[5@2000-01-05, 6@2000-01-06]' )
SELECT appendSequence(seq ORDER BY k) FROM temp;

WITH temp1(k, inst) AS (
  SELECT 1, tint '1@2000-01-01' UNION
  SELECT 2, tint '2@2000-01-02' UNION
  SELECT 3, tint '3@2000-01-03' UNION
  SELECT 4, tint '4@2000-01-04' UNION
  SELECT 5, tint '5@2000-01-05' UNION
  SELECT 6, tint '5@2000-01-06' UNION
  SELECT 7, tint '5@2000-01-07' UNION
  SELECT 8, tint '5@2000-01-08' ),
temp2(k, seq) AS (
  SELECT k / 3, appendInstant(inst ORDER BY inst)
  FROM temp1
  GROUP BY k / 3)
SELECT appendSequence(seq ORDER BY seq) FROM temp2;

WITH temp1(k, inst) AS (
  SELECT 1, tfloat '1@2000-01-01' UNION
  SELECT 2, tfloat '2@2000-01-02' UNION
  SELECT 3, tfloat '3@2000-01-03' UNION
  SELECT 4, tfloat '4@2000-01-04' UNION
  SELECT 5, tfloat '5@2000-01-05' UNION
  SELECT 6, tfloat '5@2000-01-06' UNION
  SELECT 7, tfloat '5@2000-01-07' UNION
  SELECT 8, tfloat '5@2000-01-08' ),
temp2(seq) AS (
  SELECT appendInstant(inst ORDER BY inst)
  FROM temp1
  GROUP BY k / 3)
SELECT appendSequence(seq ORDER BY seq) FROM temp2;

WITH temp(k, seq) AS (
  SELECT 1, ttext '[AA@2000-01-01, BB@2000-01-02]' UNION
  SELECT 2, ttext '[BB@2000-01-02, CC@2000-01-03]' UNION
  SELECT 3, ttext '[CC@2000-01-03, DD@2000-01-04]' UNION
  SELECT 4, ttext '[DD@2000-01-04, EE@2000-01-05]' UNION
  SELECT 5, ttext '[EE@2000-01-05, FF@2000-01-06]' )
SELECT appendSequence(seq ORDER BY k) FROM temp;

WITH temp(seq) AS (
  SELECT NULL UNION
  SELECT tfloat '[1@2000-01-01, 2@2000-01-02]' UNION
  SELECT tfloat '[3@2000-01-03, 4@2000-01-04]' )
SELECT appendSequence(seq ORDER BY seq) FROM temp;

WITH temp1(k, inst) AS (
  SELECT extract(day from d)::int % 2, tint_inst(extract(day from d)::int % 2, d)
  FROM generate_series(timestamptz '1900-01-01', '2000-01-10', interval '1 day') AS d ),
temp2(seq) AS (
  SELECT appendInstant(inst ORDER BY inst)
  FROM temp1
  GROUP BY k / 3)
SELECT numInstants(appendSequence(seq ORDER BY seq)) FROM temp2;

-------------------------------------------------------------------------------
