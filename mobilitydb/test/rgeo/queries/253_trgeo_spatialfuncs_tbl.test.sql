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
-- traversedArea
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE traversedArea(temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- atGeometry / minusGeometry
-- tbl_trgeometry2d uses SRID 5676; align tbl_geometry with SetSRID
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_geometry t2
WHERE atGeometry(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;

SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_geometry t2
WHERE minusGeometry(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL;

-- at and minus are complements: merge(at,minus) should equal original
-- (where both pieces are non-NULL and the result is well-defined)
SELECT COUNT(*) > 0 FROM tbl_trgeometry2d t1, tbl_geometry t2
WHERE t1.k % 3 = 0
  AND atGeometry(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL
  AND minusGeometry(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL
  AND t1.temp != merge(atGeometry(t1.temp, ST_SetSRID(t2.g, 5676)),
                       minusGeometry(t1.temp, ST_SetSRID(t2.g, 5676)));

-------------------------------------------------------------------------------
-- atStbox / minusStbox
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_stbox t2
WHERE atStbox(t1.temp, SetSRID(t2.b, 5676)) IS NOT NULL;

SELECT COUNT(*) FROM tbl_trgeometry2d t1, tbl_stbox t2
WHERE minusStbox(t1.temp, SetSRID(t2.b, 5676)) IS NOT NULL;

-------------------------------------------------------------------------------
-- convexHull
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE convexHull(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d_seq WHERE convexHull(seq) IS NOT NULL;

-------------------------------------------------------------------------------
-- centroid
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_trgeometry2d WHERE centroid(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_trgeometry2d_seq WHERE centroid(seq) IS NOT NULL;

-------------------------------------------------------------------------------
