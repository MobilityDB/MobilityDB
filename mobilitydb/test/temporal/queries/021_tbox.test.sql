-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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

SELECT tbox 'TBOXINT XT([1, 1],[2001-01-01,2001-01-02])'; -- Both X and T dimensions
SELECT tbox 'TBOXBIGINT XT([1, 1],[2001-01-01,2001-01-02])'; -- Both X and T dimensions
SELECT tbox 'TBOXFLOAT XT([1.0, 1.0],[2001-01-01,2001-01-02])'; -- Both X and T dimensions
SELECT tbox 'TBOXINT X([1, 1])'; -- Only X dimension
SELECT tbox 'TBOXBIGINT X([1, 1])'; -- Only X dimension
SELECT tbox 'TBOXFLOAT X([1.0, 1.0])'; -- Only X dimension
SELECT tbox 'TBOX T([2001-01-01,2001-01-02])'; -- Only T dimension
SELECT tbox 'TBOXINT XT([1,2][2001-01-01,2001-01-02])'; -- Optional comma

/* Errors */
SELECT tbox 'XXX(1, 2001-01-02)';
SELECT tbox 'TBOX(1, 2001-01-02)';
SELECT tbox 'TBOX A(1, 2001-01-02)';
SELECT tbox 'TBOXFLOAT X(1, 2001-01-02)';
SELECT tbox 'TBOXFLOAT XA(1, 2001-01-02)';
SELECT tbox 'TBOXFLOAT X((,))';
SELECT tbox 'TBOXFLOAT X((AA, 2))';
SELECT tbox 'TBOXFLOAT X((1, AA))';
SELECT tbox 'TBOXFLOAT X((1, 2001-01-01))';
SELECT tbox 'TBOXFLOAT X((1, 2), 2, 2))';
SELECT tbox 'TBOXFLOAT X((1, 2),(AA, 2))';
SELECT tbox 'TBOXFLOAT X((1, 2),(2001-01-01, AA))';
SELECT tbox 'TBOXFLOAT X((1, 2),(2001-01-01, 2001-01-02)';
SELECT tbox 'TBOXFLOAT X((2,2001-01-02),(1,2001-01-01))XXXX';

-- Send/receive functions

COPY tbl_tboxfloat TO '/tmp/tbl_tboxfloat' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tboxfloat_tmp;
CREATE TABLE tbl_tboxfloat_tmp AS TABLE tbl_tboxfloat WITH NO DATA;
COPY tbl_tboxfloat_tmp FROM '/tmp/tbl_tboxfloat' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat_tmp t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
DROP TABLE tbl_tboxfloat_tmp;

COPY tbl_tboxint TO '/tmp/tbl_tboxint' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tboxint_tmp;
CREATE TABLE tbl_tboxint_tmp AS TABLE tbl_tboxint WITH NO DATA;
COPY tbl_tboxint_tmp FROM '/tmp/tbl_tboxint' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint_tmp t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
DROP TABLE tbl_tboxint_tmp;

-- Input/output from/to WKT, WKB and HexWKB

-- Maximum decimal digits
SELECT asText(tbox 'TBOXFLOAT XT([1.123456789,2.123456789],[2001-01-01,2001-01-02])', 6);

SELECT COUNT(*) FROM tbl_tboxint WHERE tboxFromBinary(asBinary(b)) <> b;
SELECT COUNT(*) FROM tbl_tboxint WHERE tboxFromHexWKB(asHexWKB(b)) <> b;

SELECT COUNT(*) FROM tbl_tboxfloat WHERE tboxFromBinary(asBinary(b)) <> b;
SELECT COUNT(*) FROM tbl_tboxfloat WHERE tboxFromHexWKB(asHexWKB(b)) <> b;

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT tbox(floatspan '[1,2]', timestamptz '2001-01-01');
SELECT tbox(floatspan '[1,2]', tstzspan '[2001-01-01,2001-01-02]');
SELECT tbox(floatspan '[1,2]');
SELECT tbox(timestamptz '2001-01-01');
SELECT tbox(tstzspan '[2001-01-01,2001-01-02]');

SELECT tbox(bigintspan '[1,2]', timestamptz '2001-01-01');
SELECT tbox(bigintspan '[1,2]', tstzspan '[2001-01-01,2001-01-02]');
SELECT tbox(bigintspan '[1,2]');

