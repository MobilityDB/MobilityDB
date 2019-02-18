/******************************************************************************/

/* tboolinst */
--before
select * from tbl_tboolinst where inst <<# timestamp '2001-05-01';
select * from tbl_tboolinst where inst <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolinst where inst <<# tboolinst 'true@2001-05-01';
select * from tbl_tboolinst where inst <<# tboolper 'true@[2001-05-01, 2001-07-01)';
select * from tbl_tboolinst where inst <<# tboolp '{true@[2001-05-01, 2001-07-01), false@[2001-07-01, 2001-09-01)}';
select * from tbl_tboolinst where inst <<# tbooli '{true@2001-05-01, false@2001-07-01}';

--overbefore
select * from tbl_tboolinst where inst &<# timestamp '2001-07-01';
select * from tbl_tboolinst where inst &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolinst where inst &<# tboolinst 'true@2001-07-01';
select * from tbl_tboolinst where inst &<# tboolper 'true@[2001-05-01, 2001-07-01)';
select * from tbl_tboolinst where inst &<# tboolp '{true@[2001-03-01, 2001-05-01), false@[2001-05-01, 2001-07-01)}';
select * from tbl_tboolinst where inst &<# tbooli '{true@2001-05-01, false@2001-07-01}';

--after
select * from tbl_tboolinst where inst #>> timestamp '2001-05-01';
select * from tbl_tboolinst where inst #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolinst where inst #>> tboolinst 'true@2001-05-01';
select * from tbl_tboolinst where inst #>> tboolper 'true@[2001-03-01, 2001-05-01)';
select * from tbl_tboolinst where inst #>> tboolp '{true@[2001-01-01, 2001-03-01), false@[2001-03-01, 2001-05-01)}';
select * from tbl_tboolinst where inst #>> tbooli '{true@2001-03-01, false@2001-05-01}';

--overafter
select * from tbl_tboolinst where inst #&> timestamp '2001-03-01';
select * from tbl_tboolinst where inst #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolinst where inst #&> tboolinst 'true@2001-03-01';
select * from tbl_tboolinst where inst #&> tboolper 'true@[2001-03-01, 2001-05-01)';
select * from tbl_tboolinst where inst #&> tboolp '{true@[2001-03-01, 2001-05-01), false@[2001-05-01, 2001-07-01)}';
select * from tbl_tboolinst where inst #&> tbooli '{true@2001-03-01, false@2001-05-01}';

/* tboolper */
--before
select * from tbl_tboolper where per <<# timestamp '2001-05-01';
select * from tbl_tboolper where per <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolper where per <<# tboolinst 'true@2001-05-01';
select * from tbl_tboolper where per <<# tboolper 'true@[2001-05-01, 2001-07-01)';
select * from tbl_tboolper where per <<# tboolp '{true@[2001-05-01, 2001-07-01), false@[2001-07-01, 2001-09-01)}';
select * from tbl_tboolper where per <<# tbooli '{true@2001-05-01, false@2001-07-01}';

--overbefore
select * from tbl_tboolper where per &<# timestamp '2001-07-01';
select * from tbl_tboolper where per &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolper where per &<# tboolinst 'true@2001-07-01';
select * from tbl_tboolper where per &<# tboolper 'true@[2001-05-01, 2001-07-01)';
select * from tbl_tboolper where per &<# tboolp '{true@[2001-03-01, 2001-05-01), false@[2001-05-01, 2001-07-01)}';
select * from tbl_tboolper where per &<# tbooli '{true@2001-05-01, false@2001-07-01}';

--after
select * from tbl_tboolper where per #>> timestamp '2001-05-01';
select * from tbl_tboolper where per #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolper where per #>> tboolinst 'true@2001-05-01';
select * from tbl_tboolper where per #>> tboolper 'true@[2001-03-01, 2001-05-01)';
select * from tbl_tboolper where per #>> tboolp '{true@[2001-01-01, 2001-03-01), false@[2001-03-01, 2001-05-01)}';
select * from tbl_tboolper where per #>> tbooli '{true@2001-03-01, false@2001-05-01}';

--overafter
select * from tbl_tboolper where per #&> timestamp '2001-03-01';
select * from tbl_tboolper where per #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolper where per #&> tboolinst 'true@2001-03-01';
select * from tbl_tboolper where per #&> tboolper 'true@[2001-03-01, 2001-05-01)';
select * from tbl_tboolper where per #&> tboolp '{true@[2001-03-01, 2001-05-01), false@[2001-05-01, 2001-07-01)}';

/* tboolp */
--before
select * from tbl_tboolp where tp <<# timestamp '2001-05-01';
select * from tbl_tboolp where tp <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolp where tp <<# tboolinst 'true@2001-05-01';
select * from tbl_tboolp where tp <<# tboolper 'true@[2001-05-01, 2001-07-01)';
select * from tbl_tboolp where tp <<# tboolp '{true@[2001-05-01, 2001-07-01), false@[2001-07-01, 2001-09-01)}';
select * from tbl_tboolp where tp <<# tbooli '{true@2001-05-01, false@2001-07-01}';

--overbefore
select * from tbl_tboolp where tp &<# timestamp '2001-07-01';
select * from tbl_tboolp where tp &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolp where tp &<# tboolinst 'true@2001-07-01';
select * from tbl_tboolp where tp &<# tboolper 'true@[2001-05-01, 2001-07-01)';
select * from tbl_tboolp where tp &<# tboolp '{true@[2001-03-01, 2001-05-01), false@[2001-05-01, 2001-07-01)}';
select * from tbl_tboolp where tp &<# tbooli '{true@2001-05-01, false@2001-07-01}';

--after
select * from tbl_tboolp where tp #>> timestamp '2001-05-01';
select * from tbl_tboolp where tp #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolp where tp #>> tboolinst 'true@2001-05-01';
select * from tbl_tboolp where tp #>> tboolper 'true@[2001-03-01, 2001-05-01)';
select * from tbl_tboolp where tp #>> tboolp '{true@[2001-01-01, 2001-03-01), false@[2001-03-01, 2001-05-01)}';
select * from tbl_tboolp where tp #>> tbooli '{true@2001-03-01, false@2001-05-01}';

--overafter
select * from tbl_tboolp where tp #&> timestamp '2001-03-01';
select * from tbl_tboolp where tp #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tboolp where tp #&> tboolinst 'true@2001-03-01';
select * from tbl_tboolp where tp #&> tboolper 'true@[2001-03-01, 2001-05-01)';
select * from tbl_tboolp where tp #&> tboolp '{true@[2001-03-01, 2001-05-01), false@[2001-05-01, 2001-07-01)}';
select * from tbl_tboolp where tp #&> tbooli '{true@2001-03-01, false@2001-05-01}';

