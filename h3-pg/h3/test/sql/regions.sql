\pset tuples_only on
-- res 0 index
\set res0index '\'8059fffffffffff\''
-- center hex
\set center '\'81583ffffffffff\''
-- 7 child hexes in res 0 index
\set solid 'ARRAY(SELECT h3_cell_to_children(:res0index, 1))'
-- 6 child hexes in rim of res 0 index
\set hollow 'array_remove(:solid, :center)'
-- pentagon
\set pentagon '\'831c00fffffffff\'::h3index'

--
-- TEST h3_polygon_to_cells and h3_cells_to_multi_polygon
--

-- h3_polygon_to_cells is inverse of h3_cells_to_multi_polygon for set without holes
SELECT array_agg(result) is null FROM (
    SELECT h3_polygon_to_cells(exterior, holes, 1) result FROM (
        SELECT exterior, holes FROM h3_cells_to_multi_polygon(:solid)
    ) qq
    EXCEPT SELECT unnest(:solid) result
) q;

-- h3_polygon_to_cells is inverse of h3_cells_to_multi_polygon for set with a hole
SELECT array_agg(result) is null FROM (
    SELECT h3_polygon_to_cells(exterior, holes, 1) result FROM (
        SELECT exterior, holes FROM h3_cells_to_multi_polygon(:hollow)
    ) qq
    EXCEPT SELECT unnest(:hollow) result
) q;

-- h3_polyfill doesn't segfault on NULL value in holes
SELECT TRUE FROM (
    SELECT h3_polygon_to_cells(exterior, ARRAY[NULL::POLYGON], 1) result FROM (
        SELECT exterior, holes FROM h3_cells_to_multi_polygon(
            ARRAY[:pentagon]::H3Index[]
        )
    ) qq
) q LIMIT 1;

-- h3_polyfill throws on non-polygons
CREATE FUNCTION h3_test_polyfill_bad1() RETURNS boolean LANGUAGE PLPGSQL
    AS $$
        BEGIN
            PERFORM h3_polyfill(ST_GeomFromText('POINT(-71.160281 42.258729)',4326), 8);
            RETURN false;
        EXCEPTION WHEN OTHERS THEN
            RETURN true;
        END;
    $$;
CREATE FUNCTION h3_test_polyfill_bad2() RETURNS boolean LANGUAGE PLPGSQL
    AS $$
        BEGIN
            PERFORM h3_polyfill(ST_GeomFromText('LINESTRING(-71.160281 42.258729,-71.160837 42.259113,-71.161144 42.25932)',4326), 8);
            RETURN false;
        EXCEPTION WHEN OTHERS THEN
            RETURN true;
        END;
    $$;
SELECT h3_test_polyfill_bad1();
SELECT h3_test_polyfill_bad2();
DROP FUNCTION h3_test_polyfill_bad1;
DROP FUNCTION h3_test_polyfill_bad2;

--
-- TEST h3_polygon_to_cells_experimental
--

-- h3_polygon_to_cells is inverse of h3_cells_to_multi_polygon for set without holes
SELECT array_agg(result) is null FROM (
    SELECT h3_polygon_to_cells_experimental(exterior, holes, 1, 'overlapping') result FROM (
        SELECT exterior, holes FROM h3_cells_to_multi_polygon(:solid)
    ) qq
    EXCEPT SELECT h3_grid_disk(h3_cell_to_center_child(:res0index), 2) result
) q;
