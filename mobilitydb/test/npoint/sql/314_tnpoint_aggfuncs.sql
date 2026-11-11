-------------------------------------------------------------------------------

-- extent(tnpoint): mixed temporal subtypes folded together, NULLs filtered
-- round to 10 decimal places to suppress platform floating-point ULP differences
SELECT round(extent(temp), 10) FROM ( VALUES
  (NULL::tnpoint),
  ('Npoint(1, 0.5)@2001-01-01'),
  ('{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}')) t(temp);
SELECT round(extent(temp), 10) FROM ( VALUES
  (tnpoint 'Npoint(1, 0.5)@2001-01-01'),
  ('{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}'),
  (NULL)) t(temp);

-- extent(tnpoint): linear sequence + step sequenceset over disjoint footprints
SELECT round(extent(temp), 10) FROM ( VALUES
  (tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]'),
  ('{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.1)@2001-01-04, Npoint(2, 0.2)@2001-01-05]}')) t(temp);

-- extent(tnpoint): single row degenerate cases
SELECT round(extent(temp), 10) FROM ( VALUES (tnpoint 'Npoint(1, 0.5)@2001-01-01')) t(temp);
SELECT round(extent(temp), 10) FROM ( VALUES (NULL::tnpoint)) t(temp);

-- appendInstant(tnpoint): the box of a sequence the aggregate grows holds the
-- stretch of route 3 between 0.5 and 0.9, which reaches beyond its end points
WITH temp(inst) AS (
  SELECT tnpoint 'Npoint(3, 0.1)@2001-01-01' UNION
  SELECT tnpoint 'Npoint(3, 0.5)@2001-01-02' UNION
  SELECT tnpoint 'Npoint(3, 0.9)@2001-01-04' )
SELECT round(stbox(appendInstant(inst ORDER BY inst)), 6),
  stbox(appendInstant(inst ORDER BY inst)) = stbox(tnpoint
    '[Npoint(3, 0.1)@2001-01-01, Npoint(3, 0.5)@2001-01-02, Npoint(3, 0.9)@2001-01-04]')
FROM temp;

-------------------------------------------------------------------------------
