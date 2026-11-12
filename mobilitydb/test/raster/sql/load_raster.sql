-- Verify that the postgis_raster extension is loaded.
-- It is pulled in by the mobilitydb CASCADE in the test_setup step when
-- MobilityDB is built with -DQUADBIN=ON -DRASTER=ON.
SELECT COUNT(*) = 1 AS postgis_raster_loaded
FROM pg_extension
WHERE extname = 'postgis_raster';
