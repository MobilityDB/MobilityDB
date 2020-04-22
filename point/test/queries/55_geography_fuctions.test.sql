-------------------------------------------------------------------------------

-- Function added to PostGIS to interpolate a point along a geography line
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.0, false, true));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 1.0, false, true));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.1, false, false));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.1, false, true));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.1, true, true));
-- EMPTY
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring empty', 0.1, false, true));
/* Errors */
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 2, false, true));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Point(4.35 50.85)', 0.5, false, true));

-------------------------------------------------------------------------------