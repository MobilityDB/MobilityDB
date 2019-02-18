/*****************************************************************************/

CREATE INDEX tbl_tgeompointinst_gist_idx ON tbl_tgeompointinst USING GIST(inst);
CREATE INDEX tbl_tgeogpointinst_gist_idx ON tbl_tgeogpointinst USING GIST(inst);
CREATE INDEX tbl_tgeompointi_gist_idx ON tbl_tgeompointi USING GIST(ti);
CREATE INDEX tbl_tgeogpointi_gist_idx ON tbl_tgeogpointi USING GIST(ti);
CREATE INDEX tbl_tgeompointseq_gist_idx ON tbl_tgeompointseq USING GIST(seq);
CREATE INDEX tbl_tgeogpointseq_gist_idx ON tbl_tgeogpointseq USING GIST(seq);
CREATE INDEX tbl_tgeompoints_gist_idx ON tbl_tgeompoints USING GIST(ts);
CREATE INDEX tbl_tgeogpoints_gist_idx ON tbl_tgeogpoints USING GIST(ts);

/*****************************************************************************/

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointinst where g && inst )
where op = '&&' and leftarg = 'geompoint' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointinst where g @> inst )
where op = '@>' and leftarg = 'geompoint' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointinst where g <@ inst )
where op = '<@' and leftarg = 'geompoint' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointinst where g ~= inst )
where op = '~=' and leftarg = 'geompoint' and rightarg = 'tgeompointinst';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointi where g && ti )
where op = '&&' and leftarg = 'geompoint' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointi where g @> ti )
where op = '@>' and leftarg = 'geompoint' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointi where g <@ ti )
where op = '<@' and leftarg = 'geompoint' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointi where g ~= ti )
where op = '~=' and leftarg = 'geompoint' and rightarg = 'tgeompointi';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointseq where g && seq )
where op = '&&' and leftarg = 'geompoint' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointseq where g @> seq )
where op = '@>' and leftarg = 'geompoint' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointseq where g <@ seq )
where op = '<@' and leftarg = 'geompoint' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompointseq where g ~= seq )
where op = '~=' and leftarg = 'geompoint' and rightarg = 'tgeompointseq';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompoints where g && ts )
where op = '&&' and leftarg = 'geompoint' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompoints where g @> ts )
where op = '@>' and leftarg = 'geompoint' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompoints where g <@ ts )
where op = '<@' and leftarg = 'geompoint' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompoint, tbl_tgeompoints where g ~= ts )
where op = '~=' and leftarg = 'geompoint' and rightarg = 'tgeompoints';

/*****************************************************************************/

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointinst where g && inst )
where op = '&&' and leftarg = 'geompolygon' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointinst where g @> inst )
where op = '@>' and leftarg = 'geompolygon' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointinst where g <@ inst )
where op = '<@' and leftarg = 'geompolygon' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointinst where g ~= inst )
where op = '~=' and leftarg = 'geompolygon' and rightarg = 'tgeompointinst';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointi where g && ti )
where op = '&&' and leftarg = 'geompolygon' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointi where g @> ti )
where op = '@>' and leftarg = 'geompolygon' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointi where g <@ ti )
where op = '<@' and leftarg = 'geompolygon' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointi where g ~= ti )
where op = '~=' and leftarg = 'geompolygon' and rightarg = 'tgeompointi';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointseq where g && seq )
where op = '&&' and leftarg = 'geompolygon' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointseq where g @> seq )
where op = '@>' and leftarg = 'geompolygon' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointseq where g <@ seq )
where op = '<@' and leftarg = 'geompolygon' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompointseq where g ~= seq )
where op = '~=' and leftarg = 'geompolygon' and rightarg = 'tgeompointseq';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompoints where g && ts )
where op = '&&' and leftarg = 'geompolygon' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompoints where g @> ts )
where op = '@>' and leftarg = 'geompolygon' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompoints where g <@ ts )
where op = '<@' and leftarg = 'geompolygon' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_geompolygon, tbl_tgeompoints where g ~= ts )
where op = '~=' and leftarg = 'geompolygon' and rightarg = 'tgeompoints';

/*****************************************************************************/

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointinst where t && inst )
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointinst where t @> inst )
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointinst where t <@ inst )
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointinst where t ~= inst )
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeompointinst';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointi where t && ti )
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointi where t @> ti )
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointi where t <@ ti )
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointi where t ~= ti )
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeompointi';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointseq where t && seq )
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointseq where t @> seq )
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointseq where t <@ seq )
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompointseq where t ~= seq )
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeompointseq';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompoints where t && ts )
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompoints where t @> ts )
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompoints where t <@ ts )
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tgeompoints where t ~= ts )
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tgeompoints';

