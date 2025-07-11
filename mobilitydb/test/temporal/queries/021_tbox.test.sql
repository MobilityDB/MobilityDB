-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2000-2022, PostGIS contributors
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

SELECT tbox 'TBOXINT XT([1, 1],[2000-01-01,2000-01-02])'; -- Both X and T dimensions
SELECT tbox 'TBOXFLOAT XT([1.0, 1.0],[2000-01-01,2000-01-02])'; -- Both X and T dimensions
SELECT tbox 'TBOXINT X([1, 1])'; -- Only X dimension
SELECT tbox 'TBOXFLOAT X([1.0, 1.0])'; -- Only X dimension
SELECT tbox 'TBOX T([2000-01-01,2000-01-02])'; -- Only T dimension
SELECT tbox 'TBOXINT XT([1,2][2000-01-01,2000-01-02])'; -- Optional comma

/* Errors */
SELECT tbox 'XXX(1, 2000-01-02)';
SELECT tbox 'TBOX(1, 2000-01-02)';
SELECT tbox 'TBOX A(1, 2000-01-02)';
SELECT tbox 'TBOXFLOAT X(1, 2000-01-02)';
SELECT tbox 'TBOXFLOAT XA(1, 2000-01-02)';
SELECT tbox 'TBOXFLOAT X((,))';
SELECT tbox 'TBOXFLOAT X((AA, 2))';
SELECT tbox 'TBOXFLOAT X((1, AA))';
SELECT tbox 'TBOXFLOAT X((1, 2000-01-01))';
SELECT tbox 'TBOXFLOAT X((1, 2), 2, 2))';
SELECT tbox 'TBOXFLOAT X((1, 2),(AA, 2))';
SELECT tbox 'TBOXFLOAT X((1, 2),(2000-01-01, AA))';
SELECT tbox 'TBOXFLOAT X((1, 2),(2000-01-01, 2000-01-02)';
SELECT tbox 'TBOXFLOAT X((2,2000-01-02),(1,2000-01-01))XXXX';

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
SELECT asText(tbox 'TBOXFLOAT XT([1.123456789,2.123456789],[2000-01-01,2000-01-02])', 6);

SELECT COUNT(*) FROM tbl_tboxint WHERE tboxFromBinary(asBinary(b)) <> b;
SELECT COUNT(*) FROM tbl_tboxint WHERE tboxFromHexWKB(asHexWKB(b)) <> b;

SELECT COUNT(*) FROM tbl_tboxfloat WHERE tboxFromBinary(asBinary(b)) <> b;
SELECT COUNT(*) FROM tbl_tboxfloat WHERE tboxFromHexWKB(asHexWKB(b)) <> b;

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT tbox(floatspan '[1,2]', timestamptz '2000-01-01');
SELECT tbox(floatspan '[1,2]', tstzspan '[2000-01-01,2000-01-02]');
SELECT tbox(floatspan '[1,2]');
SELECT tbox(timestamptz '2000-01-01');
SELECT tbox(tstzspan '[2000-01-01,2000-01-02]');

-------------------------------------------------------------------------------
-- Conversions
-------------------------------------------------------------------------------

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])'::floatspan;
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])'::tstzspan;
SELECT tbox 'TBOXFLOAT X([1.0, 2.0])'::floatspan;
SELECT tbox 'TBOX T((2000-01-01,2000-01-02))'::tstzspan;

SELECT 1::tbox;
SELECT 1.5::tbox;
SELECT intset '{1,2}'::tbox;
SELECT floatset '{1,2}'::tbox;
SELECT tstzset '{2000-01-01,2000-01-02}'::tbox;
SELECT intspan '[1,2]'::tbox;
SELECT floatspan '[1,2]'::tbox;
SELECT tstzspan '[2000-01-01,2000-01-02]'::tbox;
SELECT intspanset '{[1,2]}'::tbox;
SELECT floatspanset '{[1,2]}'::tbox;
SELECT tstzspanset '{[2000-01-01,2000-01-02]}'::tbox;
/* Errors */
SELECT tbox 'TBOX T((2000-01-01,2000-01-02))'::floatspan;
SELECT tbox 'TBOXFLOAT X([1.0, 2.0])'::tstzspan;

SELECT tbox 'TBOXINT XT([1,2),[2000-01-01, 2000-01-02))'::intspan;
SELECT tbox 'TBOXINT XT([1,2),[2000-01-01, 2000-01-02))'::floatspan;
SELECT tbox 'TBOXFLOAT XT([1,2),[2000-01-01, 2000-01-02))'::intspan;
SELECT tbox 'TBOXFLOAT XT([1,2),[2000-01-01, 2000-01-02))'::floatspan;
/* Errors */
SELECT tbox 'TBOXINT T([2000-01-01, 2000-01-02))'::intspan;
SELECT tbox 'TBOXINT T([2000-01-01, 2000-01-02))'::floatspan;

