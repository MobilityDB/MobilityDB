﻿-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint_spgist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_spgist_idx;

-------------------------------------------------------------------------------

ALTER TABLE test_georelativeposops ADD spgistidx BIGINT;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_spgist_idx ON tbl_tgeompoint USING SPGIST(temp);
CREATE INDEX tbl_tgeogpoint_spgist_idx ON tbl_tgeogpoint USING SPGIST(temp);

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g << temp )
WHERE op = '<<' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g >> temp )
WHERE op = '>>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &< temp )
WHERE op = '&<' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &> temp )
WHERE op = '&>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g <<| temp )
WHERE op = '<<|' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |>> temp )
WHERE op = '|>>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g &<| temp )
WHERE op = '&<|' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g |&> temp )
WHERE op = '|&>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t <<# temp )
WHERE op = '<<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #>> temp )
WHERE op = '#>>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t &<# temp )
WHERE op = '&<#' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t #&> temp )
WHERE op = '#&>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts <<# temp )
WHERE op = '<<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #>> temp )
WHERE op = '#>>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts &<# temp )
WHERE op = '&<#' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts #&> temp )
WHERE op = '#&>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p <<# temp )
WHERE op = '<<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p #>> temp )
WHERE op = '#>>' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p &<# temp )
WHERE op = '&<#' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p #&> temp )
WHERE op = '#&>' AND leftarg = 'period' AND rightarg = 'tgeogpoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps <<# temp )
WHERE op = '<<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #>> temp )
WHERE op = '#>>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps &<# temp )
WHERE op = '&<#' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps #&> temp )
WHERE op = '#&>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp << g )
WHERE op = '<<' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp >> g )
WHERE op = '>>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &< g )
WHERE op = '&<' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &> g )
WHERE op = '&>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp <<| g )
WHERE op = '<<|' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |>> g )
WHERE op = '|>>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp &<| g )
WHERE op = '&<|' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp |&> g )
WHERE op = '|&>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'period';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp << t2.temp )
WHERE op = '<<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp >> t2.temp )
WHERE op = '>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &< t2.temp )
WHERE op = '&<' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &> t2.temp )
WHERE op = '&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<| t2.temp )
WHERE op = '<<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |>> t2.temp )
WHERE op = '|>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<| t2.temp )
WHERE op = '&<|' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp |&> t2.temp )
WHERE op = '|&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <<# t2.temp )
WHERE op = '<<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #>> t2.temp )
WHERE op = '#>>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp &<# t2.temp )
WHERE op = '&<#' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp #&> t2.temp )
WHERE op = '#&>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp <<# t )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #>> t )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp &<# t )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp #&> t )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp <<# ts )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #>> ts )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp &<# ts )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp #&> ts )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp <<# p )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #>> p )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp &<# p )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp #&> p )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'period';

UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp <<# ps )
WHERE op = '<<#' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #>> ps )
WHERE op = '#>>' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp &<# ps )
WHERE op = '&<#' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_georelativeposops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp #&> ps )
WHERE op = '#&>' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint_spgist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_spgist_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_georelativeposops
WHERE noidx <> spgistidx 
ORDER BY op, leftarg, rightarg;

DROP TABLE test_georelativeposops;

-------------------------------------------------------------------------------
