-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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
-- Tbox
-------------------------------------------------------------------------------

SELECT tbox 'TBOX((1.0, 2000-01-01), (1.0, 2000-01-02))'; -- Both X and T dimensions
SELECT tbox 'TBOX((1.0,), (1.0,))'; -- Only X dimension
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))'; -- Only T dimension

SELECT tbox 'TBOX((2,2000-01-02),(1,2000-01-01))';

/* Errors */
SELECT tbox 'XXX(1, 2000-01-02)';
SELECT tbox 'TBOX(1, 2000-01-02)';
SELECT tbox 'TBOX((,),(,2000-01-01))';
SELECT tbox 'TBOX((AA, 2000-01-02))';
SELECT tbox 'TBOX((1, AA))';
SELECT tbox 'TBOX((1, 2000-01-01, 2))';
SELECT tbox 'TBOX((1, 2000-01-01),2, 2000-01-02))';
SELECT tbox 'TBOX((1, 2000-01-01),(AA, 2000-01-02))';
SELECT tbox 'TBOX((1, 2000-01-01),(2, AA))';
SELECT tbox 'TBOX((1, 2000-01-01),(2, 2000-01-02)';

-- Send/receive functions

COPY tbl_tbox TO '/tmp/tbl_tbox' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tbox_tmp;
CREATE TABLE tbl_tbox_tmp AS TABLE tbl_tbox WITH NO DATA;
COPY tbl_tbox_tmp FROM '/tmp/tbl_tbox' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox_tmp t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
DROP TABLE tbl_tbox_tmp;

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT tbox(1,'2000-01-01',2,'2000-01-02');
SELECT tbox(2,'2000-01-02',1,'2000-01-01');
SELECT tbox(1,2);
SELECT tbox(2,1);
SELECT tboxt('2000-01-01','2000-01-02');
SELECT tboxt('2000-01-02','2000-01-01');

-------------------------------------------------------------------------------
-- Casting
-------------------------------------------------------------------------------

SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))'::floatrange;
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))'::period;
SELECT tbox 'TBOX((1.0,), (2.0, ))'::floatrange;
SELECT tbox 'TBOX((1.0,), (2.0, ))'::period;
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))'::floatrange;
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))'::period;

SELECT 1::tbox;
SELECT 1.5::tbox;
SELECT floatrange 'empty'::tbox;
SELECT floatrange '(,1]'::tbox;
SELECT floatrange '[1,)'::tbox;
SELECT floatrange '[1,2]'::tbox;

SELECT tbox(floatrange 'empty', timestamptz '2000-01-01');
SELECT tbox(floatrange 'empty', period '[2000-01-01,2000-01-02]');

-------------------------------------------------------------------------------

SELECT ROUND(MAX(upper(b::floatrange) - lower(b::floatrange))::numeric, 6) FROM tbl_tbox;
SELECT MAX(duration(b::period)) FROM tbl_tbox;

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT hasX(tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))');
SELECT hasX(tbox 'TBOX((1.0,), (2.0, ))');
SELECT hasX(tbox 'TBOX((, 2000-01-01), (, 2000-01-02))');

SELECT hasT(tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))');
SELECT hasT(tbox 'TBOX((1.0,), (2.0, ))');
SELECT hasT(tbox 'TBOX((, 2000-01-01), (, 2000-01-02))');

SELECT Xmin(tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))');
SELECT Xmax(tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))');
SELECT Tmin(tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))');
SELECT Tmax(tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))');

SELECT Xmin(tbox 'TBOX((1.0,), (2.0, ))');
SELECT Xmax(tbox 'TBOX((1.0,), (2.0, ))');
SELECT Tmin(tbox 'TBOX((1.0,), (2.0, ))');
SELECT Tmax(tbox 'TBOX((1.0,), (2.0, ))');

SELECT Xmin(tbox 'TBOX((, 2000-01-01), (, 2000-01-02))');
SELECT Xmax(tbox 'TBOX((, 2000-01-01), (, 2000-01-02))');
SELECT Tmin(tbox 'TBOX((, 2000-01-01), (, 2000-01-02))');
SELECT Tmax(tbox 'TBOX((, 2000-01-01), (, 2000-01-02))');

-------------------------------------------------------------------------------

