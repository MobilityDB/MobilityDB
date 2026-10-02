-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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

-- Equality of two lines with the same points, the second with one more vertex,
-- in the base values, the temporal values, the sets, the hash, the B-tree order
-- and the normalization of a sequence

SELECT geometry 'Linestring(0 0,2 0)' = geometry 'Linestring(0 0,1 0,2 0)';
SELECT geography 'Linestring(0 0,2 0)' = geography 'Linestring(0 0,1 0,2 0)';

SELECT tgeometry '[Linestring(0 0,2 0)@2001-01-01]' = tgeometry '[Linestring(0 0,1 0,2 0)@2001-01-01]';
SELECT cmp(tgeometry '[Linestring(0 0,2 0)@2001-01-01]', tgeometry '[Linestring(0 0,1 0,2 0)@2001-01-01]');
SELECT hash(tgeometry '[Linestring(0 0,2 0)@2001-01-01]') = hash(tgeometry '[Linestring(0 0,1 0,2 0)@2001-01-01]');
SELECT tgeography '[Linestring(0 0,2 0)@2001-01-01]' = tgeography '[Linestring(0 0,1 0,2 0)@2001-01-01]';
SELECT cmp(tgeography '[Linestring(0 0,2 0)@2001-01-01]', tgeography '[Linestring(0 0,1 0,2 0)@2001-01-01]');
SELECT hash(tgeography '[Linestring(0 0,2 0)@2001-01-01]') = hash(tgeography '[Linestring(0 0,1 0,2 0)@2001-01-01]');

SELECT eEqual(tgeometry '[Linestring(0 0,2 0)@2001-01-01]', geometry 'Linestring(0 0,1 0,2 0)');
SELECT eEqual(tgeography '[Linestring(0 0,2 0)@2001-01-01]', geography 'Linestring(0 0,1 0,2 0)');

SELECT geomset '{"Linestring(0 0,2 0)"}' = geomset '{"Linestring(0 0,1 0,2 0)"}';
SELECT numValues(geomset '{"Linestring(0 0,2 0)", "Linestring(0 0,1 0,2 0)"}');

SELECT numInstants(tgeometry 'Interp=Step;[Linestring(0 0,2 0)@2001-01-01, Linestring(0 0,1 0,2 0)@2001-01-02, Linestring(0 0,2 0)@2001-01-03]');
SELECT ST_AsText(valueAtTimestamp(tgeometry 'Interp=Step;[Linestring(0 0,2 0)@2001-01-01, Linestring(0 0,1 0,2 0)@2001-01-02, Linestring(0 0,2 0)@2001-01-03]', '2001-01-02'));

CREATE TEMP TABLE tbl_geo_eq(t tgeometry);
INSERT INTO tbl_geo_eq VALUES (tgeometry '[Linestring(0 0,2 0)@2001-01-01]'), (tgeometry '[Linestring(0 0,1 0,2 0)@2001-01-01]');
SET enable_hashagg = on; SET enable_sort = off;
SELECT COUNT(*) FROM (SELECT DISTINCT t FROM tbl_geo_eq) s;
SET enable_hashagg = off; SET enable_sort = on;
SELECT COUNT(*) FROM (SELECT DISTINCT t FROM tbl_geo_eq) s;
RESET enable_hashagg; RESET enable_sort;
DROP TABLE tbl_geo_eq;

-------------------------------------------------------------------------------
