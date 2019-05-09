﻿/*****************************************************************************/

DROP INDEX IF EXISTS tbl_tgeompoint_gist_idx;
DROP INDEX IF EXISTS tbl_tgeompoint_spgist_idx;

DROP INDEX IF EXISTS tbl_tgeogpoint_gist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_spgist_idx;

/*****************************************************************************/

DROP TABLE if exists test_geoboundboxops;
CREATE TABLE test_geoboundboxops(
	op char(3), 
	leftarg text, 
	rightarg text, 
	noidx bigint,
	gistidx bigint,
	spgistidx bigint );

/*****************************************************************************/
-- <type> op tgeompoint

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'geomcollection', 'tgeompoint', count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'geomcollection', 'tgeompoint', count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'geomcollection', 'tgeompoint', count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'geomcollection', 'tgeompoint', count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestamptz', 'tgeompoint', count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestamptz', 'tgeompoint', count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestamptz', 'tgeompoint', count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestamptz', 'tgeompoint', count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestampset', 'tgeompoint', count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestampset', 'tgeompoint', count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestampset', 'tgeompoint', count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestampset', 'tgeompoint', count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'period', 'tgeompoint', count(*) FROM tbl_period, tbl_tgeompoint WHERE p && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'period', 'tgeompoint', count(*) FROM tbl_period, tbl_tgeompoint WHERE p @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'period', 'tgeompoint', count(*) FROM tbl_period, tbl_tgeompoint WHERE p <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'period', 'tgeompoint', count(*) FROM tbl_period, tbl_tgeompoint WHERE p ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'periodset', 'tgeompoint', count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'periodset', 'tgeompoint', count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'periodset', 'tgeompoint', count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'periodset', 'tgeompoint', count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'gbox', 'tgeompoint', count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'gbox', 'tgeompoint', count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'gbox', 'tgeompoint', count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'gbox', 'tgeompoint', count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b ~= temp;

/*****************************************************************************/
-- <type> op tgeogpoint

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'geogcollection', 'tgeogpoint', count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'geogcollection', 'tgeogpoint', count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'geogcollection', 'tgeogpoint', count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'geogcollection', 'tgeogpoint', count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestamptz', 'tgeogpoint', count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestamptz', 'tgeogpoint', count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestamptz', 'tgeogpoint', count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestamptz', 'tgeogpoint', count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestampset', 'tgeogpoint', count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestampset', 'tgeogpoint', count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestampset', 'tgeogpoint', count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestampset', 'tgeogpoint', count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'period', 'tgeogpoint', count(*) FROM tbl_period, tbl_tgeogpoint WHERE p && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'period', 'tgeogpoint', count(*) FROM tbl_period, tbl_tgeogpoint WHERE p @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'period', 'tgeogpoint', count(*) FROM tbl_period, tbl_tgeogpoint WHERE p <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'period', 'tgeogpoint', count(*) FROM tbl_period, tbl_tgeogpoint WHERE p ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'periodset', 'tgeogpoint', count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'periodset', 'tgeogpoint', count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'periodset', 'tgeogpoint', count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'periodset', 'tgeogpoint', count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'gbox', 'tgeogpoint', count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'gbox', 'tgeogpoint', count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'gbox', 'tgeogpoint', count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'gbox', 'tgeogpoint', count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b ~= temp;

/*****************************************************************************/
--  tgeompoint op <type>

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint', 'geomcollection', count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp && g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint', 'geomcollection', count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp @> g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint', 'geomcollection', count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp <@ g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint', 'geomcollection', count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp ~= g;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint', 'timestamptz', count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp && t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint', 'timestamptz', count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint', 'timestamptz', count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint', 'timestamptz', count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp ~= t;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint', 'timestampset', count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp && ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint', 'timestampset', count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint', 'timestampset', count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint', 'timestampset', count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp ~= ts;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint', 'period', count(*) FROM tbl_tgeompoint, tbl_period WHERE temp && p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint', 'period', count(*) FROM tbl_tgeompoint, tbl_period WHERE temp @> p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint', 'period', count(*) FROM tbl_tgeompoint, tbl_period WHERE temp <@ p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint', 'period', count(*) FROM tbl_tgeompoint, tbl_period WHERE temp ~= p;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint', 'periodset', count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp && ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint', 'periodset', count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp @> ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint', 'periodset', count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint', 'periodset', count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp ~= ps;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint', 'gbox', count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp && b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint', 'gbox', count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp @> b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint', 'gbox', count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp <@ b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint', 'gbox', count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp ~= b;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint', 'tgeompoint', count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp;

