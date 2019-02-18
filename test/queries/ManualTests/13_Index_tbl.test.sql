/******************************************************************************/

/* tboolinst */
--before
select count(*) from tbl_tboolinst where inst <<# timestamp '2001-05-01';
select count(*) from tbl_tboolinst where inst <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tboolinst where inst <<# tboolinst 'true@2001-05-01';
select count(*) from tbl_tboolinst where inst <<# tbooli '{true@2001-05-01, false@2001-07-01}';
select count(*) from tbl_tboolinst where inst <<# tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tboolinst where inst <<# tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--overbefore
select count(*) from tbl_tboolinst where inst &<# timestamp '2001-05-01';
select count(*) from tbl_tboolinst where inst #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tboolinst where inst &<# tboolinst 'true@2001-07-01';
select count(*) from tbl_tboolinst where inst &<# tbooli '{true@2001-05-01, false@2001-07-01}';
select count(*) from tbl_tboolinst where inst &<# tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tboolinst where inst &<# tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--after
select count(*) from tbl_tboolinst where inst #>> timestamp '2001-05-01';
select count(*) from tbl_tboolinst where inst #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tboolinst where inst #>> tboolinst 'true@2001-05-01';
select count(*) from tbl_tboolinst where inst #>> tbooli '{true@2001-03-01, false@2001-05-01}';
select count(*) from tbl_tboolinst where inst #>> tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tboolinst where inst #>> tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--overafter
select count(*) from tbl_tboolinst where inst #&> timestamp '2001-05-01';
select count(*) from tbl_tboolinst where inst #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tboolinst where inst #&> tboolinst 'true@2001-03-01';
select count(*) from tbl_tboolinst where inst #&> tbooli '{true@2001-03-01, false@2001-05-01}';
select count(*) from tbl_tboolinst where inst #&> tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tboolinst where inst #&> tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

/* tbooli */
--before
select count(*) from tbl_tbooli where ti <<# timestamp '2001-05-01';
select count(*) from tbl_tbooli where ti <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tbooli where ti <<# tboolinst 'true@2001-05-01';
select count(*) from tbl_tbooli where ti <<# tbooli '{true@2001-05-01, false@2001-07-01}';
select count(*) from tbl_tbooli where ti <<# tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tbooli where ti <<# tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--overbefore
select count(*) from tbl_tbooli where ti &<# timestamp '2001-07-01';
select count(*) from tbl_tbooli where ti &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tbooli where ti &<# tboolinst 'true@2001-07-01';
select count(*) from tbl_tbooli where ti &<# tbooli '{true@2001-05-01, false@2001-07-01}';
select count(*) from tbl_tbooli where ti &<# tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tbooli where ti &<# tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--after
select count(*) from tbl_tbooli where ti #>> timestamp '2001-05-01';
select count(*) from tbl_tbooli where ti #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tbooli where ti #>> tboolinst 'true@2001-05-01';
select count(*) from tbl_tbooli where ti #>> tbooli '{true@2001-03-01, false@2001-05-01}';
select count(*) from tbl_tbooli where ti #>> tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tbooli where ti #>> tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--overafter
select count(*) from tbl_tbooli where ti #&> timestamp '2001-03-01';
select count(*) from tbl_tbooli where ti #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tbooli where ti #&> tboolinst 'true@2001-03-01';
select count(*) from tbl_tbooli where ti #&> tbooli '{true@2001-03-01, false@2001-05-01}';
select count(*) from tbl_tbooli where ti #&> tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tbooli where ti #&> tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

/* tboolseq */
--before
select count(*) from tbl_tboolseq where seq <<# timestamp '2001-05-01';
select count(*) from tbl_tboolseq where seq <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tboolseq where seq <<# tboolinst 'true@2001-05-01';
select count(*) from tbl_tboolseq where seq <<# tbooli '{true@2001-05-01, false@2001-07-01}';
select count(*) from tbl_tboolseq where seq <<# tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tboolseq where seq <<# tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--overbefore
select count(*) from tbl_tboolseq where seq &<# timestamp '2001-07-01';
select count(*) from tbl_tboolseq where seq &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tboolseq where seq &<# tboolinst 'true@2001-07-01';
select count(*) from tbl_tboolseq where seq &<# tbooli '{true@2001-05-01, false@2001-07-01}';
select count(*) from tbl_tboolseq where seq &<# tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tboolseq where seq &<# tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--after
select count(*) from tbl_tboolseq where seq #>> timestamp '2001-05-01';
select count(*) from tbl_tboolseq where seq #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tboolseq where seq #>> tboolinst 'true@2001-05-01';
select count(*) from tbl_tboolseq where seq #>> tbooli '{true@2001-03-01, false@2001-05-01}';
select count(*) from tbl_tboolseq where seq #>> tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tboolseq where seq #>> tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--overafter
select count(*) from tbl_tboolseq where seq #&> timestamp '2001-03-01';
select count(*) from tbl_tboolseq where seq #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tboolseq where seq #&> tboolinst 'true@2001-03-01';
select count(*) from tbl_tboolseq where seq #&> tbooli '{true@2001-03-01, false@2001-05-01}';
select count(*) from tbl_tboolseq where seq #&> tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tboolseq where seq #&> tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

/* tbools */
--before
select count(*) from tbl_tbools where ts <<# timestamp '2001-05-01';
select count(*) from tbl_tbools where ts <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tbools where ts <<# tboolinst 'true@2001-05-01';
select count(*) from tbl_tbools where ts <<# tbooli '{true@2001-05-01, false@2001-07-01}';
select count(*) from tbl_tbools where ts <<# tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tbools where ts <<# tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--overbefore
select count(*) from tbl_tbools where ts &<# timestamp '2001-07-01';
select count(*) from tbl_tbools where ts &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tbools where ts &<# tboolinst 'true@2001-07-01';
select count(*) from tbl_tbools where ts &<# tbooli '{true@2001-05-01, false@2001-07-01}';
select count(*) from tbl_tbools where ts &<# tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tbools where ts &<# tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--after
select count(*) from tbl_tbools where ts #>> timestamp '2001-05-01';
select count(*) from tbl_tbools where ts #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tbools where ts #>> tboolinst 'true@2001-05-01';
select count(*) from tbl_tbools where ts #>> tbooli '{true@2001-03-01, false@2001-05-01}';
select count(*) from tbl_tbools where ts #>> tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tbools where ts #>> tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

