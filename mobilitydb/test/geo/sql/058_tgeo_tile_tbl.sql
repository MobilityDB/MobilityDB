-------------------------------------------------------------------------------
-- Multidimensional tiling
-------------------------------------------------------------------------------

SELECT spaceTiles(b, 2.5, geometry 'Point(10 10)'), COUNT(*) FROM tbl_stbox GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT timeTiles(b, interval '1 week'), COUNT(*) FROM tbl_stbox GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT spaceTimeTiles(b, 2.5, interval '1 week'), COUNT(*) FROM tbl_stbox GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;
SELECT spaceTimeTiles(b, 2.5, interval '1 week', 'Point(10 10)', '2001-06-01'), COUNT(*) FROM tbl_stbox GROUP BY 1 ORDER BY 2 DESC, 1 LIMIT 3;

-- 2D
SELECT extent(getSpaceTile(g, 2.5)) FROM
(SELECT * FROM tbl_geom_point WHERE g IS NOT NULL AND NOT ST_IsEmpty(g) LIMIT 10) t1;
SELECT extent(getSpaceTile(g, 2.5, geometry 'Point(10 10)')) FROM
(SELECT * FROM tbl_geom_point WHERE g IS NOT NULL AND NOT ST_IsEmpty(g) LIMIT 10) t1;
-- 3D
SELECT extent(getSpaceTile(g, 2.5)) FROM
(SELECT * FROM tbl_geom_point3D WHERE g IS NOT NULL AND NOT ST_IsEmpty(g) LIMIT 10) t1;
SELECT extent(getSpaceTile(g, 2.5, geometry 'Point(10 10 10)')) FROM
(SELECT * FROM tbl_geom_point3D WHERE g IS NOT NULL AND NOT ST_IsEmpty(g) LIMIT 10) t1;

-- Time
SELECT extent(getStboxTimeTile(t, interval '2 days')) FROM
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL) t2;
SELECT extent(getStboxTimeTile(t, interval '2 days', '2001-06-01')) FROM
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL) t2;

-- 2D
SELECT extent(getSpaceTimeTile(g, t, 2.5, interval '2 days')) FROM
(SELECT * FROM tbl_geom_point WHERE g IS NOT NULL AND NOT ST_IsEmpty(g) LIMIT 10 OFFSET 10) t1,
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL) t2;
SELECT extent(getSpaceTimeTile(g, t, 2.5, interval '2 days', geometry 'Point(10 10)', '2001-06-01')) FROM
(SELECT * FROM tbl_geom_point WHERE g IS NOT NULL AND NOT ST_IsEmpty(g) LIMIT 10 OFFSET 10) t1,
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL) t2;
-- 3D
SELECT extent(getSpaceTimeTile(g, t, 2.5, interval '2 days')) FROM
(SELECT * FROM tbl_geom_point3D WHERE g IS NOT NULL AND NOT ST_IsEmpty(g) LIMIT 10 OFFSET 10) t1,
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL) t2;
SELECT extent(getSpaceTimeTile(g, t, 2.5, interval '2 days', geometry 'Point(10 10 10)', '2001-06-01')) FROM
(SELECT * FROM tbl_geom_point3D WHERE g IS NOT NULL AND NOT ST_IsEmpty(g) LIMIT 10 OFFSET 10) t1,
(SELECT * FROM tbl_timestamptz WHERE t IS NOT NULL) t2;

-------------------------------------------------------------------------------


