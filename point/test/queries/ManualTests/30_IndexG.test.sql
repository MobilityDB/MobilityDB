/******************************************************************************/

--before
select count(*) from tbl_tgeogpointinst where inst <<# timestamptz '2001-05-01';
select count(*) from tbl_tgeogpointinst where inst <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointinst where inst <<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointinst where inst <<# tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointinst where inst <<# tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointinst where inst <<# tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointinst where inst <<# tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbefore
select count(*) from tbl_tgeogpointinst where inst &<# timestamptz '2001-07-01';
select count(*) from tbl_tgeogpointinst where inst &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointinst where inst &<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointinst where inst &<# tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointinst where inst &<# tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointinst where inst &<# tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointinst where inst &<# tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--after
select count(*) from tbl_tgeogpointinst where inst #>> timestamptz '2001-05-01';
select count(*) from tbl_tgeogpointinst where inst #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointinst where inst #>> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointinst where inst #>> tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointinst where inst #>> tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointinst where inst #>> tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointinst where inst #>> tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overafter
select count(*) from tbl_tgeogpointinst where inst #&> timestamptz '2001-03-01';
select count(*) from tbl_tgeogpointinst where inst #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointinst where inst #&> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointinst where inst #&> tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointinst where inst #&> tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointinst where inst #&> tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointinst where inst #&> tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

/******************************************************************************/
/* tgeogpointper */

--before
select count(*) from tbl_tgeogpointper where per <<# timestamptz '2001-05-01';
select count(*) from tbl_tgeogpointper where per <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointper where per <<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointper where per <<# tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointper where per <<# tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointper where per <<# tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointper where per <<# tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbefore
select count(*) from tbl_tgeogpointper where per &<# timestamptz '2001-07-01';
select count(*) from tbl_tgeogpointper where per &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointper where per &<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointper where per &<# tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointper where per &<# tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointper where per &<# tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointper where per &<# tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--after
select count(*) from tbl_tgeogpointper where per #>> timestamptz '2001-05-01';
select count(*) from tbl_tgeogpointper where per #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointper where per #>> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointper where per #>> tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointper where per #>> tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointper where per #>> tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointper where per #>> tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overafter
select count(*) from tbl_tgeogpointper where per #&> timestamptz '2001-03-01';
select count(*) from tbl_tgeogpointper where per #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointper where per #&> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointper where per #&> tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointper where per #&> tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointper where per #&> tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointper where per #&> tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

/******************************************************************************/
/* tgeogpointp */

--before
select count(*) from tbl_tgeogpointp where tp <<# timestamptz '2001-05-01';
select count(*) from tbl_tgeogpointp where tp <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointp where tp <<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointp where tp <<# tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointp where tp <<# tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointp where tp <<# tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointp where tp <<# tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbefore
select count(*) from tbl_tgeogpointp where tp &<# timestamptz '2001-07-01';
select count(*) from tbl_tgeogpointp where tp &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointp where tp &<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointp where tp &<# tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointp where tp &<# tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointp where tp &<# tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointp where tp &<# tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--after
select count(*) from tbl_tgeogpointp where tp #>> timestamptz '2001-05-01';
select count(*) from tbl_tgeogpointp where tp #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointp where tp #>> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointp where tp #>> tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointp where tp #>> tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointp where tp #>> tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointp where tp #>> tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overafter
select count(*) from tbl_tgeogpointp where tp #&> timestamptz '2001-03-01';
select count(*) from tbl_tgeogpointp where tp #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointp where tp #&> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointp where tp #&> tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointp where tp #&> tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointp where tp #&> tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointp where tp #&> tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

/******************************************************************************/
/* tgeogpointi */

--before
select count(*) from tbl_tgeogpointi where ti <<# timestamptz '2001-05-01';
select count(*) from tbl_tgeogpointi where ti <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointi where ti <<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointi where ti <<# tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointi where ti <<# tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointi where ti <<# tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointi where ti <<# tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbefore
select count(*) from tbl_tgeogpointi where ti &<# timestamptz '2001-07-01';
select count(*) from tbl_tgeogpointi where ti &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointi where ti &<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointi where ti &<# tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointi where ti &<# tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointi where ti &<# tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointi where ti &<# tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--after
select count(*) from tbl_tgeogpointi where ti #>> timestamptz '2001-05-01';
select count(*) from tbl_tgeogpointi where ti #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointi where ti #>> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointi where ti #>> tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointi where ti #>> tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointi where ti #>> tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointi where ti #>> tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overafter
select count(*) from tbl_tgeogpointi where ti #&> timestamptz '2001-03-01';
select count(*) from tbl_tgeogpointi where ti #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeogpointi where ti #&> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeogpointi where ti #&> tgeogpointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeogpointi where ti #&> tgeogpointper 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeogpointi where ti #&> tgeogpointp '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeogpointi where ti #&> tgeogpointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

/******************************************************************************/
