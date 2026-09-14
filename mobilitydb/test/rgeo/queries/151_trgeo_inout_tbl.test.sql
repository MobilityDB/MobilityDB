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

SELECT asText(round(temp, 6)) FROM tbl_trgeometry2d LIMIT 10;

SELECT asEWKT(round(temp, 6)) FROM tbl_trgeometry2d LIMIT 10;

SELECT asEWKT(array_agg(round(inst, 6) ORDER BY k)) FROM tbl_trgeometry2d_inst WHERE inst IS NOT NULL AND k % 10 = 1;

-------------------------------------------------------------------------------
-- Combination of input/output functions
-- We need to add asText/asEWKT to avoid problems due to floating point precision
-- The binary and MFJSON formats does not output the SRID

SELECT DISTINCT asText(trgeometryFromText(asText(temp))) = asText(temp) FROM tbl_trgeometry2d;

SELECT DISTINCT asEWKT(trgeometryFromEWKT(asEWKT(temp))) = asEWKT(temp) FROM tbl_trgeometry2d;

SELECT DISTINCT asText(trgeometryFromMFJSON(asMFJSON(temp))) = asText(temp) FROM tbl_trgeometry2d;

SELECT DISTINCT setSRID(trgeometryFromBinary(asBinary(temp)), 5676) = temp FROM tbl_trgeometry2d;

SELECT DISTINCT trgeometryFromEWKB(asEWKB(temp)) = temp FROM tbl_trgeometry2d;

SELECT DISTINCT trgeometryFromHexEWKB(asHexEWKB(temp)) = temp FROM tbl_trgeometry2d;

-------------------------------------------------------------------------------
-- MF-JSON output of the temporal rigid geometry values coming from real AIS
-- data, that is, a vessel outline moved and rotated along the recorded
-- positions. The reference geometry and the poses keep the full precision of
-- the source data, which is what exposes the output corruption reported at
-- https://github.com/MobilityDB/MobilityDB/issues/850
-- Every query below reports the rows whose output is not valid JSON or does
-- not read back with the same shape, and must thus report zero
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_ais_trgeometry WHERE asMFJSON(temp)::jsonb IS NULL;
SELECT count(*) FROM tbl_ais_trgeometry, generate_series(0, 15) AS d
  WHERE asMFJSON(temp, 1, 0, d)::jsonb IS NULL;

SELECT count(*) FROM tbl_ais_trgeometry
  WHERE trgeometryFromMFJSON(asMFJSON(temp)) IS NULL;

-------------------------------------------------------------------------------
