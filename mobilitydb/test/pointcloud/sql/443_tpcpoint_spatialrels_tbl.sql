-- Smoke tests for spatial relationships on tpcpoint.

-------------------------------------------------------------------------------
-- Self-relations: every row eIntersects itself; nothing eDisjoint itself.
-------------------------------------------------------------------------------

SELECT bool_and(eIntersects(temp, temp)) FROM tbl_tpcpoint;
SELECT bool_and(NOT eDisjoint(temp, temp)) FROM tbl_tpcpoint;

-- aIntersects must imply eIntersects.
SELECT bool_and((NOT aIntersects(temp, temp)) OR eIntersects(temp, temp))
FROM tbl_tpcpoint;

-------------------------------------------------------------------------------
-- eDwithin with a huge radius is true for every row vs. the origin point.
-------------------------------------------------------------------------------

SELECT bool_and(eDwithin(temp, geometry 'SRID=0;POINT(0 0)', 1e9))
FROM tbl_tpcpoint;
SELECT bool_and(aDwithin(temp, geometry 'SRID=0;POINT(0 0)', 1e9))
FROM tbl_tpcpoint;

-- Reverse arg order produces the same result.
SELECT bool_and(eDwithin(geometry 'SRID=0;POINT(0 0)', temp, 1e9))
FROM tbl_tpcpoint;

-------------------------------------------------------------------------------
-- eDwithin with zero radius collapses to eIntersects against the geometry.
-------------------------------------------------------------------------------

SELECT bool_and(eDwithin(temp, geometry 'SRID=0;POINT(50 50)', 0)
              = eIntersects(temp, geometry 'SRID=0;POINT(50 50)'))
FROM tbl_tpcpoint;

-------------------------------------------------------------------------------
