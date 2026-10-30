-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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

\copy tbl_tboxfloat TO 'tbl_tboxfloat' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tboxfloat_tmp;
CREATE TABLE tbl_tboxfloat_tmp AS TABLE tbl_tboxfloat WITH NO DATA;
\copy tbl_tboxfloat_tmp FROM 'tbl_tboxfloat' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat_tmp t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
DROP TABLE tbl_tboxfloat_tmp;

\copy tbl_tboxint TO 'tbl_tboxint' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tboxint_tmp;
CREATE TABLE tbl_tboxint_tmp AS TABLE tbl_tboxint WITH NO DATA;
\copy tbl_tboxint_tmp FROM 'tbl_tboxint' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint_tmp t2 WHERE t1.k = t2.k AND t1.b <> t2.b;
DROP TABLE tbl_tboxint_tmp;

-- Input/output from/to WKT, WKB and HexWKB

-- Maximum decimal digits
SELECT asText(tbox 'TBOXFLOAT XT([1.123456789,2.123456789],[2001-01-01,2001-01-02])', 6);

SELECT COUNT(*) FROM tbl_tboxint WHERE tboxFromBinary(asBinary(b)) <> b;
SELECT COUNT(*) FROM tbl_tboxint WHERE tboxFromHexWKB(asHexWKB(b)) <> b;

SELECT COUNT(*) FROM tbl_tboxfloat WHERE tboxFromBinary(asBinary(b)) <> b;
SELECT COUNT(*) FROM tbl_tboxfloat WHERE tboxFromHexWKB(asHexWKB(b)) <> b;

SELECT tboxFromText('TBOXINT XT([1,2],[2001-01-01,2001-01-02])');
SELECT COUNT(*) FROM tbl_tboxint WHERE tboxFromText(asText(b)) <> asText(b)::tbox;
SELECT COUNT(*) FROM tbl_tboxfloat WHERE tboxFromText(asText(b)) <> asText(b)::tbox;

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
/* Errors */
SELECT tbox(float 'NaN', timestamptz '2001-01-01');
SELECT tbox(float 'NaN', tstzspan '[2001-01-01,2001-01-02]');

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
SELECT float 'NaN'::tbox;
SELECT numeric 'NaN'::tbox;
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
SELECT tboxOverlaps(tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT tboxContains(tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT tboxContained(tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT tboxAdjacent(tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');
SELECT tboxSame(tbox 'TBOXFLOAT XT([1.0, 2.0],[2001-01-02, 2001-02-01])', tbox 'TBOXFLOAT XT([1.0,2.0],[2001-01-01,2001-01-02])');

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
-- Operators of a span and a temporal box
-------------------------------------------------------------------------------

-- A span answers the operators of a temporal box as its own box does: a number
-- span on the value dimension, a timestamptz span on the time dimension
SELECT intspan '[1,5)' && tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])' && intspan '[1,5)';
SELECT intspan '[1,5)' @> tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT intspan '[3,8)' @> tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])' <@ intspan '[3,8)';
SELECT intspan '[3,8)' ~= tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT intspan '[8,9)' -|- tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT intspan '[1,5)' &< tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT intspan '[1,5)' &> tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])' &> intspan '[1,5)';
SELECT intspan '[8,9)' >> tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])' << intspan '[8,9)';
SELECT bigintspan '[1,5)' && tbox 'TBOXBIGINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT floatspan '[1,5)' && tbox 'TBOXFLOAT XT([3,8),[2001-01-01,2001-01-02])';
SELECT tstzspan '[2001-01-01,2001-01-03]' && tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT tstzspan '[2001-01-01,2001-01-03]' @> tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])' <@ tstzspan '[2001-01-01,2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-03]' &<# tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT tstzspan '[2001-01-01,2001-01-03]' #&> tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])' &<# tstzspan '[2001-01-01,2001-01-03]';
SELECT tstzspan '[2001-01-05,2001-01-06]' #>> tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT tstzspan '(2001-01-02,2001-01-03]' -|- tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT floatspan '[8,9)' -|- tbox 'TBOXFLOAT XT([3,8),[2001-01-01,2001-01-02])';
-- The portable spelling of each operator answers the same
SELECT tboxOverlaps(intspan '[1,5)', tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])');
SELECT tboxOverbefore(tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])', tstzspan '[2001-01-01,2001-01-03]');
-- A span of another number than the box, or a box without the span's
-- dimension, is refused as two such boxes are
SELECT floatspan '[1,5)' && tbox 'TBOXINT XT([3,8),[2001-01-01,2001-01-02])';
SELECT intspan '[1,5)' && tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT intspan '[1,5)' << tbox 'TBOX T([2001-01-01,2001-01-02])';
SELECT tstzspan '[2001-01-01,2001-01-03]' <<# tbox 'TBOXINT X([3,8))';
-- Over the tables, each operator answers what it answers on the box of the
-- span, for pairs answering true and pairs answering false
SELECT COUNT(*) FILTER (WHERE (t1.i && t2.b) <> (t1.i::tbox && t2.b)),
  COUNT(*) FILTER (WHERE t1.i && t2.b), COUNT(*) FILTER (WHERE NOT (t1.i && t2.b))