/* tbooli */
--before
select * from tbl_tbooli where ti <<# timestamp '2001-05-01';
select * from tbl_tbooli where ti <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tbooli where ti <<# tboolinst 'true@2001-05-01';
select * from tbl_tbooli where ti <<# tboolper 'true@[2001-05-01, 2001-07-01)';
select * from tbl_tbooli where ti <<# tboolp '{true@[2001-05-01, 2001-07-01), false@[2001-07-01, 2001-09-01)}';
select * from tbl_tbooli where ti <<# tbooli '{true@2001-05-01, false@2001-07-01}';

--overbefore
select * from tbl_tbooli where ti &<# timestamp '2001-07-01';
select * from tbl_tbooli where ti &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tbooli where ti &<# tboolinst 'true@2001-07-01';
select * from tbl_tbooli where ti &<# tboolper 'true@[2001-05-01, 2001-07-01)';
select * from tbl_tbooli where ti &<# tboolp '{true@[2001-03-01, 2001-05-01), false@[2001-05-01, 2001-07-01)}';
select * from tbl_tbooli where ti &<# tbooli '{true@2001-05-01, false@2001-07-01}';

--after
select * from tbl_tbooli where ti #>> timestamp '2001-05-01';
select * from tbl_tbooli where ti #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tbooli where ti #>> tboolinst 'true@2001-05-01';
select * from tbl_tbooli where ti #>> tboolper 'true@[2001-03-01, 2001-05-01)';
select * from tbl_tbooli where ti #>> tboolp '{true@[2001-01-01, 2001-03-01), false@[2001-03-01, 2001-05-01)}';
select * from tbl_tbooli where ti #>> tbooli '{true@2001-03-01, false@2001-05-01}';

--overafter
select * from tbl_tbooli where ti #&> timestamp '2001-03-01';
select * from tbl_tbooli where ti #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tbooli where ti #&> tboolinst 'true@2001-03-01';
select * from tbl_tbooli where ti #&> tboolper 'true@[2001-03-01, 2001-05-01)';
select * from tbl_tbooli where ti #&> tboolp '{true@[2001-03-01, 2001-05-01), false@[2001-05-01, 2001-07-01)}';
select * from tbl_tbooli where ti #&> tbooli '{true@2001-03-01, false@2001-05-01}';

/******************************************************************************/

/* tintinst */
--left
select * from tbl_tintinst where inst << 50;
select * from tbl_tintinst where inst << 50.0;
select * from tbl_tintinst where inst << intrange '[40,60]';
select * from tbl_tintinst where inst << floatrange '[40,60]';
select * from tbl_tintinst where inst << box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintinst where inst << tintinst '50@2001-05-01';
select * from tbl_tintinst where inst << tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst << tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintinst where inst << tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintinst where inst << tfloatinst '50@2001-05-01';
select * from tbl_tintinst where inst << tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst << tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintinst where inst << tfloati '{50@2001-05-01, 70@2001-07-01}';

--overleft
select * from tbl_tintinst where inst &< 50;
select * from tbl_tintinst where inst &< 50.0;
select * from tbl_tintinst where inst &< intrange '[40,60]';
select * from tbl_tintinst where inst &< floatrange '[40,60]';
select * from tbl_tintinst where inst &< box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintinst where inst &< tintinst '50@2001-05-01';
select * from tbl_tintinst where inst &< tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst &< tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintinst where inst &< tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintinst where inst &< tfloatinst '50@2001-05-01';
select * from tbl_tintinst where inst &< tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst &< tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintinst where inst &< tfloati '{50@2001-05-01, 70@2001-07-01}';

--right
select * from tbl_tintinst where inst >> 50;
select * from tbl_tintinst where inst >> 50.0;
select * from tbl_tintinst where inst >> intrange '[40,60]';
select * from tbl_tintinst where inst >> floatrange '[40,60]';
select * from tbl_tintinst where inst >> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintinst where inst >> tintinst '50@2001-05-01';
select * from tbl_tintinst where inst >> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst >> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintinst where inst >> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintinst where inst >> tfloatinst '50@2001-05-01';
select * from tbl_tintinst where inst >> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst >> tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintinst where inst >> tfloati '{50@2001-05-01, 70@2001-07-01}';

--overright
select * from tbl_tintinst where inst &> 50;
select * from tbl_tintinst where inst &> 50.0;
select * from tbl_tintinst where inst &> intrange '[40,60]';
select * from tbl_tintinst where inst &> floatrange '[40,60]';
select * from tbl_tintinst where inst &> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintinst where inst &> tintinst '50@2001-05-01';
select * from tbl_tintinst where inst &> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst &> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintinst where inst &> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintinst where inst &> tfloatinst '50@2001-05-01';
select * from tbl_tintinst where inst &> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst &> tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintinst where inst &> tfloati '{50@2001-05-01, 70@2001-07-01}';

--before
select * from tbl_tintinst where inst <<# timestamp '2001-05-01';
select * from tbl_tintinst where inst <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tintinst where inst <<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintinst where inst <<# tintinst '5@2001-05-01';
select * from tbl_tintinst where inst <<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst <<# tintp '{5@[2001-05-01, 2001-07-01), 6@[2001-07-01, 2001-09-01)}';
select * from tbl_tintinst where inst <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tintinst where inst <<# tfloatinst '3.5@2001-05-01';
select * from tbl_tintinst where inst <<# tfloatper '3.5@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst <<# tfloatp '{3.5@[2001-05-01, 2001-07-01), 5.6@[2001-07-01, 2001-09-01)}';
select * from tbl_tintinst where inst <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';

--overbefore
select * from tbl_tintinst where inst &<# timestamp '2001-07-01';
select * from tbl_tintinst where inst &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tintinst where inst &<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintinst where inst &<# tintinst '5@2001-07-01';
select * from tbl_tintinst where inst &<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst &<# tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintinst where inst &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tintinst where inst &<# tfloatinst '4.5@2001-07-01';
select * from tbl_tintinst where inst &<# tfloatper '4.5@[2001-05-01, 2001-07-01)';
select * from tbl_tintinst where inst &<# tfloatp '{4.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintinst where inst &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';

--after
select * from tbl_tintinst where inst #>> timestamp '2001-05-01';
select * from tbl_tintinst where inst #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tintinst where inst #>> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintinst where inst #>> tintinst '5@2001-05-01';
select * from tbl_tintinst where inst #>> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tintinst where inst #>> tintp '{5@[2001-01-01, 2001-03-01), 6@[2001-03-01, 2001-05-01)}';
select * from tbl_tintinst where inst #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tintinst where inst #>> tfloatinst '3.5@2001-05-01';
select * from tbl_tintinst where inst #>> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tintinst where inst #>> tfloatp '{3.5@[2001-01-01, 2001-03-01), 5.6@[2001-03-01, 2001-05-01)}';
select * from tbl_tintinst where inst #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

