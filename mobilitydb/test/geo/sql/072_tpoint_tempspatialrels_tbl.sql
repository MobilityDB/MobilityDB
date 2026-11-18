-- CREATE FUNCTION testSpatialRelsM() RETURNS void AS $$
-- BEGIN
-------------------------------------------------------------------------------
-- tContains
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint
  WHERE tContains(g, temp) IS NOT NULL;
-- Step interpolation
SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint_step_contseq
  WHERE tContains(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint_step_seqset
  WHERE tContains(g, temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint
  WHERE tContains(g, temp) ?= true <> eContains(g, temp);
-- Step interpolation
SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint_step_contseq
  WHERE tContains(g, temp) ?= true <> eContains(g, temp);
SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint_step_seqset
  WHERE tContains(g, temp) ?= true <> eContains(g, temp);

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tDisjoint(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tDisjoint(temp, g) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_contseq
  WHERE tDisjoint(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tDisjoint(g, temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tDisjoint(g, temp) ?= true <> eDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tDisjoint(temp, g) ?= true <> eDisjoint(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_contseq
  WHERE tDisjoint(g, temp) ?= true <> eDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tDisjoint(g, temp) ?= true <> eDisjoint(g, temp);

-- Temporal points
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tDisjoint(t1.temp, t2.temp) ?= true <> eDisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
  WHERE tDisjoint(t1.temp, t2.temp) ?= true <> eDisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tIntersects(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tIntersects(temp, g) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_contseq
  WHERE tIntersects(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tIntersects(g, temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tIntersects(g, temp) ?= true <> eIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tIntersects(temp, g) ?= true <> eIntersects(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_contseq
  WHERE tIntersects(g, temp) ?= true <> eIntersects(g, temp);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tIntersects(g, temp) ?= true <> eIntersects(g, temp);

-- Temporal points
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tIntersects(t1.temp, t2.temp) ?= true <> eIntersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2
  WHERE tIntersects(t1.temp, t2.temp) ?= true <> eIntersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint
  WHERE tTouches(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tTouches(temp, g) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint_step_contseq
  WHERE tTouches(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint_step_seqset
  WHERE tTouches(g, temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint
  WHERE tTouches(g, temp) ?= true <> eTouches(g, temp);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom
  WHERE tTouches(temp, g) ?= true <> eTouches(temp, g);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint_step_contseq
  WHERE tTouches(g, temp) ?= true <> eTouches(g, temp);
SELECT COUNT(*) FROM tbl_geom, tbl_tgeompoint_step_seqset
  WHERE tTouches(g, temp) ?= true <> eTouches(g, temp);

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tDwithin(g, temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tDwithin(temp, g, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_contseq
  WHERE tDwithin(g, temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tDwithin(g, temp, 10) IS NOT NULL;

-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint3D t2
  WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint t2
  WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-- Geographies, on the spheroid and on the sphere
SELECT COUNT(*) FROM tbl_geog_point, tbl_tgeogpoint
  WHERE tDwithin(g, temp, 500000) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geog_point
  WHERE tDwithin(temp, g, 500000, false) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
  WHERE tDwithin(t1.temp, t2.temp, 500000) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint
  WHERE tDwithin(g, temp, 10) ?= true <> edwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint, tbl_geom_point
  WHERE tDwithin(temp, g, 10) ?= true <> edwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);

-- Step interpolation
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_contseq
  WHERE tDwithin(g, temp, 10) ?= true <> edwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_geom_point, tbl_tgeompoint_step_seqset
  WHERE tDwithin(g, temp, 10) ?= true <> edwithin(g, temp, 10);

-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tgeompoint t1, tbl_tgeompoint3D t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);
SELECT COUNT(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);

-- Geographies, on the spheroid and on the sphere
SELECT COUNT(*) FROM tbl_geog_point, tbl_tgeogpoint
  WHERE tDwithin(g, temp, 500000) ?= true <> eDwithin(g, temp, 500000);
SELECT COUNT(*) FROM tbl_tgeogpoint, tbl_geog_point
  WHERE tDwithin(temp, g, 500000, false) ?= true <>
    eDwithin(temp, g, 500000, false);
SELECT COUNT(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2
  WHERE tDwithin(t1.temp, t2.temp, 500000) ?= true <>
    eDwithin(t1.temp, t2.temp, 500000);

-------------------------------------------------------------------------------
-- END;
-- $$ LANGUAGE plpgsql;

-- SELECT pg_backend_pid()

-- SELECT testTopologicalOps()
-------------------------------------------------------------------------------
