/*****************************************************************************/

select asewkt(tgeompoint '[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02]')
select asewkt(tgeompoint 'srid=4326;[Point(0 0)@2000-01-01, Point(0 0)@2000-01-02]')

/*****************************************************************************/

-- trajectory

select st_astext(trajectory(
tgeompoints(ARRAY[
tgeompoint '[point(0 0)@2000-01-01, point(0 1)@2000-01-02)'
])));
-- "LINESTRING(0 0,0 1)"

select st_astext(trajectory(
tgeompoints(ARRAY[tgeompoint
'[point(0 0)@2000-01-01, point(0 1)@2000-01-02)',
'[point(0 1)@2000-01-02, point(0 3)@2000-01-03)',
'[point(0 3)@2000-01-03, point(0 6)@2000-01-04)'
])));
-- "LINESTRING(0 0,0 1,0 3,0 6)"

select st_astext(trajectory(
tgeompoints(ARRAY[
tgeompoint '[point(0 0)@2000-01-01, point(0 1)@2000-01-02)',
tgeompoint '[point(0 1)@2000-01-02, point(0 0)@2000-01-03)'
])));
-- "LINESTRING(0 0,0 1)"

select st_astext(trajectory(
tgeompoints(ARRAY[
tgeompoint '[point(0 0)@2000-01-01, point(0 1)@2000-01-02)',
tgeompoint '[point(0 3)@2000-01-03, point(0 6)@2000-01-04)'
])));
-- "MULTILINESTRING((0 0,0 1),(0 3,0 6))"

-- ST_Union will also split linestrings at node intersections !!!
select st_astext(trajectory(
tgeompoints(ARRAY[
tgeompoint '[point(0 0)@2000-01-01, point(2 0)@2000-01-02)',
tgeompoint '[point(2 0)@2000-01-02, point(1 1)@2000-01-03)',
tgeompoint '[point(1 1)@2000-01-03, point(1 -1)@2000-01-04)'
])));
-- "MULTILINESTRING((0 0,1 0),(1 0,1 -1),(1 0,2 0,1 1,1 0))"

select st_astext(trajectory(
tgeompoints(ARRAY[
tgeompoint '[point(0 0)@2000-01-01, point(2 0)@2000-01-02)',
tgeompoint '[point(2 0)@2000-01-02, point(1 1)@2000-01-03)',
tgeompoint '[point(1 1)@2000-01-03, point(1 -1)@2000-01-04)',
tgeompoint '[point(2 2)@2000-01-05, point(3 3)@2000-01-06)'
])));
-- "MULTILINESTRING((0 0,1 0),(1 0,2 0,1 1,1 0),(1 0,1 -1),(2 2,3 3))"

select st_astext(trajectory(
tgeompoints(ARRAY[
tgeompoint '[point(1 1)@2000-01-02, point(1 1)@2000-01-02]'
])));
-- "POINT(1 1)"

select st_astext(trajectory(
tgeompoints(ARRAY[
tgeompoint '[point(0 0)@2000-01-01, point(0 0)@2000-01-01]',
tgeompoint '[point(1 1)@2000-01-02, point(1 1)@2000-01-02]',
tgeompoint '[point(1 1)@2000-01-03, point(1 1)@2000-01-03]'
])));
-- "MULTIPOINT(0 0,1 1)"

select st_astext(trajectory(
tgeompoints(ARRAY[
tgeompoint '[point(0 0)@2000-01-01, point(0 1)@2000-01-02)',
tgeompoint '[point(1 1)@2000-01-02, point(1 1)@2000-01-02]',
tgeompoint '(point(0 2)@2000-01-02, point(0 3)@2000-01-03)',
tgeompoint '[point(2 2)@2000-01-03, point(2 2)@2000-01-03]',
tgeompoint '(point(0 6)@2000-01-03, point(0 7)@2000-01-04)',
tgeompoint '[point(2 2)@2000-01-04, point(2 2)@2000-01-04]'
])));
-- "GEOMETRYCOLLECTION(MULTIPOINT(1 1,2 2),MULTILINESTRING((0 0,0 1),(0 2,0 3),(0 6,0 7)))"

select st_astext(trajectory(
tgeompoints(ARRAY[
tgeompoint '[point(0 0)@2000-01-01, point(0 1)@2000-01-02)',
tgeompoint '[point(1 1)@2000-01-02, point(1 1)@2000-01-02]',
tgeompoint '(point(0 3)@2000-01-03, point(0 6)@2000-01-04)'
])));
-- "GEOMETRYCOLLECTION(POINT(1 1),LINESTRING(0 0,0 1),LINESTRING(0 3,0 6))"

select st_astext(trajectory(
tgeompoints(ARRAY[
tgeompoint '[point(0 0)@2000-01-01, point(0 1)@2000-01-02)',
tgeompoint '[point(0 0)@2000-01-02, point(0 0)@2000-01-02]',
tgeompoint '(point(0 3)@2000-01-03, point(0 6)@2000-01-04)'
])));
-- "GEOMETRYCOLLECTION(MULTILINESTRING((0 0,0 1),(0 3,0 6)))"

/*****************************************************************************/
-- synctrajectory

select astext(synctrajectory(
tgeompoints(ARRAY[
tgeompoint 'point(0 0)@point(0 1)@[2000-01-01,2000-01-02)'
])));
-- "LINESTRING(0 0,0 1)"

