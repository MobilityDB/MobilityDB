-- The extension reads a PostGIS raster through its own copy of the PostGIS
-- raster core. This file loads postgis_raster BEFORE the extension: its first
-- statement calls PostGIS alone, so postgis_raster, and the GDAL and librttopo
-- it links, are in the session when the extension's library loads. librttopo
-- exports rtalloc, rtrealloc and rterror, the names of the raster core's
-- allocators, so the band count below is read by the extension's own raster
-- core only while the extension does not export those names. No statement
-- calling the extension may precede the first one.
SELECT ST_Width(ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0, 4326));

SELECT numBands(ST_AddBand(ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0,
  0.0, 4326), '32BF'::text, 0.0::float8, NULL::float8));
