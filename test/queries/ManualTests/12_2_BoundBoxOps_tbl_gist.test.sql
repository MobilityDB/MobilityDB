/*****************************************************************************/

CREATE INDEX tbl_tboolinst_gist_idx ON tbl_tboolinst USING GIST(inst);
CREATE INDEX tbl_tintinst_gist_idx ON tbl_tintinst USING GIST(inst);
CREATE INDEX tbl_tfloatinst_gist_idx ON tbl_tfloatinst USING GIST(inst);
CREATE INDEX tbl_ttextinst_gist_idx ON tbl_ttextinst USING GIST(inst);

CREATE INDEX tbl_tbooli_gist_idx ON tbl_tbooli USING GIST(ti);
CREATE INDEX tbl_tinti_gist_idx ON tbl_tinti USING GIST(ti);
CREATE INDEX tbl_tfloati_gist_idx ON tbl_tfloati USING GIST(ti);
CREATE INDEX tbl_ttexti_gist_idx ON tbl_ttexti USING GIST(ti);

CREATE INDEX tbl_tboolseq_gist_idx ON tbl_tboolseq USING GIST(seq);
CREATE INDEX tbl_tintseq_gist_idx ON tbl_tintseq USING GIST(seq);
CREATE INDEX tbl_tfloatseq_gist_idx ON tbl_tfloatseq USING GIST(seq);
CREATE INDEX tbl_ttextseq_gist_idx ON tbl_ttextseq USING GIST(seq);

CREATE INDEX tbl_tbools_gist_idx ON tbl_tbools USING GIST(ts);
CREATE INDEX tbl_tints_gist_idx ON tbl_tints USING GIST(ts);
CREATE INDEX tbl_tfloats_gist_idx ON tbl_tfloats USING GIST(ts);
CREATE INDEX tbl_ttexts_gist_idx ON tbl_ttexts USING GIST(ts);

/*****************************************************************************/

-------------------------------------------------------------------------------
-- Overlaps
-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tboolinst where t && inst ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tintinst where t && inst ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloatinst where t && inst ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttextinst where t && inst ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tbooli where t && ti ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tinti where t && ti ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloati where t && ti ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttexti where t && ti ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tboolseq where t && seq ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tintseq where t && seq ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloatseq where t && seq ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttextseq where t && seq ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tbools where t && ts ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tints where t && ts ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloats where t && ts ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttexts where t && ts ) 
where op = '&&' and leftarg = 'timestamptz' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tboolinst where ts && inst ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tintinst where ts && inst ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloatinst where ts && inst ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttextinst where ts && inst ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tbooli where ts && ti ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tinti where ts && ti ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloati where ts && ti ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttexti where ts && ti ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tboolseq where ts && seq ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tintseq where ts && seq ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloatseq where ts && seq ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttextseq where ts && seq ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tbools t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tints t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tfloats t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_ttexts t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'timestampset' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tboolinst where p && inst ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tintinst where p && inst ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloatinst where p && inst ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttextinst where p && inst ) 
where op = '&&' and leftarg = 'period' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tbooli where p && ti ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tinti where p && ti ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloati where p && ti ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttexti where p && ti ) 
where op = '&&' and leftarg = 'period' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tboolseq where p && seq ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tintseq where p && seq ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloatseq where p && seq ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttextseq where p && seq ) 
where op = '&&' and leftarg = 'period' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tbools where p && ts ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tints where p && ts ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloats where p && ts ) 
where op = '&&' and leftarg = 'period' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttexts where p && ts ) 
where op = '&&' and leftarg = 'period' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tboolinst where ps && inst ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tintinst where ps && inst ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloatinst where ps && inst ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttextinst where ps && inst ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tbooli where ps && ti ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tinti where ps && ti ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloati where ps && ti ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttexti where ps && ti ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tboolseq where ps && seq ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tintseq where ps && seq ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloatseq where ps && seq ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttextseq where ps && seq ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tbools where ps && ts ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tints where ps && ts ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloats where ps && ts ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttexts where ps && ts ) 
where op = '&&' and leftarg = 'periodset' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_timestamptz where inst && t ) 
where op = '&&' and leftarg = 'tboolinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_timestampset where inst && ts ) 
where op = '&&' and leftarg = 'tboolinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_period where inst && p ) 
where op = '&&' and leftarg = 'tboolinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_periodset where inst && ps ) 
where op = '&&' and leftarg = 'tboolinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst t1, tbl_tboolinst t2 where t1.inst && t2.inst ) 
where op = '&&' and leftarg = 'tboolinst' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tbooli where inst && ti ) 
where op = '&&' and leftarg = 'tboolinst' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tboolseq where inst && seq ) 
where op = '&&' and leftarg = 'tboolinst' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tbools where inst && ts ) 
where op = '&&' and leftarg = 'tboolinst' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_int where inst && i ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_intrange where inst && i ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_float where inst && f ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_floatrange where inst && f ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_timestamptz where inst && t ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_timestampset where inst && ts ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_period where inst && p ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_periodset where inst && ps ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_box where inst && b ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst && t2.inst ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst && t2.inst ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tinti where inst && ti ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloati where inst && ti ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tintseq where inst && seq ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloatseq where inst && seq ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tints where inst && ts ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloats where inst && ts ) 
where op = '&&' and leftarg = 'tintinst' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_int where inst && i ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_intrange where inst && i ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_float where inst && f ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_floatrange where inst && f ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_timestamptz where inst && t ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_timestampset where inst && ts ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_period where inst && p ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_periodset where inst && ps ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_box where inst && b ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst && t2.inst ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst && t2.inst ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tinti where inst && ti ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'tbl_tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloati where inst && ti ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tintseq where inst && seq ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloatseq where inst && seq ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tints where inst && ts ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloats where inst && ts ) 
where op = '&&' and leftarg = 'tfloatinst' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_timestamptz where inst && t ) 
where op = '&&' and leftarg = 'ttextinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_timestampset where inst && ts ) 
where op = '&&' and leftarg = 'ttextinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_period where inst && p ) 
where op = '&&' and leftarg = 'ttextinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_periodset where inst && ps ) 
where op = '&&' and leftarg = 'ttextinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst t1, tbl_ttextinst t2 where t1.inst && t2.inst ) 
where op = '&&' and leftarg = 'ttextinst' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttexti where inst && ti ) 
where op = '&&' and leftarg = 'ttextinst' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttextseq where inst && seq ) 
where op = '&&' and leftarg = 'ttextinst' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttexts where inst && ts ) 
where op = '&&' and leftarg = 'ttextinst' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_timestamptz where ti && t ) 
where op = '&&' and leftarg = 'tbooli' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_timestampset where ti && ts ) 
where op = '&&' and leftarg = 'tbooli' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_period where ti && p ) 
where op = '&&' and leftarg = 'tbooli' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_periodset where ti && ps ) 
where op = '&&' and leftarg = 'tbooli' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tboolinst where ti && inst ) 
where op = '&&' and leftarg = 'tbooli' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli t1, tbl_tbooli t2 where t1.ti && t2.ti ) 
where op = '&&' and leftarg = 'tbooli' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tboolseq where ti && seq ) 
where op = '&&' and leftarg = 'tbooli' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tbools where ti && ts ) 
where op = '&&' and leftarg = 'tbooli' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_int where ti && i ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_intrange where ti && i ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_float where ti && f ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_floatrange where ti && f ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_timestamptz where ti && t ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_timestampset where ti && ts ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_period where ti && p ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_periodset where ti && ps ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_box where ti && b ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tintinst where ti && inst ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloatinst where ti && inst ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti && t2.ti ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti && t2.ti ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tintseq where ti && seq ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloatseq where ti && seq ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tints where ti && ts ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloats where ti && ts ) 
where op = '&&' and leftarg = 'tinti' and rightarg = 'tfloats';
	
