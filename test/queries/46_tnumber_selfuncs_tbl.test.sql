-------------------------------------------------------------------------------
-- Test all operators without having collected statistics
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp = tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp = tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp = tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp = tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp <> tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp <> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp <> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp <> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp < tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp < tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp < tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp < tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp <= tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp <= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp <= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp <= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp > tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp > tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp > tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp > tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp >= tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp >= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp >= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp >= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';

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

-------------------------------------------------------------------------------

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

SELECT count(*) FROM tbl_ttext WHERE temp = ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp = ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp = ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp = ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp <> ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp <> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp <> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp <> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp < ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp < ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp < ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp < ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp <= ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp <= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp <= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp <= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp > ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp > ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp > ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp > ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp >= ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp >= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp >= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp >= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp && tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp && tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp && tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp && tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp && timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp && periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp && ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp && ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp && ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp && ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp && timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp && periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp @> tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp @> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp @> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp @> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp @> periodset '{[2001-06-01, 2001-07-01]}';


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

SELECT count(*) FROM tbl_ttext WHERE temp @> ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp @> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp @> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp @> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp @> periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp <@ tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp <@ tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp <@ tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp <@ tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp <@ timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp <@ periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp <@ ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp <@ ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp <@ ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp <@ ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp <@ timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp <@ periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp ~= tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp ~= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp ~= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp ~= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp ~= timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp ~= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp ~= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp ~= periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp ~= ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp ~= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp ~= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp ~= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp ~= timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp ~= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp ~= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp ~= periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp -|- tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp -|- tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp -|- tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp -|- tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp -|- periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp -|- ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp -|- ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp -|- ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp -|- ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp -|- periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_tbool WHERE temp <<# tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp <<# tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp <<# tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp <<# tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp <<# periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp <<# ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp <<# ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp <<# ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp <<# ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp <<# periodset '{[2001-06-01, 2001-07-01]}';

-- Test the commutator
SELECT count(*) FROM tbl_ttext WHERE ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' <<# temp;
SELECT count(*) FROM tbl_ttext WHERE ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' &<# temp;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp &<# tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp &<# tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp &<# tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp &<# tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp &<# periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp &<# ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp &<# ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp &<# ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp &<# ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp &<# periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp #>> tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp #>> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp #>> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp #>> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp #>> periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp #>> ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp #>> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp #>> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp #>> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp #>> periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp #&> tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp #&> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp #&> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp #&> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp #&> periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp #&> ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp #&> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp #&> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp #&> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp #&> periodset '{[2001-06-01, 2001-07-01]}';

-- Test the commutator
SELECT count(*) FROM tbl_tbool WHERE period '[2001-01-01, 2001-06-01]' <<# temp;
SELECT count(*) FROM tbl_ttext WHERE period '[2001-01-01, 2001-06-01]' <<# temp;

-------------------------------------------------------------------------------
-- Collect statistics
-------------------------------------------------------------------------------

analyze tbl_tbool;
analyze tbl_tint;
analyze tbl_tfloat;
analyze tbl_ttext;

-------------------------------------------------------------------------------
-- Test all operators after having collected statistics
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp = tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp = tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp = tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp = tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp <> tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp <> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp <> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp <> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp < tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp < tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp < tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp < tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp <= tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp <= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp <= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp <= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp > tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp > tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp > tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp > tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp >= tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp >= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp >= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp >= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';

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

-------------------------------------------------------------------------------

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

SELECT count(*) FROM tbl_ttext WHERE temp = ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp = ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp = ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp = ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp <> ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp <> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp <> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp <> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp < ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp < ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp < ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp < ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp <= ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp <= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp <= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp <= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp > ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp > ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp > ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp > ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp >= ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp >= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp >= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp >= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp && tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp && tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp && tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp && tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp && timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp && periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp && ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp && ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp && ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp && ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp && timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp && timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp && period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp && periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp @> tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp @> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp @> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp @> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp @> periodset '{[2001-06-01, 2001-07-01]}';


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

SELECT count(*) FROM tbl_ttext WHERE temp @> ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp @> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp @> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp @> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp @> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp @> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp @> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp @> periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp <@ tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp <@ tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp <@ tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp <@ tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp <@ timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp <@ periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp <@ ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp <@ ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp <@ ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp <@ ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp <@ timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp <@ timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp <@ period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp <@ periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp ~= tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp ~= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp ~= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp ~= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp ~= timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp ~= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp ~= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp ~= periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp ~= ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp ~= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp ~= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp ~= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp ~= timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp ~= timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp ~= period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp ~= periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp -|- tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp -|- tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp -|- tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp -|- tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp -|- periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp -|- ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp -|- ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp -|- ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp -|- ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp -|- timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp -|- timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp -|- period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp -|- periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_tbool WHERE temp <<# tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp <<# tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp <<# tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp <<# tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp <<# periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp <<# ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp <<# ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp <<# ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp <<# ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp <<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp <<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp <<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp <<# periodset '{[2001-06-01, 2001-07-01]}';

-- Test the commutator
SELECT count(*) FROM tbl_ttext WHERE ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' <<# temp;
SELECT count(*) FROM tbl_ttext WHERE ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' &<# temp;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp &<# tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp &<# tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp &<# tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp &<# tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp &<# periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp &<# ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp &<# ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp &<# ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp &<# ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp &<# timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp &<# timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp &<# period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp &<# periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp #>> tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp #>> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp #>> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp #>> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp #>> periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp #>> ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp #>> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp #>> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp #>> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp #>> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp #>> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp #>> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp #>> periodset '{[2001-06-01, 2001-07-01]}';

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE temp #&> tbool 'true@2000-01-01';
SELECT count(*) FROM tbl_tbool WHERE temp #&> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT count(*) FROM tbl_tbool WHERE temp #&> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT count(*) FROM tbl_tbool WHERE temp #&> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT count(*) FROM tbl_tbool WHERE temp #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_tbool WHERE temp #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_tbool WHERE temp #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_tbool WHERE temp #&> periodset '{[2001-06-01, 2001-07-01]}';

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

SELECT count(*) FROM tbl_ttext WHERE temp #&> ttext 'AAA@2000-01-01';
SELECT count(*) FROM tbl_ttext WHERE temp #&> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT count(*) FROM tbl_ttext WHERE temp #&> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT count(*) FROM tbl_ttext WHERE temp #&> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT count(*) FROM tbl_ttext WHERE temp #&> timestamptz '2001-06-01';
SELECT count(*) FROM tbl_ttext WHERE temp #&> timestampset '{2001-06-01, 2001-07-07}';
SELECT count(*) FROM tbl_ttext WHERE temp #&> period '[2001-06-01, 2001-07-01]';
SELECT count(*) FROM tbl_ttext WHERE temp #&> periodset '{[2001-06-01, 2001-07-01]}';

-- Test the commutator
SELECT count(*) FROM tbl_tbool WHERE period '[2001-01-01, 2001-06-01]' <<# temp;
SELECT count(*) FROM tbl_ttext WHERE period '[2001-01-01, 2001-06-01]' <<# temp;

-------------------------------------------------------------------------------