select pg_afterend_pid()

select astext(synctrajectory(
tgeompoints(ARRAY[
tgeompoint 'point(0 0)@point(0 1)@[2000-01-01,2000-01-02)',
tgeompoint 'point(0 1)@point(0 3)@[2000-01-02,2000-01-03)',
tgeompoint 'point(0 3)@point(0 6)@[2000-01-03,2000-01-04)'
])));
-- "LINESTRING(0 0,0 1,0 3,0 6)"

select astext(synctrajectory(
tgeompoints(ARRAY[
tgeompoint 'point(0 0)@point(0 1)@[2000-01-01,2000-01-02)',
tgeompoint 'point(0 1)@point(0 0)@[2000-01-02,2000-01-03)'
])));
-- "{"LINESTRING(0 0,0 1)","LINESTRING(0 1,0 0)"}"

select astext(synctrajectory(
tgeompoints(ARRAY[
tgeompoint 'point(0 0)@point(0 1)@[2000-01-01,2000-01-02)',
tgeompoint 'point(0 3)@point(0 6)@[2000-01-03,2000-01-04)'
])));
-- "{"LINESTRING(0 0,0 1)","LINESTRING(0 3,0 6)"}"

select astext(synctrajectory(
tgeompoints(ARRAY[
tgeompoint 'point(0 0)@point(2 0)@[2000-01-01,2000-01-02)',
tgeompoint 'point(2 0)@point(1 1)@[2000-01-02,2000-01-03)',
tgeompoint 'point(1 1)@point(1 -1)@[2000-01-03,2000-01-04)'
])));
-- "{"LINESTRING(0 0,2 0,1 1)","LINESTRING(1 1,1 -1)"}"

select astext(synctrajectory(
tgeompoints(ARRAY[
tgeompoint 'point(0 0)@point(2 0)@[2000-01-01,2000-01-02)',
tgeompoint 'point(2 0)@point(1 1)@[2000-01-02,2000-01-03)',
tgeompoint 'point(1 1)@point(1 -1)@[2000-01-03,2000-01-04)',
tgeompoint 'point(2 2)@point(3 3)@[2000-01-05,2000-01-06)'
])));
-- "{"LINESTRING(0 0,2 0,1 1)","LINESTRING(1 1,1 -1)","LINESTRING(2 2,3 3)"}"

select astext(synctrajectory(
tgeompoints(ARRAY[
tgeompoint 'point(1 1)@[2000-01-02,2000-01-02]'
])));
-- "POINT(1 1)"

select astext(synctrajectory(
tgeompoints(ARRAY[
tgeompoint 'point(0 0)@[2000-01-01,2000-01-01]',
tgeompoint 'point(1 1)@[2000-01-02,2000-01-02]',
tgeompoint 'point(1 1)@[2000-01-03,2000-01-03]'
])));
-- "{"POINT(0 0)","POINT(1 1)","POINT(1 1)"}"

select astext(synctrajectory(
tgeompoints(ARRAY[
tgeompoint 'point(0 0)@point(0 1)@[2000-01-01,2000-01-02)',
tgeompoint 'point(1 1)@[2000-01-02,2000-01-02]',
tgeompoint 'point(0 2)@point(0 3)@(2000-01-02,2000-01-03)',
tgeompoint 'point(2 2)@[2000-01-03,2000-01-03]',
tgeompoint 'point(0 6)@point(0 7)@(2000-01-03,2000-01-04)',
tgeompoint 'point(2 2)@[2000-01-04,2000-01-04]'
])));
-- "{"LINESTRING(0 0,0 1)","POINT(1 1)","LINESTRING(0 2,0 3)","POINT(2 2)","LINESTRING(0 6,0 7)","POINT(2 2)"}"

select astext(synctrajectory(
tgeompoints(ARRAY[
tgeompoint 'point(0 0)@point(0 1)@[2000-01-01, 2000-01-02)',
tgeompoint 'point(1 1)@[2000-01-02, 2000-01-02]',
tgeompoint 'point(0 3)@point(0 6)@(2000-01-03, 2000-01-04)'
])));
-- "{"LINESTRING(0 0,0 1)","POINT(1 1)","LINESTRING(0 3,0 6)"}"

select astext(synctrajectory(
tgeompoints(ARRAY[
tgeompoint 'point(0 0)@point(0 1)@[2000-01-01, 2000-01-02)',
tgeompoint 'point(0 0)@[2000-01-02, 2000-01-02]',
tgeompoint 'point(0 3)@point(0 6)@(2000-01-03, 2000-01-04)'
])));
-- "{"LINESTRING(0 0,0 1)","POINT(0 0)","LINESTRING(0 3,0 6)"}"

/*****************************************************************************/

