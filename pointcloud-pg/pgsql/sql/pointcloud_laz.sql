INSERT INTO pointcloud_formats (pcid, srid, schema)
VALUES (5, 0,
'<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <pc:dimension>
    <pc:position>1</pc:position>
    <pc:size>4</pc:size>
    <pc:description>X coordinate as a long integer. You must use the scale and offset information of the header to determine the double value.</pc:description>
    <pc:name>X</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>2</pc:position>
    <pc:size>4</pc:size>
    <pc:description>Y coordinate as a long integer. You must use the scale and offset information of the header to determine the double value.</pc:description>
    <pc:name>Y</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>3</pc:position>
    <pc:size>4</pc:size>
    <pc:description>Z coordinate as a long integer. You must use the scale and offset information of the header to determine the double value.</pc:description>
    <pc:name>Z</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>4</pc:position>
    <pc:size>2</pc:size>
    <pc:description>The intensity value is the integer representation of the pulse return magnitude. This value is optional and system specific. However, it should always be included if available.</pc:description>
    <pc:name>Intensity</pc:name>
    <pc:interpretation>uint16_t</pc:interpretation>
    <pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:metadata>
    <Metadata name="compression">laz</Metadata>
  </pc:metadata>
</pc:PointCloudSchema>'
)
,(10, 0, -- All (signed) interpretations, uncompressed
'<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <pc:dimension> <pc:position>1</pc:position> <pc:name>x</pc:name>
    <pc:size>1</pc:size> <pc:interpretation>int8_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension> <pc:position>2</pc:position> <pc:name>y</pc:name>
    <pc:size>2</pc:size> <pc:interpretation>int8_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension> <pc:position>3</pc:position> <pc:name>i2</pc:name>
    <pc:size>2</pc:size> <pc:interpretation>int16_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension> <pc:position>4</pc:position> <pc:name>i4</pc:name>
    <pc:size>4</pc:size> <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension> <pc:position>5</pc:position> <pc:name>i8</pc:name>
    <pc:size>8</pc:size> <pc:interpretation>int64_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension> <pc:position>6</pc:position> <pc:name>f4</pc:name>
    <pc:size>4</pc:size> <pc:interpretation>float</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension> <pc:position>7</pc:position> <pc:name>f8</pc:name>
    <pc:size>8</pc:size> <pc:interpretation>double</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
</pc:PointCloudSchema>'
);

CREATE TABLE IF NOT EXISTS pa_test_laz (
	id SERIAL,
    pa PCPATCH(5)
);
\d pa_test_laz

INSERT INTO pa_test_laz (pa) VALUES ('0000000005000000000000000200000002000000030000000500060000000200000003000000050008');
INSERT INTO pa_test_laz (pa) VALUES ('000000000500000000000000020000000600000007000000050006000000090000000A00000005000A');
INSERT INTO pa_test_laz (pa) VALUES ('0000000005000000000000000200000002000000030000000500060000000200000003000000050003');
INSERT INTO pa_test_laz (pa) VALUES ('0000000005000000000000000200000002000000030000000500060000000200000003000000050001');

SELECT pc_explode(pa) FROM pa_test_laz;
SELECT pc_astext(pc_explode(pa)) FROM pa_test_laz;
SELECT * FROM pa_test_laz;

SELECT Sum(PC_NumPoints(pa)) FROM pa_test_laz;
SELECT Sum(PC_MemSize(pa)) FROM pa_test_laz;
SELECT Sum(PC_PatchMax(pa,'x')) FROM pa_test_laz;
SELECT Sum(PC_PatchMin(pa,'x')) FROM pa_test_laz;

SELECT PC_Uncompress(pa) FROM pa_test_laz WHERE id=1;

DELETE FROM pa_test_laz;
INSERT INTO pa_test_laz (pa)
SELECT PC_Patch(PC_MakePoint(5, ARRAY[x,y,z,intensity]))
FROM (
 SELECT
 -127+a/100.0 AS x,
   45+a/100.0 AS y,
        1.0*a AS z,
         a/10 AS intensity,
         a/400 AS gid
  FROM generate_series(1,1600) AS a
) AS values GROUP BY gid;

SELECT pc_explode(pa) FROM pa_test_laz LIMIT 20;
SELECT pc_astext(pc_explode(pa)) FROM pa_test_laz LIMIT 20;
SELECT * FROM pa_test_laz LIMIT 20;

SELECT Sum(PC_NumPoints(pa)) FROM pa_test_laz;
SELECT Sum(PC_MemSize(pa)) FROM pa_test_laz;

