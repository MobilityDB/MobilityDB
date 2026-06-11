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
-------------------------------------------------------------------------------

-- Column-level pcid pinning via PG typmod for tpcpoint / tpcpatch.

-- Set up a second pcid so we can probe both match and mismatch paths.
-- Mirrors pcid=1's schema (the test fixture's only registered schema)
-- so PC_MakePoint succeeds for either.
INSERT INTO pointcloud_formats (pcid, srid, schema)
SELECT 2, srid, schema FROM pointcloud_formats WHERE pcid = 1;

-------------------------------------------------------------------------------
-- typmod_in / typmod_out — error cases
-------------------------------------------------------------------------------

-- negative pcid is invalid
SELECT NULL::tpcpoint(-1);
-- non-integer pcid is invalid
SELECT NULL::tpcpoint(abc);
-- zero pcid is invalid (must be positive)
SELECT NULL::tpcpoint(0);
-- two args are invalid (only single pcid is supported)
SELECT NULL::tpcpoint(1, 2);

-------------------------------------------------------------------------------
-- typmod_out — column DDL preserves the typmod in `\d`-style output
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_typmod;
CREATE TABLE tbl_typmod (
  trj1 tpcpoint(1),
  trj2 tpcpoint(2),
  pat1 tpcpatch(1),
  pat2 tpcpatch(2),
  unconstrained tpcpoint
);
SELECT format_type(atttypid, atttypmod) FROM pg_attribute
  WHERE attrelid = 'tbl_typmod'::regclass AND attnum > 0
  ORDER BY attnum;

-------------------------------------------------------------------------------
-- INSERT path — matching pcid succeeds, mismatched fails
-------------------------------------------------------------------------------

-- valid: pcid 1 into trj1 (column typmod pcid 1)
INSERT INTO tbl_typmod (trj1) VALUES
  (tpcpoint(pcpoint(1, 1.0, 1.0, 1.0), '2024-01-01'::timestamptz));
SELECT pcid(trj1) FROM tbl_typmod WHERE trj1 IS NOT NULL;

-- mismatch: pcid 2 into trj1 (typmod 1) raises
INSERT INTO tbl_typmod (trj1) VALUES
  (tpcpoint(PC_MakePoint(2, ARRAY[1.0, 1.0, 1.0]::float[]),
            '2024-01-02'::timestamptz));

-- mismatch: pcid 1 into trj2 (typmod 2) raises (symmetric)
INSERT INTO tbl_typmod (trj2) VALUES
  (tpcpoint(pcpoint(1, 1.0, 1.0, 1.0), '2024-01-03'::timestamptz));

-- valid: pcid 2 into trj2
INSERT INTO tbl_typmod (trj2) VALUES
  (tpcpoint(PC_MakePoint(2, ARRAY[1.0, 1.0, 1.0]::float[]),
            '2024-01-04'::timestamptz));
SELECT pcid(trj2) FROM tbl_typmod WHERE trj2 IS NOT NULL;

-- unconstrained column accepts any pcid
INSERT INTO tbl_typmod (unconstrained) VALUES
  (tpcpoint(pcpoint(1, 1, 1, 1), '2024-01-05'::timestamptz)),
  (tpcpoint(PC_MakePoint(2, ARRAY[1.0,1.0,1.0]::float[]),
            '2024-01-06'::timestamptz));
SELECT bool_and(pcid(unconstrained) IN (1, 2)) FROM tbl_typmod
  WHERE unconstrained IS NOT NULL;

-------------------------------------------------------------------------------
-- tpcpatch path — same surface, same checks
-------------------------------------------------------------------------------

INSERT INTO tbl_typmod (pat1) VALUES
  (tpcpatch(pcpatch(1, pcpoint(1, 1, 1, 1), pcpoint(1, 2, 2, 2)),
            '2024-01-01'::timestamptz));

-- mismatch: pcid 2 into pat1 (typmod 1) raises
INSERT INTO tbl_typmod (pat1) VALUES
  (tpcpatch(PC_Patch(ARRAY[
    PC_MakePoint(2, ARRAY[1.0,1.0,1.0]::float[])]),
    '2024-01-02'::timestamptz));

-------------------------------------------------------------------------------
-- ALTER TABLE: changing the typmod re-validates existing rows
-------------------------------------------------------------------------------

-- With a single pcid 1 row in trj1, ALTER to typmod 2 must fail
ALTER TABLE tbl_typmod ALTER COLUMN trj1 TYPE tpcpoint(2);

-- ALTER to unconstrained always works
ALTER TABLE tbl_typmod ALTER COLUMN trj1 TYPE tpcpoint;
SELECT format_type(atttypid, atttypmod) FROM pg_attribute
  WHERE attrelid = 'tbl_typmod'::regclass AND attname = 'trj1';

-------------------------------------------------------------------------------
-- Cleanup
-------------------------------------------------------------------------------

DROP TABLE tbl_typmod;
DELETE FROM pointcloud_formats WHERE pcid = 2;

-------------------------------------------------------------------------------