SELECT MIN(xmin(b)) FROM tbl_tbox;
SELECT MAX(xmax(b)) FROM tbl_tbox;
SELECT MIN(tmin(b)) FROM tbl_tbox;
SELECT MAX(tmax(b)) FROM tbl_tbox;

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT expandValue(tbox 'TBOX((1,2001-01-01),(2,2001-01-02))', 2);
SELECT expandTemporal(tbox 'TBOX((1,2001-01-01),(2,2001-01-02))', interval '1 day');
SELECT round(tbox 'TBOX((1.123456789,2001-01-01),(2.123456789,2001-01-02))', 2);
/* Errors */
SELECT expandValue(tbox 'TBOX((,2001-01-01),(,2001-01-02))', 2);
SELECT expandTemporal(tbox 'TBOX((1),(2))', interval '1 day');
SELECT round(tbox 'TBOX((,2001-01-01),(,2001-01-02))', 2);

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

SELECT tbox 'TBOX((1,2001-01-01),(2,2001-01-02))' && tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';
SELECT tbox 'TBOX((1,2001-02-01),(2,2001-01-02))' @> tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';
SELECT tbox 'TBOX((1,2001-02-01),(2,2001-01-02))' <@ tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';
SELECT tbox 'TBOX((1,2001-02-01),(2,2001-01-02))' -|- tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';
SELECT tbox 'TBOX((1,2001-02-01),(2,2001-01-02))' ~= tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';

SELECT period '[2000-01-01, 2000-01-02]'::tbox -|- period '[2000-01-02, 2000-01-03]'::tbox;

/* Errors */
SELECT tbox 'TBOX((1),(2))' && tbox 'TBOX((,2001-01-01),(,2001-01-02))';
SELECT tbox 'TBOX((1),(2))' @> tbox 'TBOX((,2001-01-01),(,2001-01-02))';
SELECT tbox 'TBOX((1),(2))' <@ tbox 'TBOX((,2001-01-01),(,2001-01-02))';
SELECT tbox 'TBOX((1),(2))' -|- tbox 'TBOX((,2001-01-01),(,2001-01-02))';
SELECT tbox 'TBOX((1),(2))' ~= tbox 'TBOX((,2001-01-01),(,2001-01-02))';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b && t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b @> t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b <@ t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b -|- t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b ~= t2.b;

-------------------------------------------------------------------------------
-- Position operators
-------------------------------------------------------------------------------

SELECT tbox 'TBOX((1,2001-01-01),(2,2001-01-02))' << tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';
SELECT tbox 'TBOX((1,2001-01-01),(2,2001-01-02))' &< tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';
SELECT tbox 'TBOX((1,2001-01-01),(2,2001-01-02))' >> tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';
SELECT tbox 'TBOX((1,2001-01-01),(2,2001-01-02))' &> tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';
SELECT tbox 'TBOX((1,2001-01-01),(2,2001-01-02))' <<# tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';
SELECT tbox 'TBOX((1,2001-01-01),(2,2001-01-02))' &<# tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';
SELECT tbox 'TBOX((1,2001-01-01),(2,2001-01-02))' #>> tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';
SELECT tbox 'TBOX((1,2001-01-01),(2,2001-01-02))' #&> tbox 'TBOX((1,2001-01-01),(2,2001-01-02))';

/* Errors */
SELECT tbox 'TBOX((1),(2))' << tbox 'TBOX((,2001-01-01),(,2001-01-02))';
SELECT tbox 'TBOX((1),(2))' &< tbox 'TBOX((,2001-01-01),(,2001-01-02))';
SELECT tbox 'TBOX((1),(2))' >> tbox 'TBOX((,2001-01-01),(,2001-01-02))';
SELECT tbox 'TBOX((1),(2))' &> tbox 'TBOX((,2001-01-01),(,2001-01-02))';
SELECT tbox 'TBOX((1),(2))' <<# tbox 'TBOX((,2001-01-01),(,2001-01-02))';
SELECT tbox 'TBOX((1),(2))' &<# tbox 'TBOX((,2001-01-01),(,2001-01-02))';
SELECT tbox 'TBOX((1),(2))' #>> tbox 'TBOX((,2001-01-01),(,2001-01-02))';
SELECT tbox 'TBOX((1),(2))' #&> tbox 'TBOX((,2001-01-01),(,2001-01-02))';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b << t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b &< t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b >> t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b &> t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b <<# t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b &<# t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b #>> t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b #&> t2.b;

-------------------------------------------------------------------------------
-- Set operators
-------------------------------------------------------------------------------

SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' + tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))';
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' + tbox 'TBOX((1.0,), (2.0,))';
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' + tbox 'TBOX((, 2000-01-01), (, 2000-01-02))';

