\pset tuples_only on
\set resolution 10
\set hexagon '\'8a63a9a99047fff\''
\set degree ST_SetSRID(ST_Point(55.6677199224442,12.592131261648213), 4326)

SELECT h3_lat_lng_to_cell(:degree, :resolution) = '8a63a9a99047fff';
SELECT h3_lat_lng_to_cell(h3_cell_to_geometry(:hexagon), :resolution) = '8a63a9a99047fff';
