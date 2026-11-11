-------------------------------------------------------------------------------
-- Test all operators without having collected statistics
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tint WHERE temp && intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp && floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

SELECT COUNT(*) FROM tbl_ttext WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tint WHERE temp @> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

SELECT COUNT(*) FROM tbl_ttext WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tint WHERE temp <@ intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tint WHERE temp ~= intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tint WHERE temp -|- intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------
-- Position operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp << intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp << floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp &< intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp >> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp &> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

-- Test the commutator
SELECT COUNT(*) FROM tbl_ttext WHERE ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}' <<# temp;
SELECT COUNT(*) FROM tbl_ttext WHERE ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}' &<# temp;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

-- Test the commutator
SELECT COUNT(*) FROM tbl_tbool WHERE tstzspan '[2001-01-01, 2001-06-01]' <<# temp;
SELECT COUNT(*) FROM tbl_ttext WHERE tstzspan '[2001-01-01, 2001-06-01]' <<# temp;

-------------------------------------------------------------------------------
-- Collect statistics
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Test all operators after having collected statistics
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp && intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp && floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

SELECT COUNT(*) FROM tbl_ttext WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tint WHERE temp @> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

SELECT COUNT(*) FROM tbl_ttext WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tint WHERE temp <@ intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tint WHERE temp ~= intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tint WHERE temp -|- intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';

-------------------------------------------------------------------------------
-- Position operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp << intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp << floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp &< intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp >> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp &> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';

SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

-- Test the commutator
SELECT COUNT(*) FROM tbl_ttext WHERE ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}' <<# temp;
SELECT COUNT(*) FROM tbl_ttext WHERE ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}' &<# temp;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool 'true@2001-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '{true@2001-01-01, false@2001-01-02, true@2001-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '[true@2001-01-01, false@2001-01-02, true@2001-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '{[true@2001-01-01, false@2001-01-02, true@2001-01-03],[true@2001-01-04, true@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tbox 'TBOXINT XT([1,3),[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '1@2001-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tbox 'TBOXFLOAT XT([1.5,2.5],[2001-01-01, 2001-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '1.5@2001-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext 'AAA@2001-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '{AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '{[AAA@2001-01-01, BBB@2001-01-02, AAA@2001-01-03],[CCC@2001-01-04, CCC@2001-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

-- Test the commutator
SELECT COUNT(*) FROM tbl_tbool WHERE tstzspan '[2001-01-01, 2001-06-01]' <<# temp;
SELECT COUNT(*) FROM tbl_ttext WHERE tstzspan '[2001-01-01, 2001-06-01]' <<# temp;

-------------------------------------------------------------------------------
