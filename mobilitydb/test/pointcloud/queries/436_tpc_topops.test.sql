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

-- Value-level tests for the topological operators (&&, @>, <@, ~=, -|-)
-- on tpcpoint / tpcpatch against tpcbox, tstzspan and self.

\set p1 'tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), ''2024-01-01''::timestamptz)'
\set p2 'tpcpoint(PC_MakePoint(1, ARRAY[5.0, 5.0, 5.0]::float[]), ''2024-01-05''::timestamptz)'
\set big_box 'tpcboxZT(0, 0, 0, 10, 10, 10, tstzspan ''[2024-01-01, 2024-01-31]'', 1)'
\set far_box 'tpcboxZT(100, 100, 100, 110, 110, 110, tstzspan ''[2099-01-01, 2099-12-31]'', 1)'
-- The box of a schema that is not registered is written rather than built: a
-- constructor reads the reference system off the schema its pcid names.
\set bad_box 'tpcbox ''TPCBOX(ZT(((0,0,0),(10,10,10)),[2024-01-01, 2024-01-31]), 999)'''
\set span_in 'tstzspan ''[2024-01-01, 2024-01-31]'''
\set span_far 'tstzspan ''[2099-01-01, 2099-12-31]'''

-------------------------------------------------------------------------------
-- Overlap (&&)
-------------------------------------------------------------------------------

SELECT (:p1) && (:big_box);
SELECT (:big_box) && (:p1);
SELECT (:p1) && (:far_box);
SELECT (:p1) && (:p1);
SELECT (:p1) && (:p2);
SELECT (:p1) && (:span_in);
SELECT (:p1) && (:span_far);
-- Pcid mismatch ⇒ false (raised as error in MEOS, but PG operator
-- catches and returns false-ish; assertion is "no crash").
-- We don't include the pcid-mismatch case here because the operator
-- raises, which is correct behaviour but produces a verbose ERROR
-- that would clutter the expected output. The _tbl test covers it.

-------------------------------------------------------------------------------
-- Contains / contained / same / adjacent
-------------------------------------------------------------------------------

SELECT (:big_box) @> (:p1);
SELECT (:p1) <@ (:big_box);
SELECT (:p1) ~= (:p1);
SELECT (:p1) -|- (:p1);
-- Note: tpcbox @> tstzspan / tstzspan <@ tpcbox are not wired up
-- (the tpcbox topological surface is tpcbox-vs-tpcbox only).

-------------------------------------------------------------------------------
-- Same suite for tpcpatch
-------------------------------------------------------------------------------

\set patch1 'PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])])'
\set q1 'tpcpatch(:patch1, ''2024-01-01''::timestamptz)'

SELECT (:q1) && (:big_box);
SELECT (:big_box) && (:q1);
SELECT (:q1) && (:far_box);
SELECT (:q1) && (:q1);
SELECT (:big_box) @> (:q1);
SELECT (:q1) <@ (:big_box);
SELECT (:q1) ~= (:q1);

-------------------------------------------------------------------------------
