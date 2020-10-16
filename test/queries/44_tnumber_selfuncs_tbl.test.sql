-------------------------------------------------------------------------------
-- Test all operators without having collected statistics
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp = tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp = tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp = tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp = tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp < tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp < tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <= tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp > tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp > tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp > tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp > tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp >= tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp >= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp >= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp >= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';

SELECT count(*) FROM tbl_tfloat WHERE temp = tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp = tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp = tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp = tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp < tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp < tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp < tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp < tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <= tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp > tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp > tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp > tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp > tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp >= tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp >= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp >= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp >= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp && 1;
SELECT count(*) FROM tbl_tint WHERE temp && 1.5;
SELECT count(*) FROM tbl_tint WHERE temp && intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp && tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp && tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp && tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp && tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp && tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp && tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp && tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp && tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp && tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp && timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp && periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp && 1;
SELECT count(*) FROM tbl_tfloat WHERE temp && 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp && floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp && tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp && tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp && tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp && tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp && tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp && tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp && tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp && tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp && tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp && timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp && periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp @> 1;
SELECT count(*) FROM tbl_tint WHERE temp @> 1.5;
SELECT count(*) FROM tbl_tint WHERE temp @> intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp @> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp @> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp @> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp @> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp @> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp @> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp @> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp @> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp @> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp @> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp @> 1;
SELECT count(*) FROM tbl_tfloat WHERE temp @> 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp @> floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp @> periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp <@ 1;
SELECT count(*) FROM tbl_tint WHERE temp <@ 1.5;
SELECT count(*) FROM tbl_tint WHERE temp <@ intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp <@ tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp <@ tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <@ tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <@ tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <@ tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <@ tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <@ tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <@ tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <@ tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <@ timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp <@ periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp <@ 1;
SELECT count(*) FROM tbl_tfloat WHERE temp <@ 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp <@ floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp ~= 1;
SELECT count(*) FROM tbl_tint WHERE temp ~= 1.5;
SELECT count(*) FROM tbl_tint WHERE temp ~= intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp ~= tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp ~= tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp ~= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp ~= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp ~= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp ~= tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp ~= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp ~= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp ~= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp ~= timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp ~= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp ~= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp ~= periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp ~= 1;
SELECT count(*) FROM tbl_tfloat WHERE temp ~= 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp ~= floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp -|- 1;
SELECT count(*) FROM tbl_tint WHERE temp -|- 1.5;
SELECT count(*) FROM tbl_tint WHERE temp -|- intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp -|- tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp -|- tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp -|- tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp -|- tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp -|- tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp -|- tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp -|- tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp -|- tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp -|- tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp -|- periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp -|- 1;
SELECT count(*) FROM tbl_tfloat WHERE temp -|- 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp -|- floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------
-- Position operators
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp << 1;
SELECT count(*) FROM tbl_tint WHERE temp << 1.5;
SELECT count(*) FROM tbl_tint WHERE temp << intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp << tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp << tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp << tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp << tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp << tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp << tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp << tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp << tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp << tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT count(*) FROM tbl_tfloat WHERE temp << 1;
SELECT count(*) FROM tbl_tfloat WHERE temp << 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp << floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp << tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp << tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp << tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp << tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp << tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp << tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp << tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp << tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp << tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp &< 1;
SELECT count(*) FROM tbl_tint WHERE temp &< 1.5;
SELECT count(*) FROM tbl_tint WHERE temp &< intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp &< tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp &< tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &< tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &< tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &< tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp &< tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &< tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &< tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &< tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT count(*) FROM tbl_tfloat WHERE temp &< 1;
SELECT count(*) FROM tbl_tfloat WHERE temp &< 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp &< floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp >> 1;
SELECT count(*) FROM tbl_tint WHERE temp >> 1.5;
SELECT count(*) FROM tbl_tint WHERE temp >> intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp >> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp >> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp >> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp >> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp >> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp >> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp >> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp >> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp >> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT count(*) FROM tbl_tfloat WHERE temp >> 1;
SELECT count(*) FROM tbl_tfloat WHERE temp >> 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp >> floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp &> 1;
SELECT count(*) FROM tbl_tint WHERE temp &> 1.5;
SELECT count(*) FROM tbl_tint WHERE temp &> intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp &> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp &> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp &> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT count(*) FROM tbl_tfloat WHERE temp &> 1;
SELECT count(*) FROM tbl_tfloat WHERE temp &> 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp &> floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp <<# tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp <<# tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <<# tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp <<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp <<# tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp &<# tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp &<# tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp &<# tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp &<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp &<# tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp #>> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp #>> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp #>> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp #>> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp #>> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp #>> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp #>> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp #>> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp #>> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp #>> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp #>> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp #&> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp #&> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp #&> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp #&> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp #&> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp #&> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp #&> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp #&> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp #&> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp #&> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp #&> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------
-- Collect statistics
-------------------------------------------------------------------------------

analyze tbl_tint;
analyze tbl_tfloat;

-------------------------------------------------------------------------------
-- Test all operators after having collected statistics
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp = tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp = tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp = tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp = tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp < tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp < tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <= tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp > tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp > tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp > tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp > tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp >= tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp >= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp >= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp >= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';

