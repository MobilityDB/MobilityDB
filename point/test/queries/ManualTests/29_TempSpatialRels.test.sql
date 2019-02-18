-------------------------------------------------------------------------------
-- tcontains
-------------------------------------------------------------------------------

select tcontains(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcontains(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcontains(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcontains(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tcontains(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select tcontains(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcontains(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcontains(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcontains(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tcontains(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select tcontains(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcontains(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcontains(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcontains(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tcontains(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select tcontains(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcontains(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcontains(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcontains(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tcontains(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select tcontains(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcontains(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcontains(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcontains(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- tcovers
-------------------------------------------------------------------------------

select tcovers(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcovers(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcovers(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcovers(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tcovers(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select tcovers(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcovers(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcovers(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcovers(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tcovers(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select tcovers(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcovers(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcovers(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcovers(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tcovers(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select tcovers(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcovers(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcovers(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcovers(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tcovers(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select tcovers(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcovers(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcovers(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcovers(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- tcoveredby
-------------------------------------------------------------------------------

select tcoveredby(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcoveredby(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcoveredby(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcoveredby(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tcoveredby(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select tcoveredby(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcoveredby(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcoveredby(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcoveredby(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------


select tcoveredby(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select tcoveredby(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcoveredby(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcoveredby(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcoveredby(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tcoveredby(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select tcoveredby(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcoveredby(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcoveredby(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcoveredby(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tcoveredby(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select tcoveredby(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tcoveredby(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tcoveredby(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tcoveredby(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- tdisjoint
-------------------------------------------------------------------------------

select tdisjoint(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tdisjoint(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tdisjoint(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tdisjoint(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tdisjoint(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select tdisjoint(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tdisjoint(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tdisjoint(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tdisjoint(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tdisjoint(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select tdisjoint(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tdisjoint(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tdisjoint(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tdisjoint(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tdisjoint(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select tdisjoint(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tdisjoint(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tdisjoint(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tdisjoint(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tdisjoint(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select tdisjoint(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tdisjoint(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tdisjoint(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tdisjoint(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- tequals
-------------------------------------------------------------------------------

select tequals(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tequals(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tequals(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tequals(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tequals(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select tequals(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tequals(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tequals(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tequals(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tequals(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select tequals(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tequals(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tequals(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tequals(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tequals(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select tequals(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tequals(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tequals(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tequals(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tequals(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select tequals(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tequals(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tequals(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tequals(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- tintersects
-------------------------------------------------------------------------------

select tintersects(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tintersects(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tintersects(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tintersects(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tintersects(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select tintersects(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tintersects(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tintersects(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tintersects(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tintersects(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select tintersects(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tintersects(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tintersects(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tintersects(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tintersects(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select tintersects(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tintersects(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tintersects(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tintersects(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select tintersects(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select tintersects(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select tintersects(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select tintersects(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select tintersects(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------
-- ttouches
-------------------------------------------------------------------------------

select ttouches(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select ttouches(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select ttouches(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select ttouches(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select ttouches(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select ttouches(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select ttouches(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select ttouches(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select ttouches(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select ttouches(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select ttouches(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select ttouches(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select ttouches(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select ttouches(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select ttouches(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select ttouches(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select ttouches(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select ttouches(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select ttouches(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select ttouches(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select ttouches(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select ttouches(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select ttouches(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select ttouches(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------------------------------
-- twithin
-------------------------------------------------------------------------------

select twithin(ST_Point(0,0),
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select twithin(ST_Point(0,0),
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select twithin(ST_Point(0,0),
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select twithin(ST_Point(0,0),
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select twithin(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00', ST_Point(0,0));

select twithin(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select twithin(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select twithin(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select twithin(tgeompointinst 'Point(1 1)@2012-01-01 08:00:00',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select twithin(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)', ST_Point(0,0));

select twithin(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select twithin(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select twithin(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select twithin(tgeompointper 'Point(1 1)->Point(0 0)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select twithin(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}', ST_Point(0,0));

select twithin(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select twithin(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select twithin(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select twithin(tgeompointp '{Point(1 1)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 0)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

select twithin(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}', ST_Point(0,0));

select twithin(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointinst 'Point(0 0)@2012-01-01 08:00:00');

select twithin(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)');

select twithin(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}');

select twithin(tgeompointi '{Point(1 1)@2012-01-01 08:00:00, Point(1 0)@2012-01-01 08:05:00}',
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00,
    Point(0 1)@2012-01-01 08:05:00}');

-------------------------------------------------------------------------------

SELECT tdwithin(
geometry 'Polygon((1 0,1 1,2 1,2 0,1 0))',
tgeompointper 'Point(0 2)->Point(3 2)@[2001-01-01 00:00:00, 2001-01-01 00:15:00)',
1) 
-- "{f@[2001-01-01 00:00:00,2001-01-01 00:05:00), t@[2001-01-01 00:05:00,2001-01-01 00:10:00], f@(2001-01-01 00:10:00,2001-01-01 00:15:00)}"

SELECT tdwithin(
geometry 'Polygon((1 0,1 0.5,2 0.5,2 0,1 0))',
tgeompointper 'Point(0 2)->Point(3 2)@[2001-01-01 00:00:00, 2001-01-01 00:15:00)',
1)
-- "{f@[2001-01-01 00:00:00,2001-01-01 00:15:00)}"

SELECT tdwithin(
geometry 'Polygon((1 0,1 1.5,2 1.5,2 0,1 0))',
tgeompointper 'Point(0 2)->Point(3 2)@[2001-01-01 00:00:00, 2001-01-01 00:15:00)',
1)
-- "{f@[2001-01-01 00:00:00,2001-01-01 00:00:41.648243), t@[2001-01-01 00:00:41.648243,2001-01-01 00:14:18.351756], f@(2001-01-01 00:14:18.351756,2001-01-01 00:15:00)}"

/*
SELECT st_astext(st_intersection(
st_buffer(geometry 'Polygon((1 0,1 1.5,2 1.5,2 0,1 0))',1),
geometry 'Linestring(0 2,3 2)'))
-- "LINESTRING(0.138827479321097 2,2.8611725206789 2)"

SELECT astext(atValue(
tgeompointper 'Point(0 2)->Point(3 2)@[2001-01-01 00:00:00, 2001-01-01 00:15:00)',
'Point(0.138827479321097 2)'))
-- "POINT(0.138827479321097 2)@[2001-01-01 00:00:41.648243,2001-01-01 00:00:41.648243]"

SELECT astext(atValue(
tgeompointper 'Point(0 2)->Point(3 2)@[2001-01-01 00:00:00, 2001-01-01 00:15:00)',
'Point(2.8611725206789 2)'))
-- "POINT(2.8611725206789 2)@[2001-01-01 00:14:18.351756,2001-01-01 00:14:18.351756]"
*/

-------------------------------------------------------------------------------

select tdwithin(
tgeompointper 'Point(0 0)->Point(1 1)@[2001-01-01 08:00:00, 2001-01-01 08:10:00)',
tgeompointper 'Point(0 0)->Point(1 1)@[2001-01-01 08:00:00, 2001-01-01 08:10:00)', 1)
-- "{t@[2001-01-01 08:00:00,2001-01-01 08:10:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(1 1)@[2001-01-01 08:00:00, 2001-01-01 08:10:00)',
tgeompointper 'Point(0 1)->Point(1 2)@[2001-01-01 08:00:00, 2001-01-01 08:10:00)', 1)
-- "{t@[2001-01-01 08:00:00,2001-01-01 08:10:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(1 1)@[2001-01-01 08:00:00, 2001-01-01 08:10:00)',
tgeompointper 'Point(0 2)->Point(1 3)@[2001-01-01 08:00:00, 2001-01-01 08:10:00)', 1)
-- "{f@[2001-01-01 08:00:00,2001-01-01 08:10:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(2 2)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(0 2)->Point(2 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)', 
1)
-- "{f@[2000-01-01 00:00:00,2000-01-01 00:05:00), t@[2000-01-01 00:05:00,2000-01-01 00:15:00), f@[2000-01-01 00:15:00,2000-01-01 00:20:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(2 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(0 1)->Point(0 2)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
1)
-- "{t@[2000-01-01 00:00:00,2000-01-01 00:00:00], f@(2000-01-01 00:00:00,2000-01-01 00:20:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(2 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(0 0)->Point(0 2)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
1)
-- "{t@[2000-01-01 00:00:00,2000-01-01 00:07:04.264068), f@[2000-01-01 00:07:04.264068,2000-01-01 00:20:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(2 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(0 2)->Point(2 1)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
1) -- <-- distance = 1 at exclusive bound
"{f@[2000-01-01 00:00:00,2000-01-01 00:20:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(2 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(0 2)->Point(2 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
1) 
"{f@[2000-01-01 00:00:00,2000-01-01 00:10:00), t@[2000-01-01 00:10:00,2000-01-01 00:20:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(2 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(0 0)->Point(0 2)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
0) -- <-- distance 0 !!!
"{t@[2000-01-01 00:00:00,2000-01-01 00:00:00], f@(2000-01-01 00:00:00,2000-01-01 00:20:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(2 2)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(0 2)->Point(2 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
0) -- <-- distance 0 !!!
-- "{f@[2000-01-01 00:00:00,2000-01-01 00:10:00), t@[2000-01-01 00:10:00,2000-01-01 00:10:00], f@(2000-01-01 00:10:00,2000-01-01 00:20:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(2 2)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(2 0)->Point(2 2)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
0) -- <-- distance 0 at an exclusive bound !!!
"{f@[2000-01-01 00:00:00,2000-01-01 00:20:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(2 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(0 0)->Point(0 2)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
-1) -- <-- distance < 0 !!!
-- ERROR:  Tolerance cannot be less than zero

select tdwithin(
tgeompointper 'Point(0 0)->Point(8 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)', 
tgeompointper 'Point(2 0)->Point(6 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
1) 
-- "{f@[2000-01-01 00:00:00,2000-01-01 00:05:00), t@[2000-01-01 00:05:00,2000-01-01 00:15:00), f@[2000-01-01 00:15:00,2000-01-01 00:20:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(8 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(2 0)->Point(8 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
1) 
-- "{f@[2000-01-01 00:00:00,2000-01-01 00:10:00), t@[2000-01-01 00:10:00,2000-01-01 00:20:00)}"

select tdwithin(
tgeompointper 'Point(0 0)->Point(8 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(2 0)->Point(7 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
1) 
-- "{f@[2000-01-01 00:00:00,2000-01-01 00:06:39.999999), t@[2000-01-01 00:06:39.999999,2000-01-01 00:20:00)}""

select tdwithin(
tgeompointper 'Point(0 0)->Point(8 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
tgeompointper 'Point(8 0)->Point(0 0)@[2000-01-01 00:00:00, 2000-01-01 00:20:00)',
1) 
-- "{f@[2000-01-01 00:00:00,2000-01-01 00:08:45), t@[2000-01-01 00:08:45,2000-01-01 00:11:15), f@[2000-01-01 00:11:15,2000-01-01 00:20:00)}"

-------------------------------------------------------------------------------
