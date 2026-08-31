-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
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

-- Static s2cellset SQL type: parser, output, constructor, accessors, the cast
-- from a single cell, set operations, and the btree opclass.
--
-- Every cell here is at level 29, whose level bit sits at position 2, so the
-- literals end in 4, c and 14. A token ending in 8 sets bit 3, which is odd
-- and encodes no level, and the type rejects it.

-------------------------------------------------------------------------------
-- Input and output
--
-- A set prints its elements in ascending identifier order, which is Hilbert
-- order, and duplicates collapse.
-------------------------------------------------------------------------------

SELECT s2cellset '{47c3c444c000000c, 47c3c444c0000004}';
SELECT s2cellset '{47c3c444c0000004, 47c3c444c0000004}';
SELECT s2cell '47c3c444c0000008';

-------------------------------------------------------------------------------
-- Constructor and accessors
-------------------------------------------------------------------------------

SELECT set(ARRAY[s2cell '47c3c444c000000c', s2cell '47c3c444c0000004']);
SELECT numValues(s2cellset '{47c3c444c000000c, 47c3c444c0000004}');
SELECT startValue(s2cellset '{47c3c444c000000c, 47c3c444c0000004}');
SELECT endValue(s2cellset '{47c3c444c000000c, 47c3c444c0000004}');
SELECT valueN(s2cellset '{47c3c444c000000c, 47c3c444c0000004}', 1);
SELECT valueN(s2cellset '{47c3c444c000000c, 47c3c444c0000004}', 3);

-------------------------------------------------------------------------------
-- The cast from a single cell, and set membership
-------------------------------------------------------------------------------

SELECT (s2cell '47c3c444c0000004')::s2cellset;
SELECT s2cell '47c3c444c0000004' <@ s2cellset '{47c3c444c0000004, 47c3c444c000000c}';
SELECT s2cell '47c3c444c0000014' <@ s2cellset '{47c3c444c0000004, 47c3c444c000000c}';
SELECT s2cellset '{47c3c444c0000004}' @> s2cell '47c3c444c0000004';

-------------------------------------------------------------------------------
-- Set operations
-------------------------------------------------------------------------------

SELECT s2cellset '{47c3c444c0000004}' + s2cellset '{47c3c444c000000c}';
SELECT s2cellset '{47c3c444c0000004, 47c3c444c000000c}' * s2cellset '{47c3c444c000000c}';
SELECT s2cellset '{47c3c444c0000004, 47c3c444c000000c}' - s2cellset '{47c3c444c000000c}';
SELECT s2cellset '{47c3c444c0000004}' && s2cellset '{47c3c444c0000004, 47c3c444c000000c}';
SELECT s2cellset '{47c3c444c0000004}' && s2cellset '{47c3c444c0000014}';

-------------------------------------------------------------------------------
-- Comparison, which the btree operator class carries
-------------------------------------------------------------------------------

SELECT s2cellset '{47c3c444c0000004}' = s2cellset '{47c3c444c0000004}';
SELECT s2cellset '{47c3c444c0000004}' < s2cellset '{47c3c444c000000c}';

-------------------------------------------------------------------------------
