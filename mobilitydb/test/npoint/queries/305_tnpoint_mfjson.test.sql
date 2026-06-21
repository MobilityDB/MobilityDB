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
-- asMFJSON / tnpointFromMFJSON round-trip across all temporal subtypes
-------------------------------------------------------------------------------

-- Instant
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint 'Npoint(1, 0.5)@2000-01-01')));
-- Discrete sequence
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}')));
-- Linear sequence
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')));
-- Sequence set (linear)
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.1)@2000-01-04, Npoint(2, 0.2)@2000-01-05]}')));
-- Step interpolation
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint 'Interp=Step;[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]')));

-- Boundary positions on the route (0.0 = start, 1.0 = end)
SELECT asText(tnpointFromMFJSON(asMFJSON(tnpoint '[Npoint(1, 0.0)@2000-01-01, Npoint(1, 1.0)@2000-01-02]')));

-------------------------------------------------------------------------------
/* Errors */
-------------------------------------------------------------------------------

-- Malformed JSON
SELECT tnpointFromMFJSON('ABC');
-- Wrong moving-feature type tag
SELECT tnpointFromMFJSON('{"type":"MovingPoint","coordinates":[1,1],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"None"}');
-- Unknown moving-feature type tag
SELECT tnpointFromMFJSON('{"type":"XXX","values":[{"route":1,"position":0.5}],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"None"}');
-- Missing 'route'
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"position":0.5}],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"None"}');
-- Missing 'position'
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1}],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"None"}');
-- Position out of [0, 1] (negative)
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1,"position":-0.1}],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"None"}');
-- Position out of [0, 1] (above 1)
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1,"position":1.5}],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"None"}');
-- Unregistered route id
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":99999,"position":0.5}],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"None"}');
-- Missing interpolation
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1,"position":0.5}],"datetimes":["2000-01-01T00:00:00+01"]}');
-- Bad interpolation
SELECT tnpointFromMFJSON('{"type":"MovingNetworkPoint","values":[{"route":1,"position":0.5}],"datetimes":["2000-01-01T00:00:00+01"],"interpolation":"XXX"}');

-------------------------------------------------------------------------------
