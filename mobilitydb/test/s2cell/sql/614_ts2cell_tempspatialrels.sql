-------------------------------------------------------------------------------

-- Temporal spatial relationships between two ts2cell values. Each converts
-- both operands to the temporal geography of their boundary and delegates to
-- the temporal relationship of two temporal geographies, answered on the
-- sphere, so the answer must equal the same relationship asked of
-- cellToBoundary directly.

-------------------------------------------------------------------------------
-- The delegation identity
-------------------------------------------------------------------------------

SELECT asText(tIntersects(ts2cell '[47c3c3@2001-01-01, 47c3c5@2001-01-02]',
    ts2cell '[47c3c3@2001-01-01, 47c3c3@2001-01-02]'))
  = asText(tIntersects(cellToBoundary(ts2cell '[47c3c3@2001-01-01, 47c3c5@2001-01-02]'),
      cellToBoundary(ts2cell '[47c3c3@2001-01-01, 47c3c3@2001-01-02]')));
SELECT asText(tDisjoint(ts2cell '[47c3c3@2001-01-01, 47c3c5@2001-01-02]',
    ts2cell '[47c3c3@2001-01-01, 47c3c3@2001-01-02]'))
  = asText(tDisjoint(cellToBoundary(ts2cell '[47c3c3@2001-01-01, 47c3c5@2001-01-02]'),
      cellToBoundary(ts2cell '[47c3c3@2001-01-01, 47c3c3@2001-01-02]')));

-------------------------------------------------------------------------------
-- The answers themselves
-------------------------------------------------------------------------------

SELECT asText(tIntersects(ts2cell '[47c3c3@2001-01-01]', ts2cell '[47c3c3@2001-01-01]'));
SELECT asText(tDisjoint(ts2cell '[47c3c3@2001-01-01]', ts2cell '[47c3c3@2001-01-01]'));

-------------------------------------------------------------------------------
