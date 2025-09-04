CREATE EXTENSION IF NOT EXISTS MobilityDB;

/*
Function that returns a file header given the file path
*/
DROP FUNCTION IF EXISTS find_header(filepath text);
CREATE OR REPLACE FUNCTION find_header(filepath text) RETURNS text AS $$
DECLARE
	content_text TEXT;
	first_pos INTEGER;
	head TEXT;
BEGIN


	SELECT pg_read_file(filepath) INTO content_text;
	SELECT position(E'\n' in content_text) INTO first_pos;
	SELECT left(content_text, first_pos) INTO head;
	
	RETURN head;
	
	END;
$$ LANGUAGE 'plpgsql';


/*
Function that loads a complete gtfs static feed into SQL tables
*/
DROP FUNCTION IF EXISTS gtfs_static_load(fullpath TEXT);
CREATE OR REPLACE FUNCTION gtfs_static_load(fullpath TEXT) 
RETURNS TEXT AS $$

DECLARE
	head TEXT;
	tablename TEXT;
	field TEXT;
	feed_idx INTEGER;
BEGIN
	FOREACH tablename IN ARRAY ARRAY['feed_info', 'stop_times', 'trips', 'routes', 'calendar_dates', 'calendar', 'shapes', 'stops', 'transfers', 'frequencies',	'attributions', 'pathways', 'levels', 'fare_attributes', 'fare_rules', 'agency'] LOOP
		BEGIN
			-- Find the file header
			SELECT find_header(format('%s%s.txt', fullpath, tablename)) INTO head;
			
			-- Add column if column specified in header does not exist
			FOREACH field IN ARRAY string_to_array(head, ',') LOOP
				EXECUTE format('ALTER TABLE %s ADD COLUMN IF NOT EXISTS %s TEXT;', tablename, field); 
			END LOOP;
			RAISE NOTICE 'treating file % % %', fullpath, tablename, head;
			-- Insert into table
			EXECUTE format('COPY %s(%s) FROM ''%s%s.txt'' DELIMITER '','' CSV HEADER', tablename, head, fullpath, tablename);
			EXCEPTION
				WHEN SQLSTATE '42601' THEN
				RAISE NOTICE 'file not found, ignoring % % %', fullpath, tablename, head;
				WHEN SQLSTATE '58P01' THEN
				RAISE NOTICE 'file not found, ignoring % % %', fullpath, tablename, head;
		END;
	END LOOP;
	
	-- Geometries set up
	INSERT INTO shape_geoms
	SELECT shape_id, ST_MakeLine(array_agg(
	ST_SetSRID(ST_MakePoint(shape_pt_lon, shape_pt_lat), 4326) ORDER BY shape_pt_sequence))
	FROM shapes
	GROUP BY shape_id;

	UPDATE stops
	SET stop_geom = ST_SetSRID(ST_MakePoint(stop_lon, stop_lat), 4326);

  	RETURN 'Tables successfully loaded';
END;
$$ LANGUAGE 'plpgsql';

-------------------------------------------------------------------------------


SELECT gtfs_static_load('/mnt/c/Users/ossama/Documents/memoire ilias/MobilityDB-PublicTransport-master/GTFS Static/gtfs/');
