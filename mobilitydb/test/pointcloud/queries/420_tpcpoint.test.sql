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

-- Value-level tests for tpcpoint — constructors, accessors, casts,
-- comparisons. Uses sample values built inline from PC_MakePoint
-- because pgPointCloud's pcpoint_in only accepts hex-WKB (the text
-- form is "not yet implemented" upstream), making constructor-based
-- expressions the readable way to write literal-value tests.

\set inst1 'tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), ''2024-01-01''::timestamptz)'
\set inst2 'tpcpoint(PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[]), ''2024-01-02''::timestamptz)'
\set inst3 'tpcpoint(PC_MakePoint(1, ARRAY[3.0, 3.0, 3.0]::float[]), ''2024-01-03''::timestamptz)'

-------------------------------------------------------------------------------
-- Ergonomic pcpoint constructors — same value as the verbose form.
-------------------------------------------------------------------------------

-- 3D form (pcid 1 is registered as a 3D schema in the test fixture).
SELECT pcpoint(1, 1.0, 1.0, 1.0)::text =
  PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[])::text;
-- inline use inside tpcpoint
SELECT tpcpoint(pcpoint(1, 1.0, 1.0, 1.0), '2024-01-01'::timestamptz)::text =
  (:inst1)::text;

-------------------------------------------------------------------------------
-- pcid + subtype size
-------------------------------------------------------------------------------

SELECT pcid(:inst1);
SELECT numInstants(:inst1);
SELECT numInstants(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT numInstants(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3], 'discrete'));

-------------------------------------------------------------------------------
-- Per-dimension projection (X / Y / Z)
-------------------------------------------------------------------------------

SELECT startValue(getX(:inst1));
SELECT startValue(getY(:inst1));
SELECT startValue(getZ(:inst1));

-------------------------------------------------------------------------------
-- Time accessors on a sequence
-------------------------------------------------------------------------------

SELECT startTimestamp(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT endTimestamp(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));
SELECT pcid(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]));

-------------------------------------------------------------------------------
-- Cast to tgeompoint preserves XYZ + timestamps
-------------------------------------------------------------------------------

SELECT numInstants((:inst1)::tgeompoint);
SELECT numInstants((tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]))::tgeompoint);

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT (:inst1) = (:inst1);
SELECT (:inst1) = (:inst2);
SELECT (:inst1) <> (:inst2);
SELECT (:inst1) < (:inst2);

-------------------------------------------------------------------------------
-- Restrictions
-------------------------------------------------------------------------------

SELECT atTime(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]),
  tstzspan '[2024-01-02, 2024-01-03]') IS NOT NULL;
SELECT atTpcbox(:inst2, tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)) IS NOT NULL;
SELECT minusTpcbox(:inst2, tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-31]', 1, 0)) IS NULL;

-- Pcid-mismatch identity: at → NULL, minus → unchanged.
SELECT atTpcbox(:inst1, tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-31]', 999, 0)) IS NULL;

-------------------------------------------------------------------------------
