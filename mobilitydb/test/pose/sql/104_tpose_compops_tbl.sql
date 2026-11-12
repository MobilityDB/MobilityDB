-------------------------------------------------------------------------------
-- Temporal equal
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_pose2d t1, tbl_tpose2d t2 WHERE t1.pose #= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_pose2d t2 WHERE t1.temp #= t2.pose IS NOT NULL;
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp #= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal not equal
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_pose2d t1, tbl_tpose2d t2 WHERE t1.pose #<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_pose2d t2 WHERE t1.temp #<> t2.pose IS NOT NULL;
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

-- Ever and always overlaps: a value and a spatiotemporal box, and two values

SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp ?&& expandTime(expandSpace(stbox(t2.temp), 20), '1 year') <> (atStbox(t1.temp, expandTime(expandSpace(stbox(t2.temp), 20), '1 year')) IS NOT NULL);
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp %&& expandTime(expandSpace(stbox(t2.temp), 20), '1 year') <> (minusStbox(t1.temp, expandTime(expandSpace(stbox(t2.temp), 20), '1 year')) IS NULL);
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.k <= 30 AND t2.k <= 30
AND expandTime(expandSpace(stbox(t2.temp), 20), '1 year') ?&& t1.temp <> t1.temp ?&& expandTime(expandSpace(stbox(t2.temp), 20), '1 year');
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp ?&& expandTime(expandSpace(stbox(t2.temp), 20), '1 year');
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp %&& expandTime(expandSpace(stbox(t2.temp), 20), '1 year');
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp && expandTime(expandSpace(stbox(t2.temp), 20), '1 year');
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2
WHERE t1.temp ?&& t2.temp <> tgeompoint(t1.temp) ?= tgeompoint(t2.temp);
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2
WHERE t1.temp %&& t2.temp <> tgeompoint(t1.temp) %= tgeompoint(t2.temp);
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2
WHERE t1.temp ?= t2.temp AND NOT t1.temp ?&& t2.temp;
SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2 WHERE t1.temp ?&& t2.temp;
