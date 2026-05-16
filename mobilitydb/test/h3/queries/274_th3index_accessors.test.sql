-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Accessors: startValue / endValue / valueN / getValues /
-- valueAtTimestamp on th3index. All route to the generic
-- `Temporal_*` C symbols — no h3_adapter.c dependency, so these
-- tests run today against any build that has H3=ON.

-- Test cells (h3-pg fixture):
--   590464338553208831 = res 3 hexagon
--   590464201114255359 = res 3 pentagon
--   612544986753269759 = res 8 hexagon

-------------------------------------------------------------------------------
-- startValue
-------------------------------------------------------------------------------

SELECT startValue(th3index '590464338553208831@2001-01-01');
SELECT startValue(th3index
  '{590464338553208831@2001-01-01, 590464201114255359@2001-01-02}');
SELECT startValue(th3index
  '[590464338553208831@2001-01-01, 590464201114255359@2001-01-02]');

-------------------------------------------------------------------------------
-- endValue
-------------------------------------------------------------------------------

SELECT endValue(th3index '590464338553208831@2001-01-01');
SELECT endValue(th3index
  '{590464338553208831@2001-01-01, 590464201114255359@2001-01-02}');

-- On a constant single-value sequence, start = end
SELECT startValue(th3index '590464338553208831@2001-01-01')
  = endValue(th3index '590464338553208831@2001-01-01');

-------------------------------------------------------------------------------
-- valueN
-------------------------------------------------------------------------------

-- 1-indexed
SELECT valueN(th3index
  '{590464338553208831@2001-01-01, 590464201114255359@2001-01-02}', 1);
SELECT valueN(th3index
  '{590464338553208831@2001-01-01, 590464201114255359@2001-01-02}', 2);

-- Out-of-range returns NULL (not an error)
SELECT valueN(th3index '590464338553208831@2001-01-01', 99);

-------------------------------------------------------------------------------
-- getValues
-------------------------------------------------------------------------------

-- Single-instant: a 1-element h3indexset
SELECT getValues(th3index '590464338553208831@2001-01-01');

-- Duplicates collapse (same value at two instants)
SELECT getValues(th3index
  '{590464338553208831@2001-01-01, 590464338553208831@2001-01-02}');

-- Three distinct values
SELECT getValues(th3index
  '[590464338553208831@2001-01-01, 590464201114255359@2001-01-02,
    612544986753269759@2001-01-03]');

-------------------------------------------------------------------------------
-- valueAtTimestamp
-------------------------------------------------------------------------------

SELECT valueAtTimestamp(th3index '590464338553208831@2001-01-01',
  '2001-01-01');

-- At a time inside a step-interp sequence — returns the prior instant's value
SELECT valueAtTimestamp(th3index
  '[590464338553208831@2001-01-01, 590464201114255359@2001-01-05]',
  '2001-01-03');

-- Before the start of a sequence — returns NULL
SELECT valueAtTimestamp(th3index
  '[590464338553208831@2001-01-01, 590464201114255359@2001-01-05]',
  '2000-12-31');

-------------------------------------------------------------------------------
