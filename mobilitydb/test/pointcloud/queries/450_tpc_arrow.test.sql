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

-- Round-trip a temporal point cloud value through the Arrow C Data Interface.
-- The value leaf is an opaque LargeBinary carrying each instant's serialized
-- pgPointCloud varlena body verbatim (the same canonical encoding used by the
-- temporal WKB); the leaf name "pcpoint"/"pcpatch" discriminates it from the
-- general-geometry LargeBinary leaf. The schema id is resolved out-of-band
-- against the pointcloud_formats catalog, exactly as for the WKB encoding.
-- The result must equal the input for every subtype. tpcpoint and tpcpatch
-- default to step interpolation, so sequences use closed bounds.

\set pt1 'PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[])'
\set pt2 'PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])'
\set pt3 'PC_MakePoint(1, ARRAY[3.0, 3.0, 3.0]::float[])'

\set tpi1 'tpcpoint(:pt1, ''2024-01-01''::timestamptz)'
\set tpi2 'tpcpoint(:pt2, ''2024-01-02''::timestamptz)'
\set tpi3 'tpcpoint(:pt3, ''2024-01-03''::timestamptz)'

-- tpcpoint: instant, discrete sequence, step sequence, sequence set
SELECT arrowRoundtrip(:tpi1) = (:tpi1);
SELECT arrowRoundtrip(tpcpointSeq(ARRAY[:tpi1, :tpi2, :tpi3], 'discrete')) =
  tpcpointSeq(ARRAY[:tpi1, :tpi2, :tpi3], 'discrete');
SELECT arrowRoundtrip(tpcpointSeq(ARRAY[:tpi1, :tpi2, :tpi3])) =
  tpcpointSeq(ARRAY[:tpi1, :tpi2, :tpi3]);
SELECT arrowRoundtrip(tpcpointSeqSet(ARRAY[
  tpcpointSeq(ARRAY[:tpi1, :tpi2]),
  tpcpointSeq(ARRAY[:tpi3])])) = tpcpointSeqSet(ARRAY[
  tpcpointSeq(ARRAY[:tpi1, :tpi2]),
  tpcpointSeq(ARRAY[:tpi3])]);

\set pa1 'PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])])'
\set pa2 'PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[5.0, 5.0, 5.0]::float[]), PC_MakePoint(1, ARRAY[6.0, 6.0, 6.0]::float[])])'

\set tqi1 'tpcpatch(:pa1, ''2024-01-01''::timestamptz)'
\set tqi2 'tpcpatch(:pa2, ''2024-01-02''::timestamptz)'
\set tqi3 'tpcpatch(:pa1, ''2024-01-03''::timestamptz)'

-- tpcpatch: instant, discrete sequence, step sequence, sequence set
SELECT arrowRoundtrip(:tqi1) = (:tqi1);
SELECT arrowRoundtrip(tpcpatchSeq(ARRAY[:tqi1, :tqi2, :tqi3], 'discrete')) =
  tpcpatchSeq(ARRAY[:tqi1, :tqi2, :tqi3], 'discrete');
SELECT arrowRoundtrip(tpcpatchSeq(ARRAY[:tqi1, :tqi2, :tqi3])) =
  tpcpatchSeq(ARRAY[:tqi1, :tqi2, :tqi3]);
SELECT arrowRoundtrip(tpcpatchSeqSet(ARRAY[
  tpcpatchSeq(ARRAY[:tqi1, :tqi2]),
  tpcpatchSeq(ARRAY[:tqi3])])) = tpcpatchSeqSet(ARRAY[
  tpcpatchSeq(ARRAY[:tqi1, :tqi2]),
  tpcpatchSeq(ARRAY[:tqi3])]);

-------------------------------------------------------------------------------
