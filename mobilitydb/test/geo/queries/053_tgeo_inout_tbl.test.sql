  -------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2024, PostGIS contributors
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

SELECT asText(round(temp, 6)) FROM tbl_tgeometry LIMIT 10;
SELECT asText(round(temp, 6)) FROM tbl_tgeography LIMIT 10;
SELECT asText(round(temp, 6)) FROM tbl_tgeometry3D LIMIT 10;
SELECT asText(round(temp, 6)) FROM tbl_tgeography3D LIMIT 10;
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT asText(array_agg(round(g, 6) ORDER BY k)) FROM tbl_geography3D WHERE g IS NOT NULL AND k % 10 = 1;
SELECT asText(array_agg(round(temp, 6) ORDER BY k)) FROM tbl_tgeography3D WHERE temp IS NOT NULL AND k % 10 = 1;

SELECT asEWKT(round(temp, 6)) FROM tbl_tgeometry LIMIT 10;
SELECT asEWKT(round(temp, 6)) FROM tbl_tgeography LIMIT 10;
SELECT asEWKT(round(temp, 6)) FROM tbl_tgeometry3D LIMIT 10;
SELECT asEWKT(round(temp, 6)) FROM tbl_tgeography3D LIMIT 10;
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT asEWKT(array_agg(round(g, 6) ORDER BY k)) FROM tbl_geography3D WHERE g IS NOT NULL AND k % 10 = 1;
SELECT asEWKT(array_agg(round(temp, 6) ORDER BY k)) FROM tbl_tgeography3D WHERE temp IS NOT NULL AND k % 10 = 1;

SELECT asEWKT(array_agg(round(inst, 6) ORDER BY k)) FROM tbl_tgeometry_inst WHERE inst IS NOT NULL AND k % 10 = 1;

-------------------------------------------------------------------------------
-- Combination of input/output functions
-- We need to add asText/asEWKT to avoid problems due to floating point precision
-- The binary and MFJSON formats does not output the SRID

SELECT DISTINCT asText(tgeometryFromText(asText(temp))) = asText(temp) FROM tbl_tgeometry;
SELECT DISTINCT asText(tgeographyFromText(asText(temp))) = asText(temp) FROM tbl_tgeography;

SELECT DISTINCT asEWKT(tgeometryFromEWKT(asEWKT(temp))) = asEWKT(temp) FROM tbl_tgeometry;
SELECT DISTINCT asEWKT(tgeographyFromEWKT(asEWKT(temp))) = asEWKT(temp) FROM tbl_tgeography;

SELECT DISTINCT asText(tgeometryFromMFJSON(asMFJSON(temp))) = asText(temp) FROM tbl_tgeometry;
SELECT DISTINCT asText(tgeographyFromMFJSON(asMFJSON(temp))) = asText(temp) FROM tbl_tgeography;

SELECT DISTINCT setSRID(tgeometryFromBinary(asBinary(temp)), 3812) = temp FROM tbl_tgeometry;
SELECT DISTINCT setSRID(tgeographyFromBinary(asBinary(temp)), 7844) = temp FROM tbl_tgeography;

SELECT DISTINCT tgeometryFromEWKB(asEWKB(temp)) = temp FROM tbl_tgeometry;
SELECT DISTINCT tgeographyFromEWKB(asEWKB(temp)) = temp FROM tbl_tgeography;

SELECT DISTINCT tgeometryFromHexEWKB(asHexEWKB(temp)) = temp FROM tbl_tgeometry;
SELECT DISTINCT tgeographyFromHexEWKB(asHexEWKB(temp)) = temp FROM tbl_tgeography;

-------------------------------------------------------------------------------