/*****************************************************************************/

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointinst where ts && inst )
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointinst where ts @> inst )
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointinst where ts <@ inst )
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointinst where ts ~= inst )
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeompointinst';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointi where ts && ti )
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointi where ts @> ti )
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointi where ts <@ ti )
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointi where ts ~= ti )
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeompointi';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointseq where ts && seq )
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointseq where ts @> seq )
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointseq where ts <@ seq )
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tgeompointseq where ts ~= seq )
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeompointseq';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tgeompoints t2 where t1.ts && t2.ts )
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tgeompoints t2 where t1.ts @> t2.ts )
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tgeompoints t2 where t1.ts <@ t2.ts )
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tgeompoints t2 where t1.ts ~= t2.ts )
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tgeompoints';

/*****************************************************************************/

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointinst where p && inst )
where op = '&&' and leftarg = 'period' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointinst where p @> inst )
where op = '@>' and leftarg = 'period' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointinst where p <@ inst )
where op = '<@' and leftarg = 'period' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointinst where p ~= inst )
where op = '~=' and leftarg = 'period' and rightarg = 'tgeompointinst';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointi where p && ti )
where op = '&&' and leftarg = 'period' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointi where p @> ti )
where op = '@>' and leftarg = 'period' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointi where p <@ ti )
where op = '<@' and leftarg = 'period' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointi where p ~= ti )
where op = '~=' and leftarg = 'period' and rightarg = 'tgeompointi';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointseq where p && seq )
where op = '&&' and leftarg = 'period' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointseq where p @> seq )
where op = '@>' and leftarg = 'period' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointseq where p <@ seq )
where op = '<@' and leftarg = 'period' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompointseq where p ~= seq )
where op = '~=' and leftarg = 'period' and rightarg = 'tgeompointseq';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompoints where p && ts )
where op = '&&' and leftarg = 'period' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompoints where p @> ts )
where op = '@>' and leftarg = 'period' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompoints where p <@ ts )
where op = '<@' and leftarg = 'period' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tgeompoints where p ~= ts )
where op = '~=' and leftarg = 'period' and rightarg = 'tgeompoints';

/*****************************************************************************/

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointinst where ps && inst )
where op = '&&' and leftarg = 'periodset' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointinst where ps @> inst )
where op = '@>' and leftarg = 'periodset' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointinst where ps <@ inst )
where op = '<@' and leftarg = 'periodset' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointinst where ps ~= inst )
where op = '~=' and leftarg = 'periodset' and rightarg = 'tgeompointinst';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointi where ps && ti )
where op = '&&' and leftarg = 'periodset' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointi where ps @> ti )
where op = '@>' and leftarg = 'periodset' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointi where ps <@ ti )
where op = '<@' and leftarg = 'periodset' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointi where ps ~= ti )
where op = '~=' and leftarg = 'periodset' and rightarg = 'tgeompointi';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointseq where ps && seq )
where op = '&&' and leftarg = 'periodset' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointseq where ps @> seq )
where op = '@>' and leftarg = 'periodset' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointseq where ps <@ seq )
where op = '<@' and leftarg = 'periodset' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompointseq where ps ~= seq )
where op = '~=' and leftarg = 'periodset' and rightarg = 'tgeompointseq';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompoints where ps && ts )
where op = '&&' and leftarg = 'periodset' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompoints where ps @> ts )
where op = '@>' and leftarg = 'periodset' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompoints where ps <@ ts )
where op = '<@' and leftarg = 'periodset' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tgeompoints where ps ~= ts )
where op = '~=' and leftarg = 'periodset' and rightarg = 'tgeompoints';