--overafter
select count(*) from tbl_tbools where ts #&> timestamp '2001-03-01';
select count(*) from tbl_tbools where ts #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tbools where ts #&> tboolinst 'true@2001-03-01';
select count(*) from tbl_tbools where ts #&> tbooli '{true@2001-03-01, false@2001-05-01}';
select count(*) from tbl_tbools where ts #&> tboolseq '[true@2001-05-01, true@2001-07-01)';
select count(*) from tbl_tbools where ts #&> tbools '{[true@2001-05-01, true@2001-07-01), [false@2001-07-01, false@2001-09-01)}';

/******************************************************************************/

/* tintinst */
--left
select count(*) from tbl_tintinst where inst << 50;
select count(*) from tbl_tintinst where inst << 50.0;
select count(*) from tbl_tintinst where inst << intrange '[40,60]';
select count(*) from tbl_tintinst where inst << floatrange '[40,60]';
select count(*) from tbl_tintinst where inst << box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintinst where inst << tintinst '50@2001-05-01';
select count(*) from tbl_tintinst where inst << tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintinst where inst << tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintinst where inst << tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintinst where inst << tfloatinst '50@2001-05-01';
select count(*) from tbl_tintinst where inst << tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintinst where inst << tfloatseq '[50@2001-05-01, 55@2001-07-01)';
select count(*) from tbl_tintinst where inst << tfloats '{[50@2001-05-01, 55@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overleft
select count(*) from tbl_tintinst where inst &< 50;
select count(*) from tbl_tintinst where inst &< 50.0;
select count(*) from tbl_tintinst where inst &< intrange '[40,60]';
select count(*) from tbl_tintinst where inst &< floatrange '[40,60]';
select count(*) from tbl_tintinst where inst &< box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintinst where inst &< tintinst '50@2001-05-01';
select count(*) from tbl_tintinst where inst &< tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintinst where inst &< tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintinst where inst &< tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintinst where inst &< tfloatinst '50@2001-05-01';
select count(*) from tbl_tintinst where inst &< tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintinst where inst &< tfloatseq '[50@2001-05-01, 55@2001-07-01)';
select count(*) from tbl_tintinst where inst &< tfloats '{[50@2001-05-01, 55@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--right
select count(*) from tbl_tintinst where inst >> 50;
select count(*) from tbl_tintinst where inst >> 50.0;
select count(*) from tbl_tintinst where inst >> intrange '[40,60]';
select count(*) from tbl_tintinst where inst >> floatrange '[40,60]';
select count(*) from tbl_tintinst where inst >> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintinst where inst >> tintinst '50@2001-05-01';
select count(*) from tbl_tintinst where inst >> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintinst where inst >> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintinst where inst >> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintinst where inst >> tfloatinst '50@2001-05-01';
select count(*) from tbl_tintinst where inst >> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintinst where inst >> tfloatseq '[50@2001-05-01, 55@2001-07-01)';
select count(*) from tbl_tintinst where inst >> tfloats '{[50@2001-05-01, 55@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overright
select count(*) from tbl_tintinst where inst &> 50;
select count(*) from tbl_tintinst where inst &> 50.0;
select count(*) from tbl_tintinst where inst &> intrange '[40,60]';
select count(*) from tbl_tintinst where inst &> floatrange '[40,60]';
select count(*) from tbl_tintinst where inst &> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintinst where inst &> tintinst '50@2001-05-01';
select count(*) from tbl_tintinst where inst &> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintinst where inst &> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintinst where inst &> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintinst where inst &> tfloatinst '50@2001-05-01';
select count(*) from tbl_tintinst where inst &> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintinst where inst &> tfloatseq '[50@2001-05-01, 55@2001-07-01)';
select count(*) from tbl_tintinst where inst &> tfloats '{[50@2001-05-01, 55@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--before
select count(*) from tbl_tintinst where inst <<# timestamp '2001-05-01';
select count(*) from tbl_tintinst where inst <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tintinst where inst <<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintinst where inst <<# tintinst '5@2001-05-01';
select count(*) from tbl_tintinst where inst <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tintinst where inst <<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintinst where inst <<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintinst where inst <<# tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tintinst where inst <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tintinst where inst <<# tfloatseq '[50@2001-05-01, 55@2001-07-01)';
select count(*) from tbl_tintinst where inst <<# tfloats '{[50@2001-05-01, 55@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overbefore
select count(*) from tbl_tintinst where inst &<# timestamp '2001-07-01';
select count(*) from tbl_tintinst where inst &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tintinst where inst &<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintinst where inst &<# tintinst '5@2001-07-01';
select count(*) from tbl_tintinst where inst &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tintinst where inst &<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintinst where inst &<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintinst where inst &<# tfloatinst '4.5@2001-07-01';
select count(*) from tbl_tintinst where inst &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tintinst where inst &<# tfloatseq '[50@2001-05-01, 55@2001-07-01)';
select count(*) from tbl_tintinst where inst &<# tfloats '{[50@2001-05-01, 55@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--after
select count(*) from tbl_tintinst where inst #>> timestamp '2001-05-01';
select count(*) from tbl_tintinst where inst #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tintinst where inst #>> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintinst where inst #>> tintinst '5@2001-05-01';
select count(*) from tbl_tintinst where inst #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tintinst where inst #>> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintinst where inst #>> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintinst where inst #>> tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tintinst where inst #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tintinst where inst #>> tfloatseq '[50@2001-05-01, 55@2001-07-01)';
select count(*) from tbl_tintinst where inst #>> tfloats '{[50@2001-05-01, 55@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overafter
select count(*) from tbl_tintinst where inst #&> timestamp '2001-03-01';
select count(*) from tbl_tintinst where inst #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tintinst where inst #&> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintinst where inst #&> tintinst '5@2001-03-01';
select count(*) from tbl_tintinst where inst #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tintinst where inst #&> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintinst where inst #&> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintinst where inst #&> tfloatinst '3.5@2001-03-01';
select count(*) from tbl_tintinst where inst #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tintinst where inst #&> tfloatseq '[50@2001-05-01, 55@2001-07-01)';
select count(*) from tbl_tintinst where inst #&> tfloats '{[50@2001-05-01, 55@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

