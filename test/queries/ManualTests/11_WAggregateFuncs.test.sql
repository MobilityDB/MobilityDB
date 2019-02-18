/*****************************************************************************
 * TemporalInst
 *****************************************************************************/

drop table if exists prescription_tintinst;
create table prescription_tintinst as (
select *
from ( Values 
('Amy', tintinst '2@2000-01-01 08:10'),
('Ben', tintinst '3@2000-01-01 08:10'),
('Cal', tintinst '1@2000-01-01 08:20'),
('Dan', tintinst '2@2000-01-01 08:05'),
('Eve', tintinst '4@2000-01-01 08:35'),
('Fay', tintinst '1@2000-01-01 08:10')
) as T(patient, dosage));

select unnest(periods(wmin(dosage, interval '5 minutes')))
from prescription_tintinst;

select unnest(periods(wmax(dosage, interval '5 minutes')))
from prescription_tintinst;

select unnest(periods(wsum(dosage, interval '5 minutes')))
from prescription_tintinst;

select unnest(periods(wcount(dosage, interval '5 minutes')))
from prescription_tintinst;

select unnest(periods(wavg(dosage, interval '5 minutes')))
from prescription_tintinst;

/*****************************************************************************/

drop table if exists prescription_tfloatinst;
create table prescription_tfloatinst as (
select *
from ( Values 
('Amy', tfloatinst '2@2000-01-01 08:10'),
('Ben', tfloatinst '3@2000-01-01 08:10'),
('Cal', tfloatinst '1@2000-01-01 08:20'),
('Dan', tfloatinst '2@2000-01-01 08:05'),
('Eve', tfloatinst '4@2000-01-01 08:35'),
('Fay', tfloatinst '1@2000-01-01 08:10')
) as T(patient, dosage));

select * from prescription_tfloatinst;

select unnest(periods(wmin(dosage, interval '5 minutes')))
from prescription_tfloatinst;

select unnest(periods(wmax(dosage, interval '5 minutes')))
from prescription_tfloatinst;

select unnest(periods(wsum(dosage, interval '5 minutes')))
from prescription_tfloatinst;

select unnest(periods(wcount(dosage, interval '5 minutes')))
from prescription_tfloatinst;

select unnest(periods(wavg(dosage, interval '5 minutes')))
from prescription_tfloatinst;

/*****************************************************************************
 * TemporalI
 *****************************************************************************/

drop table if exists prescription_tinti;
create table prescription_tinti as (
select *
from ( Values 
('Amy', tinti '{2@2000-01-01 08:10,4@[2000-01-01 08:25}'),
('Ben', tinti '{3@2000-01-01 08:10,2@[2000-01-01 08:20}'),
('Cal', tinti '{1@2000-01-01 08:20,3@[2000-01-01 08:30}'),
('Dan', tinti '{2@2000-01-01 08:05,1@[2000-01-01 08:10}'),
('Eve', tinti '{4@2000-01-01 08:35,3@[2000-01-01 08:40}'),
('Fay', tinti '{1@2000-01-01 08:10,2@[2000-01-01 08:30}')
) as T(patient, dosage));

select unnest(periods(wmin(dosage, interval '5 minutes')))
from prescription_tinti;

select unnest(periods(wmax(dosage, interval '5 minutes')))
from prescription_tinti;

select unnest(periods(wsum(dosage, interval '5 minutes')))
from prescription_tinti;

select unnest(periods(wcount(dosage, interval '5 minutes')))
from prescription_tinti;

select unnest(periods(wavg(dosage, interval '5 minutes')))
from prescription_tinti;

/*****************************************************************************/


drop table if exists prescription_tfloati;
create table prescription_tfloati as (
select *
from ( Values 
('Amy', tfloati '{2@2000-01-01 08:10,4@[2000-01-01 08:25}'),
('Ben', tfloati '{3@2000-01-01 08:10,2@[2000-01-01 08:20}'),
('Cal', tfloati '{1@2000-01-01 08:20,3@[2000-01-01 08:30}'),
('Dan', tfloati '{2@2000-01-01 08:05,1@[2000-01-01 08:10}'),
('Eve', tfloati '{4@2000-01-01 08:35,3@[2000-01-01 08:40}'),
('Fay', tfloati '{1@2000-01-01 08:10,2@[2000-01-01 08:30}')
) as T(patient, dosage));

select unnest(periods(wmin(dosage, interval '5 minutes')))
from prescription_tfloati;

select unnest(periods(wmax(dosage, interval '5 minutes')))
from prescription_tfloati;

select unnest(periods(wsum(dosage, interval '5 minutes')))
from prescription_tfloati;

select unnest(periods(wcount(dosage, interval '5 minutes')))
from prescription_tfloati;

select unnest(periods(wavg(dosage, interval '5 minutes')))
from prescription_tfloati;

/*****************************************************************************
 * TemporalSeq
 *****************************************************************************/