SELECT count(*) FROM tbl_tfloat WHERE temp = tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp = tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp = tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp = tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp < tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp < tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp < tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp < tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <= tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp > tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp > tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp > tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp > tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp >= tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp >= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp >= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp >= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp && 1;
SELECT count(*) FROM tbl_tint WHERE temp && 1.5;
SELECT count(*) FROM tbl_tint WHERE temp && intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp && tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp && tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp && tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp && tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp && tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp && tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp && tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp && tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp && tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp && timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp && periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp && 1;
SELECT count(*) FROM tbl_tfloat WHERE temp && 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp && floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp && tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp && tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp && tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp && tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp && tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp && tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp && tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp && tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp && tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp && timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp && periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp @> 1;
SELECT count(*) FROM tbl_tint WHERE temp @> 1.5;
SELECT count(*) FROM tbl_tint WHERE temp @> intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp @> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp @> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp @> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp @> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp @> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp @> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp @> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp @> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp @> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp @> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp @> 1;
SELECT count(*) FROM tbl_tfloat WHERE temp @> 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp @> floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp @> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp @> periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp <@ 1;
SELECT count(*) FROM tbl_tint WHERE temp <@ 1.5;
SELECT count(*) FROM tbl_tint WHERE temp <@ intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp <@ tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp <@ tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <@ tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <@ tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <@ tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <@ tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <@ tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <@ tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <@ tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <@ timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp <@ periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp <@ 1;
SELECT count(*) FROM tbl_tfloat WHERE temp <@ 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp <@ floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp <@ periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp ~= 1;
SELECT count(*) FROM tbl_tint WHERE temp ~= 1.5;
SELECT count(*) FROM tbl_tint WHERE temp ~= intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp ~= tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp ~= tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp ~= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp ~= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp ~= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp ~= tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp ~= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp ~= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp ~= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp ~= timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp ~= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp ~= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp ~= periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp ~= 1;
SELECT count(*) FROM tbl_tfloat WHERE temp ~= 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp ~= floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp ~= periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp -|- 1;
SELECT count(*) FROM tbl_tint WHERE temp -|- 1.5;
SELECT count(*) FROM tbl_tint WHERE temp -|- intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp -|- tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp -|- tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp -|- tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp -|- tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp -|- tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp -|- tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp -|- tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp -|- tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp -|- tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp -|- periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp -|- 1;
SELECT count(*) FROM tbl_tfloat WHERE temp -|- 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp -|- floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp -|- periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------
-- Position operators
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp << 1;
SELECT count(*) FROM tbl_tint WHERE temp << 1.5;
SELECT count(*) FROM tbl_tint WHERE temp << intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp << tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp << tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp << tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp << tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp << tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp << tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp << tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp << tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp << tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT count(*) FROM tbl_tfloat WHERE temp << 1;
SELECT count(*) FROM tbl_tfloat WHERE temp << 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp << floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp << tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp << tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp << tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp << tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp << tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp << tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp << tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp << tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp << tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp &< 1;
SELECT count(*) FROM tbl_tint WHERE temp &< 1.5;
SELECT count(*) FROM tbl_tint WHERE temp &< intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp &< tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp &< tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &< tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &< tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &< tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp &< tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &< tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &< tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &< tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT count(*) FROM tbl_tfloat WHERE temp &< 1;
SELECT count(*) FROM tbl_tfloat WHERE temp &< 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp &< floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &< tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp >> 1;
SELECT count(*) FROM tbl_tint WHERE temp >> 1.5;
SELECT count(*) FROM tbl_tint WHERE temp >> intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp >> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp >> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp >> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp >> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp >> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp >> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp >> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp >> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp >> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT count(*) FROM tbl_tfloat WHERE temp >> 1;
SELECT count(*) FROM tbl_tfloat WHERE temp >> 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp >> floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp >> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp &> 1;
SELECT count(*) FROM tbl_tint WHERE temp &> 1.5;
SELECT count(*) FROM tbl_tint WHERE temp &> intrange '[1,3]';
SELECT count(*) FROM tbl_tint WHERE temp &> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp &> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp &> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT count(*) FROM tbl_tfloat WHERE temp &> 1;
SELECT count(*) FROM tbl_tfloat WHERE temp &> 1.5;
SELECT count(*) FROM tbl_tfloat WHERE temp &> floatrange '[1,3]';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp <<# tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp <<# tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <<# tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp <<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp <<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp <<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp <<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp <<# tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp <<# periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp &<# tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp &<# tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp &<# tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp &<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp &<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp &<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp &<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp &<# tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp &<# periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp #>> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp #>> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp #>> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp #>> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp #>> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp #>> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp #>> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp #>> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp #>> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp #>> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp #>> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp #>> periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint WHERE temp #&> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tint WHERE temp #&> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp #&> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp #&> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp #&> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp #&> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tint WHERE temp #&> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tint WHERE temp #&> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tint WHERE temp #&> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tint WHERE temp #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tint WHERE temp #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tint WHERE temp #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tint WHERE temp #&> periodset '{[2001-06-01, 2001-07-01]}';

SELECT count(*) FROM tbl_tfloat WHERE temp #&> tbox 'TBOX((1.5,2001-01-01),(2.5,2001-01-03))';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tint '1@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tfloat '1.5@2000-01-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tfloat WHERE temp #&> periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------