-------------------------------------------------------------------------------
-- Conversions
-------------------------------------------------------------------------------

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])'::floatspan;
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])'::tstzspan;
SELECT tbox 'TBOXFLOAT X([1.0, 2.0])'::floatspan;
SELECT tbox 'TBOX T((2001-01-01,2001-01-02))'::tstzspan;

SELECT 1::tbox;
SELECT 1::bigint::tbox;
SELECT 1.5::tbox;
SELECT intset '{1,2}'::tbox;
SELECT bigintset '{1,2}'::tbox;
SELECT floatset '{1,2}'::tbox;
SELECT tstzset '{2001-01-01,2001-01-02}'::tbox;
SELECT intspan '[1,2]'::tbox;
SELECT bigintspan '[1,2]'::tbox;
SELECT floatspan '[1,2]'::tbox;
SELECT tstzspan '[2001-01-01,2001-01-02]'::tbox;
SELECT intspanset '{[1,2]}'::tbox;
SELECT bigintspanset '{[1,2]}'::tbox;
SELECT floatspanset '{[1,2]}'::tbox;
SELECT tstzspanset '{[2001-01-01,2001-01-02]}'::tbox;
/* Errors */
SELECT tbox 'TBOX T((2001-01-01,2001-01-02))'::floatspan;
SELECT tbox 'TBOXFLOAT X([1.0, 2.0])'::tstzspan;

SELECT tbox 'TBOXINT XT([1,2),[2001-01-01, 2001-01-02))'::intspan;
SELECT tbox 'TBOXINT XT([1,2),[2001-01-01, 2001-01-02))'::bigintspan;
SELECT tbox 'TBOXINT XT([1,2),[2001-01-01, 2001-01-02))'::floatspan;
SELECT tbox 'TBOXBIGINT XT([1,2),[2001-01-01, 2001-01-02))'::intspan;
SELECT tbox 'TBOXBIGINT XT([1,2),[2001-01-01, 2001-01-02))'::bigintspan;
SELECT tbox 'TBOXBIGINT XT([1,2),[2001-01-01, 2001-01-02))'::floatspan;
SELECT tbox 'TBOXFLOAT XT([1,2),[2001-01-01, 2001-01-02))'::intspan;
SELECT tbox 'TBOXFLOAT XT([1,2),[2001-01-01, 2001-01-02))'::bigintspan;
SELECT tbox 'TBOXFLOAT XT([1,2),[2001-01-01, 2001-01-02))'::floatspan;
/* Errors */
SELECT tbox 'TBOXINT T([2001-01-01, 2001-01-02))'::intspan;
SELECT tbox 'TBOXINT T([2001-01-01, 2001-01-02))'::floatspan;
SELECT tbox 'TBOXBIGINT T([2001-01-01, 2001-01-02))'::bigintspan;

-------------------------------------------------------------------------------

SELECT MIN(Xmin(temp::tbox)) FROM tbl_tint;
SELECT round(MIN(Xmin(temp::tbox)),6) FROM tbl_tfloat;

-------------------------------------------------------------------------------

