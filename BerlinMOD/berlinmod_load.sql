/******************************************************************************
 * Loads the BerlinMOD data with WGS84 coordinates in CSV format 
 * http://dna.fernuni-hagen.de/secondo/BerlinMOD/BerlinMOD.html  
 * into MobilityDB using projected (2D) coordinates with SRID 5676
 * https://epsg.io/5676
 * Parameters:
 *		fullpath: states the full path in which the CSV files are located.
 *		gist: states whether GiST or SP-GiST indexes are created on the tables.
 *			By default it is set to TRUE. 
 * Example of usage:
 * 		CREATE EXTENSION mobilitydb CASCADE;
 * 		<Create the function>
 * 		SELECT berlinmod_load('/home/mobilitydb/berlinmod_0.005/', false)
 *****************************************************************************/

DROP FUNCTION IF EXISTS berlinmod_load(fullpath text, gist bool);
CREATE OR REPLACE FUNCTION berlinmod_load(fullpath text, gist bool DEFAULT TRUE) 
RETURNS text AS $$
BEGIN
	DROP TABLE IF EXISTS streets;
	CREATE TABLE streets
	(
		StreetId integer,
		vmax integer,
		x1 double precision,
		y1 double precision,
		x2 double precision,
		y2 double precision,
		geom geometry(LineString,5676)
	);
	EXECUTE format('COPY streets(StreetId, vmax, x1, y1, x2, y2) FROM ''%sstreets.csv'' DELIMITER '','' CSV HEADER', fullpath);
	UPDATE streets
	SET geom = ST_Transform(ST_SetSRID(ST_MakeLine(ARRAY[ST_MakePoint(x1,y1),ST_MakePoint(x2,y2)]),4326),5676);

	DROP TABLE IF EXISTS Points CASCADE;
	CREATE TABLE Points
	(
		PointId integer,
		PosX double precision,
		PosY double precision,
		geom geometry(Point,5676)
	);
	EXECUTE format('COPY Points(PointId, PosX, PosY) FROM ''%squerypoints.csv'' DELIMITER  '','' CSV HEADER', fullpath);
	UPDATE Points
	SET geom = ST_Transform(ST_SetSRID(ST_MakePoint(PosX, PosY),4326),5676);

	IF gist THEN
		CREATE INDEX Points_geom_gist_idx ON Points USING gist(geom);
	ELSE
		CREATE INDEX Points_geom_spgist_idx ON Points USING spgist(geom);
	END IF;
	
	/* There are duplicate points in Points
	SELECT count(*)
	FROM Points P1, Points P2
	where P1.PointId < P2.PointId AND
	P1.PosX = P2.PosX AND P1.PosY = P2.PosY
	-- 4
	*/

	/* Remove duplicates in Points
	DELETE FROM Points Q1
	WHERE EXISTS (SELECT * FROM Points Q2 
	WHERE Q1.PointId < Q2.PointId AND Q1.Geom = Q2.Geom );
	-- SELECT COUNT(*) FROM Points;
	-- 96
	*/
	
	CREATE VIEW Points1 (PointId, PosX, PosY, geom) AS
	SELECT PointId, PosX, PosY, geom
	FROM Points
	LIMIT 10;

	DROP TABLE IF EXISTS RegionsInput CASCADE;
	CREATE TABLE RegionsInput
	(
		RegionId integer,
		SegNo integer,
		XStart double precision,
		YStart double precision,
		XEnd double precision,
		YEnd double precision
	);
	EXECUTE format('COPY RegionsInput(RegionId, SegNo, XStart, YStart, XEnd, YEnd) FROM ''%squeryregions.csv'' DELIMITER  '','' CSV HEADER', fullpath);
	
	DROP TABLE IF EXISTS Regions CASCADE;
	CREATE TABLE Regions
	(
		RegionId integer,
		geom Geometry(Polygon,5676)
	);
	INSERT INTO Regions (RegionId, geom)
	WITH RegionsSegs AS
	(
		SELECT RegionId, SegNo,
		ST_Transform(ST_SetSRID(St_MakeLine(ST_MakePoint(XStart, YStart), ST_MakePoint(XEnd, YEnd)),4326),5676) AS geom
		FROM RegionsInput
	)
	SELECT RegionId, ST_Polygon(ST_LineMerge(ST_Union(geom order by SegNo)),5676) AS geom
	FROM RegionsSegs
	GROUP BY RegionId;	

	IF gist THEN
		CREATE INDEX Regions_geom_gist_idx ON Regions USING spgist (geom);
	ELSE
		CREATE INDEX Regions_geom_spgist_idx ON Regions USING spgist (geom);
	END IF;

	CREATE VIEW Regions1 (RegionId, geom) AS
	SELECT RegionId, geom
	FROM Regions
	LIMIT 10;
	
	DROP TABLE IF EXISTS Instants CASCADE;
	CREATE TABLE Instants
	(
		InstantId integer,
		Instant timestamptz
	);
	EXECUTE format('COPY Instants(InstantId, Instant) FROM ''%squeryinstants.csv'' DELIMITER  '','' CSV HEADER', fullpath);

	CREATE INDEX Instants_instant_btree_idx ON Instants USING btree (instant);
  
	/* There are NO duplicate instants in Instants
	SELECT count(*)
	FROM Instants I1, Instants I2
	where I1.InstantId < I2.InstantId AND
	I1.instant = I2.instant
	*/

	CREATE VIEW Instants1 (InstantId, Instant) AS
	SELECT InstantId, Instant 
	FROM Instants
	LIMIT 10;
	
	DROP TABLE IF EXISTS Periods CASCADE;
	CREATE TABLE Periods
	(
		PeriodId integer,
		BeginP timestamp,
		EndP timestamp,
		Period period
	);
	EXECUTE format('COPY Periods(PeriodId, BeginP, EndP) FROM ''%squeryperiods.csv'' DELIMITER  '','' CSV HEADER', fullpath);
	UPDATE Periods
	SET Period = period(BeginP,EndP);

	IF gist THEN
		CREATE INDEX Periods_Period_gist_idx ON Periods USING gist (Period);
	ELSE
		CREATE INDEX Periods_Period_spgist_idx ON Periods USING spgist (Period);
	END IF;
	
	/* There are NO duplicate points in Periods
	SELECT count(*)
	FROM Periods P1, Periods P2
	where P1.PeriodId < P2.PeriodId AND
	P1.BeginP = P2.BeginP AND P1.EndP = P2.EndP
	*/
	
	CREATE VIEW Periods1 (PeriodId, BeginP, EndP, Period) AS
	SELECT PeriodId, BeginP, EndP, Period
	FROM Periods
	LIMIT 10;
	
	DROP TABLE IF EXISTS Cars CASCADE;
	CREATE TABLE Cars
	(
		CarId integer primary key,
		Licence varchar(32),
		Type varchar(32),
		Model varchar(32)
	);
	EXECUTE format('COPY Cars(CarId, Licence, Type, Model) FROM ''%sdatamcar.csv'' DELIMITER  '','' CSV HEADER', fullpath);
	
	CREATE UNIQUE INDEX Cars_CarId_idx ON Cars USING btree (CarId);
	
	DROP TABLE IF EXISTS Licences CASCADE;
	CREATE TABLE Licences
	(
		LicenceId integer,
		Licence varchar(8),
		CarId integer
	);
	EXECUTE format('COPY Licences(Licence, LicenceId) FROM ''%squerylicences.csv'' DELIMITER  '','' CSV HEADER', fullpath);
	UPDATE Licences Q
	SET CarId = ( SELECT C.CarId FROM Cars C WHERE C.Licence = Q.Licence );

	CREATE INDEX Licences_CarId_idx ON Licences USING btree (CarId);
  
	/* There are duplicates in Licences
	SELECT licence, count(*) as count
	FROM Licences 
	GROUP BY Licence
	ORDER BY count desc	
	*/

	/*
	-- Remove duplicates from Licences
	DELETE FROM Licences L1
	WHERE EXISTS (SELECT * FROM Licences L2 
	WHERE L1.LicenceId < L2.LicenceId 
	AND L1.CarId = L2.CarId AND L1.Licence = L2.Licence );
	-- SELECT COUNT(*) FROM Licences;
	-- 67
	*/

	CREATE VIEW Licences1 (LicenceId, Licence, CarId) AS
	SELECT LicenceId, Licence, CarId
	FROM Licences
	LIMIT 10;
	
	CREATE VIEW Licences2 (LicenceId, Licence, CarId) AS
	SELECT LicenceId, Licence, CarId
	FROM Licences
	LIMIT 10 OFFSET 10;

	DROP TABLE IF EXISTS TripsInput CASCADE;
	CREATE TABLE TripsInput
	(
		moid integer,
		tripid integer,
		tstart timestamp without time zone,
		tend timestamp without time zone,
		xstart double precision,
		ystart double precision,
		xend double precision,
		yend double precision,
		geom geometry(LineString)
	);
	EXECUTE format('COPY TripsInput(moid, tripid, tstart, tend, xstart, ystart, xend, yend) FROM ''%strips.csv'' DELIMITER  '','' CSV HEADER', fullpath);
	UPDATE TripsInput
	SET geom = ST_Transform(ST_SetSRID(ST_MakeLine(ARRAY[ST_MakePoint(xstart, ystart),
		ST_MakePoint(xend, yend)]),4326),5676);