-------------------------------------------------------------------------------

SELECT MIN(Xmin(temp::tbox)) FROM tbl_tint;
SELECT round(MIN(Xmin(temp::tbox)),6) FROM tbl_tfloat;

-------------------------------------------------------------------------------

SELECT ROUND(MAX(upper(b::floatspan) - lower(b::floatspan)), 6) FROM tbl_tboxfloat;
SELECT MAX(duration(b::tstzspan)) FROM tbl_tboxint;

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT hasX(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])');
SELECT hasX(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT hasX(tbox 'TBOX T([2000-01-01,2000-01-02])');

SELECT hasT(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])');
SELECT hasT(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT hasT(tbox 'TBOX T([2000-01-01,2000-01-02])');

SELECT Xmin(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])');
SELECT XminInc(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])');
SELECT Xmax(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])');
SELECT XmaxInc(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])');
SELECT Tmin(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])');
SELECT TminInc(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])');
SELECT Tmax(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])');
SELECT TmaxInc(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])');

SELECT Xmin(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT Xmax(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT Tmin(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT TminInc(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT Tmax(tbox 'TBOXFLOAT X([1.0, 2.0])');
SELECT TmaxInc(tbox 'TBOXFLOAT X([1.0, 2.0])');

SELECT Xmin(tbox 'TBOX T([2000-01-01,2000-01-02])');
SELECT XminInc(tbox 'TBOX T([2000-01-01,2000-01-02])');
SELECT Xmax(tbox 'TBOX T([2000-01-01,2000-01-02])');
SELECT XmaxInc(tbox 'TBOX T([2000-01-01,2000-01-02])');
SELECT Tmin(tbox 'TBOX T([2000-01-01,2000-01-02])');
SELECT Tmax(tbox 'TBOX T([2000-01-01,2000-01-02])');

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

SELECT shiftValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', 1.0);
SELECT shiftValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', -1.0);

SELECT shiftTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', '1 day');
SELECT shiftTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', '-1 day');

SELECT scaleValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', 2.0);
/* Errors */
SELECT scaleValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', -1.0);

SELECT scaleTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', '1 day');
SELECT scaleTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', '1 hour');
/* Errors */
SELECT scaleTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', '-1 hour');

SELECT shiftScaleValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', 1.0, 2.0);
SELECT shiftScaleValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', -1.0, 2.0);

SELECT shiftScaleTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', '1 day', '1 hour');
SELECT shiftScaleTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', '-1 day', '1 hour');

SELECT expandValue(tbox 'TBOXINT XT([1,2],[2000-01-01,2000-01-02])', 2);
SELECT expandValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', 2.0);
SELECT expandTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', interval '1 day');
SELECT expandTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', interval '-12 hours');
-- NULL
SELECT expandValue(tbox 'TBOXINT XT([1,2],[2000-01-01,2000-01-02])', -1);
SELECT expandValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', -1);
SELECT expandValue(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', -1.0);
SELECT expandTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02))', interval '-12 hours');
SELECT expandTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', interval '-1 day');
SELECT expandTime(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', interval '-2 days');
/* Errors */
SELECT expandValue(tbox 'TBOXINT XT([1,2],[2000-01-01,2000-01-02])', -1.0);
SELECT expandValue(tbox 'TBOX T([2000-01-01,2000-01-02])', 2);
SELECT expandTime(tbox 'TBOXFLOAT X([1,2])', interval '1 day');

SELECT round(tbox 'TBOXFLOAT XT([1.123456789,2.123456789],[2000-01-01,2000-01-02])', 2);
SELECT round(tbox 'TBOX T([2000-01-01,2000-01-02])', 2);

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' && tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0, 2.0],[2000-01-02, 2000-02-01])' @> tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0, 2.0],[2000-01-02, 2000-02-01])' <@ tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0, 2.0],[2000-01-02, 2000-02-01])' -|- tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0, 2.0],[2000-01-02, 2000-02-01])' ~= tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';

SELECT tstzspan '[2000-01-01,2000-01-02]'::tbox -|- tstzspan '[2000-01-02, 2000-01-03]'::tbox;

/* Errors */
SELECT tbox 'TBOXFLOAT X([1,2])' && tbox 'TBOX T([2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' @> tbox 'TBOX T([2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' <@ tbox 'TBOX T([2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' -|- tbox 'TBOX T([2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' ~= tbox 'TBOX T([2000-01-01,2000-01-02])';

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

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' << tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' &< tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' >> tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' &> tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' <<# tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' &<# tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' #>> tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' #&> tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';

