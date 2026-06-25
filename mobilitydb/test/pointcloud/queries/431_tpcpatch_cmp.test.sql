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

-- B-tree comparator audit for tpcpatch.
--
-- The comparator on the underlying pcpatch is byte-wise (memcmp over
-- the meaningful varlena bytes). The well-defined order at the
-- temporal level is the lexicographic combination of per-instant
-- timestamps and per-instant pcpatch byte order. This file pins
-- the observable consequences so a future refactor can't silently
-- change them.

-------------------------------------------------------------------------------
-- Set up a second pcid so we can probe pcid-as-primary-discriminator.
-------------------------------------------------------------------------------

INSERT INTO pointcloud_formats (pcid, srid, schema)
SELECT 2, srid, schema FROM pointcloud_formats WHERE pcid = 1;

\set p1 'tpcpatch(pcpatch(1, pcpoint(1, 1.0, 1.0, 1.0)), ''2024-01-01''::timestamptz)'
\set p1_dup 'tpcpatch(pcpatch(1, pcpoint(1, 1.0, 1.0, 1.0)), ''2024-01-01''::timestamptz)'
\set p2 'tpcpatch(pcpatch(1, pcpoint(1, 2.0, 2.0, 2.0)), ''2024-01-01''::timestamptz)'
\set p3 'tpcpatch(pcpatch(1, pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)), ''2024-01-01''::timestamptz)'
\set p_pcid2 'tpcpatch(pcpatch(2, pcpoint(2, 1.0, 1.0, 1.0)), ''2024-01-01''::timestamptz)'

-------------------------------------------------------------------------------
-- Reflexivity, equality, and inequality
-------------------------------------------------------------------------------

-- a value compares equal to itself, regardless of how it's constructed
SELECT (:p1) = (:p1_dup);
SELECT (:p1) <> (:p1_dup) IS FALSE;
SELECT (:p1) < (:p1) IS FALSE;
SELECT (:p1) > (:p1) IS FALSE;
SELECT (:p1) <= (:p1);
SELECT (:p1) >= (:p1);

-- distinct values are not equal
SELECT (:p1) <> (:p2);
SELECT (:p1) <> (:p3);
SELECT (:p2) <> (:p3);

-------------------------------------------------------------------------------
-- Anti-symmetry — exactly one of <, >, = holds for any pair
-------------------------------------------------------------------------------

-- a < b ⇔ b > a; never both
SELECT ((:p1) < (:p2)) AND ((:p2) > (:p1));
SELECT NOT ((:p1) < (:p2) AND (:p1) > (:p2));

-- = and <> are exact complements
SELECT ((:p1) = (:p2)) = NOT ((:p1) <> (:p2));
SELECT ((:p1) = (:p1_dup)) = NOT ((:p1) <> (:p1_dup));

-------------------------------------------------------------------------------
-- pcid is the primary discriminator
-- (it's the first int32 after the varlena header, so memcmp orders by it
--  before any payload bytes are considered)
-------------------------------------------------------------------------------

-- a pcpatch with pcid=1 sorts before any pcpatch with pcid=2, regardless
-- of what's inside the payload
SELECT (:p1) < (:p_pcid2);
SELECT (:p2) < (:p_pcid2);
SELECT (:p3) < (:p_pcid2);

-------------------------------------------------------------------------------
-- Transitivity — pin a 3-element chain
-------------------------------------------------------------------------------

-- p1 = (1.0,1.0,1.0); p3 = same first point + (2.0,2.0,2.0) trailing;
-- p2 = (2.0,2.0,2.0). Byte order: p1 is a prefix of p3 (so p1 < p3),
-- and p3's data starts with 1.0 while p2's starts with 2.0 (so p3 < p2).
SELECT (:p1) < (:p3);
SELECT (:p3) < (:p2);
SELECT (:p1) < (:p2);  -- transitive consequence

-------------------------------------------------------------------------------
-- B-tree opclass round-trip — ORDER BY produces a total order over a
-- mixed-pcid bag of values, with all pcid=1 rows before all pcid=2 rows.
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_cmp_ordered;
CREATE TABLE tbl_cmp_ordered AS
  SELECT 1::int AS k, (:p2)::tpcpatch AS pa
  UNION ALL SELECT 2, (:p1)
  UNION ALL SELECT 3, (:p_pcid2)
  UNION ALL SELECT 4, (:p3);

-- After ORDER BY pa, the pcid=1 rows come before the pcid=2 row,
-- and within pcid=1 the byte-wise order is p1, p3, p2 (per the
-- transitivity chain above).
SELECT array_agg(k ORDER BY pa) FROM tbl_cmp_ordered;
DROP TABLE tbl_cmp_ordered;

-------------------------------------------------------------------------------
-- Cleanup
-------------------------------------------------------------------------------

DELETE FROM pointcloud_formats WHERE pcid = 2;

-------------------------------------------------------------------------------
