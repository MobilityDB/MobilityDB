/*****************************************************************************/

DROP INDEX IF EXISTS tbl_tboolinst_gist_idx;
DROP INDEX IF EXISTS tbl_tboolinst_spgist_idx;

DROP INDEX IF EXISTS tbl_tintinst_gist_idx;
DROP INDEX IF EXISTS tbl_tintinst_spgist_idx;

DROP INDEX IF EXISTS tbl_tfloatinst_gist_idx;
DROP INDEX IF EXISTS tbl_tfloatinst_spgist_idx;

DROP INDEX IF EXISTS tbl_ttextinst_gist_idx;
DROP INDEX IF EXISTS tbl_ttextinst_spgist_idx;

DROP INDEX IF EXISTS tbl_tbooli_gist_idx;
DROP INDEX IF EXISTS tbl_tbooli_spgist_idx;

DROP INDEX IF EXISTS tbl_tinti_gist_idx;
DROP INDEX IF EXISTS tbl_tinti_spgist_idx;

DROP INDEX IF EXISTS tbl_tfloati_gist_idx;
DROP INDEX IF EXISTS tbl_tfloati_spgist_idx;

DROP INDEX IF EXISTS tbl_ttexti_gist_idx;
DROP INDEX IF EXISTS tbl_ttexti_spgist_idx;

DROP INDEX IF EXISTS tbl_tboolseq_gist_idx;
DROP INDEX IF EXISTS tbl_tboolseq_spgist_idx;

DROP INDEX IF EXISTS tbl_tintseq_gist_idx;
DROP INDEX IF EXISTS tbl_tintseq_spgist_idx;

DROP INDEX IF EXISTS tbl_tfloatseq_gist_idx;
DROP INDEX IF EXISTS tbl_tfloatseq_spgist_idx;

DROP INDEX IF EXISTS tbl_ttextseq_gist_idx;
DROP INDEX IF EXISTS tbl_ttextseq_spgist_idx;

DROP INDEX IF EXISTS tbl_tbools_gist_idx;
DROP INDEX IF EXISTS tbl_tbools_spgist_idx;

DROP INDEX IF EXISTS tbl_tints_gist_idx;
DROP INDEX IF EXISTS tbl_tints_spgist_idx;

DROP INDEX IF EXISTS tbl_tfloats_gist_idx;
DROP INDEX IF EXISTS tbl_tfloats_spgist_idx;

DROP INDEX IF EXISTS tbl_ttexts_gist_idx;
DROP INDEX IF EXISTS tbl_ttexts_spgist_idx;

/*****************************************************************************/

drop table if exists test_relativeposops;
create table test_relativeposops(
	op char(3), 
	leftarg text, 
	rightarg text, 
	noidx bigint,
	gistidx bigint,
	spgistidx bigint );

/*****************************************************************************/

-------------------------------------------------------------------------------
-- Left
-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'int', 'tintinst', count(*) from tbl_int, tbl_tintinst where i << inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'int', 'tfloatinst', count(*) from tbl_int, tbl_tfloatinst where i << inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'int', 'tinti', count(*) from tbl_int, tbl_tinti where i << ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'int', 'tfloati', count(*) from tbl_int, tbl_tfloati where i << ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'int', 'tintseq', count(*) from tbl_int, tbl_tintseq where i << seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'int', 'tfloatseq', count(*) from tbl_int, tbl_tfloatseq where i << seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'int', 'tints', count(*) from tbl_int, tbl_tints where i << ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'int', 'tfloats', count(*) from tbl_int, tbl_tfloats where i << ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'float', 'tintinst', count(*) from tbl_float, tbl_tintinst where f << inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'float', 'tfloatinst', count(*) from tbl_float, tbl_tfloatinst where f << inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'float', 'tinti', count(*) from tbl_float, tbl_tinti where f << ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'float', 'tfloati', count(*) from tbl_float, tbl_tfloati where f << ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'float', 'tintseq', count(*) from tbl_float, tbl_tintseq where f << seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'float', 'tfloatseq', count(*) from tbl_float, tbl_tfloatseq where f << seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'float', 'tints', count(*) from tbl_float, tbl_tints where f << ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'float', 'tfloats', count(*) from tbl_float, tbl_tfloats where f << ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintinst', 'int', count(*) from tbl_tintinst, tbl_int where inst << i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintinst', 'float', count(*) from tbl_tintinst, tbl_float where inst << f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintinst', 'tintinst', count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst << t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintinst', 'tfloatinst', count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst << t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintinst', 'tinti', count(*) from tbl_tintinst, tbl_tinti where inst << ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintinst', 'tfloati', count(*) from tbl_tintinst, tbl_tfloati where inst << ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintinst', 'tintseq', count(*) from tbl_tintinst, tbl_tintseq where inst << seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintinst', 'tfloatseq', count(*) from tbl_tintinst, tbl_tfloatseq where inst << seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintinst', 'tints', count(*) from tbl_tintinst, tbl_tints where inst << ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintinst', 'tfloats', count(*) from tbl_tintinst, tbl_tfloats where inst << ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatinst', 'int', count(*) from tbl_tfloatinst, tbl_int where inst << i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatinst', 'float', count(*) from tbl_tfloatinst, tbl_float where inst << f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatinst', 'tintinst', count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst << t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatinst', 'tfloatinst', count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst << t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatinst', 'tinti', count(*) from tbl_tfloatinst, tbl_tinti where inst << ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatinst', 'tfloati', count(*) from tbl_tfloatinst, tbl_tfloati where inst << ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatinst', 'tintseq', count(*) from tbl_tfloatinst, tbl_tintseq where inst << seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatinst', 'tfloatseq', count(*) from tbl_tfloatinst, tbl_tfloatseq where inst << seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatinst', 'tints', count(*) from tbl_tfloatinst, tbl_tints where inst << ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatinst', 'tfloats', count(*) from tbl_tfloatinst, tbl_tfloats where inst << ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tinti', 'int', count(*) from tbl_tinti, tbl_int where ti << i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tinti', 'float', count(*) from tbl_tinti, tbl_float where ti << f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tinti', 'tintinst', count(*) from tbl_tinti, tbl_tintinst where ti << inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tinti', 'tfloatinst', count(*) from tbl_tinti, tbl_tfloatinst where ti << inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tinti', 'tinti', count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti << t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tinti', 'tfloati', count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti << t2.ti;
	
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tinti', 'tintseq', count(*) from tbl_tinti, tbl_tintseq where ti << seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tinti', 'tfloatseq', count(*) from tbl_tinti, tbl_tfloatseq where ti << seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tinti', 'tints', count(*) from tbl_tinti, tbl_tints where ti << ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tinti', 'tfloats', count(*) from tbl_tinti, tbl_tfloats where ti << ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloati', 'int', count(*) from tbl_tfloati, tbl_int where ti << i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloati', 'float', count(*) from tbl_tfloati, tbl_float where ti << f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloati', 'tintinst', count(*) from tbl_tfloati, tbl_tintinst where ti << inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloati', 'tfloatinst', count(*) from tbl_tfloati, tbl_tfloatinst where ti << inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloati', 'tinti', count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti << t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloati', 'tfloati', count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti << t2.ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloati', 'tintseq', count(*) from tbl_tfloati, tbl_tintseq where ti << seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloati', 'tfloatseq', count(*) from tbl_tfloati, tbl_tfloatseq where ti << seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloati', 'tints', count(*) from tbl_tfloati, tbl_tints where ti << ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloati', 'tfloats', count(*) from tbl_tfloati, tbl_tfloats where ti << ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintseq', 'int', count(*) from tbl_tintseq, tbl_int where seq << i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintseq', 'float', count(*) from tbl_tintseq, tbl_float where seq << f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintseq', 'tintinst', count(*) from tbl_tintseq, tbl_tintinst where seq << inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintseq', 'tfloatinst', count(*) from tbl_tintseq, tbl_tfloatinst where seq << inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintseq', 'tinti', count(*) from tbl_tintseq, tbl_tinti where seq << ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintseq', 'tfloati', count(*) from tbl_tintseq, tbl_tfloati where seq << ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintseq', 'tintseq', count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq << t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintseq', 'tfloatseq', count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq << t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintseq', 'tints', count(*) from tbl_tintseq, tbl_tints where seq << ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tintseq', 'tfloats', count(*) from tbl_tintseq, tbl_tfloats where seq << ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatseq', 'int', count(*) from tbl_tfloatseq, tbl_int where seq << i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatseq', 'float', count(*) from tbl_tfloatseq, tbl_float where seq << f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatseq', 'tintinst', count(*) from tbl_tfloatseq, tbl_tintinst where seq << inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatseq', 'tfloatinst', count(*) from tbl_tfloatseq, tbl_tfloatinst where seq << inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatseq', 'tinti', count(*) from tbl_tfloatseq, tbl_tinti where seq << ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatseq', 'tfloati', count(*) from tbl_tfloatseq, tbl_tfloati where seq << ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatseq', 'tintseq', count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq << t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatseq', 'tfloatseq', count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq << t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatseq', 'tints', count(*) from tbl_tfloatseq, tbl_tints where seq << ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloatseq', 'tfloats', count(*) from tbl_tfloatseq, tbl_tfloats where seq << ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tints', 'int', count(*) from tbl_tints, tbl_int where ts << i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tints', 'float', count(*) from tbl_tints, tbl_float where ts << f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tints', 'tintinst', count(*) from tbl_tints, tbl_tintinst where ts << inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tints', 'tfloatinst', count(*) from tbl_tints, tbl_tfloatinst where ts << inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tints', 'tinti', count(*) from tbl_tints, tbl_tinti where ts << ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tints', 'tfloati', count(*) from tbl_tints, tbl_tfloati where ts << ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tints', 'tintseq', count(*) from tbl_tints, tbl_tintseq where ts << seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tints', 'tfloatseq', count(*) from tbl_tints, tbl_tfloatseq where ts << seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tints', 'tints', count(*) from tbl_tints t1, tbl_tints t2 where t1.ts << t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tints', 'tfloats', count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts << t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloats', 'int', count(*) from tbl_tfloats, tbl_int where ts << i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloats', 'float', count(*) from tbl_tfloats, tbl_float where ts << f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloats', 'tintinst', count(*) from tbl_tfloats, tbl_tintinst where ts << inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloats', 'tfloatinst', count(*) from tbl_tfloats, tbl_tfloatinst where ts << inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloats', 'tinti', count(*) from tbl_tfloats, tbl_tinti where ts << ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloats', 'tfloati', count(*) from tbl_tfloats, tbl_tfloati where ts << ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloats', 'tintseq', count(*) from tbl_tfloats, tbl_tintseq where ts << seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloats', 'tfloatseq', count(*) from tbl_tfloats, tbl_tfloatseq where ts << seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloats', 'tints', count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts << t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<', 'tfloats', 'tfloats', count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts << t2.ts;