/* tintseq */
--left
select count(*) from tbl_tintseq where seq << 50;
select count(*) from tbl_tintseq where seq << 50.0;
select count(*) from tbl_tintseq where seq << intrange '[40,60]';
select count(*) from tbl_tintseq where seq << floatrange '[40,60]';
select count(*) from tbl_tintseq where seq << box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintseq where seq << tintinst '50@2001-05-01';
select count(*) from tbl_tintseq where seq << tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintseq where seq << tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintseq where seq << tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintseq where seq << tfloatinst '50@2001-05-01';
select count(*) from tbl_tintseq where seq << tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintseq where seq << tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tintseq where seq << tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overleft
select count(*) from tbl_tintseq where seq &< 50;
select count(*) from tbl_tintseq where seq &< 50.0;
select count(*) from tbl_tintseq where seq &< intrange '[40,60]';
select count(*) from tbl_tintseq where seq &< floatrange '[40,60]';
select count(*) from tbl_tintseq where seq &< box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintseq where seq &< tintinst '50@2001-05-01';
select count(*) from tbl_tintseq where seq &< tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintseq where seq &< tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintseq where seq &< tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintseq where seq &< tfloatinst '50@2001-05-01';
select count(*) from tbl_tintseq where seq &< tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintseq where seq &< tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tintseq where seq &< tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--right
select count(*) from tbl_tintseq where seq >> 50;
select count(*) from tbl_tintseq where seq >> 50.0;
select count(*) from tbl_tintseq where seq >> intrange '[40,60]';
select count(*) from tbl_tintseq where seq >> floatrange '[40,60]';
select count(*) from tbl_tintseq where seq >> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintseq where seq >> tintinst '50@2001-05-01';
select count(*) from tbl_tintseq where seq >> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintseq where seq >> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintseq where seq >> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintseq where seq >> tfloatinst '50@2001-05-01';
select count(*) from tbl_tintseq where seq >> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintseq where seq >> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tintseq where seq >> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overright
select count(*) from tbl_tintseq where seq &> 50;
select count(*) from tbl_tintseq where seq &> 50.0;
select count(*) from tbl_tintseq where seq &> intrange '[40,60]';
select count(*) from tbl_tintseq where seq &> floatrange '[40,60]';
select count(*) from tbl_tintseq where seq &> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintseq where seq &> tintinst '50@2001-05-01';
select count(*) from tbl_tintseq where seq &> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintseq where seq &> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintseq where seq &> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintseq where seq &> tfloatinst '50@2001-05-01';
select count(*) from tbl_tintseq where seq &> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tintseq where seq &> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tintseq where seq &> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--before
select count(*) from tbl_tintseq where seq <<# timestamp '2001-05-01';
select count(*) from tbl_tintseq where seq <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tintseq where seq <<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintseq where seq <<# tintinst '5@2001-05-01';
select count(*) from tbl_tintseq where seq <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tintseq where seq <<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintseq where seq <<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintseq where seq <<# tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tintseq where seq <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tintseq where seq <<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tintseq where seq <<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overbefore
select count(*) from tbl_tintseq where seq &<# timestamp '2001-07-01';
select count(*) from tbl_tintseq where seq &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tintseq where seq &<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintseq where seq &<# tintinst '5@2001-07-01';
select count(*) from tbl_tintseq where seq &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tintseq where seq &<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintseq where seq &<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintseq where seq &<# tfloatinst '4.5@2001-07-01';
select count(*) from tbl_tintseq where seq &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tintseq where seq &<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tintseq where seq &<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--after
select count(*) from tbl_tintseq where seq #>> timestamp '2001-05-01';
select count(*) from tbl_tintseq where seq #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tintseq where seq #>> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintseq where seq #>> tintinst '5@2001-05-01';
select count(*) from tbl_tintseq where seq #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tintseq where seq #>> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintseq where seq #>> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintseq where seq #>> tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tintseq where seq #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tintseq where seq #>> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tintseq where seq #>> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overafter
select count(*) from tbl_tintseq where seq #&> timestamp '2001-03-01';
select count(*) from tbl_tintseq where seq #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tintseq where seq #&> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tintseq where seq #&> tintinst '5@2001-03-01';
select count(*) from tbl_tintseq where seq #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tintseq where seq #&> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tintseq where seq #&> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tintseq where seq #&> tfloatinst '3.5@2001-03-01';
select count(*) from tbl_tintseq where seq #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tintseq where seq #&> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tintseq where seq #&> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

/* tints */
--left
select count(*) from tbl_tints where ts << 50;
select count(*) from tbl_tints where ts << 50.0;
select count(*) from tbl_tints where ts << intrange '[40,60]';
select count(*) from tbl_tints where ts << floatrange '[40,60]';
select count(*) from tbl_tints where ts << box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tints where ts << tintinst '50@2001-05-01';
select count(*) from tbl_tints where ts << tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tints where ts << tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tints where ts << tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tints where ts << tfloatinst '50@2001-05-01';
select count(*) from tbl_tints where ts << tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tints where ts << tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tints where ts << tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overleft
select count(*) from tbl_tints where ts &< 50;
select count(*) from tbl_tints where ts &< 50.0;
select count(*) from tbl_tints where ts &< intrange '[40,60]';
select count(*) from tbl_tints where ts &< floatrange '[40,60]';
select count(*) from tbl_tints where ts &< box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tints where ts &< tintinst '50@2001-05-01';
select count(*) from tbl_tints where ts &< tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tints where ts &< tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tints where ts &< tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tints where ts &< tfloatinst '50@2001-05-01';
select count(*) from tbl_tints where ts &< tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tints where ts &< tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tints where ts &< tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--right
select count(*) from tbl_tints where ts >> 50;
select count(*) from tbl_tints where ts >> 50.0;
select count(*) from tbl_tints where ts >> intrange '[40,60]';
select count(*) from tbl_tints where ts >> floatrange '[40,60]';
select count(*) from tbl_tints where ts >> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tints where ts >> tintinst '50@2001-05-01';
select count(*) from tbl_tints where ts >> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tints where ts >> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tints where ts >> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tints where ts >> tfloatinst '50@2001-05-01';
select count(*) from tbl_tints where ts >> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tints where ts >> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tints where ts >> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overright
select count(*) from tbl_tints where ts &> 50;
select count(*) from tbl_tints where ts &> 50.0;
select count(*) from tbl_tints where ts &> intrange '[40,60]';
select count(*) from tbl_tints where ts &> floatrange '[40,60]';
select count(*) from tbl_tints where ts &> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tints where ts &> tintinst '50@2001-05-01';
select count(*) from tbl_tints where ts &> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tints where ts &> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tints where ts &> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tints where ts &> tfloatinst '50@2001-05-01';
select count(*) from tbl_tints where ts &> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tints where ts &> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tints where ts &> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--before
select count(*) from tbl_tints where ts <<# timestamp '2001-05-01';
select count(*) from tbl_tints where ts <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tints where ts <<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tints where ts <<# tintinst '5@2001-05-01';
select count(*) from tbl_tints where ts <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tints where ts <<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tints where ts <<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tints where ts <<# tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tints where ts <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tints where ts <<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tints where ts <<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overbefore
select count(*) from tbl_tints where ts &<# timestamp '2001-07-01';
select count(*) from tbl_tints where ts &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tints where ts &<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tints where ts &<# tintinst '5@2001-07-01';
select count(*) from tbl_tints where ts &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tints where ts &<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tints where ts &<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tints where ts &<# tfloatinst '4.5@2001-07-01';
select count(*) from tbl_tints where ts &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tints where ts &<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tints where ts &<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--after
select count(*) from tbl_tints where ts #>> timestamp '2001-05-01';
select count(*) from tbl_tints where ts #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tints where ts #>> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tints where ts #>> tintinst '5@2001-05-01';
select count(*) from tbl_tints where ts #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tints where ts #>> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tints where ts #>> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tints where ts #>> tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tints where ts #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tints where ts #>> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tints where ts #>> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overafter
select count(*) from tbl_tints where ts #&> timestamp '2001-03-01';
select count(*) from tbl_tints where ts #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tints where ts #&> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tints where ts #&> tintinst '5@2001-03-01';
select count(*) from tbl_tints where ts #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tints where ts #&> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tints where ts #&> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tints where ts #&> tfloatinst '3.5@2001-03-01';
select count(*) from tbl_tints where ts #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tints where ts #&> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tints where ts #&> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

