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

-- Value-level tests for tpcbox (constructors, accessors, set operations,
-- topological predicates, comparison operators).

-- A box reads its reference system off the schema its pcid names, so every
-- pcid constructed with below names one. Schema 2 states the same reference
-- system as schema 1, which is what leaves the schemas themselves as the only
-- thing two such boxes disagree about; schema 5 states a different one.
INSERT INTO pointcloud_formats (pcid, srid, schema)
SELECT 2, srid, schema FROM pointcloud_formats WHERE pcid = 1;
INSERT INTO pointcloud_formats (pcid, srid, schema)
SELECT 5, 4326, schema FROM pointcloud_formats WHERE pcid = 1;

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT tpcboxX(0, 0, 10, 10, 1);
SELECT tpcboxZ(0, 0, 0, 10, 10, 10, 1);
SELECT tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 1);
SELECT tpcboxXT(0, 0, 10, 10, tstzspan '[2024-01-01, 2024-01-02]', 1);
SELECT tpcboxZT(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1);

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT hasX(tpcboxZT(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1));
SELECT hasZ(tpcboxZT(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1));
SELECT hasT(tpcboxZT(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1));
SELECT hasZ(tpcboxX(0, 0, 10, 10, 1));   -- false: no Z
SELECT hasT(tpcboxX(0, 0, 10, 10, 1));   -- false: no T

SELECT xmin(tpcboxX(0, 0, 10, 10, 1));
SELECT xmax(tpcboxX(0, 0, 10, 10, 1));
SELECT ymin(tpcboxX(0, 0, 10, 10, 1));
SELECT ymax(tpcboxX(0, 0, 10, 10, 1));
SELECT zmin(tpcboxZ(0, 0, 1, 10, 10, 9, 1));
SELECT zmax(tpcboxZ(0, 0, 1, 10, 10, 9, 1));
SELECT zmin(tpcboxX(0, 0, 10, 10, 1));   -- NULL: no Z
SELECT pcid(tpcboxX(0, 0, 10, 10, 2));
SELECT SRID(tpcboxX(0, 0, 10, 10, 1));
-- The SRID is the one the schema states, so it is not the caller's to supply
SELECT SRID(tpcboxX(0, 0, 10, 10, 5));

-- A pcid naming no registered schema states no reference system to read, so
-- the box is refused rather than built with an assumed one, which is how
-- PC_MakePoint answers the same input
SELECT tpcboxX(0, 0, 10, 10, 7);

-------------------------------------------------------------------------------
-- Conversions
--
-- The X and Y extent of the patch below are distinct, so that reading the
-- PCBOUNDS header of a patch in the wrong order shows in the answer. The two
-- queries answer the same extent, the second one through pgPointCloud itself.
-------------------------------------------------------------------------------

SELECT xmin(b), xmax(b), ymin(b), ymax(b)
FROM (SELECT tpcbox(pcpatch(pcpoint(1, 1.0, 2.0, 3.0),
  pcpoint(1, 4.0, 5.0, 6.0))) AS b) t;
SELECT PC_PatchMin(p, 'X'), PC_PatchMax(p, 'X'),
  PC_PatchMin(p, 'Y'), PC_PatchMax(p, 'Y')
FROM (SELECT pcpatch(pcpoint(1, 1.0, 2.0, 3.0),
  pcpoint(1, 4.0, 5.0, 6.0)) AS p) t;

-- The extent of a patch of a single point is that point
SELECT xmin(b), xmax(b), ymin(b), ymax(b)
FROM (SELECT tpcbox(pcpatch(pcpoint(1, 1.0, 2.0, 3.0))) AS b) t;

-- The schema pcid 1 names holds a Z dimension, so the box of a patch of that
-- schema states the Z extent of its points, as the box of a pcpoint does
SELECT hasZ(b), zmin(b), zmax(b)
FROM (SELECT tpcbox(pcpatch(pcpoint(1, 1.0, 2.0, 3.0),
  pcpoint(1, 4.0, 5.0, 6.0))) AS b) t;
SELECT PC_PatchMin(p, 'Z'), PC_PatchMax(p, 'Z')
FROM (SELECT pcpatch(pcpoint(1, 1.0, 2.0, 3.0),
  pcpoint(1, 4.0, 5.0, 6.0)) AS p) t;

-------------------------------------------------------------------------------
-- Set operations
-------------------------------------------------------------------------------

