-------------------------------------------------------------------------------

SELECT astext(temp) FROM tbl_tgeompoint LIMIT 10;
SELECT astext(temp) FROM tbl_tgeogpoint LIMIT 10;
SELECT astext(temp) FROM tbl_tgeompoint3D LIMIT 10;
SELECT astext(temp) FROM tbl_tgeogpoint3D LIMIT 10;
SELECT k%90, astext(array_agg(g ORDER BY k)) FROM tbl_geography3D WHERE g IS NOT NULL GROUP BY k%90 ORDER BY k%90 LIMIT 10;
SELECT k%90, astext(array_agg(temp ORDER BY k)) FROM tbl_tgeogpoint3D WHERE temp IS NOT NULL GROUP BY k%90 ORDER BY k%90 LIMIT 10;

SELECT asEWKT(temp) FROM tbl_tgeompoint LIMIT 10;
SELECT asEWKT(temp) FROM tbl_tgeogpoint LIMIT 10;
SELECT asEWKT(temp) FROM tbl_tgeompoint3D LIMIT 10;
SELECT asEWKT(temp) FROM tbl_tgeogpoint3D LIMIT 10;
SELECT k%90, asEWKT(array_agg(g ORDER BY k)) FROM tbl_geography3D WHERE g IS NOT NULL GROUP BY k%90 ORDER BY k%90 LIMIT 10;
SELECT k%90, asEWKT(array_agg(temp ORDER BY k)) FROM tbl_tgeogpoint3D WHERE temp IS NOT NULL GROUP BY k%90 ORDER BY k%90 LIMIT 10;

-------------------------------------------------------------------------------