/* tinti */
--left
select count(*) from tbl_tinti where ti << 50;
select count(*) from tbl_tinti where ti << 50.0;
select count(*) from tbl_tinti where ti << intrange '[40,60]';
select count(*) from tbl_tinti where ti << floatrange '[40,60]';
select count(*) from tbl_tinti where ti << tintinst '50@2001-05-01';
select count(*) from tbl_tinti where ti << tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tinti where ti << tintseq '[50@2001-05-01, 60@2001-07-01]';
select count(*) from tbl_tinti where ti << tints '{[50@2001-05-01, 60@2001-07-01], [70@2001-08-01, 70@2001-09-01)}';
select count(*) from tbl_tinti where ti << tfloatinst '50@2001-05-01';
select count(*) from tbl_tinti where ti << tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tinti where ti << tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tinti where ti << tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overleft
select count(*) from tbl_tinti where ti &< 50;
select count(*) from tbl_tinti where ti &< 50.0;
select count(*) from tbl_tinti where ti &< intrange '[40,60]';
select count(*) from tbl_tinti where ti &< floatrange '[40,60]';
select count(*) from tbl_tinti where ti &< tintinst '70@2001-05-01';
select count(*) from tbl_tinti where ti &< tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tinti where ti &< tintseq '[50@2001-05-01, 60@2001-07-01]';
select count(*) from tbl_tinti where ti &< tints '{[50@2001-05-01, 60@2001-07-01], [70@2001-08-01, 70@2001-09-01)}';
select count(*) from tbl_tinti where ti &< tfloatinst '50@2001-05-01';
select count(*) from tbl_tinti where ti &< tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tinti where ti &< tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tinti where ti &< tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--right
select count(*) from tbl_tinti where ti >> 50;
select count(*) from tbl_tinti where ti >> 50.0;
select count(*) from tbl_tinti where ti >> intrange '[40,60]';
select count(*) from tbl_tinti where ti >> floatrange '[40,60]';
select count(*) from tbl_tinti where ti >> tintinst '50@2001-05-01';
select count(*) from tbl_tinti where ti >> tinti '{30@2001-05-01, 50@2001-07-01}';
select count(*) from tbl_tinti where ti >> tintseq '[50@2001-05-01, 60@2001-07-01]';
select count(*) from tbl_tinti where ti >> tints '{[50@2001-05-01, 60@2001-07-01], [70@2001-08-01, 70@2001-09-01)}';
select count(*) from tbl_tinti where ti >> tfloatinst '50@2001-05-01';
select count(*) from tbl_tinti where ti >> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tinti where ti >> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tinti where ti >> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overright
select count(*) from tbl_tinti where ti &> 50;
select count(*) from tbl_tinti where ti &> 50.0;
select count(*) from tbl_tinti where ti &> intrange '[40,60]';
select count(*) from tbl_tinti where ti &> floatrange '[40,60]';
select count(*) from tbl_tinti where ti &> tintinst '30@2001-05-01';
select count(*) from tbl_tinti where ti &> tinti '{30@2001-05-01, 50@2001-07-01}';
select count(*) from tbl_tinti where ti &> tintseq '[50@2001-05-01, 60@2001-07-01]';
select count(*) from tbl_tinti where ti &> tints '{[50@2001-05-01, 60@2001-07-01], [70@2001-08-01, 70@2001-09-01)}';
select count(*) from tbl_tinti where ti &> tfloatinst '50@2001-05-01';
select count(*) from tbl_tinti where ti &> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tinti where ti &> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tinti where ti &> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--before
select count(*) from tbl_tinti where ti <<# timestamp '2001-05-01';
select count(*) from tbl_tinti where ti <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tinti where ti <<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tinti where ti <<# tintinst '5@2001-05-01';
select count(*) from tbl_tinti where ti <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tinti where ti <<# tintseq '[50@2001-05-01, 60@2001-07-01]';
select count(*) from tbl_tinti where ti <<# tints '{[50@2001-05-01, 60@2001-07-01], [70@2001-08-01, 70@2001-09-01)}';
select count(*) from tbl_tinti where ti <<# tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tinti where ti <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tinti where ti <<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tinti where ti <<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overbefore
select count(*) from tbl_tinti where ti &<# timestamp '2001-07-01';
select count(*) from tbl_tinti where ti &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tinti where ti &<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tinti where ti &<# tintinst '5@2001-07-01';
select count(*) from tbl_tinti where ti &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tinti where ti &<# tintseq '[50@2001-05-01, 60@2001-07-01]';
select count(*) from tbl_tinti where ti &<# tints '{[50@2001-05-01, 60@2001-07-01], [70@2001-08-01, 70@2001-09-01)}';
select count(*) from tbl_tinti where ti &<# tfloatinst '4.5@2001-07-01';
select count(*) from tbl_tinti where ti &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tinti where ti &<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tinti where ti &<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--after
select count(*) from tbl_tinti where ti #>> timestamp '2001-05-01';
select count(*) from tbl_tinti where ti #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tinti where ti #>> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tinti where ti #>> tintinst '5@2001-05-01';
select count(*) from tbl_tinti where ti #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tinti where ti #>> tintseq '[50@2001-05-01, 60@2001-07-01]';
select count(*) from tbl_tinti where ti #>> tints '{[50@2001-05-01, 60@2001-07-01], [70@2001-08-01, 70@2001-09-01)}';
select count(*) from tbl_tinti where ti #>> tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tinti where ti #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tinti where ti #>> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tinti where ti #>> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overafter
select count(*) from tbl_tinti where ti #&> timestamp '2001-03-01';
select count(*) from tbl_tinti where ti #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tinti where ti #&> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tinti where ti #&> tintinst '5@2001-03-01';
select count(*) from tbl_tinti where ti #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tinti where ti #&> tintseq '[50@2001-05-01, 60@2001-07-01]';
select count(*) from tbl_tinti where ti #&> tints '{[50@2001-05-01, 60@2001-07-01], [70@2001-08-01, 70@2001-09-01)}';
select count(*) from tbl_tinti where ti #&> tfloatinst '3.5@2001-03-01';
select count(*) from tbl_tinti where ti #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tinti where ti #&> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tinti where ti #&> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

