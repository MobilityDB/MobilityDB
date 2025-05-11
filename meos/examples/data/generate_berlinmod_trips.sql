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
 * @brief Sample program that assembles the observations in the CSV file
 * `berlinmod_instants.csv` into trips stored in the file `berlinmod_trips.csv`
 */

DROP TABLE IF EXISTS trips_input;
CREATE TABLE trips_input (
  tripid int,
  vehid int,
  day date,
  seqno int,
  geom geometry,
  t timestamptz
);

COPY trips_input (tripid, vehid, day, seqno, geom, t) FROM
  '/home/esteban/src/MobilityDB/meos/examples/data/berlinmod_instants.csv' CSV HEADER;

DROP TABLE IF EXISTS trips;
CREATE TABLE trips (
  tripid int,
  vehid int,
  day date,
  seqno int,
  trip tgeompoint
);

INSERT INTO trips(tripid, vehid, day, seqno, trip)
SELECT tripid, vehid, day, seqno,
  tgeompointSeq(array_agg(tgeompoint(ST_Transform(Geom, 3857), T) ORDER BY T))
FROM trips_input
GROUP BY tripid, vehid, day, seqno
ORDER BY tripid, vehid, day, seqno;

COPY (SELECT tripid, vehid, day, seqno, ashexewkb(trip) AS trip FROM trips WHERE vehid < 6 ORDER BY tripid) TO
  '/home/esteban/src/MobilityDB/meos/examples/data/berlinmod_trips.csv' CSV HEADER;
