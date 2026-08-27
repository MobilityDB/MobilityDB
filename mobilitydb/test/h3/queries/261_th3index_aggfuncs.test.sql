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

-- extent(th3index): mixed temporal subtypes folded together, NULLs filtered
-- round to 10 decimal places to suppress platform floating-point ULP differences
SELECT round(extent(temp), 6) FROM ( VALUES
  (NULL::th3index),
  (th3index '831c02fffffffff@2001-01-01'),
  (th3index '{831c00fffffffff@2001-01-01, 831c02fffffffff@2001-01-02}')) t(temp);
SELECT round(extent(temp), 6) FROM ( VALUES
  (th3index '831c02fffffffff@2001-01-01'),
  (th3index '{831c00fffffffff@2001-01-01, 831c02fffffffff@2001-01-02}'),
  (NULL)) t(temp);

-- extent(th3index): linear sequence + step sequenceset over disjoint footprints
SELECT round(extent(temp), 6) FROM ( VALUES
  (th3index '[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02]'),
  ('{[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02], [871fa44a8ffffff@2001-01-03, 880326b885fffff@2001-01-04]}')) t(temp);

-- extent(th3index): single row degenerate cases
SELECT round(extent(temp), 6) FROM ( VALUES (th3index '831c02fffffffff@2001-01-01')) t(temp);
SELECT round(extent(temp), 6) FROM ( VALUES (NULL::th3index)) t(temp);

-------------------------------------------------------------------------------

WITH Temp(temp) AS (
  SELECT th3index '[831c02fffffffff@2001-01-01, 831c02fffffffff@2001-01-03]' UNION
  SELECT th3index '[831c00fffffffff@2001-01-02, 831c00fffffffff@2001-01-04]' )
SELECT tCount(Temp) FROM Temp;

WITH Temp(temp) AS (
  SELECT th3index '[831c02fffffffff@2001-01-01, 831c02fffffffff@2001-01-03]' UNION
  SELECT th3index '[831c00fffffffff@2001-01-02, 831c00fffffffff@2001-01-04]' )
SELECT wCount(Temp, interval '2 days') FROM Temp;

-------------------------------------------------------------------------------

SELECT merge(temp) FROM (VALUES
  (th3index '831c02fffffffff@2001-01-01'),
  (th3index '831c00fffffffff@2001-01-02')) t(temp);
SELECT mergeAgg(temp) FROM (VALUES
  (th3index '831c02fffffffff@2001-01-01'),
  (th3index '831c00fffffffff@2001-01-02')) t(temp);

-------------------------------------------------------------------------------

WITH temp(inst) AS (
  SELECT th3index '831c02fffffffff@2001-01-01' UNION
  SELECT th3index '831c00fffffffff@2001-01-02' UNION
  SELECT th3index '871fa44a8ffffff@2001-01-03' UNION
  SELECT th3index '880326b885fffff@2001-01-04' )
SELECT asText(appendInstant(inst ORDER BY inst)) FROM temp;

WITH temp(inst) AS (
  SELECT th3index '831c02fffffffff@2001-01-01' UNION
  SELECT th3index '831c00fffffffff@2001-01-02' UNION
  SELECT th3index '871fa44a8ffffff@2001-01-03' UNION
  SELECT th3index '880326b885fffff@2001-01-04' )
SELECT asText(appendInstantAgg(inst ORDER BY inst)) FROM temp;

WITH temp(inst) AS (
  SELECT th3index '831c02fffffffff@2001-01-01' UNION
  SELECT th3index '831c02fffffffff@2001-01-01' UNION
  SELECT th3index '831c00fffffffff@2001-01-02' UNION
  SELECT th3index '871fa44a8ffffff@2001-01-03' )
SELECT asText(appendInstant(inst ORDER BY inst)) FROM temp;

WITH temp(inst) AS (
  SELECT th3index '831c02fffffffff@2001-01-01' UNION
  SELECT th3index '831c00fffffffff@2001-01-02' UNION
  SELECT th3index '871fa44a8ffffff@2001-01-03' )
SELECT asText(appendInstant(inst ORDER BY inst, 'step')) FROM temp;

WITH temp(inst) AS (
  SELECT th3index '831c02fffffffff@2001-01-01' UNION
  SELECT th3index '831c00fffffffff@2001-01-02' UNION
  SELECT th3index '871fa44a8ffffff@2001-01-04' UNION
  SELECT th3index '880326b885fffff@2001-01-08' )
SELECT asText(appendInstant(inst, 'step', interval '1 day' ORDER BY inst)) FROM temp;

/* Errors */
WITH temp(inst) AS (
  SELECT th3index '831c02fffffffff@2001-01-01' UNION
  SELECT th3index '831c00fffffffff@2001-01-01' )
SELECT asText(appendInstant(inst ORDER BY inst)) FROM temp;

-------------------------------------------------------------------------------

WITH temp1(k, inst) AS (
  SELECT 1, th3index '831c02fffffffff@2001-01-01' UNION
  SELECT 2, th3index '831c00fffffffff@2001-01-02' UNION
  SELECT 3, th3index '871fa44a8ffffff@2001-01-03' UNION
  SELECT 4, th3index '880326b885fffff@2001-01-04' UNION
  SELECT 5, th3index '880326b88dfffff@2001-01-05' UNION
  SELECT 6, th3index '8a2a100d645ffff@2001-01-06' ),
temp2(k, seq) AS (
  SELECT (k - 1) / 2, appendInstant(inst ORDER BY inst)
  FROM temp1
  GROUP BY (k - 1) / 2)
SELECT asText(appendSequence(seq ORDER BY seq)) FROM temp2;

WITH temp1(k, inst) AS (
  SELECT 1, th3index '831c02fffffffff@2001-01-01' UNION
  SELECT 2, th3index '831c00fffffffff@2001-01-02' UNION
  SELECT 3, th3index '871fa44a8ffffff@2001-01-03' UNION
  SELECT 4, th3index '880326b885fffff@2001-01-04' UNION
  SELECT 5, th3index '880326b88dfffff@2001-01-05' UNION
  SELECT 6, th3index '8a2a100d645ffff@2001-01-06' ),
temp2(k, seq) AS (
  SELECT (k - 1) / 2, appendInstant(inst ORDER BY inst)
  FROM temp1
  GROUP BY (k - 1) / 2)
SELECT asText(appendSequenceAgg(seq ORDER BY seq)) FROM temp2;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The bare aggregate and its Agg twin in ONE query
--
-- The two carry identical transition machinery, so PostgreSQL shares one
-- transition state between them unless each declares FINALFUNC_MODIFY. Their
-- finalfn frees the state, so a shared one is finalized twice. Asking for both
-- at once is what exercises that, and the two must agree.
-------------------------------------------------------------------------------

SELECT merge(temp) = mergeAgg(temp) FROM (VALUES
  (th3index '831c02fffffffff@2001-01-01'),
  (th3index '831c00fffffffff@2001-01-02')) t(temp);

-------------------------------------------------------------------------------