/*****************************************************************************/

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointinst where b && inst )
where op = '&&' and leftarg = 'gbox' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointinst where b @> inst )
where op = '@>' and leftarg = 'gbox' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointinst where b <@ inst )
where op = '<@' and leftarg = 'gbox' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointinst where b ~= inst )
where op = '~=' and leftarg = 'gbox' and rightarg = 'tgeompointinst';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointi where b && ti )
where op = '&&' and leftarg = 'gbox' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointi where b @> ti )
where op = '@>' and leftarg = 'gbox' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointi where b <@ ti )
where op = '<@' and leftarg = 'gbox' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointi where b ~= ti )
where op = '~=' and leftarg = 'gbox' and rightarg = 'tgeompointi';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointseq where b && seq )
where op = '&&' and leftarg = 'gbox' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointseq where b @> seq )
where op = '@>' and leftarg = 'gbox' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointseq where b <@ seq )
where op = '<@' and leftarg = 'gbox' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompointseq where b ~= seq )
where op = '~=' and leftarg = 'gbox' and rightarg = 'tgeompointseq';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompoints where b && ts )
where op = '&&' and leftarg = 'gbox' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompoints where b @> ts )
where op = '@>' and leftarg = 'gbox' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompoints where b <@ ts )
where op = '<@' and leftarg = 'gbox' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_gbox, tbl_tgeompoints where b ~= ts )
where op = '~=' and leftarg = 'gbox' and rightarg = 'tgeompoints';

/*****************************************************************************/

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_geompoint where inst && g )
where op = '&&' and leftarg = 'tgeompointinst' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_geompoint where inst @> g )
where op = '@>' and leftarg = 'tgeompointinst' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_geompoint where inst <@ g )
where op = '<@' and leftarg = 'tgeompointinst' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_geompoint where inst ~= g )
where op = '~=' and leftarg = 'tgeompointinst' and rightarg = 'geompoint';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst && g )
where op = '&&' and leftarg = 'tgeompointinst' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst @> g )
where op = '@>' and leftarg = 'tgeompointinst' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst <@ g )
where op = '<@' and leftarg = 'tgeompointinst' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_geompolygon where inst ~= g )
where op = '~=' and leftarg = 'tgeompointinst' and rightarg = 'geompolygon';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_timestamptz where inst && t )
where op = '&&' and leftarg = 'tgeompointinst' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_timestamptz where inst @> t )
where op = '@>' and leftarg = 'tgeompointinst' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_timestamptz where inst <@ t )
where op = '<@' and leftarg = 'tgeompointinst' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_timestamptz where inst ~= t )
where op = '~=' and leftarg = 'tgeompointinst' and rightarg = 'timestamptz';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_timestampset where inst && ts )
where op = '&&' and leftarg = 'tgeompointinst' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_timestampset where inst @> ts )
where op = '@>' and leftarg = 'tgeompointinst' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_timestampset where inst <@ ts )
where op = '<@' and leftarg = 'tgeompointinst' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_timestampset where inst ~= ts )
where op = '~=' and leftarg = 'tgeompointinst' and rightarg = 'timestampset';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_period where inst && p )
where op = '&&' and leftarg = 'tgeompointinst' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_period where inst @> p )
where op = '@>' and leftarg = 'tgeompointinst' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_period where inst <@ p )
where op = '<@' and leftarg = 'tgeompointinst' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_period where inst ~= p )
where op = '~=' and leftarg = 'tgeompointinst' and rightarg = 'period';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_periodset where inst && ps )
where op = '&&' and leftarg = 'tgeompointinst' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_periodset where inst @> ps )
where op = '@>' and leftarg = 'tgeompointinst' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_periodset where inst <@ ps )
where op = '<@' and leftarg = 'tgeompointinst' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_periodset where inst ~= ps )
where op = '~=' and leftarg = 'tgeompointinst' and rightarg = 'periodset';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_gbox where inst && b )
where op = '&&' and leftarg = 'tgeompointinst' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_gbox where inst @> b )
where op = '@>' and leftarg = 'tgeompointinst' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_gbox where inst <@ b )
where op = '<@' and leftarg = 'tgeompointinst' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_gbox where inst ~= b )
where op = '~=' and leftarg = 'tgeompointinst' and rightarg = 'gbox';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst && t2.inst )
where op = '&&' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst @> t2.inst )
where op = '@>' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst <@ t2.inst )
where op = '<@' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst t1, tbl_tgeompointinst t2 where t1.inst ~= t2.inst )
where op = '~=' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointinst';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst && ti )
where op = '&&' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst @> ti )
where op = '@>' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst <@ ti )
where op = '<@' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompointi where inst ~= ti )
where op = '~=' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointi';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst && seq )
where op = '&&' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst @> seq )
where op = '@>' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst <@ seq )
where op = '<@' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompointseq where inst ~= seq )
where op = '~=' and leftarg = 'tgeompointinst' and rightarg = 'tgeompointseq';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst && ts )
where op = '&&' and leftarg = 'tgeompointinst' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst @> ts )
where op = '@>' and leftarg = 'tgeompointinst' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst <@ ts )
where op = '<@' and leftarg = 'tgeompointinst' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointinst, tbl_tgeompoints where inst ~= ts )
where op = '~=' and leftarg = 'tgeompointinst' and rightarg = 'tgeompoints';