-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_int where ti && i ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_intrange where ti && i ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_float where ti && f ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_floatrange where ti && f ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_timestamptz where ti && t ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_timestampset where ti && ts ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_period where ti && p ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_periodset where ti && ps ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_box where ti && b ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tintinst where ti && inst ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloatinst where ti && inst ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti && t2.ti ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti && t2.ti ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tintseq where ti && seq ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloatseq where ti && seq ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tints where ti && ts ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloats where ti && ts ) 
where op = '&&' and leftarg = 'tfloati' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_timestamptz where ti && t ) 
where op = '&&' and leftarg = 'ttexti' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_timestampset where ti && ts ) 
where op = '&&' and leftarg = 'ttexti' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_period where ti && p ) 
where op = '&&' and leftarg = 'ttexti' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_periodset where ti && ps ) 
where op = '&&' and leftarg = 'ttexti' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttextinst where ti && inst ) 
where op = '&&' and leftarg = 'ttexti' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti t1, tbl_ttexti t2 where t1.ti && t2.ti ) 
where op = '&&' and leftarg = 'ttexti' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttextseq where ti && seq ) 
where op = '&&' and leftarg = 'ttexti' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttexts where ti && ts ) 
where op = '&&' and leftarg = 'ttexti' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_timestamptz where seq && t ) 
where op = '&&' and leftarg = 'tboolseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_timestampset where seq && ts ) 
where op = '&&' and leftarg = 'tboolseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_period where seq && p ) 
where op = '&&' and leftarg = 'tboolseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_periodset where seq && ps ) 
where op = '&&' and leftarg = 'tboolseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tboolinst where seq && inst ) 
where op = '&&' and leftarg = 'tboolseq' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tbooli where seq && ti ) 
where op = '&&' and leftarg = 'tboolseq' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq t1, tbl_tboolseq t2 where t1.seq && t2.seq ) 
where op = '&&' and leftarg = 'tboolseq' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tbools where seq && ts ) 
where op = '&&' and leftarg = 'tboolseq' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_int where seq && i ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_intrange where seq && i ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_float where seq && f ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_floatrange where seq && f ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_timestamptz where seq && t ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_timestampset where seq && ts ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_period where seq && p ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_periodset where seq && ps ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_box where seq && b ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tintinst where seq && inst ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloatinst where seq && inst ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tinti where seq && ti ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloati where seq && ti ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq && t2.seq ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq && t2.seq ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tints where seq && ts ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloats where seq && ts ) 
where op = '&&' and leftarg = 'tintseq' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_int where seq && i ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_intrange where seq && i ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_float where seq && f ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_floatrange where seq && f ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_timestamptz where seq && t ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_timestampset where seq && ts ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_period where seq && p ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_periodset where seq && ps ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_box where seq && b ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tintinst where seq && inst ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloatinst where seq && inst ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tinti where seq && ti ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloati where seq && ti ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq && t2.seq ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq && t2.seq ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tints where seq && ts ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloats where seq && ts ) 
where op = '&&' and leftarg = 'tfloatseq' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_timestamptz where seq && t ) 
where op = '&&' and leftarg = 'ttextseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_timestampset where seq && ts ) 
where op = '&&' and leftarg = 'ttextseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_period where seq && p ) 
where op = '&&' and leftarg = 'ttextseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_periodset where seq && ps ) 
where op = '&&' and leftarg = 'ttextseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttextinst where seq && inst ) 
where op = '&&' and leftarg = 'ttextseq' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttexti where seq && ti ) 
where op = '&&' and leftarg = 'ttextseq' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq t1, tbl_ttextseq t2 where t1.seq && t2.seq ) 
where op = '&&' and leftarg = 'ttextseq' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttexts where seq && ts ) 
where op = '&&' and leftarg = 'ttextseq' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_timestamptz where ts && t ) 
where op = '&&' and leftarg = 'tbools' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools t1, tbl_timestampset t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'tbools' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_period where ts && p ) 
where op = '&&' and leftarg = 'tbools' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_periodset where ts && ps ) 
where op = '&&' and leftarg = 'tbools' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tboolinst where ts && inst ) 
where op = '&&' and leftarg = 'tbools' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tbooli where ts && ti ) 
where op = '&&' and leftarg = 'tbools' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tboolseq where ts && seq ) 
where op = '&&' and leftarg = 'tbools' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools t1, tbl_tbools t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'tbools' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_int where ts && i ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_intrange where ts && i ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_float where ts && f ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_floatrange where ts && f ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_timestamptz where ts && t ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_timestampset t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_period where ts && p ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_periodset where ts && ps ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_box where ts && b ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tintinst where ts && inst ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloatinst where ts && inst ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tinti where ts && ti ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloati where ts && ti ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tintseq where ts && seq ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloatseq where ts && seq ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_tints t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'tints' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_int where ts && i ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_intrange where ts && i ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_float where ts && f ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_floatrange where ts && f ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_timestamptz where ts && t ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_timestampset t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_period where ts && p ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_periodset where ts && ps ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_box where ts && b ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tintinst where ts && inst ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloatinst where ts && inst ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tinti where ts && ti ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloati where ts && ti ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tintseq where ts && seq ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloatseq where ts && seq ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'tfloats' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_timestamptz where ts && t ) 
where op = '&&' and leftarg = 'ttexts' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts t1, tbl_timestampset t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'ttexts' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_period where ts && p ) 
where op = '&&' and leftarg = 'ttexts' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_periodset where ts && ps ) 
where op = '&&' and leftarg = 'ttexts' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttextinst where ts && inst ) 
where op = '&&' and leftarg = 'ttexts' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttexti where ts && ti ) 
where op = '&&' and leftarg = 'ttexts' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttextseq where ts && seq ) 
where op = '&&' and leftarg = 'ttexts' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts t1, tbl_ttexts t2 where t1.ts && t2.ts ) 
where op = '&&' and leftarg = 'ttexts' and rightarg = 'ttexts';

