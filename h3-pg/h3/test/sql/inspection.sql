\pset tuples_only on

-- neighbouring indexes (one hexagon, one pentagon) at resolution 3
\set invalid '\'0\''
\set hexagon '\'831c02fffffffff\'::h3index'
\set pentagon '\'831c00fffffffff\'::h3index'
\set resolution 3

--
-- TEST h3_get_resolution
--

SELECT h3_get_resolution(:hexagon) = :resolution AND h3_get_resolution(:pentagon) = :resolution;

--
-- TEST h3_get_base_cell_number
--

-- base cell is same for parents
SELECT h3_get_base_cell_number(:hexagon) = h3_get_base_cell_number(h3_cell_to_parent(:hexagon));
SELECT h3_get_base_cell_number(:pentagon) = h3_get_base_cell_number(h3_cell_to_parent(:pentagon));

--
-- TEST h3_is_valid_cell
--

SELECT h3_is_valid_cell(:hexagon) AND h3_is_valid_cell(:pentagon) AND NOT h3_is_valid_cell(:invalid);

--
-- TEST h3_is_res_class_iii
--

-- if index is Class III then parent is not
SELECT h3_is_res_class_iii(:hexagon) AND NOT h3_is_res_class_iii(h3_cell_to_parent(:hexagon));
SELECT h3_is_res_class_iii(:pentagon) AND NOT h3_is_res_class_iii(h3_cell_to_parent(:pentagon));

--
-- TEST h3_is_pentagon
--

SELECT h3_is_pentagon(:pentagon) AND NOT h3_is_pentagon(:hexagon);

--
-- TEST h3_get_icosahedron_faces
--
SELECT h3_get_icosahedron_faces('851c0047fffffff') = ARRAY[11,6];
SELECT h3_get_icosahedron_faces('851c004bfffffff') = ARRAY[6];