SELECT Max(PC_PatchMax(pa,'x')) FROM pa_test_laz;
SELECT Min(PC_PatchMin(pa,'x')) FROM pa_test_laz;
SELECT Min(PC_PatchMin(pa,'z')) FROM pa_test_laz;

SELECT pc_astext(PC_FilterLessThan(pa, 'z', 5)) FROM pa_test_laz;
SELECT pc_astext(PC_FilterGreaterThan(pa, 'z', 1595)) FROM pa_test_laz;
SELECT pc_astext(PC_FilterEquals(pa, 'z', 500)) FROM pa_test_laz;
SELECT pc_astext(PC_FilterBetween(pa, 'z', 500, 505)) FROM pa_test_laz;

DELETE FROM pa_test_laz;
INSERT INTO pa_test_laz( pa ) VALUES ('01050000000200000004000000210000000000000000000000000000000a004417593a34c1c5f74f83179fc2448960000000');

SELECT pc_explode(pa) FROM pa_test_laz;
SELECT pc_astext(pc_explode(pa)) FROM pa_test_laz;

INSERT INTO pointcloud_formats (pcid, srid, schema)
VALUES (6, 0,
'<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <pc:dimension>
    <pc:position>1</pc:position>
    <pc:size>4</pc:size>
    <pc:description>X coordinate as a long integer. You must use the scale and offset information of the header to determine the double value.</pc:description>
    <pc:name>X</pc:name>
    <pc:interpretation>int64_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>2</pc:position>
    <pc:size>4</pc:size>
    <pc:description>Y coordinate as a long integer. You must use the scale and offset information of the header to determine the double value.</pc:description>
    <pc:name>Y</pc:name>
    <pc:interpretation>double</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>3</pc:position>
    <pc:size>4</pc:size>
    <pc:description>Z coordinate as a long integer. You must use the scale and offset information of the header to determine the double value.</pc:description>
    <pc:name>Z</pc:name>
    <pc:interpretation>float</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>4</pc:position>
    <pc:size>2</pc:size>
    <pc:description>The intensity value is the integer representation of the pulse return magnitude. This value is optional and system specific. However, it should always be included if available.</pc:description>
    <pc:name>Intensity</pc:name>
    <pc:interpretation>uint64_t</pc:interpretation>
    <pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:metadata>
    <Metadata name="compression">laz</Metadata>
  </pc:metadata>
</pc:PointCloudSchema>'
);

CREATE TABLE IF NOT EXISTS pa_test_laz_multiple_dim (
    pa PCPATCH(6)
);
\d pa_test_laz_multiple_dim

INSERT INTO pa_test_laz_multiple_dim (pa)
SELECT PC_Patch(PC_MakePoint(6, ARRAY[x,y,z,intensity]))
FROM (
 SELECT
    a*2 AS x,
    a*1.9 AS y,
    a*0.34 AS z,
    10 AS intensity,
    a/400 AS gid
  FROM generate_series(1,1600) AS a
) AS values GROUP BY gid;

SELECT pc_astext(pc_explode(pa)) FROM pa_test_laz_multiple_dim LIMIT 20;

SELECT pc_astext(PC_PointN(pa, 2)) FROM pa_test_laz;

WITH points AS (
  SELECT ARRAY[
    (2^08/256.0*v)*0.01,  -- int8_t
   -(2^08/256.0*v)*0.01,  -- int8_t
    (2^16/256.0*v)*0.01,  -- int16_t
    (2^32/256.0*v)*0.01,  -- int32_t
    (2^64/256.0*v)*0.01,  -- int64_t
    (2^32/256.0*v)*0.01,  -- float
    (2^64/256.0*v)*0.01   -- double
  ] a, v/16 v
  FROM generate_series(-127,127,4) v
), p1 AS (
  SELECT v, PC_Patch(PC_MakePoint(10, a)) p from points -- uncompressed
  GROUP BY v
)
SELECT 'compr' test,
  p1.v, compr, sc,
  PC_AsText(p1.p) =
  PC_AsText(PC_Compress(p1.p, compr,
      array_to_string(
        array_fill(sc,ARRAY[7]),
        ','
      )
    )) ok
FROM p1, ( values
  ('dimensional','rle'),
  ('dimensional','zlib'),
  ('dimensional','sigbits'),
  ('dimensional','auto'),
  ('laz', 'null')
) dimcompr(compr,sc)
ORDER BY compr,sc,v;

TRUNCATE pointcloud_formats;
