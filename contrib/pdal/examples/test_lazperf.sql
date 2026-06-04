-- Regression fixture for the readers.tpcpatch path against
-- LAZ-compressed pcpatches. Skips itself with a NOTICE if the system
-- pgPointCloud was built without --with-lazperf.

DO $$
DECLARE has_laz boolean;
BEGIN
  -- PC_LazPerfEnabled() is only declared by --with-lazperf builds.
  -- pg_proc lowercases identifiers but keeps underscores.
  PERFORM 1 FROM pg_proc WHERE proname = 'pc_lazperf_enabled';
  IF NOT FOUND THEN
    RAISE EXCEPTION 'PC_LazPerf_Enabled is absent: pgPointCloud was built '
                    'without lazperf. Rebuild with --with-lazperf=DIR.';
  END IF;
  EXECUTE 'SELECT PC_LazPerfEnabled()' INTO has_laz;
  IF NOT has_laz THEN
    RAISE EXCEPTION 'PC_LazPerfEnabled() returned false; aborting.';
  END IF;
  RAISE NOTICE 'lazperf is enabled — running LAZ regression';
END
$$;

-- Register a LAZ-compressed schema (pcid=200) with int32 X/Y/Z + scale.
INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (
  200, 0,
  '<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1">
  <pc:dimension><pc:position>1</pc:position><pc:size>4</pc:size><pc:name>X</pc:name><pc:interpretation>int32_t</pc:interpretation><pc:scale>0.01</pc:scale></pc:dimension>
  <pc:dimension><pc:position>2</pc:position><pc:size>4</pc:size><pc:name>Y</pc:name><pc:interpretation>int32_t</pc:interpretation><pc:scale>0.01</pc:scale></pc:dimension>
  <pc:dimension><pc:position>3</pc:position><pc:size>4</pc:size><pc:name>Z</pc:name><pc:interpretation>int32_t</pc:interpretation><pc:scale>0.01</pc:scale></pc:dimension>
  <pc:metadata><Metadata name="compression">laz</Metadata></pc:metadata>
</pc:PointCloudSchema>'
) ON CONFLICT (pcid) DO UPDATE SET schema = EXCLUDED.schema;

DROP TABLE IF EXISTS test_laz_plugin;
CREATE TABLE test_laz_plugin (
  id   serial PRIMARY KEY,
  t    timestamptz NOT NULL,
  pcp  pcpatch NOT NULL
);

INSERT INTO test_laz_plugin (t, pcp) VALUES
  ('2024-01-01 12:00:00+00',
   PC_Compress(
     PC_Patch(ARRAY[
       PC_MakePoint(200, ARRAY[100.0, 200.0, 300.0]),
       PC_MakePoint(200, ARRAY[400.0, 500.0, 600.0])
     ]),
     'laz')),
  ('2024-01-01 12:00:01+00',
   PC_Compress(
     PC_Patch(ARRAY[
       PC_MakePoint(200, ARRAY[700.0, 800.0, 900.0]),
       PC_MakePoint(200, ARRAY[1000.0, 1100.0, 1200.0]),
       PC_MakePoint(200, ARRAY[1300.0, 1400.0, 1500.0])
     ]),
     'laz'));

-- Sanity: confirm rows are LAZ-compressed (PC_Compression == 2).
SELECT id, t, PC_NumPoints(pcp), PC_Compression(pcp) AS comp,
       PC_PCId(pcp) AS pcid
FROM test_laz_plugin ORDER BY id;
-- Expected:
--   1 | 2024-01-01 ... | 2 | 2 | 200
--   2 | 2024-01-01 ... | 3 | 2 | 200

-- The PDAL pipeline at examples/test_lazperf.json reads this fixture via
-- readers.tpcpatch; a successful run proves that PC_LAZPERF dispatch in
-- libpc is reachable through our reader.
