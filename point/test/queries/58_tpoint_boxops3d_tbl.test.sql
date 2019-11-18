-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint3D_gist_idx;
DROP INDEX IF EXISTS tbl_tgeompoint3D_spgist_idx;

DROP INDEX IF EXISTS tbl_tgeogpoint3D_gist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint3D_spgist_idx;

-------------------------------------------------------------------------------

DROP TABLE if exists test_geoboundboxops;
CREATE TABLE test_geoboundboxops(
	op char(3), 
	leftarg text, 
	rightarg text, 
	noidx bigint,
	gistidx bigint,
	spgistidx bigint );

-------------------------------------------------------------------------------
-- <type> op tgeompoint3D

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'geomcollection3D', 'tgeompoint3D', count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestamptz', 'tgeompoint3D', count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestamptz', 'tgeompoint3D', count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestamptz', 'tgeompoint3D', count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestamptz', 'tgeompoint3D', count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestampset', 'tgeompoint3D', count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestampset', 'tgeompoint3D', count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestampset', 'tgeompoint3D', count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestampset', 'tgeompoint3D', count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'period', 'tgeompoint3D', count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'period', 'tgeompoint3D', count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'period', 'tgeompoint3D', count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'period', 'tgeompoint3D', count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'periodset', 'tgeompoint3D', count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'periodset', 'tgeompoint3D', count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'periodset', 'tgeompoint3D', count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'periodset', 'tgeompoint3D', count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'stbox', 'tgeompoint3D', count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'stbox', 'tgeompoint3D', count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'stbox', 'tgeompoint3D', count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'stbox', 'tgeompoint3D', count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b ~= temp;

-------------------------------------------------------------------------------
-- <type> op tgeogpoint3D

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'geogcollection3D', 'tgeogpoint3D', count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'geogcollection3D', 'tgeogpoint3D', count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'geogcollection3D', 'tgeogpoint3D', count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'geogcollection3D', 'tgeogpoint3D', count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestamptz', 'tgeogpoint3D', count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestamptz', 'tgeogpoint3D', count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestamptz', 'tgeogpoint3D', count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestamptz', 'tgeogpoint3D', count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestampset', 'tgeogpoint3D', count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestampset', 'tgeogpoint3D', count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestampset', 'tgeogpoint3D', count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestampset', 'tgeogpoint3D', count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'period', 'tgeogpoint3D', count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'period', 'tgeogpoint3D', count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'period', 'tgeogpoint3D', count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'period', 'tgeogpoint3D', count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'periodset', 'tgeogpoint3D', count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'periodset', 'tgeogpoint3D', count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'periodset', 'tgeogpoint3D', count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'periodset', 'tgeogpoint3D', count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps ~= temp;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'stbox', 'tgeogpoint3D', count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b && temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'stbox', 'tgeogpoint3D', count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b @> temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'stbox', 'tgeogpoint3D', count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b <@ temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'stbox', 'tgeogpoint3D', count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b ~= temp;

-------------------------------------------------------------------------------
--  tgeompoint3D op <type>

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp && g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp @> g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <@ g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint3D', 'geomcollection3D', count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp ~= g;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint3D', 'timestamptz', count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp && t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint3D', 'timestamptz', count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint3D', 'timestamptz', count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint3D', 'timestamptz', count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp ~= t;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint3D', 'timestampset', count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp && ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint3D', 'timestampset', count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint3D', 'timestampset', count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint3D', 'timestampset', count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp ~= ts;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint3D', 'period', count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp && p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint3D', 'period', count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp @> p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint3D', 'period', count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp <@ p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint3D', 'period', count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp ~= p;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint3D', 'periodset', count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp && ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint3D', 'periodset', count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp @> ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint3D', 'periodset', count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint3D', 'periodset', count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp ~= ps;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint3D', 'stbox', count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp && b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint3D', 'stbox', count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp @> b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint3D', 'stbox', count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp <@ b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint3D', 'stbox', count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp ~= b;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp && t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeompoint3D', 'tgeompoint3D', count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp ~= t2.temp;

