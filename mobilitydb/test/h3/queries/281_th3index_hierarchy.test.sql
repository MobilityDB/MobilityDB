-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- §1.4 Hierarchy — six lifts.
--
-- All six depend on `h3_cell_to_parent[_next]_meos` /
-- `h3_cell_to_center_child[_next]_meos` (h3_adapter.c) for the four
-- parent / center_child entries; the two child_pos entries are
-- backed by h3_generated.c and work without any adapter.
--
-- Test cells:
--   590464338553208831 = res 3 hexagon (parent at res 2 = 0x821c0ffffffffff)
--   612544986753269759 = res 8 hexagon
--   622236750694711295 = res 10 NYC hexagon

-------------------------------------------------------------------------------
-- h3_cell_to_parent(th3index, integer) — lift_with_const
-------------------------------------------------------------------------------

-- Round-trip property: the parent at resolution R has resolution R.
-- This holds without us having to know the parent value.
SELECT h3_get_resolution(
  h3_cell_to_parent(th3index '590464338553208831@2001-01-01', 2));
SELECT h3_get_resolution(
  h3_cell_to_parent(th3index '622236750694711295@2001-01-01', 5));

SELECT h3_get_resolution(h3_cell_to_parent(th3index
  '[590464338553208831@2001-01-01, 590464201114255359@2001-01-02]', 1));

-- res 0 is the coarsest level — asking for a parent at a finer
-- resolution must error (h3-pg semantics).
/* Errors */
SELECT h3_cell_to_parent(th3index '590464338553208831@2001-01-01', 5);

-------------------------------------------------------------------------------
-- h3_cell_to_parent(th3index) — drop one resolution
-------------------------------------------------------------------------------

SELECT h3_get_resolution(
  h3_cell_to_parent(th3index '590464338553208831@2001-01-01'));
SELECT h3_get_resolution(
  h3_cell_to_parent(th3index '622236750694711295@2001-01-01'));

-- Idempotent property: parent(parent(cell)) is the same as parent at res-2
SELECT h3_cell_to_parent(h3_cell_to_parent(
    th3index '622236750694711295@2001-01-01'))
  = h3_cell_to_parent(th3index '622236750694711295@2001-01-01', 8);

-------------------------------------------------------------------------------
-- h3_cell_to_center_child(th3index, integer) — lift_with_const
-------------------------------------------------------------------------------

SELECT h3_get_resolution(
  h3_cell_to_center_child(th3index '590464338553208831@2001-01-01', 5));
SELECT h3_get_resolution(
  h3_cell_to_center_child(th3index '590464338553208831@2001-01-01', 10));

-- Round-trip: the parent of a center child at finer res equals the cell.
SELECT h3_cell_to_parent(
  h3_cell_to_center_child(th3index '590464338553208831@2001-01-01', 7), 3)
  = th3index '590464338553208831@2001-01-01';

-------------------------------------------------------------------------------
-- h3_cell_to_center_child(th3index) — drop one resolution finer
-------------------------------------------------------------------------------

SELECT h3_get_resolution(
  h3_cell_to_center_child(th3index '590464338553208831@2001-01-01'));

-------------------------------------------------------------------------------
-- h3_cell_to_child_pos(th3index, integer)
-------------------------------------------------------------------------------

-- Position of a res-10 NYC cell among the children of its res-9 parent
-- is in [0, 6] (hex parent has 7 children).
SELECT h3_cell_to_child_pos(th3index '622236750694711295@2001-01-01', 9);

-- Position relative to res-3 ancestor (0x83 prefix) is in [0, 7^7 - 1].
SELECT h3_cell_to_child_pos(th3index '622236750694711295@2001-01-01', 3) >= 0;

SELECT h3_cell_to_child_pos(th3index
  '[622236750694711295@2001-01-01, 622236750694711295@2001-01-02]', 9);

-------------------------------------------------------------------------------
-- h3_child_pos_to_cell(tbigint, th3index, integer)
--
-- The two temporal operands must share a time axis; the integer is constant.
-- Round-trip property: child_pos_to_cell(cell_to_child_pos(c, P), P', R) = c
-------------------------------------------------------------------------------

SELECT h3_child_pos_to_cell(
    h3_cell_to_child_pos(th3index '622236750694711295@2001-01-01', 9),
    h3_cell_to_parent(th3index '622236750694711295@2001-01-01', 9),
    10)
  = th3index '622236750694711295@2001-01-01';

-------------------------------------------------------------------------------
