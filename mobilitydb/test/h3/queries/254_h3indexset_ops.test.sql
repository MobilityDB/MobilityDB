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

-- Set-theoretic operators on h3indexset. Only the operators
-- that don't depend on a total order of the basetype are exposed (see
-- 254_h3indexset_ops.in.sql for the rationale).

-------------------------------------------------------------------------------
-- contains  @>
-------------------------------------------------------------------------------

-- element in set
SELECT h3indexset '{8a2a1072b59ffff, 880326b885fffff}' @> h3index '8a2a1072b59ffff';

-- element not in set
SELECT NOT (h3indexset '{8a2a1072b59ffff}' @> h3index '831c02fffffffff');

-- set contains subset
SELECT h3indexset '{8a2a1072b59ffff, 880326b885fffff, 831c02fffffffff}'
  @> h3indexset '{8a2a1072b59ffff, 831c02fffffffff}';

-- set does NOT contain when an element is missing
SELECT NOT (h3indexset '{8a2a1072b59ffff}'
  @> h3indexset '{8a2a1072b59ffff, 831c02fffffffff}');

-------------------------------------------------------------------------------
-- contained by  <@
-------------------------------------------------------------------------------

-- element in set (commutator of @>)
SELECT h3index '8a2a1072b59ffff' <@ h3indexset '{8a2a1072b59ffff, 880326b885fffff}';

-- subset in superset
SELECT h3indexset '{8a2a1072b59ffff}'
  <@ h3indexset '{8a2a1072b59ffff, 880326b885fffff}';

-------------------------------------------------------------------------------
-- overlaps  &&
-------------------------------------------------------------------------------

-- Sets that share an element
SELECT h3indexset '{8a2a1072b59ffff, 831c02fffffffff}'
  && h3indexset '{831c02fffffffff, 880326b885fffff}';

-- Sets that share no element
SELECT NOT (h3indexset '{8a2a1072b59ffff}'
  && h3indexset '{831c02fffffffff}');

-------------------------------------------------------------------------------
-- union  +
-------------------------------------------------------------------------------

-- set + element
SELECT numValues(
  h3indexset '{8a2a1072b59ffff}' + h3index '831c02fffffffff') = 2;

-- element + set (commutator)
SELECT numValues(
  h3index '831c02fffffffff' + h3indexset '{8a2a1072b59ffff}') = 2;

-- set + set (duplicates collapse)
SELECT numValues(
  h3indexset '{8a2a1072b59ffff, 831c02fffffffff}'
  + h3indexset '{831c02fffffffff, 880326b885fffff}') = 3;

-- Adding a duplicate is idempotent
SELECT numValues(
  h3indexset '{8a2a1072b59ffff}' + h3index '8a2a1072b59ffff') = 1;

-------------------------------------------------------------------------------
-- difference  -
-------------------------------------------------------------------------------

-- set - element (drop one)
SELECT numValues(
  h3indexset '{8a2a1072b59ffff, 831c02fffffffff}' - h3index '831c02fffffffff') = 1;

-- set - set
SELECT numValues(
  h3indexset '{8a2a1072b59ffff, 831c02fffffffff, 880326b885fffff}'
  - h3indexset '{831c02fffffffff}') = 2;

-- Removing an absent element is a no-op (result equals input)
SELECT h3indexset '{8a2a1072b59ffff}' - h3index '831c02fffffffff'
  = h3indexset '{8a2a1072b59ffff}';

-------------------------------------------------------------------------------
-- intersection  *
-------------------------------------------------------------------------------

-- set * set (shared element)
SELECT numValues(
  h3indexset '{8a2a1072b59ffff, 831c02fffffffff}'
  * h3indexset '{831c02fffffffff, 880326b885fffff}') = 1;

-- Intersection with element
SELECT numValues(
  h3indexset '{8a2a1072b59ffff, 831c02fffffffff}' * h3index '831c02fffffffff') = 1;

-------------------------------------------------------------------------------
