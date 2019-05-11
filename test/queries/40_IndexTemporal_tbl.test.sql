-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tbool10K_gist_idx;
DROP INDEX IF EXISTS tbl_tbool10K_spgist_idx;

DROP INDEX IF EXISTS tbl_tint10K_gist_idx;
DROP INDEX IF EXISTS tbl_tint10K_spgist_idx;

DROP INDEX IF EXISTS tbl_tfloat10K_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat10K_spgist_idx;

DROP INDEX IF EXISTS tbl_ttext10K_gist_idx;
DROP INDEX IF EXISTS tbl_ttext10K_spgist_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool10K_gist_idx ON tbl_tbool10K USING GIST(temp);
CREATE INDEX tbl_tint10K_gist_idx ON tbl_tint10K USING GIST(temp);
CREATE INDEX tbl_tfloat10K_gist_idx ON tbl_tfloat10K USING GIST(temp);
CREATE INDEX tbl_ttext10K_gist_idx ON tbl_ttext10K USING GIST(temp);

SELECT count(*) FROM tbl_tbool10K WHERE temp && period '[2001-01-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint10K WHERE temp && intrange '[1,50]';
SELECT count(*) FROM tbl_tfloat10K WHERE temp && floatrange '[1,50]';
SELECT count(*) FROM tbl_ttext10K WHERE temp && period '[2001-01-01, 2001-07-01]';

DROP INDEX IF EXISTS tbl_tbool10K_gist_idx;
DROP INDEX IF EXISTS tbl_tint10K_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat10K_gist_idx;
DROP INDEX IF EXISTS tbl_ttext10K_gist_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool10K_spgist_idx ON tbl_tbool10K USING SPGIST(temp);
CREATE INDEX tbl_tint10K_spgist_idx ON tbl_tint10K USING SPGIST(temp);
CREATE INDEX tbl_tfloat10K_spgist_idx ON tbl_tfloat10K USING SPGIST(temp);
CREATE INDEX tbl_ttext10K_spgist_idx ON tbl_ttext10K USING SPGIST(temp);

SELECT count(*) FROM tbl_tbool10K WHERE temp && period '[2001-01-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint10K WHERE temp && intrange '[1,50]';
SELECT count(*) FROM tbl_tfloat10K WHERE temp && floatrange '[1,50]';
SELECT count(*) FROM tbl_ttext10K WHERE temp && period '[2001-01-01, 2001-07-01]';

DROP INDEX IF EXISTS tbl_tbool10K_gist_idx;
DROP INDEX IF EXISTS tbl_tint10K_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat10K_gist_idx;
DROP INDEX IF EXISTS tbl_ttext10K_gist_idx;


-------------------------------------------------------------------------------