-------------------------------------------------------------------------------
--  tgeogpoint3D op <type>

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint3D', 'geogcollection3D', count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp && g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint3D', 'geogcollection3D', count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp @> g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint3D', 'geogcollection3D', count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp <@ g;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint3D', 'geogcollection3D', count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp ~= g;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint3D', 'timestamptz', count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp && t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint3D', 'timestamptz', count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint3D', 'timestamptz', count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint3D', 'timestamptz', count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp ~= t;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint3D', 'timestampset', count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp && ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint3D', 'timestampset', count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint3D', 'timestampset', count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint3D', 'timestampset', count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp ~= ts;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint3D', 'period', count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp && p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint3D', 'period', count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp @> p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint3D', 'period', count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp <@ p;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint3D', 'period', count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp ~= p;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint3D', 'periodset', count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp && ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint3D', 'periodset', count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp @> ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint3D', 'periodset', count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint3D', 'periodset', count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp ~= ps;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint3D', 'stbox', count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp && b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint3D', 'stbox', count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp @> b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint3D', 'stbox', count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp <@ b;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint3D', 'stbox', count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp ~= b;

INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tgeogpoint3D', 'tgeogpoint3D', count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp && t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tgeogpoint3D', 'tgeogpoint3D', count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tgeogpoint3D', 'tgeogpoint3D', count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_geoboundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tgeogpoint3D', 'tgeogpoint3D', count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp ~= t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_gist_idx ON tbl_tgeompoint3D USING GIST(temp);
CREATE INDEX tbl_tgeogpoint3D_gist_idx ON tbl_tgeogpoint3D USING GIST(temp);

-------------------------------------------------------------------------------
-- <type> op tgeompoint3D

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g && temp )
WHERE op = '&&' and leftarg = 'geomcollection3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g @> temp )
WHERE op = '@>' and leftarg = 'geomcollection3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <@ temp )
WHERE op = '<@' and leftarg = 'geomcollection3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g ~= temp )
WHERE op = '~=' and leftarg = 'geomcollection3D' and rightarg = 'tgeompoint3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t && temp )
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t @> temp )
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeompoint3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts && temp )
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts @> temp )
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts <@ temp )
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts ~= temp )
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeompoint3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p && temp )
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p @> temp )
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p <@ temp )
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p ~= temp )
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tgeompoint3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps && temp )
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps @> temp )
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps <@ temp )
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps ~= temp )
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tgeompoint3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------
-- <type> op tgeogpoint3D

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g && temp )
WHERE op = '&&' and leftarg = 'geogcollection3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g @> temp )
WHERE op = '@>' and leftarg = 'geogcollection3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g <@ temp )
WHERE op = '<@' and leftarg = 'geogcollection3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g ~= temp )
WHERE op = '~=' and leftarg = 'geogcollection3D' and rightarg = 'tgeogpoint3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t && temp )
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t @> temp )
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts && temp )
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts @> temp )
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts <@ temp )
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts ~= temp )
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeogpoint3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p && temp )
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p @> temp )
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p <@ temp )
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p ~= temp )
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tgeogpoint3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps && temp )
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps @> temp )
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps <@ temp )
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps ~= temp )
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tgeogpoint3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------
-- tgeompoint3D op <type>

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp && g )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'geomcollection3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp @> g )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'geomcollection3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <@ g )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'geomcollection3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp ~= g )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'geomcollection3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'timestamptz';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'timestampset';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp && p )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp @> p )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp <@ p )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp ~= p )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'period';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp && ps )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'periodset';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp && b )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp @> b )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp <@ b )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp ~= b )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'stbox';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------
-- tgeogpoint3D op <type>

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp && g )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'geogcollection3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp @> g )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'geogcollection3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp <@ g )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'geogcollection3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp ~= g )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'geogcollection3D';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'timestamptz';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'timestampset';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp && p )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp @> p )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp <@ p )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp ~= p )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'period';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp && ps )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'periodset';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp && b )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp @> b )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp <@ b )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp ~= b )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'stbox';

UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint3D_gist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint3D_gist_idx;

CREATE INDEX tbl_tgeompoint3D_spgist_idx ON tbl_tgeompoint3D USING SPGIST(temp);
CREATE INDEX tbl_tgeogpoint3D_spgist_idx ON tbl_tgeogpoint3D USING SPGIST(temp);