SELECT ROUND(MAX(upper(b::floatspan) - lower(b::floatspan)), 6) FROM tbl_tboxfloat;
SELECT MAX(duration(b::tstzspan)) FROM tbl_tboxint;

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT hasX(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT hasX(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT hasX(tbox 'TBOX T([2001-01-01,2001-01-02])');

SELECT hasT(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT hasT(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT hasT(tbox 'TBOX T([2001-01-01,2001-01-02])');

SELECT Xmin(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT XminInc(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT Xmax(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT XmaxInc(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT Tmin(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT TminInc(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT Tmax(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT TmaxInc(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');

SELECT Xmin(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT Xmax(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT Tmin(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT TminInc(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT Tmax(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT TmaxInc(tbox 'TBOXFLOAT X([1.0, 2.0])');

SELECT Xmin(tbox 'TBOX T([2001-01-01,2001-01-02])');
SELECT XminInc(tbox 'TBOX T([2001-01-01,2001-01-02])');
SELECT Xmax(tbox 'TBOX T([2001-01-01,2001-01-02])');
SELECT XmaxInc(tbox 'TBOX T([2001-01-01,2001-01-02])');
SELECT Tmin(tbox 'TBOX T([2001-01-01,2001-01-02])');
SELECT Tmax(tbox 'TBOX T([2001-01-01,2001-01-02])');

SELECT Xmin(tbox 'TBOXBIGINT XT([1,2],[2001-01-01,2001-01-02])');
SELECT Xmax(tbox 'TBOXBIGINT XT([1,2],[2001-01-01,2001-01-02])');
-- A big integer bound above 2^53 has no exact float, so the double accessor
-- refuses it rather than answering a neighbouring value; bigintspan reads it
SELECT Xmin(tbox 'TBOXBIGINT XT([9007199254740993,9007199254740995],[2001-01-01,2001-01-02])');
SELECT Xmax(tbox 'TBOXBIGINT XT([9007199254740993,9007199254740995],[2001-01-01,2001-01-02])');
SELECT lower(bigintspan(tbox 'TBOXBIGINT XT([9007199254740993,9007199254740995],[2001-01-01,2001-01-02])'));
SELECT upper(bigintspan(tbox 'TBOXBIGINT XT([9007199254740993,9007199254740995],[2001-01-01,2001-01-02])'));
SELECT Tmin(tbox 'TBOXBIGINT XT([1,2],[2001-01-01,2001-01-02])');
SELECT Tmax(tbox 'TBOXBIGINT XT([1,2],[2001-01-01,2001-01-02])');

-------------------------------------------------------------------------------

SELECT MIN(xmin(b)) FROM tbl_tboxint;
SELECT MAX(xmax(b)) FROM tbl_tboxint;
SELECT MIN(tmin(b)) FROM tbl_tboxint;
SELECT MAX(tmax(b)) FROM tbl_tboxint;

SELECT MIN(xmin(b)) FROM tbl_tboxfloat;
SELECT MAX(xmax(b)) FROM tbl_tboxfloat;
SELECT MIN(tmin(b)) FROM tbl_tboxfloat;
SELECT MAX(tmax(b)) FROM tbl_tboxfloat;

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT round(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', 1);

SELECT shiftValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', 1.0);
SELECT shiftValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', -1.0);

SELECT shiftTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', '1 day');
SELECT shiftTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', '-1 day');

SELECT scaleValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', 2.0);
/* Errors */
SELECT scaleValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', -1.0);

SELECT scaleTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', '1 day');
SELECT scaleTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', '1 hour');
/* Errors */
SELECT scaleTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', '-1 hour');

SELECT shiftScaleValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', 1.0, 2.0);
SELECT shiftScaleValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', -1.0, 2.0);

SELECT shiftScaleTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', '1 day', '1 hour');
SELECT shiftScaleTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', '-1 day', '1 hour');
/* Errors */
SELECT shiftScaleTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', interval '-1 day', interval '-1 day');

SELECT shiftValue(tbox 'TBOXBIGINT XT([1,2],[2001-01-01,2001-01-02])', 1::bigint);
SELECT shiftValue(tbox 'TBOXBIGINT XT([1,2],[2001-01-01,2001-01-02])', -1::bigint);
SELECT scaleValue(tbox 'TBOXBIGINT XT([1,3],[2001-01-01,2001-01-02])', 4::bigint);
SELECT shiftScaleValue(tbox 'TBOXBIGINT XT([1,3],[2001-01-01,2001-01-02])', 1::bigint, 4::bigint);

SELECT expandValue(tbox 'TBOXINT XT([1,2],[2001-01-01,2001-01-02])', 2);
SELECT expandValue(tbox 'TBOXBIGINT XT([1,2],[2001-01-01,2001-01-02])', 2::bigint);
SELECT expandValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', 2.0);
-- A float box takes an integer value of either width, converting it to the
-- double the box stores. A binary integer/float dispatch reads the bigint Datum
-- as a double instead, which reinterprets its bits
SELECT expandValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', 2::bigint);
-- The two integer widths do not mix, and each message names the value it got
SELECT expandValue(tbox 'TBOXINT XT([1,2],[2001-01-01,2001-01-02])', 2::bigint);
SELECT expandValue(tbox 'TBOXBIGINT XT([1,2],[2001-01-01,2001-01-02])', 2);
SELECT expandTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', interval '1 day');
SELECT expandTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', interval '-12 hours');
-- NULL
SELECT expandValue(tbox 'TBOXINT XT([1,2],[2001-01-01,2001-01-02])', -1);
SELECT expandValue(tbox 'TBOXBIGINT XT([1,4],[2001-01-01,2001-01-02])', -1::bigint);
SELECT expandValue(tbox 'TBOXBIGINT XT([1,2],[2001-01-01,2001-01-02])', -1::bigint);
SELECT expandValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', -1);
SELECT expandValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', -1.0);
SELECT expandTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02))', interval '-12 hours');
SELECT expandTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', interval '-1 day');
SELECT expandTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', interval '-2 days');
/* Errors */
SELECT round(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', -1);
SELECT expandValue(tbox 'TBOXINT XT([1,2],[2001-01-01,2001-01-02])', -1.0);
SELECT expandValue(tbox 'TBOX T([2001-01-01,2001-01-02])', 2);
SELECT expandTime(tbox 'TBOXFLOAT X([1,2])', interval '1 day');

SELECT round(tbox 'TBOXFLOAT XT([1.123456789,2.123456789],[2001-01-01,2001-01-02])', 2);
SELECT round(tbox 'TBOX T([2001-01-01,2001-01-02])', 2);

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' && tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])' @> tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])' <@ tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])' -|- tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])' ~= tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';

SELECT tstzspan '[2001-01-01,2001-01-02]'::tbox -|- tstzspan '[2001-01-02, 2001-01-03]'::tbox;

-- Adjacency reads every dimension the boxes share: a value bound they share
-- while their periods lie apart leaves them apart
SELECT tbox 'TBOXFLOAT XT([1,5],[2001-01-01,2001-01-05])' -|- tbox 'TBOXFLOAT XT([5,9],[2001-06-01,2001-06-05])';
SELECT tbox 'TBOXFLOAT XT([1,5],[2001-01-01,2001-01-05])' -|- tbox 'TBOXFLOAT XT([5,9],[2001-01-05,2001-01-09])';
SELECT tbox 'TBOXFLOAT XT([1,5],[2001-01-01,2001-01-09])' -|- tbox 'TBOXFLOAT XT([5,9],[2001-01-05,2001-01-12])';
SELECT tbox 'TBOXFLOAT X([1,5])' -|- tbox 'TBOXFLOAT X([5,9])';
SELECT tbox 'TBOXFLOAT X([1,5])' -|- tbox 'TBOXFLOAT X([3,9])';

-- The portable spelling of each operator above answers the same
SELECT overlaps(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT contains(tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT contained(tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT adjacent(tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT same(tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');

/* Errors */
SELECT tbox 'TBOXFLOAT X([1,2])' && tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' @> tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' <@ tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' -|- tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' ~= tbox 'TBOX T([2001-01-01,2001-01-02])';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b && t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b @> t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b <@ t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b -|- t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b ~= t2.b;

SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b && t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b @> t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b <@ t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b -|- t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b ~= t2.b;

-------------------------------------------------------------------------------
-- Position operators
-------------------------------------------------------------------------------

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' << tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' &< tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' >> tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' &> tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' <<# tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' &<# tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' #>> tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' #&> tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';

/* Errors */
SELECT tbox 'TBOXFLOAT X([1,2])' << tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' &< tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' >> tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' &> tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' <<# tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' &<# tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' #>> tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' #&> tbox 'TBOX T([2001-01-01,2001-01-02])';

SELECT tbox 'TBOXINT X([1,2])' << tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOXINT X([1,2])' &< tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOXINT X([1,2])' >> tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOXINT X([1,2])' &> tbox 'TBOXFLOAT X([1,2])';

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b << t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b &< t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b >> t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b &> t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b <<# t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b &<# t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b #>> t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b #&> t2.b;

SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b << t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b &< t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b >> t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b &> t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b <<# t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b &<# t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b #>> t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b #&> t2.b;

-------------------------------------------------------------------------------
-- Set operators
-------------------------------------------------------------------------------

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' + tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' + tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' + tbox 'TBOX T([2001-01-01,2001-01-02])';

SELECT tbox 'TBOXFLOAT X([1,2])' + tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' + tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOXFLOAT X([1,2])' + tbox 'TBOX T([2001-01-01,2001-01-02])';

SELECT tbox 'TBOX T([2001-01-01,2001-01-02])' + tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOX T([2001-01-01,2001-01-02])' + tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOX T([2001-01-01,2001-01-02])' + tbox 'TBOX T([2001-01-01,2001-01-02])';

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' + tbox 'TBOXFLOAT XT([11.0,12.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' + tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-02-01,2001-02-02])';

/* Errors */
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' + tbox 'TBOXFLOAT X([3.0,4.0])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' + tbox 'TBOXFLOAT XT([3.0,4.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' + tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-03, 2001-01-04])';

-------------------------------------------------------------------------------

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' * tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' * tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' * tbox 'TBOX T([2001-01-01,2001-01-02])';

SELECT tbox 'TBOXFLOAT X([1,2])' * tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' * tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOXFLOAT X([1,2])' * tbox 'TBOX T([2001-01-01,2001-01-02])';

SELECT tbox 'TBOX T([2001-01-01,2001-01-02])' * tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOX T([2001-01-01,2001-01-02])' * tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOX T([2001-01-01,2001-01-02])' * tbox 'TBOX T([2001-01-01,2001-01-02])';

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' * tbox 'TBOXFLOAT XT([11.0,12.0],[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])' * tbox 'TBOXFLOAT XT([1.0,2.0],[2001-02-01,2001-02-02])';

-------------------------------------------------------------------------------

SELECT MAX(xmax(t1.b + t2.b)) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b && t2.b;
SELECT MAX(xmax(t1.b * t2.b)) FROM tbl_tboxint t1, tbl_tboxint t2;

SELECT MAX(xmax(t1.b + t2.b)) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b && t2.b;
SELECT MAX(xmax(t1.b * t2.b)) FROM tbl_tboxfloat t1, tbl_tboxfloat t2;

-------------------------------------------------------------------------------
-- Extent aggregation
-------------------------------------------------------------------------------

WITH test(box) AS (
  SELECT NULL::tbox UNION ALL SELECT tbox 'TBOXFLOAT XT([1,2],[2001-01-01,2001-01-02])' UNION ALL
  SELECT NULL::tbox UNION ALL SELECT tbox 'TBOXFLOAT XT([1,3],[2001-01-01,2001-01-03])' )
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

SELECT cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-02, 2001-01-02])');
SELECT cmp(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-02, 2001-01-02])', tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])');
SELECT cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])', tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-03])');
SELECT cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-03])', tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])');
SELECT cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-02, 2001-01-02])');
SELECT cmp(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-02, 2001-01-02])', tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])');
SELECT cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])', tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-03])');
SELECT cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-03])', tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])');

SELECT cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])', tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])');
SELECT cmp(tbox 'TBOXFLOAT X([1,2])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT cmp(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', tbox 'TBOXFLOAT X([1,2])');