select gbox(
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'));
-- "GBOX(0 0 0,0 0 0)"

select st_srid(gbox(
tgeompointinst(st_setsrid(st_point(0,0),4326), '2000-01-01 00:00:00')));
-- 4326

select gbox(
tgeompointper(st_point(0,1), st_point(4,1), '2000-01-01 08:00', '2000-01-01 08:20'));
-- "GBOX(0 1 28800000000,4 1 30000000000)"

select st_srid(gbox(
tgeompointper(st_setsrid(st_point(0,1),4326), st_setsrid(st_point(4,1),4326), 
'2000-01-01 08:00', '2000-01-01 08:20')));
-- 4326

select gbox(
tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(0,1), '2000-01-01 00:00:00', '2000-01-01 00:10:00'),
tgeompointper(st_point(0,1), st_point(0,3), '2000-01-01 00:10:00', '2000-01-01 00:20:00'),
tgeompointper(st_point(0,3), st_point(0,6), '2000-01-01 00:20:00', '2000-01-01 00:30:00')
]));
-- "GBOX(0 0 0,0 6 1800000000)"

select gbox(
tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(0,1), '2000-01-01 00:05:00'),
tgeompointinst(st_point(0,2), '2000-01-01 00:10:00'),
tgeompointinst(st_point(0,3), '2000-01-01 00:15:00'),
tgeompointinst(st_point(0,3), '2000-01-01 00:20:00'),
tgeompointinst(st_point(0,6), '2000-01-01 00:25:00')
]));

-- "GBOX(0 0 0,0 6 1500000000)"

/*****************************************************************************/

select setsrid(tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),5676);

select setsrid(tgeompoint 'Point(0 0)@Point(3 3)@[2012-01-01, 2012-01-04)',5676);

select setsrid(tgeompoints(ARRAY[
tgeompoint 'Point(0 0)@Point(3 3)@[2012-01-01, 2012-01-02)',
tgeompoint 'Point(3 3)@Point(2 2)@[2012-01-02, 2012-01-03)',
tgeompoint 'Point(2 2)@Point(1 1)@[2012-01-03, 2012-01-04)']),
5676);

select setsrid(tgeompointi(ARRAY[
tgeompointinst 'Point(0 0)@2012-01-01',
tgeompointinst 'Point(3 3)@2012-01-02',
tgeompointinst 'Point(2 2)@2012-01-03']),
5676);

select setsrid(tgeogpointinst(st_point(0,0), '2000-01-01 00:00:00'),4326);

select setsrid(tgeogpoint 'Point(0 0)@Point(3 3)@[2012-01-01, 2012-01-04)',4326);

select setsrid(tgeogpointp(ARRAY[
tgeogpoint 'Point(0 0)@Point(3 3)@[2012-01-01, 2012-01-02)',
tgeogpoint 'Point(3 3)@Point(2 2)@[2012-01-02, 2012-01-03)',
tgeogpoint 'Point(2 2)@Point(1 1)@[2012-01-03, 2012-01-04)']),
4326);

select setsrid(tgeogpointi(ARRAY[
tgeogpointinst 'Point(0 0)@2012-01-01',
tgeogpointinst 'Point(3 3)@2012-01-02',
tgeogpointinst 'Point(2 2)@2012-01-03']),
4326);


