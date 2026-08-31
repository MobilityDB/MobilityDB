-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
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