--overafter
select * from tbl_tintinst where inst #&> timestamp '2001-03-01';
select * from tbl_tintinst where inst #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tintinst where inst #&> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintinst where inst #&> tintinst '5@2001-03-01';
select * from tbl_tintinst where inst #&> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tintinst where inst #&> tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintinst where inst #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tintinst where inst #&> tfloatinst '3.5@2001-03-01';
select * from tbl_tintinst where inst #&> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tintinst where inst #&> tfloatp '{3.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintinst where inst #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

/* tintper */
--left
select * from tbl_tintper where per << 50;
select * from tbl_tintper where per << 50.0;
select * from tbl_tintper where per << intrange '[40,60]';
select * from tbl_tintper where per << floatrange '[40,60]';
select * from tbl_tintper where per << box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintper where per << tintinst '50@2001-05-01';
select * from tbl_tintper where per << tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per << tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintper where per << tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintper where per << tfloatinst '50@2001-05-01';
select * from tbl_tintper where per << tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per << tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintper where per << tfloati '{50@2001-05-01, 70@2001-07-01}';

--overleft
select * from tbl_tintper where per &< 50;
select * from tbl_tintper where per &< 50.0;
select * from tbl_tintper where per &< intrange '[40,60]';
select * from tbl_tintper where per &< floatrange '[40,60]';
select * from tbl_tintper where per &< box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintper where per &< tintinst '50@2001-05-01';
select * from tbl_tintper where per &< tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per &< tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintper where per &< tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintper where per &< tfloatinst '50@2001-05-01';
select * from tbl_tintper where per &< tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per &< tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintper where per &< tfloati '{50@2001-05-01, 70@2001-07-01}';

--right
select * from tbl_tintper where per >> 50;
select * from tbl_tintper where per >> 50.0;
select * from tbl_tintper where per >> intrange '[40,60]';
select * from tbl_tintper where per >> floatrange '[40,60]';
select * from tbl_tintper where per >> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintper where per >> tintinst '50@2001-05-01';
select * from tbl_tintper where per >> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per >> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintper where per >> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintper where per >> tfloatinst '50@2001-05-01';
select * from tbl_tintper where per >> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per >> tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintper where per >> tfloati '{50@2001-05-01, 70@2001-07-01}';

--overright
select * from tbl_tintper where per &> 50;
select * from tbl_tintper where per &> 50.0;
select * from tbl_tintper where per &> intrange '[40,60]';
select * from tbl_tintper where per &> floatrange '[40,60]';
select * from tbl_tintper where per &> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintper where per &> tintinst '50@2001-05-01';
select * from tbl_tintper where per &> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per &> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintper where per &> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintper where per &> tfloatinst '50@2001-05-01';
select * from tbl_tintper where per &> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per &> tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintper where per &> tfloati '{50@2001-05-01, 70@2001-07-01}';

--before
select * from tbl_tintper where per <<# timestamp '2001-05-01';
select * from tbl_tintper where per <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tintper where per <<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintper where per <<# tintinst '5@2001-05-01';
select * from tbl_tintper where per <<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per <<# tintp '{5@[2001-05-01, 2001-07-01), 6@[2001-07-01, 2001-09-01)}';
select * from tbl_tintper where per <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tintper where per <<# tfloatinst '3.5@2001-05-01';
select * from tbl_tintper where per <<# tfloatper '3.5@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per <<# tfloatp '{3.5@[2001-05-01, 2001-07-01), 5.6@[2001-07-01, 2001-09-01)}';
select * from tbl_tintper where per <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';

--overbefore
select * from tbl_tintper where per &<# timestamp '2001-07-01';
select * from tbl_tintper where per &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tintper where per &<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintper where per &<# tintinst '5@2001-07-01';
select * from tbl_tintper where per &<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per &<# tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintper where per &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tintper where per &<# tfloatinst '4.5@2001-07-01';
select * from tbl_tintper where per &<# tfloatper '4.5@[2001-05-01, 2001-07-01)';
select * from tbl_tintper where per &<# tfloatp '{4.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintper where per &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';

--after
select * from tbl_tintper where per #>> timestamp '2001-05-01';
select * from tbl_tintper where per #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tintper where per #>> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintper where per #>> tintinst '5@2001-05-01';
select * from tbl_tintper where per #>> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tintper where per #>> tintp '{5@[2001-01-01, 2001-03-01), 6@[2001-03-01, 2001-05-01)}';
select * from tbl_tintper where per #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tintper where per #>> tfloatinst '3.5@2001-05-01';
select * from tbl_tintper where per #>> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tintper where per #>> tfloatp '{3.5@[2001-01-01, 2001-03-01), 5.6@[2001-03-01, 2001-05-01)}';
select * from tbl_tintper where per #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

--overafter
select * from tbl_tintper where per #&> timestamp '2001-03-01';
select * from tbl_tintper where per #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tintper where per #&> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintper where per #&> tintinst '5@2001-03-01';
select * from tbl_tintper where per #&> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tintper where per #&> tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintper where per #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tintper where per #&> tfloatinst '3.5@2001-03-01';
select * from tbl_tintper where per #&> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tintper where per #&> tfloatp '{3.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintper where per #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

/* tintp */
--left
select * from tbl_tintp where tp << 50;
select * from tbl_tintp where tp << 50.0;
select * from tbl_tintp where tp << intrange '[40,60]';
select * from tbl_tintp where tp << floatrange '[40,60]';
select * from tbl_tintp where tp << box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintp where tp << tintinst '50@2001-05-01';
select * from tbl_tintp where tp << tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp << tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintp where tp << tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintp where tp << tfloatinst '50@2001-05-01';
select * from tbl_tintp where tp << tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp << tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintp where tp << tfloati '{50@2001-05-01, 70@2001-07-01}';

--overleft
select * from tbl_tintp where tp &< 50;
select * from tbl_tintp where tp &< 50.0;
select * from tbl_tintp where tp &< intrange '[40,60]';
select * from tbl_tintp where tp &< floatrange '[40,60]';
select * from tbl_tintp where tp &< box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintp where tp &< tintinst '50@2001-05-01';
select * from tbl_tintp where tp &< tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp &< tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintp where tp &< tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintp where tp &< tfloatinst '50@2001-05-01';
select * from tbl_tintp where tp &< tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp &< tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintp where tp &< tfloati '{50@2001-05-01, 70@2001-07-01}';

--right
select * from tbl_tintp where tp >> 50;
select * from tbl_tintp where tp >> 50.0;
select * from tbl_tintp where tp >> intrange '[40,60]';
select * from tbl_tintp where tp >> floatrange '[40,60]';
select * from tbl_tintp where tp >> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintp where tp >> tintinst '50@2001-05-01';
select * from tbl_tintp where tp >> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp >> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintp where tp >> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintp where tp >> tfloatinst '50@2001-05-01';
select * from tbl_tintp where tp >> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp >> tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintp where tp >> tfloati '{50@2001-05-01, 70@2001-07-01}';

