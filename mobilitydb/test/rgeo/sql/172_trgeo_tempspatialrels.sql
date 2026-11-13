-------------------------------------------------------------------------------
-- Regression tests for the trgeometry temporal spatial-rel surface
-- (172_trgeo_tempspatialrels.in.sql).
--
-- Each function returns a tbool whose value at instant t is the static
-- spatial relation applied to the reference geometry placed at the pose the
-- temporal rigid geometry holds at t: cast the trgeometry to tgeometry and
-- delegate to the temporal spatial-rel kernel of tgeometry
-- (072_tgeo_tempspatialrels.in.sql). Each block checks that the direct
-- trgeometry overload agrees with the equivalent manual cast chain
-- (::tgeometry).
--
-- A temporal geometry keeps its value from one instant to the next, so the
-- cast states a trgeometry with step or discrete interpolation and raises an
-- error for one with linear interpolation, which moves and turns between its
-- instants. The relationships that go through the cast raise it too.
--
-------------------------------------------------------------------------------

SELECT tgeometry(trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]');
SELECT tIntersects(trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]', geometry 'Point(5 0)');
SELECT tContains(geometry 'Polygon((-5 -5,15 -5,15 5,-5 5,-5 -5))', trgeometry 'Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]');
SELECT asText(tgeometry(trgeometry 'Interp=Step;Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02]'));

-- Test for NULL inputs since the functions are not STRICT
SELECT tContains(NULL::geometry, trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2001-01-01');
SELECT tContains(geometry 'Polygon((-1 -1,-1 5,10 5,10 -1,-1 -1))', NULL::trgeometry);

-------------------------------------------------------------------------------
-- tContains, tCovers, tDisjoint, tIntersects, tTouches, tDwithin
-------------------------------------------------------------------------------

WITH t AS (
  SELECT trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]' AS seq1,
         trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(2 0),0)@2001-01-01, Pose(Point(2 2),0)@2001-01-03]' AS seq2,
         geometry 'Polygon((-1 -1,-1 5,10 5,10 -1,-1 -1))' AS big_region,
         geometry 'Point(50 50)' AS far_point
)
SELECT
  asText(tContains(big_region, seq1)) = asText(tContains(big_region, seq1::tgeometry)) AS tc_geo_rg,
  asText(tContains(seq1, big_region)) = asText(tContains(seq1::tgeometry, big_region)) AS tc_rg_geo,
  asText(tContains(seq1, seq2)) = asText(tContains(seq1::tgeometry, seq2::tgeometry)) AS tc_rg_rg,
  asText(tCovers(big_region, seq1)) = asText(tCovers(big_region, seq1::tgeometry)) AS tcv_geo_rg,
  asText(tCovers(seq1, big_region)) = asText(tCovers(seq1::tgeometry, big_region)) AS tcv_rg_geo,
  asText(tCovers(seq1, seq2)) = asText(tCovers(seq1::tgeometry, seq2::tgeometry)) AS tcv_rg_rg,
  asText(tDisjoint(far_point, seq1)) = asText(tDisjoint(far_point, seq1::tgeometry)) AS tdj_geo_rg,
  asText(tDisjoint(seq1, far_point)) = asText(tDisjoint(seq1::tgeometry, far_point)) AS tdj_rg_geo,
  asText(tDisjoint(seq1, seq2)) = asText(tDisjoint(seq1::tgeometry, seq2::tgeometry)) AS tdj_rg_rg,
  asText(tIntersects(big_region, seq1)) = asText(tIntersects(big_region, seq1::tgeometry)) AS ti_geo_rg,
  asText(tIntersects(seq1, big_region)) = asText(tIntersects(seq1::tgeometry, big_region)) AS ti_rg_geo,
  asText(tIntersects(seq1, seq2)) = asText(tIntersects(seq1::tgeometry, seq2::tgeometry)) AS ti_rg_rg,
  asText(tTouches(big_region, seq1)) = asText(tTouches(big_region, seq1::tgeometry)) AS ttc_geo_rg,
  asText(tTouches(seq1, big_region)) = asText(tTouches(seq1::tgeometry, big_region)) AS ttc_rg_geo,
  asText(tTouches(seq1, seq2)) = asText(tTouches(seq1::tgeometry, seq2::tgeometry)) AS ttc_rg_rg,
  asText(tDwithin(big_region, seq1, 1.0)) = asText(tDwithin(big_region, seq1::tgeometry, 1.0)) AS tdw_geo_rg,
  asText(tDwithin(seq1, big_region, 1.0)) = asText(tDwithin(seq1::tgeometry, big_region, 1.0)) AS tdw_rg_geo,
  asText(tDwithin(seq1, seq2, 1.0)) = asText(tDwithin(seq1::tgeometry, seq2::tgeometry, 1.0)) AS tdw_rg_rg
