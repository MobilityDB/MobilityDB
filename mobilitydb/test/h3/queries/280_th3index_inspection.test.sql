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

-- §1.2 Index Inspection — five unary lifts.
--
-- Test cells (decimal of the h3-pg fixture hex values):
--   590464338553208831 = 0x831c02fffffffff = res 3 hexagon
--   590464201114255359 = 0x831c00fffffffff = res 3 pentagon
--   612544986753269759 = 0x880326b885fffff = res 8 hexagon
--   0                  = invalid

-------------------------------------------------------------------------------
-- getResolution
-------------------------------------------------------------------------------

SELECT getResolution(th3index '831c02fffffffff@2001-01-01');
SELECT getResolution(th3index '880326b885fffff@2001-01-01');

-- All four temporal subtypes
SELECT getResolution(th3index
  '{831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02}');
SELECT getResolution(th3index
  '[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02]');
SELECT getResolution(th3index
  '{[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02],
    [880326b885fffff@2001-01-03, 880326b88dfffff@2001-01-04]}');

-------------------------------------------------------------------------------
-- th3GetBaseCellNumber
-------------------------------------------------------------------------------

SELECT th3GetBaseCellNumber(th3index '831c02fffffffff@2001-01-01');
SELECT th3GetBaseCellNumber(th3index '831c00fffffffff@2001-01-01');
SELECT th3GetBaseCellNumber(th3index '880326b885fffff@2001-01-01');

SELECT th3GetBaseCellNumber(th3index
  '[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02]');

-------------------------------------------------------------------------------
-- isValidCell
-------------------------------------------------------------------------------

SELECT isValidCell(th3index '831c02fffffffff@2001-01-01');
SELECT isValidCell(th3index '831c00fffffffff@2001-01-01');
SELECT isValidCell(th3index '0@2001-01-01');

-- Mixed valid + invalid in a sequence
SELECT isValidCell(th3index
  '{831c02fffffffff@2001-01-01, 0@2001-01-02, 831c00fffffffff@2001-01-03}');

-------------------------------------------------------------------------------
-- th3IsResClassIii
-------------------------------------------------------------------------------

-- Class III alternates with resolution: even = class II, odd = class III.
-- res 3 cells are class III (true); res 8 cells are class II (false).
SELECT th3IsResClassIii(th3index '831c02fffffffff@2001-01-01');
SELECT th3IsResClassIii(th3index '880326b885fffff@2001-01-01');

SELECT th3IsResClassIii(th3index
  '{831c02fffffffff@2001-01-01, 880326b885fffff@2001-01-02}');

-------------------------------------------------------------------------------
-- th3IsPentagon
-------------------------------------------------------------------------------

SELECT th3IsPentagon(th3index '831c02fffffffff@2001-01-01');
SELECT th3IsPentagon(th3index '831c00fffffffff@2001-01-01');

SELECT th3IsPentagon(th3index
  '{831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02}');

-------------------------------------------------------------------------------
