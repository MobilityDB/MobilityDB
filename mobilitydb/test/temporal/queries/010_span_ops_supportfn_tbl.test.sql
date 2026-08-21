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
-- Tests of the support function for the value-domain portable predicates.
-- A predicate written in function form must answer exactly what it answers
-- without an index, whichever index the planner rewrites it into.
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_intset_rtree_idx;
DROP INDEX IF EXISTS tbl_intspan_rtree_idx;
DROP INDEX IF EXISTS tbl_intspanset_rtree_idx;
DROP INDEX IF EXISTS tbl_tstzset_rtree_idx;
DROP INDEX IF EXISTS tbl_tstzspan_rtree_idx;
DROP INDEX IF EXISTS tbl_tstzspanset_rtree_idx;

DROP INDEX IF EXISTS tbl_intset_quadtree_idx;
DROP INDEX IF EXISTS tbl_intspan_quadtree_idx;
DROP INDEX IF EXISTS tbl_intspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_tstzset_quadtree_idx;
DROP INDEX IF EXISTS tbl_tstzspan_quadtree_idx;
DROP INDEX IF EXISTS tbl_tstzspanset_quadtree_idx;

DROP INDEX IF EXISTS tbl_intset_kdtree_idx;
DROP INDEX IF EXISTS tbl_intspan_kdtree_idx;
DROP INDEX IF EXISTS tbl_intspanset_kdtree_idx;
DROP INDEX IF EXISTS tbl_tstzset_kdtree_idx;
DROP INDEX IF EXISTS tbl_tstzspan_kdtree_idx;
DROP INDEX IF EXISTS tbl_tstzspanset_kdtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_spansupport;
CREATE TABLE test_spansupport(
  func TEXT,
  leftarg TEXT,
  rightarg TEXT,
  no_idx BIGINT,
  rtree_idx BIGINT,
  quadtree_idx BIGINT,
  kdtree_idx BIGINT
);

-------------------------------------------------------------------------------

INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overlaps', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overlaps(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contains', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE contains(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contained', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE contained(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'left', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE left(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overleft', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overleft(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'right', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE right(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overright', 'intset', 'intset', COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overright(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overlaps', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overlaps(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contains', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE contains(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contained', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE contained(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'left', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE left(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overleft', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overleft(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'right', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE right(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overright', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overright(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'adjacent', 'intspan', 'intspan', COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE adjacent(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overlaps', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overlaps(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contains', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE contains(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contained', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE contained(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'left', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE left(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overleft', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overleft(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'right', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE right(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overright', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overright(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'adjacent', 'intspanset', 'intspanset', COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE adjacent(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overlaps', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overlaps(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contains', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE contains(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contained', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE contained(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'left', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE left(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overleft', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overleft(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'right', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE right(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overright', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overright(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'adjacent', 'intspan', 'intspanset', COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE adjacent(t1.i, t2.i);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overlaps', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overlaps(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contains', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE contains(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contained', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE contained(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'before', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE before(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overbefore', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overbefore(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'after', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE after(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overafter', 'tstzset', 'tstzset', COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overafter(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overlaps', 'tstzspan', 'tstzspan', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overlaps(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contains', 'tstzspan', 'tstzspan', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE contains(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contained', 'tstzspan', 'tstzspan', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE contained(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'before', 'tstzspan', 'tstzspan', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE before(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overbefore', 'tstzspan', 'tstzspan', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overbefore(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'after', 'tstzspan', 'tstzspan', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE after(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overafter', 'tstzspan', 'tstzspan', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overafter(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'adjacent', 'tstzspan', 'tstzspan', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE adjacent(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overlaps', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overlaps(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contains', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE contains(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contained', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE contained(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'before', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE before(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overbefore', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overbefore(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'after', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE after(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overafter', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overafter(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'adjacent', 'tstzspanset', 'tstzspanset', COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE adjacent(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overlaps', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overlaps(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contains', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE contains(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'contained', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE contained(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'before', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE before(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overbefore', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overbefore(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'after', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE after(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'overafter', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overafter(t1.t, t2.t);
INSERT INTO test_spansupport(func, leftarg, rightarg, no_idx)
SELECT 'adjacent', 'tstzspan', 'tstzspanset', COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE adjacent(t1.t, t2.t);

-------------------------------------------------------------------------------

CREATE INDEX tbl_intset_rtree_idx ON tbl_intset USING GIST(i);
CREATE INDEX tbl_intspan_rtree_idx ON tbl_intspan USING GIST(i);
CREATE INDEX tbl_intspanset_rtree_idx ON tbl_intspanset USING GIST(i);
CREATE INDEX tbl_tstzset_rtree_idx ON tbl_tstzset USING GIST(t);
CREATE INDEX tbl_tstzspan_rtree_idx ON tbl_tstzspan USING GIST(t);
CREATE INDEX tbl_tstzspanset_rtree_idx ON tbl_tstzspanset USING GIST(t);

UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE adjacent(t1.i, t2.i) )
WHERE func = 'adjacent' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE adjacent(t1.i, t2.i) )
WHERE func = 'adjacent' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE adjacent(t1.i, t2.i) )
WHERE func = 'adjacent' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE adjacent(t1.t, t2.t) )
WHERE func = 'adjacent' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE adjacent(t1.t, t2.t) )
WHERE func = 'adjacent' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET rtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE adjacent(t1.t, t2.t) )
WHERE func = 'adjacent' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';

DROP INDEX tbl_intset_rtree_idx;
DROP INDEX tbl_intspan_rtree_idx;
DROP INDEX tbl_intspanset_rtree_idx;
DROP INDEX tbl_tstzset_rtree_idx;
DROP INDEX tbl_tstzspan_rtree_idx;
DROP INDEX tbl_tstzspanset_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intset_quadtree_idx ON tbl_intset USING SPGIST(i);
CREATE INDEX tbl_intspan_quadtree_idx ON tbl_intspan USING SPGIST(i);
CREATE INDEX tbl_intspanset_quadtree_idx ON tbl_intspanset USING SPGIST(i);
CREATE INDEX tbl_tstzset_quadtree_idx ON tbl_tstzset USING SPGIST(t);
CREATE INDEX tbl_tstzspan_quadtree_idx ON tbl_tstzspan USING SPGIST(t);
CREATE INDEX tbl_tstzspanset_quadtree_idx ON tbl_tstzspanset USING SPGIST(t);

UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE adjacent(t1.i, t2.i) )
WHERE func = 'adjacent' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE adjacent(t1.i, t2.i) )
WHERE func = 'adjacent' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE adjacent(t1.i, t2.i) )
WHERE func = 'adjacent' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE adjacent(t1.t, t2.t) )
WHERE func = 'adjacent' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE adjacent(t1.t, t2.t) )
WHERE func = 'adjacent' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET quadtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE adjacent(t1.t, t2.t) )
WHERE func = 'adjacent' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';

DROP INDEX tbl_intset_quadtree_idx;
DROP INDEX tbl_intspan_quadtree_idx;
DROP INDEX tbl_intspanset_quadtree_idx;
DROP INDEX tbl_tstzset_quadtree_idx;
DROP INDEX tbl_tstzspan_quadtree_idx;
DROP INDEX tbl_tstzspanset_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intset_kdtree_idx ON tbl_intset USING SPGIST(i intset_kdtree_ops);
CREATE INDEX tbl_intspan_kdtree_idx ON tbl_intspan USING SPGIST(i intspan_kdtree_ops);
CREATE INDEX tbl_intspanset_kdtree_idx ON tbl_intspanset USING SPGIST(i intspanset_kdtree_ops);
CREATE INDEX tbl_tstzset_kdtree_idx ON tbl_tstzset USING SPGIST(t tstzset_kdtree_ops);
CREATE INDEX tbl_tstzspan_kdtree_idx ON tbl_tstzspan USING SPGIST(t tstzspan_kdtree_ops);
CREATE INDEX tbl_tstzspanset_kdtree_idx ON tbl_tstzspanset USING SPGIST(t tstzspanset_kdtree_ops);

UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intset t1, tbl_intset t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intset' AND rightarg = 'intset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspan t2 WHERE adjacent(t1.i, t2.i) )
WHERE func = 'adjacent' AND leftarg = 'intspan' AND rightarg = 'intspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE adjacent(t1.i, t2.i) )
WHERE func = 'adjacent' AND leftarg = 'intspanset' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overlaps(t1.i, t2.i) )
WHERE func = 'overlaps' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE contains(t1.i, t2.i) )
WHERE func = 'contains' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE contained(t1.i, t2.i) )
WHERE func = 'contained' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE left(t1.i, t2.i) )
WHERE func = 'left' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overleft(t1.i, t2.i) )
WHERE func = 'overleft' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE right(t1.i, t2.i) )
WHERE func = 'right' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE overright(t1.i, t2.i) )
WHERE func = 'overright' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE adjacent(t1.i, t2.i) )
WHERE func = 'adjacent' AND leftarg = 'intspan' AND rightarg = 'intspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzset' AND rightarg = 'tstzset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE adjacent(t1.t, t2.t) )
WHERE func = 'adjacent' AND leftarg = 'tstzspan' AND rightarg = 'tstzspan';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE adjacent(t1.t, t2.t) )
WHERE func = 'adjacent' AND leftarg = 'tstzspanset' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overlaps(t1.t, t2.t) )
WHERE func = 'overlaps' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE contains(t1.t, t2.t) )
WHERE func = 'contains' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE contained(t1.t, t2.t) )
WHERE func = 'contained' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE before(t1.t, t2.t) )
WHERE func = 'before' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overbefore(t1.t, t2.t) )
WHERE func = 'overbefore' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE after(t1.t, t2.t) )
WHERE func = 'after' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE overafter(t1.t, t2.t) )
WHERE func = 'overafter' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';
UPDATE test_spansupport
SET kdtree_idx = ( SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE adjacent(t1.t, t2.t) )
WHERE func = 'adjacent' AND leftarg = 'tstzspan' AND rightarg = 'tstzspanset';

DROP INDEX tbl_intset_kdtree_idx;
DROP INDEX tbl_intspan_kdtree_idx;
DROP INDEX tbl_intspanset_kdtree_idx;
DROP INDEX tbl_tstzset_kdtree_idx;
DROP INDEX tbl_tstzspan_kdtree_idx;
DROP INDEX tbl_tstzspanset_kdtree_idx;

-------------------------------------------------------------------------------

-- The pairs that match at least one row: a mismatch test over predicates
-- that match nothing would hold vacuously
SELECT count(*) FILTER (WHERE no_idx > 0) AS matching, count(*) AS pairs
FROM test_spansupport;

-- and the pairs the fixture leaves unexercised, named
SELECT func, leftarg, rightarg FROM test_spansupport
WHERE no_idx = 0 ORDER BY func, leftarg, rightarg;

SELECT * FROM test_spansupport
WHERE no_idx <> rtree_idx OR no_idx <> quadtree_idx OR no_idx <> kdtree_idx OR
  no_idx IS NULL OR rtree_idx IS NULL OR quadtree_idx IS NULL OR kdtree_idx IS NULL
ORDER BY func, leftarg, rightarg;

DROP TABLE test_spansupport;

-------------------------------------------------------------------------------
