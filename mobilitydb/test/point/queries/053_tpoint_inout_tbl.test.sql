-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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

SELECT asText(round(temp, 6)) FROM tbl_tgeompoint LIMIT 10;
SELECT asText(round(temp, 6)) FROM tbl_tgeogpoint LIMIT 10;
SELECT asText(round(temp, 6)) FROM tbl_tgeompoint3D LIMIT 10;
SELECT asText(round(temp, 6)) FROM tbl_tgeogpoint3D LIMIT 10;
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT asText(array_agg(round(g, 6) ORDER BY k)) FROM tbl_geography3D WHERE g IS NOT NULL AND k % 10 = 1;
SELECT asText(array_agg(round(temp, 6) ORDER BY k)) FROM tbl_tgeogpoint3D WHERE temp IS NOT NULL AND k % 10 = 1;

SELECT asEWKT(round(temp, 6)) FROM tbl_tgeompoint LIMIT 10;
SELECT asEWKT(round(temp, 6)) FROM tbl_tgeogpoint LIMIT 10;
SELECT asEWKT(round(temp, 6)) FROM tbl_tgeompoint3D LIMIT 10;
SELECT asEWKT(round(temp, 6)) FROM tbl_tgeogpoint3D LIMIT 10;
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT asEWKT(array_agg(round(g, 6) ORDER BY k)) FROM tbl_geography3D WHERE g IS NOT NULL AND k % 10 = 1;
SELECT asEWKT(array_agg(round(temp, 6) ORDER BY k)) FROM tbl_tgeogpoint3D WHERE temp IS NOT NULL AND k % 10 = 1;

SELECT asEWKT(array_agg(round(inst, 6) ORDER BY k)) FROM tbl_tgeompoint_inst WHERE inst IS NOT NULL AND k % 10 = 1;

-------------------------------------------------------------------------------
-- Combination of input/output functions

-- We need to add asewkt to avoid problems due to floating point precision
SELECT DISTINCT asText(tgeompointFromText(asText(temp))) = asText(temp) FROM tbl_tgeompoint;
SELECT DISTINCT asText(tgeogpointFromText(asText(temp))) = asText(temp) FROM tbl_tgeogpoint;

-- We need to add asewkt to avoid problems due to floating point precision
SELECT DISTINCT asEWKT(tgeompointFromEWKT(asEWKT(temp))) = asEWKT(temp) FROM tbl_tgeompoint;
SELECT DISTINCT asEWKT(tgeogpointFromEWKT(asEWKT(temp))) = asEWKT(temp) FROM tbl_tgeogpoint;

-- We need to add asewkt to avoid problems due to floating point precision
SELECT DISTINCT asEWKT(tgeompointFromMFJSON(asMFJSON(temp))) = asEWKT(temp) FROM tbl_tgeompoint;
SELECT DISTINCT asEWKT(tgeogpointFromMFJSON(asMFJSON(temp))) = asEWKT(temp) FROM tbl_tgeogpoint;

SELECT DISTINCT tgeompointFromBinary(asBinary(temp)) = temp FROM tbl_tgeompoint;
SELECT DISTINCT tgeogpointFromBinary(asBinary(temp)) = temp FROM tbl_tgeogpoint;

SELECT DISTINCT tgeompointFromEWKB(asEWKB(temp)) = temp FROM tbl_tgeompoint;
SELECT DISTINCT tgeogpointFromEWKB(asEWKB(temp)) = temp FROM tbl_tgeogpoint;

SELECT DISTINCT tgeompointFromHexEWKB(asHexEWKB(temp)) = temp FROM tbl_tgeompoint;
SELECT DISTINCT tgeogpointFromHexEWKB(asHexEWKB(temp)) = temp FROM tbl_tgeogpoint;

-------------------------------------------------------------------------------
