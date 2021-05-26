-------------------------------------------------------------------------------
set parallel_tuple_cost=0;
set parallel_setup_cost=0;
set force_parallel_mode=regress;
-------------------------------------------------------------------------------
-- Geometry rel tnpoint
 -------------------------------------------------------------------------------

SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE contains(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE containsproperly(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE covers(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE coveredby(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE crosses(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE disjoint(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE equals(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE intersects(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE overlaps(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE touches(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE within(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE dwithin(ST_SetSRID(t1.g, 5676), t2.temp, 0.01) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE relate(ST_SetSRID(t1.g, 5676), t2.temp) IS NOT NULL AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_geompoint t1, tbl_tnpoint t2 WHERE relate(ST_SetSRID(t1.g, 5676), t2.temp, 'T*****FF*') AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';

-------------------------------------------------------------------------------
-- npoint rel tnpoint
 -------------------------------------------------------------------------------

SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE contains(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE containsproperly(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE covers(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE coveredby(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE crosses(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE disjoint(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE equals(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE intersects(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE overlaps(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE touches(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE within(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE dwithin(t1.np, t2.temp, 0.01) AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE relate(t1.np, t2.temp) IS NOT NULL AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE relate(t1.np, t2.temp, 'T*****FF*') AND t1.k < 10 AND t2.k%4 = 0 AND duration(temp) != 'SequenceSet';

-------------------------------------------------------------------------------
-- tnpoint rel <type>
 -------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE contains(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE containsproperly(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE covers(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE coveredby(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE crosses(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE disjoint(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE equals(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE intersects(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE overlaps(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE touches(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE within(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE dwithin(t1.temp, ST_SetSRID(t2.g, 5676), 0.01) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE relate(t1.temp, ST_SetSRID(t2.g, 5676)) IS NOT NULL AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_geompoint t2 WHERE relate(t1.temp, ST_SetSRID(t2.g, 5676), 'T*****FF*') AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';

SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE contains(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE containsproperly(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE covers(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE coveredby(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE crosses(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE disjoint(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE equals(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE intersects(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE overlaps(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE touches(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE within(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE dwithin(t1.temp, t2.np, 0.01) AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE relate(t1.temp, t2.np) IS NOT NULL AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE relate(t1.temp, t2.np, 'T*****FF*') AND t1.k%4 = 0 AND t2.k < 10 AND duration(temp) != 'SequenceSet';

SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE contains(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE containsproperly(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE covers(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE coveredby(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE crosses(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE disjoint(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE equals(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE intersects(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE overlaps(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE touches(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE within(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE dwithin(t1.temp, t2.temp, 0.01) AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE relate(t1.temp, t2.temp) IS NOT NULL AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';
SELECT count(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE relate(t1.temp, t2.temp, 'T*****FF*') AND t1.k%4 = 0 AND t2.k%4 = 0 AND duration(t1.temp) != 'SequenceSet';

-------------------------------------------------------------------------------
set parallel_tuple_cost=100;
set parallel_setup_cost=100;
set force_parallel_mode=off;
-------------------------------------------------------------------------------