/*****************************************************************************/
--  tgeogpoint op <type>

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint', 'geogcollection', count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp && g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint', 'geogcollection', count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp @> g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint', 'geogcollection', count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp <@ g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint', 'geogcollection', count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp ~= g;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint', 'timestamptz', count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp && t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint', 'timestamptz', count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint', 'timestamptz', count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint', 'timestamptz', count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp ~= t;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint', 'timestampset', count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp && ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint', 'timestampset', count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint', 'timestampset', count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint', 'timestampset', count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp ~= ts;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint', 'period', count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp && p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint', 'period', count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp @> p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint', 'period', count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp <@ p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint', 'period', count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp ~= p;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint', 'periodset', count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp && ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint', 'periodset', count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp @> ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint', 'periodset', count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint', 'periodset', count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp ~= ps;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint', 'gbox', count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp && b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint', 'gbox', count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp @> b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint', 'gbox', count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp <@ b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint', 'gbox', count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp ~= b;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint', 'tgeogpoint', count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp && t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint', 'tgeogpoint', count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint', 'tgeogpoint', count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint', 'tgeogpoint', count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp ~= t2.temp;

/*****************************************************************************/

CREATE INDEX tbl_tgeompoint_gist_idx ON tbl_tgeompoint USING GIST(temp);
CREATE INDEX tbl_tgeogpoint_gist_idx ON tbl_tgeogpoint USING GIST(temp);

/*****************************************************************************/
-- <type> op tgeompoint

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g && temp )
WHERE op = '&&' and leftarg = 'geomcollection' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g @> temp )
WHERE op = '@>' and leftarg = 'geomcollection' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g <@ temp )
WHERE op = '<@' and leftarg = 'geomcollection' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g ~= temp )
WHERE op = '~=' and leftarg = 'geomcollection' and rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t && temp )
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t @> temp )
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts && temp )
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts @> temp )
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts <@ temp )
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts ~= temp )
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p && temp )
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p @> temp )
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p <@ temp )
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p ~= temp )
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps && temp )
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps @> temp )
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps <@ temp )
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps ~= temp )
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b && temp )
WHERE op = '&&' and leftarg = 'gbox' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b @> temp )
WHERE op = '@>' and leftarg = 'gbox' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b <@ temp )
WHERE op = '<@' and leftarg = 'gbox' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b ~= temp )
WHERE op = '~=' and leftarg = 'gbox' and rightarg = 'tgeompoint';

/*****************************************************************************/
-- <type> op tgeogpoint

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g && temp )
WHERE op = '&&' and leftarg = 'geogcollection' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g @> temp )
WHERE op = '@>' and leftarg = 'geogcollection' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g <@ temp )
WHERE op = '<@' and leftarg = 'geogcollection' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g ~= temp )
WHERE op = '~=' and leftarg = 'geogcollection' and rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t && temp )
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t @> temp )
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts && temp )
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts @> temp )
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts <@ temp )
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts ~= temp )
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p && temp )
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p @> temp )
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p <@ temp )
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p ~= temp )
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps && temp )
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps @> temp )
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps <@ temp )
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps ~= temp )
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b && temp )
WHERE op = '&&' and leftarg = 'gbox' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b @> temp )
WHERE op = '@>' and leftarg = 'gbox' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b <@ temp )
WHERE op = '<@' and leftarg = 'gbox' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b ~= temp )
WHERE op = '~=' and leftarg = 'gbox' and rightarg = 'tgeogpoint';

/*****************************************************************************/
-- tgeompoint op <type>

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp && g )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'geomcollection';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp @> g )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'geomcollection';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp <@ g )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'geomcollection';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp ~= g )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'geomcollection';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'timestampset';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp && p )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp @> p )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp <@ p )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp ~= p )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'period';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp && ps )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'periodset';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp && b )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp @> b )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp <@ b )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp ~= b )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'gbox';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';

