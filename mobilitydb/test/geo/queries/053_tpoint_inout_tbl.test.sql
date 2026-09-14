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

SELECT asText(round(temp, 6)) FROM tbl_tgeompoint LIMIT 10;
SELECT asText(round(temp, 6)) FROM tbl_tgeogpoint LIMIT 10;
SELECT asText(round(temp, 6)) FROM tbl_tgeompoint3D LIMIT 10;
SELECT asText(round(temp, 6)) FROM tbl_tgeogpoint3D LIMIT 10;
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT asText(array_agg(round(g, 6) ORDER BY k)) FROM tbl_geog3D WHERE g IS NOT NULL AND k % 10 = 1;
SELECT asText(array_agg(round(temp, 6) ORDER BY k)) FROM tbl_tgeogpoint3D WHERE temp IS NOT NULL AND k % 10 = 1;

SELECT asEWKT(round(temp, 6)) FROM tbl_tgeompoint LIMIT 10;
SELECT asEWKT(round(temp, 6)) FROM tbl_tgeogpoint LIMIT 10;
SELECT asEWKT(round(temp, 6)) FROM tbl_tgeompoint3D LIMIT 10;
SELECT asEWKT(round(temp, 6)) FROM tbl_tgeogpoint3D LIMIT 10;
-- PostGIS 3.3 changed the output of MULTIPOINT
-- SELECT asEWKT(array_agg(round(g, 6) ORDER BY k)) FROM tbl_geog3D WHERE g IS NOT NULL AND k % 10 = 1;
SELECT asEWKT(array_agg(round(temp, 6) ORDER BY k)) FROM tbl_tgeogpoint3D WHERE temp IS NOT NULL AND k % 10 = 1;

SELECT asEWKT(array_agg(round(inst, 6) ORDER BY k)) FROM tbl_tgeompoint_inst WHERE inst IS NOT NULL AND k % 10 = 1;

-------------------------------------------------------------------------------
-- Combination of input/output functions
-- We need to add asText/asEWKT to avoid problems due to floating point precision
-- The MFJSON format does not output the SRID

SELECT DISTINCT asText(tgeompointFromText(asText(temp))) = asText(temp) FROM tbl_tgeompoint;
SELECT DISTINCT asText(tgeogpointFromText(asText(temp))) = asText(temp) FROM tbl_tgeogpoint;

SELECT DISTINCT asEWKT(tgeompointFromEWKT(asEWKT(temp))) = asEWKT(temp) FROM tbl_tgeompoint;
SELECT DISTINCT asEWKT(tgeogpointFromEWKT(asEWKT(temp))) = asEWKT(temp) FROM tbl_tgeogpoint;

SELECT DISTINCT asText(tgeompointFromMFJSON(asMFJSON(temp))) = asText(temp) FROM tbl_tgeompoint;
SELECT DISTINCT asText(tgeogpointFromMFJSON(asMFJSON(temp))) = asText(temp) FROM tbl_tgeogpoint;

SELECT DISTINCT tgeompointFromBinary(asBinary(temp)) = temp FROM tbl_tgeompoint;
SELECT DISTINCT tgeogpointFromBinary(asBinary(temp)) = temp FROM tbl_tgeogpoint;

SELECT DISTINCT tgeompointFromEWKB(asEWKB(temp)) = temp FROM tbl_tgeompoint;
SELECT DISTINCT tgeogpointFromEWKB(asEWKB(temp)) = temp FROM tbl_tgeogpoint;

SELECT DISTINCT tgeompointFromHexEWKB(asHexEWKB(temp)) = temp FROM tbl_tgeompoint;
SELECT DISTINCT tgeogpointFromHexEWKB(asHexEWKB(temp)) = temp FROM tbl_tgeogpoint;

-------------------------------------------------------------------------------
-- MF-JSON output of the temporal point values coming from real AIS data
-- The AIS tables keep the full precision of the recorded positions, which is
-- what exposes the output corruption reported at
-- https://github.com/MobilityDB/MobilityDB/issues/850
-- Every query below reports the rows whose output is not valid JSON or does
-- not read back with the same shape, and must thus report zero
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_ais_tgeompoint WHERE asMFJSON(temp)::jsonb IS NULL;
SELECT count(*) FROM tbl_ais_tgeompoint
  WHERE asMFJSON(temp::tgeogpoint)::jsonb IS NULL;
SELECT count(*) FROM tbl_ais_tgeompoint, generate_series(0, 15) AS d
  WHERE asMFJSON(temp, 1, 0, d)::jsonb IS NULL;

-- Temporal types derived from the AIS trajectories

SELECT count(*) FROM tbl_ais_tgeompoint
  WHERE asMFJSON(getX(temp))::jsonb IS NULL;
SELECT count(*) FROM tbl_ais_tgeompoint WHERE asMFJSON(tIntersects(temp,
  geometry 'SRID=4326;Polygon((9 56,12 56,12 58,9 58,9 56))'))::jsonb IS NULL;

-- Temporal types built from the AIS instants

WITH ais(mmsi, speed, course, moving, name) AS (
  SELECT mmsi, tfloatSeq(array_agg(tfloat(sog, t) ORDER BY t)),
    tintSeq(array_agg(tint(round(degrees(heading))::int, t) ORDER BY t)),
    tboolSeq(array_agg(tbool(sog > 0, t) ORDER BY t)),
    ttextSeq(array_agg(ttext(mmsi::text, t) ORDER BY t))
  FROM tbl_ais_instant GROUP BY mmsi )
SELECT count(*) FROM ais WHERE asMFJSON(speed)::jsonb IS NULL OR
  asMFJSON(course)::jsonb IS NULL OR asMFJSON(moving)::jsonb IS NULL OR
  asMFJSON(name)::jsonb IS NULL;

SELECT count(*) FROM tbl_ais_tgeompoint
  WHERE tgeompointFromMFJSON(asMFJSON(temp)) IS NULL;

-------------------------------------------------------------------------------
