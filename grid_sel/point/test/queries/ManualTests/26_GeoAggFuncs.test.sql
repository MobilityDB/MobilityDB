/*****************************************************************************/

WITH Values AS (
select tgeompointinst(ST_Point(0,0), '2012-01-01 08:00:00') as value
union
select tgeompointinst(ST_Point(0,2), '2012-01-01 08:00:00')
union
select tgeompointinst(ST_Point(2,0), '2012-01-01 08:00:00')
union
select tgeompointinst(ST_Point(2,2), '2012-01-01 08:00:00')
)
select display(centroid(value))
from Values;

WITH Values AS (
select tgeompointinst(ST_Point(0,0), '2012-01-01 08:00:00') as value
union
select tgeompointinst(ST_Point(0,2), '2012-01-01 08:05:00')
union
select tgeompointinst(ST_Point(2,0), '2012-01-01 08:10:00')
union
select tgeompointinst(ST_Point(2,2), '2012-01-01 08:15:00')
)
select display(centroid(value))
from Values;

/*****************************************************************************/

WITH Values(v) AS (select unnest(ARRAY[
tgeompointper 'Point(0 0)->Point(1 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
tgeompointper 'Point(1 1)->Point(2 2)@[2012-01-01 08:05:00, 2012-01-01 08:10:00)',
tgeompointper 'Point(2 2)->Point(3 3)@[2012-01-01 08:10:00, 2012-01-01 08:15:00)',
tgeompointper 'Point(3 3)->Point(4 4)@[2012-01-01 08:15:00, 2012-01-01 08:20:00)'])
)
select display(centroid(v))
from Values;

-- "{"POINT(0 0)->POINT(4 4)@[2012-01-01 08:00:00,2012-01-01 08:20:00)"}"

WITH Values(v) AS (select unnest(ARRAY[
tgeompointper 'Point(0 0)->Point(1 1)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)',
tgeompointper 'Point(1 1)->Point(2 2)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)',
tgeompointper 'Point(2 2)->Point(3 3)@[2012-01-01 08:10:00, 2012-01-01 08:20:00)',
tgeompointper 'Point(3 3)->Point(4 4)@[2012-01-01 08:15:00, 2012-01-01 08:25:00)'])
)
select display(centroid(v))
from Values;

-- {"POINT(0 0)->POINT(0.5 0.5)@[2012-01-01 08:00:00,2012-01-01 08:05:00)",
-- "POINT(0.75 0.75)->POINT(1.25 1.25)@[2012-01-01 08:05:00,2012-01-01 08:10:00)",
-- "POINT(1.75 1.75)->POINT(2.25 2.25)@[2012-01-01 08:10:00,2012-01-01 08:15:00)",
-- "POINT(2.75 2.75)->POINT(3.25 3.25)@[2012-01-01 08:15:00,2012-01-01 08:20:00)",
-- "POINT(3.5 3.5)->POINT(4 4)@[2012-01-01 08:20:00,2012-01-01 08:25:00)"}

/*****************************************************************************/

WITH Values(v) AS (select unnest(ARRAY[
tgeompointp '{Point(0 0)->Point(1 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
	Point(1 0)->Point(0 0)@[2012-01-01 08:05:00, 2012-01-01 08:10:00)}',
tgeompointp '{Point(1 1)->Point(2 2)@[2012-01-01 08:10:00, 2012-01-01 08:15:00),
	Point(2 2)->Point(1 1)@[2012-01-01 08:15:00, 2012-01-01 08:20:00)}',
tgeompointp '{Point(2 2)->Point(3 3)@[2012-01-01 08:20:00, 2012-01-01 08:25:00),
	Point(3 3 )->Point(2 2)@[2012-01-01 08:25:00, 2012-01-01 08:05:30)}',
tgeompointp '{Point(3 3)->Point(4 4)@[2012-01-01 08:30:00, 2012-01-01 08:35:00),
	Point(4 4)->Point(3 3)@[2012-01-01 08:35:00, 2012-01-01 08:40:00)}'])
)
select display(centroid(v))
from Values;

-- "{"POINT(0 0)->POINT(4 4)@[2012-01-01 08:00:00,2012-01-01 08:20:00)"}"

WITH Values(v) AS (select unnest(ARRAY[
tgeompointp '{Point(0 0)->Point(1 1)@[2012-01-01 08:00:00, 2012-01-01 08:10:00)',
tgeompointp '{Point(1 1)->Point(2 2)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)',
tgeompointp '{Point(2 2)->Point(3 3)@[2012-01-01 08:10:00, 2012-01-01 08:20:00)',
tgeompointp '{Point(3 3)->Point(4 4)@[2012-01-01 08:15:00, 2012-01-01 08:25:00)'])
)
select display(centroid(v))
from Values;

-- {"POINT(0 0)->POINT(0.5 0.5)@[2012-01-01 08:00:00,2012-01-01 08:05:00)",
-- "POINT(0.75 0.75)->POINT(1.25 1.25)@[2012-01-01 08:05:00,2012-01-01 08:10:00)",
-- "POINT(1.75 1.75)->POINT(2.25 2.25)@[2012-01-01 08:10:00,2012-01-01 08:15:00)",
-- "POINT(2.75 2.75)->POINT(3.25 3.25)@[2012-01-01 08:15:00,2012-01-01 08:20:00)",
-- "POINT(3.5 3.5)->POINT(4 4)@[2012-01-01 08:20:00,2012-01-01 08:25:00)"}