--overright
select * from tbl_tintp where tp &> 50;
select * from tbl_tintp where tp &> 50.0;
select * from tbl_tintp where tp &> intrange '[40,60]';
select * from tbl_tintp where tp &> floatrange '[40,60]';
select * from tbl_tintp where tp &> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintp where tp &> tintinst '50@2001-05-01';
select * from tbl_tintp where tp &> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp &> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintp where tp &> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tintp where tp &> tfloatinst '50@2001-05-01';
select * from tbl_tintp where tp &> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp &> tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tintp where tp &> tfloati '{50@2001-05-01, 70@2001-07-01}';

--before
select * from tbl_tintp where tp <<# timestamp '2001-05-01';
select * from tbl_tintp where tp <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tintp where tp <<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintp where tp <<# tintinst '5@2001-05-01';
select * from tbl_tintp where tp <<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp <<# tintp '{5@[2001-05-01, 2001-07-01), 6@[2001-07-01, 2001-09-01)}';
select * from tbl_tintp where tp <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tintp where tp <<# tfloatinst '3.5@2001-05-01';
select * from tbl_tintp where tp <<# tfloatper '3.5@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp <<# tfloatp '{3.5@[2001-05-01, 2001-07-01), 5.6@[2001-07-01, 2001-09-01)}';
select * from tbl_tintp where tp <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';

--overbefore
select * from tbl_tintp where tp &<# timestamp '2001-07-01';
select * from tbl_tintp where tp &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tintp where tp &<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintp where tp &<# tintinst '5@2001-07-01';
select * from tbl_tintp where tp &<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp &<# tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintp where tp &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tintp where tp &<# tfloatinst '4.5@2001-07-01';
select * from tbl_tintp where tp &<# tfloatper '4.5@[2001-05-01, 2001-07-01)';
select * from tbl_tintp where tp &<# tfloatp '{4.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintp where tp &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';

--after
select * from tbl_tintp where tp #>> timestamp '2001-05-01';
select * from tbl_tintp where tp #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tintp where tp #>> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintp where tp #>> tintinst '5@2001-05-01';
select * from tbl_tintp where tp #>> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tintp where tp #>> tintp '{5@[2001-01-01, 2001-03-01), 6@[2001-03-01, 2001-05-01)}';
select * from tbl_tintp where tp #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tintp where tp #>> tfloatinst '3.5@2001-05-01';
select * from tbl_tintp where tp #>> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tintp where tp #>> tfloatp '{3.5@[2001-01-01, 2001-03-01), 5.6@[2001-03-01, 2001-05-01)}';
select * from tbl_tintp where tp #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

--overafter
select * from tbl_tintp where tp #&> timestamp '2001-03-01';
select * from tbl_tintp where tp #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tintp where tp #&> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tintp where tp #&> tintinst '5@2001-03-01';
select * from tbl_tintp where tp #&> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tintp where tp #&> tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintp where tp #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tintp where tp #&> tfloatinst '3.5@2001-03-01';
select * from tbl_tintp where tp #&> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tintp where tp #&> tfloatp '{3.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tintp where tp #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

/* tinti */
--left
select * from tbl_tinti where ti << 50;
select * from tbl_tinti where ti << 50.0;
select * from tbl_tinti where ti << intrange '[40,60]';
select * from tbl_tinti where ti << floatrange '[40,60]';
select * from tbl_tinti where ti << tintinst '50@2001-05-01';
select * from tbl_tinti where ti << tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti << tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tinti where ti << tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tinti where ti << tfloatinst '50@2001-05-01';
select * from tbl_tinti where ti << tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti << tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tinti where ti << tfloati '{50@2001-05-01, 70@2001-07-01}';

--overleft
select * from tbl_tinti where ti &< 50;
select * from tbl_tinti where ti &< 50.0;
select * from tbl_tinti where ti &< intrange '[40,60]';
select * from tbl_tinti where ti &< floatrange '[40,60]';
select * from tbl_tinti where ti &< tintinst '70@2001-05-01';
select * from tbl_tinti where ti &< tintper '70@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti &< tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tinti where ti &< tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tinti where ti &< tfloatinst '50@2001-05-01';
select * from tbl_tinti where ti &< tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti &< tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tinti where ti &< tfloati '{50@2001-05-01, 70@2001-07-01}';

--right
select * from tbl_tinti where ti >> 50;
select * from tbl_tinti where ti >> 50.0;
select * from tbl_tinti where ti >> intrange '[40,60]';
select * from tbl_tinti where ti >> floatrange '[40,60]';
select * from tbl_tinti where ti >> tintinst '50@2001-05-01';
select * from tbl_tinti where ti >> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti >> tintp '{30@[2001-05-01, 2001-07-01), 50@[2001-07-01, 2001-09-01)}';
select * from tbl_tinti where ti >> tinti '{30@2001-05-01, 50@2001-07-01}';
select * from tbl_tinti where ti >> tfloatinst '50@2001-05-01';
select * from tbl_tinti where ti >> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti >> tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tinti where ti >> tfloati '{50@2001-05-01, 70@2001-07-01}';

--overright
select * from tbl_tinti where ti &> 50;
select * from tbl_tinti where ti &> 50.0;
select * from tbl_tinti where ti &> intrange '[40,60]';
select * from tbl_tinti where ti &> floatrange '[40,60]';
select * from tbl_tinti where ti &> tintinst '30@2001-05-01';
select * from tbl_tinti where ti &> tintper '30@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti &> tintp '{30@[2001-05-01, 2001-07-01), 50@[2001-07-01, 2001-09-01)}';
select * from tbl_tinti where ti &> tinti '{30@2001-05-01, 50@2001-07-01}';
select * from tbl_tinti where ti &> tfloatinst '50@2001-05-01';
select * from tbl_tinti where ti &> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti &> tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tinti where ti &> tfloati '{50@2001-05-01, 70@2001-07-01}';

--before
select * from tbl_tinti where ti <<# timestamp '2001-05-01';
select * from tbl_tinti where ti <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tinti where ti <<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tinti where ti <<# tintinst '5@2001-05-01';
select * from tbl_tinti where ti <<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti <<# tintp '{5@[2001-05-01, 2001-07-01), 6@[2001-07-01, 2001-09-01)}';
select * from tbl_tinti where ti <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tinti where ti <<# tfloatinst '3.5@2001-05-01';
select * from tbl_tinti where ti <<# tfloatper '3.5@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti <<# tfloatp '{3.5@[2001-05-01, 2001-07-01), 5.6@[2001-07-01, 2001-09-01)}';
select * from tbl_tinti where ti <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';

--overbefore
select * from tbl_tinti where ti &<# timestamp '2001-07-01';
select * from tbl_tinti where ti &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tinti where ti &<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tinti where ti &<# tintinst '5@2001-07-01';
select * from tbl_tinti where ti &<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti &<# tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tinti where ti &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tinti where ti &<# tfloatinst '4.5@2001-07-01';
select * from tbl_tinti where ti &<# tfloatper '4.5@[2001-05-01, 2001-07-01)';
select * from tbl_tinti where ti &<# tfloatp '{4.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tinti where ti &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';

