-- CREATE FUNCTION testSpatialRelsM() RETURNS void AS $$
-- BEGIN
-------------------------------------------------------------------------------
-- tContains
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE tContains(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE tContains(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE tContains(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE tContains(g, temp) ?= true <> eContains(g, temp);

-------------------------------------------------------------------------------
-- tCovers
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE tCovers(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE tCovers(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE tCovers(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE tCovers(g, temp) ?= true <> eCovers(g, temp);

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE tDisjoint(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE tDisjoint(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE tDisjoint(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE tDisjoint(g, temp) ?= true <> eDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE tDisjoint(temp, g) ?= true <> eDisjoint(temp, g);
  
-- Temporal points
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
  WHERE tDisjoint(t1.temp, t2.temp) ?= true <> eDisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2
  WHERE tDisjoint(t1.temp, t2.temp) ?= true <> eDisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE tIntersects(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE tIntersects(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE tIntersects(temp, g) ?= true <> eDisjoint(temp, g);

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE tIntersects(g, temp) ?= true <> eIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE tIntersects(temp, g) ?= true <> eIntersects(temp, g);

-- Temporal points
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
  WHERE tIntersects(t1.temp, t2.temp) ?= true <> eIntersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry3D t2
  WHERE tIntersects(t1.temp, t2.temp) ?= true <> eIntersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE tTouches(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE tTouches(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE tTouches(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE tTouches(g, temp) ?= true <> eTouches(g, temp);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE tTouches(temp, g) ?= true <> eTouches(temp, g);

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry WHERE tDwithin(g, temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry WHERE tDwithin(temp, g, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2 WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry3D t2
  WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry t2
  WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tgeometry
  WHERE tDwithin(g, temp, 10) ?= true <> edwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tgeometry, tbl_geometry
  WHERE tDwithin(temp, g, 10) ?= true <> edwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);
  
-- Mixed 2D/3D
SELECT COUNT(*) FROM tbl_tgeometry t1, tbl_tgeometry3D t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);
SELECT COUNT(*) FROM tbl_tgeometry3D t1, tbl_tgeometry t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);

-------------------------------------------------------------------------------
-- END;
-- $$ LANGUAGE plpgsql;

-- SELECT pg_backend_pid()

-- SELECT testTopologicalOps()
-------------------------------------------------------------------------------
