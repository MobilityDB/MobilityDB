-------------------------------------------------------------------------------
-- File timeops.c
-- Tests of operators that do involve indexes for time types.
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tstzspanset_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tstzspan_big_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tstzspan_big_quadtree_idx ON tbl_tstzspan_big USING SPGIST(t);
CREATE INDEX tbl_tstzspanset_big_quadtree_idx ON tbl_tstzspanset_big USING SPGIST(t);

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t @> timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t -|- timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t <<# timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t &<# timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t #>> timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t #&> timestamptz '2001-01-01';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t <@ tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t -|- tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t #>> tstzspan '[2001-11-01, 2001-12-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t #&> tstzspan '[2001-11-01, 2001-12-01]';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t && tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t @> tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t <@ tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t -|- tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t <<# tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t &<# tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t #>> tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE t #&> tstzspanset '{[2001-01-01, 2001-02-01]}';

SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t && tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t @> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t <@ tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t -|- tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t #&> tstzspan '[2001-01-01, 2001-02-01]';

DROP INDEX tbl_tstzspan_big_quadtree_idx;
DROP INDEX tbl_tstzspanset_big_quadtree_idx;

-------------------------------------------------------------------------------
