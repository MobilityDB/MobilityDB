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
-- The aggregates of temporal rigid geometries. Every value aggregated carries
-- the same reference geometry, the square, which the answer carries too; a
-- value over another body is refused.
-------------------------------------------------------------------------------

-- merge
SELECT asText(merge(t)) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 1),0)@2001-01-01')) v(t);
SELECT asText(merge(t)) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(2 2),0.5)@2001-01-02'),
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 1),0)@2001-01-01'),
  (NULL::trgeometry)) v(t);
SELECT asText(merge(t)) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1),0)@2001-01-01, Pose(Point(2 2),0)@2001-01-02]'),
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));{[Pose(Point(3 3),0)@2001-01-03, Pose(Point(4 4),0)@2001-01-04],
    [Pose(Point(5 5),0)@2001-01-05]}')) v(t);
SELECT asText(merge(t)) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 1),0)@2001-01-01'),
  (trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));Pose(Point(2 2),0)@2001-01-02')) v(t);

-- appendInstant
SELECT asText(appendInstant(t)) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 1),0)@2001-01-01')) v(t);
SELECT asText(appendInstant(t ORDER BY getTime(t))) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(3 3),0)@2001-01-03'),
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 1),0)@2001-01-01'),
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(2 2),0.5)@2001-01-02')) v(t);
SELECT asText(appendInstant(t, 'step')) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 1),0)@2001-01-01'),
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(2 2),0.5)@2001-01-02')) v(t);
SELECT asText(appendInstant(t, 'linear', NULL, interval '1 day')) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 1),0)@2001-01-01'),
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(2 2),0)@2001-01-02'),
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(3 3),0)@2001-01-05')) v(t);
SELECT asText(appendInstant(t)) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 1),0)@2001-01-01'),
  (trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));Pose(Point(2 2),0)@2001-01-02')) v(t);

-- appendSequence
SELECT asText(appendSequence(t)) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1),0)@2001-01-01, Pose(Point(2 2),0)@2001-01-02]')) v(t);
SELECT asText(appendSequence(t)) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1),0)@2001-01-01, Pose(Point(2 2),0)@2001-01-02]'),
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(3 3),0)@2001-01-03, Pose(Point(4 4),0)@2001-01-04]')) v(t);
SELECT asText(appendSequence(t)) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(1 1),0)@2001-01-01, Pose(Point(2 2),0)@2001-01-02]'),
  (trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(3 3),0)@2001-01-03, Pose(Point(4 4),0)@2001-01-04]')) v(t);

-- The aggregates that answer another type
SELECT asText(tCount(t)), asText(wCount(t, interval '1 day')), extent(t) FROM (VALUES
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(1 1),0)@2001-01-01'),
  (trgeometry 'Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(2 2),0)@2001-01-02')) v(t);

-- The states of parallel workers combine into the merge of every value, which
-- carries the reference geometry whatever the plan; states over two bodies do
-- not combine
SET parallel_setup_cost = 0;
SET parallel_tuple_cost = 0;
SET min_parallel_table_scan_size = 0;
SET max_parallel_workers_per_gather = 2;
CREATE TABLE tbl_trgeo_merge AS
SELECT i, trgeometry(format('Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(%s %s),0)@%s',
  i, i, timestamptz '2001-01-01' + i * interval '1 hour')) AS t
FROM generate_series(1, 2000) i;
EXPLAIN (COSTS OFF) SELECT merge(t) FROM tbl_trgeo_merge;
SELECT numInstants(merge(t)), asText(startValue(merge(t))), asText(endValue(merge(t)))
FROM tbl_trgeo_merge;
SELECT asText(merge(t)) = (SELECT asText(merge(t ORDER BY i)) FROM tbl_trgeo_merge)
FROM tbl_trgeo_merge;
INSERT INTO tbl_trgeo_merge
SELECT 2001, trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));Pose(Point(0 0),0)@2001-01-01';
SELECT asText(merge(t)) FROM tbl_trgeo_merge;
RESET parallel_setup_cost;
RESET parallel_tuple_cost;
RESET min_parallel_table_scan_size;
RESET max_parallel_workers_per_gather;
DROP TABLE tbl_trgeo_merge;

-------------------------------------------------------------------------------