/*****************************************************************************/

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_geompoint where ti && g )
where op = '&&' and leftarg = 'tgeompointi' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_geompoint where ti @> g )
where op = '@>' and leftarg = 'tgeompointi' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_geompoint where ti <@ g )
where op = '<@' and leftarg = 'tgeompointi' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_geompoint where ti ~= g )
where op = '~=' and leftarg = 'tgeompointi' and rightarg = 'geompoint';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_geompolygon where ti && g )
where op = '&&' and leftarg = 'tgeompointi' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_geompolygon where ti @> g )
where op = '@>' and leftarg = 'tgeompointi' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_geompolygon where ti <@ g )
where op = '<@' and leftarg = 'tgeompointi' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_geompolygon where ti ~= g )
where op = '~=' and leftarg = 'tgeompointi' and rightarg = 'geompolygon';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_timestamptz where ti && t )
where op = '&&' and leftarg = 'tgeompointi' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_timestamptz where ti @> t )
where op = '@>' and leftarg = 'tgeompointi' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_timestamptz where ti <@ t )
where op = '<@' and leftarg = 'tgeompointi' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_timestamptz where ti ~= t )
where op = '~=' and leftarg = 'tgeompointi' and rightarg = 'timestamptz';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_timestampset where ti && ts )
where op = '&&' and leftarg = 'tgeompointi' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_timestampset where ti @> ts )
where op = '@>' and leftarg = 'tgeompointi' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_timestampset where ti <@ ts )
where op = '<@' and leftarg = 'tgeompointi' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_timestampset where ti ~= ts )
where op = '~=' and leftarg = 'tgeompointi' and rightarg = 'timestampset';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_period where ti && p )
where op = '&&' and leftarg = 'tgeompointi' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_period where ti @> p )
where op = '@>' and leftarg = 'tgeompointi' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_period where ti <@ p )
where op = '<@' and leftarg = 'tgeompointi' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_period where ti ~= p )
where op = '~=' and leftarg = 'tgeompointi' and rightarg = 'period';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_periodset where ti && ps )
where op = '&&' and leftarg = 'tgeompointi' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_periodset where ti @> ps )
where op = '@>' and leftarg = 'tgeompointi' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_periodset where ti <@ ps )
where op = '<@' and leftarg = 'tgeompointi' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_periodset where ti ~= ps )
where op = '~=' and leftarg = 'tgeompointi' and rightarg = 'periodset';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_gbox where ti && b )
where op = '&&' and leftarg = 'tgeompointi' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_gbox where ti @> b )
where op = '@>' and leftarg = 'tgeompointi' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_gbox where ti <@ b )
where op = '<@' and leftarg = 'tgeompointi' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_gbox where ti ~= b )
where op = '~=' and leftarg = 'tgeompointi' and rightarg = 'gbox';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti && inst )
where op = '&&' and leftarg = 'tgeompointi' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti @> inst )
where op = '@>' and leftarg = 'tgeompointi' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti <@ inst )
where op = '<@' and leftarg = 'tgeompointi' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompointinst where ti ~= inst )
where op = '~=' and leftarg = 'tgeompointi' and rightarg = 'tgeompointinst';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti && t2.ti )
where op = '&&' and leftarg = 'tgeompointi' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti @> t2.ti )
where op = '@>' and leftarg = 'tgeompointi' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti <@ t2.ti )
where op = '<@' and leftarg = 'tgeompointi' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi t1, tbl_tgeompointi t2 where t1.ti ~= t2.ti )
where op = '~=' and leftarg = 'tgeompointi' and rightarg = 'tgeompointi';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti && seq )
where op = '&&' and leftarg = 'tgeompointi' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti @> seq )
where op = '@>' and leftarg = 'tgeompointi' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti <@ seq )
where op = '<@' and leftarg = 'tgeompointi' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompointseq where ti ~= seq )
where op = '~=' and leftarg = 'tgeompointi' and rightarg = 'tgeompointseq';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti && ts )
where op = '&&' and leftarg = 'tgeompointi' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti @> ts )
where op = '@>' and leftarg = 'tgeompointi' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti <@ ts )
where op = '<@' and leftarg = 'tgeompointi' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointi, tbl_tgeompoints where ti ~= ts )
where op = '~=' and leftarg = 'tgeompointi' and rightarg = 'tgeompoints';

