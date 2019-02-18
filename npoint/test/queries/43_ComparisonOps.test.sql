/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

SELECT t1.np #= t2.inst FROM npoint_tbl t1, tbl_tnpointinst t2;
SELECT t1.np #= t2.ti FROM npoint_tbl t1, tbl_tnpointi t2;
SELECT t1.np #= t2.seq FROM npoint_tbl t1, tbl_tnpointseq t2;
SELECT t1.np #= t2.ts FROM npoint_tbl t1, tbl_tnpoints t2;

SELECT t1.inst #= t2.np FROM tbl_tnpointinst t1, npoint_tbl t2;
SELECT t1.inst #= t2.inst FROM tbl_tnpointinst t1, tbl_tnpointinst t2;
SELECT t1.inst #= t2.ti FROM tbl_tnpointinst t1, tbl_tnpointi t2;
SELECT t1.inst #= t2.seq FROM tbl_tnpointinst t1, tbl_tnpointseq t2;
SELECT t1.inst #= t2.ts FROM tbl_tnpointinst t1, tbl_tnpoints t2;

SELECT t1.ti #= t2.np FROM tbl_tnpointi t1, npoint_tbl t2;
SELECT t1.ti #= t2.inst FROM tbl_tnpointi t1, tbl_tnpointinst t2;
SELECT t1.ti #= t2.ti FROM tbl_tnpointi t1, tbl_tnpointi t2;
SELECT t1.ti #= t2.seq FROM tbl_tnpointi t1, tbl_tnpointseq t2;
SELECT t1.ti #= t2.ts FROM tbl_tnpointi t1, tbl_tnpoints t2;

SELECT t1.seq #= t2.np FROM tbl_tnpointseq t1, npoint_tbl t2;
SELECT t1.seq #= t2.inst FROM tbl_tnpointseq t1, tbl_tnpointinst t2;
SELECT t1.seq #= t2.ti FROM tbl_tnpointseq t1, tbl_tnpointi t2;
SELECT t1.seq #= t2.seq FROM tbl_tnpointseq t1, tbl_tnpointseq t2;
SELECT t1.seq #= t2.ts FROM tbl_tnpointseq t1, tbl_tnpoints t2;

SELECT t1.ts #= t2.np FROM tbl_tnpoints t1, npoint_tbl t2;
SELECT t1.ts #= t2.inst FROM tbl_tnpoints t1, tbl_tnpointinst t2;
SELECT t1.ts #= t2.ti FROM tbl_tnpoints t1, tbl_tnpointi t2;
SELECT t1.ts #= t2.seq FROM tbl_tnpoints t1, tbl_tnpointseq t2;
SELECT t1.ts #= t2.ts FROM tbl_tnpoints t1, tbl_tnpoints t2;

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

SELECT t1.np #<> t2.inst FROM npoint_tbl t1, tbl_tnpointinst t2;
SELECT t1.np #<> t2.ti FROM npoint_tbl t1, tbl_tnpointi t2;
SELECT t1.np #<> t2.seq FROM npoint_tbl t1, tbl_tnpointseq t2;
SELECT t1.np #<> t2.ts FROM npoint_tbl t1, tbl_tnpoints t2;

SELECT t1.inst #<> t2.np FROM tbl_tnpointinst t1, npoint_tbl t2;
SELECT t1.inst #<> t2.inst FROM tbl_tnpointinst t1, tbl_tnpointinst t2;
SELECT t1.inst #<> t2.ti FROM tbl_tnpointinst t1, tbl_tnpointi t2;
SELECT t1.inst #<> t2.seq FROM tbl_tnpointinst t1, tbl_tnpointseq t2;
SELECT t1.inst #<> t2.ts FROM tbl_tnpointinst t1, tbl_tnpoints t2;

SELECT t1.ti #<> t2.np FROM tbl_tnpointi t1, npoint_tbl t2;
SELECT t1.ti #<> t2.inst FROM tbl_tnpointi t1, tbl_tnpointinst t2;
SELECT t1.ti #<> t2.ti FROM tbl_tnpointi t1, tbl_tnpointi t2;
SELECT t1.ti #<> t2.seq FROM tbl_tnpointi t1, tbl_tnpointseq t2;
SELECT t1.ti #<> t2.ts FROM tbl_tnpointi t1, tbl_tnpoints t2;

SELECT t1.seq #<> t2.np FROM tbl_tnpointseq t1, npoint_tbl t2;
SELECT t1.seq #<> t2.inst FROM tbl_tnpointseq t1, tbl_tnpointinst t2;
SELECT t1.seq #<> t2.ti FROM tbl_tnpointseq t1, tbl_tnpointi t2;
SELECT t1.seq #<> t2.seq FROM tbl_tnpointseq t1, tbl_tnpointseq t2;
SELECT t1.seq #<> t2.ts FROM tbl_tnpointseq t1, tbl_tnpoints t2;

SELECT t1.ts #<> t2.np FROM tbl_tnpoints t1, npoint_tbl t2;
SELECT t1.ts #<> t2.inst FROM tbl_tnpoints t1, tbl_tnpointinst t2;
SELECT t1.ts #<> t2.ti FROM tbl_tnpoints t1, tbl_tnpointi t2;
SELECT t1.ts #<> t2.seq FROM tbl_tnpoints t1, tbl_tnpointseq t2;
SELECT t1.ts #<> t2.ts FROM tbl_tnpoints t1, tbl_tnpoints t2;
