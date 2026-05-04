\pset tuples_only on
\set degs 90.45
\set rads 1.57865030842887
\set epsilon 0.000000001
\set edge '\'1180326b885fffff\'::h3index'

--
-- TEST h3_great_circle_distance
--

\set lyon POINT(4.8422, 45.7597)
\set paris POINT(2.3508, 48.8567)
SELECT h3_great_circle_distance(:lyon, :paris, 'rads') - 0.0615628186794217 < :epsilon;
SELECT h3_great_circle_distance(:lyon, :paris, 'm') - 392217.1598841777 < :epsilon;
SELECT h3_great_circle_distance(:lyon, :paris, 'km') - 392.21715988417765 < :epsilon;

-- test that 'km' is the default unit
SELECT h3_great_circle_distance(:lyon, :paris, 'km') = h3_great_circle_distance(:lyon, :paris);

--
-- TEST h3_get_hexagon_area_avg
--

SELECT abs(h3_get_hexagon_area_avg(10, 'm') - 15047.50190766437) < :epsilon;
SELECT abs(h3_get_hexagon_area_avg(10, 'km') - 0.01504750190766435) < :epsilon;
SELECT h3_get_hexagon_area_avg(10, 'km') = h3_get_hexagon_area_avg(10);

--
-- TEST h3_cell_area
--

\set expected_km2 0.01119834221989390
SELECT abs((h3_cell_area(h3_latlng_to_cell(POINT(0, 0), 10), 'm^2') / 1000000) - :expected_km2) < :epsilon;
SELECT abs(h3_cell_area(h3_latlng_to_cell(POINT(0, 0), 10), 'km^2') - :expected_km2) < :epsilon;
SELECT h3_cell_area(h3_latlng_to_cell(POINT(0, 0), 10), 'rads^2') > 0;

-- default is km^2
SELECT h3_cell_area(h3_latlng_to_cell(POINT(0, 0), 10), 'km^2') = h3_cell_area(h3_latlng_to_cell(POINT(0, 0), 10));

--
-- TEST h3_get_hexagon_edge_length_avg
--

SELECT h3_get_hexagon_edge_length_avg(10, 'm') - 75.86378287 < :epsilon;
SELECT h3_get_hexagon_edge_length_avg(10, 'km') - 0.075863783 < :epsilon;
SELECT h3_get_hexagon_edge_length_avg(10, 'km') = h3_get_hexagon_edge_length_avg(10);

--
-- TEST h3_edge_length
--

SELECT h3_edge_length(:edge, 'rads') > 0;
SELECT h3_edge_length(:edge, 'km') > h3_edge_length(:edge, 'rads');
SELECT h3_edge_length(:edge, 'm') > h3_edge_length(:edge, 'km');

SELECT h3_edge_length(:edge) = h3_edge_length(:edge, 'km');


--
-- TEST h3_get_num_cells
--

SELECT h3_get_num_cells(0) = 122;
SELECT h3_get_num_cells(15) = 569707381193162;

--
-- TEST h3_get_res_0_cells
--

SELECT COUNT(*) = 122 FROM (SELECT h3_get_res_0_cells()) q;

--
-- TEST h3_get_pentagons
--

SELECT COUNT(*) = 12 FROM (SELECT h3_get_pentagons(6)) q; 