-------------------------------------------------------------------------------
-- Contains
-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tboolinst where t @> inst ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tintinst where t @> inst ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloatinst where t @> inst ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttextinst where t @> inst ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tbooli where t @> ti ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tinti where t @> ti ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloati where t @> ti ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttexti where t @> ti ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tboolseq where t @> seq ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tintseq where t @> seq ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloatseq where t @> seq ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttextseq where t @> seq ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tbools where t @> ts ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tints where t @> ts ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloats where t @> ts ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttexts where t @> ts ) 
where op = '@>' and leftarg = 'timestamptz' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tboolinst where ts @> inst ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tintinst where ts @> inst ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloatinst where ts @> inst ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttextinst where ts @> inst ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tbooli where ts @> ti ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tinti where ts @> ti ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloati where ts @> ti ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttexti where ts @> ti ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tboolseq where ts @> seq ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tintseq where ts @> seq ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloatseq where ts @> seq ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttextseq where ts @> seq ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tbools t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tints t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tfloats t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_ttexts t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'timestampset' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tboolinst where p @> inst ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tintinst where p @> inst ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloatinst where p @> inst ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttextinst where p @> inst ) 
where op = '@>' and leftarg = 'period' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tbooli where p @> ti ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tinti where p @> ti ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloati where p @> ti ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttexti where p @> ti ) 
where op = '@>' and leftarg = 'period' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tboolseq where p @> seq ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tintseq where p @> seq ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloatseq where p @> seq ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttextseq where p @> seq ) 
where op = '@>' and leftarg = 'period' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tbools where p @> ts ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tints where p @> ts ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloats where p @> ts ) 
where op = '@>' and leftarg = 'period' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttexts where p @> ts ) 
where op = '@>' and leftarg = 'period' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tboolinst where ps @> inst ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tintinst where ps @> inst ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloatinst where ps @> inst ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttextinst where ps @> inst ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tbooli where ps @> ti ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tinti where ps @> ti ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloati where ps @> ti ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttexti where ps @> ti ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tboolseq where ps @> seq ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tintseq where ps @> seq ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloatseq where ps @> seq ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttextseq where ps @> seq ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tbools where ps @> ts ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tints where ps @> ts ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloats where ps @> ts ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttexts where ps @> ts ) 
where op = '@>' and leftarg = 'periodset' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_timestamptz where inst @> t ) 
where op = '@>' and leftarg = 'tboolinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_timestampset where inst @> ts ) 
where op = '@>' and leftarg = 'tboolinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_period where inst @> p ) 
where op = '@>' and leftarg = 'tboolinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_periodset where inst @> ps ) 
where op = '@>' and leftarg = 'tboolinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst t1, tbl_tboolinst t2 where t1.inst @> t2.inst ) 
where op = '@>' and leftarg = 'tboolinst' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tbooli where inst @> ti ) 
where op = '@>' and leftarg = 'tboolinst' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tboolseq where inst @> seq ) 
where op = '@>' and leftarg = 'tboolinst' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tbools where inst @> ts ) 
where op = '@>' and leftarg = 'tboolinst' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_int where inst @> i ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_intrange where inst @> i ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_float where inst @> f ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_floatrange where inst @> f ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_timestamptz where inst @> t ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_timestampset where inst @> ts ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_period where inst @> p ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_periodset where inst @> ps ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_box where inst @> b ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst @> t2.inst ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst @> t2.inst ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tinti where inst @> ti ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloati where inst @> ti ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tintseq where inst @> seq ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloatseq where inst @> seq ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tints where inst @> ts ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloats where inst @> ts ) 
where op = '@>' and leftarg = 'tintinst' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_int where inst @> i ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_intrange where inst @> i ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_float where inst @> f ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_floatrange where inst @> f ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_timestamptz where inst @> t ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_timestampset where inst @> ts ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_period where inst @> p ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_periodset where inst @> ps ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_box where inst @> b ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst @> t2.inst ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst @> t2.inst ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tinti where inst @> ti ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'tbl_tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloati where inst @> ti ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tintseq where inst @> seq ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloatseq where inst @> seq ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tints where inst @> ts ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloats where inst @> ts ) 
where op = '@>' and leftarg = 'tfloatinst' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_timestamptz where inst @> t ) 
where op = '@>' and leftarg = 'ttextinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_timestampset where inst @> ts ) 
where op = '@>' and leftarg = 'ttextinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_period where inst @> p ) 
where op = '@>' and leftarg = 'ttextinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_periodset where inst @> ps ) 
where op = '@>' and leftarg = 'ttextinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst t1, tbl_ttextinst t2 where t1.inst @> t2.inst ) 
where op = '@>' and leftarg = 'ttextinst' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttexti where inst @> ti ) 
where op = '@>' and leftarg = 'ttextinst' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttextseq where inst @> seq ) 
where op = '@>' and leftarg = 'ttextinst' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttexts where inst @> ts ) 
where op = '@>' and leftarg = 'ttextinst' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_timestamptz where ti @> t ) 
where op = '@>' and leftarg = 'tbooli' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_timestampset where ti @> ts ) 
where op = '@>' and leftarg = 'tbooli' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_period where ti @> p ) 
where op = '@>' and leftarg = 'tbooli' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_periodset where ti @> ps ) 
where op = '@>' and leftarg = 'tbooli' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tboolinst where ti @> inst ) 
where op = '@>' and leftarg = 'tbooli' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli t1, tbl_tbooli t2 where t1.ti @> t2.ti ) 
where op = '@>' and leftarg = 'tbooli' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tboolseq where ti @> seq ) 
where op = '@>' and leftarg = 'tbooli' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tbools where ti @> ts ) 
where op = '@>' and leftarg = 'tbooli' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_int where ti @> i ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_intrange where ti @> i ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_float where ti @> f ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_floatrange where ti @> f ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_timestamptz where ti @> t ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_timestampset where ti @> ts ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_period where ti @> p ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_periodset where ti @> ps ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_box where ti @> b ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tintinst where ti @> inst ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloatinst where ti @> inst ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti @> t2.ti ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti @> t2.ti ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tintseq where ti @> seq ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloatseq where ti @> seq ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tints where ti @> ts ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloats where ti @> ts ) 
where op = '@>' and leftarg = 'tinti' and rightarg = 'tfloats';
	
