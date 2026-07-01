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

-- Round-trip temporal point clouds through the Arrow C Data Interface.
-- pgPointCloud's pcpoint_in/pcpatch_in accept only hex-WKB, so the sample
-- values are built inline from the ergonomic constructors (pcid 1 is
-- registered as a 3D schema in the test fixture). The result must equal the
-- input for every subtype.

\set inst1 'tpcpoint(pcpoint(1, 1.0, 1.0, 1.0), ''2024-01-01''::timestamptz)'
\set inst2 'tpcpoint(pcpoint(1, 2.0, 2.0, 2.0), ''2024-01-02''::timestamptz)'
\set inst3 'tpcpoint(pcpoint(1, 3.0, 3.0, 3.0), ''2024-01-03''::timestamptz)'

\set patch1 'PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])])'
\set patch2 'PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[5.0, 5.0, 5.0]::float[]), PC_MakePoint(1, ARRAY[6.0, 6.0, 6.0]::float[])])'
\set pinst1 'tpcpatch(:patch1, ''2024-01-01''::timestamptz)'
\set pinst2 'tpcpatch(:patch2, ''2024-01-02''::timestamptz)'

-- tpcpoint
SELECT arrowRoundtrip(:inst1) = :inst1;
SELECT arrowRoundtrip(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3])) = tpcpointSeq(ARRAY[:inst1, :inst2, :inst3]);
SELECT arrowRoundtrip(tpcpointSeq(ARRAY[:inst1, :inst2, :inst3], 'discrete')) = tpcpointSeq(ARRAY[:inst1, :inst2, :inst3], 'discrete');

-- tpcpatch
SELECT arrowRoundtrip(:pinst1) = :pinst1;
SELECT arrowRoundtrip(tpcpatchSeq(ARRAY[:pinst1, :pinst2])) = tpcpatchSeq(ARRAY[:pinst1, :pinst2]);
SELECT arrowRoundtrip(tpcpatchSeq(ARRAY[:pinst1, :pinst2], 'discrete')) = tpcpatchSeq(ARRAY[:pinst1, :pinst2], 'discrete');

-------------------------------------------------------------------------------
