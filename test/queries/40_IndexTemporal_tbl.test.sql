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

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tbool10K_gist_idx;
DROP INDEX IF EXISTS tbl_tint10K_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat10K_gist_idx;
DROP INDEX IF EXISTS tbl_ttext10K_gist_idx;

CREATE INDEX tbl_tbool10K_spgist_idx ON tbl_tbool10K USING SPGIST(temp);
CREATE INDEX tbl_tint10K_spgist_idx ON tbl_tint10K USING SPGIST(temp);
CREATE INDEX tbl_tfloat10K_spgist_idx ON tbl_tfloat10K USING SPGIST(temp);
CREATE INDEX tbl_ttext10K_spgist_idx ON tbl_ttext10K USING SPGIST(temp);

-------------------------------------------------------------------------------

