CREATE EXTENSION IF NOT EXISTS MobilityDB CASCADE;

DROP TABLE IF EXISTS feed_info CASCADE;
CREATE TABLE feed_info (
feed_publisher_name text,
feed_publisher_url text,
feed_lang text,
default_lang text default null,
feed_start_date date default null,
feed_end_date date default null,
feed_version text default null,
feed_contact_url text default null,
feed_contact_email text default null
);

DROP TABLE IF EXISTS agency CASCADE;
CREATE TABLE agency (
agency_id text default '',
agency_name text,
agency_url text,
agency_timezone text,
-- optional
agency_lang text,
agency_phone text,
agency_fare_url text,
agency_email text,
bikes_policy_url text,
CONSTRAINT agency_pkey PRIMARY KEY (agency_id)
);

--related to calendar_dates(exception_type)
DROP TABLE IF EXISTS exception_types CASCADE;
CREATE TABLE exception_types (
exception_type int PRIMARY KEY,
description text
);

--related to stops(wheelchair_accessible)
DROP TABLE IF EXISTS wheelchair_accessible CASCADE;
CREATE TABLE wheelchair_accessible (
wheelchair_accessible int PRIMARY KEY,
description text
);

--related to stops(wheelchair_boarding)
DROP TABLE IF EXISTS wheelchair_boardings CASCADE;
CREATE TABLE wheelchair_boardings (
wheelchair_boarding int PRIMARY KEY,
description text
);

DROP TABLE IF EXISTS pickup_dropoff_types CASCADE;
CREATE TABLE pickup_dropoff_types (
type_id int PRIMARY KEY,
description text
);

DROP TABLE IF EXISTS transfer_types CASCADE;
CREATE TABLE transfer_types (
transfer_type int PRIMARY KEY,
description text
);

--related to stops(location_type)
DROP TABLE IF EXISTS location_types CASCADE;
CREATE TABLE location_types (
location_type int PRIMARY KEY,
description text
);

-- related to stop_times(timepoint)
DROP TABLE IF EXISTS timepoints CASCADE;
CREATE TABLE timepoints (
timepoint int PRIMARY KEY,
description text
);

DROP TABLE IF EXISTS continuous_pickup CASCADE;
CREATE TABLE continuous_pickup (
continuous_pickup int PRIMARY KEY,
description text
);

DROP TABLE IF EXISTS continuous_drop_off CASCADE;
CREATE TABLE continuous_drop_off (
continuous_drop_off int PRIMARY KEY,
description text
);

DROP TABLE IF EXISTS calendar CASCADE;
CREATE TABLE calendar (
service_id text,
monday int,
tuesday int,
wednesday int,
thursday int,
friday int,
saturday int,
sunday int,
start_date date,
end_date date,
CONSTRAINT calendar_pkey PRIMARY KEY (service_id)
);

DROP TABLE IF EXISTS levels CASCADE;
CREATE TABLE levels (
level_id text not null,
level_index double precision,
level_name text,
PRIMARY KEY (level_id)
);

DROP TABLE IF EXISTS stops CASCADE;
CREATE TABLE stops (
stop_id text not null,
stop_code text,
stop_name text,
stop_desc text,
stop_lat double precision,
stop_lon double precision,
zone_id text,
stop_url text,
stop_street text,
stop_city text,
stop_region text,
stop_postcode text,
stop_country text,
stop_timezone text,
direction text,
position text,
parent_station text,
wheelchair_boarding integer REFERENCES wheelchair_boardings (wheelchair_boarding),
wheelchair_accessible integer REFERENCES wheelchair_accessible (wheelchair_accessible),
-- optional
location_type integer REFERENCES location_types (location_type),
vehicle_type int,
level_id text,
platform_code text,
stop_geom geometry(point, 4326),
CONSTRAINT stops_pkey PRIMARY KEY (stop_id)
);

-- trigger the_geom update with lat or lon inserted
CREATE OR REPLACE FUNCTION stop_geom_update() RETURNS TRIGGER AS $stop_geom$
BEGIN
NEW.stop_geom = ST_SetSRID(ST_MakePoint(NEW.stop_lon, NEW.stop_lat), 4326);
RETURN NEW;
END;
$stop_geom$ LANGUAGE plpgsql;

CREATE TRIGGER stop_geom_trigger BEFORE INSERT OR UPDATE ON stops
FOR EACH ROW EXECUTE PROCEDURE stop_geom_update();

