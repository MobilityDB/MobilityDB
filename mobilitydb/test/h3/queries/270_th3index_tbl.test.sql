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

-- Table-based tests for the th3index type, exercised over tbl_th3index.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

COPY tbl_th3index TO '/tmp/tbl_th3index' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_th3index_tmp;
CREATE TABLE tbl_th3index_tmp AS TABLE tbl_th3index WITH NO DATA;
COPY tbl_th3index_tmp FROM '/tmp/tbl_th3index' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_th3index_tmp;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT SUM(numInstants(temp)) FROM tbl_th3index;
SELECT COUNT(*) FROM tbl_th3index WHERE startValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE endValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE getTime(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE duration(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE startInstant(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE endInstant(temp) IS NOT NULL;
SELECT SUM(numTimestamps(temp)) FROM tbl_th3index;
SELECT COUNT(*) FROM tbl_th3index WHERE memSize(temp) > 0;

-------------------------------------------------------------------------------
-- Restrictions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index WHERE atTime(temp, getTime(temp)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_th3index WHERE minusTime(temp, getTime(temp)) IS NULL;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp = t2.temp;
SELECT COUNT(*) FROM tbl_th3index t1, tbl_th3index t2 WHERE t1.temp <> t2.temp;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- MF-JSON output of the temporal H3 cells built from real AIS data, that is,
-- the cells covering the recorded vessel positions
-- Every query below reports the rows whose output is not valid JSON, and must
-- thus report zero
-------------------------------------------------------------------------------

WITH ais(mmsi, temp) AS (
  SELECT mmsi,
    th3indexSeq(array_agg(th3index(geoToH3Cell(geom, 10), t) ORDER BY t))
  FROM tbl_ais_instant GROUP BY mmsi )
SELECT count(*) FROM ais WHERE asMFJSON(temp)::jsonb IS NULL;

-------------------------------------------------------------------------------
