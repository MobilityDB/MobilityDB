-------------------------------------------------------------------------------

SELECT round(MAX(maxValue(ST_SetSRID(t1.g, 5676) <-> t2.temp))::numeric, 6) FROM tbl_geom_point t1, tbl_tnpoint t2;
SELECT round(MAX(maxValue(t1.np <-> t2.temp))::numeric, 6) FROM tbl_npoint t1, tbl_tnpoint t2;
SELECT round(MAX(maxValue(t1.temp <-> ST_SetSRID(t2.g, 5676)))::numeric, 6) FROM tbl_tnpoint t1, tbl_geom_point t2;
SELECT round(MAX(maxValue(t1.temp <-> t2.np))::numeric, 6) FROM tbl_tnpoint t1, tbl_npoint t2;
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE t1.temp <-> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