/*****************************************************************************/

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_geompoint where seq && g )
where op = '&&' and leftarg = 'tgeompointseq' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_geompoint where seq @> g )
where op = '@>' and leftarg = 'tgeompointseq' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_geompoint where seq <@ g )
where op = '<@' and leftarg = 'tgeompointseq' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_geompoint where seq ~= g )
where op = '~=' and leftarg = 'tgeompointseq' and rightarg = 'geompoint';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq && g )
where op = '&&' and leftarg = 'tgeompointseq' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq @> g )
where op = '@>' and leftarg = 'tgeompointseq' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq <@ g )
where op = '<@' and leftarg = 'tgeompointseq' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_geompolygon where seq ~= g )
where op = '~=' and leftarg = 'tgeompointseq' and rightarg = 'geompolygon';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_timestamptz where seq && t )
where op = '&&' and leftarg = 'tgeompointseq' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_timestamptz where seq @> t )
where op = '@>' and leftarg = 'tgeompointseq' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_timestamptz where seq <@ t )
where op = '<@' and leftarg = 'tgeompointseq' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_timestamptz where seq ~= t )
where op = '~=' and leftarg = 'tgeompointseq' and rightarg = 'timestamptz';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_timestampset where seq && ts )
where op = '&&' and leftarg = 'tgeompointseq' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_timestampset where seq @> ts )
where op = '@>' and leftarg = 'tgeompointseq' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_timestampset where seq <@ ts )
where op = '<@' and leftarg = 'tgeompointseq' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_timestampset where seq ~= ts )
where op = '~=' and leftarg = 'tgeompointseq' and rightarg = 'timestampset';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_period where seq && p )
where op = '&&' and leftarg = 'tgeompointseq' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_period where seq @> p )
where op = '@>' and leftarg = 'tgeompointseq' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_period where seq <@ p )
where op = '<@' and leftarg = 'tgeompointseq' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_period where seq ~= p )
where op = '~=' and leftarg = 'tgeompointseq' and rightarg = 'period';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_periodset where seq && ps )
where op = '&&' and leftarg = 'tgeompointseq' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_periodset where seq @> ps )
where op = '@>' and leftarg = 'tgeompointseq' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_periodset where seq <@ ps )
where op = '<@' and leftarg = 'tgeompointseq' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_periodset where seq ~= ps )
where op = '~=' and leftarg = 'tgeompointseq' and rightarg = 'periodset';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_gbox where seq && b )
where op = '&&' and leftarg = 'tgeompointseq' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_gbox where seq @> b )
where op = '@>' and leftarg = 'tgeompointseq' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_gbox where seq <@ b )
where op = '<@' and leftarg = 'tgeompointseq' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_gbox where seq ~= b )
where op = '~=' and leftarg = 'tgeompointseq' and rightarg = 'gbox';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq && inst )
where op = '&&' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq @> inst )
where op = '@>' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq <@ inst )
where op = '<@' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompointinst where seq ~= inst )
where op = '~=' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointinst';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq && ti )
where op = '&&' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq @> ti )
where op = '@>' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq <@ ti )
where op = '<@' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompointi where seq ~= ti )
where op = '~=' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointi';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq s1, tbl_tgeompointseq s2 where s1.seq && s2.seq )
where op = '&&' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq s1, tbl_tgeompointseq s2 where s1.seq @> s2.seq )
where op = '@>' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq s1, tbl_tgeompointseq s2 where s1.seq <@ s2.seq )
where op = '<@' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq s1, tbl_tgeompointseq s2 where s1.seq ~= s2.seq )
where op = '~=' and leftarg = 'tgeompointseq' and rightarg = 'tgeompointseq';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq && ts )
where op = '&&' and leftarg = 'tgeompointseq' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq @> ts )
where op = '@>' and leftarg = 'tgeompointseq' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq <@ ts )
where op = '<@' and leftarg = 'tgeompointseq' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompointseq, tbl_tgeompoints where seq ~= ts )
where op = '~=' and leftarg = 'tgeompointseq' and rightarg = 'tgeompoints';