-------------------------------------------------------------------------------
-- Overleft
-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'int', 'tintinst', count(*) from tbl_int, tbl_tintinst where i &< inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'int', 'tfloatinst', count(*) from tbl_int, tbl_tfloatinst where i &< inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'int', 'tinti', count(*) from tbl_int, tbl_tinti where i &< ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'int', 'tfloati', count(*) from tbl_int, tbl_tfloati where i &< ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'int', 'tintseq', count(*) from tbl_int, tbl_tintseq where i &< seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'int', 'tfloatseq', count(*) from tbl_int, tbl_tfloatseq where i &< seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'int', 'tints', count(*) from tbl_int, tbl_tints where i &< ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'int', 'tfloats', count(*) from tbl_int, tbl_tfloats where i &< ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'float', 'tintinst', count(*) from tbl_float, tbl_tintinst where f &< inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'float', 'tfloatinst', count(*) from tbl_float, tbl_tfloatinst where f &< inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'float', 'tinti', count(*) from tbl_float, tbl_tinti where f &< ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'float', 'tfloati', count(*) from tbl_float, tbl_tfloati where f &< ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'float', 'tintseq', count(*) from tbl_float, tbl_tintseq where f &< seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'float', 'tfloatseq', count(*) from tbl_float, tbl_tfloatseq where f &< seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'float', 'tints', count(*) from tbl_float, tbl_tints where f &< ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'float', 'tfloats', count(*) from tbl_float, tbl_tfloats where f &< ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintinst', 'int', count(*) from tbl_tintinst, tbl_int where inst &< i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintinst', 'float', count(*) from tbl_tintinst, tbl_float where inst &< f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintinst', 'tintinst', count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst &< t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintinst', 'tfloatinst', count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst &< t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintinst', 'tinti', count(*) from tbl_tintinst, tbl_tinti where inst &< ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintinst', 'tfloati', count(*) from tbl_tintinst, tbl_tfloati where inst &< ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintinst', 'tintseq', count(*) from tbl_tintinst, tbl_tintseq where inst &< seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintinst', 'tfloatseq', count(*) from tbl_tintinst, tbl_tfloatseq where inst &< seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintinst', 'tints', count(*) from tbl_tintinst, tbl_tints where inst &< ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintinst', 'tfloats', count(*) from tbl_tintinst, tbl_tfloats where inst &< ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatinst', 'int', count(*) from tbl_tfloatinst, tbl_int where inst &< i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatinst', 'float', count(*) from tbl_tfloatinst, tbl_float where inst &< f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatinst', 'tintinst', count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst &< t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatinst', 'tfloatinst', count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst &< t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatinst', 'tinti', count(*) from tbl_tfloatinst, tbl_tinti where inst &< ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatinst', 'tfloati', count(*) from tbl_tfloatinst, tbl_tfloati where inst &< ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatinst', 'tintseq', count(*) from tbl_tfloatinst, tbl_tintseq where inst &< seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatinst', 'tfloatseq', count(*) from tbl_tfloatinst, tbl_tfloatseq where inst &< seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatinst', 'tints', count(*) from tbl_tfloatinst, tbl_tints where inst &< ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatinst', 'tfloats', count(*) from tbl_tfloatinst, tbl_tfloats where inst &< ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tinti', 'int', count(*) from tbl_tinti, tbl_int where ti &< i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tinti', 'float', count(*) from tbl_tinti, tbl_float where ti &< f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tinti', 'tintinst', count(*) from tbl_tinti, tbl_tintinst where ti &< inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tinti', 'tfloatinst', count(*) from tbl_tinti, tbl_tfloatinst where ti &< inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tinti', 'tinti', count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti &< t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tinti', 'tfloati', count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti &< t2.ti;
	
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tinti', 'tintseq', count(*) from tbl_tinti, tbl_tintseq where ti &< seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tinti', 'tfloatseq', count(*) from tbl_tinti, tbl_tfloatseq where ti &< seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tinti', 'tints', count(*) from tbl_tinti, tbl_tints where ti &< ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tinti', 'tfloats', count(*) from tbl_tinti, tbl_tfloats where ti &< ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloati', 'int', count(*) from tbl_tfloati, tbl_int where ti &< i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloati', 'float', count(*) from tbl_tfloati, tbl_float where ti &< f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloati', 'tintinst', count(*) from tbl_tfloati, tbl_tintinst where ti &< inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloati', 'tfloatinst', count(*) from tbl_tfloati, tbl_tfloatinst where ti &< inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloati', 'tinti', count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti &< t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloati', 'tfloati', count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti &< t2.ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloati', 'tintseq', count(*) from tbl_tfloati, tbl_tintseq where ti &< seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloati', 'tfloatseq', count(*) from tbl_tfloati, tbl_tfloatseq where ti &< seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloati', 'tints', count(*) from tbl_tfloati, tbl_tints where ti &< ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloati', 'tfloats', count(*) from tbl_tfloati, tbl_tfloats where ti &< ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintseq', 'int', count(*) from tbl_tintseq, tbl_int where seq &< i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintseq', 'float', count(*) from tbl_tintseq, tbl_float where seq &< f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintseq', 'tintinst', count(*) from tbl_tintseq, tbl_tintinst where seq &< inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintseq', 'tfloatinst', count(*) from tbl_tintseq, tbl_tfloatinst where seq &< inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintseq', 'tinti', count(*) from tbl_tintseq, tbl_tinti where seq &< ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintseq', 'tfloati', count(*) from tbl_tintseq, tbl_tfloati where seq &< ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintseq', 'tintseq', count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq &< t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintseq', 'tfloatseq', count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq &< t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintseq', 'tints', count(*) from tbl_tintseq, tbl_tints where seq &< ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tintseq', 'tfloats', count(*) from tbl_tintseq, tbl_tfloats where seq &< ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatseq', 'int', count(*) from tbl_tfloatseq, tbl_int where seq &< i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatseq', 'float', count(*) from tbl_tfloatseq, tbl_float where seq &< f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatseq', 'tintinst', count(*) from tbl_tfloatseq, tbl_tintinst where seq &< inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatseq', 'tfloatinst', count(*) from tbl_tfloatseq, tbl_tfloatinst where seq &< inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatseq', 'tinti', count(*) from tbl_tfloatseq, tbl_tinti where seq &< ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatseq', 'tfloati', count(*) from tbl_tfloatseq, tbl_tfloati where seq &< ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatseq', 'tintseq', count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq &< t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatseq', 'tfloatseq', count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq &< t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatseq', 'tints', count(*) from tbl_tfloatseq, tbl_tints where seq &< ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloatseq', 'tfloats', count(*) from tbl_tfloatseq, tbl_tfloats where seq &< ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tints', 'int', count(*) from tbl_tints, tbl_int where ts &< i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tints', 'float', count(*) from tbl_tints, tbl_float where ts &< f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tints', 'tintinst', count(*) from tbl_tints, tbl_tintinst where ts &< inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tints', 'tfloatinst', count(*) from tbl_tints, tbl_tfloatinst where ts &< inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tints', 'tinti', count(*) from tbl_tints, tbl_tinti where ts &< ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tints', 'tfloati', count(*) from tbl_tints, tbl_tfloati where ts &< ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tints', 'tintseq', count(*) from tbl_tints, tbl_tintseq where ts &< seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tints', 'tfloatseq', count(*) from tbl_tints, tbl_tfloatseq where ts &< seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tints', 'tints', count(*) from tbl_tints t1, tbl_tints t2 where t1.ts &< t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tints', 'tfloats', count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts &< t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloats', 'int', count(*) from tbl_tfloats, tbl_int where ts &< i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloats', 'float', count(*) from tbl_tfloats, tbl_float where ts &< f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloats', 'tintinst', count(*) from tbl_tfloats, tbl_tintinst where ts &< inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloats', 'tfloatinst', count(*) from tbl_tfloats, tbl_tfloatinst where ts &< inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloats', 'tinti', count(*) from tbl_tfloats, tbl_tinti where ts &< ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloats', 'tfloati', count(*) from tbl_tfloats, tbl_tfloati where ts &< ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloats', 'tintseq', count(*) from tbl_tfloats, tbl_tintseq where ts &< seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloats', 'tfloatseq', count(*) from tbl_tfloats, tbl_tfloatseq where ts &< seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloats', 'tints', count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts &< t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<', 'tfloats', 'tfloats', count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts &< t2.ts;

-------------------------------------------------------------------------------
-- Right
-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'int', 'tintinst', count(*) from tbl_int, tbl_tintinst where i >> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'int', 'tfloatinst', count(*) from tbl_int, tbl_tfloatinst where i >> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'int', 'tinti', count(*) from tbl_int, tbl_tinti where i >> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'int', 'tfloati', count(*) from tbl_int, tbl_tfloati where i >> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'int', 'tintseq', count(*) from tbl_int, tbl_tintseq where i >> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'int', 'tfloatseq', count(*) from tbl_int, tbl_tfloatseq where i >> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'int', 'tints', count(*) from tbl_int, tbl_tints where i >> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'int', 'tfloats', count(*) from tbl_int, tbl_tfloats where i >> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'float', 'tintinst', count(*) from tbl_float, tbl_tintinst where f >> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'float', 'tfloatinst', count(*) from tbl_float, tbl_tfloatinst where f >> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'float', 'tinti', count(*) from tbl_float, tbl_tinti where f >> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'float', 'tfloati', count(*) from tbl_float, tbl_tfloati where f >> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'float', 'tintseq', count(*) from tbl_float, tbl_tintseq where f >> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'float', 'tfloatseq', count(*) from tbl_float, tbl_tfloatseq where f >> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'float', 'tints', count(*) from tbl_float, tbl_tints where f >> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'float', 'tfloats', count(*) from tbl_float, tbl_tfloats where f >> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintinst', 'int', count(*) from tbl_tintinst, tbl_int where inst >> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintinst', 'float', count(*) from tbl_tintinst, tbl_float where inst >> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintinst', 'tintinst', count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst >> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintinst', 'tfloatinst', count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst >> t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintinst', 'tinti', count(*) from tbl_tintinst, tbl_tinti where inst >> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintinst', 'tfloati', count(*) from tbl_tintinst, tbl_tfloati where inst >> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintinst', 'tintseq', count(*) from tbl_tintinst, tbl_tintseq where inst >> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintinst', 'tfloatseq', count(*) from tbl_tintinst, tbl_tfloatseq where inst >> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintinst', 'tints', count(*) from tbl_tintinst, tbl_tints where inst >> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintinst', 'tfloats', count(*) from tbl_tintinst, tbl_tfloats where inst >> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatinst', 'int', count(*) from tbl_tfloatinst, tbl_int where inst >> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatinst', 'float', count(*) from tbl_tfloatinst, tbl_float where inst >> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatinst', 'tintinst', count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst >> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatinst', 'tfloatinst', count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst >> t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatinst', 'tinti', count(*) from tbl_tfloatinst, tbl_tinti where inst >> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatinst', 'tfloati', count(*) from tbl_tfloatinst, tbl_tfloati where inst >> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatinst', 'tintseq', count(*) from tbl_tfloatinst, tbl_tintseq where inst >> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatinst', 'tfloatseq', count(*) from tbl_tfloatinst, tbl_tfloatseq where inst >> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatinst', 'tints', count(*) from tbl_tfloatinst, tbl_tints where inst >> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatinst', 'tfloats', count(*) from tbl_tfloatinst, tbl_tfloats where inst >> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tinti', 'int', count(*) from tbl_tinti, tbl_int where ti >> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tinti', 'float', count(*) from tbl_tinti, tbl_float where ti >> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tinti', 'tintinst', count(*) from tbl_tinti, tbl_tintinst where ti >> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tinti', 'tfloatinst', count(*) from tbl_tinti, tbl_tfloatinst where ti >> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tinti', 'tinti', count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti >> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tinti', 'tfloati', count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti >> t2.ti;
	
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tinti', 'tintseq', count(*) from tbl_tinti, tbl_tintseq where ti >> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tinti', 'tfloatseq', count(*) from tbl_tinti, tbl_tfloatseq where ti >> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tinti', 'tints', count(*) from tbl_tinti, tbl_tints where ti >> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tinti', 'tfloats', count(*) from tbl_tinti, tbl_tfloats where ti >> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloati', 'int', count(*) from tbl_tfloati, tbl_int where ti >> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloati', 'float', count(*) from tbl_tfloati, tbl_float where ti >> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloati', 'tintinst', count(*) from tbl_tfloati, tbl_tintinst where ti >> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloati', 'tfloatinst', count(*) from tbl_tfloati, tbl_tfloatinst where ti >> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloati', 'tinti', count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti >> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloati', 'tfloati', count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti >> t2.ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloati', 'tintseq', count(*) from tbl_tfloati, tbl_tintseq where ti >> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloati', 'tfloatseq', count(*) from tbl_tfloati, tbl_tfloatseq where ti >> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloati', 'tints', count(*) from tbl_tfloati, tbl_tints where ti >> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloati', 'tfloats', count(*) from tbl_tfloati, tbl_tfloats where ti >> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintseq', 'int', count(*) from tbl_tintseq, tbl_int where seq >> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintseq', 'float', count(*) from tbl_tintseq, tbl_float where seq >> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintseq', 'tintinst', count(*) from tbl_tintseq, tbl_tintinst where seq >> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintseq', 'tfloatinst', count(*) from tbl_tintseq, tbl_tfloatinst where seq >> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintseq', 'tinti', count(*) from tbl_tintseq, tbl_tinti where seq >> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintseq', 'tfloati', count(*) from tbl_tintseq, tbl_tfloati where seq >> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintseq', 'tintseq', count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq >> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintseq', 'tfloatseq', count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq >> t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintseq', 'tints', count(*) from tbl_tintseq, tbl_tints where seq >> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tintseq', 'tfloats', count(*) from tbl_tintseq, tbl_tfloats where seq >> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatseq', 'int', count(*) from tbl_tfloatseq, tbl_int where seq >> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatseq', 'float', count(*) from tbl_tfloatseq, tbl_float where seq >> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatseq', 'tintinst', count(*) from tbl_tfloatseq, tbl_tintinst where seq >> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatseq', 'tfloatinst', count(*) from tbl_tfloatseq, tbl_tfloatinst where seq >> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatseq', 'tinti', count(*) from tbl_tfloatseq, tbl_tinti where seq >> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatseq', 'tfloati', count(*) from tbl_tfloatseq, tbl_tfloati where seq >> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatseq', 'tintseq', count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq >> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatseq', 'tfloatseq', count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq >> t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatseq', 'tints', count(*) from tbl_tfloatseq, tbl_tints where seq >> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloatseq', 'tfloats', count(*) from tbl_tfloatseq, tbl_tfloats where seq >> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tints', 'int', count(*) from tbl_tints, tbl_int where ts >> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tints', 'float', count(*) from tbl_tints, tbl_float where ts >> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tints', 'tintinst', count(*) from tbl_tints, tbl_tintinst where ts >> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tints', 'tfloatinst', count(*) from tbl_tints, tbl_tfloatinst where ts >> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tints', 'tinti', count(*) from tbl_tints, tbl_tinti where ts >> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tints', 'tfloati', count(*) from tbl_tints, tbl_tfloati where ts >> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tints', 'tintseq', count(*) from tbl_tints, tbl_tintseq where ts >> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tints', 'tfloatseq', count(*) from tbl_tints, tbl_tfloatseq where ts >> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tints', 'tints', count(*) from tbl_tints t1, tbl_tints t2 where t1.ts >> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tints', 'tfloats', count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts >> t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloats', 'int', count(*) from tbl_tfloats, tbl_int where ts >> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloats', 'float', count(*) from tbl_tfloats, tbl_float where ts >> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloats', 'tintinst', count(*) from tbl_tfloats, tbl_tintinst where ts >> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloats', 'tfloatinst', count(*) from tbl_tfloats, tbl_tfloatinst where ts >> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloats', 'tinti', count(*) from tbl_tfloats, tbl_tinti where ts >> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloats', 'tfloati', count(*) from tbl_tfloats, tbl_tfloati where ts >> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloats', 'tintseq', count(*) from tbl_tfloats, tbl_tintseq where ts >> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloats', 'tfloatseq', count(*) from tbl_tfloats, tbl_tfloatseq where ts >> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloats', 'tints', count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts >> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '>>', 'tfloats', 'tfloats', count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts >> t2.ts;

