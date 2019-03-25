/*****************************************************************************/

select ST_Point(0,0) << tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select ST_Point(0,0) >> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select ST_Point(0,0) &< tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select ST_Point(0,0) &> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

select ST_Point(0,0) <<| tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select ST_Point(0,0) |>> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select ST_Point(0,0) &<| tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select ST_Point(0,0) |&> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

select timestamp '2001-01-01 07:00:00' <<# tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select timestamp '2001-01-01 07:00:00' #>> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select timestamp '2001-01-01 07:00:00' &<# tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select timestamp '2001-01-01 07:00:00' #&> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

/*****************************************************************************/

select ST_Point(0,0) << tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select ST_Point(0,0) >> tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select ST_Point(0,0) &< tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select ST_Point(0,0) &> tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

select ST_Point(0,0) <<| tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select ST_Point(0,0) |>> tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select ST_Point(0,0) &<| tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select ST_Point(0,0) |&> tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

select timestamp '2001-01-01 07:00:00' <<# tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select timestamp '2001-01-01 07:00:00' #>> tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select timestamp '2001-01-01 07:00:00' &<# tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select timestamp '2001-01-01 07:00:00' #&> tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

/*****************************************************************************/

select ST_Point(0,0) << tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select ST_Point(0,0) >> tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select ST_Point(0,0) &< tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select ST_Point(0,0) &> tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

select ST_Point(0,0) <<| tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select ST_Point(0,0) |>> tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select ST_Point(0,0) &<| tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select ST_Point(0,0) |&> tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

select timestamp '2001-01-01 07:00:00' <<# tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select timestamp '2001-01-01 07:00:00' #>> tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select timestamp '2001-01-01 07:00:00' &<# tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select timestamp '2001-01-01 07:00:00' #&> tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

/*****************************************************************************/

select ST_Point(0,0) << tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select ST_Point(0,0) >> tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select ST_Point(0,0) &< tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select ST_Point(0,0) &> tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

select ST_Point(0,0) <<| tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select ST_Point(0,0) |>> tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select ST_Point(0,0) &<| tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select ST_Point(0,0) |&> tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

select timestamp '2001-01-01 07:00:00' <<# tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select timestamp '2001-01-01 07:00:00' #>> tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select timestamp '2001-01-01 07:00:00' &<# tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select timestamp '2001-01-01 07:00:00' #&> tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

/*****************************************************************************/

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' << ST_Point(0,0);
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' >> ST_Point(0,0);
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &< ST_Point(0,0);
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &> ST_Point(0,0);

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' <<| ST_Point(0,0);
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' |>> ST_Point(0,0);
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &<| ST_Point(0,0);
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' |&> ST_Point(0,0);

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' <<# timestamp '2001-01-01 07:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' #>> timestamp '2001-01-01 07:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &<# timestamp '2001-01-01 07:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' #&> timestamp '2001-01-01 07:00:00';


/*****************************************************************************/

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' << tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' >> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &< tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' <<| tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' |>> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &<| tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' |&> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' <<# tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' #>> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &<# tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' #&> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

/*****************************************************************************/

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' << 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' >> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &< 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' <<| 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' |>> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &<| 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' |&> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' <<# 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' #>> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &<# 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' #&> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

/*****************************************************************************/

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' << 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' >> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &< 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' <<| 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' |>> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &<| 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' |&> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' <<# 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' #>> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &<# 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' #&> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

/*****************************************************************************/

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' << 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' >> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &< 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' <<| 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' |>> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &<| 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' |&> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' <<# 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' #>> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' &<# 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointinst 'Point(1 1)@2001-01-01 08:00:00' #&> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

/*****************************************************************************/

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' << ST_Point(0,0);
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' >> ST_Point(0,0);
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &< ST_Point(0,0);
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &> ST_Point(0,0);

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' <<| ST_Point(0,0);
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' |>> ST_Point(0,0);
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &<| ST_Point(0,0);
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' |&> ST_Point(0,0);

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' <<# timestamp '2001-01-01 07:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' #>> timestamp '2001-01-01 07:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &<# timestamp '2001-01-01 07:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' #&> timestamp '2001-01-01 07:00:00';


