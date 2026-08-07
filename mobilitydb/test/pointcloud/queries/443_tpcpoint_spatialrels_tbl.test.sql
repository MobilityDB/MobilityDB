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

-- Smoke tests for spatial relationships on tpcpoint.

-------------------------------------------------------------------------------
-- Self-relations: every row eIntersects itself; nothing eDisjoint itself.
-------------------------------------------------------------------------------

SELECT bool_and(eIntersects(temp, temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT eDisjoint(temp, temp)) FROM tbl_tpcpoint;

-- aIntersects must imply eIntersects.
SELECT bool_and((NOT aIntersects(temp, temp)) OR eIntersects(temp, temp))
FROM tbl_tpcpoint;

-------------------------------------------------------------------------------
-- eDwithin with a huge radius is true for every row vs. the origin point.
-------------------------------------------------------------------------------

SELECT bool_and(eDwithin(temp, geometry 'SRID=0;POINT(0 0)', 1e9))
FROM tbl_tpcpoint;
SELECT bool_and(aDwithin(temp, geometry 'SRID=0;POINT(0 0)', 1e9))
FROM tbl_tpcpoint;

-- Reverse arg order produces the same result.
SELECT bool_and(eDwithin(geometry 'SRID=0;POINT(0 0)', temp, 1e9))
FROM tbl_tpcpoint;

-------------------------------------------------------------------------------
-- eDwithin with zero radius collapses to eIntersects against the geometry.
-------------------------------------------------------------------------------

SELECT bool_and(eDwithin(temp, geometry 'SRID=0;POINT(50 50)', 0)
              = eIntersects(temp, geometry 'SRID=0;POINT(50 50)'))
FROM tbl_tpcpoint;

-------------------------------------------------------------------------------