-------------------------------------------------------------------------------
-- Overright
-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'int', 'tintinst', count(*) from tbl_int, tbl_tintinst where i &> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'int', 'tfloatinst', count(*) from tbl_int, tbl_tfloatinst where i &> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'int', 'tinti', count(*) from tbl_int, tbl_tinti where i &> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'int', 'tfloati', count(*) from tbl_int, tbl_tfloati where i &> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'int', 'tintseq', count(*) from tbl_int, tbl_tintseq where i &> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'int', 'tfloatseq', count(*) from tbl_int, tbl_tfloatseq where i &> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'int', 'tints', count(*) from tbl_int, tbl_tints where i &> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'int', 'tfloats', count(*) from tbl_int, tbl_tfloats where i &> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'float', 'tintinst', count(*) from tbl_float, tbl_tintinst where f &> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'float', 'tfloatinst', count(*) from tbl_float, tbl_tfloatinst where f &> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'float', 'tinti', count(*) from tbl_float, tbl_tinti where f &> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'float', 'tfloati', count(*) from tbl_float, tbl_tfloati where f &> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'float', 'tintseq', count(*) from tbl_float, tbl_tintseq where f &> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'float', 'tfloatseq', count(*) from tbl_float, tbl_tfloatseq where f &> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'float', 'tints', count(*) from tbl_float, tbl_tints where f &> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'float', 'tfloats', count(*) from tbl_float, tbl_tfloats where f &> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintinst', 'int', count(*) from tbl_tintinst, tbl_int where inst &> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintinst', 'float', count(*) from tbl_tintinst, tbl_float where inst &> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintinst', 'tintinst', count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst &> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintinst', 'tfloatinst', count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst &> t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintinst', 'tinti', count(*) from tbl_tintinst, tbl_tinti where inst &> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintinst', 'tfloati', count(*) from tbl_tintinst, tbl_tfloati where inst &> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintinst', 'tintseq', count(*) from tbl_tintinst, tbl_tintseq where inst &> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintinst', 'tfloatseq', count(*) from tbl_tintinst, tbl_tfloatseq where inst &> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintinst', 'tints', count(*) from tbl_tintinst, tbl_tints where inst &> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintinst', 'tfloats', count(*) from tbl_tintinst, tbl_tfloats where inst &> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatinst', 'int', count(*) from tbl_tfloatinst, tbl_int where inst &> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatinst', 'float', count(*) from tbl_tfloatinst, tbl_float where inst &> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatinst', 'tintinst', count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst &> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatinst', 'tfloatinst', count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst &> t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatinst', 'tinti', count(*) from tbl_tfloatinst, tbl_tinti where inst &> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatinst', 'tfloati', count(*) from tbl_tfloatinst, tbl_tfloati where inst &> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatinst', 'tintseq', count(*) from tbl_tfloatinst, tbl_tintseq where inst &> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatinst', 'tfloatseq', count(*) from tbl_tfloatinst, tbl_tfloatseq where inst &> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatinst', 'tints', count(*) from tbl_tfloatinst, tbl_tints where inst &> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatinst', 'tfloats', count(*) from tbl_tfloatinst, tbl_tfloats where inst &> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tinti', 'int', count(*) from tbl_tinti, tbl_int where ti &> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tinti', 'float', count(*) from tbl_tinti, tbl_float where ti &> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tinti', 'tintinst', count(*) from tbl_tinti, tbl_tintinst where ti &> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tinti', 'tfloatinst', count(*) from tbl_tinti, tbl_tfloatinst where ti &> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tinti', 'tinti', count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti &> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tinti', 'tfloati', count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti &> t2.ti;
	
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tinti', 'tintseq', count(*) from tbl_tinti, tbl_tintseq where ti &> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tinti', 'tfloatseq', count(*) from tbl_tinti, tbl_tfloatseq where ti &> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tinti', 'tints', count(*) from tbl_tinti, tbl_tints where ti &> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tinti', 'tfloats', count(*) from tbl_tinti, tbl_tfloats where ti &> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloati', 'int', count(*) from tbl_tfloati, tbl_int where ti &> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloati', 'float', count(*) from tbl_tfloati, tbl_float where ti &> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloati', 'tintinst', count(*) from tbl_tfloati, tbl_tintinst where ti &> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloati', 'tfloatinst', count(*) from tbl_tfloati, tbl_tfloatinst where ti &> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloati', 'tinti', count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti &> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloati', 'tfloati', count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti &> t2.ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloati', 'tintseq', count(*) from tbl_tfloati, tbl_tintseq where ti &> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloati', 'tfloatseq', count(*) from tbl_tfloati, tbl_tfloatseq where ti &> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloati', 'tints', count(*) from tbl_tfloati, tbl_tints where ti &> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloati', 'tfloats', count(*) from tbl_tfloati, tbl_tfloats where ti &> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintseq', 'int', count(*) from tbl_tintseq, tbl_int where seq &> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintseq', 'float', count(*) from tbl_tintseq, tbl_float where seq &> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintseq', 'tintinst', count(*) from tbl_tintseq, tbl_tintinst where seq &> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintseq', 'tfloatinst', count(*) from tbl_tintseq, tbl_tfloatinst where seq &> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintseq', 'tinti', count(*) from tbl_tintseq, tbl_tinti where seq &> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintseq', 'tfloati', count(*) from tbl_tintseq, tbl_tfloati where seq &> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintseq', 'tintseq', count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq &> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintseq', 'tfloatseq', count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq &> t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintseq', 'tints', count(*) from tbl_tintseq, tbl_tints where seq &> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tintseq', 'tfloats', count(*) from tbl_tintseq, tbl_tfloats where seq &> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatseq', 'int', count(*) from tbl_tfloatseq, tbl_int where seq &> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatseq', 'float', count(*) from tbl_tfloatseq, tbl_float where seq &> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatseq', 'tintinst', count(*) from tbl_tfloatseq, tbl_tintinst where seq &> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatseq', 'tfloatinst', count(*) from tbl_tfloatseq, tbl_tfloatinst where seq &> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatseq', 'tinti', count(*) from tbl_tfloatseq, tbl_tinti where seq &> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatseq', 'tfloati', count(*) from tbl_tfloatseq, tbl_tfloati where seq &> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatseq', 'tintseq', count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq &> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatseq', 'tfloatseq', count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq &> t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatseq', 'tints', count(*) from tbl_tfloatseq, tbl_tints where seq &> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloatseq', 'tfloats', count(*) from tbl_tfloatseq, tbl_tfloats where seq &> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tints', 'int', count(*) from tbl_tints, tbl_int where ts &> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tints', 'float', count(*) from tbl_tints, tbl_float where ts &> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tints', 'tintinst', count(*) from tbl_tints, tbl_tintinst where ts &> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tints', 'tfloatinst', count(*) from tbl_tints, tbl_tfloatinst where ts &> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tints', 'tinti', count(*) from tbl_tints, tbl_tinti where ts &> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tints', 'tfloati', count(*) from tbl_tints, tbl_tfloati where ts &> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tints', 'tintseq', count(*) from tbl_tints, tbl_tintseq where ts &> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tints', 'tfloatseq', count(*) from tbl_tints, tbl_tfloatseq where ts &> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tints', 'tints', count(*) from tbl_tints t1, tbl_tints t2 where t1.ts &> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tints', 'tfloats', count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts &> t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloats', 'int', count(*) from tbl_tfloats, tbl_int where ts &> i;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloats', 'float', count(*) from tbl_tfloats, tbl_float where ts &> f;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloats', 'tintinst', count(*) from tbl_tfloats, tbl_tintinst where ts &> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloats', 'tfloatinst', count(*) from tbl_tfloats, tbl_tfloatinst where ts &> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloats', 'tinti', count(*) from tbl_tfloats, tbl_tinti where ts &> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloats', 'tfloati', count(*) from tbl_tfloats, tbl_tfloati where ts &> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloats', 'tintseq', count(*) from tbl_tfloats, tbl_tintseq where ts &> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloats', 'tfloatseq', count(*) from tbl_tfloats, tbl_tfloatseq where ts &> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloats', 'tints', count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts &> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&>', 'tfloats', 'tfloats', count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts &> t2.ts;