FROM t;

-------------------------------------------------------------------------------
-- Direct values
-------------------------------------------------------------------------------

SELECT tContains(geometry 'Polygon((-1 -1,-1 5,10 5,10 -1,-1 -1))',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]');
SELECT tCovers(geometry 'Polygon((-1 -1,-1 5,10 5,10 -1,-1 -1))',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]');
SELECT tDisjoint(geometry 'Point(50 50)',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]');
SELECT tIntersects(
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(2 0),0)@2001-01-01, Pose(Point(2 2),0)@2001-01-03]');
SELECT tDwithin(
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(2 0),0)@2001-01-01, Pose(Point(2 2),0)@2001-01-03]', 1.0);

-------------------------------------------------------------------------------

-- A body in linear motion: the times at which it passes over a polygon, to the microsecond
SELECT tIntersects(trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))');
SELECT tIntersects(geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))', trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]');
SELECT tIntersects(trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', geometry 'Polygon((5 5,6 5,6 6,5 6,5 5))');
SELECT tIntersects(trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(10 0),0)@2001-01-01,Pose(Point(0 0),0)@2001-01-02]');
SELECT tDisjoint(trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))');
SELECT tDisjoint(geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))', trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]');
SELECT tDisjoint(trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', geometry 'Polygon((5 5,6 5,6 6,5 6,5 5))');
SELECT tDisjoint(trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(10 0),0)@2001-01-01,Pose(Point(0 0),0)@2001-01-02]');
-- The answer is that of tDwithin at the distance 0
SELECT tIntersects(trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))') = tDwithin(trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))', 0.0);
-- A point that a body slides along until its vertex passes
SELECT tIntersects(trgeometry 'Polygon((-4 -4,4 -4,-2 2,-4 -4));[Pose(Point(-1 2),0)@2001-01-01,Pose(Point(2 -1),0)@2001-01-02,Pose(Point(8 -7),0)@2001-01-03]', geometry 'Point(1.5 -0.5)');
SELECT round(tDistance(trgeometry 'Polygon((-4 -4,4 -4,-2 2,-4 -4));[Pose(Point(-1 2),0)@2001-01-01,Pose(Point(2 -1),0)@2001-01-02,Pose(Point(8 -7),0)@2001-01-03]', geometry 'Point(1.5 -0.5)'), 6);
-- A distance that is an extremum of the walk keeps its value, the contact being elsewhere
SELECT tDwithin(trgeometry 'Polygon((-3 0,3 -4,-3 3,-3 0));[Pose(Point(0 -3),0)@2001-01-01,Pose(Point(4 1),0)@2001-01-02,Pose(Point(0 1),0)@2001-01-03]', geometry 'Point(0 0)', 0.0);
-- With step interpolation a body holds each pose
SELECT tIntersects(trgeometry 'Interp=Step;Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))');
SELECT tIntersects(trgeometry 'Interp=Step;Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))') = tIntersects(trgeometry 'Interp=Step;Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]'::tgeometry, geometry 'Polygon((5 0,6 0,6 1,5 1,5 0))');
-- A body in three dimensions bounded by a polyhedral surface
SELECT tIntersects(trgeometry 'PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,1 0 0,0 0 1,0 0 0)),((1 0 0,0 1 0,0 0 1,1 0 0)));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(10 0 0),1,0,0,0)@2001-01-02]', geometry 'Point(5 0.1 0.1)');
SELECT tIntersects(trgeometry 'PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,1 0 0,0 0 1,0 0 0)),((1 0 0,0 1 0,0 0 1,1 0 0)));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(10 0 0),1,0,0,0)@2001-01-02]', geometry 'Point(5 0.1 5)');
SELECT tDisjoint(trgeometry 'PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,1 0 0,0 0 1,0 0 0)),((1 0 0,0 1 0,0 0 1,1 0 0)));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(10 0 0),1,0,0,0)@2001-01-02]', geometry 'Point(5 0.1 0.1)');
SELECT tIntersects(trgeometry 'Interp=Step;PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,1 0 0,0 0 1,0 0 0)),((1 0 0,0 1 0,0 0 1,1 0 0)));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(10 0 0),1,0,0,0)@2001-01-02]', geometry 'Point(0.1 0.1 0.1)');
SELECT tDisjoint(trgeometry 'PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,1 0 0,0 0 1,0 0 0)),((1 0 0,0 1 0,0 0 1,1 0 0)));Pose(Point(0 0 0),1,0,0,0)@2001-01-01', geometry 'Point(0.1 0.1 0.1)');
SELECT tIntersects(trgeometry 'SRID=5676;PolyhedralSurface(((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1)),((2 0 1,4 0 1,4 2 1,2 2 1,2 0 1)));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(0 0 0),1,0,0,0)@2001-01-02]', geometry 'SRID=5676;Point(3 1 1)');
SELECT tDisjoint(trgeometry 'SRID=5676;PolyhedralSurface(((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1)),((2 0 1,4 0 1,4 2 1,2 2 1,2 0 1)));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(0 0 0),1,0,0,0)@2001-01-02]', geometry 'SRID=5676;Point(3 1 5)');
SELECT tIntersects(trgeometry 'Interp=Step;PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,1 0 0,0 0 1,0 0 0)),((1 0 0,0 1 0,0 0 1,1 0 0)));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(10 0 0),1,0,0,0)@2001-01-02]', geometry 'Point(0.2 0.2 0)');
-- A body in two dimensions reads a geometry in three dimensions in two
SELECT tIntersects(trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', geometry 'Point(5 1 7)');
SELECT tDisjoint(geometry 'Point(5 1 7)', trgeometry 'Interp=Step;Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]');
SELECT tDwithin(trgeometry 'Polygon((0 0,2 0,2 2,0 2,0 0));[Pose(Point(0 0),0)@2001-01-01,Pose(Point(10 0),0)@2001-01-02]', geometry 'Point(5 3 7)', 1.5);
-- Two bodies in linear motion that start to overlap between two instants: a
-- car of scene-0757 of nuScenes v1.0-mini and the footprint of the vehicle
-- recording it, both projected to the ground. eIntersects is true where
-- tIntersects is ever true, and aDisjoint false, from the contact on.
SELECT eIntersects(trgeometry 'Polygon((-2.35 -0.9,-2.35 0.9,2.35 0.9,2.35 -0.9,-2.35 -0.9));[Pose(Point(309.202946 668.650686),2.67835)@2018-08-30 12:25:17.149304-07, Pose(Point(309.173975 668.66787),2.678817)@2018-08-30 12:25:17.649169-07, Pose(Point(309.184429 668.663831),2.678596)@2018-08-30 12:25:18.149616-07, Pose(Point(309.185792 668.664064),2.678427)@2018-08-30 12:25:18.64948-07]', trgeometry 'Polygon((-2.3975 -1.045,-2.3975 1.045,2.3975 1.045,2.3975 -1.045,-2.3975 -1.045));[Pose(Point(314.395 666.879),2.704325)@2018-08-30 12:25:17.149304-07, Pose(Point(314.093 667.066),2.687517)@2018-08-30 12:25:17.649169-07, Pose(Point(313.791 667.253),2.670689)@2018-08-30 12:25:18.149616-07, Pose(Point(313.787 667.253),2.678445)@2018-08-30 12:25:18.64948-07]');
SELECT aDisjoint(trgeometry 'Polygon((-2.35 -0.9,-2.35 0.9,2.35 0.9,2.35 -0.9,-2.35 -0.9));[Pose(Point(309.202946 668.650686),2.67835)@2018-08-30 12:25:17.149304-07, Pose(Point(309.173975 668.66787),2.678817)@2018-08-30 12:25:17.649169-07, Pose(Point(309.184429 668.663831),2.678596)@2018-08-30 12:25:18.149616-07, Pose(Point(309.185792 668.664064),2.678427)@2018-08-30 12:25:18.64948-07]', trgeometry 'Polygon((-2.3975 -1.045,-2.3975 1.045,2.3975 1.045,2.3975 -1.045,-2.3975 -1.045));[Pose(Point(314.395 666.879),2.704325)@2018-08-30 12:25:17.149304-07, Pose(Point(314.093 667.066),2.687517)@2018-08-30 12:25:17.649169-07, Pose(Point(313.791 667.253),2.670689)@2018-08-30 12:25:18.149616-07, Pose(Point(313.787 667.253),2.678445)@2018-08-30 12:25:18.64948-07]');
SELECT tIntersects(trgeometry 'Polygon((-2.35 -0.9,-2.35 0.9,2.35 0.9,2.35 -0.9,-2.35 -0.9));[Pose(Point(309.202946 668.650686),2.67835)@2018-08-30 12:25:17.149304-07, Pose(Point(309.173975 668.66787),2.678817)@2018-08-30 12:25:17.649169-07, Pose(Point(309.184429 668.663831),2.678596)@2018-08-30 12:25:18.149616-07, Pose(Point(309.185792 668.664064),2.678427)@2018-08-30 12:25:18.64948-07]', trgeometry 'Polygon((-2.3975 -1.045,-2.3975 1.045,2.3975 1.045,2.3975 -1.045,-2.3975 -1.045));[Pose(Point(314.395 666.879),2.704325)@2018-08-30 12:25:17.149304-07, Pose(Point(314.093 667.066),2.687517)@2018-08-30 12:25:17.649169-07, Pose(Point(313.791 667.253),2.670689)@2018-08-30 12:25:18.149616-07, Pose(Point(313.787 667.253),2.678445)@2018-08-30 12:25:18.64948-07]');
-- A closed polyhedral surface bounds a solid: a point inside the body meets it
SELECT tDisjoint(geometry 'Point(0.1 0.1 0.1)', trgeometry 'PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,1 0 0,0 0 1,0 0 0)),((1 0 0,0 1 0,0 0 1,1 0 0)));Pose(Point(0 0 0),1,0,0,0)@2001-01-01');
-- A body that is not convex: inside the L, and in the plane of its base outside the L
SELECT tIntersects(trgeometry 'Interp=Step;PolyhedralSurface Z(((0 0 0,0 3 0,1 3 0,1 1 0,4 1 0,4 0 0,0 0 0)),((0 0 2,4 0 2,4 1 2,1 1 2,1 3 2,0 3 2,0 0 2)),((0 0 0,4 0 0,4 0 2,0 0 2,0 0 0)),((4 0 0,4 1 0,4 1 2,4 0 2,4 0 0)),((4 1 0,1 1 0,1 1 2,4 1 2,4 1 0)),((1 1 0,1 3 0,1 3 2,1 1 2,1 1 0)),((1 3 0,0 3 0,0 3 2,1 3 2,1 3 0)),((0 3 0,0 0 0,0 0 2,0 3 2,0 3 0)));[Pose(Point(0 0 0),1,0,0,0)@2001-01-01,Pose(Point(0 0 0),0,0,0,1)@2001-01-02]', geometry 'Point(3 0.5 1)');
SELECT tIntersects(trgeometry 'PolyhedralSurface Z(((0 0 0,0 3 0,1 3 0,1 1 0,4 1 0,4 0 0,0 0 0)),((0 0 2,4 0 2,4 1 2,1 1 2,1 3 2,0 3 2,0 0 2)),((0 0 0,4 0 0,4 0 2,0 0 2,0 0 0)),((4 0 0,4 1 0,4 1 2,4 0 2,4 0 0)),((4 1 0,1 1 0,1 1 2,4 1 2,4 1 0)),((1 1 0,1 3 0,1 3 2,1 1 2,1 1 0)),((1 3 0,0 3 0,0 3 2,1 3 2,1 3 0)),((0 3 0,0 0 0,0 0 2,0 3 2,0 3 0)));Pose(Point(0 0 0),1,0,0,0)@2001-01-01', geometry 'Point(2 2 1)');
-- A point below the body on the line of one of its edges
SELECT tIntersects(trgeometry 'PolyhedralSurface Z(((0 0 0,0 3 0,2 3 0,2 0 0,0 0 0)),((0 0 1,2 0 1,2 3 1,0 3 1,0 0 1)),((0 0 0,2 0 0,2 0 1,0 0 1,0 0 0)),((0 3 0,0 3 1,2 3 1,2 3 0,0 3 0)),((0 0 0,0 0 1,0 3 1,0 3 0,0 0 0)),((2 0 0,2 3 0,2 3 1,2 0 1,2 0 0)));Pose(Point(0 0 0),1,0,0,0)@2001-01-01', geometry 'Point(0 0 -2)');
-- The solid is kept through the text and the binary of the value
SELECT tIntersects(trgeometry(asText(trgeometry 'PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,1 0 0,0 0 1,0 0 0)),((1 0 0,0 1 0,0 0 1,1 0 0)));Pose(Point(0 0 0),1,0,0,0)@2001-01-01')), geometry 'Point(0.1 0.1 0.1)');
SELECT tIntersects(trgeometryFromBinary(asBinary(trgeometry 'PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,1 0 0,0 0 1,0 0 0)),((1 0 0,0 1 0,0 0 1,1 0 0)));Pose(Point(0 0 0),1,0,0,0)@2001-01-01')), geometry 'Point(0.1 0.1 0.1)');
-- An open polyhedral surface is its faces
SELECT tIntersects(trgeometry 'PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)));Pose(Point(0 0 0),1,0,0,0)@2001-01-01', geometry 'Point(0.1 0.1 0.1)');
-- The reference geometry a value returns is the one it was given
SELECT geometry(trgeometry 'PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,1 0 0,0 0 1,0 0 0)),((1 0 0,0 1 0,0 0 1,1 0 0)));Pose(Point(0 0 0),1,0,0,0)@2001-01-01') = geometry 'PolyhedralSurface Z(((0 0 0,0 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 0,0 0 0)),((0 0 0,1 0 0,0 0 1,0 0 0)),((1 0 0,0 1 0,0 0 1,1 0 0)))';