-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_int where ti @> i ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_intrange where ti @> i ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_float where ti @> f ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_floatrange where ti @> f ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_timestamptz where ti @> t ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_timestampset where ti @> ts ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_period where ti @> p ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_periodset where ti @> ps ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_box where ti @> b ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tintinst where ti @> inst ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloatinst where ti @> inst ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti @> t2.ti ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti @> t2.ti ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tintseq where ti @> seq ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloatseq where ti @> seq ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tints where ti @> ts ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloats where ti @> ts ) 
where op = '@>' and leftarg = 'tfloati' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_timestamptz where ti @> t ) 
where op = '@>' and leftarg = 'ttexti' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_timestampset where ti @> ts ) 
where op = '@>' and leftarg = 'ttexti' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_period where ti @> p ) 
where op = '@>' and leftarg = 'ttexti' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_periodset where ti @> ps ) 
where op = '@>' and leftarg = 'ttexti' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttextinst where ti @> inst ) 
where op = '@>' and leftarg = 'ttexti' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti t1, tbl_ttexti t2 where t1.ti @> t2.ti ) 
where op = '@>' and leftarg = 'ttexti' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttextseq where ti @> seq ) 
where op = '@>' and leftarg = 'ttexti' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttexts where ti @> ts ) 
where op = '@>' and leftarg = 'ttexti' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_timestamptz where seq @> t ) 
where op = '@>' and leftarg = 'tboolseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_timestampset where seq @> ts ) 
where op = '@>' and leftarg = 'tboolseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_period where seq @> p ) 
where op = '@>' and leftarg = 'tboolseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_periodset where seq @> ps ) 
where op = '@>' and leftarg = 'tboolseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tboolinst where seq @> inst ) 
where op = '@>' and leftarg = 'tboolseq' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tbooli where seq @> ti ) 
where op = '@>' and leftarg = 'tboolseq' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq t1, tbl_tboolseq t2 where t1.seq @> t2.seq ) 
where op = '@>' and leftarg = 'tboolseq' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tbools where seq @> ts ) 
where op = '@>' and leftarg = 'tboolseq' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_int where seq @> i ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_intrange where seq @> i ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_float where seq @> f ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_floatrange where seq @> f ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_timestamptz where seq @> t ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_timestampset where seq @> ts ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_period where seq @> p ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_periodset where seq @> ps ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_box where seq @> b ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tintinst where seq @> inst ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloatinst where seq @> inst ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tinti where seq @> ti ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloati where seq @> ti ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq @> t2.seq ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq @> t2.seq ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tints where seq @> ts ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloats where seq @> ts ) 
where op = '@>' and leftarg = 'tintseq' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_int where seq @> i ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_intrange where seq @> i ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_float where seq @> f ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_floatrange where seq @> f ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_timestamptz where seq @> t ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_timestampset where seq @> ts ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_period where seq @> p ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_periodset where seq @> ps ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_box where seq @> b ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tintinst where seq @> inst ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloatinst where seq @> inst ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tinti where seq @> ti ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloati where seq @> ti ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq @> t2.seq ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq @> t2.seq ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tints where seq @> ts ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloats where seq @> ts ) 
where op = '@>' and leftarg = 'tfloatseq' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_timestamptz where seq @> t ) 
where op = '@>' and leftarg = 'ttextseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_timestampset where seq @> ts ) 
where op = '@>' and leftarg = 'ttextseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_period where seq @> p ) 
where op = '@>' and leftarg = 'ttextseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_periodset where seq @> ps ) 
where op = '@>' and leftarg = 'ttextseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttextinst where seq @> inst ) 
where op = '@>' and leftarg = 'ttextseq' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttexti where seq @> ti ) 
where op = '@>' and leftarg = 'ttextseq' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq t1, tbl_ttextseq t2 where t1.seq @> t2.seq ) 
where op = '@>' and leftarg = 'ttextseq' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttexts where seq @> ts ) 
where op = '@>' and leftarg = 'ttextseq' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_timestamptz where ts @> t ) 
where op = '@>' and leftarg = 'tbools' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools t1, tbl_timestampset t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'tbools' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_period where ts @> p ) 
where op = '@>' and leftarg = 'tbools' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_periodset where ts @> ps ) 
where op = '@>' and leftarg = 'tbools' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tboolinst where ts @> inst ) 
where op = '@>' and leftarg = 'tbools' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tbooli where ts @> ti ) 
where op = '@>' and leftarg = 'tbools' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tboolseq where ts @> seq ) 
where op = '@>' and leftarg = 'tbools' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools t1, tbl_tbools t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'tbools' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_int where ts @> i ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_intrange where ts @> i ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_float where ts @> f ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_floatrange where ts @> f ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_timestamptz where ts @> t ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_timestampset t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_period where ts @> p ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_periodset where ts @> ps ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_box where ts @> b ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tintinst where ts @> inst ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloatinst where ts @> inst ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tinti where ts @> ti ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloati where ts @> ti ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tintseq where ts @> seq ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloatseq where ts @> seq ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_tints t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'tints' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_int where ts @> i ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_intrange where ts @> i ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_float where ts @> f ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_floatrange where ts @> f ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_timestamptz where ts @> t ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_timestampset t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_period where ts @> p ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_periodset where ts @> ps ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_box where ts @> b ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tintinst where ts @> inst ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloatinst where ts @> inst ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tinti where ts @> ti ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloati where ts @> ti ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tintseq where ts @> seq ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloatseq where ts @> seq ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'tfloats' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_timestamptz where ts @> t ) 
where op = '@>' and leftarg = 'ttexts' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts t1, tbl_timestampset t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'ttexts' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_period where ts @> p ) 
where op = '@>' and leftarg = 'ttexts' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_periodset where ts @> ps ) 
where op = '@>' and leftarg = 'ttexts' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttextinst where ts @> inst ) 
where op = '@>' and leftarg = 'ttexts' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttexti where ts @> ti ) 
where op = '@>' and leftarg = 'ttexts' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttextseq where ts @> seq ) 
where op = '@>' and leftarg = 'ttexts' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts t1, tbl_ttexts t2 where t1.ts @> t2.ts ) 
where op = '@>' and leftarg = 'ttexts' and rightarg = 'ttexts';

