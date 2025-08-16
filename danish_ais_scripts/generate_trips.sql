drop table if exists AISInput;
CREATE TABLE AISInput(
 T timestamp,
 TypeOfMobile varchar(100),
 MMSI integer,
 Latitude float,
 Longitude float,
 navigationalStatus varchar(100),
 ROT float,
 SOG float,
 COG float,
 Heading integer,
 IMO varchar(100),
 Callsign varchar(100),
 Name varchar(100),
 ShipType varchar(100),
 CargoType varchar(100),
 Width float,
 Length float,
 TypeOfPositionFixingDevice varchar(100),
 Draught float,
 Destination varchar(100),
 ETA varchar(100),
 DataSourceType varchar(100),
 SizeA float,
 SizeB float,
 SizeC float,
 SizeD float,
 Geom geometry(Point,4326)
 );


COPY AISInput(T,TypeOfMobile,MMSI,Latitude,Longitude,NavigationalStatus,
 ROT,SOG,COG,Heading,IMO,CallSign,Name,ShipType,CargoType,Width,Length,
 TypeOfPositionFixingDevice,Draught, Destination,ETA,DataSourceType,
 SizeA,SizeB,SizeC,SizeD)
 FROM '/mnt/c/Users/ossama/Documents/MobilityDB/danish_ais_scripts/aisdk-2024-12-04.csv' DELIMITER ','CSV HEADER;


drop table if exists AISInputFiltered;

CREATE TABLE AISInputFiltered AS
 SELECT DISTINCT ON(MMSI,T) *
 FROM AISInput
 WHERE Longitude BETWEEN-16.1 and 32.88 AND Latitude BETWEEN 40.18 AND 84.17;



UPDATE AISInputFiltered SET
 NavigationalStatus = CASE NavigationalStatus WHEN 'Unknown value' THEN NULL END,
 IMO = CASE IMO WHEN 'Unknown' THEN NULL END,
 ShipType = CASE ShipType WHEN 'Undefined' THEN NULL END,
 TypeOfPositionFixingDevice = CASE TypeOfPositionFixingDevice
 WHEN 'Undefined' THEN NULL END,
 Geom = ST_SetSRID( ST_MakePoint( Longitude, Latitude ), 4326);



DROP TABLE IF EXISTS Ships;
CREATE TABLE Ships (
  MMSI integer PRIMARY KEY,
  Trip tgeompoint,
  SOG  tfloat,
  COG  tfloat
);


drop table if exists AISInputFiltered2;
drop table if exists AISInputFiltered3;

create table AISInputFiltered2 as select * from AISInputFiltered ;
create table AISInputFiltered3 as select * from AISInputFiltered limit 1;
  


DO $$
DECLARE
  two_mmsi   integer[];
  iter       integer;
  max_iter   integer := 1500;  -- nombre de lots à traiter
BEGIN
  FOR iter IN 1..max_iter LOOP
    -- 1.a) Prendre les 2 premiers MMSI encore en stock
    SELECT array_agg(m ORDER BY m)
      INTO two_mmsi
    FROM (
      SELECT DISTINCT MMSI AS m
      FROM AISInputFiltered2
      ORDER BY m
      LIMIT 2
    ) sub;

    -- 1.b) Si moins de 2 navires restants, on arrête
    IF two_mmsi IS NULL OR array_length(two_mmsi, 1) < 2 THEN
      EXIT;
    END IF;

    -- 1.c) Réinitialiser AISInputFiltered3 pour ne contenir que ces deux MMSI
    TRUNCATE AISInputFiltered3;
    INSERT INTO AISInputFiltered3
    SELECT *
    FROM AISInputFiltered2
    WHERE MMSI = ANY(two_mmsi);

    -- 1.d) Supprimer ces lignes de la table source
    DELETE FROM AISInputFiltered2
    WHERE MMSI = ANY(two_mmsi);

    INSERT INTO Ships (MMSI, Trip, SOG, COG)
    SELECT
      MMSI,
      tgeompointseq(
        array_agg(
          tgeompoint(ST_Transform(Geom, 25832), T)
          ORDER BY T
        )
      ) AS Trip,
      tfloatseq(
        array_agg(tfloat(SOG, T) ORDER BY T)
        FILTER (WHERE SOG IS NOT NULL)
      ) AS SOG,
      tfloatseq(
        array_agg(tfloat(COG, T) ORDER BY T)
        FILTER (WHERE COG IS NOT NULL)
      ) AS COG
    FROM AISInputFiltered3
    GROUP BY MMSI
    ON CONFLICT (MMSI) DO NOTHING;

  END LOOP;
END
$$ LANGUAGE plpgsql;


drop table if exists AISInputFiltered2;
drop table if exists AISInputFiltered3;





ALTER TABLE Ships ADD COLUMN Traj geometry;
 UPDATE Ships SET Traj= trajectory(Trip);



DELETE FROM Ships
 WHERE length(Trip)=0 OR length(Trip)>=1500000;



drop table if exists SimplifiedShips;

CREATE TABLE SimplifiedShips AS
SELECT MMSI, douglasPeuckerSimplify(Trip, 100) AS Trip, traj as trajectory
FROM Ships;


DROP TABLE IF EXISTS bigsample;

CREATE TABLE bigsample AS
SELECT
  mmsi,
  trip     AS tripsegment,
  trajectory AS trajsegment
FROM simplifiedships;
