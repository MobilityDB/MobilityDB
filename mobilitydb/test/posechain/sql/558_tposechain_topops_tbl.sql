-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp && t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp @> t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp <@ t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp ~= t2.temp;
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2 WHERE t1.temp -|- t2.temp;

-- Containment implies overlap, and a value contains itself
SELECT COUNT(*) FROM tbl_tposechain t1, tbl_tposechain t2
WHERE t1.temp @> t2.temp AND NOT (t1.temp && t2.temp);
SELECT COUNT(*) FROM tbl_tposechain WHERE NOT (temp @> temp);
SELECT COUNT(*) FROM tbl_tposechain WHERE NOT (temp ~= temp);

-------------------------------------------------------------------------------
