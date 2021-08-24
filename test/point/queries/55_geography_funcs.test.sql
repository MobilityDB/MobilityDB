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

SELECT ST_AsText(setPrecision(st_LineInterpolatePoint(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.0), 6));
SELECT ST_AsText(setPrecision(st_LineInterpolatePoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.0, true), 6));
SELECT ST_AsText(setPrecision(st_LineInterpolatePoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 1.0, false), 6));
SELECT ST_AsText(setPrecision(st_LineInterpolatePoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 0.1, true), 6));
-- EMPTY
SELECT ST_AsText(setPrecision(st_LineInterpolatePoints(geography 'Linestring empty', 0.1, true), 6));
/* Errors */
SELECT ST_AsText(setPrecision(st_LineInterpolatePoints(geography 'Point(4.35 50.85)', 0.5, true), 6));
SELECT ST_AsText(setPrecision(st_LineInterpolatePoints(geography 'Linestring(4.35 50.85, 37.617222 55.755833)', 2, true), 6));

-------------------------------------------------------------------------------
