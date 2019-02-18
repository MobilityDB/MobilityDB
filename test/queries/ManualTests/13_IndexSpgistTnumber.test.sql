------------------------------------------------------

select count(*) from tbl_tintseq
where getValue(seq) = 5;

select count(*) from tbl_tintseq
where seq <@ 5;

select count(*) from tbl_tintseq
where seq <@ 5.0;

select count(*) from tbl_tintseq
where getValue(seq) <@ intrange '[2,5]';

select count(*) from tbl_tintseq
where seq <@ intrange '[2,5]';

select count(*) from tbl_tintseq
where seq <@ floatrange '[2,5]';

select count(*) from tbl_tintseq
where lower(getTime(seq)) = timestamp '2001-01-21 18:43:36';

select count(*) from tbl_tintseq
where seq <@ timestamp '2001-01-21 18:43:36';

select count(*) from tbl_tintseq
where getTime(seq) <@ period('2001-03-01', '2001-07-31');

select count(*) from tbl_tintseq
where seq <@ period('2001-03-01', '2001-07-31');

select count(*) from tbl_tintseq
where seq <@ box(tintseq(5, '2001-03-01', '2001-07-31'));

select count(*) from tbl_tintseq
where seq <@ tintseq(5, '2001-03-01', '2001-07-31');

select count(*) from tbl_tintseq
where seq <@ tfloatper(5, 5, '2001-03-01', '2001-07-31');

select count(*) from tbl_tintseq
where seq <@ tfloatper(5, 7, '2001-03-01', '2001-07-31');

select count(*) from tbl_tintseq
where seq <@ tints(ARRAY[
tintseq(5, '2001-03-01', '2001-05-01'),
tintseq(7, '2001-05-01', '2001-07-31', true, true)]);

select count(*) from tbl_tintseq
where seq <@ tfloats(ARRAY[
tfloatper(5, 6, '2001-03-01', '2001-05-01'),
tfloatper(6, 7, '2001-05-01', '2001-07-31', true, true)]);

select count(*) from tbl_tintseq
where seq <@ tinti(ARRAY[
tintinst(5, '2001-03-01'),
tintinst(7, '2001-07-31')]);

select count(*) from tbl_tintseq
where seq <@ tfloati(ARRAY[
tfloatinst(5, '2001-03-01'),
tfloatinst(7, '2001-07-31')]);

------------------------------------------------------

-- drop index if exists tbl_tfloatseq_spgist_idx; 

-- create index tbl_tfloatseq_spgist_idx on tbl_tfloatseq using spgist(seq)

------------------------------------------------------

-- select count(*) from tbl_tfloatseq
-- where getValues(seq) @> 5.0;

select count(*) from tbl_tfloatseq
where seq <@ 5;

select count(*) from tbl_tfloatseq
where seq <@ 5.0;

select count(*) from tbl_tfloatseq
where getValues(seq) <@ floatrange '[2,5]';

select count(*) from tbl_tfloatseq
where seq <@ intrange '[2,5]';

select count(*) from tbl_tfloatseq
where seq <@ floatrange '[2,5]';

select count(*) from tbl_tfloatseq
where getTime(seq) @> timestamp '2001-06-05 13:14:33';

select count(*) from tbl_tfloatseq
where seq <@ timestamp '2001-06-05 13:14:33';

select count(*) from tbl_tfloatseq
where getTime(seq) <@ period('2001-03-01', '2001-07-31');

select count(*) from tbl_tfloatseq
where seq <@ period('2001-03-01', '2001-07-31');

select count(*) from tbl_tfloatseq
where seq <@ box(tintseq(5, '2001-03-01', '2001-07-31'));

select count(*) from tbl_tfloatseq
where seq <@ tintseq(5, '2001-03-01', '2001-07-31');

select count(*) from tbl_tfloatseq
where seq <@ tfloatper(5, 5, '2001-03-01', '2001-07-31');

select count(*) from tbl_tfloatseq
where seq <@ tfloatper(5, 7, '2001-03-01', '2001-07-31');

select count(*) from tbl_tfloatseq
where seq <@ tints(ARRAY[
tintseq(5, '2001-03-01', '2001-05-01'),
tintseq(7, '2001-05-01', '2001-07-31', true, true)]);

select count(*) from tbl_tfloatseq
where seq <@ tfloats(ARRAY[
tfloatper(5, 6, '2001-03-01', '2001-05-01'),
tfloatper(6, 7, '2001-05-01', '2001-07-31', true, true)]);

select count(*) from tbl_tfloatseq
where seq <@ tinti(ARRAY[
tintinst(5, '2001-03-01'),
tintinst(7, '2001-07-31')]);

select count(*) from tbl_tfloatseq
where seq <@ tfloati(ARRAY[
tfloatinst(5, '2001-03-01'),
tfloatinst(7, '2001-07-31')]);

------------------------------------------------------

select count(*) from tbl_tfloats
where ts && 10.0;

select count(*) from tbl_tfloats
where ts @> 10.0;

select count(*) from tbl_tfloats
where ts <@ 10.0;

select count(*) from tbl_tfloats
where ts ~= 10.0;

select count(*) from tbl_tfloats
where ts << 10.0;

select count(*) from tbl_tfloats
where ts &< 10.0;

select count(*) from tbl_tfloats
where ts >> 10.0;

select count(*) from tbl_tfloats
where ts &> 10.0;

select count(*) from tbl_tfloats
where ts && floatrange '[10, 30]';

select count(*) from tbl_tfloats
where ts @> floatrange '[10, 30]';

select count(*) from tbl_tfloats
where ts <@ floatrange '[10, 30]';

select count(*) from tbl_tfloats
where ts ~= floatrange '[10, 30]';

select count(*) from tbl_tfloats
where ts << floatrange '[10, 30]';

select count(*) from tbl_tfloats
where ts &< floatrange '[10, 30]';

select count(*) from tbl_tfloats
where ts >> floatrange '[10, 30]';

select count(*) from tbl_tfloats
where ts &> floatrange '[10, 30]';

-- create index tbl_tfloats_gist_idx on tbl_tfloats using gist(seq)

------------------------------------------------------


