\pset tuples_only on

-- neighbouring indexes (one hexagon, one pentagon) at resolution 3
\set geo POINT(-144.52399108028, 49.7165031828995)
\set hexagon '\'831c02fffffffff\'::h3index'
\set resolution 3

SELECT h3_cell_to_lat_lng(:hexagon) ~= :geo;
SELECT h3_lat_lng_to_cell(:geo, :resolution) = :hexagon;
