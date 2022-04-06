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

ANALYZE tbl_tgeompoint;
DROP INDEX IF EXISTS tbl_tgeompoint_spgist_idx;
CREATE INDEX tbl_tgeompoint_spgist_idx ON tbl_tgeompoint USING SPGIST(temp);

SELECT k, temp |=| geometry 'Point empty' FROM tbl_tgeompoint ORDER BY 2, 1 LIMIT 3;
SELECT k, round((temp |=| tgeompoint '[Point(1 1)@2001-06-01, Point(2 2)@2001-07-01]')::numeric, 6) FROM tbl_tgeompoint ORDER BY 2, 1 LIMIT 3;
SELECT k, round((temp |=| tgeompoint '[Point(-1 -1 -1)@2001-06-01, Point(-2 -2 -2)@2001-07-01]')::numeric, 6) FROM tbl_tgeompoint3D ORDER BY 2, 1 LIMIT 3;

DROP INDEX tbl_tgeompoint_spgist_idx;

-------------------------------------------------------------------------------

ANALYZE tbl_tgeompoint3D;
DROP INDEX IF EXISTS tbl_tgeompoint3D_spgist_idx;
CREATE INDEX tbl_tgeompoint3D_spgist_idx ON tbl_tgeompoint3D USING SPGIST(temp);

SELECT k, round((temp |=| tgeompoint '[Point(1 1 1)@2001-06-01, Point(2 2 2)@2001-07-01]')::numeric, 6) FROM tbl_tgeompoint3D ORDER BY 2, 1 LIMIT 3;

DROP INDEX tbl_tgeompoint3D_spgist_idx;

-------------------------------------------------------------------------------
-- Coverage of all the same logic in SP-GiST indexes

CREATE TABLE tbl_tgeompoint3D_big_allthesame AS SELECT k, tgeompoint_seq(geometry 'Point(5 5 5)', p) AS temp FROM tbl_period_big;
CREATE INDEX tbl_tgeompoint3D_big_allthesame_spgist_idx ON tbl_tgeompoint3D_big_allthesame USING SPGIST(temp);
ANALYZE tbl_tgeompoint3D_big_allthesame;

SELECT COUNT(*) FROM tbl_tgeompoint3D_big_allthesame WHERE temp && geometry 'Point(5 5 5)';

DROP TABLE tbl_tgeompoint3D_big_allthesame;

-------------------------------------------------------------------------------
