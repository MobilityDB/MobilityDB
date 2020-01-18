-------------------------------------------------------------------------------

VACUUM ANALYZE tbl_tbool_big;
VACUUM ANALYZE tbl_tint_big;
VACUUM ANALYZE tbl_tfloat_big;
VACUUM ANALYZE tbl_ttext_big;

DROP INDEX IF EXISTS tbl_tbool_big_gist_idx;
DROP INDEX IF EXISTS tbl_tint_big_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat_big_gist_idx;
DROP INDEX IF EXISTS tbl_ttext_big_gist_idx;

#if MOBDB_PGSQL_VERSION >= 110000
DROP INDEX IF EXISTS tbl_tbool_big_spgist_idx;
DROP INDEX IF EXISTS tbl_tint_big_spgist_idx;
DROP INDEX IF EXISTS tbl_tfloat_big_spgist_idx;
DROP INDEX IF EXISTS tbl_ttext_big_spgist_idx;
#endif

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_big_gist_idx ON tbl_tbool_big USING GIST(temp);
CREATE INDEX tbl_tint_big_gist_idx ON tbl_tint_big USING GIST(temp);
CREATE INDEX tbl_tfloat_big_gist_idx ON tbl_tfloat_big USING GIST(temp);
CREATE INDEX tbl_ttext_big_gist_idx ON tbl_ttext_big USING GIST(temp);

SELECT count(*) FROM tbl_tbool_big WHERE temp && NULL::period;
SELECT count(*) FROM tbl_tbool_big WHERE temp && NULL::tbool;
SELECT count(*) FROM tbl_tbool_big WHERE temp && tbool '[true@2001-01-01, true@2001-02-01]';

SELECT count(*) FROM tbl_tbool_big WHERE temp && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp ~= period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';

SELECT count(*) FROM tbl_tint_big WHERE temp && intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp @> intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp <@ intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp ~= intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp << intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp &< intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp >> intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp &> intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp && tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp @> tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp <@ tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp ~= tint '[1@2001-01-01, 10@2001-02-01]';

SELECT count(*) FROM tbl_tfloat_big WHERE temp && floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp @> floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <@ floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp ~= floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp << floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &< floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp >> floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &> floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp && tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp @> tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <@ tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp ~= tfloat '[1@2001-01-01, 10@2001-02-01]';

SELECT count(*) FROM tbl_ttext_big WHERE temp && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp ~= period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';

DROP INDEX IF EXISTS tbl_tbool_big_gist_idx;
DROP INDEX IF EXISTS tbl_tint_big_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat_big_gist_idx;
DROP INDEX IF EXISTS tbl_ttext_big_gist_idx;

-------------------------------------------------------------------------------

#if MOBDB_PGSQL_VERSION >= 110000

CREATE INDEX tbl_tbool_big_spgist_idx ON tbl_tbool_big USING SPGIST(temp);
CREATE INDEX tbl_tint_big_spgist_idx ON tbl_tint_big USING SPGIST(temp);
CREATE INDEX tbl_tfloat_big_spgist_idx ON tbl_tfloat_big USING SPGIST(temp);
CREATE INDEX tbl_ttext_big_spgist_idx ON tbl_ttext_big USING SPGIST(temp);

SELECT count(*) FROM tbl_tbool_big WHERE temp && NULL::period;
SELECT count(*) FROM tbl_tbool_big WHERE temp && NULL::tbool;
SELECT count(*) FROM tbl_tbool_big WHERE temp && tbool '[true@2001-01-01, true@2001-02-01]';

SELECT count(*) FROM tbl_tbool_big WHERE temp && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp ~= period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';

SELECT count(*) FROM tbl_tint_big WHERE temp && intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp @> intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp <@ intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp ~= intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp << intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp &< intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp >> intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp &> intrange '[1,50]';
SELECT count(*) FROM tbl_tint_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp && tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp @> tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp <@ tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp ~= tint '[1@2001-01-01, 10@2001-02-01]';

SELECT count(*) FROM tbl_tfloat_big WHERE temp && floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp @> floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <@ floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp ~= floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp << floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &< floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp >> floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &> floatrange '[1,50]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp && tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp @> tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <@ tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp ~= tfloat '[1@2001-01-01, 10@2001-02-01]';

SELECT count(*) FROM tbl_ttext_big WHERE temp && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp ~= period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';

DROP INDEX IF EXISTS tbl_tbool_big_spgist_idx;
DROP INDEX IF EXISTS tbl_tint_big_spgist_idx;
DROP INDEX IF EXISTS tbl_tfloat_big_spgist_idx;
DROP INDEX IF EXISTS tbl_ttext_big_spgist_idx;

#endif

-------------------------------------------------------------------------------

