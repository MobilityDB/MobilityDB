-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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
-- Tests for constructors of PostgreSQL geometry types.
-- File geo_constructors.c
-------------------------------------------------------------------------------

SELECT point(1.5, 2.5);
SELECT line(1.5, 2.5, 3.5);
SELECT lseg(point(1.5, 2.5), point(3.5, 4.5));
SELECT box(point(1.5, 2.5), point(3.5, 4.5));
SELECT circle(point(1.5, 2.5), 2.5);
SELECT path(ARRAY[point(1.5, 1.5), point(2.5, 2.5)]);
SELECT path(ARRAY[point(1.5, 1.5), point(2.5, 2.5), point(1.5, 1.5)]);
SELECT polygon(ARRAY[point(1.5, 1.5), point(2.5, 2.5), point(1.5, 1.5)]);
SELECT polygon(ARRAY[point(2.5, 2.5), point(3.5, 1.5), point(1.5, 1.5), point(2.5, 2.5)]);

-------------------------------------------------------------------------------
