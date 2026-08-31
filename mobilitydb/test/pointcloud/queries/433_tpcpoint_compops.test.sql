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

-- Ever/always and temporal comparison operators for tpcpoint. Values are built
-- with the ergonomic pcpoint constructor because pgPointCloud's pcpoint_in
-- accepts hex-WKB only, so a literal is not readable here. Equality on a
-- pcpoint is byte equality of the serialized point, so the result is
-- deterministic. Points: A = (1,1,1), B = (2,2,2).

\set pA 'pcpoint(1, 1.0, 1.0, 1.0)'
\set pB 'pcpoint(1, 2.0, 2.0, 2.0)'
\set iA 'tpcpoint(:pA, ''2001-01-01''::timestamptz)'
\set iB 'tpcpoint(:pB, ''2001-01-02''::timestamptz)'
\set seqAB 'tpcpointSeq(ARRAY[:iA, :iB])'
\set seqAA 'tpcpointSeq(ARRAY[:iA, tpcpoint(:pA, ''2001-01-02''::timestamptz)])'

-------------------------------------------------------------------------------
-- eEq / ?=
-------------------------------------------------------------------------------

SELECT (:pA) ?= (:iA);
SELECT (:iA) ?= (:pA);
SELECT (:iA) ?= (:iA);
-- mid-trajectory match (ever equal)
SELECT (:seqAB) ?= (:pB);
-- no match
SELECT (:iA) ?= (:pB);

-------------------------------------------------------------------------------
-- aEq / %=
-------------------------------------------------------------------------------

SELECT (:iA) %= (:pA);
-- mixed trajectory, not always equal
SELECT (:seqAB) %= (:pA);
-- constant trajectory, always equal
SELECT (:seqAA) %= (:pA);
SELECT (:seqAA) %= (:seqAA);

-------------------------------------------------------------------------------
-- eNe / ?<> and aNe / %<>
-------------------------------------------------------------------------------

SELECT (:pA) ?<> (:seqAB);
SELECT (:seqAB) ?<> (:pA);
SELECT (:iA) %<> (:pB);
SELECT (:seqAB) ?<> (:seqAA);
-- a value cannot be both ever equal and always different
SELECT NOT ((:iA) ?= (:pA) AND (:iA) %<> (:pA));

-------------------------------------------------------------------------------
-- tEq / #= and tNe / #<>
-------------------------------------------------------------------------------

SELECT asText((:pA) #= (:iA));
SELECT asText((:iA) #= (:pA));
SELECT asText((:seqAB) #= (:pA));
SELECT asText((:iA) #<> (:pB));
SELECT asText((:seqAB) #<> (:seqAA));

-------------------------------------------------------------------------------
