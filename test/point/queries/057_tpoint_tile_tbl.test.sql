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
-- Multidimensional tiling
-------------------------------------------------------------------------------

SELECT multidimGrid(b, 2.5, geometry 'Point(10 10)'), COUNT(*) FROM tbl_stbox GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT multidimGrid(b, 2.5, interval '1 week'), COUNT(*) FROM tbl_stbox GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT multidimGrid(b, 2.5, interval '1 week', 'Point(10 10)', '2001-06-01'), COUNT(*) FROM tbl_stbox GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

-- 2D
SELECT extent(multidimTile(g, 2.5)) FROM
(SELECT * FROM tbl_geom_point WHERE g IS NOT NULL LIMIT 10) t1;
SELECT extent(multidimTile(g, 2.5, geometry 'Point(10 10)')) FROM
(SELECT * FROM tbl_geom_point WHERE g IS NOT NULL LIMIT 10) t1;
-- 3D
SELECT extent(multidimTile(g, 2.5)) FROM
(SELECT * FROM tbl_geom_point3D WHERE g IS NOT NULL LIMIT 10) t1;
SELECT extent(multidimTile(g, 2.5, geometry 'Point(10 10 10)')) FROM
(SELECT * FROM tbl_geom_point3D WHERE g IS NOT NULL LIMIT 10) t1;

-- 2D
SELECT extent(multidimTile(g, t, 2.5, interval '2 days')) FROM
(SELECT * FROM tbl_geom_point WHERE g IS NOT NULL LIMIT 10 OFFSET 10) t1,
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10) t2;
SELECT extent(multidimTile(g, t, 2.5, interval '2 days', geometry 'Point(10 10)', '2001-06-01')) FROM
(SELECT * FROM tbl_geom_point WHERE g IS NOT NULL LIMIT 10 OFFSET 10) t1,
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10) t2;
-- 3D
SELECT extent(multidimTile(g, t, 2.5, interval '2 days')) FROM
(SELECT * FROM tbl_geom_point3D WHERE g IS NOT NULL LIMIT 10 OFFSET 10) t1,
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10) t2;
SELECT extent(multidimTile(g, t, 2.5, interval '2 days', geometry 'Point(10 10 10)', '2001-06-01')) FROM
(SELECT * FROM tbl_geom_point3D WHERE g IS NOT NULL LIMIT 10 OFFSET 10) t1,
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL LIMIT 10) t2;

-------------------------------------------------------------------------------


