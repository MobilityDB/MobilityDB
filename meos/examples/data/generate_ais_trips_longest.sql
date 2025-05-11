/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Sample program that reads AIS data from the file
 * `aisdk-2025-03-01.csv` provided by the Danish Maritime Authority in
 * https://web.ais.dk/aisdata/, assembles the observations in a table trips
 * and outputs in the file `ais_trips_longest.csv` the temporal values for
 * trip and SOG for the longest 10 trips from the input data
 */
DO $$
BEGIN

RAISE INFO 'Reading input data from the CSV file ...';

DROP TABLE IF EXISTS AISInput;
CREATE TABLE AISInput(
  T timestamp,
  TypeOfMobile varchar(50),
  MMSI integer,
  Latitude float,
  Longitude float,
  navigationalStatus varchar(100),
  ROT float,
  SOG float,
  COG float,
  Heading integer,
  IMO varchar(50),
  Callsign varchar(50),
  Name varchar(100),
  ShipType varchar(50),
  CargoType varchar(100),
  Width float,
  Length float,
  TypeOfPositionFixingDevice varchar(50),
  Draught float,
  Destination varchar(50),
  ETA varchar(50),
  DataSourceType varchar(50),
  SizeA float,
  SizeB float,
  SizeC float,
  SizeD float,
  Geom geometry(Point, 4326)
);

COPY AISInput(T, TypeOfMobile, MMSI, Latitude, Longitude, NavigationalStatus,
  ROT, SOG, COG, Heading, IMO, CallSign, Name, ShipType, CargoType, Width,
  Length, TypeOfPositionFixingDevice, Draught, Destination, ETA,
  DataSourceType, SizeA, SizeB, SizeC, SizeD) FROM
  '/home/esteban/src/MobilityDB/meos/examples/data/aisdk-2025-03-01.csv' CSV HEADER;

RAISE INFO 'Filtering input data removing erroneous records ...';

DROP TABLE IF EXISTS AISInputFiltered;
CREATE TABLE AISInputFiltered AS
SELECT DISTINCT ON(MMSI,T) *
FROM AISInput
WHERE Longitude BETWEEN -16.1 and 32.88 AND Latitude BETWEEN 40.18 AND 84.17;

UPDATE AISInputFiltered
SET Geom = ST_SetSRID(ST_Point(Longitude, Latitude), 4326);

DROP TABLE IF EXISTS ships;
CREATE TABLE ships (
  MMSI integer PRIMARY KEY,
  trip tgeompoint,
  SOG tfloat
);

RAISE INFO 'Assembling the trip and the SOG from the input data ...';

INSERT INTO ships(MMSI, trip, SOG)
SELECT MMSI,
  tgeompointSeq(array_agg(tgeompoint(ST_Transform(Geom, 25832), T) ORDER BY T)),
  tfloatSeq(array_agg(tfloat(SOG, T) ORDER BY T) FILTER (WHERE SOG IS NOT NULL))
FROM AISInputFiltered
WHERE Geom IS NOT NULL AND T IS NOT NULL
GROUP BY MMSI
ORDER BY MMSI;

RAISE INFO 'Creating the output CSV file for the 10 longest trips ...';

COPY (SELECT MMSI, ashexewkb(trip) AS trip, ashexwkb(SOG) AS SOG FROM ships ORDER BY length(trip) DESC LIMIT 10) TO
  '/home/esteban/src/MobilityDB/meos/examples/data/ais_trips_longest.csv' CSV HEADER;

RAISE INFO 'The END';

END;
$$;
