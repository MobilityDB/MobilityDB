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

SELECT round(MAX(maxValue(ST_SetSRID(g, 3812) <-> temp)), 6) FROM tbl_geom_point t1, tbl_tcbuffer t2
WHERE ST_SetSRID(g, 3812) <-> temp IS NOT NULL;
SELECT round(MAX(maxValue(temp <-> ST_SetSRID(g, 3812))), 6) FROM tbl_tcbuffer t1, tbl_geom_point t2
WHERE temp <-> ST_SetSRID(g, 3812) IS NOT NULL;
SELECT round(MAX(maxValue(t1.temp <-> t2.temp)), 6) FROM tbl_tcbuffer t1, tbl_tcbuffer t2
WHERE t1.temp <-> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tcbuffer,
( SELECT * FROM tbl_geom LIMIT 10 ) t
WHERE NearestApproachInstant(temp, ST_SetSRID(g, 3812)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1,
( SELECT * FROM tbl_tcbuffer t2 LIMIT 10 ) t2
WHERE NearestApproachInstant(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tcbuffer,
( SELECT * FROM tbl_geom LIMIT 10 ) t
WHERE NearestApproachDistance(temp, ST_SetSRID(g, 3812)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1,
( SELECT * FROM tbl_tcbuffer t2 LIMIT 10 ) t2
WHERE NearestApproachDistance(t1.temp, t2.temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tcbuffer,
( SELECT * FROM tbl_geom LIMIT 10 ) t
WHERE ST_SetSRID(g, 3812) |=| temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1,
( SELECT * FROM tbl_tcbuffer t2 LIMIT 10 ) t2
WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_tcbuffer,
( SELECT * FROM tbl_geom LIMIT 10 ) t
WHERE shortestLine(ST_SetSRID(g, 3812), temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1,
( SELECT * FROM tbl_tcbuffer t2 LIMIT 10 ) t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL;

--------------------------------------------------------

-- set parallel_tuple_cost=100;
-- set parallel_setup_cost=100;

-------------------------------------------------------------------------------
