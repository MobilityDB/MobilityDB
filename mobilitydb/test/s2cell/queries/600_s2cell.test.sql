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

-- Static s2cell SQL type: parser, output, comparison operators, btree +
-- hash opclasses, ASSIGNMENT casts to/from bigint, validity predicates.

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Input and output
--
-- The parser accepts the 16-digit hexadecimal identifier, and the output
-- function answers S2's canonical TOKEN, which is that identifier with its
-- trailing zeros stripped, so the two spellings below denote one cell and the
-- text form round-trips. A literal that encodes no cell is refused rather than
-- stored.
-------------------------------------------------------------------------------

SELECT s2cell '47c3c444c0000000';
SELECT s2cell '47c3c444c';
SELECT s2cell '1';
SELECT s2cell '0';
SELECT s2cell 'zzz';

SELECT s2cell '47c3c444c0000000' = s2cell '47c3c444c0000000';

-- the token and the full identifier are the same cell, so the text form of a
-- cell parses back to that cell
SELECT s2cell '47c3c444c' = s2cell '47c3c444c0000000';

-------------------------------------------------------------------------------
-- Casts to and from bigint
--
-- The two share one representation and the cast carries no function, so the
-- round trip is the identity, and it is explicit in both directions.
-------------------------------------------------------------------------------

SELECT (s2cell '47c3c444c0000000')::bigint;
SELECT ((s2cell '47c3c444c0000000')::bigint)::s2cell = s2cell '47c3c444c0000000';

-------------------------------------------------------------------------------
-- Comparison operators
--
-- The order is the order of the identifier, which follows the Hilbert curve.
-------------------------------------------------------------------------------

SELECT s2cell '47c3c444c0000000' < s2cell '47c3c444c0000004';
SELECT s2cell '47c3c444c0000000' <= s2cell '47c3c444c0000000';
SELECT s2cell '47c3c444c0000004' > s2cell '47c3c444c0000000';
SELECT s2cell '47c3c444c0000000' <> s2cell '47c3c444c0000004';
SELECT cmp(s2cell '47c3c444c0000000', s2cell '47c3c444c0000004');
SELECT cmp(s2cell '47c3c444c0000000', s2cell '47c3c444c0000000');

-------------------------------------------------------------------------------
-- Hash
--
-- Equal cells hash equally, and the seeded form answers a 64-bit value.
-------------------------------------------------------------------------------

SELECT hash(s2cell '47c3c444c0000000') = hash(s2cell '47c3c444c0000000');
SELECT hash(s2cell '47c3c444c0000000') = hash(s2cell '47c3c444c0000004');
SELECT hashExtended(s2cell '47c3c444c0000000', 1) =
  hashExtended(s2cell '47c3c444c0000000', 1);

-------------------------------------------------------------------------------
-- Validity
-------------------------------------------------------------------------------

SELECT isValidCell(s2cell '47c3c444c0000000');
SELECT isValidCell(s2cell '1');

-------------------------------------------------------------------------------
-- Ordering and grouping, which the btree and hash operator classes carry
--
-- No cell family declares a min or a max aggregate over its base type, so what
-- the operator classes answer is DISTINCT, GROUP BY and ORDER BY.
-------------------------------------------------------------------------------

WITH Cells(cell) AS (VALUES
  (s2cell '47c3c444c0000004'), (s2cell '47c3c444c0000000'),
  (s2cell '47c3c444c0000004') )
SELECT count(*), count(DISTINCT cell) FROM Cells;

WITH Cells(cell) AS (VALUES
  (s2cell '47c3c444c0000004'), (s2cell '47c3c444c0000000'),
  (s2cell '47c3c444c0000004') )
SELECT cell, count(*) FROM Cells GROUP BY cell ORDER BY cell;

-------------------------------------------------------------------------------
