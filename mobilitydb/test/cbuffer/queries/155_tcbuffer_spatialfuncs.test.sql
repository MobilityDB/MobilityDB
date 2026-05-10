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
-- SRID
-------------------------------------------------------------------------------

SELECT SRID(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT SRID(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}');
SELECT SRID(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]');
SELECT SRID(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}');

SELECT SRID(tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01');

/*****************************************************************************/

SELECT asEWKT(setSRID(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', 5676));
SELECT asEWKT(setSRID(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', 5676));
SELECT asEWKT(setSRID(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', 5676));
SELECT asEWKT(setSRID(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', 5676));

/*****************************************************************************/

-- Tests independent of the PROJ version
WITH test(t) AS (
  SELECT tcbuffer 'SRID=4326;Cbuffer(Point(1.0 2.0),0.5)@2000-01-01' )
SELECT asEWKT(transform(transform(t, 5676), 4326), 6) FROM test;

WITH test(t) AS (
  SELECT tcbuffer 'SRID=4326;[Cbuffer(Point(1.0 2.0),0.5)@2000-01-01, Cbuffer(Point(2.0 3.0),0.5)@2000-01-02]' )
SELECT asEWKT(transform(transform(t, 5676), 4326), 6) FROM test;

-- Noop: transform to same SRID
SELECT asEWKT(transform(tcbuffer 'SRID=4326;Cbuffer(Point(1.0 2.0),0.5)@2000-01-01', 4326));

-------------------------------------------------------------------------------
-- traversedArea
-------------------------------------------------------------------------------

SELECT round(traversedArea(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01'), 6);
SELECT round(traversedArea(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}'), 6);
SELECT round(traversedArea(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]'), 6);
SELECT round(traversedArea(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}'), 6);

-- No simplification
SELECT round(traversedArea(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02]', false), 6);

-------------------------------------------------------------------------------
-- atValue, minusValue
-------------------------------------------------------------------------------

SELECT asText(atValue(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', cbuffer 'Cbuffer(Point(1 1),0.5)'));
SELECT asText(atValue(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', cbuffer 'Cbuffer(Point(1 1),0.5)'));
SELECT asText(atValue(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', cbuffer 'Cbuffer(Point(1 1),0.5)'));
SELECT asText(atValue(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', cbuffer 'Cbuffer(Point(1 1),0.5)'));

SELECT asText(minusValue(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', cbuffer 'Cbuffer(Point(1 1),0.5)'));
SELECT asText(minusValue(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', cbuffer 'Cbuffer(Point(1 1),0.5)'));
SELECT asText(minusValue(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', cbuffer 'Cbuffer(Point(1 1),0.5)'));
SELECT asText(minusValue(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', cbuffer 'Cbuffer(Point(1 1),0.5)'));

-------------------------------------------------------------------------------
-- atGeometry, minusGeometry
-------------------------------------------------------------------------------

SELECT asText(atGeometry(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))'));
SELECT asText(atGeometry(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))'));
SELECT asText(atGeometry(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))'));
SELECT asText(atGeometry(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))'));

-- Disk fully outside geometry
SELECT asText(atGeometry(tcbuffer 'Cbuffer(Point(10 10),0.5)@2000-01-01', geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))'));
-- Empty geometry
SELECT asText(atGeometry(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02]', geometry 'Polygon empty'));

SELECT asText(minusGeometry(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))'));
SELECT asText(minusGeometry(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))'));
SELECT asText(minusGeometry(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))'));
SELECT asText(minusGeometry(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', geometry 'Polygon((0 0,0 3,3 3,3 0,0 0))'));

/* Errors */
SELECT asText(atGeometry(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', geometry 'SRID=3812;Polygon((0 0,0 3,3 3,3 0,0 0))'));

-------------------------------------------------------------------------------
-- atStbox, minusStbox
-------------------------------------------------------------------------------

SELECT asText(atStbox(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', stbox 'STBOX XT(((0,0),(3,3)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', stbox 'STBOX XT(((0,0),(3,3)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', stbox 'STBOX XT(((0,0),(3,3)),[2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', stbox 'STBOX XT(((0,0),(3,3)),[2000-01-01,2000-01-02])'));

-- Space-only stbox
SELECT asText(atStbox(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', stbox 'STBOX X((0,0),(2,2))'));

-- borderInc set to false
SELECT asText(atStbox(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', stbox 'STBOX XT(((0,0),(3,3)),[2000-01-01,2000-01-02])', false));

SELECT asText(minusStbox(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01', stbox 'STBOX XT(((0,0),(3,3)),[2000-01-01,2000-01-02])'));
SELECT asText(minusStbox(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03}', stbox 'STBOX XT(((0,0),(3,3)),[2000-01-01,2000-01-02])'));
SELECT asText(minusStbox(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', stbox 'STBOX XT(((0,0),(3,3)),[2000-01-01,2000-01-02])'));
SELECT asText(minusStbox(tcbuffer '{[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}', stbox 'STBOX XT(((0,0),(3,3)),[2000-01-01,2000-01-02])'));

-- Space-only stbox
SELECT asText(minusStbox(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', stbox 'STBOX X((0,0),(2,2))'));

-- borderInc set to false
SELECT asText(minusStbox(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.5)@2000-01-03]', stbox 'STBOX XT(((0,0),(3,3)),[2000-01-01,2000-01-02])', false));

/* Errors */
-- tcbuffer requires X dimension in stbox (unlike tgeompoint, T-only stbox not allowed)
SELECT asText(atStbox(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02]', stbox 'STBOX T([2000-01-01,2000-01-02])'));
SELECT asText(atStbox(tcbuffer 'SRID=4326;Cbuffer(Point(1 1),0.5)@2000-01-01', stbox 'GEODSTBOX ZT(((1,1,1),(2,2,2)),[2000-01-01,2000-01-02])'));

/*****************************************************************************/
