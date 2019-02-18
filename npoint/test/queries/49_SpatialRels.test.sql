/*****************************************************************************
 * Geometry rel temporal npoint
 *****************************************************************************/

SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE contains(t1.g, t2.inst);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE containsproperly(t1.g, t2.inst);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE covers(t1.g, t2.inst);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE coveredby(t1.g, t2.inst);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE crosses(t1.g, t2.inst);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE disjoint(t1.g, t2.inst);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE equals(t1.g, t2.inst);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE intersects(t1.g, t2.inst);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE overlaps(t1.g, t2.inst);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE touches(t1.g, t2.inst);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE within(t1.g, t2.inst);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE dwithin(t1.g, t2.inst, 0.01);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE relate(t1.g, t2.inst) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE relate(t1.g, t2.inst, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE contains(t1.g, t2.ti);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE containsproperly(t1.g, t2.ti);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE covers(t1.g, t2.ti);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE coveredby(t1.g, t2.ti);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE crosses(t1.g, t2.ti);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE disjoint(t1.g, t2.ti);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE equals(t1.g, t2.ti);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE intersects(t1.g, t2.ti);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE overlaps(t1.g, t2.ti);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE touches(t1.g, t2.ti);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE within(t1.g, t2.ti);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE dwithin(t1.g, t2.ti, 0.01);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE relate(t1.g, t2.ti) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE relate(t1.g, t2.ti, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE contains(t1.g, t2.seq);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE containsproperly(t1.g, t2.seq);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE covers(t1.g, t2.seq);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE coveredby(t1.g, t2.seq);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE crosses(t1.g, t2.seq);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE disjoint(t1.g, t2.seq);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE equals(t1.g, t2.seq);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE intersects(t1.g, t2.seq);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE overlaps(t1.g, t2.seq);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE touches(t1.g, t2.seq);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE within(t1.g, t2.seq);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE dwithin(t1.g, t2.seq, 0.01);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE relate(t1.g, t2.seq) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE relate(t1.g, t2.seq, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE contains(t1.g, t2.ts);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE containsproperly(t1.g, t2.ts);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE covers(t1.g, t2.ts);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE coveredby(t1.g, t2.ts);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE crosses(t1.g, t2.ts);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE disjoint(t1.g, t2.ts);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE equals(t1.g, t2.ts);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE intersects(t1.g, t2.ts);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE overlaps(t1.g, t2.ts);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE touches(t1.g, t2.ts);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE within(t1.g, t2.ts);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE dwithin(t1.g, t2.ts, 0.01);
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE relate(t1.g, t2.ts) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE relate(t1.g, t2.ts, 'T*****FF*');

/*****************************************************************************
 * TNpointInst rel temporal npoint
 *****************************************************************************/

SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE contains(t1.inst, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE containsproperly(t1.inst, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE covers(t1.inst, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE coveredby(t1.inst, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE crosses(t1.inst, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE disjoint(t1.inst, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE equals(t1.inst, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE intersects(t1.inst, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE overlaps(t1.inst, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE touches(t1.inst, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE within(t1.inst, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE dwithin(t1.inst, t2.g, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE relate(t1.inst, t2.g) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE relate(t1.inst, t2.g, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE contains(t1.inst, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE containsproperly(t1.inst, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE covers(t1.inst, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE coveredby(t1.inst, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE crosses(t1.inst, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE disjoint(t1.inst, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE equals(t1.inst, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE intersects(t1.inst, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE overlaps(t1.inst, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE touches(t1.inst, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE within(t1.inst, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE dwithin(t1.inst, t2.inst, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE relate(t1.inst, t2.inst) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE relate(t1.inst, t2.inst, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE contains(t1.inst, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE containsproperly(t1.inst, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE covers(t1.inst, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE coveredby(t1.inst, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE crosses(t1.inst, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE disjoint(t1.inst, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE equals(t1.inst, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE intersects(t1.inst, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE overlaps(t1.inst, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE touches(t1.inst, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE within(t1.inst, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE dwithin(t1.inst, t2.ti, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE relate(t1.inst, t2.ti) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE relate(t1.inst, t2.ti, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE contains(t1.inst, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE containsproperly(t1.inst, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE covers(t1.inst, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE coveredby(t1.inst, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE crosses(t1.inst, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE disjoint(t1.inst, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE equals(t1.inst, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE intersects(t1.inst, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE overlaps(t1.inst, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE touches(t1.inst, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE within(t1.inst, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE dwithin(t1.inst, t2.seq, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE relate(t1.inst, t2.seq) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE relate(t1.inst, t2.seq, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE contains(t1.inst, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE containsproperly(t1.inst, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE covers(t1.inst, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE coveredby(t1.inst, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE crosses(t1.inst, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE disjoint(t1.inst, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE equals(t1.inst, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE intersects(t1.inst, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE overlaps(t1.inst, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE touches(t1.inst, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE within(t1.inst, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE dwithin(t1.inst, t2.ts, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE relate(t1.inst, t2.ts) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE relate(t1.inst, t2.ts, 'T*****FF*');

/*****************************************************************************
 * TNpointI rel temporal npoint
 *****************************************************************************/

SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE contains(t1.ti, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE containsproperly(t1.ti, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE covers(t1.ti, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE coveredby(t1.ti, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE crosses(t1.ti, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE disjoint(t1.ti, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE equals(t1.ti, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE intersects(t1.ti, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE overlaps(t1.ti, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE touches(t1.ti, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE within(t1.ti, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE dwithin(t1.ti, t2.g, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE relate(t1.ti, t2.g) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE relate(t1.ti, t2.g, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE contains(t1.ti, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE containsproperly(t1.ti, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE covers(t1.ti, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE coveredby(t1.ti, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE crosses(t1.ti, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE disjoint(t1.ti, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE equals(t1.ti, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE intersects(t1.ti, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE overlaps(t1.ti, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE touches(t1.ti, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE within(t1.ti, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE dwithin(t1.ti, t2.inst, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE relate(t1.ti, t2.inst) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE relate(t1.ti, t2.inst, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE contains(t1.ti, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE containsproperly(t1.ti, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE covers(t1.ti, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE coveredby(t1.ti, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE crosses(t1.ti, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE disjoint(t1.ti, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE equals(t1.ti, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE intersects(t1.ti, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE overlaps(t1.ti, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE touches(t1.ti, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE within(t1.ti, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE dwithin(t1.ti, t2.ti, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE relate(t1.ti, t2.ti) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE relate(t1.ti, t2.ti, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE contains(t1.ti, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE containsproperly(t1.ti, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE covers(t1.ti, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE coveredby(t1.ti, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE crosses(t1.ti, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE disjoint(t1.ti, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE equals(t1.ti, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE intersects(t1.ti, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE overlaps(t1.ti, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE touches(t1.ti, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE within(t1.ti, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE dwithin(t1.ti, t2.seq, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE relate(t1.ti, t2.seq) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE relate(t1.ti, t2.seq, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE contains(t1.ti, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE containsproperly(t1.ti, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE covers(t1.ti, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE coveredby(t1.ti, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE crosses(t1.ti, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE disjoint(t1.ti, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE equals(t1.ti, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE intersects(t1.ti, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE overlaps(t1.ti, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE touches(t1.ti, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE within(t1.ti, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE dwithin(t1.ti, t2.ts, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE relate(t1.ti, t2.ts) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE relate(t1.ti, t2.ts, 'T*****FF*');

/*****************************************************************************
 * TNpointSeq rel temporal npoint
 *****************************************************************************/

SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE contains(t1.seq, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE containsproperly(t1.seq, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE covers(t1.seq, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE coveredby(t1.seq, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE crosses(t1.seq, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE disjoint(t1.seq, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE equals(t1.seq, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE intersects(t1.seq, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE overlaps(t1.seq, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE touches(t1.seq, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE within(t1.seq, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE dwithin(t1.seq, t2.g, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE relate(t1.seq, t2.g) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE relate(t1.seq, t2.g, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE contains(t1.seq, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE containsproperly(t1.seq, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE covers(t1.seq, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE coveredby(t1.seq, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE crosses(t1.seq, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE disjoint(t1.seq, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE equals(t1.seq, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE intersects(t1.seq, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE overlaps(t1.seq, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE touches(t1.seq, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE within(t1.seq, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE dwithin(t1.seq, t2.inst, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE relate(t1.seq, t2.inst) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE relate(t1.seq, t2.inst, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE contains(t1.seq, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE containsproperly(t1.seq, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE covers(t1.seq, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE coveredby(t1.seq, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE crosses(t1.seq, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE disjoint(t1.seq, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE equals(t1.seq, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE intersects(t1.seq, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE overlaps(t1.seq, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE touches(t1.seq, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE within(t1.seq, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE dwithin(t1.seq, t2.ti, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE relate(t1.seq, t2.ti) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE relate(t1.seq, t2.ti, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE contains(t1.seq, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE containsproperly(t1.seq, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE covers(t1.seq, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE coveredby(t1.seq, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE crosses(t1.seq, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE disjoint(t1.seq, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE equals(t1.seq, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE intersects(t1.seq, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE overlaps(t1.seq, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE touches(t1.seq, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE within(t1.seq, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE dwithin(t1.seq, t2.seq, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE relate(t1.seq, t2.seq) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE relate(t1.seq, t2.seq, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE contains(t1.seq, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE containsproperly(t1.seq, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE covers(t1.seq, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE coveredby(t1.seq, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE crosses(t1.seq, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE disjoint(t1.seq, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE equals(t1.seq, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE intersects(t1.seq, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE overlaps(t1.seq, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE touches(t1.seq, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE within(t1.seq, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE dwithin(t1.seq, t2.ts, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE relate(t1.seq, t2.ts) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE relate(t1.seq, t2.ts, 'T*****FF*');

/*****************************************************************************
 * TNpointS rel temporal npoint
 *****************************************************************************/

SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE contains(t1.ts, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE containsproperly(t1.ts, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE covers(t1.ts, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE coveredby(t1.ts, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE crosses(t1.ts, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE disjoint(t1.ts, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE equals(t1.ts, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE intersects(t1.ts, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE overlaps(t1.ts, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE touches(t1.ts, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE within(t1.ts, t2.g);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE dwithin(t1.ts, t2.g, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE relate(t1.ts, t2.g) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2 WHERE relate(t1.ts, t2.g, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE contains(t1.ts, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE containsproperly(t1.ts, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE covers(t1.ts, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE coveredby(t1.ts, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE crosses(t1.ts, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE disjoint(t1.ts, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE equals(t1.ts, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE intersects(t1.ts, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE overlaps(t1.ts, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE touches(t1.ts, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE within(t1.ts, t2.inst);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE dwithin(t1.ts, t2.inst, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE relate(t1.ts, t2.inst) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2 WHERE relate(t1.ts, t2.inst, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE contains(t1.ts, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE containsproperly(t1.ts, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE covers(t1.ts, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE coveredby(t1.ts, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE crosses(t1.ts, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE disjoint(t1.ts, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE equals(t1.ts, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE intersects(t1.ts, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE overlaps(t1.ts, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE touches(t1.ts, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE within(t1.ts, t2.ti);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE dwithin(t1.ts, t2.ti, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE relate(t1.ts, t2.ti) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2 WHERE relate(t1.ts, t2.ti, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE contains(t1.ts, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE containsproperly(t1.ts, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE covers(t1.ts, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE coveredby(t1.ts, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE crosses(t1.ts, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE disjoint(t1.ts, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE equals(t1.ts, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE intersects(t1.ts, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE overlaps(t1.ts, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE touches(t1.ts, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE within(t1.ts, t2.seq);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE dwithin(t1.ts, t2.seq, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE relate(t1.ts, t2.seq) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2 WHERE relate(t1.ts, t2.seq, 'T*****FF*');

SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE contains(t1.ts, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE containsproperly(t1.ts, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE covers(t1.ts, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE coveredby(t1.ts, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE crosses(t1.ts, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE disjoint(t1.ts, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE equals(t1.ts, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE intersects(t1.ts, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE overlaps(t1.ts, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE touches(t1.ts, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE within(t1.ts, t2.ts);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE dwithin(t1.ts, t2.ts, 0.01);
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE relate(t1.ts, t2.ts) IS NOT NULL;
SELECT count(*) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2 WHERE relate(t1.ts, t2.ts, 'T*****FF*');

/*****************************************************************************/