/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief SQL script to generate the berlinmod_trips.csv file from the file
 * berlinmod_instants.csv
 */

CREATE TABLE trip_instants (
  tripid int NOT NULL,
  vehid int NOT NULL,
  day date NOT NULL,
  seqno int NOT NULL,
  geom geometry NOT NULL,
  t timestamptz NOT NULL
);
/* PostgreSQL requires a full path for the COPY command. Please change the
 * path below to your settings */
COPY trip_instants FROM
  '/home/esteban/src/MobilityDB/meos/examples/data/berlinmod_instants.csv' CSV HEADER;

CREATE TABLE trips (
  tripid int NOT NULL,
  vehid int NOT NULL,
  day date NOT NULL,
  seqno int NOT NULL,
  trip tgeompoint NOT NULL
);

INSERT INTO trips
SELECT tripid, vehid, day, seqno,
  tgeompoint_contseq(array_agg(tgeompoint(geom, t) ORDER BY t)) AS trip
FROM trip_instants
GROUP BY tripid, vehid, day, seqno
ORDER BY tripid, vehid, day, seqno;

COPY (SELECT tripid, vehid, day, seqno, asHexEWKB(trip) AS trip FROM trips) TO
  '/home/esteban/src/MobilityDB/meos/examples/berlinmod_trips.csv' CSV HEADER;
