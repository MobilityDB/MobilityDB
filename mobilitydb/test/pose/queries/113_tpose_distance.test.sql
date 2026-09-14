-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
-- distance
-------------------------------------------------------------------------------

SELECT round(distance(geometry 'Point(1 0)', pose 'Pose(Point(4 0),0)'), 6);
SELECT round(nearestApproachDistance(stbox 'STBOX X((1,-1),(3,1))', pose 'Pose(Point(4 0),0)'), 6);
SELECT round(distance(pose 'Pose(Point(4 0),0)', geometry 'Point(1 0)'), 6);
SELECT round(nearestApproachDistance(pose 'Pose(Point(4 0),0)', stbox 'STBOX X((1,-1),(3,1))'), 6);
SELECT round(stbox 'STBOX X((1,-1),(3,1))' |=| pose 'Pose(Point(4 0),0)', 6);
SELECT round(pose 'Pose(Point(4 0),0)' |=| stbox 'STBOX X((1,-1),(3,1))', 6);
SELECT round(distance(pose 'Pose(Point(0 0),0)', pose 'Pose(Point(3 4),0)'), 6);

SELECT round(geometry 'Point(1 0)' <-> pose 'Pose(Point(4 0),0)', 6);
SELECT round(stbox 'STBOX X((1,-1),(3,1))' <-> pose 'Pose(Point(4 0),0)', 6);
SELECT round(pose 'Pose(Point(4 0),0)' <-> geometry 'Point(1 0)', 6);
SELECT round(pose 'Pose(Point(4 0),0)' <-> stbox 'STBOX X((1,-1),(3,1))', 6);
SELECT round(pose 'Pose(Point(0 0),0)' <-> pose 'Pose(Point(3 4),0)', 6);

SELECT round(pose 'Pose(Point(0 0),0)' <-> pose 'Pose(Point(0 0),0)', 6);
SELECT round(pose 'Pose(Point(0 0),0.5)' <-> pose 'Pose(Point(3 4),1.5)', 6);
SELECT round(pose 'SRID=5676;Pose(Point(0 0),0)' <-> pose 'SRID=5676;Pose(Point(3 4),0)', 6);
SELECT round(pose 'Pose(Point(0 0 0), 1, 0, 0, 0)' <-> pose 'Pose(Point(3 4 12), 1, 0, 0, 0)', 6);

-- Empty geometry and NULL arguments
SELECT round(geometry 'Point empty' <-> pose 'Pose(Point(4 0),0)', 6);
SELECT round(pose 'Pose(Point(4 0),0)' <-> geometry 'Point empty', 6);
SELECT NULL::geometry <-> pose 'Pose(Point(4 0),0)';
SELECT pose 'Pose(Point(4 0),0)' <-> NULL::stbox;
SELECT NULL::pose <-> pose 'Pose(Point(4 0),0)';

-- Errors
SELECT pose 'SRID=5676;Pose(Point(0 0),0)' <-> pose 'Pose(Point(3 4),0)';
SELECT geometry 'SRID=5676;Point(1 0)' <-> pose 'Pose(Point(4 0),0)';

-------------------------------------------------------------------------------
-- tDistance
-------------------------------------------------------------------------------

SELECT round(tDistance(geometry 'Point(1 0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
SELECT round(tDistance(pose 'Pose(Point(1 0),0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
SELECT round(tDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', geometry 'Point(1 0)'), 6);
SELECT round(tDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', pose 'Pose(Point(1 0),0)'), 6);
SELECT round(tDistance(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  tpose '[Pose(Point(0 2),0)@2001-01-01, Pose(Point(4 2),0)@2001-01-05]'), 6);

-------------------------------------------------------------------------------
-- nearestApproachInstant
-------------------------------------------------------------------------------

SELECT asText(nearestApproachInstant(geometry 'Point(2 0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'));
SELECT asText(nearestApproachInstant(pose 'Pose(Point(2 0),0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'));
SELECT asText(nearestApproachInstant(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', geometry 'Point(2 0)'));
SELECT asText(nearestApproachInstant(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', pose 'Pose(Point(2 0),0)'));
SELECT asText(nearestApproachInstant(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  tpose '[Pose(Point(0 2),0)@2001-01-01, Pose(Point(4 2),0)@2001-01-05]'));

-------------------------------------------------------------------------------
-- nearestApproachDistance
-------------------------------------------------------------------------------

SELECT round(nearestApproachDistance(geometry 'Point(2 0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
SELECT round(nearestApproachDistance(stbox 'STBOX X((1,-1),(3,1))', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
SELECT round(nearestApproachDistance(pose 'Pose(Point(2 0),0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', geometry 'Point(2 0)'), 6);
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', stbox 'STBOX X((1,-1),(3,1))'), 6);
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', pose 'Pose(Point(2 0),0)'), 6);

-- A box carrying a period measures the part of the temporal pose inside it
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', stbox 'STBOX XT(((10,-1),(12,1)),[2001-01-01, 2001-01-03])'), 6);
SELECT round(nearestApproachDistance(stbox 'STBOX XT(((10,-1),(12,1)),[2001-01-01, 2001-01-03])', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'), 6);
-- The same box with no period measures the whole temporal pose
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', stbox 'STBOX X((10,-1),(12,1))'), 6);
-- No part of the temporal pose is inside the period
SELECT round(nearestApproachDistance(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', stbox 'STBOX XT(((10,-1),(12,1)),[2001-02-01, 2001-02-02])'), 6);
SELECT round(nearestApproachDistance(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  tpose '[Pose(Point(0 2),0)@2001-01-01, Pose(Point(4 2),0)@2001-01-05]'), 6);

-------------------------------------------------------------------------------
-- shortestLine
-------------------------------------------------------------------------------

SELECT ST_AsText(shortestLine(geometry 'Point(2 0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'));
SELECT ST_AsText(shortestLine(pose 'Pose(Point(2 0),0)', tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]'));
SELECT ST_AsText(shortestLine(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', geometry 'Point(2 0)'));
SELECT ST_AsText(shortestLine(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]', pose 'Pose(Point(2 0),0)'));
SELECT ST_AsText(shortestLine(
  tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]',
  tpose '[Pose(Point(0 2),0)@2001-01-01, Pose(Point(4 2),0)@2001-01-05]'));

-------------------------------------------------------------------------------
-- Table queries
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2
WHERE tDistance(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2
WHERE nearestApproachInstant(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2
WHERE nearestApproachDistance(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

SELECT COUNT(*) FROM tbl_tpose2d t1, tbl_tpose2d t2
WHERE shortestLine(t1.temp, t2.temp) IS NOT NULL LIMIT 10;

-------------------------------------------------------------------------------
