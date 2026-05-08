-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Comparison ops: ever/always + temporal eq/ne on th3index.
-- No h3_adapter.c dependency (the operators route through
-- eacomp_temporal_* / tcomp_temporal_* which are purely generic).

-------------------------------------------------------------------------------
-- ever_eq / ?= : at least one instant equals the probe
-------------------------------------------------------------------------------

-- (bigint, th3index)
SELECT ever_eq(590464338553208831::h3index,
  th3index '590464338553208831@2001-01-01');
SELECT 590464338553208831::bigint ?=
  th3index '590464338553208831@2001-01-01';

-- Cell not present in the trajectory → false
SELECT 0::bigint ?= th3index '590464338553208831@2001-01-01';

-- (th3index, bigint) — same as above, commuted
SELECT th3index '590464338553208831@2001-01-01' ?= 590464338553208831::h3index;

-- (th3index, th3index)
SELECT th3index '590464338553208831@2001-01-01'
  ?= th3index '590464338553208831@2001-01-01';

-- A mid-trajectory match — ?= is ever-equal, returns true on any instant match
SELECT th3index
  '[590464338553208831@2001-01-01, 612544986753269759@2001-01-05]'
  ?= 612544986753269759::h3index;

-------------------------------------------------------------------------------
-- always_eq / %= : every instant equals the probe
-------------------------------------------------------------------------------

-- Constant trajectory — always equal
SELECT th3index '590464338553208831@2001-01-01' %= 590464338553208831::h3index;

-- Mixed trajectory — not always
SELECT th3index
  '{590464338553208831@2001-01-01, 612544986753269759@2001-01-02}'
  %= 590464338553208831::h3index;

-- Two identical trajectories — always equal
SELECT th3index '[590464338553208831@2001-01-01, 590464338553208831@2001-01-02]'
  %= th3index '[590464338553208831@2001-01-01, 590464338553208831@2001-01-02]';

-------------------------------------------------------------------------------
-- ever_ne / ?<> and always_ne / %<>
-------------------------------------------------------------------------------

-- Trajectory has at least one instant ≠ probe
SELECT th3index
  '{590464338553208831@2001-01-01, 612544986753269759@2001-01-02}'
  ?<> 590464338553208831::h3index;

-- Trajectory is always different from the probe
SELECT th3index '590464338553208831@2001-01-01' %<> 612544986753269759::h3index;

-- A trajectory can't be both ever-equal AND always-not-equal
SELECT NOT (
  (th3index '590464338553208831@2001-01-01' ?= 590464338553208831::h3index)
  AND
  (th3index '590464338553208831@2001-01-01' %<> 590464338553208831::h3index));

-------------------------------------------------------------------------------
-- temporal_teq / #= : tbool result
-------------------------------------------------------------------------------

SELECT th3index '590464338553208831@2001-01-01' #= 590464338553208831::h3index;
SELECT th3index '590464338553208831@2001-01-01' #= 612544986753269759::h3index;

-- Sequence: returns a tbool with per-instant truth
SELECT th3index
  '[590464338553208831@2001-01-01, 612544986753269759@2001-01-02]'
  #= 590464338553208831::h3index;

-- Two temporals — pointwise equality over the shared time axis
SELECT th3index '[590464338553208831@2001-01-01, 590464201114255359@2001-01-02]'
  #= th3index '[590464338553208831@2001-01-01, 590464338553208831@2001-01-02]';

-------------------------------------------------------------------------------
-- temporal_tne / #<>
-------------------------------------------------------------------------------

SELECT th3index '590464338553208831@2001-01-01' #<> 590464338553208831::h3index;
SELECT th3index '590464338553208831@2001-01-01' #<> 612544986753269759::h3index;

-------------------------------------------------------------------------------