FROM tbl_intspan t1, tbl_tboxint t2;
SELECT COUNT(*) FILTER (WHERE (t1.i @> t2.b) <> (t1.i::tbox @> t2.b)),
  COUNT(*) FILTER (WHERE t1.i @> t2.b), COUNT(*) FILTER (WHERE NOT (t1.i @> t2.b))
FROM tbl_intspan t1, tbl_tboxint t2;
SELECT COUNT(*) FILTER (WHERE (t1.i <@ t2.b) <> (t1.i::tbox <@ t2.b)),
  COUNT(*) FILTER (WHERE t1.i <@ t2.b), COUNT(*) FILTER (WHERE NOT (t1.i <@ t2.b))
FROM tbl_intspan t1, tbl_tboxint t2;
SELECT COUNT(*) FILTER (WHERE (t1.i ~= t2.b) <> (t1.i::tbox ~= t2.b)),
  COUNT(*) FILTER (WHERE t1.i ~= t2.b), COUNT(*) FILTER (WHERE NOT (t1.i ~= t2.b))
FROM tbl_intspan t1, tbl_tboxint t2;
SELECT COUNT(*) FILTER (WHERE (t1.i -|- t2.b) <> (t1.i::tbox -|- t2.b)),
  COUNT(*) FILTER (WHERE t1.i -|- t2.b), COUNT(*) FILTER (WHERE NOT (t1.i -|- t2.b))
FROM tbl_intspan t1, tbl_tboxint t2;
SELECT COUNT(*) FILTER (WHERE (t1.i << t2.b) <> (t1.i::tbox << t2.b)),
  COUNT(*) FILTER (WHERE t1.i << t2.b), COUNT(*) FILTER (WHERE NOT (t1.i << t2.b))
FROM tbl_intspan t1, tbl_tboxint t2;
SELECT COUNT(*) FILTER (WHERE (t1.i &< t2.b) <> (t1.i::tbox &< t2.b)),
  COUNT(*) FILTER (WHERE t1.i &< t2.b), COUNT(*) FILTER (WHERE NOT (t1.i &< t2.b))
FROM tbl_intspan t1, tbl_tboxint t2;
SELECT COUNT(*) FILTER (WHERE (t1.i >> t2.b) <> (t1.i::tbox >> t2.b)),
  COUNT(*) FILTER (WHERE t1.i >> t2.b), COUNT(*) FILTER (WHERE NOT (t1.i >> t2.b))
FROM tbl_intspan t1, tbl_tboxint t2;
SELECT COUNT(*) FILTER (WHERE (t1.i &> t2.b) <> (t1.i::tbox &> t2.b)),
  COUNT(*) FILTER (WHERE t1.i &> t2.b), COUNT(*) FILTER (WHERE NOT (t1.i &> t2.b))
FROM tbl_intspan t1, tbl_tboxint t2;
SELECT COUNT(*) FILTER (WHERE (t1.b && t2.f) <> (t1.b && t2.f::tbox)),
  COUNT(*) FILTER (WHERE t1.b && t2.f), COUNT(*) FILTER (WHERE NOT (t1.b && t2.f))
FROM tbl_tboxfloat t1, tbl_floatspan t2;
SELECT COUNT(*) FILTER (WHERE (t1.b @> t2.f) <> (t1.b @> t2.f::tbox)),
  COUNT(*) FILTER (WHERE t1.b @> t2.f), COUNT(*) FILTER (WHERE NOT (t1.b @> t2.f))
FROM tbl_tboxfloat t1, tbl_floatspan t2;
SELECT COUNT(*) FILTER (WHERE (t1.b <@ t2.f) <> (t1.b <@ t2.f::tbox)),
  COUNT(*) FILTER (WHERE t1.b <@ t2.f), COUNT(*) FILTER (WHERE NOT (t1.b <@ t2.f))
FROM tbl_tboxfloat t1, tbl_floatspan t2;
SELECT COUNT(*) FILTER (WHERE (t1.b ~= t2.f) <> (t1.b ~= t2.f::tbox)),
  COUNT(*) FILTER (WHERE t1.b ~= t2.f), COUNT(*) FILTER (WHERE NOT (t1.b ~= t2.f))
