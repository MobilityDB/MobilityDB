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

-- Value-level tests for tpcbox (constructors, accessors, set operations,
-- topological predicates, comparison operators).

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT tpcbox(0, 0, 10, 10, 1, 0);
SELECT tpcbox_z(0, 0, 0, 10, 10, 10, 1, 0);
SELECT tpcbox_t(tstzspan '[2024-01-01, 2024-01-02]', 1);
SELECT tpcbox(0, 0, 10, 10, tstzspan '[2024-01-01, 2024-01-02]', 1, 0);
SELECT tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1, 0);

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT hasX(tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1, 0));
SELECT hasZ(tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1, 0));
SELECT hasT(tpcbox_zt(0, 0, 0, 10, 10, 10,
  tstzspan '[2024-01-01, 2024-01-02]', 1, 0));
SELECT hasZ(tpcbox(0, 0, 10, 10, 1, 0));   -- false: no Z
SELECT hasT(tpcbox(0, 0, 10, 10, 1, 0));   -- false: no T

SELECT xmin(tpcbox(0, 0, 10, 10, 1, 0));
SELECT xmax(tpcbox(0, 0, 10, 10, 1, 0));
SELECT ymin(tpcbox(0, 0, 10, 10, 1, 0));
SELECT ymax(tpcbox(0, 0, 10, 10, 1, 0));
SELECT zmin(tpcbox_z(0, 0, 1, 10, 10, 9, 1, 0));
SELECT zmax(tpcbox_z(0, 0, 1, 10, 10, 9, 1, 0));
SELECT zmin(tpcbox(0, 0, 10, 10, 1, 0));   -- NULL: no Z
SELECT pcid(tpcbox(0, 0, 10, 10, 7, 0));
SELECT SRID(tpcbox(0, 0, 10, 10, 1, 4326));

-------------------------------------------------------------------------------
-- Set operations
-------------------------------------------------------------------------------

SELECT tpcbox(0, 0, 5, 5, 1, 0) + tpcbox(3, 3, 10, 10, 1, 0);
SELECT tpcbox(0, 0, 5, 5, 1, 0) * tpcbox(3, 3, 10, 10, 1, 0);
SELECT tpcbox(0, 0, 5, 5, 1, 0) * tpcbox(50, 50, 60, 60, 1, 0);  -- NULL (disjoint)

-------------------------------------------------------------------------------
-- Topological predicates — same pcid
-------------------------------------------------------------------------------

SELECT tpcbox(0, 0, 10, 10, 1, 0) @> tpcbox(2, 2, 8, 8, 1, 0);
SELECT tpcbox(0, 0, 10, 10, 1, 0) @> tpcbox(2, 2, 20, 20, 1, 0);
SELECT tpcbox(2, 2, 8, 8, 1, 0)   <@ tpcbox(0, 0, 10, 10, 1, 0);
SELECT tpcbox(0, 0, 5, 5, 1, 0)   && tpcbox(3, 3, 10, 10, 1, 0);
SELECT tpcbox(0, 0, 5, 5, 1, 0)   && tpcbox(50, 50, 60, 60, 1, 0);
SELECT tpcbox(0, 0, 5, 5, 1, 0)   ~= tpcbox(0, 0, 5, 5, 1, 0);
SELECT tpcbox(0, 0, 5, 5, 1, 0)  -|- tpcbox(5, 0, 10, 5, 1, 0);

-------------------------------------------------------------------------------
-- Topological predicates — pcid mismatch always returns false
-------------------------------------------------------------------------------

SELECT tpcbox(0, 0, 10, 10, 1, 0) @> tpcbox(2, 2, 8, 8, 2, 0);
SELECT tpcbox(0, 0, 5, 5, 1, 0)   && tpcbox(0, 0, 5, 5, 2, 0);
SELECT tpcbox(0, 0, 5, 5, 1, 0)   ~= tpcbox(0, 0, 5, 5, 2, 0);

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT tpcbox(0, 0, 5, 5, 1, 0) =  tpcbox(0, 0, 5, 5, 1, 0);
SELECT tpcbox(0, 0, 5, 5, 1, 0) <> tpcbox(0, 0, 5, 5, 2, 0);
SELECT tpcbox(0, 0, 5, 5, 1, 0) <  tpcbox(0, 0, 5, 5, 2, 0);

-------------------------------------------------------------------------------