-------------------------------------------------------------------------------
-- Contained
-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tboolinst where t <@ inst ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tintinst where t <@ inst ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloatinst where t <@ inst ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttextinst where t <@ inst ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tbooli where t <@ ti ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tinti where t <@ ti ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloati where t <@ ti ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttexti where t <@ ti ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tboolseq where t <@ seq ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tintseq where t <@ seq ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloatseq where t <@ seq ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttextseq where t <@ seq ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tbools where t <@ ts ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tints where t <@ ts ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloats where t <@ ts ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttexts where t <@ ts ) 
where op = '<@' and leftarg = 'timestamptz' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tboolinst where ts <@ inst ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tintinst where ts <@ inst ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloatinst where ts <@ inst ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttextinst where ts <@ inst ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tbooli where ts <@ ti ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tinti where ts <@ ti ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloati where ts <@ ti ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttexti where ts <@ ti ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tboolseq where ts <@ seq ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tintseq where ts <@ seq ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloatseq where ts <@ seq ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttextseq where ts <@ seq ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tbools t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tints t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tfloats t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_ttexts t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'timestampset' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tboolinst where p <@ inst ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tintinst where p <@ inst ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloatinst where p <@ inst ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttextinst where p <@ inst ) 
where op = '<@' and leftarg = 'period' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tbooli where p <@ ti ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tinti where p <@ ti ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloati where p <@ ti ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttexti where p <@ ti ) 
where op = '<@' and leftarg = 'period' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tboolseq where p <@ seq ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tintseq where p <@ seq ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloatseq where p <@ seq ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttextseq where p <@ seq ) 
where op = '<@' and leftarg = 'period' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tbools where p <@ ts ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tints where p <@ ts ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloats where p <@ ts ) 
where op = '<@' and leftarg = 'period' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttexts where p <@ ts ) 
where op = '<@' and leftarg = 'period' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tboolinst where ps <@ inst ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tintinst where ps <@ inst ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloatinst where ps <@ inst ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttextinst where ps <@ inst ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tbooli where ps <@ ti ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tinti where ps <@ ti ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloati where ps <@ ti ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttexti where ps <@ ti ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tboolseq where ps <@ seq ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tintseq where ps <@ seq ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloatseq where ps <@ seq ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttextseq where ps <@ seq ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tbools where ps <@ ts ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tints where ps <@ ts ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloats where ps <@ ts ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttexts where ps <@ ts ) 
where op = '<@' and leftarg = 'periodset' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_timestamptz where inst <@ t ) 
where op = '<@' and leftarg = 'tboolinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_timestampset where inst <@ ts ) 
where op = '<@' and leftarg = 'tboolinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_period where inst <@ p ) 
where op = '<@' and leftarg = 'tboolinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_periodset where inst <@ ps ) 
where op = '<@' and leftarg = 'tboolinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst t1, tbl_tboolinst t2 where t1.inst <@ t2.inst ) 
where op = '<@' and leftarg = 'tboolinst' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tbooli where inst <@ ti ) 
where op = '<@' and leftarg = 'tboolinst' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tboolseq where inst <@ seq ) 
where op = '<@' and leftarg = 'tboolinst' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tbools where inst <@ ts ) 
where op = '<@' and leftarg = 'tboolinst' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_int where inst <@ i ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_intrange where inst <@ i ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_float where inst <@ f ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_floatrange where inst <@ f ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_timestamptz where inst <@ t ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_timestampset where inst <@ ts ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_period where inst <@ p ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_periodset where inst <@ ps ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_box where inst <@ b ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst <@ t2.inst ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst <@ t2.inst ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tinti where inst <@ ti ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloati where inst <@ ti ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tintseq where inst <@ seq ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloatseq where inst <@ seq ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tints where inst <@ ts ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloats where inst <@ ts ) 
where op = '<@' and leftarg = 'tintinst' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_int where inst <@ i ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_intrange where inst <@ i ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_float where inst <@ f ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_floatrange where inst <@ f ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_timestamptz where inst <@ t ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_timestampset where inst <@ ts ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_period where inst <@ p ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_periodset where inst <@ ps ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_box where inst <@ b ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst <@ t2.inst ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst <@ t2.inst ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tinti where inst <@ ti ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'tbl_tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloati where inst <@ ti ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tintseq where inst <@ seq ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloatseq where inst <@ seq ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tints where inst <@ ts ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloats where inst <@ ts ) 
where op = '<@' and leftarg = 'tfloatinst' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_timestamptz where inst <@ t ) 
where op = '<@' and leftarg = 'ttextinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_timestampset where inst <@ ts ) 
where op = '<@' and leftarg = 'ttextinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_period where inst <@ p ) 
where op = '<@' and leftarg = 'ttextinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_periodset where inst <@ ps ) 
where op = '<@' and leftarg = 'ttextinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst t1, tbl_ttextinst t2 where t1.inst <@ t2.inst ) 
where op = '<@' and leftarg = 'ttextinst' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttexti where inst <@ ti ) 
where op = '<@' and leftarg = 'ttextinst' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttextseq where inst <@ seq ) 
where op = '<@' and leftarg = 'ttextinst' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttexts where inst <@ ts ) 
where op = '<@' and leftarg = 'ttextinst' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_timestamptz where ti <@ t ) 
where op = '<@' and leftarg = 'tbooli' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_timestampset where ti <@ ts ) 
where op = '<@' and leftarg = 'tbooli' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_period where ti <@ p ) 
where op = '<@' and leftarg = 'tbooli' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_periodset where ti <@ ps ) 
where op = '<@' and leftarg = 'tbooli' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tboolinst where ti <@ inst ) 
where op = '<@' and leftarg = 'tbooli' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli t1, tbl_tbooli t2 where t1.ti <@ t2.ti ) 
where op = '<@' and leftarg = 'tbooli' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tboolseq where ti <@ seq ) 
where op = '<@' and leftarg = 'tbooli' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tbools where ti <@ ts ) 
where op = '<@' and leftarg = 'tbooli' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_int where ti <@ i ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_intrange where ti <@ i ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_float where ti <@ f ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_floatrange where ti <@ f ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_timestamptz where ti <@ t ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_timestampset where ti <@ ts ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_period where ti <@ p ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_periodset where ti <@ ps ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_box where ti <@ b ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tintinst where ti <@ inst ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloatinst where ti <@ inst ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti <@ t2.ti ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti <@ t2.ti ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tintseq where ti <@ seq ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloatseq where ti <@ seq ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tints where ti <@ ts ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloats where ti <@ ts ) 
where op = '<@' and leftarg = 'tinti' and rightarg = 'tfloats';
	