SELECT tpcboxX(0, 0, 5, 5, 1) + tpcboxX(3, 3, 10, 10, 1);
SELECT tpcboxX(0, 0, 5, 5, 1) * tpcboxX(3, 3, 10, 10, 1);
SELECT tpcboxX(0, 0, 5, 5, 1) * tpcboxX(50, 50, 60, 60, 1);  -- NULL (disjoint)

-- A box carrying coordinates states the schema they are read in, so a box
-- naming schema 0 and one naming schema 1 have no common extent to combine
SELECT tpcboxX(0, 0, 5, 5, 0) + tpcboxX(3, 3, 10, 10, 1);
SELECT tpcboxX(0, 0, 5, 5, 0) * tpcboxX(3, 3, 10, 10, 1);

-------------------------------------------------------------------------------
-- Topological predicates — same pcid
-------------------------------------------------------------------------------

SELECT tpcboxX(0, 0, 10, 10, 1) @> tpcboxX(2, 2, 8, 8, 1);
SELECT tpcboxX(0, 0, 10, 10, 1) @> tpcboxX(2, 2, 20, 20, 1);
SELECT tpcboxX(2, 2, 8, 8, 1)   <@ tpcboxX(0, 0, 10, 10, 1);
SELECT tpcboxX(0, 0, 5, 5, 1)   && tpcboxX(3, 3, 10, 10, 1);
SELECT tpcboxX(0, 0, 5, 5, 1)   && tpcboxX(50, 50, 60, 60, 1);
SELECT tpcboxX(0, 0, 5, 5, 1)   ~= tpcboxX(0, 0, 5, 5, 1);

-- The portable spelling of each operator above answers the same
SELECT contains(tpcboxX(0, 0, 10, 10, 1), tpcboxX(2, 2, 8, 8, 1));
SELECT contained(tpcboxX(2, 2, 8, 8, 1), tpcboxX(0, 0, 10, 10, 1));
SELECT overlaps(tpcboxX(0, 0, 5, 5, 1), tpcboxX(3, 3, 10, 10, 1));
SELECT same(tpcboxX(0, 0, 5, 5, 1), tpcboxX(0, 0, 5, 5, 1));
SELECT tpcboxX(0, 0, 5, 5, 1)  -|- tpcboxX(5, 0, 10, 5, 1);
SELECT adjacent(tpcboxX(0, 0, 5, 5, 1), tpcboxX(5, 0, 10, 5, 1));

-------------------------------------------------------------------------------
-- Topological predicates — the boxes must share a dimension to be compared
-------------------------------------------------------------------------------

-- A box holding coordinates and a box holding a period have no axis in
-- common, so there is nothing for a topological predicate to compare and it
-- has no answer to give. The box naming no schema is comparable with any
-- schema, which leaves the shared dimension the only question left to fail on
SELECT tpcboxX(0, 0, 10, 10, 1) && tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0);
SELECT tpcboxX(0, 0, 10, 10, 1) @> tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0);
SELECT tpcboxX(0, 0, 10, 10, 1) <@ tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0);
SELECT tpcboxX(0, 0, 10, 10, 1) -|- tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0);
SELECT tpcboxX(0, 0, 10, 10, 1) ~= tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0);

-- The portable spelling of each of them refuses the same pair
SELECT overlaps(tpcboxX(0, 0, 10, 10, 1), tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0));
SELECT contains(tpcboxX(0, 0, 10, 10, 1), tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0));
SELECT contained(tpcboxX(0, 0, 10, 10, 1), tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0));
SELECT adjacent(tpcboxX(0, 0, 10, 10, 1), tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0));
SELECT same(tpcboxX(0, 0, 10, 10, 1), tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0));

-- A box carrying both axes shares one with each of them, so it is compared
SELECT tpcboxXT(0, 0, 10, 10, tstzspan '[2024-01-01, 2024-01-31]', 1) &&
  tpcboxX(2, 2, 8, 8, 1);
SELECT tpcboxXT(0, 0, 10, 10, tstzspan '[2024-01-01, 2024-01-31]', 1) &&
  tpcboxT(tstzspan '[2024-01-02, 2024-01-03]', 0);

-------------------------------------------------------------------------------
-- Topological predicates — the schema decides what a coordinate means
-------------------------------------------------------------------------------