/* Errors */
SELECT tbox 'TBOXFLOAT X([1,2])' << tbox 'TBOX T([2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' &< tbox 'TBOX T([2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' >> tbox 'TBOX T([2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' &> tbox 'TBOX T([2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' <<# tbox 'TBOX T([2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' &<# tbox 'TBOX T([2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' #>> tbox 'TBOX T([2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' #&> tbox 'TBOX T([2000-01-01,2000-01-02])';

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

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' + tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' + tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' + tbox 'TBOX T([2000-01-01,2000-01-02])';

SELECT tbox 'TBOXFLOAT X([1,2])' + tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' + tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOXFLOAT X([1,2])' + tbox 'TBOX T([2000-01-01,2000-01-02])';

SELECT tbox 'TBOX T([2000-01-01,2000-01-02])' + tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOX T([2000-01-01,2000-01-02])' + tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOX T([2000-01-01,2000-01-02])' + tbox 'TBOX T([2000-01-01,2000-01-02])';

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' + tbox 'TBOXFLOAT XT([11.0,12.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' + tbox 'TBOXFLOAT XT([1.0, 2.0],[2000-02-01,2000-02-02])';

/* Errors */
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' + tbox 'TBOXFLOAT XT([3.0,4.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' + tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-03, 2000-01-04])';

-------------------------------------------------------------------------------

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' * tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' * tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' * tbox 'TBOX T([2000-01-01,2000-01-02])';

SELECT tbox 'TBOXFLOAT X([1,2])' * tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT X([1,2])' * tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOXFLOAT X([1,2])' * tbox 'TBOX T([2000-01-01,2000-01-02])';

SELECT tbox 'TBOX T([2000-01-01,2000-01-02])' * tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOX T([2000-01-01,2000-01-02])' * tbox 'TBOXFLOAT X([1,2])';
SELECT tbox 'TBOX T([2000-01-01,2000-01-02])' * tbox 'TBOX T([2000-01-01,2000-01-02])';

SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' * tbox 'TBOXFLOAT XT([11.0,12.0],[2000-01-01,2000-01-02])';
SELECT tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])' * tbox 'TBOXFLOAT XT([1.0,2.0],[2000-02-01,2000-02-02])';

-------------------------------------------------------------------------------

SELECT MAX(xmax(t1.b + t2.b)) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b && t2.b;
SELECT MAX(xmax(t1.b * t2.b)) FROM tbl_tboxint t1, tbl_tboxint t2;

SELECT MAX(xmax(t1.b + t2.b)) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b && t2.b;
SELECT MAX(xmax(t1.b * t2.b)) FROM tbl_tboxfloat t1, tbl_tboxfloat t2;

-------------------------------------------------------------------------------
-- Extent aggregation
-------------------------------------------------------------------------------

WITH test(box) AS (
  SELECT NULL::tbox UNION ALL SELECT tbox 'TBOXFLOAT XT([1,2],[2000-01-01,2000-01-02])' UNION ALL
  SELECT NULL::tbox UNION ALL SELECT tbox 'TBOXFLOAT XT([1,3],[2000-01-01,2000-01-03])' )
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

SELECT tbox_cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-02])', tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-02, 2000-01-02])');
SELECT tbox_cmp(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-02, 2000-01-02])', tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-02])');
SELECT tbox_cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-02])', tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-03])');
SELECT tbox_cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-03])', tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-02])');
SELECT tbox_cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-02])', tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-02, 2000-01-02])');
SELECT tbox_cmp(tbox 'TBOXFLOAT XT([1.0,2.0],[2000-01-02, 2000-01-02])', tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-02])');
SELECT tbox_cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-02])', tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-03])');
SELECT tbox_cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-03])', tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-02])');

SELECT tbox_cmp(tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-02])', tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-02])');
SELECT tbox_cmp('TBOXFLOAT X([1,2])', 'TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])');
SELECT tbox_cmp('TBOXFLOAT XT([1.0,2.0],[2000-01-01,2000-01-02])', 'TBOXFLOAT X([1,2])');

SELECT tbox 'TBOXFLOAT XT([1.0,1.0],[2000-01-02,2000-01-02])' = floatspan '[1, 2]'::tbox;

-------------------------------------------------------------------------------

SELECT tbox_cmp(t1.b, t2.b), COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 GROUP BY tbox_cmp(t1.b, t2.b) ORDER BY 1;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b = t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b <> t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b < t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b <= t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b > t2.b;
SELECT COUNT(*) FROM tbl_tboxint t1, tbl_tboxint t2 WHERE t1.b >= t2.b;

SELECT tbox_cmp(t1.b, t2.b), COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 GROUP BY tbox_cmp(t1.b, t2.b) ORDER BY 1;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b = t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b <> t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b < t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b <= t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b > t2.b;
SELECT COUNT(*) FROM tbl_tboxfloat t1, tbl_tboxfloat t2 WHERE t1.b >= t2.b;

-------------------------------------------------------------------------------
