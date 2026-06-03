/* Regression: scale/offset XForm round-trip through PDAL plugin.
 *
 * pcid=600 stores X / Y / Z as int32 with scale=0.001 and per-dim offsets
 * mimicking a typical LAS-1.4 schema (X/Y in metric coordinates with a
 * survey-area offset, Z in millimetres). The reader must apply
 * raw_int32 * scale + offset, and the writer must invert it before
 * packing back into int32. Round-trip proves both sides agree.
 */

INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (600, 0,
'<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1">
  <pc:dimension><pc:position>1</pc:position><pc:size>4</pc:size><pc:description>X</pc:description><pc:name>X</pc:name><pc:interpretation>int32_t</pc:interpretation><pc:scale>0.001</pc:scale><pc:offset>100000</pc:offset></pc:dimension>
  <pc:dimension><pc:position>2</pc:position><pc:size>4</pc:size><pc:description>Y</pc:description><pc:name>Y</pc:name><pc:interpretation>int32_t</pc:interpretation><pc:scale>0.001</pc:scale><pc:offset>200000</pc:offset></pc:dimension>
  <pc:dimension><pc:position>3</pc:position><pc:size>4</pc:size><pc:description>Z</pc:description><pc:name>Z</pc:name><pc:interpretation>int32_t</pc:interpretation><pc:scale>0.001</pc:scale><pc:offset>0</pc:offset></pc:dimension>
  <pc:metadata><Metadata name="compression">none</Metadata></pc:metadata>
</pc:PointCloudSchema>')
ON CONFLICT (pcid) DO NOTHING;

DROP TABLE IF EXISTS xform_patches;
CREATE TABLE xform_patches (id int, t timestamptz, pcp pcpatch);
INSERT INTO xform_patches VALUES
  (1, '2024-04-29 10:00:00+00', PC_Patch(ARRAY[
    PC_MakePoint(600, ARRAY[100123.456, 200456.789, 12.345]),
    PC_MakePoint(600, ARRAY[100124.000, 200457.500, 13.000])
  ])),
  (2, '2024-04-29 10:00:01+00', PC_Patch(ARRAY[
    PC_MakePoint(600, ARRAY[100125.123, 200458.987, 14.654])
  ]));

DROP TABLE IF EXISTS xform_roundtrip;
CREATE TABLE xform_roundtrip (id serial primary key, traj tpcpatch);
