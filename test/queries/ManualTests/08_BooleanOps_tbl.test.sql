-----------------------------------------------------------------------
-- Boolean functions and operators
-- N.B. The names and, or, not are reserved words 
-----------------------------------------------------------------------

select count(*) from tbl_tboolinst where TRUE & inst is not null;
select count(*) from tbl_tbooli where TRUE & ti is not null;
select count(*) from tbl_tboolseq where TRUE & seq is not null;
select count(*) from tbl_tbools where TRUE & ts is not null;

select count(*) from tbl_tboolinst where inst & TRUE is not null;
select count(*) from tbl_tboolinst t1, tbl_tboolinst t2 where t1.inst & t2.inst is not null;
select count(*) from tbl_tboolinst, tbl_tbooli where inst & ti is not null;
select count(*) from tbl_tboolinst, tbl_tboolseq where inst & seq is not null;
select count(*) from tbl_tboolinst, tbl_tbools where inst & ts is not null;

select count(*) from tbl_tbooli where ti & TRUE is not null;
select count(*) from tbl_tbooli, tbl_tboolinst where ti & inst is not null;
select count(*) from tbl_tbooli t1, tbl_tbooli t2 where t1.ti & t2.ti is not null;
select count(*) from tbl_tbooli, tbl_tboolseq where ti & seq is not null;
select count(*) from tbl_tbooli, tbl_tbools where ti & ts is not null;

select count(*) from tbl_tboolseq where seq & TRUE is not null;
select count(*) from tbl_tboolseq, tbl_tboolinst where seq & inst is not null;
select count(*) from tbl_tboolseq, tbl_tbooli where seq  & ti is not null;
select count(*) from tbl_tboolseq t1, tbl_tboolseq t2 where t1.seq & t2.seq is not null;
select count(*) from tbl_tboolseq, tbl_tbools where seq & ts is not null;

select count(*) from tbl_tbools where ts & TRUE is not null;
select count(*) from tbl_tbools, tbl_tboolinst where ts & inst is not null;
select count(*) from tbl_tbools, tbl_tbooli where ts & ti is not null;
select count(*) from tbl_tbools, tbl_tboolseq where ts & seq is not null;
select count(*) from tbl_tbools t1, tbl_tbools t2 where t1.ts & t2.ts is not null;

-------------------------------------------------------------------------------

select count(*) from tbl_tboolinst where TRUE | inst is not null;
select count(*) from tbl_tbooli where TRUE | ti is not null;
select count(*) from tbl_tboolseq where TRUE | seq is not null;
select count(*) from tbl_tbools where TRUE | ts is not null;

select count(*) from tbl_tboolinst where inst | TRUE is not null;
select count(*) from tbl_tboolinst t1, tbl_tboolinst t2 where t1.inst | t2.inst is not null;
select count(*) from tbl_tboolinst, tbl_tbooli where inst | ti is not null;
select count(*) from tbl_tboolinst, tbl_tboolseq where inst | seq is not null;
select count(*) from tbl_tboolinst, tbl_tbools where inst | ts is not null;

select count(*) from tbl_tbooli where ti | TRUE is not null;
select count(*) from tbl_tbooli, tbl_tboolinst where ti | inst is not null;
select count(*) from tbl_tbooli t1, tbl_tbooli t2 where t1.ti | t2.ti is not null;
select count(*) from tbl_tbooli, tbl_tboolseq where ti | seq is not null;
select count(*) from tbl_tbooli, tbl_tbools where ti | ts is not null;

select count(*) from tbl_tboolseq where seq | TRUE is not null;
select count(*) from tbl_tboolseq, tbl_tboolinst where seq | inst is not null;
select count(*) from tbl_tboolseq, tbl_tbooli where seq  | ti is not null;
select count(*) from tbl_tboolseq t1, tbl_tboolseq t2 where t1.seq | t2.seq is not null;
select count(*) from tbl_tboolseq, tbl_tbools where seq | ts is not null;

select count(*) from tbl_tbools where ts | TRUE is not null;
select count(*) from tbl_tbools, tbl_tboolinst where ts | inst is not null;
select count(*) from tbl_tbools, tbl_tbooli where ts | ti is not null;
select count(*) from tbl_tbools, tbl_tboolseq where ts | seq is not null;
select count(*) from tbl_tbools t1, tbl_tbools t2 where t1.ts | t2.ts is not null;

-------------------------------------------------------------------------------

select count(*) from tbl_tboolinst where ~ inst is not null;
select count(*) from tbl_tbooli where ~ ti is not null;
select count(*) from tbl_tboolseq where ~ seq is not null;
select count(*) from tbl_tbools where ~ ts is not null;

/*****************************************************************************/