--after
select * from tbl_tinti where ti #>> timestamp '2001-05-01';
select * from tbl_tinti where ti #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tinti where ti #>> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tinti where ti #>> tintinst '5@2001-05-01';
select * from tbl_tinti where ti #>> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tinti where ti #>> tintp '{5@[2001-01-01, 2001-03-01), 6@[2001-03-01, 2001-05-01)}';
select * from tbl_tinti where ti #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tinti where ti #>> tfloatinst '3.5@2001-05-01';
select * from tbl_tinti where ti #>> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tinti where ti #>> tfloatp '{3.5@[2001-01-01, 2001-03-01), 5.6@[2001-03-01, 2001-05-01)}';
select * from tbl_tinti where ti #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

--overafter
select * from tbl_tinti where ti #&> timestamp '2001-03-01';
select * from tbl_tinti where ti #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tinti where ti #&> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tinti where ti #&> tintinst '5@2001-03-01';
select * from tbl_tinti where ti #&> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tinti where ti #&> tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tinti where ti #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tinti where ti #&> tfloatinst '3.5@2001-03-01';
select * from tbl_tinti where ti #&> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tinti where ti #&> tfloatp '{3.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tinti where ti #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

/******************************************************************************/

/* tfloatinst */
--left
select * from tbl_tfloatinst where inst << 50;
select * from tbl_tfloatinst where inst << 50.0;
select * from tbl_tfloatinst where inst << intrange '[40,60]';
select * from tbl_tfloatinst where inst << floatrange '[40,60]';
select * from tbl_tfloatinst where inst << tintinst '50@2001-05-01';
select * from tbl_tfloatinst where inst << tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst << tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatinst where inst << tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatinst where inst << tfloatinst '50@2001-05-01';
select * from tbl_tfloatinst where inst << tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst << tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatinst where inst << tfloati '{50@2001-05-01, 70@2001-07-01}';

--overleft
select * from tbl_tfloatinst where inst &< 50;
select * from tbl_tfloatinst where inst &< 50.0;
select * from tbl_tfloatinst where inst &< intrange '[40,60]';
select * from tbl_tfloatinst where inst &< floatrange '[40,60]';
select * from tbl_tfloatinst where inst &< tintinst '50@2001-05-01';
select * from tbl_tfloatinst where inst &< tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst &< tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatinst where inst &< tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatinst where inst &< tfloatinst '70@2001-05-01';
select * from tbl_tfloatinst where inst &< tfloatper '70@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst &< tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatinst where inst &< tfloati '{50@2001-05-01, 70@2001-07-01}';

--right
select * from tbl_tfloatinst where inst >> 50;
select * from tbl_tfloatinst where inst >> 50.0;
select * from tbl_tfloatinst where inst >> intrange '[40,60]';
select * from tbl_tfloatinst where inst >> floatrange '[40,60]';
select * from tbl_tfloatinst where inst >> tintinst '50@2001-05-01';
select * from tbl_tfloatinst where inst >> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst >> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatinst where inst >> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatinst where inst >> tfloatinst '50@2001-05-01';
select * from tbl_tfloatinst where inst >> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst >> tfloatp '{30@[2001-05-01, 2001-07-01), 50@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatinst where inst >> tfloati '{30@2001-05-01, 50@2001-07-01}';

