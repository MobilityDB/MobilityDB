/*****************************************************************************
 * Geometry rel temporal npoint
 *****************************************************************************/

SELECT tcontains(t1.g, t2.inst) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tcovers(t1.g, t2.inst) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tcoveredby(t1.g, t2.inst) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tdisjoint(t1.g, t2.inst) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tequals(t1.g, t2.inst) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tintersects(t1.g, t2.inst) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT ttouches(t1.g, t2.inst) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT twithin(t1.g, t2.inst) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tdwithin(t1.g, t2.inst, 0.01) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT trelate(t1.g, t2.inst) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT trelate(t1.g, t2.inst, 'T*****FF*') FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;

SELECT tcontains(t1.g, t2.ti) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tcovers(t1.g, t2.ti) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tcoveredby(t1.g, t2.ti) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tdisjoint(t1.g, t2.ti) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tequals(t1.g, t2.ti) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tintersects(t1.g, t2.ti) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT ttouches(t1.g, t2.ti) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT twithin(t1.g, t2.ti) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tdwithin(t1.g, t2.ti, 0.01) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT trelate(t1.g, t2.ti) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT trelate(t1.g, t2.ti, 'T*****FF*') FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;

SELECT tcontains(t1.g, t2.seq) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tcovers(t1.g, t2.seq) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tcoveredby(t1.g, t2.seq) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tdisjoint(t1.g, t2.seq) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tequals(t1.g, t2.seq) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tintersects(t1.g, t2.seq) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT ttouches(t1.g, t2.seq) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT twithin(t1.g, t2.seq) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tdwithin(t1.g, t2.seq, 0.01) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT trelate(t1.g, t2.seq) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT trelate(t1.g, t2.seq, 'T*****FF*') FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;