/*****************************************************************************/

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_geompoint where ts && g )
where op = '&&' and leftarg = 'tgeompoints' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_geompoint where ts @> g )
where op = '@>' and leftarg = 'tgeompoints' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_geompoint where ts <@ g )
where op = '<@' and leftarg = 'tgeompoints' and rightarg = 'geompoint';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_geompoint where ts ~= g )
where op = '~=' and leftarg = 'tgeompoints' and rightarg = 'geompoint';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_geompolygon where ts && g )
where op = '&&' and leftarg = 'tgeompoints' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_geompolygon where ts @> g )
where op = '@>' and leftarg = 'tgeompoints' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_geompolygon where ts <@ g )
where op = '<@' and leftarg = 'tgeompoints' and rightarg = 'geompolygon';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_geompolygon where ts ~= g )
where op = '~=' and leftarg = 'tgeompoints' and rightarg = 'geompolygon';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_timestamptz where ts && t )
where op = '&&' and leftarg = 'tgeompoints' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_timestamptz where ts @> t )
where op = '@>' and leftarg = 'tgeompoints' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_timestamptz where ts <@ t )
where op = '<@' and leftarg = 'tgeompoints' and rightarg = 'timestamptz';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_timestamptz where ts ~= t )
where op = '~=' and leftarg = 'tgeompoints' and rightarg = 'timestamptz';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints t1, tbl_timestampset t2 where t1.ts && t2.ts )
where op = '&&' and leftarg = 'tgeompoints' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints t1, tbl_timestampset t2 where t1.ts @> t2.ts )
where op = '@>' and leftarg = 'tgeompoints' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints t1, tbl_timestampset t2 where t1.ts <@ t2.ts )
where op = '<@' and leftarg = 'tgeompoints' and rightarg = 'timestampset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints t1, tbl_timestampset t2 where t1.ts ~= t2.ts )
where op = '~=' and leftarg = 'tgeompoints' and rightarg = 'timestampset';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_period where ts && p )
where op = '&&' and leftarg = 'tgeompoints' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_period where ts @> p )
where op = '@>' and leftarg = 'tgeompoints' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_period where ts <@ p )
where op = '<@' and leftarg = 'tgeompoints' and rightarg = 'period';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_period where ts ~= p )
where op = '~=' and leftarg = 'tgeompoints' and rightarg = 'period';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_periodset where ts && ps )
where op = '&&' and leftarg = 'tgeompoints' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_periodset where ts @> ps )
where op = '@>' and leftarg = 'tgeompoints' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_periodset where ts <@ ps )
where op = '<@' and leftarg = 'tgeompoints' and rightarg = 'periodset';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_periodset where ts ~= ps )
where op = '~=' and leftarg = 'tgeompoints' and rightarg = 'periodset';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_gbox where ts && b )
where op = '&&' and leftarg = 'tgeompoints' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_gbox where ts @> b )
where op = '@>' and leftarg = 'tgeompoints' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_gbox where ts <@ b )
where op = '<@' and leftarg = 'tgeompoints' and rightarg = 'gbox';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_gbox where ts ~= b )
where op = '~=' and leftarg = 'tgeompoints' and rightarg = 'gbox';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts && inst )
where op = '&&' and leftarg = 'tgeompoints' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts @> inst )
where op = '@>' and leftarg = 'tgeompoints' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts <@ inst )
where op = '<@' and leftarg = 'tgeompoints' and rightarg = 'tgeompointinst';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointinst where ts ~= inst )
where op = '~=' and leftarg = 'tgeompoints' and rightarg = 'tgeompointinst';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts && ti )
where op = '&&' and leftarg = 'tgeompoints' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts @> ti )
where op = '@>' and leftarg = 'tgeompoints' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts <@ ti )
where op = '<@' and leftarg = 'tgeompoints' and rightarg = 'tgeompointi';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointi where ts ~= ti )
where op = '~=' and leftarg = 'tgeompoints' and rightarg = 'tgeompointi';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts && seq )
where op = '&&' and leftarg = 'tgeompoints' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts @> seq )
where op = '@>' and leftarg = 'tgeompoints' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts <@ seq )
where op = '<@' and leftarg = 'tgeompoints' and rightarg = 'tgeompointseq';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints, tbl_tgeompointseq where ts ~= seq )
where op = '~=' and leftarg = 'tgeompoints' and rightarg = 'tgeompointseq';

update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts && t2.ts )
where op = '&&' and leftarg = 'tgeompoints' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts @> t2.ts )
where op = '@>' and leftarg = 'tgeompoints' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts <@ t2.ts )
where op = '<@' and leftarg = 'tgeompoints' and rightarg = 'tgeompoints';
update test_geoboundboxops
set gistidx = ( select count(*) from tbl_tgeompoints t1, tbl_tgeompoints t2 where t1.ts ~= t2.ts )
where op = '~=' and leftarg = 'tgeompoints' and rightarg = 'tgeompoints';

/*****************************************************************************/
