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
-- Send/receive functions
-------------------------------------------------------------------------------

COPY tbl_npoint TO '/tmp/tbl_npoint' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_npoint_tmp;
CREATE TABLE tbl_npoint_tmp AS TABLE tbl_npoint WITH NO DATA;
COPY tbl_npoint_tmp FROM '/tmp/tbl_npoint' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint_tmp t2 WHERE t1.k = t2.k AND t1.np <> t2.np;
DROP TABLE tbl_npoint_tmp;

COPY tbl_nsegment TO '/tmp/tbl_nsegment' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_nsegment_tmp;
CREATE TABLE tbl_nsegment_tmp AS TABLE tbl_nsegment WITH NO DATA;
COPY tbl_nsegment_tmp FROM '/tmp/tbl_nsegment' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment_tmp t2 WHERE t1.k = t2.k AND t1.ns <> t2.ns;
DROP TABLE tbl_nsegment_tmp;

-------------------------------------------------------------------------------
-- Accessing values
-------------------------------------------------------------------------------

SELECT MAX(route(np)) FROM tbl_npoint;
SELECT MAX(getPosition(np)) FROM tbl_npoint;

SELECT MAX(route(ns)) FROM tbl_nsegment;
SELECT MAX(startPosition(ns)) FROM tbl_nsegment;
SELECT MAX(endPosition(ns)) FROM tbl_nsegment;

-------------------------------------------------------------------------------
-- Cast functions between network and space
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_npoint WHERE np::geometry IS NOT NULL;
SELECT COUNT(*) FROM tbl_nsegment WHERE ns::geometry IS NOT NULL;

SELECT COUNT(*) FROM tbl_npoint WHERE np = (np::geometry)::npoint;
SELECT COUNT(*) FROM tbl_nsegment WHERE ns = (ns::geometry)::nsegment;

SELECT COUNT(*) FROM tbl_geom_point WHERE CASE WHEN NOT ST_IsEmpty(g) THEN ST_SetSRID(g, 5676)::npoint IS NOT NULL END;
SELECT COUNT(*) FROM tbl_geom_linestring WHERE CASE WHEN NOT ST_IsEmpty(g) THEN ST_SetSRID(g, 5676)::nsegment IS NOT NULL END;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np = t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np != t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np < t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np <= t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np > t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np >= t2.np;

SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns = t2.ns;
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns != t2.ns;
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns < t2.ns;
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns <= t2.ns;
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns > t2.ns;
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns >= t2.ns;

-------------------------------------------------------------------------------/
