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

-- Table-level smoke tests for the bbox topological operators on
-- tpcpoint / tpcpatch.  Each row should self-overlap, self-contain,
-- self-equal under ~=, and not be adjacent to itself; a query box
-- spanning the entire datagen extent should contain every row.

-------------------------------------------------------------------------------
-- Self-relations: every row's bbox is reflexive under &&, @>, <@, ~=.
-------------------------------------------------------------------------------

SELECT bool_and(temp && temp) FROM tbl_tpcpoint;
SELECT bool_and(temp @> temp) FROM tbl_tpcpoint;
SELECT bool_and(temp <@ temp) FROM tbl_tpcpoint;
SELECT bool_and(temp ~= temp) FROM tbl_tpcpoint;
SELECT bool_and(NOT (temp -|- temp)) FROM tbl_tpcpoint;

SELECT bool_and(temp && temp) FROM tbl_tpcpatch;
SELECT bool_and(temp @> temp) FROM tbl_tpcpatch;
SELECT bool_and(temp <@ temp) FROM tbl_tpcpatch;
SELECT bool_and(temp ~= temp) FROM tbl_tpcpatch;
SELECT bool_and(NOT (temp -|- temp)) FROM tbl_tpcpatch;

-------------------------------------------------------------------------------
-- All-encompassing query box should contain every row.
-------------------------------------------------------------------------------

SELECT bool_and(tpcbox_zt(-200, -200, -200, 200, 200, 200,
  tstzspan '[2000-01-01, 2030-01-01]', 1, 0) @> temp)
FROM tbl_tpcpoint;
SELECT bool_and(tpcbox_zt(-200, -200, -200, 200, 200, 200,
  tstzspan '[2000-01-01, 2030-01-01]', 1, 0) @> temp)
FROM tbl_tpcpatch;

-- Reverse direction: every row is contained in that big box.
SELECT bool_and(temp <@ tpcbox_zt(-200, -200, -200, 200, 200, 200,
  tstzspan '[2000-01-01, 2030-01-01]', 1, 0)) FROM tbl_tpcpoint;
SELECT bool_and(temp <@ tpcbox_zt(-200, -200, -200, 200, 200, 200,
  tstzspan '[2000-01-01, 2030-01-01]', 1, 0)) FROM tbl_tpcpatch;

-- Time-only: every row's tstzspan is contained in this wide span.
SELECT bool_and(temp <@ tstzspan '[2000-01-01, 2030-01-01]') FROM tbl_tpcpoint;
SELECT bool_and(temp <@ tstzspan '[2000-01-01, 2030-01-01]') FROM tbl_tpcpatch;

-------------------------------------------------------------------------------