/******************************************************************************/

/* tfloatinst */
--left
select count(*) from tbl_tfloatinst where inst << 50;
select count(*) from tbl_tfloatinst where inst << 50.0;
select count(*) from tbl_tfloatinst where inst << intrange '[40,60]';
select count(*) from tbl_tfloatinst where inst << floatrange '[40,60]';
select count(*) from tbl_tfloatinst where inst << tintinst '50@2001-05-01';
select count(*) from tbl_tfloatinst where inst << tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatinst where inst << tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatinst where inst << tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatinst where inst << tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloatinst where inst << tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatinst where inst << tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatinst where inst << tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overleft
select count(*) from tbl_tfloatinst where inst &< 50;
select count(*) from tbl_tfloatinst where inst &< 50.0;
select count(*) from tbl_tfloatinst where inst &< intrange '[40,60]';
select count(*) from tbl_tfloatinst where inst &< floatrange '[40,60]';
select count(*) from tbl_tfloatinst where inst &< tintinst '50@2001-05-01';
select count(*) from tbl_tfloatinst where inst &< tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatinst where inst &< tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatinst where inst &< tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatinst where inst &< tfloatinst '70@2001-05-01';
select count(*) from tbl_tfloatinst where inst &< tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatinst where inst &< tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatinst where inst &< tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--right
select count(*) from tbl_tfloatinst where inst >> 50;
select count(*) from tbl_tfloatinst where inst >> 50.0;
select count(*) from tbl_tfloatinst where inst >> intrange '[40,60]';
select count(*) from tbl_tfloatinst where inst >> floatrange '[40,60]';
select count(*) from tbl_tfloatinst where inst >> tintinst '50@2001-05-01';
select count(*) from tbl_tfloatinst where inst >> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatinst where inst >> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatinst where inst >> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatinst where inst >> tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloatinst where inst >> tfloati '{30@2001-05-01, 50@2001-07-01}';
select count(*) from tbl_tfloatinst where inst >> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatinst where inst >> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overright
select count(*) from tbl_tfloatinst where inst &> 50;
select count(*) from tbl_tfloatinst where inst &> 50.0;
select count(*) from tbl_tfloatinst where inst &> intrange '[40,60]';
select count(*) from tbl_tfloatinst where inst &> floatrange '[40,60]';
select count(*) from tbl_tfloatinst where inst &> tintinst '50@2001-05-01';
select count(*) from tbl_tfloatinst where inst &> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatinst where inst &> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatinst where inst &> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatinst where inst &> tfloatinst '30@2001-05-01';
select count(*) from tbl_tfloatinst where inst &> tfloati '{30@2001-05-01, 50@2001-07-01}';
select count(*) from tbl_tfloatinst where inst &> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatinst where inst &> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--before
select count(*) from tbl_tfloatinst where inst <<# timestamp '2001-05-01';
select count(*) from tbl_tfloatinst where inst <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloatinst where inst <<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatinst where inst <<# tintinst '5@2001-05-01';
select count(*) from tbl_tfloatinst where inst <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tfloatinst where inst <<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatinst where inst <<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatinst where inst <<# tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tfloatinst where inst <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tfloatinst where inst <<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatinst where inst <<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overbefore
select count(*) from tbl_tfloatinst where inst &<# timestamp '2001-07-01';
select count(*) from tbl_tfloatinst where inst &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloatinst where inst &<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatinst where inst &<# tintinst '5@2001-07-01';
select count(*) from tbl_tfloatinst where inst &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tfloatinst where inst &<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatinst where inst &<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatinst where inst &<# tfloatinst '4.5@2001-07-01';
select count(*) from tbl_tfloatinst where inst &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tfloatinst where inst &<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatinst where inst &<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--after
select count(*) from tbl_tfloatinst where inst #>> timestamp '2001-05-01';
select count(*) from tbl_tfloatinst where inst #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloatinst where inst #>> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatinst where inst #>> tintinst '5@2001-05-01';
select count(*) from tbl_tfloatinst where inst #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tfloatinst where inst #>> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatinst where inst #>> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatinst where inst #>> tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tfloatinst where inst #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tfloatinst where inst #>> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatinst where inst #>> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overafter
select count(*) from tbl_tfloatinst where inst #&> timestamp '2001-03-01';
select count(*) from tbl_tfloatinst where inst #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloatinst where inst #&> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatinst where inst #&> tintinst '5@2001-03-01';
select count(*) from tbl_tfloatinst where inst #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tfloatinst where inst #&> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatinst where inst #&> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatinst where inst #&> tfloatinst '3.5@2001-03-01';
select count(*) from tbl_tfloatinst where inst #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tfloatinst where inst #&> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatinst where inst #&> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

