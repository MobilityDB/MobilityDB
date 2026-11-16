-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Send and receive
-------------------------------------------------------------------------------

\copy tbl_tposechain TO 'tbl_tposechain' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_tposechain_tmp;
CREATE TEMP TABLE tbl_tposechain_tmp AS TABLE tbl_tposechain WITH NO DATA;
\copy tbl_tposechain_tmp FROM 'tbl_tposechain' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain_tmp t2 WHERE t1.k = t2.k AND t1.temp <> t2.temp;
DROP TABLE tbl_tposechain_tmp;

-------------------------------------------------------------------------------
-- Representation round trips
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tposechain WHERE tposechainFromEWKT(asEWKT(temp)) <> temp;
SELECT COUNT(*) FROM tbl_tposechain WHERE tposechainFromHexEWKB(asHexEWKB(temp)) <> temp;
SELECT DISTINCT asText(tposechainFromHexWKB(asHexWKB(temp))) = asText(temp) FROM tbl_tposechain;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT MIN(numPoses(temp)), MAX(numPoses(temp)) FROM tbl_tposechain;
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