-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_int where ti <@ i ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_intrange where ti <@ i ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_float where ti <@ f ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_floatrange where ti <@ f ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_timestamptz where ti <@ t ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_timestampset where ti <@ ts ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_period where ti <@ p ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_periodset where ti <@ ps ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_box where ti <@ b ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tintinst where ti <@ inst ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloatinst where ti <@ inst ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti <@ t2.ti ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti <@ t2.ti ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tintseq where ti <@ seq ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloatseq where ti <@ seq ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tints where ti <@ ts ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloats where ti <@ ts ) 
where op = '<@' and leftarg = 'tfloati' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_timestamptz where ti <@ t ) 
where op = '<@' and leftarg = 'ttexti' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_timestampset where ti <@ ts ) 
where op = '<@' and leftarg = 'ttexti' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_period where ti <@ p ) 
where op = '<@' and leftarg = 'ttexti' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_periodset where ti <@ ps ) 
where op = '<@' and leftarg = 'ttexti' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttextinst where ti <@ inst ) 
where op = '<@' and leftarg = 'ttexti' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti t1, tbl_ttexti t2 where t1.ti <@ t2.ti ) 
where op = '<@' and leftarg = 'ttexti' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttextseq where ti <@ seq ) 
where op = '<@' and leftarg = 'ttexti' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttexts where ti <@ ts ) 
where op = '<@' and leftarg = 'ttexti' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_timestamptz where seq <@ t ) 
where op = '<@' and leftarg = 'tboolseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_timestampset where seq <@ ts ) 
where op = '<@' and leftarg = 'tboolseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_period where seq <@ p ) 
where op = '<@' and leftarg = 'tboolseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_periodset where seq <@ ps ) 
where op = '<@' and leftarg = 'tboolseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tboolinst where seq <@ inst ) 
where op = '<@' and leftarg = 'tboolseq' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tbooli where seq <@ ti ) 
where op = '<@' and leftarg = 'tboolseq' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq t1, tbl_tboolseq t2 where t1.seq <@ t2.seq ) 
where op = '<@' and leftarg = 'tboolseq' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tbools where seq <@ ts ) 
where op = '<@' and leftarg = 'tboolseq' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_int where seq <@ i ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_intrange where seq <@ i ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_float where seq <@ f ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_floatrange where seq <@ f ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_timestamptz where seq <@ t ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_timestampset where seq <@ ts ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_period where seq <@ p ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_periodset where seq <@ ps ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_box where seq <@ b ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tintinst where seq <@ inst ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloatinst where seq <@ inst ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tinti where seq <@ ti ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloati where seq <@ ti ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq <@ t2.seq ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq <@ t2.seq ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tints where seq <@ ts ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloats where seq <@ ts ) 
where op = '<@' and leftarg = 'tintseq' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_int where seq <@ i ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_intrange where seq <@ i ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_float where seq <@ f ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_floatrange where seq <@ f ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_timestamptz where seq <@ t ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_timestampset where seq <@ ts ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_period where seq <@ p ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_periodset where seq <@ ps ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_box where seq <@ b ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tintinst where seq <@ inst ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloatinst where seq <@ inst ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tinti where seq <@ ti ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloati where seq <@ ti ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq <@ t2.seq ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq <@ t2.seq ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tints where seq <@ ts ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloats where seq <@ ts ) 
where op = '<@' and leftarg = 'tfloatseq' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_timestamptz where seq <@ t ) 
where op = '<@' and leftarg = 'ttextseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_timestampset where seq <@ ts ) 
where op = '<@' and leftarg = 'ttextseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_period where seq <@ p ) 
where op = '<@' and leftarg = 'ttextseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_periodset where seq <@ ps ) 
where op = '<@' and leftarg = 'ttextseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttextinst where seq <@ inst ) 
where op = '<@' and leftarg = 'ttextseq' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttexti where seq <@ ti ) 
where op = '<@' and leftarg = 'ttextseq' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq t1, tbl_ttextseq t2 where t1.seq <@ t2.seq ) 
where op = '<@' and leftarg = 'ttextseq' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttexts where seq <@ ts ) 
where op = '<@' and leftarg = 'ttextseq' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_timestamptz where ts <@ t ) 
where op = '<@' and leftarg = 'tbools' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools t1, tbl_timestampset t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'tbools' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_period where ts <@ p ) 
where op = '<@' and leftarg = 'tbools' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_periodset where ts <@ ps ) 
where op = '<@' and leftarg = 'tbools' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tboolinst where ts <@ inst ) 
where op = '<@' and leftarg = 'tbools' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tbooli where ts <@ ti ) 
where op = '<@' and leftarg = 'tbools' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tboolseq where ts <@ seq ) 
where op = '<@' and leftarg = 'tbools' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools t1, tbl_tbools t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'tbools' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_int where ts <@ i ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_intrange where ts <@ i ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_float where ts <@ f ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_floatrange where ts <@ f ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_timestamptz where ts <@ t ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_timestampset t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_period where ts <@ p ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_periodset where ts <@ ps ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_box where ts <@ b ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tintinst where ts <@ inst ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloatinst where ts <@ inst ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tinti where ts <@ ti ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloati where ts <@ ti ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tintseq where ts <@ seq ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloatseq where ts <@ seq ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_tints t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'tints' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_int where ts <@ i ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_intrange where ts <@ i ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_float where ts <@ f ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_floatrange where ts <@ f ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_timestamptz where ts <@ t ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_timestampset t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_period where ts <@ p ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_periodset where ts <@ ps ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_box where ts <@ b ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tintinst where ts <@ inst ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloatinst where ts <@ inst ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tinti where ts <@ ti ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloati where ts <@ ti ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tintseq where ts <@ seq ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloatseq where ts <@ seq ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'tfloats' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_timestamptz where ts <@ t ) 
where op = '<@' and leftarg = 'ttexts' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts t1, tbl_timestampset t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'ttexts' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_period where ts <@ p ) 
where op = '<@' and leftarg = 'ttexts' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_periodset where ts <@ ps ) 
where op = '<@' and leftarg = 'ttexts' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttextinst where ts <@ inst ) 
where op = '<@' and leftarg = 'ttexts' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttexti where ts <@ ti ) 
where op = '<@' and leftarg = 'ttexts' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttextseq where ts <@ seq ) 
where op = '<@' and leftarg = 'ttexts' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts t1, tbl_ttexts t2 where t1.ts <@ t2.ts ) 
where op = '<@' and leftarg = 'ttexts' and rightarg = 'ttexts';