-- Two schemas give the same number two meanings, so the boxes are not
-- comparable and the predicate has no answer to give
SELECT tpcboxX(0, 0, 10, 10, 1) @> tpcboxX(2, 2, 8, 8, 2);
SELECT tpcboxX(0, 0, 5, 5, 1)   && tpcboxX(0, 0, 5, 5, 2);
SELECT tpcboxX(0, 0, 5, 5, 1)   ~= tpcboxX(0, 0, 5, 5, 2);

-- Schema 0 is a schema like any other once a box carries coordinates
SELECT tpcboxX(0, 0, 5, 5, 0) && tpcboxX(3, 3, 10, 10, 1);
SELECT tpcboxX(0, 0, 5, 5, 0) @> tpcboxX(1, 1, 4, 4, 1);

-- The SRID is read on the same terms. A constructor can no longer state one
-- apart from the schema, so the disagreeing pair is written: the schema pcid 1
-- names states no reference system, which leaves the `SRID=` prefix the only
-- level that speaks, and two values whose prefixes differ denote two different
-- places
SELECT tpcbox 'SRID=4326;TPCBOX(X((0,0),(5,5)), 1)' &&
  tpcbox 'SRID=3857;TPCBOX(X((3,3),(10,10)), 1)';
SELECT tpcbox 'SRID=4326;TPCBOX(X((0,0),(5,5)), 1)' +
  tpcbox 'SRID=3857;TPCBOX(X((3,3),(10,10)), 1)';

-- A box carrying no coordinates names no schema, so it meets a box of any
-- schema: this is the shape a time-only query box takes against an index
SELECT tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0) &&
  tpcboxT(tstzspan '[2024-01-02, 2024-01-03]', 1);
SELECT tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0) +
  tpcboxT(tstzspan '[2024-01-02, 2024-01-03]', 1);

-- Planar and spherical coordinates are two different measurements of the
-- world, so a box of each has nothing comparable to offer the other even when
-- both name the same schema and the same reference system
SELECT tpcbox 'SRID=4326;TPCBOX(X((1,1),(2,2)), 1)' &&
  tpcbox 'GEODTPCBOX(X((1,1),(2,2)), 1)';
SELECT tpcbox 'SRID=4326;TPCBOX(X((1,1),(2,2)), 1)' @>
  tpcbox 'GEODTPCBOX(X((1,1),(2,2)), 1)';
SELECT tpcbox 'SRID=4326;TPCBOX(X((1,1),(2,2)), 1)' <<
  tpcbox 'GEODTPCBOX(X((5,5),(7,7)), 1)';

-- Two spherical boxes of the same schema are compared as any other pair
SELECT tpcbox 'GEODTPCBOX(X((1,1),(2,2)), 1)' &&
  tpcbox 'GEODTPCBOX(X((1,1),(3,3)), 1)';

-------------------------------------------------------------------------------
-- Position predicates — each asks about the axis it reads
-------------------------------------------------------------------------------

-- The time axis carries no schema and no reference system: a timestamp means
-- the same thing whatever schema the coordinates are read in, so the four
-- time-axis predicates compare two boxes of different schemas
SELECT tpcboxXT(0, 0, 5, 5, tstzspan '[2024-01-01, 2024-01-15]', 1) <<#
  tpcboxXT(0, 0, 5, 5, tstzspan '[2025-01-01, 2025-01-15]', 2);
SELECT tpcboxXT(0, 0, 5, 5, tstzspan '[2024-01-01, 2024-01-15]', 1) &<#
  tpcboxXT(0, 0, 5, 5, tstzspan '[2025-01-01, 2025-01-15]', 2);
SELECT tpcboxXT(0, 0, 5, 5, tstzspan '[2025-01-01, 2025-01-15]', 1) #>>
  tpcboxXT(0, 0, 5, 5, tstzspan '[2024-01-01, 2024-01-15]', 2);
SELECT tpcboxXT(0, 0, 5, 5, tstzspan '[2025-01-01, 2025-01-15]', 1) #&>
  tpcboxXT(0, 0, 5, 5, tstzspan '[2024-01-01, 2024-01-15]', 2);

-- The twelve that read coordinates still refuse the same pair, the schema being
-- what gives a coordinate its meaning
SELECT tpcboxXT(0, 0, 5, 5, tstzspan '[2024-01-01, 2024-01-15]', 1) <<
  tpcboxXT(10, 10, 20, 20, tstzspan '[2025-01-01, 2025-01-15]', 2);
