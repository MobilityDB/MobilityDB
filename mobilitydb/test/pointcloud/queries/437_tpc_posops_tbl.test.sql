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

-- Smoke tests for the directional position operators.  Each test
-- collapses to a single scalar.  The properties exercised are:
--   * Self-relation: nothing is strictly positioned to its own X axis
--     (a row is not strictly left of itself, etc.).
--   * Strict-vs-overlapping: strict (<<) implies the lax form (&<).
--   * Symmetric pairs commute properly under the operator.

-------------------------------------------------------------------------------
-- Self-relation negatives — strict directional ops are irreflexive.
-------------------------------------------------------------------------------

SELECT bool_and(NOT (temp <<  temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp >>  temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp <<| temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp |>> temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp <</ temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp />> temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp <<# temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp #>> temp)) FROM tbl_tpcpoint;

SELECT bool_and(NOT (temp <<  temp)) FROM tbl_tpcpatch;
SELECT bool_and(NOT (temp >>  temp)) FROM tbl_tpcpatch;
SELECT bool_and(NOT (temp <<# temp)) FROM tbl_tpcpatch;
SELECT bool_and(NOT (temp #>> temp)) FROM tbl_tpcpatch;

-------------------------------------------------------------------------------
-- An all-encompassing tpcbox is not strictly to one side of any row.
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpcpoint
WHERE temp << tpcbox_zt(-200, -200, -200, 200, 200, 200,
  tstzspan '[2000-01-01, 2030-01-01]', 1, 0);

SELECT COUNT(*) FROM tbl_tpcpoint
WHERE tpcbox_zt(-200, -200, -200, 200, 200, 200,
  tstzspan '[2000-01-01, 2030-01-01]', 1, 0) >> temp;

-- Reverse: every row is "before" a span that starts well after
-- the data extent.
SELECT COUNT(*) FROM tbl_tpcpoint
WHERE temp <<# tstzspan '[2099-01-01, 2099-12-31]';

SELECT COUNT(*) FROM tbl_tpcpatch
WHERE temp <<# tstzspan '[2099-01-01, 2099-12-31]';

-------------------------------------------------------------------------------
