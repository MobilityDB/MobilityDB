-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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

-- Round-trip the temporal box, a finite subset of the time x value product
-- domain, through the Arrow C Data Interface. The result must equal the
-- input for every flag-gated combination of the value and time dimensions.

SELECT arrowRoundtrip(tbox 'TBOXINT XT([1,10),[2000-01-01,2000-01-05])');
SELECT arrowRoundtrip(tbox 'TBOXFLOAT XT([1.5,9.75],[2000-01-01,2000-12-31))');
SELECT arrowRoundtrip(tbox 'TBOXINT X([1,10))');
SELECT arrowRoundtrip(tbox 'TBOXFLOAT X([1.5,9.75])');
SELECT arrowRoundtrip(tbox 'TBOX T([2000-01-01,2000-01-05])');
SELECT arrowRoundtrip(tbox 'TBOX T([2000-01-01 08:00:00+00,2000-12-31 23:59:59+00))');
SELECT arrowRoundtrip(tbox 'TBOXINT XT([-5,5],[2000-06-15,2001-06-15])');

-- Value identity over the flag-gated temporal-box surface

SELECT arrowRoundtrip(tbox 'TBOXINT XT([1,10),[2000-01-01,2000-01-05])') = tbox 'TBOXINT XT([1,10),[2000-01-01,2000-01-05])';
SELECT arrowRoundtrip(tbox 'TBOXFLOAT XT([1.5,9.75],[2000-01-01,2000-12-31))') = tbox 'TBOXFLOAT XT([1.5,9.75],[2000-01-01,2000-12-31))';
SELECT arrowRoundtrip(tbox 'TBOXINT X([1,10))') = tbox 'TBOXINT X([1,10))';
SELECT arrowRoundtrip(tbox 'TBOXFLOAT X([1.5,9.75])') = tbox 'TBOXFLOAT X([1.5,9.75])';
SELECT arrowRoundtrip(tbox 'TBOX T([2000-01-01,2000-01-05])') = tbox 'TBOX T([2000-01-01,2000-01-05])';
SELECT arrowRoundtrip(tbox 'TBOX T([2000-01-01 08:00:00+00,2000-12-31 23:59:59+00))') = tbox 'TBOX T([2000-01-01 08:00:00+00,2000-12-31 23:59:59+00))';
SELECT arrowRoundtrip(tbox 'TBOXINT XT([-5,5],[2000-06-15,2001-06-15])') = tbox 'TBOXINT XT([-5,5],[2000-06-15,2001-06-15])';

-------------------------------------------------------------------------------