-------------------------------------------------------------------------------

	DROP TABLE IF EXISTS berlinmod_input_instants;
	CREATE TABLE berlinmod_input_instants AS (
	SELECT moid, tripid, tstart, xstart, ystart, 
		ST_Transform(ST_SetSRID(ST_MakePoint(xstart,ystart),4326),5676) as geom
	FROM TripsInput
	UNION ALL
	SELECT b1.moid, b1.tripid, b1.tend, b1.xend, b1.yend, 
		ST_Transform(ST_SetSRID(ST_MakePoint(b1.xend,b1.yend),4326),5676) as geom
	FROM TripsInput b1
	inner join (
		SELECT moid, tripid, max(tend) as MaxTend
		FROM TripsInput 
		GROUP BY moid, tripid
	) b2 ON b1.moid = b2.moid AND b1.tripid = b2.tripid AND b1.tend = b2.MaxTend );
	ALTER TABLE berlinmod_input_instants ADD COLUMN inst tgeompoint;
	UPDATE berlinmod_input_instants
	SET inst = tgeompointinst(geom, tstart);

	DROP TABLE IF EXISTS Trips CASCADE;
	CREATE TABLE Trips
	(
		CarId integer NOT NULL,
		TripId integer NOT NULL,
		Trip tgeompoint,
		Traj geometry,
		PRIMARY KEY (CarId, TripId),
		FOREIGN KEY (CarId) REFERENCES Cars (CarId) 
	);
	INSERT INTO Trips
		SELECT moid, tripid, tgeompointseq(array_agg(inst order by tstart), true, false)
		FROM berlinmod_input_instants
		GROUP BY moid, tripid;
	UPDATE Trips
	SET Traj = trajectory(Trip);

	CREATE INDEX Trips_CarId_idx ON Trips USING btree (CarId);
	CREATE UNIQUE INDEX Trips_pkey_idx ON Trips USING btree (CarId, TripId);

	IF gist THEN
		CREATE INDEX Trips_gist_idx ON Trips USING gist (trip);
	ELSE
		CREATE INDEX Trips_spgist_idx ON Trips USING spgist (trip);
	END IF;
	
	DROP VIEW IF EXISTS Trips1;
	CREATE VIEW Trips1 AS
	SELECT * FROM Trips LIMIT 100;
	
-------------------------------------------------------------------------------
/*
-- Loads the BerlinMOD dataset using PostGIS trajectories	
-- https://postgis.net/docs/reference.html#Temporal
   
	DROP TABLE IF EXISTS TripsGeo3DM;
	CREATE TABLE TripsGeo3DM AS
	SELECT CarId, TripId, Trip::geometry AS Trip
	FROM Trips;

	CREATE INDEX TripsGeo3DM_CarId_idx ON TripsGeo3DM USING btree (CarId);
	CREATE UNIQUE INDEX TripsGeo3DM_pkey_idx ON TripsGeo3DM USING btree (CarId, TripId);
	CREATE INDEX TripsGeo3DM_spatial_idx ON TripsGeo3DM USING gist (Trip);
*/
-------------------------------------------------------------------------------

	DROP TABLE RegionsInput;
	DROP TABLE TripsInput;
	DROP TABLE berlinmod_input_instants;

	RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-------------------------------------------------------------------------------