/*****************************************************************************/

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' << tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' >> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &< tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' <<| tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' |>> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &<| tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' |&> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' <<# tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' #>> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &<# tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' #&> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

/*****************************************************************************/

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' << 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' >> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &< 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' <<| 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' |>> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &<| 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' |&> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' <<# 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' #>> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &<# 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' #&> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

/*****************************************************************************/

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' << 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' >> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &< 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' <<| 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' |>> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &<| 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' |&> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' <<# 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' #>> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &<# 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' #&> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

/*****************************************************************************/

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' << 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' >> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &< 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' <<| 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' |>> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &<| 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' |&> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' <<# 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' #>> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' &<# 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)' #&> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';


/*****************************************************************************/

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' << ST_Point(0,0);
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' >> ST_Point(0,0);
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &< ST_Point(0,0);
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &> ST_Point(0,0);

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' <<| ST_Point(0,0);
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' |>> ST_Point(0,0);
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &<| ST_Point(0,0);
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' |&> ST_Point(0,0);

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' <<# timestamp '2001-01-01 07:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' #>> timestamp '2001-01-01 07:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &<# timestamp '2001-01-01 07:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' #&> timestamp '2001-01-01 07:00:00';


/*****************************************************************************/

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' << tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' >> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &< tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' <<| tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' |>> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &<| tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' |&> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' <<# tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' #>> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &<# tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' #&> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

/*****************************************************************************/

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' << 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' >> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &< 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' <<| 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' |>> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &<| 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' |&> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' <<# 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' #>> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &<# 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' #&> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

/*****************************************************************************/

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' << 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' >> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &< 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' <<| 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' |>> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &<| 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' |&> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' <<# 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' #>> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &<# 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' #&> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

/*****************************************************************************/

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' << 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' >> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &< 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' <<| 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' |>> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &<| 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' |&> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' <<# 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' #>> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' &<# 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}' #&> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

/*****************************************************************************/

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' << ST_Point(0,0);
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' >> ST_Point(0,0);
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &< ST_Point(0,0);
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &> ST_Point(0,0);

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' <<| ST_Point(0,0);
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' |>> ST_Point(0,0);
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &<| ST_Point(0,0);
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' |&> ST_Point(0,0);

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' <<# timestamp '2001-01-01 07:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' #>> timestamp '2001-01-01 07:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &<# timestamp '2001-01-01 07:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' #&> timestamp '2001-01-01 07:00:00';

/*****************************************************************************/

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' << tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' >> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &< tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' <<| tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' |>> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &<| tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' |&> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' <<# tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' #>> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &<# tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' #&> tgeompointinst 'Point(1 1)@2001-01-01 08:00:00';

/*****************************************************************************/

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' << 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' >> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &< 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' <<| 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' |>> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &<| 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' |&> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' <<# 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' #>> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &<# 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' #&> 
	tgeompointper 'Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00)';

/*****************************************************************************/

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' << 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' >> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &< 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' <<| 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' |>> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &<| 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' |&> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' <<# 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' #>> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &<# 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' #&> 
	tgeompointp '{Point(0 0)->Point(0 1)@[2012-01-01 08:00:00, 2012-01-01 08:05:00),
    Point(0 1)->Point(1 1)@[2012-01-01 08:05:00, 2012-01-01 08:15:00)}';

/*****************************************************************************/

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' << 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' >> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &< 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' <<| 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' |>> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &<| 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' |&> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' <<# 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' #>> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' &<# 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';
select tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}' #&> 
	tgeompointi '{Point(0 0)@2012-01-01 08:00:00, Point(0 1)@2012-01-01 08:05:00}';

/*****************************************************************************/





