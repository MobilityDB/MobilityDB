-------------------------------------------------------------------------------
-- tContains
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tContains(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tContains(temp, g) IS NOT NULL;

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE tContains(cb, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_cbuffer WHERE tContains(temp, cb) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE tContains(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tContains(g, temp) ?= true <> eContains(g, temp);

SELECT COUNT(*) FROM tbl_cbuffer, tbl_tcbuffer WHERE tContains(cb, temp) ?= true <> eContains(cb, temp);
-- SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE tContains(t1.temp, t2.temp) ?= true <> eContains(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tCovers
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tCovers(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tCovers(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE tCovers(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tCovers(g, temp) ?= true <> eCovers(g, temp);

-------------------------------------------------------------------------------
-- tDisjoint
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tDisjoint(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tDisjoint(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE tDisjoint(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tDisjoint(g, temp) ?= true <> eDisjoint(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tDisjoint(temp, g) ?= true <> eDisjoint(temp, g);
  
-- Temporal points
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2
  WHERE tDisjoint(t1.temp, t2.temp) ?= true <> eDisjoint(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tIntersects
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tIntersects(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tIntersects(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tIntersects(temp, g) ?= true <> eDisjoint(temp, g);

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tIntersects(g, temp) ?= true <> eIntersects(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tIntersects(temp, g) ?= true <> eIntersects(temp, g);

-- Temporal points
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2
  WHERE tIntersects(t1.temp, t2.temp) ?= true <> eIntersects(t1.temp, t2.temp);

-------------------------------------------------------------------------------
-- tTouches
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tTouches(g, temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tTouches(temp, g) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE tTouches(t1.temp, t2.temp) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tTouches(g, temp) ?= true <> eTouches(g, temp);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tTouches(temp, g) ?= true <> eTouches(temp, g);

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer WHERE tDwithin(g, temp, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry WHERE tDwithin(temp, g, 10) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE tDwithin(t1.temp, t2.temp, 10) IS NOT NULL;

-------------------------------------------------------------------------------
-- Robustness test

SELECT COUNT(*) FROM tbl_geometry, tbl_tcbuffer
  WHERE tDwithin(g, temp, 10) ?= true <> edwithin(g, temp, 10);
SELECT COUNT(*) FROM tbl_tcbuffer, tbl_geometry
  WHERE tDwithin(temp, g, 10) ?= true <> edwithin(temp, g, 10);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2
  WHERE tDwithin(t1.temp, t2.temp, 10) ?= true <> edwithin(t1.temp, t2.temp, 10);
  
-------------------------------------------------------------------------------

