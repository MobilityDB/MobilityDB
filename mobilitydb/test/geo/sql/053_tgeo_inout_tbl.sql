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
SELECT DISTINCT asText(tgeometryFromHexWKB(asHexWKB(temp))) = asText(temp) FROM tbl_tgeometry;
SELECT DISTINCT tgeographyFromHexEWKB(asHexEWKB(temp)) = temp FROM tbl_tgeography;
SELECT DISTINCT asText(tgeographyFromHexWKB(asHexWKB(temp))) = asText(temp) FROM tbl_tgeography;

-------------------------------------------------------------------------------
-- MF-JSON output of the temporal geometry values coming from real AIS data
-- The AIS footprints are the vessel outlines computed from the recorded
-- positions, and keep the full precision of the computed coordinates, which is
-- what exposes the output corruption reported at
-- https://github.com/MobilityDB/MobilityDB/issues/850
-- Every query below reports the rows whose output is not valid JSON or does
-- not read back with the same shape, and must thus report zero
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_ais_tgeometry WHERE asMFJSON(temp)::jsonb IS NULL;
SELECT count(*) FROM tbl_ais_tgeometry
  WHERE asMFJSON(temp::tgeography)::jsonb IS NULL;
WITH ais(mmsi, temp) AS (
  SELECT mmsi, tgeometrySeq(array_agg(tgeometry(geom, t) ORDER BY t))
  FROM tbl_ais_instant GROUP BY mmsi )
SELECT count(*) FROM ais WHERE asMFJSON(temp)::jsonb IS NULL;
SELECT count(*) FROM tbl_ais_tgeometry, generate_series(0, 15) AS d
  WHERE asMFJSON(temp, 1, 0, d)::jsonb IS NULL;

SELECT count(*) FROM tbl_ais_tgeometry
  WHERE tgeometryFromMFJSON(asMFJSON(temp)) IS NULL;

-------------------------------------------------------------------------------
