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

-- Comparison operators (=, <>, <, <=, >=, >, cmp, B-tree opclass) for the
-- static pcpoint / pcpatch base types. Uses the ergonomic pcpoint/pcpatch
-- constructors (same pattern as 420_tpcpoint.test.sql / 430_tpcpatch.test.sql).

-------------------------------------------------------------------------------
-- pcpoint
-------------------------------------------------------------------------------

-- Equality of two identical points built independently.
SELECT pcpoint(1, 1.0, 1.0, 1.0) = pcpoint(1, 1.0, 1.0, 1.0);
SELECT pcpoint(1, 1.0, 1.0, 1.0) = pcpoint(1, 2.0, 2.0, 2.0);

SELECT pcpoint(1, 1.0, 1.0, 1.0) <> pcpoint(1, 1.0, 1.0, 1.0);
SELECT pcpoint(1, 1.0, 1.0, 1.0) <> pcpoint(1, 2.0, 2.0, 2.0);

-- cmp() is reflexive and zero only for equal values.
SELECT cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 1.0, 1.0, 1.0)) = 0;
SELECT cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <> 0;

-- The </<=/>/>= operators agree with the sign of cmp().
SELECT (pcpoint(1, 1.0, 1.0, 1.0) < pcpoint(1, 2.0, 2.0, 2.0)) =
  (cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) < 0);
SELECT (pcpoint(1, 1.0, 1.0, 1.0) <= pcpoint(1, 2.0, 2.0, 2.0)) =
  (cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <= 0);
SELECT (pcpoint(1, 1.0, 1.0, 1.0) > pcpoint(1, 2.0, 2.0, 2.0)) =
  (cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) > 0);
SELECT (pcpoint(1, 1.0, 1.0, 1.0) >= pcpoint(1, 2.0, 2.0, 2.0)) =
  (cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) >= 0);
-- Antisymmetry.
SELECT (pcpoint(1, 1.0, 1.0, 1.0) < pcpoint(1, 2.0, 2.0, 2.0)) =
  (pcpoint(1, 2.0, 2.0, 2.0) > pcpoint(1, 1.0, 1.0, 1.0));
-- Irreflexivity of the strict operators.
SELECT pcpoint(1, 1.0, 1.0, 1.0) < pcpoint(1, 1.0, 1.0, 1.0);
SELECT pcpoint(1, 1.0, 1.0, 1.0) > pcpoint(1, 1.0, 1.0, 1.0);
SELECT pcpoint(1, 1.0, 1.0, 1.0) <= pcpoint(1, 1.0, 1.0, 1.0);
SELECT pcpoint(1, 1.0, 1.0, 1.0) >= pcpoint(1, 1.0, 1.0, 1.0);

-- ORDER BY over three distinct values produces a sequence that is
-- non-decreasing with respect to the <= operator (a genuine B-tree total
-- order, whatever its relationship to the coordinate magnitudes).
WITH v(pt) AS (
  VALUES (pcpoint(1, 1.0, 1.0, 1.0)), (pcpoint(1, 2.0, 2.0, 2.0)),
    (pcpoint(1, 3.0, 3.0, 3.0))
), ordered AS (
  SELECT pt, row_number() OVER (ORDER BY pt) AS rn FROM v
)
SELECT (SELECT count(*) FROM ordered) = 3
  AND bool_and(a.pt <= b.pt)
FROM ordered a JOIN ordered b ON b.rn = a.rn + 1;

-------------------------------------------------------------------------------
-- pcpatch
-------------------------------------------------------------------------------

-- Equality of two identical patches built independently.
SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) =
  pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0));
SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) =
  pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0));

SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <>
  pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0));
SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <>
  pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0));

-- cmp() is reflexive and zero only for equal values.
SELECT cmp(pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)),
  pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0))) = 0;
SELECT cmp(pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)),
  pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) <> 0;

-- The </<=/>/>= operators agree with the sign of cmp().
SELECT (pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <
    pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) =
  (cmp(pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)),
    pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) < 0);
SELECT (pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) >
    pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) =
  (cmp(pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)),
    pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) > 0);
-- Antisymmetry.
SELECT (pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <
    pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) =
  (pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0)) >
    pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)));
-- Irreflexivity of the strict operators.
SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <
  pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0));
SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <=
  pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0));

-- ORDER BY over three distinct values produces a sequence that is
-- non-decreasing with respect to the <= operator.
WITH v(pa) AS (
  VALUES
    (pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0))),
    (pcpatch(pcpoint(1, 3.0, 3.0, 3.0), pcpoint(1, 4.0, 4.0, 4.0))),
    (pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0)))
), ordered AS (
  SELECT pa, row_number() OVER (ORDER BY pa) AS rn FROM v
)
SELECT (SELECT count(*) FROM ordered) = 3
  AND bool_and(a.pa <= b.pa)
FROM ordered a JOIN ordered b ON b.rn = a.rn + 1;

-------------------------------------------------------------------------------
