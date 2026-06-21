-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- §1.7 Vertices — three lifts.
--
-- `cell_to_vertex` and `is_valid_vertex` are backed by h3_generated.c
-- and work without any adapter. `vertex_to_latlng` needs the
-- h3_vertex_to_gs_point adapter.
--
-- Test cells (h3-pg's vertex.sql fixture):
--   612544986753269759 = res 8 hexagon

-------------------------------------------------------------------------------
-- h3_cell_to_vertex(th3index, integer) — lift_with_const
-------------------------------------------------------------------------------

-- A hexagon has six vertices (numbered 0..5)
SELECT h3_cell_to_vertex(th3index '612544986753269759@2001-01-01', 0)
  IS NOT NULL;
SELECT h3_cell_to_vertex(th3index '612544986753269759@2001-01-01', 5)
  IS NOT NULL;

-- All four temporal subtypes
SELECT h3_cell_to_vertex(th3index
  '{612544986753269759@2001-01-01, 612544986761658367@2001-01-02}', 0)
  IS NOT NULL;
SELECT h3_cell_to_vertex(th3index
  '[612544986753269759@2001-01-01, 612544986761658367@2001-01-02]', 0)
  IS NOT NULL;

-- Distinct vertex numbers must yield distinct vertices for the same cell
SELECT h3_cell_to_vertex(th3index '612544986753269759@2001-01-01', 0)
  <> h3_cell_to_vertex(th3index '612544986753269759@2001-01-01', 1);

-- Out-of-range vertex number must error
/* Errors */
SELECT h3_cell_to_vertex(th3index '612544986753269759@2001-01-01', 9);

-------------------------------------------------------------------------------
-- h3_is_valid_vertex
-------------------------------------------------------------------------------

-- A freshly built vertex is valid
SELECT h3_is_valid_vertex(
  h3_cell_to_vertex(th3index '612544986753269759@2001-01-01', 0));

-- A plain cell is not a valid vertex
SELECT h3_is_valid_vertex(th3index '612544986753269759@2001-01-01');
SELECT h3_is_valid_vertex(th3index '0@2001-01-01');

-------------------------------------------------------------------------------
-- h3_vertex_to_latlng — needs h3_vertex_to_gs_point adapter
-------------------------------------------------------------------------------

SELECT h3_vertex_to_latlng(
  h3_cell_to_vertex(th3index '612544986753269759@2001-01-01', 0))
  IS NOT NULL;

-------------------------------------------------------------------------------