DROP TABLE IF EXISTS route_types CASCADE;
CREATE TABLE route_types (
route_type int PRIMARY KEY,
description text
);

DROP TABLE IF EXISTS routes CASCADE;
CREATE TABLE routes (
route_id text not null,
agency_id text,
route_short_name text default '',
route_long_name text default '',
route_desc text,
route_type int,
route_url text,
route_color text,
route_text_color text,
route_sort_order integer default null,
continuous_pickup int default null REFERENCES continuous_pickup (continuous_pickup),
continuous_drop_off int default null REFERENCES continuous_drop_off (continuous_drop_off),
CONSTRAINT routes_pkey PRIMARY KEY (route_id)
);

DROP TABLE IF EXISTS calendar_dates CASCADE;
CREATE TABLE calendar_dates (
service_id text not null,
date date not null,
exception_type int REFERENCES exception_types (exception_type),
CONSTRAINT calendar_dates_pkey PRIMARY KEY (service_id, date)
);

DROP TABLE IF EXISTS payment_methods CASCADE;
CREATE TABLE payment_methods (
payment_method int PRIMARY KEY,
description text
);

DROP TABLE IF EXISTS fare_attributes CASCADE;
CREATE TABLE fare_attributes (
fare_id text not null,
price double precision,
currency_type text,
payment_method int REFERENCES payment_methods,
transfers int,
transfer_duration int,
-- unofficial features
agency_id text,
CONSTRAINT fare_attributes_pkey PRIMARY KEY (fare_id)
);

DROP TABLE IF EXISTS fare_rules CASCADE;
CREATE TABLE fare_rules (
fare_id text NOT NULL,
route_id text,
origin_id text,
destination_id text,
contains_id text,
-- unofficial features
service_id text,
CONSTRAINT fare_rules_pkey
PRIMARY KEY (fare_id, route_id, origin_id, destination_id)
);

DROP TABLE IF EXISTS shapes CASCADE;
CREATE TABLE shapes (
shape_id text not null,
shape_pt_lat double precision,
shape_pt_lon double precision,
shape_pt_sequence int not null,
-- optional
shape_dist_traveled double precision,
CONSTRAINT shapes_pk PRIMARY KEY (shape_id, shape_pt_sequence)
);

-- Create new table to store the shape geometries
DROP TABLE IF EXISTS shape_geoms CASCADE;
CREATE TABLE shape_geoms (
shape_id text not null,
shape_geom geometry(LineString, 4326),
CONSTRAINT shape_geom_pkey PRIMARY KEY (shape_id)
);

DROP TABLE IF EXISTS trips CASCADE;
CREATE TABLE trips (
route_id text,
service_id text,
trip_id text not null,
trip_headsign text,
direction_id int,
block_id text,
shape_id text,
trip_short_name text,
wheelchair_accessible int REFERENCES wheelchair_accessible(wheelchair_accessible),
-- unofficial features
direction text,
schd_trip_id text,
trip_type text,
exceptional int,
bikes_allowed int,
CONSTRAINT trips_pkey PRIMARY KEY (trip_id)
);

DROP TABLE IF EXISTS stop_times CASCADE;
CREATE TABLE stop_times (
trip_id text not null,
-- Check that casting to time interval works.
-- Interval used rather than Time because: 
-- "For times occurring after midnight on the service day, 
-- enter the time as a value greater than 24:00:00" 
-- https://developers.google.com/transit/gtfs/reference#stop_timestxt
-- conversion tool: https://github.com/Bus-Data-NYC/nyc-bus-stats/blob/master/sql/util.sql#L48
arrival_time interval CHECK (arrival_time::interval = arrival_time::interval),
departure_time interval CHECK (departure_time::interval = departure_time::interval),
stop_id text,
stop_sequence int not null,
stop_headsign text,
pickup_type int REFERENCES pickup_dropoff_types(type_id),
drop_off_type int REFERENCES pickup_dropoff_types(type_id),
continuous_pickup int default null REFERENCES continuous_pickup (continuous_pickup),
continuous_drop_off int default null REFERENCES continuous_drop_off (continuous_drop_off),
shape_dist_traveled numeric(10, 2),
timepoint int REFERENCES timepoints (timepoint),

-- unofficial features
-- the following are not in the spec
arrival_time_seconds int default null,
departure_time_seconds int default null,
CONSTRAINT stop_times_pkey PRIMARY KEY (trip_id, stop_sequence)
);