/* tfloati */
--left
select count(*) from tbl_tfloati where ti << 50;
select count(*) from tbl_tfloati where ti << 50.0;
select count(*) from tbl_tfloati where ti << intrange '[40,60]';
select count(*) from tbl_tfloati where ti << floatrange '[40,60]';
select count(*) from tbl_tfloati where ti << tintinst '50@2001-05-01';
select count(*) from tbl_tfloati where ti << tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloati where ti << tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloati where ti << tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloati where ti << tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloati where ti << tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloati where ti << tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloati where ti << tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overleft
select count(*) from tbl_tfloati where ti &< 50;
select count(*) from tbl_tfloati where ti &< 50.0;
select count(*) from tbl_tfloati where ti &< intrange '[40,60]';
select count(*) from tbl_tfloati where ti &< floatrange '[40,60]';
select count(*) from tbl_tfloati where ti &< tintinst '50@2001-05-01';
select count(*) from tbl_tfloati where ti &< tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloati where ti &< tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloati where ti &< tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloati where ti &< tfloatinst '70@2001-05-01';
select count(*) from tbl_tfloati where ti &< tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloati where ti &< tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloati where ti &< tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--right
select count(*) from tbl_tfloati where ti >> 50;
select count(*) from tbl_tfloati where ti >> 50.0;
select count(*) from tbl_tfloati where ti >> intrange '[40,60]';
select count(*) from tbl_tfloati where ti >> floatrange '[40,60]';
select count(*) from tbl_tfloati where ti >> tintinst '50@2001-05-01';
select count(*) from tbl_tfloati where ti >> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloati where ti >> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloati where ti >> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloati where ti >> tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloati where ti >> tfloati '{30@2001-05-01, 50@2001-07-01}';
select count(*) from tbl_tfloati where ti >> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloati where ti >> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overright
select count(*) from tbl_tfloati where ti &> 50;
select count(*) from tbl_tfloati where ti &> 50.0;
select count(*) from tbl_tfloati where ti &> intrange '[40,60]';
select count(*) from tbl_tfloati where ti &> floatrange '[40,60]';
select count(*) from tbl_tfloati where ti &> tintinst '50@2001-05-01';
select count(*) from tbl_tfloati where ti &> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloati where ti &> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloati where ti &> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloati where ti &> tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloati where ti &> tfloati '{30@2001-05-01, 50@2001-07-01}';
select count(*) from tbl_tfloati where ti &> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloati where ti &> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--before
select count(*) from tbl_tfloati where ti <<# timestamp '2001-05-01';
select count(*) from tbl_tfloati where ti <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloati where ti <<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloati where ti <<# tintinst '5@2001-05-01';
select count(*) from tbl_tfloati where ti <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tfloati where ti <<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloati where ti <<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloati where ti <<# tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tfloati where ti <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tfloati where ti <<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloati where ti <<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overbefore
select count(*) from tbl_tfloati where ti &<# timestamp '2001-07-01';
select count(*) from tbl_tfloati where ti &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloati where ti &<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloati where ti &<# tintinst '5@2001-07-01';
select count(*) from tbl_tfloati where ti &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tfloati where ti &<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloati where ti &<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloati where ti &<# tfloatinst '4.5@2001-07-01';
select count(*) from tbl_tfloati where ti &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tfloati where ti &<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloati where ti &<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--after
select count(*) from tbl_tfloati where ti #>> timestamp '2001-05-01';
select count(*) from tbl_tfloati where ti #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloati where ti #>> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloati where ti #>> tintinst '5@2001-05-01';
select count(*) from tbl_tfloati where ti #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tfloati where ti #>> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloati where ti #>> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloati where ti #>> tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tfloati where ti #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tfloati where ti #>> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloati where ti #>> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overafter
select count(*) from tbl_tfloati where ti #&> timestamp '2001-03-01';
select count(*) from tbl_tfloati where ti #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloati where ti #&> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloati where ti #&> tintinst '5@2001-03-01';
select count(*) from tbl_tfloati where ti #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tfloati where ti #&> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloati where ti #&> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloati where ti #&> tfloatinst '3.5@2001-03-01';
select count(*) from tbl_tfloati where ti #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tfloati where ti #&> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloati where ti #&> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

/* tfloatseq */
--left
select count(*) from tbl_tfloatseq where seq << 50;
select count(*) from tbl_tfloatseq where seq << 50.0;
select count(*) from tbl_tfloatseq where seq << intrange '[40,60]';
select count(*) from tbl_tfloatseq where seq << floatrange '[40,60]';
select count(*) from tbl_tfloatseq where seq << box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatseq where seq << tintinst '50@2001-05-01';
select count(*) from tbl_tfloatseq where seq << tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatseq where seq << tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatseq where seq << tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatseq where seq << tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloatseq where seq << tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatseq where seq << tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatseq where seq << tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overleft
select count(*) from tbl_tfloatseq where seq &< 50;
select count(*) from tbl_tfloatseq where seq &< 50.0;
select count(*) from tbl_tfloatseq where seq &< intrange '[40,60]';
select count(*) from tbl_tfloatseq where seq &< floatrange '[40,60]';
select count(*) from tbl_tfloatseq where seq &< box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatseq where seq &< tintinst '50@2001-05-01';
select count(*) from tbl_tfloatseq where seq &< tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatseq where seq &< tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatseq where seq &< tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatseq where seq &< tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloatseq where seq &< tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatseq where seq &< tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatseq where seq &< tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--right
select count(*) from tbl_tfloatseq where seq >> 50;
select count(*) from tbl_tfloatseq where seq >> 50.0;
select count(*) from tbl_tfloatseq where seq >> intrange '[40,60]';
select count(*) from tbl_tfloatseq where seq >> floatrange '[40,60]';
select count(*) from tbl_tfloatseq where seq >> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatseq where seq >> tintinst '50@2001-05-01';
select count(*) from tbl_tfloatseq where seq >> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatseq where seq >> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatseq where seq >> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatseq where seq >> tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloatseq where seq >> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatseq where seq >> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatseq where seq >> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overright
select count(*) from tbl_tfloatseq where seq &> 50;
select count(*) from tbl_tfloatseq where seq &> 50.0;
select count(*) from tbl_tfloatseq where seq &> intrange '[40,60]';
select count(*) from tbl_tfloatseq where seq &> floatrange '[40,60]';
select count(*) from tbl_tfloatseq where seq &> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatseq where seq &> tintinst '50@2001-05-01';
select count(*) from tbl_tfloatseq where seq &> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatseq where seq &> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatseq where seq &> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatseq where seq &> tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloatseq where seq &> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloatseq where seq &> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatseq where seq &> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--before
select count(*) from tbl_tfloatseq where seq <<# timestamp '2001-05-01';
select count(*) from tbl_tfloatseq where seq <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloatseq where seq <<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatseq where seq <<# tintinst '5@2001-05-01';
select count(*) from tbl_tfloatseq where seq <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tfloatseq where seq <<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatseq where seq <<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatseq where seq <<# tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tfloatseq where seq <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tfloatseq where seq <<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatseq where seq <<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overbefore
select count(*) from tbl_tfloatseq where seq &<# timestamp '2001-07-01';
select count(*) from tbl_tfloatseq where seq &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloatseq where seq &<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatseq where seq &<# tintinst '5@2001-07-01';
select count(*) from tbl_tfloatseq where seq &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tfloatseq where seq &<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatseq where seq &<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatseq where seq &<# tfloatinst '4.5@2001-07-01';
select count(*) from tbl_tfloatseq where seq &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tfloatseq where seq &<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatseq where seq &<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--after
select count(*) from tbl_tfloatseq where seq #>> timestamp '2001-05-01';
select count(*) from tbl_tfloatseq where seq #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloatseq where seq #>> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatseq where seq #>> tintinst '5@2001-05-01';
select count(*) from tbl_tfloatseq where seq #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tfloatseq where seq #>> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatseq where seq #>> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatseq where seq #>> tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tfloatseq where seq #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tfloatseq where seq #>> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatseq where seq #>> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overafter
select count(*) from tbl_tfloatseq where seq #&> timestamp '2001-03-01';
select count(*) from tbl_tfloatseq where seq #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloatseq where seq #&> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloatseq where seq #&> tintinst '5@2001-03-01';
select count(*) from tbl_tfloatseq where seq #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tfloatseq where seq #&> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloatseq where seq #&> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloatseq where seq #&> tfloatinst '3.5@2001-03-01';
select count(*) from tbl_tfloatseq where seq #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tfloatseq where seq #&> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloatseq where seq #&> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