-------------------------------------------------------------------------------
-- Before
-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tboolinst', count(*) from tbl_timestamptz, tbl_tboolinst where t <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tintinst', count(*) from tbl_timestamptz, tbl_tintinst where t <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tfloatinst', count(*) from tbl_timestamptz, tbl_tfloatinst where t <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'ttextinst', count(*) from tbl_timestamptz, tbl_ttextinst where t <<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tbooli', count(*) from tbl_timestamptz, tbl_tbooli where t <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tinti', count(*) from tbl_timestamptz, tbl_tinti where t <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tfloati', count(*) from tbl_timestamptz, tbl_tfloati where t <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'ttexti', count(*) from tbl_timestamptz, tbl_ttexti where t <<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tboolseq', count(*) from tbl_timestamptz, tbl_tboolseq where t <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tintseq', count(*) from tbl_timestamptz, tbl_tintseq where t <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tfloatseq', count(*) from tbl_timestamptz, tbl_tfloatseq where t <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'ttextseq', count(*) from tbl_timestamptz, tbl_ttextseq where t <<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tbools', count(*) from tbl_timestamptz, tbl_tbools where t <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tints', count(*) from tbl_timestamptz, tbl_tints where t <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'tfloats', count(*) from tbl_timestamptz, tbl_tfloats where t <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestamptz', 'ttexts', count(*) from tbl_timestamptz, tbl_ttexts where t <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tboolinst', count(*) from tbl_timestampset, tbl_tboolinst where ts <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tintinst', count(*) from tbl_timestampset, tbl_tintinst where ts <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tfloatinst', count(*) from tbl_timestampset, tbl_tfloatinst where ts <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'ttextinst', count(*) from tbl_timestampset, tbl_ttextinst where ts <<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tbooli', count(*) from tbl_timestampset, tbl_tbooli where ts <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tinti', count(*) from tbl_timestampset, tbl_tinti where ts <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tfloati', count(*) from tbl_timestampset, tbl_tfloati where ts <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'ttexti', count(*) from tbl_timestampset, tbl_ttexti where ts <<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tboolseq', count(*) from tbl_timestampset, tbl_tboolseq where ts <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tintseq', count(*) from tbl_timestampset, tbl_tintseq where ts <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tfloatseq', count(*) from tbl_timestampset, tbl_tfloatseq where ts <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'ttextseq', count(*) from tbl_timestampset, tbl_ttextseq where ts <<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tbools', count(*) from tbl_timestampset t1, tbl_tbools t2 where t1.ts <<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tints', count(*) from tbl_timestampset t1, tbl_tints t2 where t1.ts <<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'tfloats', count(*) from tbl_timestampset t1, tbl_tfloats t2 where t1.ts <<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'timestampset', 'ttexts', count(*) from tbl_timestampset t1, tbl_ttexts t2 where t1.ts <<# t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tboolinst', count(*) from tbl_period, tbl_tboolinst where p <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tintinst', count(*) from tbl_period, tbl_tintinst where p <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tfloatinst', count(*) from tbl_period, tbl_tfloatinst where p <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'ttextinst', count(*) from tbl_period, tbl_ttextinst where p <<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tbooli', count(*) from tbl_period, tbl_tbooli where p <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tinti', count(*) from tbl_period, tbl_tinti where p <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tfloati', count(*) from tbl_period, tbl_tfloati where p <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'ttexti', count(*) from tbl_period, tbl_ttexti where p <<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tboolseq', count(*) from tbl_period, tbl_tboolseq where p <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tintseq', count(*) from tbl_period, tbl_tintseq where p <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tfloatseq', count(*) from tbl_period, tbl_tfloatseq where p <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'ttextseq', count(*) from tbl_period, tbl_ttextseq where p <<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tbools', count(*) from tbl_period, tbl_tbools where p <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tints', count(*) from tbl_period, tbl_tints where p <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'tfloats', count(*) from tbl_period, tbl_tfloats where p <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'period', 'ttexts', count(*) from tbl_period, tbl_ttexts where p <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tboolinst', count(*) from tbl_periodset, tbl_tboolinst where ps <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tintinst', count(*) from tbl_periodset, tbl_tintinst where ps <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tfloatinst', count(*) from tbl_periodset, tbl_tfloatinst where ps <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'ttextinst', count(*) from tbl_periodset, tbl_ttextinst where ps <<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tbooli', count(*) from tbl_periodset, tbl_tbooli where ps <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tinti', count(*) from tbl_periodset, tbl_tinti where ps <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tfloati', count(*) from tbl_periodset, tbl_tfloati where ps <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'ttexti', count(*) from tbl_periodset, tbl_ttexti where ps <<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tboolseq', count(*) from tbl_periodset, tbl_tboolseq where ps <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tintseq', count(*) from tbl_periodset, tbl_tintseq where ps <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tfloatseq', count(*) from tbl_periodset, tbl_tfloatseq where ps <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'ttextseq', count(*) from tbl_periodset, tbl_ttextseq where ps <<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tbools', count(*) from tbl_periodset, tbl_tbools where ps <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tints', count(*) from tbl_periodset, tbl_tints where ps <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'tfloats', count(*) from tbl_periodset, tbl_tfloats where ps <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'periodset', 'ttexts', count(*) from tbl_periodset, tbl_ttexts where ps <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolinst', 'timestamptz', count(*) from tbl_tboolinst, tbl_timestamptz where inst <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolinst', 'timestampset', count(*) from tbl_tboolinst, tbl_timestampset where inst <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolinst', 'period', count(*) from tbl_tboolinst, tbl_period where inst <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolinst', 'periodset', count(*) from tbl_tboolinst, tbl_periodset where inst <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolinst', 'tboolinst', count(*) from tbl_tboolinst t1, tbl_tboolinst t2 where t1.inst <<# t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolinst', 'tbooli', count(*) from tbl_tboolinst, tbl_tbooli where inst <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolinst', 'tboolseq', count(*) from tbl_tboolinst, tbl_tboolseq where inst <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolinst', 'tbools', count(*) from tbl_tboolinst, tbl_tbools where inst <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'timestamptz', count(*) from tbl_tintinst, tbl_timestamptz where inst <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'timestampset', count(*) from tbl_tintinst, tbl_timestampset where inst <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'period', count(*) from tbl_tintinst, tbl_period where inst <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'periodset', count(*) from tbl_tintinst, tbl_periodset where inst <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'tintinst', count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst <<# t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'tfloatinst', count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst <<# t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'tinti', count(*) from tbl_tintinst, tbl_tinti where inst <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'tfloati', count(*) from tbl_tintinst, tbl_tfloati where inst <<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'tintseq', count(*) from tbl_tintinst, tbl_tintseq where inst <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'tfloatseq', count(*) from tbl_tintinst, tbl_tfloatseq where inst <<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'tints', count(*) from tbl_tintinst, tbl_tints where inst <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintinst', 'tfloats', count(*) from tbl_tintinst, tbl_tfloats where inst <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'timestamptz', count(*) from tbl_tfloatinst, tbl_timestamptz where inst <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'timestampset', count(*) from tbl_tfloatinst, tbl_timestampset where inst <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'period', count(*) from tbl_tfloatinst, tbl_period where inst <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'periodset', count(*) from tbl_tfloatinst, tbl_periodset where inst <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'tintinst', count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst <<# t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'tfloatinst', count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst <<# t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'tinti', count(*) from tbl_tfloatinst, tbl_tinti where inst <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'tfloati', count(*) from tbl_tfloatinst, tbl_tfloati where inst <<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'tintseq', count(*) from tbl_tfloatinst, tbl_tintseq where inst <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'tfloatseq', count(*) from tbl_tfloatinst, tbl_tfloatseq where inst <<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'tints', count(*) from tbl_tfloatinst, tbl_tints where inst <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatinst', 'tfloats', count(*) from tbl_tfloatinst, tbl_tfloats where inst <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextinst', 'timestamptz', count(*) from tbl_ttextinst, tbl_timestamptz where inst <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextinst', 'timestampset', count(*) from tbl_ttextinst, tbl_timestampset where inst <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextinst', 'period', count(*) from tbl_ttextinst, tbl_period where inst <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextinst', 'periodset', count(*) from tbl_ttextinst, tbl_periodset where inst <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextinst', 'ttextinst', count(*) from tbl_ttextinst t1, tbl_ttextinst t2 where t1.inst <<# t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextinst', 'ttexti', count(*) from tbl_ttextinst, tbl_ttexti where inst <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextinst', 'ttextseq', count(*) from tbl_ttextinst, tbl_ttextseq where inst <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextinst', 'ttexts', count(*) from tbl_ttextinst, tbl_ttexts where inst <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbooli', 'timestamptz', count(*) from tbl_tbooli, tbl_timestamptz where ti <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbooli', 'timestampset', count(*) from tbl_tbooli, tbl_timestampset where ti <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbooli', 'period', count(*) from tbl_tbooli, tbl_period where ti <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbooli', 'periodset', count(*) from tbl_tbooli, tbl_periodset where ti <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbooli', 'tboolinst', count(*) from tbl_tbooli, tbl_tboolinst where ti <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbooli', 'tbooli', count(*) from tbl_tbooli t1, tbl_tbooli t2 where t1.ti <<# t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbooli', 'tboolseq', count(*) from tbl_tbooli, tbl_tboolseq where ti <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbooli', 'tbools', count(*) from tbl_tbooli, tbl_tbools where ti <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'timestamptz', count(*) from tbl_tinti, tbl_timestamptz where ti <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'timestampset', count(*) from tbl_tinti, tbl_timestampset where ti <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'period', count(*) from tbl_tinti, tbl_period where ti <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'periodset', count(*) from tbl_tinti, tbl_periodset where ti <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'tintinst', count(*) from tbl_tinti, tbl_tintinst where ti <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'tfloatinst', count(*) from tbl_tinti, tbl_tfloatinst where ti <<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'tinti', count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti <<# t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'tfloati', count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti <<# t2.ti;
	
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'tintseq', count(*) from tbl_tinti, tbl_tintseq where ti <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'tfloatseq', count(*) from tbl_tinti, tbl_tfloatseq where ti <<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'tints', count(*) from tbl_tinti, tbl_tints where ti <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tinti', 'tfloats', count(*) from tbl_tinti, tbl_tfloats where ti <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'timestamptz', count(*) from tbl_tfloati, tbl_timestamptz where ti <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'timestampset', count(*) from tbl_tfloati, tbl_timestampset where ti <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'period', count(*) from tbl_tfloati, tbl_period where ti <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'periodset', count(*) from tbl_tfloati, tbl_periodset where ti <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'tintinst', count(*) from tbl_tfloati, tbl_tintinst where ti <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'tfloatinst', count(*) from tbl_tfloati, tbl_tfloatinst where ti <<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'tinti', count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti <<# t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'tfloati', count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti <<# t2.ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'tintseq', count(*) from tbl_tfloati, tbl_tintseq where ti <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'tfloatseq', count(*) from tbl_tfloati, tbl_tfloatseq where ti <<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'tints', count(*) from tbl_tfloati, tbl_tints where ti <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloati', 'tfloats', count(*) from tbl_tfloati, tbl_tfloats where ti <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexti', 'timestamptz', count(*) from tbl_ttexti, tbl_timestamptz where ti <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexti', 'timestampset', count(*) from tbl_ttexti, tbl_timestampset where ti <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexti', 'period', count(*) from tbl_ttexti, tbl_period where ti <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexti', 'periodset', count(*) from tbl_ttexti, tbl_periodset where ti <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexti', 'ttextinst', count(*) from tbl_ttexti t1, tbl_ttextinst where ti <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexti', 'ttexti', count(*) from tbl_ttexti t1, tbl_ttexti t2 where t1.ti <<# t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexti', 'ttextseq', count(*) from tbl_ttexti, tbl_ttextseq where ti <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexti', 'ttexts', count(*) from tbl_ttexti, tbl_ttexts where ti <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolseq', 'timestamptz', count(*) from tbl_tboolseq, tbl_timestamptz where seq <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolseq', 'timestampset', count(*) from tbl_tboolseq, tbl_timestampset where seq <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolseq', 'period', count(*) from tbl_tboolseq, tbl_period where seq <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolseq', 'periodset', count(*) from tbl_tboolseq, tbl_periodset where seq <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolseq', 'tboolinst', count(*) from tbl_tboolseq, tbl_tboolinst where seq <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolseq', 'tbooli', count(*) from tbl_tboolseq, tbl_tbooli where seq <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolseq', 'tboolseq', count(*) from tbl_tboolseq t1, tbl_tboolseq t2 where t1.seq <<# t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tboolseq', 'tbools', count(*) from tbl_tboolseq, tbl_tbools where seq <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'timestamptz', count(*) from tbl_tintseq, tbl_timestamptz where seq <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'timestampset', count(*) from tbl_tintseq, tbl_timestampset where seq <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'period', count(*) from tbl_tintseq, tbl_period where seq <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'periodset', count(*) from tbl_tintseq, tbl_periodset where seq <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'tintinst', count(*) from tbl_tintseq, tbl_tintinst where seq <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'tfloatinst', count(*) from tbl_tintseq, tbl_tfloatinst where seq <<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'tinti', count(*) from tbl_tintseq, tbl_tinti where seq <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'tfloati', count(*) from tbl_tintseq, tbl_tfloati where seq <<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'tintseq', count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq <<# t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'tfloatseq', count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq <<# t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'tints', count(*) from tbl_tintseq, tbl_tints where seq <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tintseq', 'tfloats', count(*) from tbl_tintseq, tbl_tfloats where seq <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'timestamptz', count(*) from tbl_tfloatseq, tbl_timestamptz where seq <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'timestampset', count(*) from tbl_tfloatseq, tbl_timestampset where seq <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'period', count(*) from tbl_tfloatseq, tbl_period where seq <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'periodset', count(*) from tbl_tfloatseq, tbl_periodset where seq <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'tintinst', count(*) from tbl_tfloatseq, tbl_tintinst where seq <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'tfloatinst', count(*) from tbl_tfloatseq, tbl_tfloatinst where seq <<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'tinti', count(*) from tbl_tfloatseq, tbl_tinti where seq <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'tfloati', count(*) from tbl_tfloatseq, tbl_tfloati where seq <<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'tintseq', count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq <<# t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'tfloatseq', count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq <<# t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'tints', count(*) from tbl_tfloatseq, tbl_tints where seq <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloatseq', 'tfloats', count(*) from tbl_tfloatseq, tbl_tfloats where seq <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextseq', 'timestamptz', count(*) from tbl_ttextseq, tbl_timestamptz where seq <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextseq', 'timestampset', count(*) from tbl_ttextseq, tbl_timestampset where seq <<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextseq', 'period', count(*) from tbl_ttextseq, tbl_period where seq <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextseq', 'periodset', count(*) from tbl_ttextseq, tbl_periodset where seq <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextseq', 'ttextinst', count(*) from tbl_ttextseq, tbl_ttextinst where seq <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextseq', 'ttexti', count(*) from tbl_ttextseq, tbl_ttexti where seq <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextseq', 'ttextseq', count(*) from tbl_ttextseq t1, tbl_ttextseq t2 where t1.seq <<# t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttextseq', 'ttexts', count(*) from tbl_ttextseq, tbl_ttexts where seq <<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbools', 'timestamptz', count(*) from tbl_tbools, tbl_timestamptz where ts <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbools', 'timestampset', count(*) from tbl_tbools t1, tbl_timestampset t2 where t1.ts <<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbools', 'period', count(*) from tbl_tbools, tbl_period where ts <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbools', 'periodset', count(*) from tbl_tbools, tbl_periodset where ts <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbools', 'tboolinst', count(*) from tbl_tbools, tbl_tboolinst where ts <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbools', 'tbooli', count(*) from tbl_tbools, tbl_tbooli where ts <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbools', 'tboolseq', count(*) from tbl_tbools, tbl_tboolseq where ts <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tbools', 'tbools', count(*) from tbl_tbools t1, tbl_tbools t2 where t1.ts <<# t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'timestamptz', count(*) from tbl_tints, tbl_timestamptz where ts <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'timestampset', count(*) from tbl_tints t1, tbl_timestampset t2 where t1.ts <<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'period', count(*) from tbl_tints, tbl_period where ts <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'periodset', count(*) from tbl_tints, tbl_periodset where ts <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'tintinst', count(*) from tbl_tints, tbl_tintinst where ts <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'tfloatinst', count(*) from tbl_tints, tbl_tfloatinst where ts <<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'tinti', count(*) from tbl_tints, tbl_tinti where ts <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'tfloati', count(*) from tbl_tints, tbl_tfloati where ts <<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'tintseq', count(*) from tbl_tints, tbl_tintseq where ts <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'tfloatseq', count(*) from tbl_tints, tbl_tfloatseq where ts <<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'tints', count(*) from tbl_tints t1, tbl_tints t2 where t1.ts <<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tints', 'tfloats', count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts <<# t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'timestamptz', count(*) from tbl_tfloats, tbl_timestamptz where ts <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'timestampset', count(*) from tbl_tfloats t1, tbl_timestampset t2 where t1.ts <<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'period', count(*) from tbl_tfloats, tbl_period where ts <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'periodset', count(*) from tbl_tfloats, tbl_periodset where ts <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'tintinst', count(*) from tbl_tfloats, tbl_tintinst where ts <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'tfloatinst', count(*) from tbl_tfloats, tbl_tfloatinst where ts <<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'tinti', count(*) from tbl_tfloats, tbl_tinti where ts <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'tfloati', count(*) from tbl_tfloats, tbl_tfloati where ts <<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'tintseq', count(*) from tbl_tfloats, tbl_tintseq where ts <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'tfloatseq', count(*) from tbl_tfloats, tbl_tfloatseq where ts <<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'tints', count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts <<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'tfloats', 'tfloats', count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts <<# t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexts', 'timestamptz', count(*) from tbl_ttexts, tbl_timestamptz where ts <<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexts', 'timestampset', count(*) from tbl_ttexts t1, tbl_timestampset t2 where t1.ts <<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexts', 'period', count(*) from tbl_ttexts, tbl_period where ts <<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexts', 'periodset', count(*) from tbl_ttexts, tbl_periodset where ts <<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexts', 'ttextinst', count(*) from tbl_ttexts, tbl_ttextinst where ts <<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexts', 'ttexti', count(*) from tbl_ttexts, tbl_ttexti where ts <<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexts', 'ttextseq', count(*) from tbl_ttexts, tbl_ttextseq where ts <<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '<<#', 'ttexts', 'ttexts', count(*) from tbl_ttexts t1, tbl_ttexts t2 where t1.ts <<# t2.ts;

