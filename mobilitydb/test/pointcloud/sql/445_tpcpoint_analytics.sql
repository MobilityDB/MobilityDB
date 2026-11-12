-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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
-- Simplification functions
--
-- Simplification applies to linear interpolation, and a temporal point cloud
-- point has step or discrete interpolation, so each function returns a copy
-- of the value it is given.
-------------------------------------------------------------------------------

\set seq 'tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 0.0]::float[]), ''2001-01-01''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[2.0, 2.0, 0.0]::float[]), ''2001-01-02''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[3.0, 1.0, 0.0]::float[]), ''2001-01-03''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[4.0, 4.0, 0.0]::float[]), ''2001-01-04''::timestamptz)])'
\set disc 'tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 0.0]::float[]), ''2001-01-01''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[2.0, 2.0, 0.0]::float[]), ''2001-01-02''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[3.0, 1.0, 0.0]::float[]), ''2001-01-03''::timestamptz)], ''discrete'')'
\set ss 'tpcpointSeqSet(ARRAY[tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), ''2001-01-01''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[1.0, 0.0, 0.0]::float[]), ''2001-01-02''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[4.0, 0.0, 0.0]::float[]), ''2001-01-03''::timestamptz)]), tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[9.0, 0.0, 0.0]::float[]), ''2001-01-05''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[20.0, 0.0, 0.0]::float[]), ''2001-01-06''::timestamptz)])])'

SELECT minDistSimplify(NULL::tpcpoint, 1.0);
SELECT minTimeDeltaSimplify(NULL::tpcpoint, interval '1 day');

SELECT interp(minDistSimplify(:seq, 2.0));
SELECT minDistSimplify(:seq, 2.0) = :seq;
SELECT minTimeDeltaSimplify(:seq, interval '2 days') = :seq;
SELECT maxDistSimplify(:seq, 1.0) = :seq;
SELECT maxDistSimplify(:seq, 1.0, false) = :seq;
SELECT douglasPeuckerSimplify(:seq, 1.0) = :seq;
SELECT douglasPeuckerSimplify(:seq, 1.0, false) = :seq;

SELECT interp(minDistSimplify(:disc, 2.0));
SELECT minDistSimplify(:disc, 2.0) = :disc;

SELECT numSequences(minDistSimplify(:ss, 2.0));
SELECT minDistSimplify(:ss, 2.0) = :ss;

-- A negative distance is refused
SELECT minDistSimplify(:seq, -1.0);

-------------------------------------------------------------------------------