/*****************************************************************************/

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') @>
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') @>
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') @>
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') @>
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') @>
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') @>
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') @>
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') @>
tgeompointper(st_point(1,1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') @>
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') @>
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') @>
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) @>
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) @>
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]) @>
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) @>
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1,1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) @>
tgeompoints(ARRAY[
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1.1,1.1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) @>
tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) @> 
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) @> 
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) @> 
tgeompoints(ARRAY[
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1.1,1.1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00')]) @>
tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00')]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) @>
tgeompointi(ARRAY[
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) @> 
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') <@
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') <@
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') <@
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') <@
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') <@
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') <@
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') <@
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') <@
tgeompointper(st_point(1,1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') <@
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') <@
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') <@
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) <@
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) <@
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]) <@
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) <@
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1,1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) <@
tgeompoints(ARRAY[
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1.1,1.1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) <@
tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) <@ 
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) <@ 
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) <@ 
tgeompoints(ARRAY[
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1.1,1.1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00')]) <@
tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00')]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) <@
tgeompointi(ARRAY[
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) <@ 
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') &&
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') &&
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') &&
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') &&
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') &&
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') &&
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') &&
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') &&
tgeompointper(st_point(1,1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') &&
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') &&
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') &&
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) &&
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) &&
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]) &&
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) &&
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1,1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) &&
tgeompoints(ARRAY[
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1.1,1.1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) &&
tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) && 
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) && 
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) && 
tgeompoints(ARRAY[
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1.1,1.1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00')]) &&
tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00')]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) &&
tgeompointi(ARRAY[
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) && 
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') ~=
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') ~=
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') ~=
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') ~=
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompointinst(st_point(0,0), '2000-01-01 00:00:00') ~=
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') ~=
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') ~=
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') ~=
tgeompointper(st_point(1,1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') ~=
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') ~=
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10') ~=
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) ~=
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) ~=
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]) ~=
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10')]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) ~=
tgeompoints(ARRAY[
tgeompointper(st_point(1,1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1,1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) ~=
tgeompoints(ARRAY[
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1.1,1.1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompoints(ARRAY[
tgeompointper(st_point(0,0), st_point(1,1), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]) ~=
tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]);

/*****************************************************************************/

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) ~= 
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00');

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) ~= 
tgeompointper(st_point(1,1), st_point(0,0), '2000-01-01 00:00:00', '2000-01-01 00:00:10');

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) ~= 
tgeompoints(ARRAY[
tgeompointper(st_point(1.1,1.1), st_point(2,2), '2000-01-01 00:00:00', '2000-01-01 00:00:10'),
tgeompointper(st_point(2,2), st_point(1.1,1.1), '2000-01-01 00:00:10', '2000-01-01 00:00:20')
]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00')]) ~=
tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00')]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) ~=
tgeompointi(ARRAY[
tgeompointinst(st_point(1,1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

select tgeompointi(ARRAY[
tgeompointinst(st_point(0,0), '2000-01-01 00:00:00'),
tgeompointinst(st_point(1,1), '2000-01-01 00:00:10')
]) ~= 
tgeompointi(ARRAY[
tgeompointinst(st_point(1.1,1.1), '2000-01-01 00:00:00'),
tgeompointinst(st_point(2,2), '2000-01-01 00:00:10')
]);

/*****************************************************************************
 * Cast to/from PostGIS trajectories
 *****************************************************************************/
-- TPointInst

SELECT st_astext(tgeompointinst(ST_MakePoint(0,0), timestamp '2012-01-01 08:00:00')::geometry);
SELECT st_astext(tgeompointinst(ST_MakePoint(0,0,0), timestamp '2012-01-01 08:00:00')::geometry);

SELECT st_astext(tgeogpointinst(ST_MakePoint(0,0), timestamp '2012-01-01 08:00:00')::geography);
SELECT st_astext(tgeogpointinst(ST_MakePoint(0,0,0), timestamp '2012-01-01 08:00:00')::geography);

SELECT astext((geometry 'POINT M (0 0 1325404800)')::tgeompointinst);
SELECT astext((geometry 'POINT ZM (0 0 0 1325404800)')::tgeompointinst);

SELECT astext((geography 'POINT M (0 0 1325404800)')::tgeogpointinst);
SELECT astext((geography 'POINT ZM (0 0 0 1325404800)')::tgeogpointinst);

--------------------------------------------------------
-- TPointI

SELECT st_astext((tgeompointi '{Point(0 0)@2012-01-01 08:00:00}')::geometry);
SELECT st_astext((tgeompointi '{Point(0 0 0)@2012-01-01 08:00:00}')::geometry);

SELECT st_astext((tgeogpointi '{Point(0 0)@2012-01-01 08:00:00}')::geography );
SELECT st_astext((tgeogpointi '{Point(0 0 0)@2012-01-01 08:00:00}')::geography );

SELECT st_astext((tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(1 1)@2012-01-01 08:10:00}')::geometry);
SELECT st_astext((tgeompointi '{Point(0 0 0)@2012-01-01 08:00:00, Point(1 1 1)@2012-01-01 08:10:00}')::geometry);

SELECT st_astext((tgeogpointi '{Point(0 0)@2012-01-01 08:00:00, Point(1 1)@2012-01-01 08:10:00}')::geography );
SELECT st_astext((tgeogpointi '{Point(0 0 0)@2012-01-01 08:00:00, Point(1 1 1)@2012-01-01 08:10:00}')::geography );

SELECT astext((geometry 'POINT M (0 0 1325404800)')::tgeompointi);
SELECT astext((geometry 'POINT ZM (0 0 0 1325404800)')::tgeompointi);

SELECT astext((geography 'POINT M (0 0 1325404800)')::tgeogpointi);
SELECT astext((geography 'POINT ZM (0 0 0 1325404800)')::tgeogpointi);

SELECT astext((geometry 'MULTIPOINT M (0 0 1325404800,1 1 1325405400)')::tgeompointi);
SELECT astext((geometry 'MULTIPOINT ZM (0 0 0 1325404800,1 1 1 1325405400)')::tgeompointi);

SELECT astext((geography 'MULTIPOINT M (0 0 1325404800,1 1 1325405400)')::tgeogpointi);
SELECT astext((geography 'MULTIPOINT ZM (0 0 0 1325404800,1 1 1 1325405400)')::tgeogpointi);

--------------------------------------------------------
-- TPointSeq

SELECT st_astext((tgeompointseq '[Point(0 0)@2012-01-01 08:00:00]')::geometry);
SELECT st_astext((tgeompointseq '[Point(0 0 0)@2012-01-01 08:00:00]')::geometry);

SELECT st_astext((tgeogpointseq '[Point(0 0)@2012-01-01 08:00:00]')::geography );
SELECT st_astext((tgeogpointseq '[Point(0 0 0)@2012-01-01 08:00:00]')::geography );

SELECT st_astext((tgeompointseq '[Point(0 0)@2012-01-01 08:00:00, Point(1 1)@2012-01-01 08:10:00]')::geometry);
SELECT st_astext((tgeompointseq '[Point(0 0 0)@2012-01-01 08:00:00, Point(1 1 1)@2012-01-01 08:10:00]')::geometry);

SELECT st_astext((tgeogpointseq '[Point(0 0)@2012-01-01 08:00:00, Point(1 1)@2012-01-01 08:10:00]')::geography );
SELECT st_astext((tgeogpointseq '[Point(0 0 0)@2012-01-01 08:00:00, Point(1 1 1)@2012-01-01 08:10:00]')::geography );

SELECT astext((geometry 'POINT M (0 0 1325404800)')::tgeompointseq);
SELECT astext((geometry 'POINT ZM (0 0 0 1325404800)')::tgeompointseq);

SELECT astext((geography 'POINT M (0 0 1325404800)')::tgeogpointseq);
SELECT astext((geography 'POINT ZM (0 0 0 1325404800)')::tgeogpointseq);

SELECT astext((geometry 'LINESTRING M (0 0 1325404800,1 1 1325405400)')::tgeompointseq);
SELECT astext((geometry 'LINESTRING ZM (0 0 0 1325404800,1 1 1 1325405400)')::tgeompointseq);

SELECT astext((geography 'LINESTRING M (0 0 1325404800,1 1 1325405400)')::tgeogpointseq);
SELECT astext((geography 'LINESTRING ZM (0 0 0 1325404800,1 1 1 1325405400)')::tgeogpointseq);

--------------------------------------------------------
-- TPointS

SELECT st_astext((tgeompoints '{[Point(0 0)@2012-01-01 08:00:00]}')::geometry);
SELECT st_astext((tgeompoints '{[Point(0 0 0)@2012-01-01 08:00:00]}')::geometry);

SELECT st_astext((tgeogpoints '{[Point(0 0)@2012-01-01 08:00:00]}')::geography );
SELECT st_astext((tgeogpoints '{[Point(0 0 0)@2012-01-01 08:00:00]}')::geography );

SELECT st_astext((tgeompoints '{[Point(0 0)@2012-01-01 08:00:00, Point(1 1)@2012-01-01 08:10:00]}')::geometry);
SELECT st_astext((tgeompoints '{[Point(0 0 0)@2012-01-01 08:00:00, Point(1 1 1)@2012-01-01 08:10:00]}')::geometry);

SELECT st_astext((tgeogpoints '{[Point(0 0)@2012-01-01 08:00:00, Point(1 1)@2012-01-01 08:10:00]}')::geography);
SELECT st_astext((tgeogpoints '{[Point(0 0 0)@2012-01-01 08:00:00, Point(1 1 1)@2012-01-01 08:10:00]}')::geography);

SELECT st_astext((tgeompoints '{[Point(0 0)@2012-01-01 08:00:00, Point(1 1)@2012-01-01 08:10:00],
	[Point(1 1)@2012-01-01 08:20:00, Point(0 0)@2012-01-01 08:30:00]}')::geometry);
SELECT st_astext((tgeompoints '{[Point(0 0 0)@2012-01-01 08:00:00, Point(1 1 1)@2012-01-01 08:10:00],
	[Point(1 1 1)@2012-01-01 08:20:00, Point(0 0 0)@2012-01-01 08:30:00]}')::geometry);

SELECT st_astext((tgeogpoints '{[Point(0 0)@2012-01-01 08:00:00, Point(1 1)@2012-01-01 08:10:00],
	[Point(1 1)@2012-01-01 08:20:00, Point(0 0)@2012-01-01 08:30:00]}')::geography );
SELECT st_astext((tgeogpoints '{[Point(0 0 0)@2012-01-01 08:00:00, Point(1 1 1)@2012-01-01 08:10:00],
	[Point(1 1 1)@2012-01-01 08:20:00, Point(0 0 0)@2012-01-01 08:30:00]}')::geography );

SELECT st_astext((tgeompoints '{[Point(0 0)@2012-01-01 08:00:00, Point(1 1)@2012-01-01 08:10:00],
	[Point(1 1)@2012-01-01 08:15:00],
	[Point(1 1)@2012-01-01 08:20:00, Point(0 0)@2012-01-01 08:30:00]}')::geometry );
SELECT st_astext((tgeompoints '{[Point(0 0 0)@2012-01-01 08:00:00, Point(1 1 1)@2012-01-01 08:10:00],
	[Point(1 1 1)@2012-01-01 08:15:00],
	[Point(1 1 1)@2012-01-01 08:20:00, Point(0 0 0)@2012-01-01 08:30:00]}')::geometry );

SELECT st_astext((tgeogpoints '{[Point(0 0)@2012-01-01 08:00:00, Point(1 1)@2012-01-01 08:10:00],
	[Point(1 1)@2012-01-01 08:15:00],
	[Point(1 1)@2012-01-01 08:20:00, Point(0 0)@2012-01-01 08:30:00]}')::geography );
SELECT st_astext((tgeogpoints '{[Point(0 0 0)@2012-01-01 08:00:00, Point(1 1 1)@2012-01-01 08:10:00],
	[Point(1 1 1)@2012-01-01 08:15:00],
	[Point(1 1 1)@2012-01-01 08:20:00, Point(0 0 0)@2012-01-01 08:30:00]}')::geography );

SELECT astext((geometry 'POINT M (0 0 1325404800)')::tgeompoints);
SELECT astext((geometry 'POINT ZM (0 0 0 1325404800)')::tgeompoints);

SELECT astext((geography 'POINT M (0 0 1325404800)')::tgeogpoints);
SELECT astext((geography 'POINT ZM (0 0 0 1325404800)')::tgeogpoints);

SELECT astext((geometry 'MULTIPOINT M (0 0 1325404800,1 1 1325405400)')::tgeompoints);
SELECT astext((geometry 'MULTIPOINT ZM (0 0 0 1325404800,1 1 1 1325405400)')::tgeompoints);

SELECT astext((geography 'MULTIPOINT M (0 0 1325404800,1 1 1325405400)')::tgeogpoints);
SELECT astext((geography 'MULTIPOINT ZM (0 0 0 1325404800,1 1 1 1325405400)')::tgeogpoints);

SELECT astext((geometry 'LINESTRING M (0 0 1325404800,1 1 1325405400)')::tgeompoints);
SELECT astext((geometry 'LINESTRING ZM (0 0 0 1325404800,1 1 1 1325405400)')::tgeompoints);

SELECT astext((geography 'LINESTRING M (0 0 1325404800,1 1 1325405400)')::tgeogpoints);
SELECT astext((geography 'LINESTRING ZM (0 0 0 1325404800,1 1 1 1325405400)')::tgeogpoints);

SELECT astext((geometry 'MULTILINESTRING M ((0 0 1325404800,1 1 1325405400),(1 1 1325406000,0 0 1325406600))')::tgeompoints);
SELECT astext((geometry 'MULTILINESTRING M ((0 0 1325404800,1 1 1325405400),(1 1 1325406000,0 0 1325406600))')::tgeompoints);

SELECT astext((geography 'MULTILINESTRING M ((0 0 1325404800,1 1 1325405400),(1 1 1325406000,0 0 1325406600))')::tgeogpoints);
SELECT astext((geography 'MULTILINESTRING M ((0 0 1325404800,1 1 1325405400),(1 1 1325406000,0 0 1325406600))')::tgeogpoints);

SELECT astext((geometry 'GEOMETRYCOLLECTION M (LINESTRING M (0 0 1325404800,1 1 1325405400),
	POINT M (1 1 1325405700), LINESTRING M (1 1 1325406000,0 0 1325406600))')::tgeompoints);
SELECT astext((geometry 'GEOMETRYCOLLECTION ZM (LINESTRING ZM (0 0 0 1325404800,1 1 1 1325405400),
	POINT ZM (1 1 1 1325405700), LINESTRING ZM (1 1 1 1325406000,0 0 0 1325406600))')::tgeompoints);

SELECT astext((geography 'GEOMETRYCOLLECTION M (LINESTRING M (0 0 1325404800,1 1 1325405400),
	POINT M (1 1 1325405700), LINESTRING M (1 1 1325406000,0 0 1325406600))')::tgeogpoints);
SELECT astext((geography 'GEOMETRYCOLLECTION ZM (LINESTRING ZM (0 0 0 1325404800,1 1 1 1325405400),
	POINT ZM (1 1 1 1325405700), LINESTRING ZM (1 1 1 1325406000,0 0 0 1325406600))')::tgeogpoints);
	
/*****************************************************************************/

SELECT st_astext((tgeompointinst 'Point(0 0)@2001-01-01')::geometry);

SELECT (tgeompointinst 'Point(0 0)@[2001-01-01')::geometry;

SELECT st_astext((tgeogpointinst 'Point(0 0)@2001-01-01')::geography);

SELECT (tgeogpointinst 'Point(0 0)@2001-01-01')::geography;

--------------------------------------------------------

SELECT st_astext((tgeompoint 'Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02)')::geometry);

SELECT (tgeompoint 'Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02)')::geometry;

SELECT st_astext((tgeogpoint 'Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02)')::geography);

SELECT (tgeogpoint 'Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02)')::geography;

--------------------------------------------------------

SELECT st_astext((tgeompointp '{Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02),
    Point(0 1)@Point(1 1)@[2001-01-02, 2001-01-03)}')::geometry);

SELECT (tgeompointp '{Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02),
    Point(0 1)@Point(1 1)@[2001-01-02, 2001-01-03)}')::geometry;

SELECT st_astext((tgeogpointp '{Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02),
    Point(0 1)@Point(1 1)@[2001-01-02, 2001-01-03)}')::geography);

SELECT (tgeogpointp '{Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02),
    Point(0 1)@Point(1 1)@[2001-01-02, 2001-01-03)}'::geography);

--------------------------------------------------------

SELECT st_astext((tgeompointi '{Point(0 0)@2001-01-01, Point(0 1)@2001-01-02}')::geometry);

SELECT (tgeompointi '{Point(0 0)@2001-01-01, Point(0 1)@2001-01-02}'::geometry);

SELECT st_astext((tgeogpointi '{Point(0 0)@2001-01-01, Point(0 1)@2001-01-02}')::geography);

SELECT (tgeogpointi '{Point(0 0)@2001-01-01, Point(0 1)@2001-01-02}')::geography;

/*****************************************************************************/

SELECT astext(((tgeompointinst 'Point(0 0)@2001-01-01')::geometry)::tgeompointinst);

SELECT astext(((tgeogpointinst 'Point(0 0)@2001-01-01')::geography)::tgeogpointinst);

-- ERRORS
SELECT (geometry 'LINESTRING(0 0,0 1)')::tgeompointinst
SELECT (geography 'LINESTRING(0 0,0 1)')::tgeogpointinst
SELECT (geometry 'POINTM EMPTY')::tgeompointinst
SELECT (geometry 'POINTM EMPTY')::tgeompointinst

--------------------------------------------------------

SELECT astext(((tgeompoint 'Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02)')::geometry)::tgeompointper);

SELECT astext(((tgeogpoint 'Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02)')::geography)::tgeogpointper);

-- ERRORS
SELECT (geometry 'LINESTRING(0 0,0 1)')::tgeompointper;
SELECT (geometry 'LINESTRINGM EMPTY')::tgeompointper;
SELECT (geometry 'LINESTRINGM(0 0 0,0 1 1,1 1 2)')::tgeompointper;
SELECT (geometry 'LINESTRINGM(0 0 0,0 1 0)')::tgeompointper;

--------------------------------------------------------

SELECT astext(((tgeompointp '{Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02)}')::geometry)::tgeompointp);

SELECT astext(((tgeogpointp '{Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02)}')::geography)::tgeogpointp);
    
SELECT astext(((tgeompointp '{Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02),
    Point(0 1)@Point(1 1)@[2001-01-02, 2001-01-03)}')::geometry)::tgeompointp);

SELECT astext(((tgeogpointp '{Point(0 0)@Point(0 1)@[2001-01-01, 2001-01-02),
    Point(0 1)@Point(1 1)@[2001-01-02, 2001-01-03)}')::geography)::tgeogpointp);

-- ERRORS
SELECT (geometry 'LINESTRING(0 0,0 1)')::tgeompointp;
SELECT (geography 'LINESTRING(0 0,0 1)')::tgeogpointp;
SELECT (geometry 'LINESTRINGM EMPTY')::tgeompointp;
SELECT (geography 'LINESTRINGM EMPTY')::tgeogpointp;
SELECT (geometry 'LINESTRINGM(0 0 0,0 1 0)')::tgeogpointp;
SELECT (geography 'LINESTRINGM(0 0 0,0 1 0)')::tgeogpointp;

--------------------------------------------------------

SELECT astext(((tgeompointi '{Point(0 0)@2001-01-01}')::geometry)::tgeompointi);

SELECT astext(((tgeogpointi '{Point(0 0)@2001-01-01}')::geography)::tgeogpointi);

SELECT astext(((tgeompointi '{Point(0 0)@2001-01-01, Point(0 1)@2001-01-02}')::geometry)::tgeompointi);

SELECT astext(((tgeogpointi '{Point(0 0)@2001-01-01, Point(0 1)@2001-01-02}')::geography)::tgeogpointi);

-- ERRORS
SELECT (geometry 'LINESTRINGM(0 0 0,0 1 0)')::tgeompointi;
SELECT astext((geometry 'POINTM EMPTY')::tgeompointi);
SELECT astext((geometry 'MULTIPOINTM EMPTY')::tgeompointi);
SELECT (geometry 'MULTIPOINTM(0 0 0,0 1 0)')::tgeompointi;

/*****************************************************************************/

select astext(atGeometry(
tgeompointp '{Point(0 0)@Point(1 1)@[2000-01-01,2000-01-02),
Point(1 1)@[2000-01-02,2000-01-03),Point(1 1)@Point(0 0)@[2000-01-03,2000-01-04)}',
'Point(1 1)'))
-- "{POINT(1 1)@[2000-01-02 00:00:00+00,2000-01-03 00:00:00+00]}"

select astext(atGeometry(
tgeompointp '{Point(0 0)@Point(1 1)@[2000-01-01,2000-01-02),
Point(1 1)@Point(0 0)@[2000-01-02,2000-01-03)}',
'Point(1 1)'))
-- "{POINT(1 1)@[2000-01-02 00:00:00+00,2000-01-02 00:00:00+00]}"

select astext(atGeometry(
tgeompointp '{Point(0 0)@Point(1 1)@[2000-01-01,2000-01-02),
Point(1 1)@Point(0 0)@[2000-01-02,2000-01-03)}',
'Point(0 0)'))
-- "{POINT(0 0)@[2000-01-01 00:00:00+00,2000-01-01 00:00:00+00]}"

select astext(atGeometry(
tgeompointp '{Point(0 0)@Point(1 1)@(2000-01-01,2000-01-02),
Point(1 1)@Point(0 0)@[2000-01-02,2000-01-03)}',
'Point(0 0)'))
-- NULL

select astext(atGeometry(
tgeompointp '{Point(0 0)@Point(1 1)@[2000-01-01,2000-01-02),
Point(1 1)@[2000-01-02,2000-01-03),
Point(1 1)@Point(1 2)@[2000-01-03,2000-01-04),
Point(1 2)@[2000-01-04,2000-01-05),
Point(1 2)@Point(0 0)@[2000-01-05,2000-01-06)}',
'Linestring(1 1,1 2)'))
-- "{POINT(1 1)@POINT(1 2)@[2000-01-02 00:00:00+00,2000-01-05 00:00:00+00]}"

select astext(atGeometry(
tgeompointp '{Point(1 1)@Point(1 2)@(2000-01-03,2000-01-04),
Point(1 2)@[2000-01-04,2000-01-05),
Point(1 2)@Point(0 0)@[2000-01-05,2000-01-06)}',
'Linestring(1 1,1 2)'))
-- "{POINT(1 1)@POINT(1 2)@(2000-01-03 00:00:00+00,2000-01-05 00:00:00+00]}"

select astext(atGeometry(
tgeompointp '{Point(0 0)@Point(1 1)@[2000-01-01,2000-01-02),
Point(1 1)@[2000-01-02,2000-01-03),
Point(1 1)@Point(1 2)@[2000-01-03,2000-01-04)}',
'Linestring(1 1,1 2)'))
"{POINT(1 1)@POINT(1 2)@[2000-01-02 00:00:00+00,2000-01-04 00:00:00+00)}"

select astext(atGeometry(
tgeompointp '{Point(1 1)@Point(1 2)@(2000-01-03,2000-01-04)}',
'Linestring(1 1,1 2)'))
-- "{POINT(1 1)@POINT(1 2)@(2000-01-03 00:00:00+00,2000-01-04 00:00:00+00)}"

-------------------------------------------------------------


SELECT asText(minusGeometry(tgeompointseq '[Point(-1 -1)@2012-01-01, Point(3 3)@2012-01-05)',
geometry 'Polygon((0 0,0 1,1 1,1 0,0 0))'));
--"{[POINT(-1 -1)@2012-01-01 00:00:00+01, POINT(0 0)@2012-01-02 00:00:00+01), (POINT(1 1)@2012-01-03 00:00:00+01, POINT(3 3)@2012-01-05 00:00:00+01)}"

SELECT asText(minusGeometry(
	tgeompoints(ARRAY[
	tgeompointseq '[Point(-1 -1)@2012-01-01, Point(3 3)@2012-01-05)',
	tgeompointseq '[Point(7 7)@2012-01-07, Point(3 3)@2012-01-10)'
	]),
	geometry 'Polygon((0 0,0 1,1 1,1 0,0 0))'));

--"{[POINT(-1 -1)@2012-01-01 00:00:00+01, POINT(0 0)@2012-01-02 00:00:00+01),
--  (POINT(1 1)@2012-01-03 00:00:00+01, POINT(3 3)@2012-01-05 00:00:00+01),
-- [POINT(7 7)@2012-01-07 00:00:00+01, POINT(3 3)@2012-01-10 00:00:00+01)}"

SELECT asText(minusGeometry(
	tgeompoints(ARRAY[
	tgeompointseq '[Point(-1 -1)@2012-01-01, Point(3 3)@2012-01-05)',
	tgeompointseq '[Point(8 7)@2012-01-07, Point(0 -1)@2012-01-10]'
	]),
	geometry 'Polygon((0 0,0 1,1 1,1 0,0 0))'));
--{[POINT(-1 -1)@2012-01-01 00:00:00+01, POINT(0 0)@2012-01-02 00:00:00+01),
-- (POINT(1 1)@2012-01-03 00:00:00+01, POINT(3 3)@2012-01-05 00:00:00+01),
-- [POINT(8 7)@2012-01-07 00:00:00+01, POINT(1 0)@2012-01-09 15:00:00+01),
-- (POINT(1 0)@2012-01-09 15:00:00+01, POINT(0 -1)@2012-01-10 00:00:00+01]}


SELECT asText(minusGeometry(
	tgeompoints(ARRAY[
	tgeompointseq '[Point(-1 -1)@2012-01-01, Point(3 3)@2012-01-05)',
	tgeompointseq '[Point(7 7)@2012-01-07, Point(1 0)@2012-01-10]'
	]),
	geometry 'Polygon((0 0,0 1,1 1,1 0,0 0))'));
--"{[POINT(-1 -1)@2012-01-01 00:00:00+01, POINT(0 0)@2012-01-02 00:00:00+01),
--  (POINT(1 1)@2012-01-03 00:00:00+01, POINT(3 3)@2012-01-05 00:00:00+01),
--  [POINT(7 7)@2012-01-07 00:00:00+01, POINT(1 0)@2012-01-10 00:00:00+01]}"
-- Need to be enhanced to exclude the last point (i.e., !rc instrad of rc).


SELECT asText(minusGeometry(
	tgeompointi(ARRAY[
	tgeompointinst 'Point(-1 -1)@2012-01-01',
	tgeompointinst 'Point(3 3)@2012-01-05)',
	tgeompointinst 'Point(0.5 0.5)@2012-01-07',
	tgeompointinst 'Point(0 0)@2012-01-10'
	]),
	geometry 'Polygon((0 0,0 1,1 1,1 0,0 0))'));
--"{POINT(-1 -1)@2012-01-01 00:00:00+01, POINT(3 3)@2012-01-05 00:00:00+01}"


--------------------------------------------------------------------------

select astext(NearestApproachInstant(
tgeompointseq '[Point(0 1)@2000-01-01, Point(1 0)@2000-01-02, Point(2 1)@2000-01-03, Point(0 1)@2000-01-04]',
geometry 'Linestring(1 2,1 3)'));
-- "POINT(1 1)@2000-01-03 12:00:00+00"

select astext(NearestApproachInstant(tgeompoints(ARRAY[tgeompointseq
'[Point(0 1)@2000-01-01, Point(1 0)@2000-01-02, Point(2 1)@2000-01-03, Point(0 1)@2000-01-04]',
'[Point(0 2.5)@2000-01-05, Point(2 2.5)@2000-01-06]']),
geometry 'Point(1 2,1 3)'));
-- "POINT(1 2.5)@2000-01-05 12:00:00+00"

select astext(NearestApproachInstant(
tgeompointseq '[Point(0 1)@2000-01-01, Point(1 2)@2000-01-02]',
tgeompointseq '[Point(1 0)@2000-01-01, Point(0 1)@2000-01-02]'));
-- "POINT(0.5 1.5)@2000-01-01 12:00:00+00"

/*****************************************************************************/