--overright
select * from tbl_tfloatinst where inst &> 50;
select * from tbl_tfloatinst where inst &> 50.0;
select * from tbl_tfloatinst where inst &> intrange '[40,60]';
select * from tbl_tfloatinst where inst &> floatrange '[40,60]';
select * from tbl_tfloatinst where inst &> tintinst '50@2001-05-01';
select * from tbl_tfloatinst where inst &> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst &> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatinst where inst &> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatinst where inst &> tfloatinst '30@2001-05-01';
select * from tbl_tfloatinst where inst &> tfloatper '30@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst &> tfloatp '{30@[2001-05-01, 2001-07-01), 50@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatinst where inst &> tfloati '{30@2001-05-01, 50@2001-07-01}';

--before
select * from tbl_tfloatinst where inst <<# timestamp '2001-05-01';
select * from tbl_tfloatinst where inst <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatinst where inst <<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatinst where inst <<# tintinst '5@2001-05-01';
select * from tbl_tfloatinst where inst <<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst <<# tintp '{5@[2001-05-01, 2001-07-01), 6@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatinst where inst <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tfloatinst where inst <<# tfloatinst '3.5@2001-05-01';
select * from tbl_tfloatinst where inst <<# tfloatper '3.5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst <<# tfloatp '{3.5@[2001-05-01, 2001-07-01), 5.6@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatinst where inst <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';

--overbefore
select * from tbl_tfloatinst where inst &<# timestamp '2001-07-01';
select * from tbl_tfloatinst where inst &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatinst where inst &<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatinst where inst &<# tintinst '5@2001-07-01';
select * from tbl_tfloatinst where inst &<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst &<# tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatinst where inst &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tfloatinst where inst &<# tfloatinst '4.5@2001-07-01';
select * from tbl_tfloatinst where inst &<# tfloatper '4.5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatinst where inst &<# tfloatp '{4.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatinst where inst &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';

--after
select * from tbl_tfloatinst where inst #>> timestamp '2001-05-01';
select * from tbl_tfloatinst where inst #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatinst where inst #>> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatinst where inst #>> tintinst '5@2001-05-01';
select * from tbl_tfloatinst where inst #>> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatinst where inst #>> tintp '{5@[2001-01-01, 2001-03-01), 6@[2001-03-01, 2001-05-01)}';
select * from tbl_tfloatinst where inst #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tfloatinst where inst #>> tfloatinst '3.5@2001-05-01';
select * from tbl_tfloatinst where inst #>> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatinst where inst #>> tfloatp '{3.5@[2001-01-01, 2001-03-01), 5.6@[2001-03-01, 2001-05-01)}';
select * from tbl_tfloatinst where inst #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

--overafter
select * from tbl_tfloatinst where inst #&> timestamp '2001-03-01';
select * from tbl_tfloatinst where inst #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatinst where inst #&> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatinst where inst #&> tintinst '5@2001-03-01';
select * from tbl_tfloatinst where inst #&> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatinst where inst #&> tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatinst where inst #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tfloatinst where inst #&> tfloatinst '3.5@2001-03-01';
select * from tbl_tfloatinst where inst #&> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatinst where inst #&> tfloatp '{3.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatinst where inst #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

/* tfloatper */
--left
select * from tbl_tfloatper where per << 50;
select * from tbl_tfloatper where per << 50.0;
select * from tbl_tfloatper where per << intrange '[40,60]';
select * from tbl_tfloatper where per << floatrange '[40,60]';
select * from tbl_tfloatper where per << tintinst '70@2001-05-01';
select * from tbl_tfloatper where per << tintper '70@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per << tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatper where per << tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatper where per << tfloatinst '50@2001-05-01';
select * from tbl_tfloatper where per << tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per << tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatper where per << tfloati '{50@2001-05-01, 70@2001-07-01}';

--overleft
select * from tbl_tfloatper where per &< 50;
select * from tbl_tfloatper where per &< 50.0;
select * from tbl_tfloatper where per &< intrange '[40,60]';
select * from tbl_tfloatper where per &< floatrange '[40,60]';
select * from tbl_tfloatper where per &< tintinst '70@2001-05-01';
select * from tbl_tfloatper where per &< tintper '70@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per &< tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatper where per &< tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatper where per &< tfloatinst '70@2001-05-01';
select * from tbl_tfloatper where per &< tfloatper '70@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per &< tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatper where per &< tfloati '{50@2001-05-01, 70@2001-07-01}';

--right
select * from tbl_tfloatper where per >> 50;
select * from tbl_tfloatper where per >> 50.0;
select * from tbl_tfloatper where per >> intrange '[40,60]';
select * from tbl_tfloatper where per >> floatrange '[40,60]';
select * from tbl_tfloatper where per >> tintinst '70@2001-05-01';
select * from tbl_tfloatper where per >> tintper '70@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per >> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatper where per >> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatper where per >> tfloatinst '50@2001-05-01';
select * from tbl_tfloatper where per >> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per >> tfloatp '{30@[2001-05-01, 2001-07-01), 50@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatper where per >> tfloati '{30@2001-05-01, 50@2001-07-01}';

--overright
select * from tbl_tfloatper where per &> 50;
select * from tbl_tfloatper where per &> 50.0;
select * from tbl_tfloatper where per &> intrange '[40,60]';
select * from tbl_tfloatper where per &> floatrange '[40,60]';
select * from tbl_tfloatper where per &> tintinst '70@2001-05-01';
select * from tbl_tfloatper where per &> tintper '70@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per &> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatper where per &> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatper where per &> tfloatinst '30@2001-05-01';
select * from tbl_tfloatper where per &> tfloatper '30@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per &> tfloatp '{30@[2001-05-01, 2001-07-01), 50@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatper where per &> tfloati '{30@2001-05-01, 50@2001-07-01}';

--before
select * from tbl_tfloatper where per <<# timestamp '2001-05-01';
select * from tbl_tfloatper where per <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatper where per <<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatper where per <<# tintinst '5@2001-05-01';
select * from tbl_tfloatper where per <<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per <<# tintp '{5@[2001-05-01, 2001-07-01), 6@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatper where per <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tfloatper where per <<# tfloatinst '3.5@2001-05-01';
select * from tbl_tfloatper where per <<# tfloatper '3.5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per <<# tfloatp '{3.5@[2001-05-01, 2001-07-01), 5.6@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatper where per <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';

--overbefore
select * from tbl_tfloatper where per &<# timestamp '2001-07-01';
select * from tbl_tfloatper where per &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatper where per &<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatper where per &<# tintinst '5@2001-07-01';
select * from tbl_tfloatper where per &<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per &<# tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatper where per &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tfloatper where per &<# tfloatinst '4.5@2001-07-01';
select * from tbl_tfloatper where per &<# tfloatper '4.5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatper where per &<# tfloatp '{4.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatper where per &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';

--after
select * from tbl_tfloatper where per #>> timestamp '2001-05-01';
select * from tbl_tfloatper where per #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatper where per #>> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatper where per #>> tintinst '5@2001-05-01';
select * from tbl_tfloatper where per #>> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatper where per #>> tintp '{5@[2001-01-01, 2001-03-01), 6@[2001-03-01, 2001-05-01)}';
select * from tbl_tfloatper where per #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tfloatper where per #>> tfloatinst '3.5@2001-05-01';
select * from tbl_tfloatper where per #>> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatper where per #>> tfloatp '{3.5@[2001-01-01, 2001-03-01), 5.6@[2001-03-01, 2001-05-01)}';
select * from tbl_tfloatper where per #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

--overafter
select * from tbl_tfloatper where per #&> timestamp '2001-03-01';
select * from tbl_tfloatper where per #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatper where per #&> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatper where per #&> tintinst '5@2001-03-01';
select * from tbl_tfloatper where per #&> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatper where per #&> tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatper where per #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tfloatper where per #&> tfloatinst '3.5@2001-03-01';
select * from tbl_tfloatper where per #&> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatper where per #&> tfloatp '{3.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatper where per #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

/* tfloatp */
--left
select * from tbl_tfloatp where tp << 50;
select * from tbl_tfloatp where tp << 50.0;
select * from tbl_tfloatp where tp << intrange '[40,60]';
select * from tbl_tfloatp where tp << floatrange '[40,60]';
select * from tbl_tfloatp where tp << tintinst '50@2001-05-01';
select * from tbl_tfloatp where tp << tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp << tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatp where tp << tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatp where tp << tfloatinst '50@2001-05-01';
select * from tbl_tfloatp where tp << tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp << tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatp where tp << tfloati '{50@2001-05-01, 70@2001-07-01}';

--overleft
select * from tbl_tfloatp where tp &< 50;
select * from tbl_tfloatp where tp &< 50.0;
select * from tbl_tfloatp where tp &< intrange '[40,60]';
select * from tbl_tfloatp where tp &< floatrange '[40,60]';
select * from tbl_tfloatp where tp &< tintinst '50@2001-05-01';
select * from tbl_tfloatp where tp &< tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp &< tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatp where tp &< tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatp where tp &< tfloatinst '70@2001-05-01';
select * from tbl_tfloatp where tp &< tfloatper '70@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp &< tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatp where tp &< tfloati '{50@2001-05-01, 70@2001-07-01}';

--right
select * from tbl_tfloatp where tp >> 50;
select * from tbl_tfloatp where tp >> 50.0;
select * from tbl_tfloatp where tp >> intrange '[40,60]';
select * from tbl_tfloatp where tp >> floatrange '[40,60]';
select * from tbl_tfloatp where tp >> tintinst '50@2001-05-01';
select * from tbl_tfloatp where tp >> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp >> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatp where tp >> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatp where tp >> tfloatinst '50@2001-05-01';
select * from tbl_tfloatp where tp >> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp >> tfloatp '{30@[2001-05-01, 2001-07-01), 50@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatp where tp >> tfloati '{30@2001-05-01, 50@2001-07-01}';

--overright
select * from tbl_tfloatp where tp &> 50;
select * from tbl_tfloatp where tp &> 50.0;
select * from tbl_tfloatp where tp &> intrange '[40,60]';
select * from tbl_tfloatp where tp &> floatrange '[40,60]';
select * from tbl_tfloatp where tp &> tintinst '50@2001-05-01';
select * from tbl_tfloatp where tp &> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp &> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatp where tp &> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloatp where tp &> tfloatinst '30@2001-05-01';
select * from tbl_tfloatp where tp &> tfloatper '30@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp &> tfloatp '{30@[2001-05-01, 2001-07-01), 50@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatp where tp &> tfloati '{30@2001-05-01, 50@2001-07-01}';

--before
select * from tbl_tfloatp where tp <<# timestamp '2001-05-01';
select * from tbl_tfloatp where tp <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatp where tp <<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatp where tp <<# tintinst '5@2001-05-01';
select * from tbl_tfloatp where tp <<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp <<# tintp '{5@[2001-05-01, 2001-07-01), 6@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatp where tp <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tfloatp where tp <<# tfloatinst '3.5@2001-05-01';
select * from tbl_tfloatp where tp <<# tfloatper '3.5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp <<# tfloatp '{3.5@[2001-05-01, 2001-07-01), 5.6@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloatp where tp <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';

--overbefore
select * from tbl_tfloatp where tp &<# timestamp '2001-07-01';
select * from tbl_tfloatp where tp &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatp where tp &<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatp where tp &<# tintinst '5@2001-07-01';
select * from tbl_tfloatp where tp &<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp &<# tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatp where tp &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tfloatp where tp &<# tfloatinst '4.5@2001-07-01';
select * from tbl_tfloatp where tp &<# tfloatper '4.5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloatp where tp &<# tfloatp '{4.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatp where tp &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';

--after
select * from tbl_tfloatp where tp #>> timestamp '2001-05-01';
select * from tbl_tfloatp where tp #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatp where tp #>> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatp where tp #>> tintinst '5@2001-05-01';
select * from tbl_tfloatp where tp #>> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatp where tp #>> tintp '{5@[2001-01-01, 2001-03-01), 6@[2001-03-01, 2001-05-01)}';
select * from tbl_tfloatp where tp #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tfloatp where tp #>> tfloatinst '3.5@2001-05-01';
select * from tbl_tfloatp where tp #>> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatp where tp #>> tfloatp '{3.5@[2001-01-01, 2001-03-01), 5.6@[2001-03-01, 2001-05-01)}';
select * from tbl_tfloatp where tp #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

--overafter
select * from tbl_tfloatp where tp #&> timestamp '2001-03-01';
select * from tbl_tfloatp where tp #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloatp where tp #&> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloatp where tp #&> tintinst '5@2001-03-01';
select * from tbl_tfloatp where tp #&> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatp where tp #&> tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatp where tp #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tfloatp where tp #&> tfloatinst '3.5@2001-03-01';
select * from tbl_tfloatp where tp #&> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloatp where tp #&> tfloatp '{3.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloatp where tp #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

/* tfloati */
--left
select * from tbl_tfloati where ti << 50;
select * from tbl_tfloati where ti << 50.0;
select * from tbl_tfloati where ti << intrange '[40,60]';
select * from tbl_tfloati where ti << floatrange '[40,60]';
select * from tbl_tfloati where ti << tintinst '50@2001-05-01';
select * from tbl_tfloati where ti << tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti << tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloati where ti << tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloati where ti << tfloatinst '50@2001-05-01';
select * from tbl_tfloati where ti << tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti << tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloati where ti << tfloati '{50@2001-05-01, 70@2001-07-01}';

--overleft
select * from tbl_tfloati where ti &< 50;
select * from tbl_tfloati where ti &< 50.0;
select * from tbl_tfloati where ti &< intrange '[40,60]';
select * from tbl_tfloati where ti &< floatrange '[40,60]';
select * from tbl_tfloati where ti &< tintinst '50@2001-05-01';
select * from tbl_tfloati where ti &< tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti &< tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloati where ti &< tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloati where ti &< tfloatinst '70@2001-05-01';
select * from tbl_tfloati where ti &< tfloatper '70@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti &< tfloatp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloati where ti &< tfloati '{50@2001-05-01, 70@2001-07-01}';

--right
select * from tbl_tfloati where ti >> 50;
select * from tbl_tfloati where ti >> 50.0;
select * from tbl_tfloati where ti >> intrange '[40,60]';
select * from tbl_tfloati where ti >> floatrange '[40,60]';
select * from tbl_tfloati where ti >> tintinst '50@2001-05-01';
select * from tbl_tfloati where ti >> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti >> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloati where ti >> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloati where ti >> tfloatinst '50@2001-05-01';
select * from tbl_tfloati where ti >> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti >> tfloatp '{30@[2001-05-01, 2001-07-01), 50@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloati where ti >> tfloati '{30@2001-05-01, 50@2001-07-01}';

--overright
select * from tbl_tfloati where ti &> 50;
select * from tbl_tfloati where ti &> 50.0;
select * from tbl_tfloati where ti &> intrange '[40,60]';
select * from tbl_tfloati where ti &> floatrange '[40,60]';
select * from tbl_tfloati where ti &> tintinst '50@2001-05-01';
select * from tbl_tfloati where ti &> tintper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti &> tintp '{50@[2001-05-01, 2001-07-01), 70@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloati where ti &> tinti '{50@2001-05-01, 70@2001-07-01}';
select * from tbl_tfloati where ti &> tfloatinst '50@2001-05-01';
select * from tbl_tfloati where ti &> tfloatper '50@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti &> tfloatp '{30@[2001-05-01, 2001-07-01), 50@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloati where ti &> tfloati '{30@2001-05-01, 50@2001-07-01}';

--before
select * from tbl_tfloati where ti <<# timestamp '2001-05-01';
select * from tbl_tfloati where ti <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloati where ti <<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloati where ti <<# tintinst '5@2001-05-01';
select * from tbl_tfloati where ti <<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti <<# tintp '{5@[2001-05-01, 2001-07-01), 6@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloati where ti <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tfloati where ti <<# tfloatinst '3.5@2001-05-01';
select * from tbl_tfloati where ti <<# tfloatper '3.5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti <<# tfloatp '{3.5@[2001-05-01, 2001-07-01), 5.6@[2001-07-01, 2001-09-01)}';
select * from tbl_tfloati where ti <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';

--overbefore
select * from tbl_tfloati where ti &<# timestamp '2001-07-01';
select * from tbl_tfloati where ti &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloati where ti &<# box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloati where ti &<# tintinst '5@2001-07-01';
select * from tbl_tfloati where ti &<# tintper '5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti &<# tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloati where ti &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select * from tbl_tfloati where ti &<# tfloatinst '4.5@2001-07-01';
select * from tbl_tfloati where ti &<# tfloatper '4.5@[2001-05-01, 2001-07-01)';
select * from tbl_tfloati where ti &<# tfloatp '{4.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloati where ti &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';

--after
select * from tbl_tfloati where ti #>> timestamp '2001-05-01';
select * from tbl_tfloati where ti #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloati where ti #>> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloati where ti #>> tintinst '5@2001-05-01';
select * from tbl_tfloati where ti #>> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloati where ti #>> tintp '{5@[2001-01-01, 2001-03-01), 6@[2001-03-01, 2001-05-01)}';
select * from tbl_tfloati where ti #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tfloati where ti #>> tfloatinst '3.5@2001-05-01';
select * from tbl_tfloati where ti #>> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloati where ti #>> tfloatp '{3.5@[2001-01-01, 2001-03-01), 5.6@[2001-03-01, 2001-05-01)}';
select * from tbl_tfloati where ti #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

--overafter
select * from tbl_tfloati where ti #&> timestamp '2001-03-01';
select * from tbl_tfloati where ti #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_tfloati where ti #&> box '(60,47260800000000),(40,41990400000000)';
select * from tbl_tfloati where ti #&> tintinst '5@2001-03-01';
select * from tbl_tfloati where ti #&> tintper '5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloati where ti #&> tintp '{5@[2001-03-01, 2001-05-01), 6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloati where ti #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select * from tbl_tfloati where ti #&> tfloatinst '3.5@2001-03-01';
select * from tbl_tfloati where ti #&> tfloatper '3.5@[2001-03-01, 2001-05-01)';
select * from tbl_tfloati where ti #&> tfloatp '{3.5@[2001-03-01, 2001-05-01), 5.6@[2001-05-01, 2001-07-01)}';
select * from tbl_tfloati where ti #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';

/******************************************************************************/

/* ttextinst */
--before
select * from tbl_ttextinst where inst <<# timestamp '2001-05-01';
select * from tbl_ttextinst where inst <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextinst where inst <<# ttextinst 'AAA@2001-05-01';
select * from tbl_ttextinst where inst <<# ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextinst where inst <<# ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextinst where inst <<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--overbefore
select * from tbl_ttextinst where inst &<# timestamp '2001-07-01';
select * from tbl_ttextinst where inst &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextinst where inst &<# ttextinst 'AAA@2001-05-01';
select * from tbl_ttextinst where inst &<# ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextinst where inst &<# ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextinst where inst &<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--after
select * from tbl_ttextinst where inst #>> timestamp '2001-05-01';
select * from tbl_ttextinst where inst #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextinst where inst #>> ttextinst 'AAA@2001-05-01';
select * from tbl_ttextinst where inst #>> ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextinst where inst #>> ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextinst where inst #>> ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--overafter
select * from tbl_ttextinst where inst #&> timestamp '2001-03-01';
select * from tbl_ttextinst where inst #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextinst where inst #&> ttextinst 'AAA@2001-05-01';
select * from tbl_ttextinst where inst #&> ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextinst where inst #&> ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextinst where inst #&> ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

/* ttextper */
--before
select * from tbl_ttextper where per <<# timestamp '2001-05-01';
select * from tbl_ttextper where per <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextper where per <<# ttextinst 'AAA@2001-05-01';
select * from tbl_ttextper where per <<# ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextper where per <<# ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextper where per <<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--overbefore
select * from tbl_ttextper where per &<# timestamp '2001-07-01';
select * from tbl_ttextper where per &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextper where per &<# ttextinst 'AAA@2001-05-01';
select * from tbl_ttextper where per &<# ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextper where per &<# ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextper where per &<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--after
select * from tbl_ttextper where per #>> timestamp '2001-05-01';
select * from tbl_ttextper where per #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextper where per #>> ttextinst 'AAA@2001-05-01';
select * from tbl_ttextper where per #>> ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextper where per #>> ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextper where per #>> ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--overafter
select * from tbl_ttextper where per #&> timestamp '2001-03-01';
select * from tbl_ttextper where per #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextper where per #&> ttextinst 'AAA@2001-05-01';
select * from tbl_ttextper where per #&> ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextper where per #&> ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextper where per #&> ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

/* ttextp */
--before
select * from tbl_ttextp where tp <<# timestamp '2001-05-01';
select * from tbl_ttextp where tp <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextp where tp <<# ttextinst 'AAA@2001-05-01';
select * from tbl_ttextp where tp <<# ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextp where tp <<# ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextp where tp <<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--overbefore
select * from tbl_ttextp where tp &<# timestamp '2001-07-01';
select * from tbl_ttextp where tp &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextp where tp &<# ttextinst 'AAA@2001-05-01';
select * from tbl_ttextp where tp &<# ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextp where tp &<# ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextp where tp &<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--after
select * from tbl_ttextp where tp #>> timestamp '2001-05-01';
select * from tbl_ttextp where tp #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextp where tp #>> ttextinst 'AAA@2001-05-01';
select * from tbl_ttextp where tp #>> ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextp where tp #>> ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextp where tp #>> ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--overafter
select * from tbl_ttextp where tp #&> timestamp '2001-03-01';
select * from tbl_ttextp where tp #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_ttextp where tp #&> ttextinst 'AAA@2001-05-01';
select * from tbl_ttextp where tp #&> ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttextp where tp #&> ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttextp where tp #&> ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

/* ttexti */
--before
select * from tbl_ttexti where ti <<# timestamp '2001-05-01';
select * from tbl_ttexti where ti <<# period '[2001-03-01, 2001-06-01]';
select * from tbl_ttexti where ti <<# ttextinst 'AAA@2001-05-01';
select * from tbl_ttexti where ti <<# ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttexti where ti <<# ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttexti where ti <<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--overbefore
select * from tbl_ttexti where ti &<# timestamp '2001-07-01';
select * from tbl_ttexti where ti &<# period '[2001-03-01, 2001-06-01]';
select * from tbl_ttexti where ti &<# ttextinst 'AAA@2001-05-01';
select * from tbl_ttexti where ti &<# ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttexti where ti &<# ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttexti where ti &<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--after
select * from tbl_ttexti where ti #>> timestamp '2001-05-01';
select * from tbl_ttexti where ti #>> period '[2001-03-01, 2001-06-01]';
select * from tbl_ttexti where ti #>> ttextinst 'AAA@2001-05-01';
select * from tbl_ttexti where ti #>> ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttexti where ti #>> ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttexti where ti #>> ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

--overafter
select * from tbl_ttexti where ti #&> timestamp '2001-03-01';
select * from tbl_ttexti where ti #&> period '[2001-03-01, 2001-06-01]';
select * from tbl_ttexti where ti #&> ttextinst 'AAA@2001-05-01';
select * from tbl_ttexti where ti #&> ttextper 'AAA@[2001-05-01, 2001-07-01)';
select * from tbl_ttexti where ti #&> ttextp '{AAA@[2001-05-01, 2001-07-01), BBB@[2001-07-01, 2001-09-01)}';
select * from tbl_ttexti where ti #&> ttexti '{AAA@2001-05-01, BBB@2001-07-01}';

/******************************************************************************/

-- create gist index and test again

-- create spgist index and test again

/******************************************************************************/

