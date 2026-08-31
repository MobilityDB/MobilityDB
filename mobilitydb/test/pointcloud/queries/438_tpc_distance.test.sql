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

-- Value-level tests for the |=| (nearest-approach distance) operator.

\set p1 'tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), ''2024-01-01''::timestamptz)'
\set p2 'tpcpoint(PC_MakePoint(1, ARRAY[3.0, 4.0, 0.0]::float[]), ''2024-01-01''::timestamptz)'
\set box_at_origin 'tpcbox_zt(0, 0, 0, 0, 0, 0, tstzspan ''[2024-01-01, 2024-01-02]'', 1, 0)'
\set box_far       'tpcbox_zt(3, 4, 0, 3, 4, 0, tstzspan ''[2024-01-01, 2024-01-02]'', 1, 0)'

-- Self-distance is zero.
SELECT (:p1) |=| (:p1);

-- 3-4-5 right triangle in the xy plane.
SELECT (:p1) |=| (:p2);
SELECT (:p2) |=| (:p1);

-- Box-to-box at the same coords.
SELECT (:box_at_origin) |=| (:box_at_origin);

-- Distance from a point to a box at the same location is zero.
SELECT (:p1) |=| (:box_at_origin);

-- Disjoint time spans yield NULL, as the temporal geo nearest approach does.
SELECT (tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]),
                 '2024-01-01'::timestamptz)) |=|
       (tpcbox_zt(0, 0, 0, 0, 0, 0,
                  tstzspan '[2099-01-01, 2099-01-02]', 1, 0)) IS NULL;

-- Pcid mismatch is an error: values of two schemas cannot be compared.
SELECT (:p1) |=| (tpcbox_zt(0, 0, 0, 0, 0, 0,
  tstzspan '[2024-01-01, 2024-01-02]', 999, 0)) > 1e10;

-- The same holds between two temporal pointcloud values. A second schema is
-- registered as a copy of the first one, as the typmod test does.
INSERT INTO pointcloud_formats (pcid, srid, schema)
SELECT 2, srid, schema FROM pointcloud_formats WHERE pcid = 1;

SELECT (:p1) |=| tpcpoint(PC_MakePoint(2, ARRAY[1.0, 1.0, 1.0]::float[]),
  '2024-01-01'::timestamptz);

DELETE FROM pointcloud_formats WHERE pcid = 2;

-------------------------------------------------------------------------------
-- Temporal-temporal nearest-approach distance is the minimum of the
-- SYNCHRONIZED distance between the two values, not the distance between
-- their bounding boxes: the box-corner value is only reachable when the
-- two values are simultaneously at their extreme positions, which need
-- not happen.
-------------------------------------------------------------------------------

\set d1 '''2024-01-01 00:00:00''::timestamptz'
\set d2 '''2024-01-01 00:30:00''::timestamptz'
\set d3 '''2024-01-02 00:00:00''::timestamptz'
\set s1 'tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), :d1), tpcpoint(PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[]), :d2), tpcpoint(PC_MakePoint(1, ARRAY[3.0, 3.0, 3.0]::float[]), :d3)])'
\set s2 'tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[10.0, 10.0, 10.0]::float[]), :d1), tpcpoint(PC_MakePoint(1, ARRAY[12.0, 12.0, 12.0]::float[]), :d3)])'

-- Synchronized minimum, symmetric in both argument orders. It is strictly
-- greater than the bounding-box distance between [1,3] and [10,12] on
-- each axis (7 * sqrt(3) ~ 12.1244): the values never simultaneously
-- reach their closest box corners.
SELECT round((:s1 |=| :s2)::numeric, 4);
SELECT round((:s2 |=| :s1)::numeric, 4);
SELECT (:s1 |=| :s2) > 12.1244;

-- Nearest-approach distance of a value with itself is zero.
SELECT (:s1) |=| (:s1);

-- Consistency with eDwithin on both sides of the synchronized minimum.
SELECT eDwithin(:s1, :s2, 13);
SELECT eDwithin(:s1, :s2, 14);

-------------------------------------------------------------------------------