/*****************************************************************************/

copy (
WITH Values(v) AS (select unnest(ARRAY[
tgeompointp '{Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
	Point(2 2)->Point(3 3)@[2012-01-01 08:05:00, 2012-01-01 08:10:00)}',
tgeompointp '{Point(3 3)->Point(4 4)@[2012-01-01 08:10:00, 2012-01-01 08:15:00),
	Point(4 4)->Point(6 6)@[2012-01-01 08:15:00, 2012-01-01 08:20:00)}',
tgeompointp '{Point(6 6)->Point(8 8)@[2012-01-01 08:20:00, 2012-01-01 08:25:00),
	Point(8 8)->Point(9 9)@[2012-01-01 08:25:00, 2012-01-01 08:30:00)}',
tgeompointp '{Point(9 9)->Point(10 10)@[2012-01-01 08:30:00, 2012-01-01 08:35:00),
	Point(10 10)->Point(12 12)@[2012-01-01 08:35:00, 2012-01-01 08:40:00)}'])
)
select display(centroid(v))
from Values) to stdout;

-- "{POINT(0 0)->POINT(2 2)@[2012-01-01 08:00:00,2012-01-01 08:05:00), 
-- POINT(2 2)->POINT(4 4)@[2012-01-01 08:05:00,2012-01-01 08:15:00], 
-- POINT(4 4)->POINT(8 8)@(2012-01-01 08:15:00,2012-01-01 08:25:00], 
-- POINT(8 8)->POINT(10 10)@(2012-01-01 08:25:00,2012-01-01 08:35:00], 
-- POINT(10 10)->POINT(12 12)@(2012-01-01 08:35:00,2012-01-01 08:40:00)}"

copy (
WITH Values(v) AS (select unnest(ARRAY[
tgeompointp '{Point(0 0)->Point(2 2)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
	Point(2 2)->Point(3 3)@[2012-01-01 08:05:00, 2012-01-01 08:10:00)}',
tgeompointp '{Point(3 3)->Point(4 4)@[2012-01-01 08:05:00, 2012-01-01 08:10:00),
	Point(4 4)->Point(6 6)@[2012-01-01 08:10:00, 2012-01-01 08:15:00)}',
tgeompointp '{Point(5 5)->Point(7 7)@[2012-01-01 08:10:00, 2012-01-01 08:15:00),
	Point(7 7)->Point(8 8)@[2012-01-01 08:15:00, 2012-01-01 08:20:00)}',
tgeompointp '{Point(8 8)->Point(9 9)@[2012-01-01 08:15:00, 2012-01-01 08:20:00),
	Point(9 9)->Point(11 11)@[2012-01-01 08:20:00, 2012-01-01 08:25:00)}'])
)
select display(centroid(v))
from Values) to stdout;

-- "{POINT(0 0)->POINT(2 2)@[2012-01-01 08:00:00,2012-01-01 08:05:00), 
-- POINT(2.5 2.5)->POINT(3.5 3.5)@[2012-01-01 08:05:00,2012-01-01 08:10:00), 
-- POINT(4.5 4.5)->POINT(6.5 6.5)@[2012-01-01 08:10:00,2012-01-01 08:15:00), 
-- POINT(7.5 7.5)->POINT(8.5 8.5)@[2012-01-01 08:15:00,2012-01-01 08:20:00), 
-- POINT(9 9)->POINT(11 11)@[2012-01-01 08:20:00,2012-01-01 08:25:00)}"}

/*****************************************************************************/

with values(v) as (
select tgeompointi(ARRAY[
  tgeompointinst 'Point(1 1)@2000-01-01 08:00',
  tgeompointinst 'Point(3 3)@2000-01-01 08:10',
  tgeompointinst 'Point(5 5)@2000-01-01 08:20',
  tgeompointinst 'Point(7 7)@2000-01-01 08:30']) 
union all
select tgeompointi(ARRAY[
  tgeompointinst 'Point(2 2)@2000-01-01 08:05',
  tgeompointinst 'Point(4 4)@2000-01-01 08:15',
  tgeompointinst 'Point(6 6)@2000-01-01 08:25']) 
)
select centroid(v) from values;

with values(v) as (
select tgeompointi(ARRAY[
  tgeompointinst 'Point(1 1)@2000-01-01 08:00',
  tgeompointinst 'Point(3 3)@2000-01-01 08:10',
  tgeompointinst 'Point(5 5)@2000-01-01 08:20',
  tgeompointinst 'Point(7 7)@2000-01-01 08:30']) 
union all
select tgeompointi(ARRAY[
  tgeompointinst 'Point(2 2)@2000-01-01 08:05',
  tgeompointinst 'Point(4 4)@2000-01-01 08:20',
  tgeompointinst 'Point(6 6)@2000-01-01 08:25']) 
)
select centroid(v) from values;

/*****************************************************************************/