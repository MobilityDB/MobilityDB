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
-- geometry rel tnpoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE tcontains(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE tdisjoint(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE tintersects(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT COUNT(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE ttouches(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE tdwithin(ST_SetSRID(t1.g, 5676), t2.temp, 0.01) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;

-------------------------------------------------------------------------------
-- npoint rel tnpoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE tdisjoint(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE tintersects(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE ttouches(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE tdwithin(t1.np, t2.temp, 0.01) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;

-------------------------------------------------------------------------------
-- tnpoint rel <Type>
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE tdisjoint(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE tintersects(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE ttouches(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE tdwithin(t1.temp, ST_SetSRID(t2.g, 5676), 0.01) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE tdisjoint(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE tintersects(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE ttouches(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE tdwithin(t1.temp, t2.np, 0.01) IS NOT NULL AND t1.k < 5 AND t2.k < 5;

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE tdwithin(t1.temp, t2.temp, 0.01) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;

-------------------------------------------------------------------------------