/*****************************************************************************/
-- tgeogpoint op <type>

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp && g )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'geogcollection';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp @> g )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'geogcollection';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp <@ g )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'geogcollection';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp ~= g )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'geogcollection';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp && p )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp @> p )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp <@ p )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp ~= p )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'period';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp && ps )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'periodset';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp && b )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp @> b )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp <@ b )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp ~= b )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'gbox';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'tgeogpoint';

/*****************************************************************************/

DROP INDEX IF EXISTS tbl_tgeompoint_gist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_gist_idx;

CREATE INDEX tbl_tgeompoint_spgist_idx ON tbl_tgeompoint USING SPGIST(temp);
CREATE INDEX tbl_tgeogpoint_spgist_idx ON tbl_tgeogpoint USING SPGIST(temp);

/*****************************************************************************/
-- <type> op tgeompoint

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g && temp )
WHERE op = '&&' and leftarg = 'geomcollection' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g @> temp )
WHERE op = '@>' and leftarg = 'geomcollection' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g <@ temp )
WHERE op = '<@' and leftarg = 'geomcollection' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geomcollection, tbl_tgeompoint WHERE g ~= temp )
WHERE op = '~=' and leftarg = 'geomcollection' and rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t && temp )
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t @> temp )
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts && temp )
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts @> temp )
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts <@ temp )
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts ~= temp )
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p && temp )
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p @> temp )
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p <@ temp )
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p ~= temp )
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps && temp )
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps @> temp )
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps <@ temp )
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps ~= temp )
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b && temp )
WHERE op = '&&' and leftarg = 'gbox' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b @> temp )
WHERE op = '@>' and leftarg = 'gbox' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b <@ temp )
WHERE op = '<@' and leftarg = 'gbox' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeompoint WHERE b ~= temp )
WHERE op = '~=' and leftarg = 'gbox' and rightarg = 'tgeompoint';

/*****************************************************************************/
-- <type> op tgeogpoint

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g && temp )
WHERE op = '&&' and leftarg = 'geogcollection' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g @> temp )
WHERE op = '@>' and leftarg = 'geogcollection' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g <@ temp )
WHERE op = '<@' and leftarg = 'geogcollection' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geogcollection, tbl_tgeogpoint WHERE g ~= temp )
WHERE op = '~=' and leftarg = 'geogcollection' and rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t && temp )
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t @> temp )
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts && temp )
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts @> temp )
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts <@ temp )
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts ~= temp )
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p && temp )
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p @> temp )
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p <@ temp )
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p ~= temp )
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps && temp )
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps @> temp )
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps <@ temp )
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps ~= temp )
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b && temp )
WHERE op = '&&' and leftarg = 'gbox' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b @> temp )
WHERE op = '@>' and leftarg = 'gbox' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b <@ temp )
WHERE op = '<@' and leftarg = 'gbox' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_gbox, tbl_tgeogpoint WHERE b ~= temp )
WHERE op = '~=' and leftarg = 'gbox' and rightarg = 'tgeogpoint';

/*****************************************************************************/
-- tgeompoint op <type>

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp && g )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'geomcollection';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp @> g )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'geomcollection';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp <@ g )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'geomcollection';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geomcollection WHERE temp ~= g )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'geomcollection';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'timestamptz';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'timestampset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp && p )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp @> p )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp <@ p )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp ~= p )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'period';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp && ps )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'periodset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp && b )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp @> b )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp <@ b )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_gbox WHERE temp ~= b )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'gbox';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tgeompoint' and rightarg = 'tgeompoint';

/*****************************************************************************/
-- tgeogpoint op <type>

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp && g )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'geogcollection';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp @> g )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'geogcollection';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp <@ g )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'geogcollection';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geogcollection WHERE temp ~= g )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'geogcollection';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'timestamptz';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'timestampset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp && p )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp @> p )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp <@ p )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp ~= p )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'period';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp && ps )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'periodset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp && b )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp @> b )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp <@ b )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'gbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_gbox WHERE temp ~= b )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'gbox';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tgeogpoint' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tgeogpoint' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tgeogpoint' and rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tgeogpoint' and rightarg = 'tgeogpoint';

/*****************************************************************************/

SELECT * FROM test_geoboundboxops
WHERE noidx <> gistidx or noidx <> spgistidx or gistidx <> spgistidx; 

/*****************************************************************************/



