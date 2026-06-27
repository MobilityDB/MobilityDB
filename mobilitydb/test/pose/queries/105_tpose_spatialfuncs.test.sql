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
-- SRID functions

SELECT SRID(tpose 'Pose(Point(1 2),0.5)@2000-01-01');
SELECT SRID(tpose 'SRID=5676;Pose(Point(1 2),0.5)@2000-01-01');

SELECT asEWKT(setSRID(tpose 'Pose(Point(1 2),0.5)@2000-01-01', 5676));

-------------------------------------------------------------------------------
-- atGeometry / minusGeometry — instant

SELECT asText(atGeometry(
  tpose 'Pose(Point(0 0),0)@2000-01-01',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));
SELECT asText(atGeometry(
  tpose 'Pose(Point(10 10),0)@2000-01-01',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));
SELECT asText(minusGeometry(
  tpose 'Pose(Point(0 0),0)@2000-01-01',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));
SELECT asText(minusGeometry(
  tpose 'Pose(Point(10 10),0)@2000-01-01',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));

-------------------------------------------------------------------------------
-- atGeometry / minusGeometry — discrete sequence

SELECT asText(atGeometry(
  tpose '{Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),0)@2000-01-02}',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));
SELECT asText(minusGeometry(
  tpose '{Pose(Point(0 0),0)@2000-01-01, Pose(Point(10 0),0)@2000-01-02}',
  'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1))'::geometry));

-------------------------------------------------------------------------------
-- atGeometry / minusGeometry — continuous sequence

SELECT asText(atGeometry(
  tpose '[Pose(Point(0 0),0)@2000-01-01, Pose(Point(5 0),0)@2000-01-06]',
  'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'::geometry));
SELECT asText(minusGeometry(
  tpose '[Pose(Point(0 0),0)@2000-01-01, Pose(Point(5 0),0)@2000-01-06]',
  'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'::geometry));

-------------------------------------------------------------------------------
-- atGeometry / minusGeometry — sequence set

SELECT asText(atGeometry(
  tpose '{[Pose(Point(0 0),0)@2000-01-01, Pose(Point(5 0),0)@2000-01-06]}',
  'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'::geometry));
SELECT asText(minusGeometry(
  tpose '{[Pose(Point(0 0),0)@2000-01-01, Pose(Point(5 0),0)@2000-01-06]}',
  'Polygon((1 -1,3 -1,3 1,1 1,1 -1))'::geometry));

-------------------------------------------------------------------------------
-- atStbox / minusStbox — spatial only

SELECT asText(atStbox(
  tpose '[Pose(Point(0 0),0)@2000-01-01, Pose(Point(5 0),0)@2000-01-06]',
  stbox 'STBOX X((1,-1),(3,1))'));
SELECT asText(minusStbox(
  tpose '[Pose(Point(0 0),0)@2000-01-01, Pose(Point(5 0),0)@2000-01-06]',
  stbox 'STBOX X((1,-1),(3,1))'));

-------------------------------------------------------------------------------
-- atStbox / minusStbox — temporal only

SELECT asText(atStbox(
  tpose '[Pose(Point(0 0),0)@2000-01-01, Pose(Point(5 0),0)@2000-01-06]',
  stbox 'STBOX T([2000-01-02, 2000-01-04])'));
SELECT asText(minusStbox(
  tpose '[Pose(Point(0 0),0)@2000-01-01, Pose(Point(5 0),0)@2000-01-06]',
  stbox 'STBOX T([2000-01-02, 2000-01-04])'));

-------------------------------------------------------------------------------
-- atStbox / minusStbox — spatiotemporal

SELECT asText(atStbox(
  tpose '[Pose(Point(0 0),0)@2000-01-01, Pose(Point(5 0),0)@2000-01-06]',
  stbox 'STBOX XT(((1,-1),(3,1)),[2000-01-02, 2000-01-04])'));
SELECT asText(minusStbox(
  tpose '[Pose(Point(0 0),0)@2000-01-01, Pose(Point(5 0),0)@2000-01-06]',
  stbox 'STBOX XT(((1,-1),(3,1)),[2000-01-02, 2000-01-04])'));

-------------------------------------------------------------------------------
-- NULL returns: no overlap

SELECT atGeometry(
  tpose 'Pose(Point(10 10),0)@2000-01-01',
  'Polygon((0 0,1 0,1 1,0 1,0 0))'::geometry);
SELECT atStbox(
  tpose '[Pose(Point(0 0),0)@2000-01-01, Pose(Point(1 0),0)@2000-01-02]',
  stbox 'STBOX X((10,-1),(20,1))');

-------------------------------------------------------------------------------
-- Table queries

SELECT COUNT(*) FROM tbl_tpose2d WHERE atGeometry(temp,
  ST_SetSRID('Polygon((-200 -200,200 -200,200 200,-200 200,-200 -200))'::geometry, 3812)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tpose2d WHERE minusGeometry(temp,
  ST_SetSRID('Polygon((-200 -200,200 -200,200 200,-200 200,-200 -200))'::geometry, 3812)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tpose2d WHERE atStbox(temp,
  stbox 'SRID=3812;STBOX XT(((-200,-200),(200,200)),[2001-06-01, 2001-12-31])') IS NOT NULL;
SELECT COUNT(*) FROM tbl_tpose2d WHERE minusStbox(temp,
  stbox 'SRID=3812;STBOX XT(((-200,-200),(200,200)),[2001-06-01, 2001-12-31])') IS NOT NULL;

-------------------------------------------------------------------------------
