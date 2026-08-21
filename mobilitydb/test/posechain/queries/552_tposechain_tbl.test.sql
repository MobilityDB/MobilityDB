-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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

-------------------------------------------------------------------------------
-- Send and receive
-------------------------------------------------------------------------------

COPY tbl_tposechain TO '/tmp/tbl_tposechain' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tposechain_tmp;
CREATE TABLE tbl_tposechain_tmp AS TABLE tbl_tposechain WITH NO DATA;
COPY tbl_tposechain_tmp FROM '/tmp/tbl_tposechain' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_tposechain_tmp;

-------------------------------------------------------------------------------
-- Representation round trips
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tposechain WHERE tposechainFromEWKT(asEWKT(temp)) <> temp;
SELECT COUNT(*) FROM tbl_tposechain WHERE tposechainFromHexEWKB(asHexEWKB(temp)) <> temp;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT MIN(numLinks(temp)), MAX(numLinks(temp)) FROM tbl_tposechain;
SELECT DISTINCT tempSubtype(temp) FROM tbl_tposechain ORDER BY 1;
SELECT DISTINCT SRID(temp) FROM tbl_tposechain;
SELECT MIN(numInstants(temp)), MAX(numInstants(temp)) FROM tbl_tposechain;
-- Every instant of a value holds the same number of links
SELECT COUNT(*) FROM tbl_tposechain WHERE numPoses(startValue(temp)) <> numPoses(endValue(temp));

-------------------------------------------------------------------------------
-- Conversions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tposechain WHERE stbox(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tposechain WHERE tpose(temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Restrictions and transformations
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tposechain WHERE atTime(temp, timeSpan(temp)) <> temp;
SELECT COUNT(*) FROM tbl_tposechain WHERE round(temp, 6) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tposechain WHERE shiftTime(temp, interval '1 day') <> temp;

-------------------------------------------------------------------------------
-- Spatial reference system: only the outer link names a frame
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tposechain WHERE SRID(setSRID(temp, 3812)) <> 3812;

-------------------------------------------------------------------------------
