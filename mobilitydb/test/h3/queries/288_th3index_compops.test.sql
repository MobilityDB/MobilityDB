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

-- Comparison ops: ever/always + temporal eq/ne on th3index.
-- No h3_adapter.c dependency (the operators route through
-- eacomp_temporal_* / tcomp_temporal_* which are purely generic).

-------------------------------------------------------------------------------
-- eEq / ?= : at least one instant equals the probe
-------------------------------------------------------------------------------

-- (h3index, th3index)
SELECT eEq(590464338553208831::h3index,
  th3index '831c02fffffffff@2001-01-01');
SELECT 590464338553208831::h3index ?=
  th3index '831c02fffffffff@2001-01-01';

-- Cell not present in the trajectory → false
SELECT 612544986753269759::h3index ?= th3index '831c02fffffffff@2001-01-01';

-- (th3index, bigint) — same as above, commuted
SELECT th3index '831c02fffffffff@2001-01-01' ?= 590464338553208831::h3index;

-- (th3index, th3index)
SELECT th3index '831c02fffffffff@2001-01-01'
  ?= th3index '831c02fffffffff@2001-01-01';

-- A mid-trajectory match — ?= is ever-equal, returns true on any instant match
SELECT th3index
  '[831c02fffffffff@2001-01-01, 880326b885fffff@2001-01-05]'
  ?= 612544986753269759::h3index;

-------------------------------------------------------------------------------
-- aEq / %= : every instant equals the probe
-------------------------------------------------------------------------------

-- Constant trajectory — always equal
SELECT th3index '831c02fffffffff@2001-01-01' %= 590464338553208831::h3index;

-- Mixed trajectory — not always
SELECT th3index
  '{831c02fffffffff@2001-01-01, 880326b885fffff@2001-01-02}'
  %= 590464338553208831::h3index;

-- Two identical trajectories — always equal
SELECT th3index '[831c02fffffffff@2001-01-01, 831c02fffffffff@2001-01-02]'
  %= th3index '[831c02fffffffff@2001-01-01, 831c02fffffffff@2001-01-02]';

-------------------------------------------------------------------------------
-- eNe / ?<> and aNe / %<>
-------------------------------------------------------------------------------

-- Trajectory has at least one instant ≠ probe
SELECT th3index
  '{831c02fffffffff@2001-01-01, 880326b885fffff@2001-01-02}'
  ?<> 590464338553208831::h3index;

-- Trajectory is always different from the probe
SELECT th3index '831c02fffffffff@2001-01-01' %<> 612544986753269759::h3index;

-- A trajectory can't be both ever-equal AND always-not-equal
SELECT NOT (
  (th3index '831c02fffffffff@2001-01-01' ?= 590464338553208831::h3index)
  AND
  (th3index '831c02fffffffff@2001-01-01' %<> 590464338553208831::h3index));

-------------------------------------------------------------------------------
-- temporal_teq / #= : tbool result
-------------------------------------------------------------------------------

SELECT th3index '831c02fffffffff@2001-01-01' #= 590464338553208831::h3index;
SELECT th3index '831c02fffffffff@2001-01-01' #= 612544986753269759::h3index;

-- Sequence: returns a tbool with per-instant truth
SELECT th3index
  '[831c02fffffffff@2001-01-01, 880326b885fffff@2001-01-02]'
  #= 590464338553208831::h3index;

-- Two temporals — pointwise equality over the shared time axis
SELECT th3index '[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02]'
  #= th3index '[831c02fffffffff@2001-01-01, 831c02fffffffff@2001-01-02]';

-------------------------------------------------------------------------------
-- temporal_tne / #<>
-------------------------------------------------------------------------------

SELECT th3index '831c02fffffffff@2001-01-01' #<> 590464338553208831::h3index;
SELECT th3index '831c02fffffffff@2001-01-01' #<> 612544986753269759::h3index;

-------------------------------------------------------------------------------
