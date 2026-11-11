-------------------------------------------------------------------------------
-- Send/receive functions
-------------------------------------------------------------------------------

\copy tbl_pose2d TO 'tbl_pose2d' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_pose2d_tmp;
CREATE TEMP TABLE tbl_pose2d_tmp AS TABLE tbl_pose2d WITH NO DATA;
\copy tbl_pose2d_tmp FROM 'tbl_pose2d' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d_tmp t2 WHERE t1.k = t2.k AND t1.pose <> t2.pose;
DROP TABLE tbl_pose2d_tmp;

-------------------------------------------------------------------------------
-- Accessing values
-------------------------------------------------------------------------------

SELECT MAX(ST_X(point(pose))) FROM tbl_pose2d;
SELECT MAX(yaw(pose)) FROM tbl_pose2d;

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_pose2d WHERE pose::geometry IS NOT NULL;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose = t2.pose;
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose != t2.pose;
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose < t2.pose;
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose <= t2.pose;
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose > t2.pose;
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose >= t2.pose;

-------------------------------------------------------------------------------/
