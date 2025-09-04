DELETE FROM shape_geoms
WHERE NOT ST_Intersects(shape_geom, ST_MakeEnvelope(4.2146, 50.6836, 4.4912, 50.9289, 4326));

DELETE FROM stops
WHERE NOT ST_Intersects(stop_geom, ST_MakeEnvelope(4.2146, 50.6836, 4.4912, 50.9289, 4326));

CREATE TABLE tmp_trips AS (
	SELECT distinct t.*
  	FROM shape_geoms AS s
 	INNER JOIN trips AS t ON s.shape_id = t.shape_id
);
DROP TABLE trips;
ALTER TABLE tmp_trips RENAME TO trips;


CREATE TABLE tmp_stop_times AS (
    SELECT distinct s.*
    FROM stop_times s
    INNER JOIN trips t ON t.trip_id = s.trip_id
);
DROP TABLE stop_times;
ALTER TABLE tmp_stop_times RENAME TO stop_times;

CREATE TABLE tmp_calendar AS (
	SELECT DISTINCT c.* FROM calendar c
	INNER JOIN trips t ON c.service_id = t.service_id
);
DROP TABLE calendar;
ALTER TABLE tmp_calendar RENAME TO calendar;

CREATE TABLE tmp_calendar_dates AS (
	SELECT DISTINCT c.* FROM calendar_dates c
	INNER JOIN trips t ON c.service_id = t.service_id
);
DROP TABLE calendar_dates;
ALTER TABLE tmp_calendar_dates RENAME TO calendar_dates;