SELECT tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])' = floatspan '[1, 2]'::tbox;

-------------------------------------------------------------------------------

SELECT cmp(t1.b, t2.b), COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 GROUP BY cmp(t1.b, t2.b) ORDER BY 1;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b = t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b <> t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b < t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b <= t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b > t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b >= t2.b;

SELECT cmp(t1.b, t2.b), COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 GROUP BY cmp(t1.b, t2.b) ORDER BY 1;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b = t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b <> t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b < t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b <= t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b > t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b >= t2.b;

-------------------------------------------------------------------------------

SELECT hash(tbox 'TBOXFLOAT X([1.0,1.0])');
SELECT hashExtended(tbox 'TBOXFLOAT X([1.0,1.0])', 1);

SELECT hash(tbox 'TBOXFLOAT T([2001-01-02,2001-01-02])');
SELECT hashExtended(tbox 'TBOXFLOAT T([2001-01-02,2001-01-02])', 1);

SELECT hash(tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])');
SELECT hashExtended(tbox 'TBOXFLOAT XT([1.0,1.0],[2001-01-02,2001-01-02])', 1);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Nearest approach ordering answered by the indexes
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tboxfloat_rtree_idx;
CREATE INDEX tbl_tboxfloat_rtree_idx ON tbl_tboxfloat USING GIST(b);
WITH test AS (
  SELECT b |=| tbox 'TBOXFLOAT XT([1.0,2.0],[2001-06-01, 2001-07-01])' AS distance
  FROM tbl_tboxfloat ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
DROP INDEX tbl_tboxfloat_rtree_idx;

DROP INDEX IF EXISTS tbl_tboxfloat_quadtree_idx;
CREATE INDEX tbl_tboxfloat_quadtree_idx ON tbl_tboxfloat USING SPGIST(b);
WITH test AS (
  SELECT b |=| tbox 'TBOXFLOAT XT([1.0,2.0],[2001-06-01, 2001-07-01])' AS distance
  FROM tbl_tboxfloat ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
DROP INDEX tbl_tboxfloat_quadtree_idx;

DROP INDEX IF EXISTS tbl_tboxfloat_kdtree_idx;
CREATE INDEX tbl_tboxfloat_kdtree_idx ON tbl_tboxfloat USING SPGIST(b tbox_kdtree_ops);
WITH test AS (
  SELECT b |=| tbox 'TBOXFLOAT XT([1.0,2.0],[2001-06-01, 2001-07-01])' AS distance
  FROM tbl_tboxfloat ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
DROP INDEX tbl_tboxfloat_kdtree_idx;

-------------------------------------------------------------------------------
