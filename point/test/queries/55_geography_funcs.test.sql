-------------------------------------------------------------------------------

SELECT ST_AsText(st_lineinterpolatepoint(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.0));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.0, true));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 1.0, false));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.1, true));
-- EMPTY
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring empty', 0.1, true));
/* Errors */
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Point(4.35 50.85)', 0.5, true));
SELECT ST_AsText(st_lineinterpolatepoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 2, true));

-------------------------------------------------------------------------------