-------------------------------------------------------------------------------
-- Overbefore
-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tboolinst', count(*) from tbl_timestamptz, tbl_tboolinst where t &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tintinst', count(*) from tbl_timestamptz, tbl_tintinst where t &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tfloatinst', count(*) from tbl_timestamptz, tbl_tfloatinst where t &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'ttextinst', count(*) from tbl_timestamptz, tbl_ttextinst where t &<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tbooli', count(*) from tbl_timestamptz, tbl_tbooli where t &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tinti', count(*) from tbl_timestamptz, tbl_tinti where t &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tfloati', count(*) from tbl_timestamptz, tbl_tfloati where t &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'ttexti', count(*) from tbl_timestamptz, tbl_ttexti where t &<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tboolseq', count(*) from tbl_timestamptz, tbl_tboolseq where t &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tintseq', count(*) from tbl_timestamptz, tbl_tintseq where t &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tfloatseq', count(*) from tbl_timestamptz, tbl_tfloatseq where t &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'ttextseq', count(*) from tbl_timestamptz, tbl_ttextseq where t &<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tbools', count(*) from tbl_timestamptz, tbl_tbools where t &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tints', count(*) from tbl_timestamptz, tbl_tints where t &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'tfloats', count(*) from tbl_timestamptz, tbl_tfloats where t &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestamptz', 'ttexts', count(*) from tbl_timestamptz, tbl_ttexts where t &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tboolinst', count(*) from tbl_timestampset, tbl_tboolinst where ts &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tintinst', count(*) from tbl_timestampset, tbl_tintinst where ts &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tfloatinst', count(*) from tbl_timestampset, tbl_tfloatinst where ts &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'ttextinst', count(*) from tbl_timestampset, tbl_ttextinst where ts &<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tbooli', count(*) from tbl_timestampset, tbl_tbooli where ts &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tinti', count(*) from tbl_timestampset, tbl_tinti where ts &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tfloati', count(*) from tbl_timestampset, tbl_tfloati where ts &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'ttexti', count(*) from tbl_timestampset, tbl_ttexti where ts &<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tboolseq', count(*) from tbl_timestampset, tbl_tboolseq where ts &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tintseq', count(*) from tbl_timestampset, tbl_tintseq where ts &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tfloatseq', count(*) from tbl_timestampset, tbl_tfloatseq where ts &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'ttextseq', count(*) from tbl_timestampset, tbl_ttextseq where ts &<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tbools', count(*) from tbl_timestampset t1, tbl_tbools t2 where t1.ts &<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tints', count(*) from tbl_timestampset t1, tbl_tints t2 where t1.ts &<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'tfloats', count(*) from tbl_timestampset t1, tbl_tfloats t2 where t1.ts &<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'timestampset', 'ttexts', count(*) from tbl_timestampset t1, tbl_ttexts t2 where t1.ts &<# t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tboolinst', count(*) from tbl_period, tbl_tboolinst where p &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tintinst', count(*) from tbl_period, tbl_tintinst where p &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tfloatinst', count(*) from tbl_period, tbl_tfloatinst where p &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'ttextinst', count(*) from tbl_period, tbl_ttextinst where p &<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tbooli', count(*) from tbl_period, tbl_tbooli where p &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tinti', count(*) from tbl_period, tbl_tinti where p &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tfloati', count(*) from tbl_period, tbl_tfloati where p &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'ttexti', count(*) from tbl_period, tbl_ttexti where p &<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tboolseq', count(*) from tbl_period, tbl_tboolseq where p &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tintseq', count(*) from tbl_period, tbl_tintseq where p &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tfloatseq', count(*) from tbl_period, tbl_tfloatseq where p &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'ttextseq', count(*) from tbl_period, tbl_ttextseq where p &<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tbools', count(*) from tbl_period, tbl_tbools where p &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tints', count(*) from tbl_period, tbl_tints where p &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'tfloats', count(*) from tbl_period, tbl_tfloats where p &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'period', 'ttexts', count(*) from tbl_period, tbl_ttexts where p &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tboolinst', count(*) from tbl_periodset, tbl_tboolinst where ps &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tintinst', count(*) from tbl_periodset, tbl_tintinst where ps &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tfloatinst', count(*) from tbl_periodset, tbl_tfloatinst where ps &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'ttextinst', count(*) from tbl_periodset, tbl_ttextinst where ps &<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tbooli', count(*) from tbl_periodset, tbl_tbooli where ps &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tinti', count(*) from tbl_periodset, tbl_tinti where ps &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tfloati', count(*) from tbl_periodset, tbl_tfloati where ps &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'ttexti', count(*) from tbl_periodset, tbl_ttexti where ps &<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tboolseq', count(*) from tbl_periodset, tbl_tboolseq where ps &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tintseq', count(*) from tbl_periodset, tbl_tintseq where ps &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tfloatseq', count(*) from tbl_periodset, tbl_tfloatseq where ps &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'ttextseq', count(*) from tbl_periodset, tbl_ttextseq where ps &<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tbools', count(*) from tbl_periodset, tbl_tbools where ps &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tints', count(*) from tbl_periodset, tbl_tints where ps &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'tfloats', count(*) from tbl_periodset, tbl_tfloats where ps &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'periodset', 'ttexts', count(*) from tbl_periodset, tbl_ttexts where ps &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolinst', 'timestamptz', count(*) from tbl_tboolinst, tbl_timestamptz where inst &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolinst', 'timestampset', count(*) from tbl_tboolinst, tbl_timestampset where inst &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolinst', 'period', count(*) from tbl_tboolinst, tbl_period where inst &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolinst', 'periodset', count(*) from tbl_tboolinst, tbl_periodset where inst &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolinst', 'tboolinst', count(*) from tbl_tboolinst t1, tbl_tboolinst t2 where t1.inst &<# t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolinst', 'tbooli', count(*) from tbl_tboolinst, tbl_tbooli where inst &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolinst', 'tboolseq', count(*) from tbl_tboolinst, tbl_tboolseq where inst &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolinst', 'tbools', count(*) from tbl_tboolinst, tbl_tbools where inst &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'timestamptz', count(*) from tbl_tintinst, tbl_timestamptz where inst &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'timestampset', count(*) from tbl_tintinst, tbl_timestampset where inst &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'period', count(*) from tbl_tintinst, tbl_period where inst &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'periodset', count(*) from tbl_tintinst, tbl_periodset where inst &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'tintinst', count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst &<# t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'tfloatinst', count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst &<# t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'tinti', count(*) from tbl_tintinst, tbl_tinti where inst &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'tfloati', count(*) from tbl_tintinst, tbl_tfloati where inst &<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'tintseq', count(*) from tbl_tintinst, tbl_tintseq where inst &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'tfloatseq', count(*) from tbl_tintinst, tbl_tfloatseq where inst &<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'tints', count(*) from tbl_tintinst, tbl_tints where inst &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintinst', 'tfloats', count(*) from tbl_tintinst, tbl_tfloats where inst &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'timestamptz', count(*) from tbl_tfloatinst, tbl_timestamptz where inst &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'timestampset', count(*) from tbl_tfloatinst, tbl_timestampset where inst &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'period', count(*) from tbl_tfloatinst, tbl_period where inst &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'periodset', count(*) from tbl_tfloatinst, tbl_periodset where inst &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'tintinst', count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst &<# t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'tfloatinst', count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst &<# t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'tinti', count(*) from tbl_tfloatinst, tbl_tinti where inst &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'tfloati', count(*) from tbl_tfloatinst, tbl_tfloati where inst &<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'tintseq', count(*) from tbl_tfloatinst, tbl_tintseq where inst &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'tfloatseq', count(*) from tbl_tfloatinst, tbl_tfloatseq where inst &<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'tints', count(*) from tbl_tfloatinst, tbl_tints where inst &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatinst', 'tfloats', count(*) from tbl_tfloatinst, tbl_tfloats where inst &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextinst', 'timestamptz', count(*) from tbl_ttextinst, tbl_timestamptz where inst &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextinst', 'timestampset', count(*) from tbl_ttextinst, tbl_timestampset where inst &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextinst', 'period', count(*) from tbl_ttextinst, tbl_period where inst &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextinst', 'periodset', count(*) from tbl_ttextinst, tbl_periodset where inst &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextinst', 'ttextinst', count(*) from tbl_ttextinst t1, tbl_ttextinst t2 where t1.inst &<# t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextinst', 'ttexti', count(*) from tbl_ttextinst, tbl_ttexti where inst &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextinst', 'ttextseq', count(*) from tbl_ttextinst, tbl_ttextseq where inst &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextinst', 'ttexts', count(*) from tbl_ttextinst, tbl_ttexts where inst &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbooli', 'timestamptz', count(*) from tbl_tbooli, tbl_timestamptz where ti &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbooli', 'timestampset', count(*) from tbl_tbooli, tbl_timestampset where ti &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbooli', 'period', count(*) from tbl_tbooli, tbl_period where ti &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbooli', 'periodset', count(*) from tbl_tbooli, tbl_periodset where ti &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbooli', 'tboolinst', count(*) from tbl_tbooli, tbl_tboolinst where ti &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbooli', 'tbooli', count(*) from tbl_tbooli t1, tbl_tbooli t2 where t1.ti &<# t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbooli', 'tboolseq', count(*) from tbl_tbooli, tbl_tboolseq where ti &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbooli', 'tbools', count(*) from tbl_tbooli, tbl_tbools where ti &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'timestamptz', count(*) from tbl_tinti, tbl_timestamptz where ti &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'timestampset', count(*) from tbl_tinti, tbl_timestampset where ti &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'period', count(*) from tbl_tinti, tbl_period where ti &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'periodset', count(*) from tbl_tinti, tbl_periodset where ti &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'tintinst', count(*) from tbl_tinti, tbl_tintinst where ti &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'tfloatinst', count(*) from tbl_tinti, tbl_tfloatinst where ti &<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'tinti', count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti &<# t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'tfloati', count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti &<# t2.ti;
	
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'tintseq', count(*) from tbl_tinti, tbl_tintseq where ti &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'tfloatseq', count(*) from tbl_tinti, tbl_tfloatseq where ti &<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'tints', count(*) from tbl_tinti, tbl_tints where ti &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tinti', 'tfloats', count(*) from tbl_tinti, tbl_tfloats where ti &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'timestamptz', count(*) from tbl_tfloati, tbl_timestamptz where ti &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'timestampset', count(*) from tbl_tfloati, tbl_timestampset where ti &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'period', count(*) from tbl_tfloati, tbl_period where ti &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'periodset', count(*) from tbl_tfloati, tbl_periodset where ti &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'tintinst', count(*) from tbl_tfloati, tbl_tintinst where ti &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'tfloatinst', count(*) from tbl_tfloati, tbl_tfloatinst where ti &<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'tinti', count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti &<# t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'tfloati', count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti &<# t2.ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'tintseq', count(*) from tbl_tfloati, tbl_tintseq where ti &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'tfloatseq', count(*) from tbl_tfloati, tbl_tfloatseq where ti &<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'tints', count(*) from tbl_tfloati, tbl_tints where ti &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloati', 'tfloats', count(*) from tbl_tfloati, tbl_tfloats where ti &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexti', 'timestamptz', count(*) from tbl_ttexti, tbl_timestamptz where ti &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexti', 'timestampset', count(*) from tbl_ttexti, tbl_timestampset where ti &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexti', 'period', count(*) from tbl_ttexti, tbl_period where ti &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexti', 'periodset', count(*) from tbl_ttexti, tbl_periodset where ti &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexti', 'ttextinst', count(*) from tbl_ttexti t1, tbl_ttextinst where ti &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexti', 'ttexti', count(*) from tbl_ttexti t1, tbl_ttexti t2 where t1.ti &<# t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexti', 'ttextseq', count(*) from tbl_ttexti, tbl_ttextseq where ti &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexti', 'ttexts', count(*) from tbl_ttexti, tbl_ttexts where ti &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolseq', 'timestamptz', count(*) from tbl_tboolseq, tbl_timestamptz where seq &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolseq', 'timestampset', count(*) from tbl_tboolseq, tbl_timestampset where seq &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolseq', 'period', count(*) from tbl_tboolseq, tbl_period where seq &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolseq', 'periodset', count(*) from tbl_tboolseq, tbl_periodset where seq &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolseq', 'tboolinst', count(*) from tbl_tboolseq, tbl_tboolinst where seq &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolseq', 'tbooli', count(*) from tbl_tboolseq, tbl_tbooli where seq &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolseq', 'tboolseq', count(*) from tbl_tboolseq t1, tbl_tboolseq t2 where t1.seq &<# t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tboolseq', 'tbools', count(*) from tbl_tboolseq, tbl_tbools where seq &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'timestamptz', count(*) from tbl_tintseq, tbl_timestamptz where seq &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'timestampset', count(*) from tbl_tintseq, tbl_timestampset where seq &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'period', count(*) from tbl_tintseq, tbl_period where seq &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'periodset', count(*) from tbl_tintseq, tbl_periodset where seq &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'tintinst', count(*) from tbl_tintseq, tbl_tintinst where seq &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'tfloatinst', count(*) from tbl_tintseq, tbl_tfloatinst where seq &<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'tinti', count(*) from tbl_tintseq, tbl_tinti where seq &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'tfloati', count(*) from tbl_tintseq, tbl_tfloati where seq &<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'tintseq', count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq &<# t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'tfloatseq', count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq &<# t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'tints', count(*) from tbl_tintseq, tbl_tints where seq &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tintseq', 'tfloats', count(*) from tbl_tintseq, tbl_tfloats where seq &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'timestamptz', count(*) from tbl_tfloatseq, tbl_timestamptz where seq &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'timestampset', count(*) from tbl_tfloatseq, tbl_timestampset where seq &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'period', count(*) from tbl_tfloatseq, tbl_period where seq &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'periodset', count(*) from tbl_tfloatseq, tbl_periodset where seq &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'tintinst', count(*) from tbl_tfloatseq, tbl_tintinst where seq &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'tfloatinst', count(*) from tbl_tfloatseq, tbl_tfloatinst where seq &<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'tinti', count(*) from tbl_tfloatseq, tbl_tinti where seq &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'tfloati', count(*) from tbl_tfloatseq, tbl_tfloati where seq &<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'tintseq', count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq &<# t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'tfloatseq', count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq &<# t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'tints', count(*) from tbl_tfloatseq, tbl_tints where seq &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloatseq', 'tfloats', count(*) from tbl_tfloatseq, tbl_tfloats where seq &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextseq', 'timestamptz', count(*) from tbl_ttextseq, tbl_timestamptz where seq &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextseq', 'timestampset', count(*) from tbl_ttextseq, tbl_timestampset where seq &<# ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextseq', 'period', count(*) from tbl_ttextseq, tbl_period where seq &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextseq', 'periodset', count(*) from tbl_ttextseq, tbl_periodset where seq &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextseq', 'ttextinst', count(*) from tbl_ttextseq, tbl_ttextinst where seq &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextseq', 'ttexti', count(*) from tbl_ttextseq, tbl_ttexti where seq &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextseq', 'ttextseq', count(*) from tbl_ttextseq t1, tbl_ttextseq t2 where t1.seq &<# t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttextseq', 'ttexts', count(*) from tbl_ttextseq, tbl_ttexts where seq &<# ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbools', 'timestamptz', count(*) from tbl_tbools, tbl_timestamptz where ts &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbools', 'timestampset', count(*) from tbl_tbools t1, tbl_timestampset t2 where t1.ts &<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbools', 'period', count(*) from tbl_tbools, tbl_period where ts &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbools', 'periodset', count(*) from tbl_tbools, tbl_periodset where ts &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbools', 'tboolinst', count(*) from tbl_tbools, tbl_tboolinst where ts &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbools', 'tbooli', count(*) from tbl_tbools, tbl_tbooli where ts &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbools', 'tboolseq', count(*) from tbl_tbools, tbl_tboolseq where ts &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tbools', 'tbools', count(*) from tbl_tbools t1, tbl_tbools t2 where t1.ts &<# t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'timestamptz', count(*) from tbl_tints, tbl_timestamptz where ts &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'timestampset', count(*) from tbl_tints t1, tbl_timestampset t2 where t1.ts &<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'period', count(*) from tbl_tints, tbl_period where ts &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'periodset', count(*) from tbl_tints, tbl_periodset where ts &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'tintinst', count(*) from tbl_tints, tbl_tintinst where ts &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'tfloatinst', count(*) from tbl_tints, tbl_tfloatinst where ts &<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'tinti', count(*) from tbl_tints, tbl_tinti where ts &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'tfloati', count(*) from tbl_tints, tbl_tfloati where ts &<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'tintseq', count(*) from tbl_tints, tbl_tintseq where ts &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'tfloatseq', count(*) from tbl_tints, tbl_tfloatseq where ts &<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'tints', count(*) from tbl_tints t1, tbl_tints t2 where t1.ts &<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tints', 'tfloats', count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts &<# t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'timestamptz', count(*) from tbl_tfloats, tbl_timestamptz where ts &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'timestampset', count(*) from tbl_tfloats t1, tbl_timestampset t2 where t1.ts &<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'period', count(*) from tbl_tfloats, tbl_period where ts &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'periodset', count(*) from tbl_tfloats, tbl_periodset where ts &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'tintinst', count(*) from tbl_tfloats, tbl_tintinst where ts &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'tfloatinst', count(*) from tbl_tfloats, tbl_tfloatinst where ts &<# inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'tinti', count(*) from tbl_tfloats, tbl_tinti where ts &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'tfloati', count(*) from tbl_tfloats, tbl_tfloati where ts &<# ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'tintseq', count(*) from tbl_tfloats, tbl_tintseq where ts &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'tfloatseq', count(*) from tbl_tfloats, tbl_tfloatseq where ts &<# seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'tints', count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts &<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'tfloats', 'tfloats', count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts &<# t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexts', 'timestamptz', count(*) from tbl_ttexts, tbl_timestamptz where ts &<# t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexts', 'timestampset', count(*) from tbl_ttexts t1, tbl_timestampset t2 where t1.ts &<# t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexts', 'period', count(*) from tbl_ttexts, tbl_period where ts &<# p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexts', 'periodset', count(*) from tbl_ttexts, tbl_periodset where ts &<# ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexts', 'ttextinst', count(*) from tbl_ttexts, tbl_ttextinst where ts &<# inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexts', 'ttexti', count(*) from tbl_ttexts, tbl_ttexti where ts &<# ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexts', 'ttextseq', count(*) from tbl_ttexts, tbl_ttextseq where ts &<# seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '&<#', 'ttexts', 'ttexts', count(*) from tbl_ttexts t1, tbl_ttexts t2 where t1.ts &<# t2.ts;

