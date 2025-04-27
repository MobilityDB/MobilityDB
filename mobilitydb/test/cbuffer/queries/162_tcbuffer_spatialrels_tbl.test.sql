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

-------------------------------------------------------------------------------
-- eContains, aContains
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE aContains(g, temp);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE eContains(temp, g);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE aContains(temp, g);

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE eContains(cb, temp);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE aContains(cb, temp);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE eContains(temp, cb);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE aContains(temp, cb);

-------------------------------------------------------------------------------
-- eCovers, aCovers
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE aCovers(g, temp);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE eCovers(temp, g);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE aCovers(temp, g);

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE eCovers(cb, temp);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE aCovers(cb, temp);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE eCovers(temp, cb);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE aCovers(temp, cb);

-------------------------------------------------------------------------------
-- eDisjoint, aDisjoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE eDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE aDisjoint(g, temp);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE eDisjoint(temp, g);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE aDisjoint(temp, g);

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE eDisjoint(cb, temp);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE aDisjoint(cb, temp);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE eDisjoint(temp, cb);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE aDisjoint(temp, cb);

SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE eDisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE aDisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eIntersects, aIntersects
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE eIntersects(g, temp);
SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE aIntersects(g, temp);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE eIntersects(temp, g);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE aIntersects(temp, g);

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE eIntersects(cb, temp);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE aIntersects(cb, temp);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE eIntersects(temp, cb);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE aIntersects(temp, cb);

SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE eIntersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE aIntersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- eTouches, aTouches
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE eTouches(g, temp);
SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE aTouches(g, temp);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE eTouches(temp, g);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE aTouches(temp, g);

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE eTouches(cb, temp);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE aTouches(cb, temp);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE eTouches(temp, cb);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE aTouches(temp, cb);

-------------------------------------------------------------------------------
-- eDwithin, aDwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE eDwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE aDwithin(g, temp, 10);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE eDwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE aDwithin(temp, g, 10);

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE eDwithin(cb, temp, 10);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE aDwithin(cb, temp, 10);

SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE eDwithin(temp, cb, 10);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE aDwithin(temp, cb, 10);

SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE eDwithin(t1.temp, t2.temp, 10);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE aDwithin(t1.temp, t2.temp, 10);

-- Step interpolation

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer_seq WHERE eDwithin(cb, seq, 10);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer_seqset WHERE eDwithin(cb, ss, 10);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer_seq WHERE aDwithin(cb, seq, 10);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer_seqset WHERE aDwithin(cb, ss, 10);

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer_seq WHERE eDwithin(seq, cb, 10);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer_seqset WHERE eDwithin(ss, cb, 10);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer_seq WHERE aDwithin(seq, cb, 10);
SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer_seqset WHERE aDwithin(ss, cb, 10);

SELECT COUNT(*) FROM tbl_tcbuffer_seq t1, tbl_tcbuffer t2 WHERE eDwithin(t1.seq, t2.temp, 10);
SELECT COUNT(*) FROM tbl_tcbuffer_seqset t1, tbl_tcbuffer t2 WHERE eDwithin(t1.ss, t2.temp, 10);
SELECT COUNT(*) FROM tbl_tcbuffer_seq t1, tbl_tcbuffer t2 WHERE aDwithin(t1.seq, t2.temp, 10);
SELECT COUNT(*) FROM tbl_tcbuffer_seqset t1, tbl_tcbuffer t2 WHERE aDwithin(t1.ss, t2.temp, 10);

-------------------------------------------------------------------------------
