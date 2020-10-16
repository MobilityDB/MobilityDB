﻿-------------------------------------------------------------------------------

ANALYZE tbl_tbool_big;
ANALYZE tbl_tint_big;
ANALYZE tbl_tfloat_big;
ANALYZE tbl_ttext_big;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tbool_big_gist_idx;
DROP INDEX IF EXISTS tbl_tint_big_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat_big_gist_idx;
DROP INDEX IF EXISTS tbl_ttext_big_gist_idx;

CREATE INDEX tbl_tbool_big_gist_idx ON tbl_tbool_big USING GIST(temp);
CREATE INDEX tbl_tint_big_gist_idx ON tbl_tint_big USING GIST(temp);
CREATE INDEX tbl_tfloat_big_gist_idx ON tbl_tfloat_big USING GIST(temp);
CREATE INDEX tbl_ttext_big_gist_idx ON tbl_ttext_big USING GIST(temp);

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool_big WHERE temp && NULL::period;
SELECT count(*) FROM tbl_tbool_big WHERE temp && NULL::tbool;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool_big WHERE temp && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp ~= period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp -|- period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';

SELECT count(*) FROM tbl_tbool_big WHERE temp < tbool '[true@2001-01-01, true@2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp <= tbool '[true@2001-01-01, true@2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp > tbool '[true@2001-01-01, true@2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp >= tbool '[true@2001-01-01, true@2001-02-01]';

SELECT count(*) FROM tbl_tbool_big WHERE temp && tbool '[true@2001-01-01, true@2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp @> tbool '[true@2001-01-01, true@2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp <@ tbool '[true@2001-01-01, true@2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp ~= tbool '[true@2001-01-01, true@2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp -|- tbool '[true@2001-01-01, true@2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp <<# tbool '[true@2001-01-01, true@2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp &<# tbool '[true@2001-01-01, true@2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp #>> tbool '[true@2001-01-01, true@2001-02-01]';
SELECT count(*) FROM tbl_tbool_big WHERE temp #&> tbool '[true@2001-01-01, true@2001-02-01]';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint_big WHERE temp && intrange '[1,3]';
SELECT count(*) FROM tbl_tint_big WHERE temp @> intrange '[1,3]';
SELECT count(*) FROM tbl_tint_big WHERE temp <@ intrange '[1,3]';
SELECT count(*) FROM tbl_tint_big WHERE temp ~= intrange '[1,3]';
SELECT count(*) FROM tbl_tint_big WHERE temp -|- intrange '[1,3]';
SELECT count(*) FROM tbl_tint_big WHERE temp << intrange '[1,3]';
SELECT count(*) FROM tbl_tint_big WHERE temp &< intrange '[1,3]';
SELECT count(*) FROM tbl_tint_big WHERE temp >> intrange '[97,100]';
SELECT count(*) FROM tbl_tint_big WHERE temp &> intrange '[97,100]';
SELECT count(*) FROM tbl_tint_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp #>> period '[2001-11-01, 2001-12-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp #&> period '[2001-11-01, 2001-12-01]';

SELECT count(*) FROM tbl_tint_big WHERE temp && tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp @> tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp <@ tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp ~= tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp -|- tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp << tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp &< tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp >> tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp &> tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp <<# tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp &<# tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp #>> tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tint_big WHERE temp #&> tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';

SELECT count(*) FROM tbl_tint_big WHERE temp < tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp <= tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp > tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp >= tint '[1@2001-01-01, 10@2001-02-01]';

SELECT count(*) FROM tbl_tint_big WHERE temp && tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp @> tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp <@ tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp ~= tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp -|- tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp << tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp &< tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp >> tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp &> tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp <<# tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp &<# tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp #>> tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp #&> tint '[1@2001-01-01, 10@2001-02-01]';

SELECT count(*) FROM tbl_tint_big WHERE temp && tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp @> tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp <@ tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp ~= tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp -|- tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp << tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp &< tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp >> tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp &> tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp <<# tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp &<# tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp #>> tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tint_big WHERE temp #&> tfloat '[1@2001-01-01, 10@2001-02-01]';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tfloat_big WHERE temp && floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp @> floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <@ floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp ~= floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp -|- floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp << floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &< floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp >> floatrange '[97,100]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &> floatrange '[97,100]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #>> period '[2001-11-01, 2001-12-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #&> period '[2001-11-01, 2001-12-01]';

SELECT count(*) FROM tbl_tfloat_big WHERE temp && tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp @> tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <@ tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp ~= tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp -|- tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp << tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &< tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp >> tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &> tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <<# tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &<# tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #>> tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #&> tbox 'TBOX((1,2001-01-01),(50,2001-02-01))';

SELECT count(*) FROM tbl_tfloat_big WHERE temp < tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <= tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp > tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp >= tfloat '[1@2001-01-01, 10@2001-02-01]';

SELECT count(*) FROM tbl_tfloat_big WHERE temp && tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp @> tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <@ tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp ~= tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp -|- tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp << tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &< tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp >> tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &> tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <<# tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &<# tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #>> tint '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #&> tint '[1@2001-01-01, 10@2001-02-01]';

SELECT count(*) FROM tbl_tfloat_big WHERE temp && tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp @> tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <@ tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp ~= tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp -|- tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp << tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &< tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp >> tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &> tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp <<# tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp &<# tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #>> tfloat '[1@2001-01-01, 10@2001-02-01]';
SELECT count(*) FROM tbl_tfloat_big WHERE temp #&> tfloat '[1@2001-01-01, 10@2001-02-01]';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_ttext_big WHERE temp && period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp @> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp <@ period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp ~= period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp -|- period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';

SELECT count(*) FROM tbl_ttext_big WHERE temp < ttext '[AAA@2001-01-01, BBB@2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp <= ttext '[AAA@2001-01-01, BBB@2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp > ttext '[AAA@2001-01-01, BBB@2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp >= ttext '[AAA@2001-01-01, BBB@2001-02-01]';

SELECT count(*) FROM tbl_ttext_big WHERE temp && ttext '[AAA@2001-01-01, BBB@2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp @> ttext '[AAA@2001-01-01, BBB@2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp <@ ttext '[AAA@2001-01-01, BBB@2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp ~= ttext '[AAA@2001-01-01, BBB@2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp -|- ttext '[AAA@2001-01-01, BBB@2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp <<# ttext '[AAA@2001-01-01, BBB@2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp &<# ttext '[AAA@2001-01-01, BBB@2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp #>> ttext '[AAA@2001-01-01, BBB@2001-02-01]';
SELECT count(*) FROM tbl_ttext_big WHERE temp #&> ttext '[AAA@2001-01-01, BBB@2001-02-01]';

-------------------------------------------------------------------------------

-- Test the commutator for the selectivity
SELECT count(*) FROM tbl_tint_big WHERE tint '[1@2001-01-01, 10@2001-02-01]' << temp;
SELECT count(*) FROM tbl_tint_big WHERE tint '[1@2001-01-01, 10@2001-02-01]' &< temp;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tbool_big_gist_idx;
DROP INDEX IF EXISTS tbl_tint_big_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat_big_gist_idx;
DROP INDEX IF EXISTS tbl_ttext_big_gist_idx;

-------------------------------------------------------------------------------