-------------------------------------------------------------------------------
-- After
-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tboolinst', count(*) from tbl_timestamptz, tbl_tboolinst where t #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tintinst', count(*) from tbl_timestamptz, tbl_tintinst where t #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tfloatinst', count(*) from tbl_timestamptz, tbl_tfloatinst where t #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'ttextinst', count(*) from tbl_timestamptz, tbl_ttextinst where t #>> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tbooli', count(*) from tbl_timestamptz, tbl_tbooli where t #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tinti', count(*) from tbl_timestamptz, tbl_tinti where t #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tfloati', count(*) from tbl_timestamptz, tbl_tfloati where t #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'ttexti', count(*) from tbl_timestamptz, tbl_ttexti where t #>> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tboolseq', count(*) from tbl_timestamptz, tbl_tboolseq where t #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tintseq', count(*) from tbl_timestamptz, tbl_tintseq where t #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tfloatseq', count(*) from tbl_timestamptz, tbl_tfloatseq where t #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'ttextseq', count(*) from tbl_timestamptz, tbl_ttextseq where t #>> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tbools', count(*) from tbl_timestamptz, tbl_tbools where t #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tints', count(*) from tbl_timestamptz, tbl_tints where t #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'tfloats', count(*) from tbl_timestamptz, tbl_tfloats where t #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestamptz', 'ttexts', count(*) from tbl_timestamptz, tbl_ttexts where t #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tboolinst', count(*) from tbl_timestampset, tbl_tboolinst where ts #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tintinst', count(*) from tbl_timestampset, tbl_tintinst where ts #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tfloatinst', count(*) from tbl_timestampset, tbl_tfloatinst where ts #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'ttextinst', count(*) from tbl_timestampset, tbl_ttextinst where ts #>> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tbooli', count(*) from tbl_timestampset, tbl_tbooli where ts #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tinti', count(*) from tbl_timestampset, tbl_tinti where ts #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tfloati', count(*) from tbl_timestampset, tbl_tfloati where ts #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'ttexti', count(*) from tbl_timestampset, tbl_ttexti where ts #>> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tboolseq', count(*) from tbl_timestampset, tbl_tboolseq where ts #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tintseq', count(*) from tbl_timestampset, tbl_tintseq where ts #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tfloatseq', count(*) from tbl_timestampset, tbl_tfloatseq where ts #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'ttextseq', count(*) from tbl_timestampset, tbl_ttextseq where ts #>> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tbools', count(*) from tbl_timestampset t1, tbl_tbools t2 where t1.ts #>> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tints', count(*) from tbl_timestampset t1, tbl_tints t2 where t1.ts #>> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'tfloats', count(*) from tbl_timestampset t1, tbl_tfloats t2 where t1.ts #>> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'timestampset', 'ttexts', count(*) from tbl_timestampset t1, tbl_ttexts t2 where t1.ts #>> t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tboolinst', count(*) from tbl_period, tbl_tboolinst where p #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tintinst', count(*) from tbl_period, tbl_tintinst where p #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tfloatinst', count(*) from tbl_period, tbl_tfloatinst where p #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'ttextinst', count(*) from tbl_period, tbl_ttextinst where p #>> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tbooli', count(*) from tbl_period, tbl_tbooli where p #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tinti', count(*) from tbl_period, tbl_tinti where p #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tfloati', count(*) from tbl_period, tbl_tfloati where p #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'ttexti', count(*) from tbl_period, tbl_ttexti where p #>> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tboolseq', count(*) from tbl_period, tbl_tboolseq where p #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tintseq', count(*) from tbl_period, tbl_tintseq where p #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tfloatseq', count(*) from tbl_period, tbl_tfloatseq where p #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'ttextseq', count(*) from tbl_period, tbl_ttextseq where p #>> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tbools', count(*) from tbl_period, tbl_tbools where p #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tints', count(*) from tbl_period, tbl_tints where p #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'tfloats', count(*) from tbl_period, tbl_tfloats where p #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'period', 'ttexts', count(*) from tbl_period, tbl_ttexts where p #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tboolinst', count(*) from tbl_periodset, tbl_tboolinst where ps #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tintinst', count(*) from tbl_periodset, tbl_tintinst where ps #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tfloatinst', count(*) from tbl_periodset, tbl_tfloatinst where ps #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'ttextinst', count(*) from tbl_periodset, tbl_ttextinst where ps #>> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tbooli', count(*) from tbl_periodset, tbl_tbooli where ps #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tinti', count(*) from tbl_periodset, tbl_tinti where ps #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tfloati', count(*) from tbl_periodset, tbl_tfloati where ps #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'ttexti', count(*) from tbl_periodset, tbl_ttexti where ps #>> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tboolseq', count(*) from tbl_periodset, tbl_tboolseq where ps #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tintseq', count(*) from tbl_periodset, tbl_tintseq where ps #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tfloatseq', count(*) from tbl_periodset, tbl_tfloatseq where ps #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'ttextseq', count(*) from tbl_periodset, tbl_ttextseq where ps #>> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tbools', count(*) from tbl_periodset, tbl_tbools where ps #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tints', count(*) from tbl_periodset, tbl_tints where ps #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'tfloats', count(*) from tbl_periodset, tbl_tfloats where ps #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'periodset', 'ttexts', count(*) from tbl_periodset, tbl_ttexts where ps #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolinst', 'timestamptz', count(*) from tbl_tboolinst, tbl_timestamptz where inst #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolinst', 'timestampset', count(*) from tbl_tboolinst, tbl_timestampset where inst #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolinst', 'period', count(*) from tbl_tboolinst, tbl_period where inst #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolinst', 'periodset', count(*) from tbl_tboolinst, tbl_periodset where inst #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolinst', 'tboolinst', count(*) from tbl_tboolinst t1, tbl_tboolinst t2 where t1.inst #>> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolinst', 'tbooli', count(*) from tbl_tboolinst, tbl_tbooli where inst #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolinst', 'tboolseq', count(*) from tbl_tboolinst, tbl_tboolseq where inst #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolinst', 'tbools', count(*) from tbl_tboolinst, tbl_tbools where inst #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'timestamptz', count(*) from tbl_tintinst, tbl_timestamptz where inst #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'timestampset', count(*) from tbl_tintinst, tbl_timestampset where inst #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'period', count(*) from tbl_tintinst, tbl_period where inst #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'periodset', count(*) from tbl_tintinst, tbl_periodset where inst #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'tintinst', count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst #>> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'tfloatinst', count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst #>> t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'tinti', count(*) from tbl_tintinst, tbl_tinti where inst #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'tfloati', count(*) from tbl_tintinst, tbl_tfloati where inst #>> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'tintseq', count(*) from tbl_tintinst, tbl_tintseq where inst #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'tfloatseq', count(*) from tbl_tintinst, tbl_tfloatseq where inst #>> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'tints', count(*) from tbl_tintinst, tbl_tints where inst #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintinst', 'tfloats', count(*) from tbl_tintinst, tbl_tfloats where inst #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'timestamptz', count(*) from tbl_tfloatinst, tbl_timestamptz where inst #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'timestampset', count(*) from tbl_tfloatinst, tbl_timestampset where inst #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'period', count(*) from tbl_tfloatinst, tbl_period where inst #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'periodset', count(*) from tbl_tfloatinst, tbl_periodset where inst #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'tintinst', count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst #>> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'tfloatinst', count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst #>> t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'tinti', count(*) from tbl_tfloatinst, tbl_tinti where inst #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'tfloati', count(*) from tbl_tfloatinst, tbl_tfloati where inst #>> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'tintseq', count(*) from tbl_tfloatinst, tbl_tintseq where inst #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'tfloatseq', count(*) from tbl_tfloatinst, tbl_tfloatseq where inst #>> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'tints', count(*) from tbl_tfloatinst, tbl_tints where inst #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatinst', 'tfloats', count(*) from tbl_tfloatinst, tbl_tfloats where inst #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextinst', 'timestamptz', count(*) from tbl_ttextinst, tbl_timestamptz where inst #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextinst', 'timestampset', count(*) from tbl_ttextinst, tbl_timestampset where inst #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextinst', 'period', count(*) from tbl_ttextinst, tbl_period where inst #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextinst', 'periodset', count(*) from tbl_ttextinst, tbl_periodset where inst #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextinst', 'ttextinst', count(*) from tbl_ttextinst t1, tbl_ttextinst t2 where t1.inst #>> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextinst', 'ttexti', count(*) from tbl_ttextinst, tbl_ttexti where inst #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextinst', 'ttextseq', count(*) from tbl_ttextinst, tbl_ttextseq where inst #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextinst', 'ttexts', count(*) from tbl_ttextinst, tbl_ttexts where inst #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbooli', 'timestamptz', count(*) from tbl_tbooli, tbl_timestamptz where ti #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbooli', 'timestampset', count(*) from tbl_tbooli, tbl_timestampset where ti #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbooli', 'period', count(*) from tbl_tbooli, tbl_period where ti #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbooli', 'periodset', count(*) from tbl_tbooli, tbl_periodset where ti #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbooli', 'tboolinst', count(*) from tbl_tbooli, tbl_tboolinst where ti #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbooli', 'tbooli', count(*) from tbl_tbooli t1, tbl_tbooli t2 where t1.ti #>> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbooli', 'tboolseq', count(*) from tbl_tbooli, tbl_tboolseq where ti #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbooli', 'tbools', count(*) from tbl_tbooli, tbl_tbools where ti #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'timestamptz', count(*) from tbl_tinti, tbl_timestamptz where ti #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'timestampset', count(*) from tbl_tinti, tbl_timestampset where ti #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'period', count(*) from tbl_tinti, tbl_period where ti #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'periodset', count(*) from tbl_tinti, tbl_periodset where ti #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'tintinst', count(*) from tbl_tinti, tbl_tintinst where ti #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'tfloatinst', count(*) from tbl_tinti, tbl_tfloatinst where ti #>> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'tinti', count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti #>> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'tfloati', count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti #>> t2.ti;
	
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'tintseq', count(*) from tbl_tinti, tbl_tintseq where ti #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'tfloatseq', count(*) from tbl_tinti, tbl_tfloatseq where ti #>> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'tints', count(*) from tbl_tinti, tbl_tints where ti #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tinti', 'tfloats', count(*) from tbl_tinti, tbl_tfloats where ti #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'timestamptz', count(*) from tbl_tfloati, tbl_timestamptz where ti #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'timestampset', count(*) from tbl_tfloati, tbl_timestampset where ti #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'period', count(*) from tbl_tfloati, tbl_period where ti #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'periodset', count(*) from tbl_tfloati, tbl_periodset where ti #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'tintinst', count(*) from tbl_tfloati, tbl_tintinst where ti #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'tfloatinst', count(*) from tbl_tfloati, tbl_tfloatinst where ti #>> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'tinti', count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti #>> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'tfloati', count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti #>> t2.ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'tintseq', count(*) from tbl_tfloati, tbl_tintseq where ti #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'tfloatseq', count(*) from tbl_tfloati, tbl_tfloatseq where ti #>> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'tints', count(*) from tbl_tfloati, tbl_tints where ti #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloati', 'tfloats', count(*) from tbl_tfloati, tbl_tfloats where ti #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexti', 'timestamptz', count(*) from tbl_ttexti, tbl_timestamptz where ti #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexti', 'timestampset', count(*) from tbl_ttexti, tbl_timestampset where ti #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexti', 'period', count(*) from tbl_ttexti, tbl_period where ti #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexti', 'periodset', count(*) from tbl_ttexti, tbl_periodset where ti #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexti', 'ttextinst', count(*) from tbl_ttexti t1, tbl_ttextinst where ti #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexti', 'ttexti', count(*) from tbl_ttexti t1, tbl_ttexti t2 where t1.ti #>> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexti', 'ttextseq', count(*) from tbl_ttexti, tbl_ttextseq where ti #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexti', 'ttexts', count(*) from tbl_ttexti, tbl_ttexts where ti #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolseq', 'timestamptz', count(*) from tbl_tboolseq, tbl_timestamptz where seq #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolseq', 'timestampset', count(*) from tbl_tboolseq, tbl_timestampset where seq #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolseq', 'period', count(*) from tbl_tboolseq, tbl_period where seq #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolseq', 'periodset', count(*) from tbl_tboolseq, tbl_periodset where seq #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolseq', 'tboolinst', count(*) from tbl_tboolseq, tbl_tboolinst where seq #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolseq', 'tbooli', count(*) from tbl_tboolseq, tbl_tbooli where seq #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolseq', 'tboolseq', count(*) from tbl_tboolseq t1, tbl_tboolseq t2 where t1.seq #>> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tboolseq', 'tbools', count(*) from tbl_tboolseq, tbl_tbools where seq #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'timestamptz', count(*) from tbl_tintseq, tbl_timestamptz where seq #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'timestampset', count(*) from tbl_tintseq, tbl_timestampset where seq #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'period', count(*) from tbl_tintseq, tbl_period where seq #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'periodset', count(*) from tbl_tintseq, tbl_periodset where seq #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'tintinst', count(*) from tbl_tintseq, tbl_tintinst where seq #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'tfloatinst', count(*) from tbl_tintseq, tbl_tfloatinst where seq #>> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'tinti', count(*) from tbl_tintseq, tbl_tinti where seq #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'tfloati', count(*) from tbl_tintseq, tbl_tfloati where seq #>> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'tintseq', count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq #>> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'tfloatseq', count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq #>> t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'tints', count(*) from tbl_tintseq, tbl_tints where seq #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tintseq', 'tfloats', count(*) from tbl_tintseq, tbl_tfloats where seq #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'timestamptz', count(*) from tbl_tfloatseq, tbl_timestamptz where seq #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'timestampset', count(*) from tbl_tfloatseq, tbl_timestampset where seq #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'period', count(*) from tbl_tfloatseq, tbl_period where seq #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'periodset', count(*) from tbl_tfloatseq, tbl_periodset where seq #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'tintinst', count(*) from tbl_tfloatseq, tbl_tintinst where seq #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'tfloatinst', count(*) from tbl_tfloatseq, tbl_tfloatinst where seq #>> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'tinti', count(*) from tbl_tfloatseq, tbl_tinti where seq #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'tfloati', count(*) from tbl_tfloatseq, tbl_tfloati where seq #>> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'tintseq', count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq #>> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'tfloatseq', count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq #>> t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'tints', count(*) from tbl_tfloatseq, tbl_tints where seq #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloatseq', 'tfloats', count(*) from tbl_tfloatseq, tbl_tfloats where seq #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextseq', 'timestamptz', count(*) from tbl_ttextseq, tbl_timestamptz where seq #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextseq', 'timestampset', count(*) from tbl_ttextseq, tbl_timestampset where seq #>> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextseq', 'period', count(*) from tbl_ttextseq, tbl_period where seq #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextseq', 'periodset', count(*) from tbl_ttextseq, tbl_periodset where seq #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextseq', 'ttextinst', count(*) from tbl_ttextseq, tbl_ttextinst where seq #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextseq', 'ttexti', count(*) from tbl_ttextseq, tbl_ttexti where seq #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextseq', 'ttextseq', count(*) from tbl_ttextseq t1, tbl_ttextseq t2 where t1.seq #>> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttextseq', 'ttexts', count(*) from tbl_ttextseq, tbl_ttexts where seq #>> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbools', 'timestamptz', count(*) from tbl_tbools, tbl_timestamptz where ts #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbools', 'timestampset', count(*) from tbl_tbools t1, tbl_timestampset t2 where t1.ts #>> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbools', 'period', count(*) from tbl_tbools, tbl_period where ts #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbools', 'periodset', count(*) from tbl_tbools, tbl_periodset where ts #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbools', 'tboolinst', count(*) from tbl_tbools, tbl_tboolinst where ts #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbools', 'tbooli', count(*) from tbl_tbools, tbl_tbooli where ts #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbools', 'tboolseq', count(*) from tbl_tbools, tbl_tboolseq where ts #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tbools', 'tbools', count(*) from tbl_tbools t1, tbl_tbools t2 where t1.ts #>> t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'timestamptz', count(*) from tbl_tints, tbl_timestamptz where ts #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'timestampset', count(*) from tbl_tints t1, tbl_timestampset t2 where t1.ts #>> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'period', count(*) from tbl_tints, tbl_period where ts #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'periodset', count(*) from tbl_tints, tbl_periodset where ts #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'tintinst', count(*) from tbl_tints, tbl_tintinst where ts #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'tfloatinst', count(*) from tbl_tints, tbl_tfloatinst where ts #>> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'tinti', count(*) from tbl_tints, tbl_tinti where ts #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'tfloati', count(*) from tbl_tints, tbl_tfloati where ts #>> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'tintseq', count(*) from tbl_tints, tbl_tintseq where ts #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'tfloatseq', count(*) from tbl_tints, tbl_tfloatseq where ts #>> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'tints', count(*) from tbl_tints t1, tbl_tints t2 where t1.ts #>> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tints', 'tfloats', count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts #>> t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'timestamptz', count(*) from tbl_tfloats, tbl_timestamptz where ts #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'timestampset', count(*) from tbl_tfloats t1, tbl_timestampset t2 where t1.ts #>> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'period', count(*) from tbl_tfloats, tbl_period where ts #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'periodset', count(*) from tbl_tfloats, tbl_periodset where ts #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'tintinst', count(*) from tbl_tfloats, tbl_tintinst where ts #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'tfloatinst', count(*) from tbl_tfloats, tbl_tfloatinst where ts #>> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'tinti', count(*) from tbl_tfloats, tbl_tinti where ts #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'tfloati', count(*) from tbl_tfloats, tbl_tfloati where ts #>> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'tintseq', count(*) from tbl_tfloats, tbl_tintseq where ts #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'tfloatseq', count(*) from tbl_tfloats, tbl_tfloatseq where ts #>> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'tints', count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts #>> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'tfloats', 'tfloats', count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts #>> t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexts', 'timestamptz', count(*) from tbl_ttexts, tbl_timestamptz where ts #>> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexts', 'timestampset', count(*) from tbl_ttexts t1, tbl_timestampset t2 where t1.ts #>> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexts', 'period', count(*) from tbl_ttexts, tbl_period where ts #>> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexts', 'periodset', count(*) from tbl_ttexts, tbl_periodset where ts #>> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexts', 'ttextinst', count(*) from tbl_ttexts, tbl_ttextinst where ts #>> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexts', 'ttexti', count(*) from tbl_ttexts, tbl_ttexti where ts #>> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexts', 'ttextseq', count(*) from tbl_ttexts, tbl_ttextseq where ts #>> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#>>', 'ttexts', 'ttexts', count(*) from tbl_ttexts t1, tbl_ttexts t2 where t1.ts #>> t2.ts;