/* tfloats */
--left
select count(*) from tbl_tfloats where ts << 50;
select count(*) from tbl_tfloats where ts << 50.0;
select count(*) from tbl_tfloats where ts << intrange '[40,60]';
select count(*) from tbl_tfloats where ts << floatrange '[40,60]';
select count(*) from tbl_tfloats where ts << box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloats where ts << tintinst '50@2001-05-01';
select count(*) from tbl_tfloats where ts << tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloats where ts << tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloats where ts << tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloats where ts << tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloats where ts << tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloats where ts << tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloats where ts << tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overleft
select count(*) from tbl_tfloats where ts &< 50;
select count(*) from tbl_tfloats where ts &< 50.0;
select count(*) from tbl_tfloats where ts &< intrange '[40,60]';
select count(*) from tbl_tfloats where ts &< floatrange '[40,60]';
select count(*) from tbl_tfloats where ts &< box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloats where ts &< tintinst '50@2001-05-01';
select count(*) from tbl_tfloats where ts &< tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloats where ts &< tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloats where ts &< tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloats where ts &< tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloats where ts &< tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloats where ts &< tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloats where ts &< tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--right
select count(*) from tbl_tfloats where ts >> 50;
select count(*) from tbl_tfloats where ts >> 50.0;
select count(*) from tbl_tfloats where ts >> intrange '[40,60]';
select count(*) from tbl_tfloats where ts >> floatrange '[40,60]';
select count(*) from tbl_tfloats where ts >> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloats where ts >> tintinst '50@2001-05-01';
select count(*) from tbl_tfloats where ts >> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloats where ts >> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloats where ts >> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloats where ts >> tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloats where ts >> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloats where ts >> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloats where ts >> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overright
select count(*) from tbl_tfloats where ts &> 50;
select count(*) from tbl_tfloats where ts &> 50.0;
select count(*) from tbl_tfloats where ts &> intrange '[40,60]';
select count(*) from tbl_tfloats where ts &> floatrange '[40,60]';
select count(*) from tbl_tfloats where ts &> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloats where ts &> tintinst '50@2001-05-01';
select count(*) from tbl_tfloats where ts &> tinti '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloats where ts &> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloats where ts &> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloats where ts &> tfloatinst '50@2001-05-01';
select count(*) from tbl_tfloats where ts &> tfloati '{50@2001-05-01, 70@2001-07-01}';
select count(*) from tbl_tfloats where ts &> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloats where ts &> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--before
select count(*) from tbl_tfloats where ts <<# timestamp '2001-05-01';
select count(*) from tbl_tfloats where ts <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloats where ts <<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloats where ts <<# tintinst '5@2001-05-01';
select count(*) from tbl_tfloats where ts <<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tfloats where ts <<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloats where ts <<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloats where ts <<# tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tfloats where ts <<# tfloati '{3.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tfloats where ts <<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloats where ts <<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overbefore
select count(*) from tbl_tfloats where ts &<# timestamp '2001-07-01';
select count(*) from tbl_tfloats where ts &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloats where ts &<# box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloats where ts &<# tintinst '5@2001-07-01';
select count(*) from tbl_tfloats where ts &<# tinti '{5@2001-05-01, 6@2001-07-01}';
select count(*) from tbl_tfloats where ts &<# tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloats where ts &<# tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloats where ts &<# tfloatinst '4.5@2001-07-01';
select count(*) from tbl_tfloats where ts &<# tfloati '{4.5@2001-05-01, 5.6@2001-07-01}';
select count(*) from tbl_tfloats where ts &<# tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloats where ts &<# tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--after
select count(*) from tbl_tfloats where ts #>> timestamp '2001-05-01';
select count(*) from tbl_tfloats where ts #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloats where ts #>> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloats where ts #>> tintinst '5@2001-05-01';
select count(*) from tbl_tfloats where ts #>> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tfloats where ts #>> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloats where ts #>> tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloats where ts #>> tfloatinst '3.5@2001-05-01';
select count(*) from tbl_tfloats where ts #>> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tfloats where ts #>> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloats where ts #>> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

--overafter
select count(*) from tbl_tfloats where ts #&> timestamp '2001-03-01';
select count(*) from tbl_tfloats where ts #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_tfloats where ts #&> box '(60,47260800000000),(40,41990400000000)';
select count(*) from tbl_tfloats where ts #&> tintinst '5@2001-03-01';
select count(*) from tbl_tfloats where ts #&> tinti '{5@2001-03-01, 6@2001-05-01}';
select count(*) from tbl_tfloats where ts #&> tintseq '[50@2001-05-01, 50@2001-07-01)';
select count(*) from tbl_tfloats where ts << tints '{[50@2001-05-01, 50@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';
select count(*) from tbl_tfloats where ts #&> tfloatinst '3.5@2001-03-01';
select count(*) from tbl_tfloats where ts #&> tfloati '{3.5@2001-03-01, 5.6@2001-05-01}';
select count(*) from tbl_tfloats where ts #&> tfloatseq '[50@2001-05-01, 60@2001-07-01)';
select count(*) from tbl_tfloats where ts #&> tfloats '{[50@2001-05-01, 60@2001-07-01), [70@2001-07-01, 70@2001-09-01)}';

/******************************************************************************/

/* ttextinst */

