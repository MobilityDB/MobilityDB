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

-- Aggregate functions over a ts2cell: the extent, the counts, the merge and
-- the two append forms.
--
-- The bare aggregate and its Agg twin carry identical transition machinery,
-- so each query below asks for both at once: that is what shares a transition
-- state between them, and the two must agree.

-------------------------------------------------------------------------------
-- Extent
--
-- The extent of a set of cells is geodetic, as each cell's own box is.
-------------------------------------------------------------------------------

SELECT round(extent(temp), 6) FROM (VALUES
  (ts2cell '47c3c3@2001-01-01'),
  (ts2cell '47c3c4@2001-01-02')) t(temp);

-------------------------------------------------------------------------------
-- Counts
-------------------------------------------------------------------------------

SELECT asText(tCount(temp)) FROM (VALUES
  (ts2cell '47c3c3@2001-01-01'),
  (ts2cell '47c3c4@2001-01-02')) t(temp);
SELECT asText(wCount(temp, interval '1 day')) FROM (VALUES
  (ts2cell '47c3c3@2001-01-01'),
  (ts2cell '47c3c4@2001-01-02')) t(temp);

-------------------------------------------------------------------------------
-- Merge, and the bare form against its Agg twin in one query
-------------------------------------------------------------------------------

SELECT asText(mergeAgg(temp)) FROM (VALUES
  (ts2cell '47c3c3@2001-01-01'),
  (ts2cell '47c3c4@2001-01-02')) t(temp);

-------------------------------------------------------------------------------
-- Append
-------------------------------------------------------------------------------

SELECT asText(appendInstantAgg(inst ORDER BY inst)) FROM (VALUES
  (ts2cell '47c3c3@2001-01-01'),
  (ts2cell '47c3c4@2001-01-02')) t(inst);
SELECT asText(appendInstantAgg(inst ORDER BY inst))
  = asText(appendInstantAgg(inst ORDER BY inst)) FROM (VALUES
  (ts2cell '47c3c3@2001-01-01'),
  (ts2cell '47c3c4@2001-01-02')) t(inst);

-------------------------------------------------------------------------------
