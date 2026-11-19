-- This test creates its indexes on copies of the tables of the setup scripts,
-- which hide those tables in its session: every test reads them, and an
-- index on them would change the plans of the other tests.
CREATE TEMP TABLE tbl_intspanset AS TABLE tbl_intspanset;
CREATE TEMP TABLE tbl_floatspanset AS TABLE tbl_floatspanset;
ANALYZE tbl_intspanset, tbl_floatspanset;

-------------------------------------------------------------------------------
-- File span_ops.c
-- Tests of operators for span types.
-------------------------------------------------------------------------------
DROP INDEX IF EXISTS tbl_intspanset_rtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_rtree_idx;

DROP INDEX IF EXISTS tbl_intspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_rtree_idx ON tbl_intspanset USING GIST(i);
CREATE INDEX tbl_floatspanset_rtree_idx ON tbl_floatspanset USING GIST(f);

SELECT COUNT(*) FROM tbl_intspanset WHERE i @> 50;
SELECT COUNT(*) FROM tbl_intspanset WHERE i -|- 96;
SELECT COUNT(*) FROM tbl_intspanset WHERE i << 15;
SELECT COUNT(*) FROM tbl_intspanset WHERE i &< 15;
SELECT COUNT(*) FROM tbl_intspanset WHERE i >> 85;
SELECT COUNT(*) FROM tbl_intspanset WHERE i &> 85;

SELECT COUNT(*) FROM tbl_intspanset WHERE i && intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i @> intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i <@ intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i -|- intspan '[96, 105]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i << intspan '[15, 25]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i &< intspan '[15, 25]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i >> intspan '[85, 95]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i &> intspan '[85, 95]';

SELECT i <-> 101 FROM tbl_intspanset ORDER BY 1 LIMIT 3;
SELECT i <-> intspan '[101,105]' FROM tbl_intspanset ORDER BY 1 LIMIT 3;

SELECT COUNT(*) FROM tbl_floatspanset WHERE f @> 50.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f -|- 50.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f << 15.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &< 15.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f >> 85.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &> 85.0;

SELECT COUNT(*) FROM tbl_floatspanset WHERE f && floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f @> floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f <@ floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f << floatspan '[15, 25]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &< floatspan '[15, 25]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f >> floatspan '[85, 95]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &> floatspan '[85, 95]';

SELECT round((f <-> 101.0), 6) FROM tbl_floatspanset ORDER BY 1 LIMIT 3;
SELECT round((f <-> floatspan '[101,105]'), 6) FROM tbl_floatspanset ORDER BY 1 LIMIT 3;

DROP INDEX IF EXISTS tbl_intspanset_rtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_quadtree_idx ON tbl_intspanset USING SPGIST(i);
CREATE INDEX tbl_floatspanset_quadtree_idx ON tbl_floatspanset USING SPGIST(f);

SELECT COUNT(*) FROM tbl_intspanset WHERE i @> 50;
SELECT COUNT(*) FROM tbl_intspanset WHERE i -|- 96;
SELECT COUNT(*) FROM tbl_intspanset WHERE i << 15;
SELECT COUNT(*) FROM tbl_intspanset WHERE i &< 15;
SELECT COUNT(*) FROM tbl_intspanset WHERE i >> 85;
SELECT COUNT(*) FROM tbl_intspanset WHERE i &> 85;

SELECT COUNT(*) FROM tbl_intspanset WHERE i && intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i @> intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i <@ intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i -|- intspan '[96, 105]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i << intspan '[15, 25]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i &< intspan '[15, 25]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i >> intspan '[85, 95]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i &> intspan '[85, 95]';

SELECT i <-> 101 FROM tbl_intspanset ORDER BY 1 LIMIT 3;
SELECT i <-> intspan '[101,105]' FROM tbl_intspanset ORDER BY 1 LIMIT 3;

SELECT COUNT(*) FROM tbl_floatspanset WHERE f @> 50.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f -|- 50.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f << 15.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &< 15.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f >> 85.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &> 85.0;

SELECT COUNT(*) FROM tbl_floatspanset WHERE f && floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f @> floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f <@ floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f << floatspan '[15, 25]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &< floatspan '[15, 25]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f >> floatspan '[85, 95]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &> floatspan '[85, 95]';

SELECT round((f <-> 101.0), 6) FROM tbl_floatspanset ORDER BY 1 LIMIT 3;
SELECT round((f <-> floatspan '[101,105]'), 6) FROM tbl_floatspanset ORDER BY 1 LIMIT 3;

DROP INDEX IF EXISTS tbl_intspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_quadtree_idx;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Exclusion constraints
-------------------------------------------------------------------------------

CREATE TEMP TABLE tbl_datespanset_excl (k int, d datespanset,
  EXCLUDE USING gist (d WITH &&));
INSERT INTO tbl_datespanset_excl VALUES
  (1, '{[2001-01-01, 2001-01-05), [2001-01-10, 2001-01-15)}'),
  (2, '{[2001-01-05, 2001-01-10)}');
/* Errors */
INSERT INTO tbl_datespanset_excl VALUES (3, '{[2001-01-12, 2001-01-20)}');
SELECT k, d FROM tbl_datespanset_excl ORDER BY k;
DROP TABLE tbl_datespanset_excl;

CREATE TEMP TABLE tbl_tstzspanset_excl (k int, t tstzspanset,
  EXCLUDE USING gist (t WITH &&));
INSERT INTO tbl_tstzspanset_excl VALUES
  (1, '{[2001-01-01, 2001-01-05), [2001-01-10, 2001-01-15)}'),
  (2, '{[2001-01-05, 2001-01-10)}');
/* Errors */
INSERT INTO tbl_tstzspanset_excl VALUES (3, '{[2001-01-12, 2001-01-20)}');
SELECT COUNT(*) FROM tbl_tstzspanset_excl;
DROP TABLE tbl_tstzspanset_excl;

CREATE TEMP TABLE tbl_datespan_excl (k int, d datespan,
  EXCLUDE USING gist (d WITH &&));
INSERT INTO tbl_datespan_excl VALUES
  (1, '[2001-01-01, 2001-01-05)'), (2, '[2001-01-05, 2001-01-10)');
/* Errors */
INSERT INTO tbl_datespan_excl VALUES (3, '[2001-01-06, 2001-01-08)');
SELECT COUNT(*) FROM tbl_datespan_excl;
DROP TABLE tbl_datespan_excl;

-------------------------------------------------------------------------------
