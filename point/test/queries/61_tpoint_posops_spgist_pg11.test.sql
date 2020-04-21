-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint_spgist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_spgist_idx;

-------------------------------------------------------------------------------

ALTER TABLE test_georelativeposops ADD spgistidx bigint ;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_spgist_idx ON tbl_tgeompoint USING SPGIST(temp);
CREATE INDEX tbl_tgeogpoint_spgist_idx ON tbl_tgeogpoint USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g << temp )
WHERE op = '<<' and leftarg = 'geometry' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g >> temp )
WHERE op = '>>' and leftarg = 'geometry' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &< temp )
WHERE op = '&<' and leftarg = 'geometry' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &> temp )
WHERE op = '&>' and leftarg = 'geometry' and rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g <<| temp )
WHERE op = '<<|' and leftarg = 'geometry' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |>> temp )
WHERE op = '|>>' and leftarg = 'geometry' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &<| temp )
WHERE op = '&<|' and leftarg = 'geometry' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |&> temp )
WHERE op = '|&>' and leftarg = 'geometry' and rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts <<# temp )
WHERE op = '<<#' and leftarg = 'timestampset' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #>> temp )
WHERE op = '#>>' and leftarg = 'timestampset' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts &<# temp )
WHERE op = '&<#' and leftarg = 'timestampset' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #&> temp )
WHERE op = '#&>' and leftarg = 'timestampset' and rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p <<# temp )
WHERE op = '<<#' and leftarg = 'period' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p #>> temp )
WHERE op = '#>>' and leftarg = 'period' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p &<# temp )
WHERE op = '&<#' and leftarg = 'period' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p #&> temp )
WHERE op = '#&>' and leftarg = 'period' and rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps <<# temp )
WHERE op = '<<#' and leftarg = 'periodset' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #>> temp )
WHERE op = '#>>' and leftarg = 'periodset' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps &<# temp )
WHERE op = '&<#' and leftarg = 'periodset' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #&> temp )
WHERE op = '#&>' and leftarg = 'periodset' and rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t <<# temp )
WHERE op = '<<#' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #>> temp )
WHERE op = '#>>' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t &<# temp )
WHERE op = '&<#' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #&> temp )
WHERE op = '#&>' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts <<# temp )
WHERE op = '<<#' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #>> temp )
WHERE op = '#>>' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts &<# temp )
WHERE op = '&<#' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #&> temp )
WHERE op = '#&>' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p <<# temp )
WHERE op = '<<#' and leftarg = 'period' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p #>> temp )
WHERE op = '#>>' and leftarg = 'period' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p &<# temp )
WHERE op = '&<#' and leftarg = 'period' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p #&> temp )
WHERE op = '#&>' and leftarg = 'period' and rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps <<# temp )
WHERE op = '<<#' and leftarg = 'periodset' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #>> temp )
WHERE op = '#>>' and leftarg = 'periodset' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps &<# temp )
WHERE op = '&<#' and leftarg = 'periodset' and rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #&> temp )
WHERE op = '#&>' and leftarg = 'periodset' and rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp << g )
WHERE op = '<<' and leftarg = 'tgeompoint' and rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp >> g )
WHERE op = '>>' and leftarg = 'tgeompoint' and rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &< g )
WHERE op = '&<' and leftarg = 'tgeompoint' and rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &> g )
WHERE op = '&>' and leftarg = 'tgeompoint' and rightarg = 'geometry';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp <<| g )
WHERE op = '<<|' and leftarg = 'tgeompoint' and rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |>> g )
WHERE op = '|>>' and leftarg = 'tgeompoint' and rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &<| g )
WHERE op = '&<|' and leftarg = 'tgeompoint' and rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |&> g )
WHERE op = '|&>' and leftarg = 'tgeompoint' and rightarg = 'geometry';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' and leftarg = 'tgeompoint' and rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' and leftarg = 'tgeompoint' and rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' and leftarg = 'tgeompoint' and rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' and leftarg = 'tgeompoint' and rightarg = 'timestampset';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' and leftarg = 'tgeompoint' and rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' and leftarg = 'tgeompoint' and rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' and leftarg = 'tgeompoint' and rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' and leftarg = 'tgeompoint' and rightarg = 'period';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' and leftarg = 'tgeompoint' and rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' and leftarg = 'tgeompoint' and rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' and leftarg = 'tgeompoint' and rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' and leftarg = 'tgeompoint' and rightarg = 'periodset';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' and leftarg = 'tgeogpoint' and rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' and leftarg = 'tgeogpoint' and rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' and leftarg = 'tgeogpoint' and rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' and leftarg = 'tgeogpoint' and rightarg = 'period';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' and leftarg = 'tgeogpoint' and rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' and leftarg = 'tgeogpoint' and rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' and leftarg = 'tgeogpoint' and rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' and leftarg = 'tgeogpoint' and rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint_spgist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_spgist_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_georelativeposops
WHERE noidx <> spgistidx 
ORDER BY op, leftarg, rightarg;

DROP TABLE test_georelativeposops;

-------------------------------------------------------------------------------