SELECT tpcboxXT(0, 0, 5, 5, tstzspan '[2024-01-01, 2024-01-15]', 1) <<|
  tpcboxXT(10, 10, 20, 20, tstzspan '[2025-01-01, 2025-01-15]', 2);

-- Every predicate needs both boxes to carry the axis it reads, which two boxes
-- of coordinates alone do not for the Z and the time axis
SELECT tpcboxX(0, 0, 2, 2, 1) <</ tpcboxX(5, 5, 7, 7, 1);
SELECT tpcboxX(0, 0, 2, 2, 1) <<# tpcboxX(5, 5, 7, 7, 1);

-------------------------------------------------------------------------------
-- Extent aggregation
-------------------------------------------------------------------------------

-- The aggregate answers the extent of the boxes it folds, so it is bounded by
-- the same comparability the operators are: an extent spanning two schemas, or
-- two reference systems, states a region no schema can read
SELECT extent(b) FROM (VALUES (tpcboxX(0, 0, 5, 5, 1)),
  (tpcboxX(3, 3, 10, 10, 1))) t(b);
SELECT extent(b) FROM (VALUES (tpcboxX(0, 0, 5, 5, 1)),
  (tpcboxX(3, 3, 10, 10, 2))) t(b);
SELECT extent(b) FROM (VALUES (tpcboxX(0, 0, 5, 5, 0)),
  (tpcboxX(3, 3, 10, 10, 1))) t(b);
SELECT extent(b) FROM (VALUES (tpcbox 'SRID=4326;TPCBOX(X((0,0),(5,5)), 1)'),
  (tpcbox 'SRID=3857;TPCBOX(X((3,3),(10,10)), 1)')) t(b);

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT tpcboxX(0, 0, 5, 5, 1) =  tpcboxX(0, 0, 5, 5, 1);
SELECT tpcboxX(0, 0, 5, 5, 1) <> tpcboxX(0, 0, 5, 5, 2);
SELECT tpcboxX(0, 0, 5, 5, 1) <  tpcboxX(0, 0, 5, 5, 2);

-------------------------------------------------------------------------------
-- Input — the two levels that state the reference system
--
-- The written form is the one place a value states its reference system twice,
-- as an `SRID=` prefix and as the schema its pcid names, so it is the one place
-- the two are reconciled. A level reading SRID_UNKNOWN states nothing.
-------------------------------------------------------------------------------

-- The schema is the level that holds the SRID, so a value stating none takes
-- the schema's, and the constructor has no other level to read
SELECT tpcbox 'TPCBOX(X((0,0),(10,10)), 5)';
-- Both levels stating the same system is one statement written twice
SELECT tpcbox 'SRID=4326;TPCBOX(X((0,0),(10,10)), 5)';
-- Two levels stating different systems is a value contradicting itself
SELECT tpcbox 'SRID=3857;TPCBOX(X((0,0),(10,10)), 5)';
-- Where no schema resolves the prefix is the only level there is, which is
-- what lets a value be read where its schema is not registered
SELECT tpcbox 'SRID=3857;TPCBOX(X((0,0),(10,10)), 7)';
SELECT tpcbox 'TPCBOX(X((0,0),(10,10)), 7)';

-------------------------------------------------------------------------------
-- setSRID states a reference system the schema does not state
-------------------------------------------------------------------------------

-- A box naming no schema has no other level, so it takes what it is given
SELECT SRID(setSRID(tpcboxX(0, 0, 10, 10), 4326));
-- The schema pcid 1 names states none either, which leaves the box the only
-- level there is
SELECT SRID(setSRID(tpcboxX(0, 0, 10, 10, 1), 4326));
-- The schema pcid 5 names states one, so the SRID is not the caller's to set,
-- and agreeing with it is not a licence to set it either
SELECT setSRID(tpcboxX(0, 0, 10, 10, 5), 3857);
SELECT setSRID(tpcboxX(0, 0, 10, 10, 5), 4326);
-- A box carrying no coordinates states no reference system, and pcid 5 still
-- speaks for it
SELECT setSRID(tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 5), 3857);

-------------------------------------------------------------------------------

DELETE FROM pointcloud_formats WHERE pcid IN (2, 5);

-------------------------------------------------------------------------------
