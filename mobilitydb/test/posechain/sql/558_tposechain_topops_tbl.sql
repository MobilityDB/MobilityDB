-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Topological operators
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tposechain2d t1, tbl_tposechain2d t2 WHERE t1.temp && t2.temp;
SELECT COUNT(*) FROM tbl_tposechain2d t1, tbl_tposechain2d t2 WHERE t1.temp @> t2.temp;
SELECT COUNT(*) FROM tbl_tposechain2d t1, tbl_tposechain2d t2 WHERE t1.temp <@ t2.temp;
SELECT COUNT(*) FROM tbl_tposechain2d t1, tbl_tposechain2d t2 WHERE t1.temp ~= t2.temp;
SELECT COUNT(*) FROM tbl_tposechain2d t1, tbl_tposechain2d t2 WHERE t1.temp -|- t2.temp;

-- Containment implies overlap, and a value contains itself
SELECT COUNT(*) FROM tbl_tposechain2d t1, tbl_tposechain2d t2
WHERE t1.temp @> t2.temp AND NOT (t1.temp && t2.temp);
SELECT COUNT(*) FROM tbl_tposechain2d WHERE NOT (temp @> temp);
SELECT COUNT(*) FROM tbl_tposechain2d WHERE NOT (temp ~= temp);

-------------------------------------------------------------------------------
