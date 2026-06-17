-- Drone photogrammetry ingest — pcid registration for an SfM / MVS
-- point cloud (Pix4D, RealityCapture, Metashape, OpenDroneMap output).
--
-- Photogrammetry differs from LiDAR in three ways that matter for the
-- schema:
--   - Per-point colour (RGB) is the headline payload; intensity is rarely
--     present.
--   - Z scale is usually metric (1 mm); X / Y are in the project's
--     working CRS (Lambert 72 EPSG:31370 for Belgian surveys, or
--     WGS84 lon/lat for export).
--   - Point density is typically 2–3 orders of magnitude higher than
--     LiDAR for the same survey area, which makes dimensional
--     compression (via writers.tpcpatch's compression='dimensional')
--     valuable for the SQL transport.
--
-- Adjust the offsets to your project area. The values below assume
-- a Belgian site in Lambert 72 (EPSG:31370): X around 148000,
-- Y around 168000.
-- For a WGS84 export, switch to 0.0000001 deg scale and longitude /
-- latitude offsets (see drone_lidar_ingest.sql for the WGS84 layout).

INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (
  700,
  31370,
  '<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1">
  <pc:dimension>
    <pc:position>1</pc:position>
    <pc:size>4</pc:size>
    <pc:name>X</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.001</pc:scale>
    <pc:offset>148000</pc:offset>
  </pc:dimension>
  <pc:dimension>
    <pc:position>2</pc:position>
    <pc:size>4</pc:size>
    <pc:name>Y</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.001</pc:scale>
    <pc:offset>168000</pc:offset>
  </pc:dimension>
  <pc:dimension>
    <pc:position>3</pc:position>
    <pc:size>4</pc:size>
    <pc:name>Z</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.001</pc:scale>
    <pc:offset>0</pc:offset>
  </pc:dimension>
  <pc:dimension>
    <pc:position>4</pc:position>
    <pc:size>2</pc:size>
    <pc:name>Red</pc:name>
    <pc:interpretation>uint16_t</pc:interpretation>
    <pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>5</pc:position>
    <pc:size>2</pc:size>
    <pc:name>Green</pc:name>
    <pc:interpretation>uint16_t</pc:interpretation>
    <pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>6</pc:position>
    <pc:size>2</pc:size>
    <pc:name>Blue</pc:name>
    <pc:interpretation>uint16_t</pc:interpretation>
    <pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:metadata>
    <Metadata name="compression">none</Metadata>
  </pc:metadata>
</pc:PointCloudSchema>'
) ON CONFLICT (pcid) DO UPDATE SET schema = EXCLUDED.schema;

-- Destination table. survey_date carries the campaign date (a
-- photogrammetry survey is one flight, so each row is one campaign);
-- the tpcpatch's per-instant timestamps carry sub-flight time when the
-- source format records it (LAS GpsTime, COPC), or all-equal when the
-- source format does not (PLY).
CREATE TABLE IF NOT EXISTS drone_photo_surveys (
  id           serial PRIMARY KEY,
  site_name    text,
  survey_date  date,
  cloud        tpcpatch
);