SELECT tbox 'TBOX((1.0,), (2.0,))' + tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))';
SELECT tbox 'TBOX((1.0,), (2.0,))' + tbox 'TBOX((1.0,), (2.0,))';
SELECT tbox 'TBOX((1.0,), (2.0,))' + tbox 'TBOX((, 2000-01-01), (, 2000-01-02))';

SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))' + tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))';
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))' + tbox 'TBOX((1.0,), (2.0,))';
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))' + tbox 'TBOX((, 2000-01-01), (, 2000-01-02))';

SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' + tbox 'TBOX((11.0, 2000-01-01), (12.0, 2000-01-02))';
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' + tbox 'TBOX((1.0, 2000-02-01), (2.0, 2000-02-02))';

/* Errors */
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' + tbox 'TBOX((3.0, 2000-01-01), (4.0, 2000-01-02))';
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' + tbox 'TBOX((1.0, 2000-01-03), (2.0, 2000-01-04))';

-------------------------------------------------------------------------------

SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' * tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))';
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' * tbox 'TBOX((1.0,), (2.0,))';
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' * tbox 'TBOX((, 2000-01-01), (, 2000-01-02))';

SELECT tbox 'TBOX((1.0,), (2.0,))' * tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))';
SELECT tbox 'TBOX((1.0,), (2.0,))' * tbox 'TBOX((1.0,), (2.0,))';
SELECT tbox 'TBOX((1.0,), (2.0,))' * tbox 'TBOX((, 2000-01-01), (, 2000-01-02))';

SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))' * tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))';
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))' * tbox 'TBOX((1.0,), (2.0,))';
SELECT tbox 'TBOX((, 2000-01-01), (, 2000-01-02))' * tbox 'TBOX((, 2000-01-01), (, 2000-01-02))';

SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' * tbox 'TBOX((11.0, 2000-01-01), (12.0, 2000-01-02))';
SELECT tbox 'TBOX((1.0, 2000-01-01), (2.0, 2000-01-02))' * tbox 'TBOX((1.0, 2000-02-01), (2.0, 2000-02-02))';

-------------------------------------------------------------------------------

SELECT MAX(xmax(t1.b + t2.b)) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b && t2.b;
SELECT MAX(xmax(t1.b * t2.b)) FROM tbl_tbox t1, tbl_tbox t2;

-------------------------------------------------------------------------------
-- Extent aggregation
-------------------------------------------------------------------------------

WITH test(box) AS (
  SELECT NULL::tbox UNION ALL SELECT tbox 'TBOX((1, 2000-01-01),(2, 2000-01-02))' UNION ALL
  SELECT NULL::tbox UNION ALL SELECT tbox 'TBOX((1, 2000-01-01),(3, 2000-01-03))' )
SELECT extent(box) FROM test;

-- encourage use of parallel plans
set parallel_setup_cost=0;
set parallel_tuple_cost=0;
set min_parallel_table_scan_size=0;
set max_parallel_workers_per_gather=2;

SELECT round(extent(temp::tbox),6) FROM tbl_tfloat_big;

-- reset to default values
reset parallel_setup_cost;
reset parallel_tuple_cost;
reset min_parallel_table_scan_size;
reset max_parallel_workers_per_gather;

-------------------------------------------------------------------------------
-- Comparison functions
-------------------------------------------------------------------------------

SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((2.0, 2000-01-02), (1.0, 2000-01-02))');
SELECT tbox_cmp(tbox 'TBOX((2.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))');
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-03), (1.0, 2000-01-02))');
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-03), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))');
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (2.0, 2000-01-02))');
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (2.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))');
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-03))');
SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-03))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))');

SELECT tbox_cmp(tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))', tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))');
SELECT tbox_cmp('TBOX((1,),(2,))', 'TBOX((1,2001-01-01),(2,2001-01-02))');
SELECT tbox_cmp('TBOX((1,2001-01-01),(2,2001-01-02))', 'TBOX((1,),(2,))');

SELECT tbox 'TBOX((1.0, 2000-01-02), (1.0, 2000-01-02))' = floatrange '[1, 2]'::tbox;

-------------------------------------------------------------------------------

SELECT tbox_cmp(t1.b, t2.b), COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 GROUP BY tbox_cmp(t1.b, t2.b) ORDER BY 1;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b = t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b <> t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b < t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b <= t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b > t2.b;
SELECT COUNT(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b >= t2.b;

-------------------------------------------------------------------------------
