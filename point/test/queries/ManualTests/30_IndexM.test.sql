/******************************************************************************/

/* tgeompointinst */
--left
select count(*) from tbl_tgeompointinst where inst << geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointinst where inst << gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst << tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst << tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst << tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst << tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overleft
select count(*) from tbl_tgeompointinst where inst &< geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointinst where inst &< gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst &< tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst &< tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst &< tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst &< tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--right
select count(*) from tbl_tgeompointinst where inst >> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointinst where inst >> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst >> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst >> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst >> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst >> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overright
select count(*) from tbl_tgeompointinst where inst &> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointinst where inst &> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst &> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst &> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst &> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst &> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--below
select count(*) from tbl_tgeompointinst where inst <<| geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointinst where inst <<| gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst <<| tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst <<| tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst <<| tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst <<| tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbelow
select count(*) from tbl_tgeompointinst where inst &<| geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointinst where inst &<| gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst &<| tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst &<| tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst &<| tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst &<| tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--above
select count(*) from tbl_tgeompointinst where inst |>> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointinst where inst |>> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst |>> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst |>> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst |>> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst |>> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overabove
select count(*) from tbl_tgeompointinst where inst |&> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointinst where inst |&> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst |&> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst |&> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst |&> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst |&> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--before
select count(*) from tbl_tgeompointinst where inst <<# timestamptz '2001-05-01';
select count(*) from tbl_tgeompointinst where inst <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointinst where inst <<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst <<# tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst <<# tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst <<# tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst <<# tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbefore
select count(*) from tbl_tgeompointinst where inst &<# timestamptz '2001-07-01';
select count(*) from tbl_tgeompointinst where inst &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointinst where inst &<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst &<# tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst &<# tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst &<# tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst &<# tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--after
select count(*) from tbl_tgeompointinst where inst #>> timestamptz '2001-05-01';
select count(*) from tbl_tgeompointinst where inst #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointinst where inst #>> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst #>> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst #>> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst #>> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst #>> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overafter
select count(*) from tbl_tgeompointinst where inst #&> timestamptz '2001-03-01';
select count(*) from tbl_tgeompointinst where inst #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointinst where inst #&> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointinst where inst #&> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointinst where inst #&> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointinst where inst #&> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointinst where inst #&> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

/******************************************************************************/
/* tgeompointseq */

--left
select count(*) from tbl_tgeompointper where per << geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointper where per << gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per << tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per << tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per << tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per << tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overleft
select count(*) from tbl_tgeompointper where per &< geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointper where per &< gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per &< tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per &< tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per &< tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per &< tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--right
select count(*) from tbl_tgeompointper where per >> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointper where per >> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per >> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per >> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per >> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per >> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overright
select count(*) from tbl_tgeompointper where per &> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointper where per &> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per &> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per &> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per &> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per &> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--below
select count(*) from tbl_tgeompointper where per <<| geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointper where per <<| gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per <<| tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per <<| tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per <<| tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per <<| tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbelow
select count(*) from tbl_tgeompointper where per &<| geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointper where per &<| gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per &<| tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per &<| tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per &<| tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per &<| tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--above
select count(*) from tbl_tgeompointper where per |>> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointper where per |>> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per |>> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per |>> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per |>> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per |>> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overabove
select count(*) from tbl_tgeompointper where per |&> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointper where per |&> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per |&> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per |&> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per |&> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per |&> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--before
select count(*) from tbl_tgeompointper where per <<# timestamptz '2001-05-01';
select count(*) from tbl_tgeompointper where per <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointper where per <<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per <<# tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per <<# tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per <<# tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per <<# tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbefore
select count(*) from tbl_tgeompointper where per &<# timestamptz '2001-07-01';
select count(*) from tbl_tgeompointper where per &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointper where per &<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per &<# tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per &<# tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per &<# tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per &<# tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--after
select count(*) from tbl_tgeompointper where per #>> timestamptz '2001-05-01';
select count(*) from tbl_tgeompointper where per #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointper where per #>> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per #>> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per #>> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per #>> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per #>> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overafter
select count(*) from tbl_tgeompointper where per #&> timestamptz '2001-03-01';
select count(*) from tbl_tgeompointper where per #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointper where per #&> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointper where per #&> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointper where per #&> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointper where per #&> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointper where per #&> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

/******************************************************************************/
/* tgeompoints */

