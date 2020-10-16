DROP FUNCTION IF EXISTS create_test_tables_big_dup();
CREATE OR REPLACE FUNCTION create_test_tables_big_dup(size int DEFAULT 10000) 
RETURNS text AS $$
DECLARE
  perc int;
BEGIN
perc := size * 0.5;
IF perc < 1 THEN perc := 1; END IF;

-----------------------------------------------------------

DROP TABLE IF EXISTS tbl_timestampset_test;
CREATE TABLE tbl_timestampset_test AS
SELECT k, random_timestampset('2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
INSERT INTO tbl_timestampset_test
SELECT i + 50, timestampset '{2001-01-01, 2001-02-01, 2001-03-01}'
FROM generate_series(1, perc) i;

DROP INDEX IF EXISTS tbl_timestampset_test_gist_idx;
CREATE INDEX tbl_timestampset_test_gist_idx ON tbl_timestampset_test USING gist(ti);

-- DROP INDEX IF EXISTS tbl_timestampset_test_spgist_idx;
-- CREATE INDEX tbl_timestampset_test_spgist_idx ON tbl_timestampset_test USING spgist(ti);

-- SELECT count(*) FROM tbl_timestampset_test 
-- WHERE ti && timestampset '{2001-01-01, 2001-02-01, 2001-03-01}';

-----------------------------------------------------------

DROP TABLE IF EXISTS tbl_ttexti_test;
CREATE TABLE tbl_ttexti_test AS
SELECT k, random_ttexti('2001-01-01', '2001-12-31', 10, 10, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
INSERT INTO tbl_ttexti_test
SELECT i + 50, ttext '{AA@2001-01-01, DD@2001-02-01, FF@2001-03-01}'
FROM generate_series(1, perc) i;

DROP INDEX IF EXISTS tbl_ttexti_test_gist_idx;
CREATE INDEX tbl_ttexti_test_gist_idx ON tbl_ttexti_test USING gist(ti);

-- DROP INDEX IF EXISTS tbl_ttexti_test_spgist_idx;
-- CREATE INDEX tbl_ttexti_test_spgist_idx ON tbl_ttexti_test USING spgist(ti);

-- SELECT count(*) FROM tbl_ttexti_test 
-- WHERE ti && ttext '{AA@2001-01-01, DD@2001-02-01, FF@2001-03-01}';

-----------------------------------------------------------

DROP TABLE IF EXISTS tbl_tfloati_test;
CREATE TABLE tbl_tfloati_test AS
SELECT k, random_tfloati(0, 100, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
INSERT INTO tbl_tfloati_test
SELECT i + 50, tfloat '{1@2001-01-01, 10@2001-02-01, 20@2001-03-01}'
FROM generate_series(1, perc) i;

DROP INDEX IF EXISTS tbl_tfloati_test_gist_idx;
CREATE INDEX tbl_tfloati_test_gist_idx ON tbl_tfloati_test USING gist(ti);

-- DROP INDEX IF EXISTS tbl_tfloati_test_spgist_idx;
-- CREATE INDEX tbl_tfloati_test_spgist_idx ON tbl_tfloati_test USING spgist(ti);

-- SELECT count(*) FROM tbl_tfloati_test 
-- WHERE ti && tfloat '{1@2001-01-01, 10@2001-02-01, 20@2001-03-01}';

-----------------------------------------------------------

DROP TABLE IF EXISTS tbl_tgeompointi_test;
CREATE TABLE tbl_tgeompointi_test AS
SELECT k, random_tgeompointi(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1, size) k;
/* Add perc duplicates */
INSERT INTO tbl_tgeompointi_test
SELECT i + 50, tgeompoint '{Point(1 1)@2001-01-01, Point(10 10)@2001-02-01, Point(20 20)@2001-03-01}'
FROM generate_series(1, perc) i;

DROP INDEX IF EXISTS tbl_tgeompointi_test_gist_idx;
CREATE INDEX tbl_tgeompointi_test_gist_idx ON tbl_tgeompointi_test USING gist(ti);

-- DROP INDEX IF EXISTS tbl_tgeompointi_test_spgist_idx;
-- CREATE INDEX tbl_tgeompointi_test_spgist_idx ON tbl_tgeompointi_test USING spgist(ti);

-- SELECT count(*) FROM tbl_tgeompointi_test 
-- WHERE ti && tgeompoint '{Point(1 1)@2001-01-01, Point(10 10)@2001-02-01, Point(20 20)@2001-03-01}';

-------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_big_dup(10000)
