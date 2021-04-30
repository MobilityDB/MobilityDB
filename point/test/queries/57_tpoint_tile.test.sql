-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
--
-- Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
-- contributors
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
-- Multidimensional tiling
-------------------------------------------------------------------------------

SELECT multidimGrid(tgeompoint '[Point(3 3)@2000-01-15, Point(15 15)@2000-01-25]'::stbox, 2.0) LIMIT 3;
SELECT multidimGrid(tgeompoint 'SRID=3812;[Point(3 3)@2000-01-15, Point(15 15)@2000-01-25]'::stbox, 2.0, geometry 'Point(3 3)') LIMIT 3;
SELECT multidimGrid(tgeompoint '[Point(3 3 3)@2000-01-15, Point(15 15 15)@2000-01-25]'::stbox, 2.0, geometry 'Point(3 3 3)') LIMIT 3;
SELECT multidimGrid(tgeompoint '[Point(3 3)@2000-01-15, Point(15 15)@2000-01-25]'::stbox, 2.0, '2 days', 'Point(3 3)', '2000-01-15') LIMIT 3;
SELECT multidimGrid(tgeompoint '[Point(3 3 3)@2000-01-15, Point(15 15 15)@2000-01-25]'::stbox, 2.0, '2 days', 'Point(3 3 3)', '2000-01-15') LIMIT 3;

SELECT multidimTileStbox(ARRAY[2,0], 2.0);
SELECT multidimTileStbox(ARRAY[2,0,0], 2.0);
SELECT multidimTileStbox(ARRAY[2,0,0], 2.0, interval '2 days');
SELECT multidimTileStbox(ARRAY[2,0,0,0], 2.0, interval '2 days');
SELECT multidimTileStbox(ARRAY[2,0,0,0], 2.0, interval '2 days', geometry 'Point(1 1 1)', '2020-06-15');

-------------------------------------------------------------------------------
