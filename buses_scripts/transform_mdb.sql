DROP FUNCTION IF EXISTS create_from_calendar();
CREATE OR REPLACE FUNCTION create_from_calendar() RETURNS void AS $$
BEGIN
	DROP TABLE IF EXISTS service_dates;
	CREATE TABLE service_dates AS (
		SELECT service_id, date_trunc('day', d)::date AS date
		FROM calendar c, generate_series(start_date, end_date, '1 day'::interval) AS d
		WHERE (
			(monday = 1 AND extract(isodow FROM d) = 1) OR
			(tuesday = 1 AND extract(isodow FROM d) = 2) OR
			(wednesday = 1 AND extract(isodow FROM d) = 3) OR
			(thursday = 1 AND extract(isodow FROM d) = 4) OR
			(friday = 1 AND extract(isodow FROM d) = 5) OR
			(saturday = 1 AND extract(isodow FROM d) = 6) OR
			(sunday = 1 AND extract(isodow FROM d) = 7)
		)
		EXCEPT
		SELECT service_id, date
		FROM calendar_dates WHERE exception_type = 2
		UNION
		SELECT service_id, date
		FROM calendar_dates
		WHERE exception_type = 1
	);
END;
$$ LANGUAGE 'plpgsql';


DROP FUNCTION IF EXISTS create_from_calendar_dates();
CREATE OR REPLACE FUNCTION create_from_calendar_dates() RETURNS void AS $$
BEGIN
	DROP TABLE IF EXISTS service_dates;
	CREATE TABLE service_dates AS (
		SELECT service_id, date_trunc('day', date)::date AS date
		FROM calendar_dates d
		WHERE exception_type = 1
	);
END;
$$ LANGUAGE 'plpgsql';


DROP FUNCTION IF EXISTS transform_gtfs_mdb();
CREATE OR REPLACE FUNCTION transform_gtfs_mdb() RETURNS text AS $$
DECLARE
	calendar_cnt integer;