--left
select count(*) from tbl_tgeompointp where tp << geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointp where tp << gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp << tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp << tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp << tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp << tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overleft
select count(*) from tbl_tgeompointp where tp &< geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointp where tp &< gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp &< tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp &< tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp &< tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp &< tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--right
select count(*) from tbl_tgeompointp where tp >> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointp where tp >> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp >> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp >> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp >> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp >> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overright
select count(*) from tbl_tgeompointp where tp &> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointp where tp &> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp &> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp &> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp &> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp &> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--below
select count(*) from tbl_tgeompointp where tp <<| geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointp where tp <<| gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp <<| tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp <<| tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp <<| tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp <<| tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbelow
select count(*) from tbl_tgeompointp where tp &<| geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointp where tp &<| gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp &<| tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp &<| tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp &<| tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp &<| tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--above
select count(*) from tbl_tgeompointp where tp |>> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointp where tp |>> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp |>> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp |>> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp |>> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp |>> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overabove
select count(*) from tbl_tgeompointp where tp |&> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointp where tp |&> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp |&> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp |&> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp |&> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp |&> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--before
select count(*) from tbl_tgeompointp where tp <<# timestamptz '2001-05-01';
select count(*) from tbl_tgeompointp where tp <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointp where tp <<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp <<# tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp <<# tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp <<# tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp <<# tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbefore
select count(*) from tbl_tgeompointp where tp &<# timestamptz '2001-07-01';
select count(*) from tbl_tgeompointp where tp &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointp where tp &<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp &<# tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp &<# tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp &<# tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp &<# tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--after
select count(*) from tbl_tgeompointp where tp #>> timestamptz '2001-05-01';
select count(*) from tbl_tgeompointp where tp #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointp where tp #>> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp #>> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp #>> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp #>> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp #>> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overafter
select count(*) from tbl_tgeompointp where tp #&> timestamptz '2001-03-01';
select count(*) from tbl_tgeompointp where tp #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointp where tp #&> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointp where tp #&> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointp where tp #&> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointp where tp #&> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointp where tp #&> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

/******************************************************************************/
/* tgeompointi */

--left
select count(*) from tbl_tgeompointi where ti << geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointi where ti << tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointi where ti << tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti << tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti << tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overleft
select count(*) from tbl_tgeompointi where ti &< geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointi where ti &< tgeompointinst 'Point(70 70)@2001-05-01';
select count(*) from tbl_tgeompointi where ti &< tgeompointseq 'Point(70 70)->Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti &< tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti &< tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--right
select count(*) from tbl_tgeompointi where ti >> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointi where ti >> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointi where ti >> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti >> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti >> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overright
select count(*) from tbl_tgeompointi where ti &> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointi where ti &> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointi where ti &> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti &> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti &> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--below
select count(*) from tbl_tgeompointi where ti <<| geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointi where ti <<| tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointi where ti <<| tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti <<| tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti <<| tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbelow
select count(*) from tbl_tgeompointi where ti &<| geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointi where ti &<| tgeompointinst 'Point(70 70)@2001-05-01';
select count(*) from tbl_tgeompointi where ti &<| tgeompointseq 'Point(70 70)->Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti &<| tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti &<| tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--above
select count(*) from tbl_tgeompointi where ti |>> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointi where ti |>> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointi where ti |>> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti |>> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti |>> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overabove
select count(*) from tbl_tgeompointi where ti |&> geometry 'Polygon((40 40,40 60,60 60,60 40,40 40))';
select count(*) from tbl_tgeompointi where ti |&> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointi where ti |&> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti |&> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti |&> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--before
select count(*) from tbl_tgeompointi where ti <<# timestamptz '2001-05-01';
select count(*) from tbl_tgeompointi where ti <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointi where ti <<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointi where ti <<# tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointi where ti <<# tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti <<# tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti <<# tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overbefore
select count(*) from tbl_tgeompointi where ti &<# timestamptz '2001-07-01';
select count(*) from tbl_tgeompointi where ti &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointi where ti &<# gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointi where ti &<# tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointi where ti &<# tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti &<# tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti &<# tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--after
select count(*) from tbl_tgeompointi where ti #>> timestamptz '2001-05-01';
select count(*) from tbl_tgeompointi where ti #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointi where ti #>> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointi where ti #>> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointi where ti #>> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti #>> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti #>> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

--overafter
select count(*) from tbl_tgeompointi where ti #&> timestamptz '2001-03-01';
select count(*) from tbl_tgeompointi where ti #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tgeompointi where ti #&> gbox 'GBOX(60 60 47260800000000,40 40 41990400000000)';
select count(*) from tbl_tgeompointi where ti #&> tgeompointinst 'Point(50 50)@2001-05-01';
select count(*) from tbl_tgeompointi where ti #&> tgeompointseq 'Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01)';
select count(*) from tbl_tgeompointi where ti #&> tgeompoints '{Point(50 50)->Point(70 70)@[2001-05-01, 2001-07-01), Point(70 70)->Point(50 50)->Point(70 70)@[2001-07-01, 2001-09-01)}';
select count(*) from tbl_tgeompointi where ti #&> tgeompointi '{Point(50 50)@2001-05-01, Point(70 70)@2001-07-01}';

/******************************************************************************/
