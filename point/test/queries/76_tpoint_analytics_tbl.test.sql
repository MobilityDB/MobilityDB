-------------------------------------------------------------------------------

-- set parallel_tuple_cost=0;
-- set parallel_setup_cost=0;
set force_parallel_mode=regress;

--------------------------------------------------------

SELECT st_astext(temp::geometry) FROM tbl_tgeompoint LIMIT 10;
SELECT st_astext(temp::geometry) FROM tbl_tgeompoint3D LIMIT 10;

SELECT temp::geometry FROM tbl_tgeompoint LIMIT 10;
SELECT temp::geometry FROM tbl_tgeompoint3D LIMIT 10;

SELECT st_astext(temp::geography) FROM tbl_tgeogpoint LIMIT 10;
SELECT st_astext(temp::geography) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT temp::geography FROM tbl_tgeogpoint LIMIT 10;
SELECT temp::geography FROM tbl_tgeogpoint3D LIMIT 10;

--------------------------------------------------------

SELECT st_astext(asGeometry(temp, true)) FROM tbl_tgeompoint LIMIT 10;
SELECT st_astext(asGeometry(temp, true)) FROM tbl_tgeompoint3D LIMIT 10;

SELECT asGeometry(temp, true) FROM tbl_tgeompoint LIMIT 10;
SELECT asGeometry(temp, true) FROM tbl_tgeompoint3D LIMIT 10;

SELECT st_astext(asGeography(temp, true)) FROM tbl_tgeogpoint LIMIT 10;
SELECT st_astext(asGeography(temp, true)) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT asGeography(temp, true) FROM tbl_tgeogpoint LIMIT 10;
SELECT asGeography(temp, true) FROM tbl_tgeogpoint3D LIMIT 10;

-------------------------------------------------------------------------------

SELECT asText((temp::geometry)::tgeompoint) FROM tbl_tgeompoint LIMIT 10;
SELECT asText((temp::geometry)::tgeompoint) FROM tbl_tgeompoint3D LIMIT 10;

SELECT count(*) FROM tbl_tgeompoint WHERE (temp::geometry)::tgeompoint = temp;
SELECT count(*) FROM tbl_tgeompoint3D WHERE (temp::geometry)::tgeompoint = temp;

SELECT asText((temp::geography)::tgeogpoint) FROM tbl_tgeogpoint LIMIT 10;
SELECT asText((temp::geography)::tgeogpoint) FROM tbl_tgeogpoint3D LIMIT 10;

SELECT (temp::geography)::tgeogpoint FROM tbl_tgeogpoint LIMIT 10;
SELECT (temp::geography)::tgeogpoint FROM tbl_tgeogpoint3D LIMIT 10;

-------------------------------------------------------------------------------

SELECT st_astext(geoMeasure(t1.temp, t2.temp)) FROM tbl_tgeompoint t1, tbl_tfloat t2 WHERE getTime(t1.temp) && getTime(t2.temp);
SELECT st_astext(geoMeasure(t1.temp, t2.temp)) FROM tbl_tgeompoint3D t1, tbl_tfloat t2 WHERE getTime(t1.temp) && getTime(t2.temp);

SELECT st_astext(geoMeasure(temp, round(speed(temp),2))) FROM tbl_tgeompoint WHERE speed(temp) IS NOT NULL;
SELECT st_astext(geoMeasure(temp, round(speed(temp),2))) FROM tbl_tgeompoint3D WHERE speed(temp) IS NOT NULL;

-------------------------------------------------------------------------------

SELECT MAX(numInstants(simplify(temp, 4))) FROM tbl_tfloat;
SELECT MAX(numInstants(simplify(temp, 4))) FROM tbl_tgeompoint;

-------------------------------------------------------------------------------

-- set parallel_tuple_cost=100;
-- set parallel_setup_cost=100;
set force_parallel_mode=off;

-------------------------------------------------------------------------------