FROM tbl_tboxfloat t1, tbl_floatspan t2;
SELECT COUNT(*) FILTER (WHERE (t1.b -|- t2.f) <> (t1.b -|- t2.f::tbox)),
  COUNT(*) FILTER (WHERE t1.b -|- t2.f), COUNT(*) FILTER (WHERE NOT (t1.b -|- t2.f))
FROM tbl_tboxfloat t1, tbl_floatspan t2;
SELECT COUNT(*) FILTER (WHERE (t1.b << t2.f) <> (t1.b << t2.f::tbox)),
  COUNT(*) FILTER (WHERE t1.b << t2.f), COUNT(*) FILTER (WHERE NOT (t1.b << t2.f))
FROM tbl_tboxfloat t1, tbl_floatspan t2;
SELECT COUNT(*) FILTER (WHERE (t1.b &< t2.f) <> (t1.b &< t2.f::tbox)),
  COUNT(*) FILTER (WHERE t1.b &< t2.f), COUNT(*) FILTER (WHERE NOT (t1.b &< t2.f))
FROM tbl_tboxfloat t1, tbl_floatspan t2;
SELECT COUNT(*) FILTER (WHERE (t1.b >> t2.f) <> (t1.b >> t2.f::tbox)),
  COUNT(*) FILTER (WHERE t1.b >> t2.f), COUNT(*) FILTER (WHERE NOT (t1.b >> t2.f))
FROM tbl_tboxfloat t1, tbl_floatspan t2;
SELECT COUNT(*) FILTER (WHERE (t1.b &> t2.f) <> (t1.b &> t2.f::tbox)),
  COUNT(*) FILTER (WHERE t1.b &> t2.f), COUNT(*) FILTER (WHERE NOT (t1.b &> t2.f))
FROM tbl_tboxfloat t1, tbl_floatspan t2;
SELECT COUNT(*) FILTER (WHERE (t1.b::tstzspan && t2.b) <> (t1.b::tstzspan::tbox && t2.b)),
  COUNT(*) FILTER (WHERE t1.b::tstzspan && t2.b), COUNT(*) FILTER (WHERE NOT (t1.b::tstzspan && t2.b))
FROM tbl_tboxfloat t1, tbl_tboxfloat t2;
SELECT COUNT(*) FILTER (WHERE (t1.b::tstzspan @> t2.b) <> (t1.b::tstzspan::tbox @> t2.b)),
  COUNT(*) FILTER (WHERE t1.b::tstzspan @> t2.b), COUNT(*) FILTER (WHERE NOT (t1.b::tstzspan @> t2.b))
FROM tbl_tboxfloat t1, tbl_tboxfloat t2;
SELECT COUNT(*) FILTER (WHERE (t1.b::tstzspan <@ t2.b) <> (t1.b::tstzspan::tbox <@ t2.b)),
  COUNT(*) FILTER (WHERE t1.b::tstzspan <@ t2.b), COUNT(*) FILTER (WHERE NOT (t1.b::tstzspan <@ t2.b))
FROM tbl_tboxfloat t1, tbl_tboxfloat t2;
SELECT COUNT(*) FILTER (WHERE (t1.b::tstzspan ~= t2.b) <> (t1.b::tstzspan::tbox ~= t2.b)),
  COUNT(*) FILTER (WHERE t1.b::tstzspan ~= t2.b), COUNT(*) FILTER (WHERE NOT (t1.b::tstzspan ~= t2.b))
FROM tbl_tboxfloat t1, tbl_tboxfloat t2;
SELECT COUNT(*) FILTER (WHERE (t1.b::tstzspan -|- t2.b) <> (t1.b::tstzspan::tbox -|- t2.b)),
  COUNT(*) FILTER (WHERE t1.b::tstzspan -|- t2.b), COUNT(*) FILTER (WHERE NOT (t1.b::tstzspan -|- t2.b))
