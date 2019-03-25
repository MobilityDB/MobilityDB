-------------------------------------------------------------------------------
-- contains
-------------------------------------------------------------------------------

select contains(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select contains(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select contains(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select contains(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select contains(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select contains(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select contains(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select contains(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select contains(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select contains(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select contains(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select contains(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select contains(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select contains(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select contains(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select contains(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select contains(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select contains(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select contains(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select contains(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select contains(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select contains(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select contains(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select contains(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- containsproperly
-------------------------------------------------------------------------------

select containsproperly(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select containsproperly(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select containsproperly(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select containsproperly(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select containsproperly(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select containsproperly(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select containsproperly(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select containsproperly(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select containsproperly(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------


select containsproperly(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select containsproperly(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select containsproperly(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select containsproperly(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select containsproperly(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select containsproperly(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select containsproperly(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select containsproperly(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select containsproperly(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select containsproperly(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select containsproperly(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select containsproperly(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select containsproperly(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select containsproperly(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select containsproperly(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- covers
-------------------------------------------------------------------------------

select covers(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select covers(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select covers(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select covers(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select covers(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select covers(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select covers(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select covers(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select covers(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------


select covers(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select covers(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select covers(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select covers(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select covers(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select covers(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select covers(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select covers(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select covers(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select covers(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select covers(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select covers(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select covers(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select covers(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select covers(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- coveredby
-------------------------------------------------------------------------------

select coveredby(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select coveredby(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select coveredby(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select coveredby(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select coveredby(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select coveredby(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select coveredby(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select coveredby(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select coveredby(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------


select coveredby(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select coveredby(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select coveredby(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select coveredby(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select coveredby(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select coveredby(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select coveredby(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select coveredby(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select coveredby(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select coveredby(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select coveredby(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select coveredby(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select coveredby(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select coveredby(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select coveredby(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- crosses
-------------------------------------------------------------------------------

select crosses(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select crosses(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select crosses(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select crosses(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select crosses(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select crosses(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select crosses(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select crosses(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select crosses(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select crosses(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select crosses(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select crosses(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select crosses(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select crosses(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select crosses(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select crosses(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select crosses(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select crosses(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select crosses(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select crosses(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select crosses(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select crosses(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select crosses(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select crosses(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- disjoint
-------------------------------------------------------------------------------

select disjoint(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select disjoint(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select disjoint(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select disjoint(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select disjoint(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select disjoint(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select disjoint(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select disjoint(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select disjoint(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select disjoint(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select disjoint(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select disjoint(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select disjoint(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select disjoint(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select disjoint(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select disjoint(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select disjoint(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select disjoint(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select disjoint(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select disjoint(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select disjoint(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select disjoint(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
		tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select disjoint(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select disjoint(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- equals
-------------------------------------------------------------------------------

select equals(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select equals(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select equals(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select equals(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select equals(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select equals(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select equals(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select equals(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select equals(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select equals(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select equals(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select equals(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select equals(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select equals(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select equals(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select equals(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select equals(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select equals(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select equals(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select equals(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select equals(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select equals(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select equals(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select equals(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- intersects
-------------------------------------------------------------------------------

select intersects(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select intersects(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select intersects(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select intersects(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select intersects(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select intersects(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select intersects(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select intersects(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select intersects(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select intersects(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select intersects(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select intersects(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select intersects(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select intersects(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select intersects(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select intersects(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select intersects(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select intersects(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select intersects(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select intersects(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select intersects(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select intersects(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select intersects(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select intersects(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- overlaps
-------------------------------------------------------------------------------

select overlaps(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select overlaps(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select overlaps(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select overlaps(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select overlaps(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select overlaps(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select overlaps(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select overlaps(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select overlaps(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select overlaps(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select overlaps(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select overlaps(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select overlaps(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select overlaps(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select overlaps(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select overlaps(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select overlaps(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select overlaps(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select overlaps(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select overlaps(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select overlaps(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select overlaps(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select overlaps(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select overlaps(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- touches
-------------------------------------------------------------------------------

select touches(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select touches(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select touches(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select touches(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select touches(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select touches(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select touches(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select touches(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select touches(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select touches(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select touches(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select touches(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select touches(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select touches(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select touches(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select touches(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select touches(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select touches(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select touches(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select touches(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select touches(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select touches(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select touches(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select touches(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------------------------------
-- within
-------------------------------------------------------------------------------

select within(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select within(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select within(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select within(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select within(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select within(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select within(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select within(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select within(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select within(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select within(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select within(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select within(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select within(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select within(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select within(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select within(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select within(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select within(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select within(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select within(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select within(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select within(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select within(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- dwithin
-------------------------------------------------------------------------------

select dwithin(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00', 10);

select dwithin(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 10);

select dwithin(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', 10);

select dwithin(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}', 10);

-------------------------------------------------------------------------------

select dwithin(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0), 10);

select dwithin(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00', 10);

select dwithin(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 10);

select dwithin(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', 10);

select dwithin(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}', 10);

-------------------------------------------------------------------------------

select dwithin(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0), 10);

select dwithin(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00', 10);

select dwithin(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 10);

select dwithin(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', 10);

select dwithin(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}', 10);

-------------------------------------------------------------------------------

select dwithin(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0), 10);

select dwithin(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00', 10);

select dwithin(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 10);

select dwithin(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', 10);

select dwithin(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}', 10);

-------------------------------------------------------------------------------

select dwithin(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0), 10);

select dwithin(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00', 10);

select dwithin(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 10);

select dwithin(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', 10);

select dwithin(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}', 10);

-------------------------------------------------------------------------------
-- relate (2 arguments returns text)
-------------------------------------------------------------------------------

select relate(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select relate(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select st_relate(ST_Point(0,0), ST_Point(0,0));

select relate(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select relate(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select relate(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select relate(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select relate(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select relate(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select relate(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select relate(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');
    
-------------------------------------------------------------------------------

select relate(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select relate(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select relate(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select relate(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select relate(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select relate(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select relate(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select relate(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select relate(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select relate(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select relate(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select relate(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select relate(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select relate(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select relate(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- relate (3 arguments returns boolean)
-------------------------------------------------------------------------------

select relate(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00', 'T*****FF*');

select relate(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00', 'T*****FF*');


select st_relate(ST_Point(0,0), ST_Point(0,0), 'T*****FF*');

select relate(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 'T*****FF*');

select relate(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', 'T*****FF*');

select relate(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}', 'T*****FF*');

-------------------------------------------------------------------------------

select relate(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0), 'T*****FF*');

select relate(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00', 'T*****FF*');

select relate(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 'T*****FF*');

select relate(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', 'T*****FF*');

select relate(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}', 'T*****FF*');

-------------------------------------------------------------------------------

select relate(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0), 'T*****FF*');

select relate(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00', 'T*****FF*');

select relate(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 'T*****FF*');

select relate(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', 'T*****FF*');

select relate(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}', 'T*****FF*');

-------------------------------------------------------------------------------

select relate(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0), 'T*****FF*');

select relate(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00', 'T*****FF*');

select relate(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 'T*****FF*');

select relate(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', 'T*****FF*');

select relate(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}', 'T*****FF*');

-------------------------------------------------------------------------------

select relate(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0), 'T*****FF*');

select relate(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00', 'T*****FF*');

select relate(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', 'T*****FF*');

select relate(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', 'T*****FF*');

select relate(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}', 'T*****FF*');
	
-------------------------------------------------------------------------------