-------------------------------------------------------------------------------
-- Contained
-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tboolinst where t ~= inst ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tintinst where t ~= inst ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloatinst where t ~= inst ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttextinst where t ~= inst ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tbooli where t ~= ti ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tinti where t ~= ti ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloati where t ~= ti ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttexti where t ~= ti ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tboolseq where t ~= seq ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tintseq where t ~= seq ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloatseq where t ~= seq ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttextseq where t ~= seq ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tbools where t ~= ts ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tints where t ~= ts ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_tfloats where t ~= ts ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestamptz, tbl_ttexts where t ~= ts ) 
where op = '~=' and leftarg = 'timestamptz' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tboolinst where ts ~= inst ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tintinst where ts ~= inst ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloatinst where ts ~= inst ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttextinst where ts ~= inst ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tbooli where ts ~= ti ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tinti where ts ~= ti ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloati where ts ~= ti ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttexti where ts ~= ti ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tboolseq where ts ~= seq ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tintseq where ts ~= seq ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_tfloatseq where ts ~= seq ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset, tbl_ttextseq where ts ~= seq ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tbools t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tints t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_tfloats t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_timestampset t1, tbl_ttexts t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'timestampset' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tboolinst where p ~= inst ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tintinst where p ~= inst ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloatinst where p ~= inst ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttextinst where p ~= inst ) 
where op = '~=' and leftarg = 'period' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tbooli where p ~= ti ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tinti where p ~= ti ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloati where p ~= ti ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttexti where p ~= ti ) 
where op = '~=' and leftarg = 'period' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tboolseq where p ~= seq ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tintseq where p ~= seq ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloatseq where p ~= seq ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttextseq where p ~= seq ) 
where op = '~=' and leftarg = 'period' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tbools where p ~= ts ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tints where p ~= ts ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_tfloats where p ~= ts ) 
where op = '~=' and leftarg = 'period' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_period, tbl_ttexts where p ~= ts ) 
where op = '~=' and leftarg = 'period' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tboolinst where ps ~= inst ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tintinst where ps ~= inst ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloatinst where ps ~= inst ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttextinst where ps ~= inst ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'ttextinst';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tbooli where ps ~= ti ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tinti where ps ~= ti ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloati where ps ~= ti ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttexti where ps ~= ti ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'ttexti';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tboolseq where ps ~= seq ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tintseq where ps ~= seq ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloatseq where ps ~= seq ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttextseq where ps ~= seq ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'ttextseq';

update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tbools where ps ~= ts ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tbools';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tints where ps ~= ts ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_tfloats where ps ~= ts ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'tfloats';
update test_boundboxops
set gistidx = ( select count(*) from tbl_periodset, tbl_ttexts where ps ~= ts ) 
where op = '~=' and leftarg = 'periodset' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_timestamptz where inst ~= t ) 
where op = '~=' and leftarg = 'tboolinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_timestampset where inst ~= ts ) 
where op = '~=' and leftarg = 'tboolinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_period where inst ~= p ) 
where op = '~=' and leftarg = 'tboolinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_periodset where inst ~= ps ) 
where op = '~=' and leftarg = 'tboolinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst t1, tbl_tboolinst t2 where t1.inst ~= t2.inst ) 
where op = '~=' and leftarg = 'tboolinst' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tbooli where inst ~= ti ) 
where op = '~=' and leftarg = 'tboolinst' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tboolseq where inst ~= seq ) 
where op = '~=' and leftarg = 'tboolinst' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolinst, tbl_tbools where inst ~= ts ) 
where op = '~=' and leftarg = 'tboolinst' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_int where inst ~= i ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_intrange where inst ~= i ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_float where inst ~= f ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_floatrange where inst ~= f ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_timestamptz where inst ~= t ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_timestampset where inst ~= ts ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_period where inst ~= p ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_periodset where inst ~= ps ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_box where inst ~= b ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst ~= t2.inst ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst ~= t2.inst ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tinti where inst ~= ti ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloati where inst ~= ti ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tintseq where inst ~= seq ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloatseq where inst ~= seq ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tints where inst ~= ts ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintinst, tbl_tfloats where inst ~= ts ) 
where op = '~=' and leftarg = 'tintinst' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_int where inst ~= i ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_intrange where inst ~= i ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_float where inst ~= f ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_floatrange where inst ~= f ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_timestamptz where inst ~= t ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_timestampset where inst ~= ts ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_period where inst ~= p ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_periodset where inst ~= ps ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_box where inst ~= b ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst ~= t2.inst ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst ~= t2.inst ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tinti where inst ~= ti ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'tbl_tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloati where inst ~= ti ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tintseq where inst ~= seq ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloatseq where inst ~= seq ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tints where inst ~= ts ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatinst, tbl_tfloats where inst ~= ts ) 
where op = '~=' and leftarg = 'tfloatinst' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_timestamptz where inst ~= t ) 
where op = '~=' and leftarg = 'ttextinst' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_timestampset where inst ~= ts ) 
where op = '~=' and leftarg = 'ttextinst' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_period where inst ~= p ) 
where op = '~=' and leftarg = 'ttextinst' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_periodset where inst ~= ps ) 
where op = '~=' and leftarg = 'ttextinst' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst t1, tbl_ttextinst t2 where t1.inst ~= t2.inst ) 
where op = '~=' and leftarg = 'ttextinst' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttexti where inst ~= ti ) 
where op = '~=' and leftarg = 'ttextinst' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttextseq where inst ~= seq ) 
where op = '~=' and leftarg = 'ttextinst' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextinst, tbl_ttexts where inst ~= ts ) 
where op = '~=' and leftarg = 'ttextinst' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_timestamptz where ti ~= t ) 
where op = '~=' and leftarg = 'tbooli' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_timestampset where ti ~= ts ) 
where op = '~=' and leftarg = 'tbooli' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_period where ti ~= p ) 
where op = '~=' and leftarg = 'tbooli' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_periodset where ti ~= ps ) 
where op = '~=' and leftarg = 'tbooli' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tboolinst where ti ~= inst ) 
where op = '~=' and leftarg = 'tbooli' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli t1, tbl_tbooli t2 where t1.ti ~= t2.ti ) 
where op = '~=' and leftarg = 'tbooli' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tboolseq where ti ~= seq ) 
where op = '~=' and leftarg = 'tbooli' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbooli, tbl_tbools where ti ~= ts ) 
where op = '~=' and leftarg = 'tbooli' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_int where ti ~= i ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_intrange where ti ~= i ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_float where ti ~= f ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_floatrange where ti ~= f ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_timestamptz where ti ~= t ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_timestampset where ti ~= ts ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_period where ti ~= p ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_periodset where ti ~= ps ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_box where ti ~= b ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tintinst where ti ~= inst ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloatinst where ti ~= inst ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti ~= t2.ti ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti ~= t2.ti ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tintseq where ti ~= seq ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloatseq where ti ~= seq ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tints where ti ~= ts ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tinti, tbl_tfloats where ti ~= ts ) 
where op = '~=' and leftarg = 'tinti' and rightarg = 'tfloats';
	
