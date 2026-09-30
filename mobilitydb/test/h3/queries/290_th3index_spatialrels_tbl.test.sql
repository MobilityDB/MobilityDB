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

-----------------------------------------------------------------------------
-- Bulk spatial-relationship tests over temporal H3 cells (th3index/th3index),
-- answered on the sphere over the temporal geography of the cell boundaries.
-- The geography-operand overloads are covered by the scalar test
-- 290_th3index_spatialrels.
-----------------------------------------------------------------------------

-----------------------------------------------------------------------------
-- Disjoint
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE eDisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE aDisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE tDisjoint(t1.temp, t2.temp) IS NOT NULL;

-----------------------------------------------------------------------------
-- Intersects
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE eIntersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE aIntersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE tIntersects(t1.temp, t2.temp) IS NOT NULL;

-----------------------------------------------------------------------------
-- Dwithin, in metres
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE eDwithin(t1.temp, t2.temp, 5000.0);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE aDwithin(t1.temp, t2.temp, 5000.0);

-----------------------------------------------------------------------------
