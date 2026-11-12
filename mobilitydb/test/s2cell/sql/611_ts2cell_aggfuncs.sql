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

SELECT asText(merge(temp)) FROM (VALUES
  (ts2cell '47c3c3@2001-01-01'),
  (ts2cell '47c3c4@2001-01-02')) t(temp);

-------------------------------------------------------------------------------
-- Append
-------------------------------------------------------------------------------

SELECT asText(appendInstant(inst ORDER BY inst)) FROM (VALUES
  (ts2cell '47c3c3@2001-01-01'),
  (ts2cell '47c3c4@2001-01-02')) t(inst);

-------------------------------------------------------------------------------
