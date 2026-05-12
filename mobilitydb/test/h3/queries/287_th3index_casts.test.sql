-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- §1.10 Casts — pure sugar over h3_cell_to_latlng (212).
--
-- The casts depend on the same h3_cell_to_gs_point adapter that
-- h3_cell_to_latlng uses; they fail at runtime in the same way until
-- the adapter lands. The tests below exercise only the cast
-- machinery, leaving correctness of the underlying conversion to
-- 282_th3index_latlng.

-------------------------------------------------------------------------------
-- th3index :: tgeogpoint
-------------------------------------------------------------------------------

SELECT (th3index '590464338553208831@2001-01-01')::tgeogpoint IS NOT NULL;

-- Sequence form
SELECT (th3index
  '[590464338553208831@2001-01-01, 622236750694711295@2001-01-02]')::tgeogpoint
  IS NOT NULL;

-- Equivalence with the explicit function call
SELECT (th3index '590464338553208831@2001-01-01')::tgeogpoint
  ~= h3_cell_to_latlng(th3index '590464338553208831@2001-01-01');

-- The cast is EXPLICIT, not IMPLICIT — assignment without `::` fails.
/* Errors */
DROP TABLE IF EXISTS tbl_th3index_cast_test;
CREATE TABLE tbl_th3index_cast_test(k int, p tgeogpoint);
INSERT INTO tbl_th3index_cast_test VALUES
  (1, th3index '590464338553208831@2001-01-01');
DROP TABLE IF EXISTS tbl_th3index_cast_test;

-------------------------------------------------------------------------------
-- th3index :: tgeompoint
-------------------------------------------------------------------------------

SELECT (th3index '590464338553208831@2001-01-01')::tgeompoint IS NOT NULL;

-- Sequence form
SELECT (th3index
  '[590464338553208831@2001-01-01, 622236750694711295@2001-01-02]')::tgeompoint
  IS NOT NULL;

-- Equivalence with the explicit function call
SELECT (th3index '590464338553208831@2001-01-01')::tgeompoint
  ~= h3_cell_to_latlng_tgeompoint(th3index '590464338553208831@2001-01-01');

-- The cast is EXPLICIT
/* Errors */
DROP TABLE IF EXISTS tbl_th3index_cast_test;
CREATE TABLE tbl_th3index_cast_test(k int, p tgeompoint);
INSERT INTO tbl_th3index_cast_test VALUES
  (1, th3index '590464338553208831@2001-01-01');
DROP TABLE IF EXISTS tbl_th3index_cast_test;

-------------------------------------------------------------------------------
