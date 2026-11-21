-------------------------------------------------------------------------------
-- A point cloud value transformed into the schema of another pcid
-------------------------------------------------------------------------------

-- A point cloud value takes its SRID from the schema its pcid names, so it is
-- transformed into another schema, which states the target SRID and how the
-- transformed coordinates are stored.

-- pcid 101: WGS84 longitude, latitude and height, stored as doubles
INSERT INTO pointcloud_schemas (pcid, srid, compression) VALUES (101, 4326, 'none');
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation) VALUES
  (101, 1, 'X', 'double'), (101, 2, 'Y', 'double'), (101, 3, 'Z', 'double');
-- pcid 102: Web Mercator metres, with an intensity pcid 101 has not
INSERT INTO pointcloud_schemas (pcid, srid, compression) VALUES (102, 3857, 'none');
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation) VALUES
  (102, 1, 'X', 'double'), (102, 2, 'Y', 'double'), (102, 3, 'Z', 'double'),
  (102, 4, 'Intensity', 'uint16_t');
-- pcid 103: Web Mercator metres stored as int16, too narrow for Brussels
INSERT INTO pointcloud_schemas (pcid, srid, compression) VALUES (103, 3857, 'none');
INSERT INTO pointcloud_dimensions (pcid, dim_no, dim_name, interpretation) VALUES
  (103, 1, 'X', 'int16_t'), (103, 2, 'Y', 'int16_t');
-- pcid 104: WGS84 again, stored as int32 at 1e-7 degree
INSERT INTO pointcloud_schemas (pcid, srid, compression) VALUES (104, 4326, 'none');
INSERT INTO pointcloud_dimensions
    (pcid, dim_no, dim_name, interpretation, dim_scale, dim_offset) VALUES
  (104, 1, 'X', 'int32_t', 0.0000001, 0), (104, 2, 'Y', 'int32_t', 0.0000001, 0);

-- Brussels into Web Mercator: X and Y follow the projection, Z is kept, and
-- the intensity pcid 101 has not is 0, as pgPointCloud's PC_Transform fills it
SELECT round(getX(p)::numeric, 3) AS x, round(getY(p)::numeric, 3) AS y,
  getZ(p) AS z, getDim(p, 'Intensity') AS intensity, pcid(p), SRID(p)
FROM (SELECT transform(pcpoint(101, 4.35, 50.85, 10), 102) AS p) t;

-- The round trip returns the longitude and latitude
SELECT round(getX(p)::numeric, 9) AS x, round(getY(p)::numeric, 9) AS y
FROM (SELECT transform(transform(pcpoint(101, 4.35, 50.85, 10), 102), 101)
  AS p) t;

-- A schema stating the same SRID only stores the coordinates again
SELECT round(getX(p)::numeric, 7) AS x, round(getY(p)::numeric, 7) AS y,
  pcid(p), SRID(p)
FROM (SELECT transform(pcpoint(101, 4.35, 50.85, 10), 104) AS p) t;

-- A coordinate the target dimension cannot hold is refused rather than
-- clamped into it
SELECT transform(pcpoint(101, 4.35, 50.85, 10), 103);

-- A target pcid naming no schema is refused
SELECT transform(pcpoint(101, 4.35, 50.85, 10), 199);

-- A patch transforms each of its points
SELECT pcid(transform(pcpatch(pcpoint(101, 4.35, 50.85, 10),
  pcpoint(101, 3.72, 51.05, 5)), 102));
SELECT numPoints(transform(pcpatch(pcpoint(101, 4.35, 50.85, 10),
  pcpoint(101, 3.72, 51.05, 5)), 102));
SELECT SRID(transform(pcpatch(pcpoint(101, 4.35, 50.85, 10),
  pcpoint(101, 3.72, 51.05, 5)), 102));
SELECT ST_AsText(geometry(transform(pcpatch(pcpoint(101, 4.35, 50.85, 10),
  pcpoint(101, 3.72, 51.05, 5)), 102)), 3);

-- A set transforms each of its values
SELECT pcid(startValue(s)), SRID(startValue(s))
FROM (SELECT transform(set(ARRAY[pcpoint(101, 4.35, 50.85, 10),
  pcpoint(101, 3.72, 51.05, 5)]), 102) AS s) t;

-- A temporal value transforms the value of each instant
SELECT pcid(t), SRID(t), round(getX(startValue(t))::numeric, 3)
FROM (SELECT transform(tpcpoint(pcpoint(101, 4.35, 50.85, 10),
  timestamptz '2024-01-01'), 102) AS t) s;

-- A box transforms its extent and takes the target pcid
SELECT round(xMin(b)::numeric, 3), pcid(b), SRID(b)
FROM (SELECT transform(tpcbox(pcpoint(101, 4.35, 50.85, 10)), 102) AS b) t;

-- A box with no spatial extent takes the pcid and the SRID of the target
SELECT transform(tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 101), 102);

-------------------------------------------------------------------------------

DELETE FROM pointcloud_schemas WHERE pcid IN (101, 102, 103, 104);

-------------------------------------------------------------------------------