BEGIN
	SELECT COUNT(*) FROM calendar INTO calendar_cnt;
	IF calendar_cnt > 0 THEN
		PERFORM create_from_calendar();
	ELSE
		PERFORM create_from_calendar_dates();
	END IF;
	
	-- Uncomment if you need to reduce to a given date
    DELETE FROM service_dates WHERE date <> '2025-07-07'; -- date à modifier selon la date présente dans le fichier calendar
	DELETE FROM service_dates WHERE
	(service_id, date) IN (
		SELECT service_id, date
		FROM service_dates
		GROUP BY service_id, date
		HAVING COUNT(*) > 1
	)
	AND ctid NOT IN (
		SELECT MIN(ctid)
		FROM service_dates
		GROUP BY service_id, date
		HAVING COUNT(*) > 1
	);
	
	RAISE NOTICE 'updating trip_positions...';
	DROP TABLE IF EXISTS trip_positions;
	CREATE TABLE trip_positions
	(
	  trip_id text,
	  stop_sequence integer,
	  no_stops integer,
	  route_id text,
	  service_id text,
	  shape_id text,
	  stop_id text,
	  arrival_time interval,
	  perc float
	);
	RAISE NOTICE 'debug1 OK';
	INSERT INTO trip_positions (trip_id, stop_sequence, no_stops, route_id, service_id,
		shape_id, stop_id, arrival_time) (
	SELECT t.trip_id, stop_sequence,
		MAX(stop_sequence) OVER (PARTITION BY t.trip_id),
		route_id, t.service_id, t.shape_id, st.stop_id, arrival_time
	FROM trips t JOIN stop_times st ON t.trip_id = st.trip_id
    JOIN service_dates sd ON t.service_id = sd.service_id
	);

	RAISE NOTICE 'debug2 OK';
	UPDATE trip_positions t
	SET perc = CASE
		WHEN stop_sequence =  1 then 0::float
		WHEN stop_sequence =  no_stops then 1.0::float
		ELSE ST_LineLocatePoint(shape_geom, stop_geom)
	END
	FROM shape_geoms g, stops s
	WHERE t.shape_id = g.shape_id
	AND t.stop_id = s.stop_id;


	RAISE NOTICE 'trip_positions OK';
	RAISE NOTICE 'updating trip_segs...';
	DROP TABLE IF EXISTS trip_segs;
	CREATE TABLE trip_segs (
		trip_id text,
		route_id text,
		service_id text,
		stop1_sequence integer,
		stop2_sequence integer,
		no_stops integer,
		shape_id text,
		stop1_arrival_time interval,
		stop2_arrival_time interval,
		perc1 float,
		perc2 float,
		seg_geom geometry,
		seg_length float,
		no_points integer
	);

	INSERT INTO trip_segs (trip_id, route_id, service_id, stop1_sequence, stop2_sequence,
		no_stops, shape_id, stop1_arrival_time, stop2_arrival_time, perc1, perc2)
	WITH temp AS (
		SELECT t.trip_id, t.route_id, t.service_id, t.stop_sequence,
			LEAD(stop_sequence) OVER w AS stop_sequence2,
			MAX(stop_sequence) OVER (PARTITION BY trip_id),
			t.shape_id, t.arrival_time, LEAD(arrival_time) OVER w,
			t.perc, LEAD(perc) OVER w
		FROM trip_positions t WINDOW w AS (PARTITION BY trip_id ORDER BY stop_sequence)
	)
	SELECT * FROM temp WHERE stop_sequence2 IS NOT null;

	UPDATE trip_segs t
	SET seg_geom =
	   (CASE WHEN perc1 > perc2 THEN seg_geom
		ELSE ST_LineSubstring(shape_geom, perc1, perc2)
		END)
	FROM shape_geoms g
	WHERE t.shape_id = g.shape_id;


	UPDATE trip_segs t
	SET seg_length = ST_Length(seg_geom), no_points = ST_NumPoints(seg_geom);



	RAISE NOTICE 'trip_segs OK';
	RAISE NOTICE 'updating trip_points...';
	DROP TABLE IF EXISTS trip_points;
	CREATE TABLE trip_points (
		trip_id text,
		route_id text,
		service_id text,
		stop1_sequence integer,
		point_sequence integer,
		point_geom geometry,
		point_arrival_time interval,
		PRIMARY KEY (trip_id, stop1_sequence, point_sequence)
	);

	-- deconnexion si l'insérsion est faite ainsi , il faut utiliser une boucle for et itterer cette insersion pour chaque trip_id
	INSERT INTO trip_points (trip_id, route_id, service_id, stop1_sequence,
		point_sequence, point_geom, point_arrival_time)
	WITH temp1 AS (
		SELECT trip_id, route_id, service_id, stop1_sequence,
			stop2_sequence, no_stops, stop1_arrival_time, stop2_arrival_time, seg_length,
			(dp).path[1] AS point_sequence, no_points, (dp).geom as point_geom
		FROM trip_segs, ST_DumpPoints(seg_geom) AS dp
	),
	temp2 AS (
		SELECT trip_id, route_id, service_id, stop1_sequence,
			stop1_arrival_time, stop2_arrival_time, seg_length,  point_sequence,
			no_points, point_geom
		FROM temp1
		WHERE (point_sequence <> no_points OR stop2_sequence = no_stops) and temp1.seg_length <> 0
	),
	temp3 AS (
		SELECT trip_id, route_id, service_id, stop1_sequence,
			stop1_arrival_time, stop2_arrival_time, point_sequence, no_points, point_geom,
			ST_Length(ST_Makeline(array_agg(point_geom) OVER w)) / seg_length AS perc
		FROM temp2 WINDOW w AS (PARTITION BY trip_id, service_id, stop1_sequence
			ORDER BY point_sequence)
	)
	SELECT trip_id, route_id, service_id, stop1_sequence,
		point_sequence, point_geom,
		CASE
		WHEN point_sequence = 1 then stop1_arrival_time
		WHEN point_sequence = no_points then stop2_arrival_time
		ELSE stop1_arrival_time + ((stop2_arrival_time - stop1_arrival_time) * perc)
		END AS point_arrival_time
	FROM temp3;



	RAISE NOTICE 'trip_points OK';
	RAISE NOTICE 'updating trips_input...';
	DROP TABLE IF EXISTS trips_input;
	CREATE TABLE trips_input (
		trip_id text,
		route_id text,
		service_id text,
		date date,
		point_geom geometry,
		t timestamptz
	);


	INSERT INTO trips_input
	SELECT trip_id, route_id, t.service_id,
		date, point_geom, date + point_arrival_time AS t
	FROM trip_points t JOIN service_dates s ON t.service_id = s.service_id;
	
	CREATE INDEX idx_trips_input ON trips_input (trip_id, route_id, t);


	DELETE FROM trips_input WHERE
	(trip_id, route_id, t) IN (
		SELECT trip_id, route_id, t
		FROM trips_input
		GROUP BY trip_id, route_id, t
		HAVING COUNT(*) > 1
	)
	AND ctid NOT IN (
		SELECT MIN(ctid)
		FROM trips_input
		GROUP BY trip_id, route_id, t
		HAVING COUNT(*) > 1
	);



	RAISE NOTICE 'trip_points OK';
	RAISE NOTICE 'updating trips_mdb...';
	DROP TABLE IF EXISTS trips_mdb;
	CREATE TABLE trips_mdb (
		trip_id text NOT NULL,
		route_id text NOT NULL,
		date date NOT NULL,
		trip tgeompoint,
		PRIMARY KEY (trip_id, date)
	);

	
	INSERT INTO trips_mdb(trip_id, route_id, date, trip)
	SELECT trip_id, route_id, date, tgeompointseq(array_agg(tgeompoint(point_geom, t) ORDER BY T)) -- à vérifier avec ilias
	FROM trips_input
	GROUP BY trip_id, route_id, date;


	ALTER TABLE trips_mdb ADD COLUMN traj geometry;
	UPDATE trips_mdb
	SET Traj = trajectory(Trip);

	ALTER TABLE trips_mdb ADD COLUMN starttime timestamp;
	UPDATE trips_mdb SET starttime = startTimestamp(trip);
	RAISE NOTICE 'SUCCESS : Results in table trips_mdb';

	
	RETURN 'transformation success';
END;
$$ LANGUAGE 'plpgsql';

SELECT transform_gtfs_mdb();