SELECT tcontains(t1.g, t2.ts) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tcovers(t1.g, t2.ts) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tcoveredby(t1.g, t2.ts) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tdisjoint(t1.g, t2.ts) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tequals(t1.g, t2.ts) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tintersects(t1.g, t2.ts) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT ttouches(t1.g, t2.ts) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT twithin(t1.g, t2.ts) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tdwithin(t1.g, t2.ts, 0.01) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT trelate(t1.g, t2.ts) FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT trelate(t1.g, t2.ts, 'T*****FF*') FROM (SELECT * FROM geompoint_tbl LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;

/*****************************************************************************
 * TNpointInst rel temporal npoint
 *****************************************************************************/

SELECT tcontains(t1.inst, t2.g) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tcovers(t1.inst, t2.g) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tcoveredby(t1.inst, t2.g) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tdisjoint(t1.inst, t2.g) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tequals(t1.inst, t2.g) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tintersects(t1.inst, t2.g) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT ttouches(t1.inst, t2.g) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT twithin(t1.inst, t2.g) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tdwithin(t1.inst, t2.g, 0.01) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT trelate(t1.inst, t2.g) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT trelate(t1.inst, t2.g, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;

SELECT tcontains(t1.inst, t2.inst) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tcovers(t1.inst, t2.inst) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tcoveredby(t1.inst, t2.inst) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tdisjoint(t1.inst, t2.inst) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tequals(t1.inst, t2.inst) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tintersects(t1.inst, t2.inst) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT ttouches(t1.inst, t2.inst) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT twithin(t1.inst, t2.inst) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tdwithin(t1.inst, t2.inst, 0.01) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT trelate(t1.inst, t2.inst) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT trelate(t1.inst, t2.inst, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;

SELECT tcontains(t1.inst, t2.ti) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tcovers(t1.inst, t2.ti) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tcoveredby(t1.inst, t2.ti) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tdisjoint(t1.inst, t2.ti) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tequals(t1.inst, t2.ti) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tintersects(t1.inst, t2.ti) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT ttouches(t1.inst, t2.ti) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT twithin(t1.inst, t2.ti) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tdwithin(t1.inst, t2.ti, 0.01) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT trelate(t1.inst, t2.ti) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT trelate(t1.inst, t2.ti, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;

SELECT tcontains(t1.inst, t2.seq) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tcovers(t1.inst, t2.seq) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tcoveredby(t1.inst, t2.seq) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tdisjoint(t1.inst, t2.seq) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tequals(t1.inst, t2.seq) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tintersects(t1.inst, t2.seq) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT ttouches(t1.inst, t2.seq) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT twithin(t1.inst, t2.seq) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tdwithin(t1.inst, t2.seq, 0.01) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT trelate(t1.inst, t2.seq) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT trelate(t1.inst, t2.seq, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;

SELECT tcontains(t1.inst, t2.ts) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tcovers(t1.inst, t2.ts) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tcoveredby(t1.inst, t2.ts) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tdisjoint(t1.inst, t2.ts) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tequals(t1.inst, t2.ts) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tintersects(t1.inst, t2.ts) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT ttouches(t1.inst, t2.ts) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT twithin(t1.inst, t2.ts) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tdwithin(t1.inst, t2.ts, 0.01) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT trelate(t1.inst, t2.ts) FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT trelate(t1.inst, t2.ts, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointinst LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;

/*****************************************************************************
 * TNpointI rel temporal npoint
 *****************************************************************************/

SELECT tcontains(t1.ti, t2.g) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tcovers(t1.ti, t2.g) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tcoveredby(t1.ti, t2.g) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tdisjoint(t1.ti, t2.g) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tequals(t1.ti, t2.g) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tintersects(t1.ti, t2.g) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT ttouches(t1.ti, t2.g) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT twithin(t1.ti, t2.g) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tdwithin(t1.ti, t2.g, 0.01) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT trelate(t1.ti, t2.g) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT trelate(t1.ti, t2.g, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;

SELECT tcontains(t1.ti, t2.inst) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tcovers(t1.ti, t2.inst) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tcoveredby(t1.ti, t2.inst) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tdisjoint(t1.ti, t2.inst) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tequals(t1.ti, t2.inst) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tintersects(t1.ti, t2.inst) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT ttouches(t1.ti, t2.inst) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT twithin(t1.ti, t2.inst) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tdwithin(t1.ti, t2.inst, 0.01) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT trelate(t1.ti, t2.inst) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT trelate(t1.ti, t2.inst, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;

SELECT tcontains(t1.ti, t2.ti) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tcovers(t1.ti, t2.ti) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tcoveredby(t1.ti, t2.ti) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tdisjoint(t1.ti, t2.ti) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tequals(t1.ti, t2.ti) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tintersects(t1.ti, t2.ti) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT ttouches(t1.ti, t2.ti) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT twithin(t1.ti, t2.ti) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tdwithin(t1.ti, t2.ti, 0.01) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT trelate(t1.ti, t2.ti) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT trelate(t1.ti, t2.ti, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;

SELECT tcontains(t1.ti, t2.seq) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tcovers(t1.ti, t2.seq) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tcoveredby(t1.ti, t2.seq) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tdisjoint(t1.ti, t2.seq) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tequals(t1.ti, t2.seq) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tintersects(t1.ti, t2.seq) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT ttouches(t1.ti, t2.seq) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT twithin(t1.ti, t2.seq) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tdwithin(t1.ti, t2.seq, 0.01) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT trelate(t1.ti, t2.seq) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT trelate(t1.ti, t2.seq, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;

SELECT tcontains(t1.ti, t2.ts) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tcovers(t1.ti, t2.ts) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tcoveredby(t1.ti, t2.ts) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tdisjoint(t1.ti, t2.ts) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tequals(t1.ti, t2.ts) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tintersects(t1.ti, t2.ts) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT ttouches(t1.ti, t2.ts) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT twithin(t1.ti, t2.ts) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tdwithin(t1.ti, t2.ts, 0.01) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT trelate(t1.ti, t2.ts) FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT trelate(t1.ti, t2.ts, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointi LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;

/*****************************************************************************
 * TNpointSeq rel temporal npoint
 *****************************************************************************/

SELECT tcontains(t1.seq, t2.g) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tcovers(t1.seq, t2.g) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tcoveredby(t1.seq, t2.g) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tdisjoint(t1.seq, t2.g) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tequals(t1.seq, t2.g) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tintersects(t1.seq, t2.g) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT ttouches(t1.seq, t2.g) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT twithin(t1.seq, t2.g) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tdwithin(t1.seq, t2.g, 0.01) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT trelate(t1.seq, t2.g) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT trelate(t1.seq, t2.g, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;

SELECT tcontains(t1.seq, t2.inst) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tcovers(t1.seq, t2.inst) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tcoveredby(t1.seq, t2.inst) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tdisjoint(t1.seq, t2.inst) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tequals(t1.seq, t2.inst) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tintersects(t1.seq, t2.inst) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT ttouches(t1.seq, t2.inst) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT twithin(t1.seq, t2.inst) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tdwithin(t1.seq, t2.inst, 0.01) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT trelate(t1.seq, t2.inst) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT trelate(t1.seq, t2.inst, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;

SELECT tcontains(t1.seq, t2.ti) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tcovers(t1.seq, t2.ti) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tcoveredby(t1.seq, t2.ti) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tdisjoint(t1.seq, t2.ti) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tequals(t1.seq, t2.ti) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tintersects(t1.seq, t2.ti) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT ttouches(t1.seq, t2.ti) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT twithin(t1.seq, t2.ti) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tdwithin(t1.seq, t2.ti, 0.01) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT trelate(t1.seq, t2.ti) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT trelate(t1.seq, t2.ti, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;

SELECT tcontains(t1.seq, t2.seq) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tcovers(t1.seq, t2.seq) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tcoveredby(t1.seq, t2.seq) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tdisjoint(t1.seq, t2.seq) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tequals(t1.seq, t2.seq) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tintersects(t1.seq, t2.seq) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT ttouches(t1.seq, t2.seq) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT twithin(t1.seq, t2.seq) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tdwithin(t1.seq, t2.seq, 0.01) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT trelate(t1.seq, t2.seq) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT trelate(t1.seq, t2.seq, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;

SELECT tcontains(t1.seq, t2.ts) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tcovers(t1.seq, t2.ts) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tcoveredby(t1.seq, t2.ts) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tdisjoint(t1.seq, t2.ts) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tequals(t1.seq, t2.ts) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tintersects(t1.seq, t2.ts) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT ttouches(t1.seq, t2.ts) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT twithin(t1.seq, t2.ts) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tdwithin(t1.seq, t2.ts, 0.01) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT trelate(t1.seq, t2.ts) FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT trelate(t1.seq, t2.ts, 'T*****FF*') FROM (SELECT * FROM tbl_tnpointseq LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;

/*****************************************************************************
 * TNpointS rel temporal npoints
 *****************************************************************************/

SELECT tcontains(t1.ts, t2.g) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tcovers(t1.ts, t2.g) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tcoveredby(t1.ts, t2.g) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tdisjoint(t1.ts, t2.g) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tequals(t1.ts, t2.g) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tintersects(t1.ts, t2.g) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT ttouches(t1.ts, t2.g) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT twithin(t1.ts, t2.g) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT tdwithin(t1.ts, t2.g, 0.01) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT trelate(t1.ts, t2.g) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;
SELECT trelate(t1.ts, t2.g, 'T*****FF*') FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM geompoint_tbl LIMIT 100) t2;

SELECT tcontains(t1.ts, t2.inst) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tcovers(t1.ts, t2.inst) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tcoveredby(t1.ts, t2.inst) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tdisjoint(t1.ts, t2.inst) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tequals(t1.ts, t2.inst) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tintersects(t1.ts, t2.inst) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT ttouches(t1.ts, t2.inst) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT twithin(t1.ts, t2.inst) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT tdwithin(t1.ts, t2.inst, 0.01) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT trelate(t1.ts, t2.inst) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;
SELECT trelate(t1.ts, t2.inst, 'T*****FF*') FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointinst LIMIT 100) t2;

SELECT tcontains(t1.ts, t2.ti) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tcovers(t1.ts, t2.ti) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tcoveredby(t1.ts, t2.ti) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tdisjoint(t1.ts, t2.ti) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tequals(t1.ts, t2.ti) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tintersects(t1.ts, t2.ti) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT ttouches(t1.ts, t2.ti) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT twithin(t1.ts, t2.ti) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT tdwithin(t1.ts, t2.ti, 0.01) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT trelate(t1.ts, t2.ti) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;
SELECT trelate(t1.ts, t2.ti, 'T*****FF*') FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointi LIMIT 100) t2;

SELECT tcontains(t1.ts, t2.seq) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tcovers(t1.ts, t2.seq) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tcoveredby(t1.ts, t2.seq) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tdisjoint(t1.ts, t2.seq) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tequals(t1.ts, t2.seq) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tintersects(t1.ts, t2.seq) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT ttouches(t1.ts, t2.seq) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT twithin(t1.ts, t2.seq) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT tdwithin(t1.ts, t2.seq, 0.01) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT trelate(t1.ts, t2.seq) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;
SELECT trelate(t1.ts, t2.seq, 'T*****FF*') FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpointseq LIMIT 100) t2;

SELECT tcontains(t1.ts, t2.ts) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tcovers(t1.ts, t2.ts) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tcoveredby(t1.ts, t2.ts) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tdisjoint(t1.ts, t2.ts) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tequals(t1.ts, t2.ts) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tintersects(t1.ts, t2.ts) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT ttouches(t1.ts, t2.ts) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT twithin(t1.ts, t2.ts) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT tdwithin(t1.ts, t2.ts, 0.01) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT trelate(t1.ts, t2.ts) FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;
SELECT trelate(t1.ts, t2.ts, 'T*****FF*') FROM (SELECT * FROM tbl_tnpoints LIMIT 100) t1, (SELECT * FROM tbl_tnpoints LIMIT 100) t2;

/*****************************************************************************/