drop table if exists prescription_tintseq;
create table prescription_tintseq as (
select *
from ( Values 
('Amy', tintseq '[2@2000-01-01 08:10, 2@2000-01-01 08:40)'),
('Ben', tintseq '[3@2000-01-01 08:10, 3@2000-01-01 08:30)'),
('Cal', tintseq '[1@2000-01-01 08:20, 1@2000-01-01 08:40)'),
('Dan', tintseq '[2@2000-01-01 08:05, 2@2000-01-01 08:15)'),
('Eve', tintseq '[4@2000-01-01 08:35, 4@2000-01-01 08:45)'),
('Fay', tintseq '[2000-01-01 08:10, 1@2000-01-01 08:50)')
) as T(patient, dosage));

select unnest(periods(wmin(dosage, interval '5 minutes')))
from prescription_tintseq;

select unnest(periods(wmax(dosage, interval '5 minutes')))
from prescription_tintseq;

select unnest(periods(wsum(dosage, interval '5 minutes')))
from prescription_tintseq;

select unnest(periods(wcount(dosage, interval '5 minutes')))
from prescription_tintseq;

select unnest(periods(wavg(dosage, interval '5 minutes')))
from prescription_tintseq;

/*****************************************************************************/

drop table if exists prescription_tfloatseq;
create table prescription_tfloatseq as (
select *
from ( Values 
('Amy', tfloatseq '[2@2000-01-01 08:10, 3@2000-01-01 08:40)'),
('Ben', tfloatseq '[3@2000-01-01 08:10, 3@2000-01-01 08:30)'),
('Cal', tfloatseq '[1@2000-01-01 08:20, 3@2000-01-01 08:40)'),
('Dan', tfloatseq '[2@2000-01-01 08:05, 4@2000-01-01 08:15)'),
('Eve', tfloatseq '[4@2000-01-01 08:35, 2@2000-01-01 08:45)'),
('Fay', tfloatseq '[1@2000-01-01 08:10, 4@2000-01-01 08:50)')
) as T(patient, dosage));

select unnest(periods(wmin(dosage, interval '5 minutes')))
from prescription_tfloatseq;

select unnest(periods(wmax(dosage, interval '5 minutes')))
from prescription_tfloatseq;

select unnest(periods(wcount(dosage, interval '5 minutes')))
from prescription_tfloatseq;

/*****************************************************************************
 * TemporalP
 *****************************************************************************/

drop table if exists prescription_tints;
create table prescription_tints as (
select *
from ( Values 
('Amy', tints '{2@[2000-01-01 08:10, 2000-01-01 08:25),4@[2000-01-01 08:25, 2000-01-01 08:40)}'),
('Ben', tints '{3@[2000-01-01 08:10, 2000-01-01 08:20),2@[2000-01-01 08:20, 2000-01-01 08:30)}'),
('Cal', tints '{1@[2000-01-01 08:20, 2000-01-01 08:30),3@[2000-01-01 08:30, 2000-01-01 08:40)}'),
('Dan', tints '{2@[2000-01-01 08:05, 2000-01-01 08:10),1@[2000-01-01 08:10, 2000-01-01 08:15)}'),
('Eve', tints '{4@[2000-01-01 08:35, 2000-01-01 08:40),3@[2000-01-01 08:40, 2000-01-01 08:45)}'),
('Fay', tints '{1@[2000-01-01 08:10, 2000-01-01 08:30),2@[2000-01-01 08:30, 2000-01-01 08:50)}')
) as T(patient, dosage));

select unnest(periods(wmin(dosage, interval '5 minutes')))
from prescription_tints;

select unnest(periods(wmax(dosage, interval '5 minutes')))
from prescription_tints;

select unnest(periods(wsum(dosage, interval '5 minutes')))
from prescription_tints;

select unnest(periods(wcount(dosage, interval '5 minutes')))
from prescription_tints;

select unnest(periods(wavg(dosage, interval '5 minutes')))
from prescription_tints;

/*****************************************************************************/

drop table if exists prescription_tfloats;
create table prescription_tfloats as (
select *
from ( Values 
('Amy', tfloats '{[2@2000-01-01 08:10, 4@2000-01-01 08:25),[4@2000-01-01 08:25, 2@2000-01-01 08:40)}'),
('Ben', tfloats '{[3@2000-01-01 08:10, 2@2000-01-01 08:20),[2@2000-01-01 08:20, 3@2000-01-01 08:30)}'),
('Cal', tfloats '{[1@2000-01-01 08:20, 3@2000-01-01 08:30),[1@2000-01-01 08:30, 1@2000-01-01 08:40)}'),
('Dan', tfloats '{[2@2000-01-01 08:05, 2@2000-01-01 08:10),[1@2000-01-01 08:10, 1@2000-01-01 08:15)}'),
('Eve', tfloats '{[4@2000-01-01 08:35, 3@2000-01-01 08:40),[3@2000-01-01 08:40, 4@2000-01-01 08:45)}'),
('Fay', tfloats '{[1@2000-01-01 08:10, 2@2000-01-01 08:30),[2@2000-01-01 08:30, 1@2000-01-01 08:50)}')
) as T(patient, dosage));

select unnest(periods(wmin(dosage, interval '5 minutes')))
from prescription_tfloats;

select unnest(periods(wmax(dosage, interval '5 minutes')))
from prescription_tfloats;

select unnest(periods(wcount(dosage, interval '5 minutes')))
from prescription_tfloats;

/*****************************************************************************/
