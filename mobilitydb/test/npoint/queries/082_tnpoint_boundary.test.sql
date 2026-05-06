-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Boundary-condition coverage for npoint position clamping at [0, 1].
-- Documented contract: 0.0 (start of route) and 1.0 (end of route) are
-- valid; values outside [0, 1] are rejected at the constructor.
-------------------------------------------------------------------------------

-- Endpoints are valid
SELECT npoint(1, 0.0);
SELECT npoint(1, 1.0);

-- Just-inside values are valid
SELECT npoint(1, 0.0001);
SELECT npoint(1, 0.9999);

-- Interpolating to geometry at the endpoints lands on the route's start
-- and end points respectively; both are well-defined.
SELECT round(geometry(npoint(1, 0.0))::geometry, 6);
SELECT round(geometry(npoint(1, 1.0))::geometry, 6);

-- Position parameter outside [0, 1] is rejected at construction
/* Errors */
SELECT npoint(1, -0.0001);
SELECT npoint(1, 1.0001);
SELECT npoint(1, -1);
SELECT npoint(1, 2);

-- Same boundary contract on the WKT input path
SELECT npoint 'Npoint(1, 0.0)';
SELECT npoint 'Npoint(1, 1.0)';
/* Errors */
SELECT npoint 'Npoint(1, -0.0001)';
SELECT npoint 'Npoint(1, 1.0001)';

-- Same boundary contract on the MFJSON input path
SELECT asText(tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1,"position":0.0}],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"None"}'));
SELECT asText(tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1,"position":1.0}],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"None"}'));

-- nsegment endpoints: nsegment_set normalises (pos1, pos2) via Min / Max,
-- so a constructor call with the positions reversed returns a normalised
-- segment rather than an error.
SELECT nsegment(1, 1.0, 0.0);
SELECT nsegment(1, 0.0, 1.0);

-- nsegment also rejects out-of-range positions at construction
/* Errors */
SELECT nsegment(1, -0.0001, 1.0);
SELECT nsegment(1, 0.0, 1.0001);

-------------------------------------------------------------------------------
