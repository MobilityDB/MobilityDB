-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint3D_spgist_idx;

-------------------------------------------------------------------------------

ALTER TABLE test_georelativeposops3d ADD spgistidx BIGINT;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_spgist_idx ON tbl_tgeompoint3D USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g << temp )
WHERE op = '<<' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g >> temp )
WHERE op = '>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &< temp )
WHERE op = '&<' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &> temp )
WHERE op = '&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <<| temp )
WHERE op = '<<|' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |>> temp )
WHERE op = '|>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &<| temp )
WHERE op = '&<|' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g |&> temp )
WHERE op = '|&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <</ temp )
WHERE op = '<</' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g />> temp )
WHERE op = '/>>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g &</ temp )
WHERE op = '&</' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g /&> temp )
WHERE op = '/&>' AND leftarg = 'geomcollection3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeogpoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp << g )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp >> g )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &< g )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &> g )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <<| g )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |>> g )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &<| g )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp |&> g )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <</ g )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp />> g )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp &</ g )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp /&> g )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'geomcollection3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'timestampset';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'period';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'periodset';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <</ t2.temp )
WHERE op = '<</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp />> t2.temp )
WHERE op = '/>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &</ t2.temp )
WHERE op = '&</' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp /&> t2.temp )
WHERE op = '/&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint3D' AND rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'timestampset';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'period';

UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';
UPDATE test_georelativeposops3d
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeogpoint3D' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint3D_spgist_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_georelativeposops3d
WHERE noidx <> spgistidx 
ORDER BY op, leftarg, rightarg;

DROP TABLE test_georelativeposops3d;

-------------------------------------------------------------------------------