DROP TABLE IF EXISTS frequencies CASCADE;
CREATE TABLE frequencies (
trip_id text,
start_time text not null CHECK (start_time::interval = start_time::interval),
end_time text not null CHECK (end_time::interval = end_time::interval),
headway_secs int not null,
exact_times int,
start_time_seconds int,
end_time_seconds int,
CONSTRAINT frequencies_pkey PRIMARY KEY (trip_id, start_time)
);

DROP TABLE IF EXISTS transfers CASCADE;
CREATE TABLE transfers (
from_stop_id text not null,
to_stop_id text not null,
transfer_type int REFERENCES transfer_types(transfer_type),
min_transfer_time int,
-- Unofficial fields
from_route_id text default null,
to_route_id text default null,
service_id text default null
);

DROP TABLE IF EXISTS pathway_modes CASCADE;
CREATE TABLE pathway_modes (
pathway_mode integer PRIMARY KEY,
description text
);

DROP TABLE IF EXISTS pathways CASCADE;
CREATE TABLE pathways (
pathway_id text not null,
from_stop_id text,
to_stop_id text,
pathway_mode integer REFERENCES pathway_modes (pathway_mode),
is_bidirectional integer,
length double precision,
traversal_time integer,
stair_count integer,
max_slope numeric,
min_width double precision,
signposted_as text,
reversed_signposted_as text,
PRIMARY KEY (pathway_id)
);

DROP TABLE IF EXISTS translations CASCADE;
CREATE TABLE translations (
table_name text not null,
field_name text,
language text not null,
translation text,
record_id text,
record_sub_id text,
field_value text,
PRIMARY KEY (table_name, field_value, language)
);

DROP TABLE IF EXISTS attributions CASCADE;
CREATE TABLE attributions (
attribution_id text,
agency_id text,
route_id text,
trip_id text,
organization_name text,
is_producer boolean,
is_operator boolean,
is_authority boolean,
attribution_url text,
attribution_email text,
attribution_phone text,
PRIMARY KEY (attribution_id)
);

insert into exception_types (exception_type, description) values 
  (1, 'service has been added'),
  (2, 'service has been removed');

insert into transfer_types (transfer_type, description) VALUES
  (0,'Preferred transfer point'),
  (1,'Designated transfer point'),
  (2,'Transfer possible with min_transfer_time window'),
  (3,'Transfers forbidden');

insert into location_types(location_type, description) values 
  (0,'stop'),
  (1,'station'),
  (2,'station entrance'),
  (3,'generic node'),
  (4,'boarding area');

insert into wheelchair_boardings(wheelchair_boarding, description) values
   (0, 'No accessibility information available for the stop'),
   (1, 'At least some vehicles at this stop can be boarded by a rider in a wheelchair'),
   (2, 'Wheelchair boarding is not possible at this stop');

insert into wheelchair_accessible(wheelchair_accessible, description) values
  (0, 'No accessibility information available for this trip'),
  (1, 'The vehicle being used on this particular trip can accommodate at least one rider in a wheelchair'),
  (2, 'No riders in wheelchairs can be accommodated on this trip');

insert into pickup_dropoff_types (type_id, description) values
  (0,'Regularly Scheduled'),
  (1,'Not available'),
  (2,'Phone arrangement only'),
  (3,'Driver arrangement only');

insert into payment_methods (payment_method, description) values
  (0,'On Board'),
  (1,'Prepay');

insert into timepoints (timepoint, description) values
  (0, 'Times are considered approximate'),
  (1, 'Times are considered exact');

insert into continuous_pickup (continuous_pickup, description) values
  (0, 'Continuous stopping pickup'),
  (1, 'No continuous stopping pickup'),
  (2, 'Must phone agency to arrange continuous stopping pickup'),
  (3, 'Must coordinate with driver to arrange continuous stopping pickup');

insert into continuous_drop_off (continuous_drop_off, description) values
  (0, 'Continuous stopping drop-off'),
  (1, 'No continuous stopping drop-off'),
  (2, 'Must phone agency to arrange continuous stopping drop-off'),
  (3, 'Must coordinate with driver to arrange continuous stopping drop-off');

insert into pathway_modes (pathway_mode, description) values
  (1, 'walkway'),
  (2, 'stairs'),
  (3, 'moving sidewalk/travelator'),
  (4, 'escalator'),
  (5, 'elevator'),
  (6, 'fare gate (or payment gate)'),
  (7, 'exit gate');


