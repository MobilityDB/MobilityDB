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

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT tpcboxX(0, 0, 10, 10, 1, 0);
SELECT tpcboxZ(0, 0, 0, 10, 10, 10, 1, 0);
SELECT tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 1);
SELECT tpcboxXT(0, 0, 10, 10, tstzspan '[2024-01-01, 2024-01-02]', 1, 0);
SELECT tpcboxZT(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1, 0);

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT hasX(tpcboxZT(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1, 0));
SELECT hasZ(tpcboxZT(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1, 0));
SELECT hasT(tpcboxZT(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1, 0));
SELECT hasZ(tpcboxX(0, 0, 10, 10, 1, 0));   -- false: no Z
SELECT hasT(tpcboxX(0, 0, 10, 10, 1, 0));   -- false: no T

SELECT xmin(tpcboxX(0, 0, 10, 10, 1, 0));
SELECT xmax(tpcboxX(0, 0, 10, 10, 1, 0));
SELECT ymin(tpcboxX(0, 0, 10, 10, 1, 0));
SELECT ymax(tpcboxX(0, 0, 10, 10, 1, 0));
SELECT zmin(tpcboxZ(0, 0, 1, 10, 10, 9, 1, 0));
SELECT zmax(tpcboxZ(0, 0, 1, 10, 10, 9, 1, 0));
SELECT zmin(tpcboxX(0, 0, 10, 10, 1, 0));   -- NULL: no Z
SELECT pcid(tpcboxX(0, 0, 10, 10, 7, 0));
SELECT SRID(tpcboxX(0, 0, 10, 10, 1, 4326));

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

SELECT tpcboxX(0, 0, 5, 5, 1, 0) + tpcboxX(3, 3, 10, 10, 1, 0);
SELECT tpcboxX(0, 0, 5, 5, 1, 0) * tpcboxX(3, 3, 10, 10, 1, 0);
SELECT tpcboxX(0, 0, 5, 5, 1, 0) * tpcboxX(50, 50, 60, 60, 1, 0);  -- NULL (disjoint)

-- A box carrying coordinates states the schema they are read in, so a box
-- naming schema 0 and one naming schema 1 have no common extent to combine
SELECT tpcboxX(0, 0, 5, 5, 0, 0) + tpcboxX(3, 3, 10, 10, 1, 0);
SELECT tpcboxX(0, 0, 5, 5, 0, 0) * tpcboxX(3, 3, 10, 10, 1, 0);

-------------------------------------------------------------------------------
-- Topological predicates — same pcid
-------------------------------------------------------------------------------

SELECT tpcboxX(0, 0, 10, 10, 1, 0) @> tpcboxX(2, 2, 8, 8, 1, 0);
SELECT tpcboxX(0, 0, 10, 10, 1, 0) @> tpcboxX(2, 2, 20, 20, 1, 0);
SELECT tpcboxX(2, 2, 8, 8, 1, 0)   <@ tpcboxX(0, 0, 10, 10, 1, 0);
SELECT tpcboxX(0, 0, 5, 5, 1, 0)   && tpcboxX(3, 3, 10, 10, 1, 0);
SELECT tpcboxX(0, 0, 5, 5, 1, 0)   && tpcboxX(50, 50, 60, 60, 1, 0);
SELECT tpcboxX(0, 0, 5, 5, 1, 0)   ~= tpcboxX(0, 0, 5, 5, 1, 0);

-- The portable spelling of each operator above answers the same
SELECT contains(tpcboxX(0, 0, 10, 10, 1, 0), tpcboxX(2, 2, 8, 8, 1, 0));
SELECT contained(tpcboxX(2, 2, 8, 8, 1, 0), tpcboxX(0, 0, 10, 10, 1, 0));
SELECT overlaps(tpcboxX(0, 0, 5, 5, 1, 0), tpcboxX(3, 3, 10, 10, 1, 0));
SELECT same(tpcboxX(0, 0, 5, 5, 1, 0), tpcboxX(0, 0, 5, 5, 1, 0));
SELECT tpcboxX(0, 0, 5, 5, 1, 0)  -|- tpcboxX(5, 0, 10, 5, 1, 0);
SELECT adjacent(tpcboxX(0, 0, 5, 5, 1, 0), tpcboxX(5, 0, 10, 5, 1, 0));

-------------------------------------------------------------------------------
-- Topological predicates — the schema decides what a coordinate means
-------------------------------------------------------------------------------

-- Two schemas give the same number two meanings, so the boxes are not
-- comparable and the predicate has no answer to give
SELECT tpcboxX(0, 0, 10, 10, 1, 0) @> tpcboxX(2, 2, 8, 8, 2, 0);
SELECT tpcboxX(0, 0, 5, 5, 1, 0)   && tpcboxX(0, 0, 5, 5, 2, 0);
SELECT tpcboxX(0, 0, 5, 5, 1, 0)   ~= tpcboxX(0, 0, 5, 5, 2, 0);

-- Schema 0 is a schema like any other once a box carries coordinates
SELECT tpcboxX(0, 0, 5, 5, 0, 0) && tpcboxX(3, 3, 10, 10, 1, 0);
SELECT tpcboxX(0, 0, 5, 5, 0, 0) @> tpcboxX(1, 1, 4, 4, 1, 0);

-- The SRID is read on the same terms: one schema, two reference systems, so
-- the same pair of numbers denotes two different places
SELECT tpcboxX(0, 0, 5, 5, 1, 4326) && tpcboxX(3, 3, 10, 10, 1, 3857);
SELECT tpcboxX(0, 0, 5, 5, 1, 4326) + tpcboxX(3, 3, 10, 10, 1, 3857);

-- A box carrying no coordinates names no schema, so it meets a box of any
-- schema: this is the shape a time-only query box takes against an index
SELECT tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0) &&
  tpcboxT(tstzspan '[2024-01-02, 2024-01-03]', 1);
SELECT tpcboxT(tstzspan '[2024-01-01, 2024-01-02]', 0) +
  tpcboxT(tstzspan '[2024-01-02, 2024-01-03]', 1);

-------------------------------------------------------------------------------
-- Extent aggregation
-------------------------------------------------------------------------------

-- The aggregate answers the extent of the boxes it folds, so it is bounded by
-- the same comparability the operators are: an extent spanning two schemas, or
-- two reference systems, states a region no schema can read
SELECT extent(b) FROM (VALUES (tpcboxX(0, 0, 5, 5, 1, 0)),
  (tpcboxX(3, 3, 10, 10, 1, 0))) t(b);
SELECT extent(b) FROM (VALUES (tpcboxX(0, 0, 5, 5, 1, 0)),
  (tpcboxX(3, 3, 10, 10, 2, 0))) t(b);
SELECT extent(b) FROM (VALUES (tpcboxX(0, 0, 5, 5, 0, 0)),
  (tpcboxX(3, 3, 10, 10, 1, 0))) t(b);
SELECT extent(b) FROM (VALUES (tpcboxX(0, 0, 5, 5, 1, 4326)),
  (tpcboxX(3, 3, 10, 10, 1, 3857))) t(b);

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT tpcboxX(0, 0, 5, 5, 1, 0) =  tpcboxX(0, 0, 5, 5, 1, 0);
SELECT tpcboxX(0, 0, 5, 5, 1, 0) <> tpcboxX(0, 0, 5, 5, 2, 0);
SELECT tpcboxX(0, 0, 5, 5, 1, 0) <  tpcboxX(0, 0, 5, 5, 2, 0);

-------------------------------------------------------------------------------
