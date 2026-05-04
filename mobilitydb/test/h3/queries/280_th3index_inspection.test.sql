-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
-- h3_get_resolution
-------------------------------------------------------------------------------

SELECT h3_get_resolution(th3index '590464338553208831@2001-01-01');
SELECT h3_get_resolution(th3index '612544986753269759@2001-01-01');

-- All four temporal subtypes
SELECT h3_get_resolution(th3index
  '{590464338553208831@2001-01-01, 590464201114255359@2001-01-02}');
SELECT h3_get_resolution(th3index
  '[590464338553208831@2001-01-01, 590464201114255359@2001-01-02]');
SELECT h3_get_resolution(th3index
  '{[590464338553208831@2001-01-01, 590464201114255359@2001-01-02],
    [612544986753269759@2001-01-03, 612544986761658367@2001-01-04]}');

-------------------------------------------------------------------------------
-- h3_get_base_cell_number
-------------------------------------------------------------------------------

SELECT h3_get_base_cell_number(th3index '590464338553208831@2001-01-01');
SELECT h3_get_base_cell_number(th3index '590464201114255359@2001-01-01');
SELECT h3_get_base_cell_number(th3index '612544986753269759@2001-01-01');

SELECT h3_get_base_cell_number(th3index
  '[590464338553208831@2001-01-01, 590464201114255359@2001-01-02]');

-------------------------------------------------------------------------------
-- h3_is_valid_cell
-------------------------------------------------------------------------------

SELECT h3_is_valid_cell(th3index '590464338553208831@2001-01-01');
SELECT h3_is_valid_cell(th3index '590464201114255359@2001-01-01');
SELECT h3_is_valid_cell(th3index '0@2001-01-01');

-- Mixed valid + invalid in a sequence
SELECT h3_is_valid_cell(th3index
  '{590464338553208831@2001-01-01, 0@2001-01-02, 590464201114255359@2001-01-03}');

-------------------------------------------------------------------------------
-- h3_is_res_class_iii
-------------------------------------------------------------------------------

-- Class III alternates with resolution: even = class II, odd = class III.
-- res 3 cells are class III (true); res 8 cells are class II (false).
SELECT h3_is_res_class_iii(th3index '590464338553208831@2001-01-01');
SELECT h3_is_res_class_iii(th3index '612544986753269759@2001-01-01');

SELECT h3_is_res_class_iii(th3index
  '{590464338553208831@2001-01-01, 612544986753269759@2001-01-02}');

-------------------------------------------------------------------------------
-- h3_is_pentagon
-------------------------------------------------------------------------------

SELECT h3_is_pentagon(th3index '590464338553208831@2001-01-01');
SELECT h3_is_pentagon(th3index '590464201114255359@2001-01-01');

SELECT h3_is_pentagon(th3index
  '{590464338553208831@2001-01-01, 590464201114255359@2001-01-02}');

-------------------------------------------------------------------------------
