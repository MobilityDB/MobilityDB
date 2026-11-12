-------------------------------------------------------------------------------
-- File span_ops.c
-- Tests of operators for span types.
-------------------------------------------------------------------------------

ANALYZE tbl_tstzspan_big;
ANALYZE tbl_tstzspanset_big;

DROP INDEX IF EXISTS tbl_tstzspan_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tstzspanset_big_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tstzspan_big_rtree_idx ON tbl_tstzspan_big USING GIST(t);
CREATE INDEX tbl_tstzspanset_big_rtree_idx ON tbl_tstzspanset_big USING GIST(t);

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
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE t #&> tstzspan '[2001-01-01, 2001-02-01]';

DROP INDEX IF EXISTS tbl_tstzspan_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tstzspanset_big_rtree_idx;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tstzspan_test;
CREATE TABLE tbl_tstzspan_test AS
SELECT tstzspan '[2001-01-01,2001-01-02]';
ANALYZE tbl_tstzspan_test;
DELETE FROM tbl_tstzspan_test;
INSERT INTO tbl_tstzspan_test
SELECT NULL::tstzspan UNION SELECT NULL::tstzspan;
ANALYZE tbl_tstzspan_test;
DROP TABLE tbl_tstzspan_test;

-------------------------------------------------------------------------------