FROM tbl_tboxfloat t1, tbl_tboxfloat t2;
SELECT COUNT(*) FILTER (WHERE (t1.b::tstzspan <<# t2.b) <> (t1.b::tstzspan::tbox <<# t2.b)),
  COUNT(*) FILTER (WHERE t1.b::tstzspan <<# t2.b), COUNT(*) FILTER (WHERE NOT (t1.b::tstzspan <<# t2.b))
FROM tbl_tboxfloat t1, tbl_tboxfloat t2;
SELECT COUNT(*) FILTER (WHERE (t1.b::tstzspan &<# t2.b) <> (t1.b::tstzspan::tbox &<# t2.b)),
  COUNT(*) FILTER (WHERE t1.b::tstzspan &<# t2.b), COUNT(*) FILTER (WHERE NOT (t1.b::tstzspan &<# t2.b))
FROM tbl_tboxfloat t1, tbl_tboxfloat t2;
SELECT COUNT(*) FILTER (WHERE (t1.b::tstzspan #>> t2.b) <> (t1.b::tstzspan::tbox #>> t2.b)),
  COUNT(*) FILTER (WHERE t1.b::tstzspan #>> t2.b), COUNT(*) FILTER (WHERE NOT (t1.b::tstzspan #>> t2.b))
FROM tbl_tboxfloat t1, tbl_tboxfloat t2;
SELECT COUNT(*) FILTER (WHERE (t1.b::tstzspan #&> t2.b) <> (t1.b::tstzspan::tbox #&> t2.b)),
  COUNT(*) FILTER (WHERE t1.b::tstzspan #&> t2.b), COUNT(*) FILTER (WHERE NOT (t1.b::tstzspan #&> t2.b))
FROM tbl_tboxfloat t1, tbl_tboxfloat t2;
-- ... and the planner estimates them over a span column and a box column
SELECT COUNT(*) FROM tbl_intspan WHERE i && tbox 'TBOXINT X([10,20))';
SELECT COUNT(*) FROM tbl_tboxint WHERE b && intspan '[10,20)';
SELECT COUNT(*) FROM tbl_tstzspan WHERE t <<# tbox 'TBOX T([2001-06-01,2001-06-02])';
SELECT COUNT(*) FROM tbl_intspan t1, tbl_tboxint t2 WHERE t1.i && t2.b;

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

-- The span type states what a box measures, so an extent spanning two of them
-- states a quantity nothing reads, exactly as the + operator on the same pair
-- already answers
WITH test(box) AS (
  SELECT tbox 'TBOXINT XT([1,2],[2001-01-01,2001-01-02])' UNION ALL
  SELECT tbox 'TBOXFLOAT XT([3,4],[2001-01-03,2001-01-04])' )
SELECT extent(box) FROM test;

-- Both machinery functions of the aggregate answer on the same terms as the
-- operator. The combine is reached by a parallel plan alone, so it is called
-- directly here
SELECT extentTransition(tbox 'TBOXINT X([1,2])', tbox 'TBOXFLOAT X([3,4])');
SELECT extentCombine(tbox 'TBOXINT X([1,2])', tbox 'TBOXFLOAT X([3,4])');
SELECT extentTransition(tbox 'TBOXFLOAT X([1,2])', tbox 'TBOXFLOAT X([3,4])');
SELECT extentCombine(tbox 'TBOXFLOAT X([1,2])', tbox 'TBOXFLOAT X([3,4])');
-- A box carrying no value span states no span type, so it is comparable with
-- any box, and a null side leaves the other untouched
SELECT extentCombine(NULL, tbox 'TBOXFLOAT X([3,4])');
SELECT extentCombine(tbox 'TBOXFLOAT X([1,2])', NULL);
-- Which dimensions a box holds is the second question, asked after the first
SELECT extentTransition(tbox 'TBOXFLOAT X([1,2])',
  tbox 'TBOXFLOAT XT([3,4],[2001-01-03,2001-01-04])');
SELECT extentCombine(tbox 'TBOXFLOAT X([1,2])',
  tbox 'TBOXFLOAT XT([3,4],[2001-01-03,2001-01-04])');

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

-- Boxes of different dimensions or value types are ordered by them first
SELECT cmp(tbox 'TBOXFLOAT XT([0,0],[2001-01-02,2001-01-02])', tbox 'TBOXFLOAT X([1,1])');
SELECT cmp(tbox 'TBOXFLOAT X([1,1])', tbox 'TBOXFLOAT XT([2,2],[2001-01-01,2001-01-01])');
SELECT cmp(tbox 'TBOXFLOAT XT([0,0],[2001-01-02,2001-01-02])', tbox 'TBOXFLOAT XT([2,2],[2001-01-01,2001-01-01])');
SELECT cmp(tbox 'TBOXINT X([1,1])', tbox 'TBOXFLOAT X([1,1])');
SELECT tbox 'TBOXINT X([1,1])' = tbox 'TBOXFLOAT X([1,1])';
SELECT b FROM (VALUES
  (tbox 'TBOXFLOAT XT([0,0],[2001-01-02,2001-01-02])'),
  (tbox 'TBOXFLOAT X([1,1])'),
  (tbox 'TBOXFLOAT XT([2,2],[2001-01-01,2001-01-01])'),
  (tbox 'TBOX T([2001-01-01,2001-01-01])'),
  (tbox 'TBOXINT X([1,1])'),
  (tbox 'TBOXINT XT([0,0],[2001-01-03,2001-01-03])')) t(b)
ORDER BY b;

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