-------------------------------------------------------------------------------
-- Overafter
-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tboolinst', count(*) from tbl_timestamptz, tbl_tboolinst where t #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tintinst', count(*) from tbl_timestamptz, tbl_tintinst where t #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tfloatinst', count(*) from tbl_timestamptz, tbl_tfloatinst where t #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'ttextinst', count(*) from tbl_timestamptz, tbl_ttextinst where t #&> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tbooli', count(*) from tbl_timestamptz, tbl_tbooli where t #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tinti', count(*) from tbl_timestamptz, tbl_tinti where t #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tfloati', count(*) from tbl_timestamptz, tbl_tfloati where t #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'ttexti', count(*) from tbl_timestamptz, tbl_ttexti where t #&> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tboolseq', count(*) from tbl_timestamptz, tbl_tboolseq where t #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tintseq', count(*) from tbl_timestamptz, tbl_tintseq where t #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tfloatseq', count(*) from tbl_timestamptz, tbl_tfloatseq where t #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'ttextseq', count(*) from tbl_timestamptz, tbl_ttextseq where t #&> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tbools', count(*) from tbl_timestamptz, tbl_tbools where t #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tints', count(*) from tbl_timestamptz, tbl_tints where t #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'tfloats', count(*) from tbl_timestamptz, tbl_tfloats where t #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestamptz', 'ttexts', count(*) from tbl_timestamptz, tbl_ttexts where t #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tboolinst', count(*) from tbl_timestampset, tbl_tboolinst where ts #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tintinst', count(*) from tbl_timestampset, tbl_tintinst where ts #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tfloatinst', count(*) from tbl_timestampset, tbl_tfloatinst where ts #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'ttextinst', count(*) from tbl_timestampset, tbl_ttextinst where ts #&> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tbooli', count(*) from tbl_timestampset, tbl_tbooli where ts #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tinti', count(*) from tbl_timestampset, tbl_tinti where ts #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tfloati', count(*) from tbl_timestampset, tbl_tfloati where ts #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'ttexti', count(*) from tbl_timestampset, tbl_ttexti where ts #&> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tboolseq', count(*) from tbl_timestampset, tbl_tboolseq where ts #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tintseq', count(*) from tbl_timestampset, tbl_tintseq where ts #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tfloatseq', count(*) from tbl_timestampset, tbl_tfloatseq where ts #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'ttextseq', count(*) from tbl_timestampset, tbl_ttextseq where ts #&> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tbools', count(*) from tbl_timestampset t1, tbl_tbools t2 where t1.ts #&> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tints', count(*) from tbl_timestampset t1, tbl_tints t2 where t1.ts #&> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'tfloats', count(*) from tbl_timestampset t1, tbl_tfloats t2 where t1.ts #&> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'timestampset', 'ttexts', count(*) from tbl_timestampset t1, tbl_ttexts t2 where t1.ts #&> t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tboolinst', count(*) from tbl_period, tbl_tboolinst where p #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tintinst', count(*) from tbl_period, tbl_tintinst where p #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tfloatinst', count(*) from tbl_period, tbl_tfloatinst where p #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'ttextinst', count(*) from tbl_period, tbl_ttextinst where p #&> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tbooli', count(*) from tbl_period, tbl_tbooli where p #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tinti', count(*) from tbl_period, tbl_tinti where p #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tfloati', count(*) from tbl_period, tbl_tfloati where p #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'ttexti', count(*) from tbl_period, tbl_ttexti where p #&> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tboolseq', count(*) from tbl_period, tbl_tboolseq where p #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tintseq', count(*) from tbl_period, tbl_tintseq where p #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tfloatseq', count(*) from tbl_period, tbl_tfloatseq where p #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'ttextseq', count(*) from tbl_period, tbl_ttextseq where p #&> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tbools', count(*) from tbl_period, tbl_tbools where p #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tints', count(*) from tbl_period, tbl_tints where p #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'tfloats', count(*) from tbl_period, tbl_tfloats where p #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'period', 'ttexts', count(*) from tbl_period, tbl_ttexts where p #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tboolinst', count(*) from tbl_periodset, tbl_tboolinst where ps #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tintinst', count(*) from tbl_periodset, tbl_tintinst where ps #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tfloatinst', count(*) from tbl_periodset, tbl_tfloatinst where ps #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'ttextinst', count(*) from tbl_periodset, tbl_ttextinst where ps #&> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tbooli', count(*) from tbl_periodset, tbl_tbooli where ps #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tinti', count(*) from tbl_periodset, tbl_tinti where ps #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tfloati', count(*) from tbl_periodset, tbl_tfloati where ps #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'ttexti', count(*) from tbl_periodset, tbl_ttexti where ps #&> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tboolseq', count(*) from tbl_periodset, tbl_tboolseq where ps #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tintseq', count(*) from tbl_periodset, tbl_tintseq where ps #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tfloatseq', count(*) from tbl_periodset, tbl_tfloatseq where ps #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'ttextseq', count(*) from tbl_periodset, tbl_ttextseq where ps #&> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tbools', count(*) from tbl_periodset, tbl_tbools where ps #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tints', count(*) from tbl_periodset, tbl_tints where ps #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'tfloats', count(*) from tbl_periodset, tbl_tfloats where ps #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'periodset', 'ttexts', count(*) from tbl_periodset, tbl_ttexts where ps #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolinst', 'timestamptz', count(*) from tbl_tboolinst, tbl_timestamptz where inst #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolinst', 'timestampset', count(*) from tbl_tboolinst, tbl_timestampset where inst #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolinst', 'period', count(*) from tbl_tboolinst, tbl_period where inst #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolinst', 'periodset', count(*) from tbl_tboolinst, tbl_periodset where inst #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolinst', 'tboolinst', count(*) from tbl_tboolinst t1, tbl_tboolinst t2 where t1.inst #&> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolinst', 'tbooli', count(*) from tbl_tboolinst, tbl_tbooli where inst #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolinst', 'tboolseq', count(*) from tbl_tboolinst, tbl_tboolseq where inst #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolinst', 'tbools', count(*) from tbl_tboolinst, tbl_tbools where inst #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'timestamptz', count(*) from tbl_tintinst, tbl_timestamptz where inst #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'timestampset', count(*) from tbl_tintinst, tbl_timestampset where inst #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'period', count(*) from tbl_tintinst, tbl_period where inst #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'periodset', count(*) from tbl_tintinst, tbl_periodset where inst #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'tintinst', count(*) from tbl_tintinst t1, tbl_tintinst t2 where t1.inst #&> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'tfloatinst', count(*) from tbl_tintinst t1, tbl_tfloatinst t2 where t1.inst #&> t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'tinti', count(*) from tbl_tintinst, tbl_tinti where inst #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'tfloati', count(*) from tbl_tintinst, tbl_tfloati where inst #&> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'tintseq', count(*) from tbl_tintinst, tbl_tintseq where inst #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'tfloatseq', count(*) from tbl_tintinst, tbl_tfloatseq where inst #&> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'tints', count(*) from tbl_tintinst, tbl_tints where inst #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintinst', 'tfloats', count(*) from tbl_tintinst, tbl_tfloats where inst #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'timestamptz', count(*) from tbl_tfloatinst, tbl_timestamptz where inst #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'timestampset', count(*) from tbl_tfloatinst, tbl_timestampset where inst #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'period', count(*) from tbl_tfloatinst, tbl_period where inst #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'periodset', count(*) from tbl_tfloatinst, tbl_periodset where inst #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'tintinst', count(*) from tbl_tfloatinst t1, tbl_tintinst t2 where t1.inst #&> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'tfloatinst', count(*) from tbl_tfloatinst t1, tbl_tfloatinst t2 where t1.inst #&> t2.inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'tinti', count(*) from tbl_tfloatinst, tbl_tinti where inst #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'tfloati', count(*) from tbl_tfloatinst, tbl_tfloati where inst #&> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'tintseq', count(*) from tbl_tfloatinst, tbl_tintseq where inst #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'tfloatseq', count(*) from tbl_tfloatinst, tbl_tfloatseq where inst #&> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'tints', count(*) from tbl_tfloatinst, tbl_tints where inst #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatinst', 'tfloats', count(*) from tbl_tfloatinst, tbl_tfloats where inst #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextinst', 'timestamptz', count(*) from tbl_ttextinst, tbl_timestamptz where inst #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextinst', 'timestampset', count(*) from tbl_ttextinst, tbl_timestampset where inst #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextinst', 'period', count(*) from tbl_ttextinst, tbl_period where inst #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextinst', 'periodset', count(*) from tbl_ttextinst, tbl_periodset where inst #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextinst', 'ttextinst', count(*) from tbl_ttextinst t1, tbl_ttextinst t2 where t1.inst #&> t2.inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextinst', 'ttexti', count(*) from tbl_ttextinst, tbl_ttexti where inst #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextinst', 'ttextseq', count(*) from tbl_ttextinst, tbl_ttextseq where inst #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextinst', 'ttexts', count(*) from tbl_ttextinst, tbl_ttexts where inst #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbooli', 'timestamptz', count(*) from tbl_tbooli, tbl_timestamptz where ti #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbooli', 'timestampset', count(*) from tbl_tbooli, tbl_timestampset where ti #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbooli', 'period', count(*) from tbl_tbooli, tbl_period where ti #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbooli', 'periodset', count(*) from tbl_tbooli, tbl_periodset where ti #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbooli', 'tboolinst', count(*) from tbl_tbooli, tbl_tboolinst where ti #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbooli', 'tbooli', count(*) from tbl_tbooli t1, tbl_tbooli t2 where t1.ti #&> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbooli', 'tboolseq', count(*) from tbl_tbooli, tbl_tboolseq where ti #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbooli', 'tbools', count(*) from tbl_tbooli, tbl_tbools where ti #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'timestamptz', count(*) from tbl_tinti, tbl_timestamptz where ti #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'timestampset', count(*) from tbl_tinti, tbl_timestampset where ti #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'period', count(*) from tbl_tinti, tbl_period where ti #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'periodset', count(*) from tbl_tinti, tbl_periodset where ti #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'tintinst', count(*) from tbl_tinti, tbl_tintinst where ti #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'tfloatinst', count(*) from tbl_tinti, tbl_tfloatinst where ti #&> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'tinti', count(*) from tbl_tinti t1, tbl_tinti t2 where t1.ti #&> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'tfloati', count(*) from tbl_tinti t1, tbl_tfloati t2 where t1.ti #&> t2.ti;
	
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'tintseq', count(*) from tbl_tinti, tbl_tintseq where ti #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'tfloatseq', count(*) from tbl_tinti, tbl_tfloatseq where ti #&> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'tints', count(*) from tbl_tinti, tbl_tints where ti #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tinti', 'tfloats', count(*) from tbl_tinti, tbl_tfloats where ti #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'timestamptz', count(*) from tbl_tfloati, tbl_timestamptz where ti #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'timestampset', count(*) from tbl_tfloati, tbl_timestampset where ti #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'period', count(*) from tbl_tfloati, tbl_period where ti #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'periodset', count(*) from tbl_tfloati, tbl_periodset where ti #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'tintinst', count(*) from tbl_tfloati, tbl_tintinst where ti #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'tfloatinst', count(*) from tbl_tfloati, tbl_tfloatinst where ti #&> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'tinti', count(*) from tbl_tfloati t1, tbl_tinti t2 where t1.ti #&> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'tfloati', count(*) from tbl_tfloati t1, tbl_tfloati t2 where t1.ti #&> t2.ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'tintseq', count(*) from tbl_tfloati, tbl_tintseq where ti #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'tfloatseq', count(*) from tbl_tfloati, tbl_tfloatseq where ti #&> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'tints', count(*) from tbl_tfloati, tbl_tints where ti #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloati', 'tfloats', count(*) from tbl_tfloati, tbl_tfloats where ti #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexti', 'timestamptz', count(*) from tbl_ttexti, tbl_timestamptz where ti #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexti', 'timestampset', count(*) from tbl_ttexti, tbl_timestampset where ti #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexti', 'period', count(*) from tbl_ttexti, tbl_period where ti #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexti', 'periodset', count(*) from tbl_ttexti, tbl_periodset where ti #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexti', 'ttextinst', count(*) from tbl_ttexti t1, tbl_ttextinst where ti #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexti', 'ttexti', count(*) from tbl_ttexti t1, tbl_ttexti t2 where t1.ti #&> t2.ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexti', 'ttextseq', count(*) from tbl_ttexti, tbl_ttextseq where ti #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexti', 'ttexts', count(*) from tbl_ttexti, tbl_ttexts where ti #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolseq', 'timestamptz', count(*) from tbl_tboolseq, tbl_timestamptz where seq #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolseq', 'timestampset', count(*) from tbl_tboolseq, tbl_timestampset where seq #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolseq', 'period', count(*) from tbl_tboolseq, tbl_period where seq #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolseq', 'periodset', count(*) from tbl_tboolseq, tbl_periodset where seq #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolseq', 'tboolinst', count(*) from tbl_tboolseq, tbl_tboolinst where seq #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolseq', 'tbooli', count(*) from tbl_tboolseq, tbl_tbooli where seq #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolseq', 'tboolseq', count(*) from tbl_tboolseq t1, tbl_tboolseq t2 where t1.seq #&> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tboolseq', 'tbools', count(*) from tbl_tboolseq, tbl_tbools where seq #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'timestamptz', count(*) from tbl_tintseq, tbl_timestamptz where seq #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'timestampset', count(*) from tbl_tintseq, tbl_timestampset where seq #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'period', count(*) from tbl_tintseq, tbl_period where seq #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'periodset', count(*) from tbl_tintseq, tbl_periodset where seq #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'tintinst', count(*) from tbl_tintseq, tbl_tintinst where seq #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'tfloatinst', count(*) from tbl_tintseq, tbl_tfloatinst where seq #&> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'tinti', count(*) from tbl_tintseq, tbl_tinti where seq #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'tfloati', count(*) from tbl_tintseq, tbl_tfloati where seq #&> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'tintseq', count(*) from tbl_tintseq t1, tbl_tintseq t2 where t1.seq #&> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'tfloatseq', count(*) from tbl_tintseq t1, tbl_tfloatseq t2 where t1.seq #&> t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'tints', count(*) from tbl_tintseq, tbl_tints where seq #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tintseq', 'tfloats', count(*) from tbl_tintseq, tbl_tfloats where seq #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'timestamptz', count(*) from tbl_tfloatseq, tbl_timestamptz where seq #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'timestampset', count(*) from tbl_tfloatseq, tbl_timestampset where seq #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'period', count(*) from tbl_tfloatseq, tbl_period where seq #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'periodset', count(*) from tbl_tfloatseq, tbl_periodset where seq #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'tintinst', count(*) from tbl_tfloatseq, tbl_tintinst where seq #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'tfloatinst', count(*) from tbl_tfloatseq, tbl_tfloatinst where seq #&> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'tinti', count(*) from tbl_tfloatseq, tbl_tinti where seq #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'tfloati', count(*) from tbl_tfloatseq, tbl_tfloati where seq #&> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'tintseq', count(*) from tbl_tfloatseq t1, tbl_tintseq t2 where t1.seq #&> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'tfloatseq', count(*) from tbl_tfloatseq t1, tbl_tfloatseq t2 where t1.seq #&> t2.seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'tints', count(*) from tbl_tfloatseq, tbl_tints where seq #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloatseq', 'tfloats', count(*) from tbl_tfloatseq, tbl_tfloats where seq #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextseq', 'timestamptz', count(*) from tbl_ttextseq, tbl_timestamptz where seq #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextseq', 'timestampset', count(*) from tbl_ttextseq, tbl_timestampset where seq #&> ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextseq', 'period', count(*) from tbl_ttextseq, tbl_period where seq #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextseq', 'periodset', count(*) from tbl_ttextseq, tbl_periodset where seq #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextseq', 'ttextinst', count(*) from tbl_ttextseq, tbl_ttextinst where seq #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextseq', 'ttexti', count(*) from tbl_ttextseq, tbl_ttexti where seq #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextseq', 'ttextseq', count(*) from tbl_ttextseq t1, tbl_ttextseq t2 where t1.seq #&> t2.seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttextseq', 'ttexts', count(*) from tbl_ttextseq, tbl_ttexts where seq #&> ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbools', 'timestamptz', count(*) from tbl_tbools, tbl_timestamptz where ts #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbools', 'timestampset', count(*) from tbl_tbools t1, tbl_timestampset t2 where t1.ts #&> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbools', 'period', count(*) from tbl_tbools, tbl_period where ts #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbools', 'periodset', count(*) from tbl_tbools, tbl_periodset where ts #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbools', 'tboolinst', count(*) from tbl_tbools, tbl_tboolinst where ts #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbools', 'tbooli', count(*) from tbl_tbools, tbl_tbooli where ts #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbools', 'tboolseq', count(*) from tbl_tbools, tbl_tboolseq where ts #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tbools', 'tbools', count(*) from tbl_tbools t1, tbl_tbools t2 where t1.ts #&> t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'timestamptz', count(*) from tbl_tints, tbl_timestamptz where ts #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'timestampset', count(*) from tbl_tints t1, tbl_timestampset t2 where t1.ts #&> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'period', count(*) from tbl_tints, tbl_period where ts #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'periodset', count(*) from tbl_tints, tbl_periodset where ts #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'tintinst', count(*) from tbl_tints, tbl_tintinst where ts #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'tfloatinst', count(*) from tbl_tints, tbl_tfloatinst where ts #&> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'tinti', count(*) from tbl_tints, tbl_tinti where ts #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'tfloati', count(*) from tbl_tints, tbl_tfloati where ts #&> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'tintseq', count(*) from tbl_tints, tbl_tintseq where ts #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'tfloatseq', count(*) from tbl_tints, tbl_tfloatseq where ts #&> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'tints', count(*) from tbl_tints t1, tbl_tints t2 where t1.ts #&> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tints', 'tfloats', count(*) from tbl_tints t1, tbl_tfloats t2 where t1.ts #&> t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'timestamptz', count(*) from tbl_tfloats, tbl_timestamptz where ts #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'timestampset', count(*) from tbl_tfloats t1, tbl_timestampset t2 where t1.ts #&> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'period', count(*) from tbl_tfloats, tbl_period where ts #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'periodset', count(*) from tbl_tfloats, tbl_periodset where ts #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'tintinst', count(*) from tbl_tfloats, tbl_tintinst where ts #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'tfloatinst', count(*) from tbl_tfloats, tbl_tfloatinst where ts #&> inst;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'tinti', count(*) from tbl_tfloats, tbl_tinti where ts #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'tfloati', count(*) from tbl_tfloats, tbl_tfloati where ts #&> ti;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'tintseq', count(*) from tbl_tfloats, tbl_tintseq where ts #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'tfloatseq', count(*) from tbl_tfloats, tbl_tfloatseq where ts #&> seq;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'tints', count(*) from tbl_tfloats t1, tbl_tints t2 where t1.ts #&> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'tfloats', 'tfloats', count(*) from tbl_tfloats t1, tbl_tfloats t2 where t1.ts #&> t2.ts;

-------------------------------------------------------------------------------

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexts', 'timestamptz', count(*) from tbl_ttexts, tbl_timestamptz where ts #&> t;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexts', 'timestampset', count(*) from tbl_ttexts t1, tbl_timestampset t2 where t1.ts #&> t2.ts;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexts', 'period', count(*) from tbl_ttexts, tbl_period where ts #&> p;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexts', 'periodset', count(*) from tbl_ttexts, tbl_periodset where ts #&> ps;

insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexts', 'ttextinst', count(*) from tbl_ttexts, tbl_ttextinst where ts #&> inst;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexts', 'ttexti', count(*) from tbl_ttexts, tbl_ttexti where ts #&> ti;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexts', 'ttextseq', count(*) from tbl_ttexts, tbl_ttextseq where ts #&> seq;
insert into test_relativeposops(op, leftarg, rightarg, noidx) 
select '#&>', 'ttexts', 'ttexts', count(*) from tbl_ttexts t1, tbl_ttexts t2 where t1.ts #&> t2.ts;

/*****************************************************************************/
