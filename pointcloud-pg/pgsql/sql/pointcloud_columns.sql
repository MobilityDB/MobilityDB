INSERT INTO pointcloud_formats (pcid, srid) VALUES (777, 666);
CREATE TABLE pc1(p pcpoint);
CREATE TABLE pc2(p pcpoint);
DELETE FROM pointcloud_formats WHERE pcid = 777;
SELECT * from pointcloud_columns ORDER BY 1,2,3,4;
DROP TABLE pc1;
DROP TABLE pc2;