-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_int where ti ~= i ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_intrange where ti ~= i ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_float where ti ~= f ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_floatrange where ti ~= f ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_timestamptz where ti ~= t ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_timestampset where ti ~= ts ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_period where ti ~= p ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_periodset where ti ~= ps ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_box where ti ~= b ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tintinst where ti ~= inst ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloatinst where ti ~= inst ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti ~= t2.ti ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti ~= t2.ti ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tintseq where ti ~= seq ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloatseq where ti ~= seq ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tints where ti ~= ts ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloati, tbl_tfloats where ti ~= ts ) 
where op = '~=' and leftarg = 'tfloati' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_timestamptz where ti ~= t ) 
where op = '~=' and leftarg = 'ttexti' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_timestampset where ti ~= ts ) 
where op = '~=' and leftarg = 'ttexti' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_period where ti ~= p ) 
where op = '~=' and leftarg = 'ttexti' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_periodset where ti ~= ps ) 
where op = '~=' and leftarg = 'ttexti' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttextinst where ti ~= inst ) 
where op = '~=' and leftarg = 'ttexti' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti t1, tbl_ttexti t2 where t1.ti ~= t2.ti ) 
where op = '~=' and leftarg = 'ttexti' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttextseq where ti ~= seq ) 
where op = '~=' and leftarg = 'ttexti' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexti, tbl_ttexts where ti ~= ts ) 
where op = '~=' and leftarg = 'ttexti' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_timestamptz where seq ~= t ) 
where op = '~=' and leftarg = 'tboolseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_timestampset where seq ~= ts ) 
where op = '~=' and leftarg = 'tboolseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_period where seq ~= p ) 
where op = '~=' and leftarg = 'tboolseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_periodset where seq ~= ps ) 
where op = '~=' and leftarg = 'tboolseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tboolinst where seq ~= inst ) 
where op = '~=' and leftarg = 'tboolseq' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tbooli where seq ~= ti ) 
where op = '~=' and leftarg = 'tboolseq' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq t1, tbl_tboolseq t2 where t1.seq ~= t2.seq ) 
where op = '~=' and leftarg = 'tboolseq' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tboolseq, tbl_tbools where seq ~= ts ) 
where op = '~=' and leftarg = 'tboolseq' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_int where seq ~= i ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_intrange where seq ~= i ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_float where seq ~= f ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_floatrange where seq ~= f ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_timestamptz where seq ~= t ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_timestampset where seq ~= ts ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_period where seq ~= p ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_periodset where seq ~= ps ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_box where seq ~= b ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tintinst where seq ~= inst ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloatinst where seq ~= inst ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tinti where seq ~= ti ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloati where seq ~= ti ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq ~= t2.seq ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq ~= t2.seq ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tints where seq ~= ts ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tintseq, tbl_tfloats where seq ~= ts ) 
where op = '~=' and leftarg = 'tintseq' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_int where seq ~= i ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_intrange where seq ~= i ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_float where seq ~= f ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_floatrange where seq ~= f ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_timestamptz where seq ~= t ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_timestampset where seq ~= ts ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_period where seq ~= p ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_periodset where seq ~= ps ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_box where seq ~= b ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tintinst where seq ~= inst ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloatinst where seq ~= inst ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tinti where seq ~= ti ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloati where seq ~= ti ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq ~= t2.seq ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq ~= t2.seq ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tints where seq ~= ts ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloatseq, tbl_tfloats where seq ~= ts ) 
where op = '~=' and leftarg = 'tfloatseq' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_timestamptz where seq ~= t ) 
where op = '~=' and leftarg = 'ttextseq' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_timestampset where seq ~= ts ) 
where op = '~=' and leftarg = 'ttextseq' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_period where seq ~= p ) 
where op = '~=' and leftarg = 'ttextseq' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_periodset where seq ~= ps ) 
where op = '~=' and leftarg = 'ttextseq' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttextinst where seq ~= inst ) 
where op = '~=' and leftarg = 'ttextseq' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttexti where seq ~= ti ) 
where op = '~=' and leftarg = 'ttextseq' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq t1, tbl_ttextseq t2 where t1.seq ~= t2.seq ) 
where op = '~=' and leftarg = 'ttextseq' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttextseq, tbl_ttexts where seq ~= ts ) 
where op = '~=' and leftarg = 'ttextseq' and rightarg = 'ttexts';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_timestamptz where ts ~= t ) 
where op = '~=' and leftarg = 'tbools' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools t1, tbl_timestampset t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'tbools' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_period where ts ~= p ) 
where op = '~=' and leftarg = 'tbools' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_periodset where ts ~= ps ) 
where op = '~=' and leftarg = 'tbools' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tboolinst where ts ~= inst ) 
where op = '~=' and leftarg = 'tbools' and rightarg = 'tboolinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tbooli where ts ~= ti ) 
where op = '~=' and leftarg = 'tbools' and rightarg = 'tbooli';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools, tbl_tboolseq where ts ~= seq ) 
where op = '~=' and leftarg = 'tbools' and rightarg = 'tboolseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tbools t1, tbl_tbools t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'tbools' and rightarg = 'tbools';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_int where ts ~= i ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_intrange where ts ~= i ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_float where ts ~= f ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_floatrange where ts ~= f ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_timestamptz where ts ~= t ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_timestampset t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_period where ts ~= p ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_periodset where ts ~= ps ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_box where ts ~= b ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tintinst where ts ~= inst ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloatinst where ts ~= inst ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tinti where ts ~= ti ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloati where ts ~= ti ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tintseq where ts ~= seq ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints, tbl_tfloatseq where ts ~= seq ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_tints t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'tints' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_int where ts ~= i ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'int';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_intrange where ts ~= i ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'intrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_float where ts ~= f ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'float';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_floatrange where ts ~= f ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'floatrange';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_timestamptz where ts ~= t ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_timestampset t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_period where ts ~= p ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_periodset where ts ~= ps ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_box where ts ~= b ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'box';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tintinst where ts ~= inst ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'tintinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloatinst where ts ~= inst ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'tfloatinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tinti where ts ~= ti ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'tinti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloati where ts ~= ti ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'tfloati';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tintseq where ts ~= seq ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'tintseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats, tbl_tfloatseq where ts ~= seq ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'tfloatseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'tints';
update test_boundboxops
set gistidx = ( select count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'tfloats' and rightarg = 'tfloats';

-------------------------------------------------------------------------------

update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_timestamptz where ts ~= t ) 
where op = '~=' and leftarg = 'ttexts' and rightarg = 'timestamptz';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts t1, tbl_timestampset t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'ttexts' and rightarg = 'timestampset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_period where ts ~= p ) 
where op = '~=' and leftarg = 'ttexts' and rightarg = 'period';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_periodset where ts ~= ps ) 
where op = '~=' and leftarg = 'ttexts' and rightarg = 'periodset';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttextinst where ts ~= inst ) 
where op = '~=' and leftarg = 'ttexts' and rightarg = 'ttextinst';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttexti where ts ~= ti ) 
where op = '~=' and leftarg = 'ttexts' and rightarg = 'ttexti';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts, tbl_ttextseq where ts ~= seq ) 
where op = '~=' and leftarg = 'ttexts' and rightarg = 'ttextseq';
update test_boundboxops
set gistidx = ( select count(*) from tbl_ttexts t1, tbl_ttexts t2 where t1.ts ~= t2.ts ) 
where op = '~=' and leftarg = 'ttexts' and rightarg = 'ttexts';

/*****************************************************************************/