-------------------------------------------------------------------------------
-- <type> op tgeompoint3D

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g && temp )
WHERE op = '&&' and leftarg = 'geomcollection3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g @> temp )
WHERE op = '@>' and leftarg = 'geomcollection3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g <@ temp )
WHERE op = '<@' and leftarg = 'geomcollection3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry3D, tbl_tgeompoint3D WHERE g ~= temp )
WHERE op = '~=' and leftarg = 'geomcollection3D' and rightarg = 'tgeompoint3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t && temp )
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t @> temp )
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint3D WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeompoint3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts && temp )
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts @> temp )
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts <@ temp )
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint3D WHERE ts ~= temp )
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeompoint3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p && temp )
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p @> temp )
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p <@ temp )
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint3D WHERE p ~= temp )
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tgeompoint3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps && temp )
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps @> temp )
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps <@ temp )
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint3D WHERE ps ~= temp )
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tgeompoint3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint3D WHERE b ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------
-- <type> op tgeogpoint3D

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g && temp )
WHERE op = '&&' and leftarg = 'geogcollection3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g @> temp )
WHERE op = '@>' and leftarg = 'geogcollection3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g <@ temp )
WHERE op = '<@' and leftarg = 'geogcollection3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geography3D, tbl_tgeogpoint3D WHERE g ~= temp )
WHERE op = '~=' and leftarg = 'geogcollection3D' and rightarg = 'tgeogpoint3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t && temp )
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t @> temp )
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t <@ temp )
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint3D WHERE t ~= temp )
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeogpoint3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts && temp )
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts @> temp )
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts <@ temp )
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint3D WHERE ts ~= temp )
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeogpoint3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p && temp )
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p @> temp )
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p <@ temp )
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint3D WHERE p ~= temp )
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tgeogpoint3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps && temp )
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps @> temp )
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps <@ temp )
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint3D WHERE ps ~= temp )
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tgeogpoint3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b && temp )
WHERE op = '&&' and leftarg = 'stbox' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b @> temp )
WHERE op = '@>' and leftarg = 'stbox' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b <@ temp )
WHERE op = '<@' and leftarg = 'stbox' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeogpoint3D WHERE b ~= temp )
WHERE op = '~=' and leftarg = 'stbox' and rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------
-- tgeompoint3D op <type>

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp && g )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'geomcollection3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp @> g )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'geomcollection3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp <@ g )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'geomcollection3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_geometry3D WHERE temp ~= g )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'geomcollection3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'timestamptz';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'timestampset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp && p )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp @> p )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp <@ p )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_period WHERE temp ~= p )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'period';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp && ps )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'periodset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp && b )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp @> b )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp <@ b )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D, tbl_stbox WHERE temp ~= b )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'stbox';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tgeompoint3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tgeompoint3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tgeompoint3D' and rightarg = 'tgeompoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint3D t1, tbl_tgeompoint3D t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tgeompoint3D' and rightarg = 'tgeompoint3D';

-------------------------------------------------------------------------------
-- tgeogpoint3D op <type>

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp && g )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'geogcollection3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp @> g )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'geogcollection3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp <@ g )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'geogcollection3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_geography3D WHERE temp ~= g )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'geogcollection3D';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'timestamptz';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'timestampset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp && p )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp @> p )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp <@ p )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_period WHERE temp ~= p )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'period';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp && ps )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'periodset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp && b )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp @> b )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp <@ b )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D, tbl_stbox WHERE temp ~= b )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'stbox';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' and leftarg = 'tgeogpoint3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' and leftarg = 'tgeogpoint3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' and leftarg = 'tgeogpoint3D' and rightarg = 'tgeogpoint3D';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint3D t1, tbl_tgeogpoint3D t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' and leftarg = 'tgeogpoint3D' and rightarg = 'tgeogpoint3D';

-------------------------------------------------------------------------------

SELECT * FROM test_geoboundboxops
WHERE noidx <> gistidx or noidx <> spgistidx or gistidx <> spgistidx;

DROP INDEX IF EXISTS tbl_tgeompoint3D_spgist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint3D_spgist_idx;

DROP TABLE test_geoboundboxops;

-------------------------------------------------------------------------------
