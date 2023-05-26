-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Test all operators without having collected statistics
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp && intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp && floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp @> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp <@ intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp ~= intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp -|- intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------
-- Position operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp << intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp << floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp &< intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp >> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp &> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

-- Test the commutator
SELECT COUNT(*) FROM tbl_ttext WHERE ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' <<# temp;
SELECT COUNT(*) FROM tbl_ttext WHERE ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' &<# temp;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

-- Test the commutator
SELECT COUNT(*) FROM tbl_tbool WHERE tstzspan '[2001-01-01, 2001-06-01]' <<# temp;
SELECT COUNT(*) FROM tbl_ttext WHERE tstzspan '[2001-01-01, 2001-06-01]' <<# temp;

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

SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp = tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp < tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp > tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp >= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp = tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp > tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp = tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp < tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp > tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp = ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp < ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp > ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp >= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp && intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp && floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp && tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';


SELECT COUNT(*) FROM tbl_tint WHERE temp @> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp @> tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp <@ intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <@ tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp ~= intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp ~= tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp -|- intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp -|- tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------
-- Position operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp << intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp << tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp << floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp << tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp &< intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &< tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &< tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp >> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp >> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp >> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE temp &> intspan '[1,3]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> floatspan '[1,3]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp <<# tstzspan '[2001-06-01, 2001-07-01]';

-- Test the commutator
SELECT COUNT(*) FROM tbl_ttext WHERE ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' <<# temp;
SELECT COUNT(*) FROM tbl_ttext WHERE ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' &<# temp;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp &<# tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #>> tstzspan '[2001-06-01, 2001-07-01]';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool 'true@2000-01-01';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '{true@2000-01-01, false@2000-01-02, true@2000-01-03}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tbool '{[true@2000-01-01, false@2000-01-02, true@2000-01-03],[true@2000-01-04, true@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tbool WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tint WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tbox 'TBOX XT([1.5,2.5],[2000-01-01, 2000-01-03])';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tint '1@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '1.5@2000-01-01';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT COUNT(*) FROM tbl_tfloat WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext 'AAA@2000-01-01';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT COUNT(*) FROM tbl_ttext WHERE temp #&> tstzspan '[2001-06-01, 2001-07-01]';

-- Test the commutator
SELECT COUNT(*) FROM tbl_tbool WHERE tstzspan '[2001-01-01, 2001-06-01]' <<# temp;
SELECT COUNT(*) FROM tbl_ttext WHERE tstzspan '[2001-01-01, 2001-06-01]' <<# temp;

-------------------------------------------------------------------------------
