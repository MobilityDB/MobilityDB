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