--before
select count(*) from tbl_ttextinst where inst <<# timestamp '2001-05-01';
select count(*) from tbl_ttextinst where inst <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttextinst where inst <<# ttextinst 'AAA@2001-05-01';
select count(*) from tbl_ttextinst where inst <<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';
select count(*) from tbl_ttextinst where inst <<# ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttextinst where inst <<# ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--overbefore
select count(*) from tbl_ttextinst where inst &<# timestamp '2001-05-01';
select count(*) from tbl_ttextinst where inst #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttextinst where inst &<# ttextinst 'AAA@2001-07-01';
select count(*) from tbl_ttextinst where inst &<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';
select count(*) from tbl_ttextinst where inst &<# ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttextinst where inst &<# ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--after
select count(*) from tbl_ttextinst where inst #>> timestamp '2001-05-01';
select count(*) from tbl_ttextinst where inst #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttextinst where inst #>> ttextinst 'AAA@2001-05-01';
select count(*) from tbl_ttextinst where inst #>> ttexti '{AAA@2001-03-01, BBB@2001-05-01}';
select count(*) from tbl_ttextinst where inst #>> ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttextinst where inst #>> ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--overafter
select count(*) from tbl_ttextinst where inst #&> timestamp '2001-05-01';
select count(*) from tbl_ttextinst where inst #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttextinst where inst #&> ttextinst 'AAA@2001-03-01';
select count(*) from tbl_ttextinst where inst #&> ttexti '{AAA@2001-03-01, BBB@2001-05-01}';
select count(*) from tbl_ttextinst where inst #&> ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttextinst where inst #&> ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

/* ttexti */
--before
select count(*) from tbl_ttexti where ti <<# timestamp '2001-05-01';
select count(*) from tbl_ttexti where ti <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttexti where ti <<# ttextinst 'AAA@2001-05-01';
select count(*) from tbl_ttexti where ti <<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';
select count(*) from tbl_ttexti where ti <<# ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttexti where ti <<# ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--overbefore
select count(*) from tbl_ttexti where ti &<# timestamp '2001-07-01';
select count(*) from tbl_ttexti where ti &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttexti where ti &<# ttextinst 'AAA@2001-07-01';
select count(*) from tbl_ttexti where ti &<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';
select count(*) from tbl_ttexti where ti &<# ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttexti where ti &<# ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--after
select count(*) from tbl_ttexti where ti #>> timestamp '2001-05-01';
select count(*) from tbl_ttexti where ti #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttexti where ti #>> ttextinst 'AAA@2001-05-01';
select count(*) from tbl_ttexti where ti #>> ttexti '{AAA@2001-03-01, BBB@2001-05-01}';
select count(*) from tbl_ttexti where ti #>> ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttexti where ti #>> ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--overafter
select count(*) from tbl_ttexti where ti #&> timestamp '2001-03-01';
select count(*) from tbl_ttexti where ti #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttexti where ti #&> ttextinst 'AAA@2001-03-01';
select count(*) from tbl_ttexti where ti #&> ttexti '{AAA@2001-03-01, BBB@2001-05-01}';
select count(*) from tbl_ttexti where ti #&> ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttexti where ti #&> ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

/* ttextseq */
--before
select count(*) from tbl_ttextseq where seq <<# timestamp '2001-05-01';
select count(*) from tbl_ttextseq where seq <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttextseq where seq <<# ttextinst 'AAA@2001-05-01';
select count(*) from tbl_ttextseq where seq <<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';
select count(*) from tbl_ttextseq where seq <<# ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttextseq where seq <<# ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--overbefore
select count(*) from tbl_ttextseq where seq &<# timestamp '2001-07-01';
select count(*) from tbl_ttextseq where seq &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttextseq where seq &<# ttextinst 'AAA@2001-07-01';
select count(*) from tbl_ttextseq where seq &<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';
select count(*) from tbl_ttextseq where seq &<# ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttextseq where seq &<# ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--after
select count(*) from tbl_ttextseq where seq #>> timestamp '2001-05-01';
select count(*) from tbl_ttextseq where seq #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttextseq where seq #>> ttextinst 'AAA@2001-05-01';
select count(*) from tbl_ttextseq where seq #>> ttexti '{AAA@2001-03-01, BBB@2001-05-01}';
select count(*) from tbl_ttextseq where seq #>> ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttextseq where seq #>> ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--overafter
select count(*) from tbl_ttextseq where seq #&> timestamp '2001-03-01';
select count(*) from tbl_ttextseq where seq #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttextseq where seq #&> ttextinst 'AAA@2001-03-01';
select count(*) from tbl_ttextseq where seq #&> ttexti '{AAA@2001-03-01, BBB@2001-05-01}';
select count(*) from tbl_ttextseq where seq #&> ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttextseq where seq #&> ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

/* ttexts */
--before
select count(*) from tbl_ttexts where ts <<# timestamp '2001-05-01';
select count(*) from tbl_ttexts where ts <<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttexts where ts <<# ttextinst 'AAA@2001-05-01';
select count(*) from tbl_ttexts where ts <<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';
select count(*) from tbl_ttexts where ts <<# ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttexts where ts <<# ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--overbefore
select count(*) from tbl_ttexts where ts &<# timestamp '2001-07-01';
select count(*) from tbl_ttexts where ts &<# period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttexts where ts &<# ttextinst 'AAA@2001-07-01';
select count(*) from tbl_ttexts where ts &<# ttexti '{AAA@2001-05-01, BBB@2001-07-01}';
select count(*) from tbl_ttexts where ts &<# ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttexts where ts &<# ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--after
select count(*) from tbl_ttexts where ts #>> timestamp '2001-05-01';
select count(*) from tbl_ttexts where ts #>> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttexts where ts #>> ttextinst 'AAA@2001-05-01';
select count(*) from tbl_ttexts where ts #>> ttexti '{AAA@2001-03-01, BBB@2001-05-01}';
select count(*) from tbl_ttexts where ts #>> ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttexts where ts #>> ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

--overafter
select count(*) from tbl_ttexts where ts #&> timestamp '2001-03-01';
select count(*) from tbl_ttexts where ts #&> period '[2001-03-01, 2001-06-01]';
select count(*) from tbl_ttexts where ts #&> ttextinst 'AAA@2001-03-01';
select count(*) from tbl_ttexts where ts #&> ttexti '{AAA@2001-03-01, BBB@2001-05-01}';
select count(*) from tbl_ttexts where ts #&> ttextseq '[AAA@2001-05-01, AAA@2001-07-01)';
select count(*) from tbl_ttexts where ts #&> ttexts '{[AAA@2001-05-01, AAA@2001-07-01), [BBB@2001-07-01, BBB@2001-09-01)}';

/******************************************************************************/
