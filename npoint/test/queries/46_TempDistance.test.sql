SELECT t1.g <-> t2.inst FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT t1.g <-> t2.ti FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT t1.g <-> t2.seq FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT t1.g <-> t2.ts FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;

SELECT t1.inst <-> t2.g FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT t1.inst <-> t2.inst FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT t1.inst <-> t2.ti FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT t1.inst <-> t2.seq FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT t1.inst <-> t2.ts FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;

SELECT t1.ti <-> t2.g FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT t1.ti <-> t2.inst FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT t1.ti <-> t2.ti FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT t1.ti <-> t2.seq FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT t1.ti <-> t2.ts FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;

SELECT t1.seq <-> t2.g FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT t1.seq <-> t2.inst FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT t1.seq <-> t2.ti FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT t1.seq <-> t2.seq FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT t1.seq <-> t2.ts FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;

SELECT t1.ts <-> t2.g FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT t1.ts <-> t2.inst FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT t1.ts <-> t2.ti FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT t1.ts <-> t2.seq FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT t1.ts <-> t2.ts FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;

/*****************************************************************************/
