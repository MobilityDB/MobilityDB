\pset tuples_only on

\set hexagon '\'880326b885fffff\'::h3index'
\set pentagon '\'831c00fffffffff\'::h3index'
\set vertex2 '\'2280326b885fffff\'::h3index'
\set geo POINT(65.60200108645547,89.57740563247555)

--
-- TEST h3_cell_to_vertex
--

SELECT h3_cell_to_vertex(:hexagon, 2) = :vertex2;

--
-- TEST h3_cell_to_vertexes
--

SELECT COUNT(*) = 6 FROM (
    SELECT h3_cell_to_vertexes(:hexagon)
) q;

SELECT COUNT(*) = 5 FROM (
    SELECT h3_cell_to_vertexes(:pentagon)
) q;

--
-- TEST h3_vertex_to_latlng
--
 
SELECT h3_vertex_to_latlng(:vertex2) ~= :geo;

--
-- TEST h3_is_valid_vertex and
--

SELECT h3_is_valid_vertex(:vertex2);
SELECT NOT h3_is_valid_vertex(:hexagon);