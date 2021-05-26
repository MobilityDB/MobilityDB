-------------------------------------------------------------------------------
-- Geometry rel tnpoint
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE tcontains(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE tcovers(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE tcoveredby(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE tdisjoint(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE tequals(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE tintersects(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE ttouches(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE twithin(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE tdwithin(ST_SetSRID(t1.g, 5676), t2.temp, 0.01) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE trelate(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_geometry t1, tbl_tnpoint t2 WHERE trelate(ST_SetSRID(t1.g, 5676), t2.temp, 'T*****FF*') IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;

-------------------------------------------------------------------------------
-- Geometry rel tnpoint
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE tcontains(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE tcovers(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE tcoveredby(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE tdisjoint(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE tequals(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE tintersects(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE ttouches(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE twithin(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE tdwithin(t1.np, t2.temp, 0.01) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE trelate(t1.np, t2.temp) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE trelate(t1.np, t2.temp, 'T*****FF*') IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;

-------------------------------------------------------------------------------
-- tnpoint rel <Type>
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE tcontains(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE tcovers(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE tcoveredby(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE tdisjoint(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE tequals(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE tintersects(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE ttouches(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE twithin(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE tdwithin(t1.temp, ST_SetSRID(t2.g, 5676), 0.01) IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE trelate(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_geometry t2 WHERE trelate(t1.temp, ST_SetSRID(t2.g, 5676), 'T*****FF*') IS NOT NULL AND t1.k < 5 AND t2.k%25 < 5;

SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE tcontains(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE tcovers(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE tcoveredby(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE tdisjoint(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE tequals(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE tintersects(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE ttouches(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE twithin(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE tdwithin(t1.temp, t2.np, 0.01) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE trelate(t1.temp, t2.np) IS NOT NULL AND t1.k < 5 AND t2.k < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE trelate(t1.temp, t2.np, 'T*****FF*') IS NOT NULL AND t1.k < 5 AND t2.k < 5;

SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE tdisjoint(t1.temp, t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE tequals(t1.temp, t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE tintersects(t1.temp, t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE tdwithin(t1.temp, t2.temp, 0.01) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE trelate(t1.temp, t2.temp) IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE trelate(t1.temp, t2.temp, 'T*****FF*') IS NOT NULL AND t1.k%25 < 5 AND t2.k%25 < 5;

-------------------------------------------------------------------------------
