-- GiST index test: build the index, run a few box-overlap queries
-- and verify the answers match a sequential-scan plan.

-------------------------------------------------------------------------------
-- tpcpoint
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_gist_idx ON tbl_tpcpoint USING gist(temp);

-- Index plan should match sequential scan for the same query.
SET enable_seqscan = on;
SET enable_indexscan = off;
SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_count FROM tbl_tpcpoint
WHERE temp && tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1);

SET enable_seqscan = off;
SET enable_indexscan = on;
SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_count FROM tbl_tpcpoint
WHERE temp && tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1);

DROP INDEX tbl_tpcpoint_gist_idx;
RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;

-------------------------------------------------------------------------------
-- tpcpatch
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_gist_idx ON tbl_tpcpatch USING gist(temp);

SET enable_seqscan = on;
SET enable_indexscan = off;
SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_count FROM tbl_tpcpatch
WHERE temp && tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1);

SET enable_seqscan = off;
SET enable_indexscan = on;
SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_count FROM tbl_tpcpatch
WHERE temp && tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1);

DROP INDEX tbl_tpcpatch_gist_idx;
RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;

-------------------------------------------------------------------------------
-- Operator-family coverage. For each operator the seqscan and indexed
-- counts must match. This locks index correctness across the kinds of
-- queries a real drone-LiDAR workload pivots on.
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_gist_idx ON tbl_tpcpatch USING gist(temp);

-- &&  : bbox overlap (already covered above; repeated for symmetry).
-- <@  : value's bbox contained in the query bbox.
SET enable_seqscan = on;  SET enable_indexscan = off; SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_contained FROM tbl_tpcpatch
WHERE temp <@ tpcboxZT(-1000, -1000, -1000, 1000, 1000, 1000,
  tstzspan '[2001-01-01, 2002-01-01]', 1);
SET enable_seqscan = off; SET enable_indexscan = on; SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_contained FROM tbl_tpcpatch
WHERE temp <@ tpcboxZT(-1000, -1000, -1000, 1000, 1000, 1000,
  tstzspan '[2001-01-01, 2002-01-01]', 1);

-- <<# : strictly before (time)
SET enable_seqscan = on;  SET enable_indexscan = off; SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_before FROM tbl_tpcpatch
WHERE temp <<# tpcboxZT(0, 0, 0, 100, 100, 100,
  tstzspan '[2001-12-15, 2002-01-01]', 1);
SET enable_seqscan = off; SET enable_indexscan = on; SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_before FROM tbl_tpcpatch
WHERE temp <<# tpcboxZT(0, 0, 0, 100, 100, 100,
  tstzspan '[2001-12-15, 2002-01-01]', 1);

-- << : strictly left (X)
SET enable_seqscan = on;  SET enable_indexscan = off; SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_left FROM tbl_tpcpatch
WHERE temp << tpcboxZT(50, -1000, -1000, 1000, 1000, 1000,
  tstzspan '[2001-01-01, 2002-01-01]', 1);
SET enable_seqscan = off; SET enable_indexscan = on; SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_left FROM tbl_tpcpatch
WHERE temp << tpcboxZT(50, -1000, -1000, 1000, 1000, 1000,
  tstzspan '[2001-01-01, 2002-01-01]', 1);

DROP INDEX tbl_tpcpatch_gist_idx;
RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;

-------------------------------------------------------------------------------
-- Query arguments other than a tpcbox. The opclass declares members against
-- tstzspan and against the temporal type itself, so the consistent method
-- meets those types as the query and the counts it produces must equal the
-- ones a scan produces.
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_gist_idx ON tbl_tpcpoint USING gist(temp);
CREATE INDEX tbl_tpcpatch_gist_idx ON tbl_tpcpatch USING gist(temp);

SET enable_seqscan = on;  SET enable_indexscan = off; SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_span FROM tbl_tpcpoint
WHERE temp && tstzspan '[2001-06-01, 2001-12-31]';
SELECT COUNT(*) AS seq_span_patch FROM tbl_tpcpatch
WHERE temp && tstzspan '[2001-06-01, 2001-12-31]';
SELECT COUNT(*) AS seq_contained_span FROM tbl_tpcpoint
WHERE temp <@ tstzspan '[2001-01-01, 2002-01-01]';
SELECT COUNT(*) AS seq_self FROM tbl_tpcpoint t1
WHERE t1.temp && (SELECT temp FROM tbl_tpcpoint WHERE temp IS NOT NULL ORDER BY k LIMIT 1);

SET enable_seqscan = off; SET enable_indexscan = on; SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_span FROM tbl_tpcpoint
WHERE temp && tstzspan '[2001-06-01, 2001-12-31]';
SELECT COUNT(*) AS idx_span_patch FROM tbl_tpcpatch
WHERE temp && tstzspan '[2001-06-01, 2001-12-31]';
SELECT COUNT(*) AS idx_contained_span FROM tbl_tpcpoint
WHERE temp <@ tstzspan '[2001-01-01, 2002-01-01]';
SELECT COUNT(*) AS idx_self FROM tbl_tpcpoint t1
WHERE t1.temp && (SELECT temp FROM tbl_tpcpoint WHERE temp IS NOT NULL ORDER BY k LIMIT 1);

DROP INDEX tbl_tpcpoint_gist_idx;
DROP INDEX tbl_tpcpatch_gist_idx;
RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;

-------------------------------------------------------------------------------
-- Functional call form. The planner support function rewrites a call such as
-- overlaps(temp, box) into the indexable temp && box, so the counts a scan
-- produces and the counts an index produces must agree for the call form
-- exactly as they do for the operator form.
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_gist_idx ON tbl_tpcpoint USING gist(temp);
CREATE INDEX tbl_tpcpatch_gist_idx ON tbl_tpcpatch USING gist(temp);

SET enable_seqscan = on;  SET enable_indexscan = off; SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_fn_box FROM tbl_tpcpoint
WHERE tpcboxOverlaps(temp, tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1));
SELECT COUNT(*) AS seq_fn_span FROM tbl_tpcpoint
WHERE tpcboxOverlaps(temp, tstzspan '[2001-06-01, 2001-12-31]');
SELECT COUNT(*) AS seq_fn_patch FROM tbl_tpcpatch
WHERE tpcboxOverlaps(temp, tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1));
SELECT COUNT(*) AS seq_fn_left FROM tbl_tpcpoint
WHERE tpcboxLeft(temp, tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1));
SELECT COUNT(*) AS seq_fn_overfront FROM tbl_tpcpoint
WHERE tpcboxOverfront(temp, tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1));
SELECT COUNT(*) AS seq_fn_before FROM tbl_tpcpoint
WHERE tpcboxBefore(temp, tstzspan '[2001-06-01, 2001-12-31]');
SELECT COUNT(*) AS seq_fn_patch_overbelow FROM tbl_tpcpatch
WHERE tpcboxOverbelow(temp, tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1));

SET enable_seqscan = off; SET enable_indexscan = on; SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_fn_box FROM tbl_tpcpoint
WHERE tpcboxOverlaps(temp, tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1));
SELECT COUNT(*) AS idx_fn_span FROM tbl_tpcpoint
WHERE tpcboxOverlaps(temp, tstzspan '[2001-06-01, 2001-12-31]');
SELECT COUNT(*) AS idx_fn_patch FROM tbl_tpcpatch
WHERE tpcboxOverlaps(temp, tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1));
SELECT COUNT(*) AS idx_fn_left FROM tbl_tpcpoint
WHERE tpcboxLeft(temp, tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1));
SELECT COUNT(*) AS idx_fn_overfront FROM tbl_tpcpoint
WHERE tpcboxOverfront(temp, tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1));
SELECT COUNT(*) AS idx_fn_before FROM tbl_tpcpoint
WHERE tpcboxBefore(temp, tstzspan '[2001-06-01, 2001-12-31]');
SELECT COUNT(*) AS idx_fn_patch_overbelow FROM tbl_tpcpatch
WHERE tpcboxOverbelow(temp, tpcboxZT(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1));

-- The plan of a call of a topological or a position function probes the index,
-- the support function of the call rewriting it into its operator
CREATE FUNCTION test_probes_index(query text)
RETURNS boolean LANGUAGE plpgsql AS $$
DECLARE
  line text;
BEGIN
  FOR line IN EXECUTE 'EXPLAIN (COSTS OFF) ' || query LOOP
    IF line LIKE '%Index Cond%' THEN
      RETURN true;
    END IF;
  END LOOP;
  RETURN false;
END;
$$;

SELECT test_probes_index('SELECT * FROM tbl_tpcpoint WHERE tpcboxOverlaps(temp, '
  'tpcboxZT(0, 0, 0, 50, 50, 50, tstzspan ''[2001-06-01, 2001-12-31]'', 1))') AS fn_overlaps,
  test_probes_index('SELECT * FROM tbl_tpcpoint WHERE tpcboxLeft(temp, '
  'tpcboxZT(0, 0, 0, 50, 50, 50, tstzspan ''[2001-06-01, 2001-12-31]'', 1))') AS fn_left,
  test_probes_index('SELECT * FROM tbl_tpcpoint WHERE tpcboxOverfront(temp, '
  'tpcboxZT(0, 0, 0, 50, 50, 50, tstzspan ''[2001-06-01, 2001-12-31]'', 1))') AS fn_overfront,
  test_probes_index('SELECT * FROM tbl_tpcpoint WHERE tpcboxBefore(temp, '
  'tstzspan ''[2001-06-01, 2001-12-31]'')') AS fn_before,
  test_probes_index('SELECT * FROM tbl_tpcpatch WHERE tpcboxOverbelow(temp, '
  'tpcboxZT(0, 0, 0, 50, 50, 50, tstzspan ''[2001-06-01, 2001-12-31]'', 1))') AS fn_patch_overbelow;

DROP FUNCTION test_probes_index(text);

DROP INDEX tbl_tpcpoint_gist_idx;
DROP INDEX tbl_tpcpatch_gist_idx;
RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;

-- typanalyze: ANALYZE must succeed on the temporal pointcloud columns; tspatial_analyze
-- projects each TPCBox bounding box to an STBox for the spatial statistics
ANALYZE tbl_tpcpoint;
ANALYZE tbl_tpcpatch;

-------------------------------------------------------------------------------
-- An index over boxes of several structures answers as the sequential scan:
-- the same count, or an error where the scan raises one, for every operator
-- and every query, and the same 25 nearest distances for |=|. Where the boxes
-- of several structures raise different errors, the error is the one of the
-- first box a plan meets, which no plan orders. Per operator class: whether
-- the plan reads the index, how many answers are errors, and how many differ
-- from the scan
-------------------------------------------------------------------------------

CREATE FUNCTION index_structures(tbl text, typ text, opclasses text[],
  queries text[], ops text[])
RETURNS TABLE(opclass text, uses_index boolean, errors int,
  disagreements int) AS $$
DECLARE
  a text; o text; q text; r bigint; ans text; line text;
  seqans jsonb := '{}';
BEGIN
  FOREACH a IN ARRAY array_prepend('seq', opclasses) LOOP
    EXECUTE format('DROP INDEX IF EXISTS %I', tbl || '_idx');
    IF a = 'seq' THEN
      SET LOCAL enable_seqscan = on;
      SET LOCAL enable_indexscan = off;
      SET LOCAL enable_bitmapscan = off;
    ELSE
      EXECUTE format('CREATE INDEX %I ON %I USING %s', tbl || '_idx', tbl, a);
      SET LOCAL enable_seqscan = off;
      SET LOCAL enable_indexscan = on;
      SET LOCAL enable_bitmapscan = on;
    END IF;
    opclass := a; uses_index := false; errors := 0; disagreements := 0;
    FOR line IN EXECUTE format('EXPLAIN SELECT count(*) FROM %I WHERE b && %L::%s',
      tbl, queries[1], typ) LOOP
      uses_index := uses_index OR line LIKE '%' || tbl || '_idx%';
    END LOOP;
    FOREACH o IN ARRAY ops LOOP
      FOREACH q IN ARRAY queries LOOP
        BEGIN
          IF o = '|=|' THEN
            EXECUTE format('SELECT string_agg(coalesce(round(d::numeric, 6)::text, '
              '''null''), '','' ORDER BY d) FROM (SELECT b |=| %L::%s AS d '
              'FROM %I ORDER BY b |=| %L::%s LIMIT 25) s', q, typ, tbl, q, typ)
              INTO ans;
          ELSE
            EXECUTE format('SELECT count(*) FROM %I WHERE b %s %L::%s',
              tbl, o, q, typ) INTO r;
            ans := r::text;
          END IF;
        EXCEPTION WHEN OTHERS THEN
          ans := 'ERROR: ' || SQLERRM;
          errors := errors + 1;
        END;
        IF a = 'seq' THEN
          seqans := seqans || jsonb_build_object(o || ' ' || q, ans);
        ELSIF seqans ->> (o || ' ' || q) IS DISTINCT FROM ans AND
            NOT (seqans ->> (o || ' ' || q) LIKE 'ERROR: %' AND
              ans LIKE 'ERROR: %') THEN
          disagreements := disagreements + 1;
        END IF;
      END LOOP;
    END LOOP;
    RETURN NEXT;
  END LOOP;
  EXECUTE format('DROP INDEX IF EXISTS %I', tbl || '_idx');
END;
$$ LANGUAGE plpgsql;

CREATE FUNCTION day_of(d int) RETURNS timestamptz AS $$
  SELECT timestamptz '2001-01-01' + d * interval '1 day'
$$ LANGUAGE sql IMMUTABLE;

CREATE TABLE tbl_index_structures AS
  SELECT k, CASE k % 4
    WHEN 0 THEN tpcboxXT(k % 50, k % 47, k % 50 + 2, k % 47 + 2,
      span(day_of(k % 40), day_of(k % 40 + 2)), 1)
    WHEN 1 THEN tpcboxX(k % 50, k % 47, k % 50 + 2, k % 47 + 2, 1)
    WHEN 2 THEN tpcboxZT(k % 50, k % 47, k % 43, k % 50 + 2, k % 47 + 2,
      k % 43 + 2, span(day_of(k % 40), day_of(k % 40 + 2)), 1)
    ELSE tpcboxT(span(day_of(k % 40), day_of(k % 40 + 2)), 1) END AS b
  FROM generate_series(1, 4000) AS k;

SELECT * FROM index_structures('tbl_index_structures', 'tpcbox',
  ARRAY['gist(b tpcbox_rtree_ops)', 'spgist(b tpcbox_quadtree_ops)',
    'spgist(b tpcbox_kdtree_ops)'],
  ARRAY[tpcboxXT(10, 10, 20, 20, span(day_of(9), day_of(19)), 1)::text,
    tpcboxX(10, 10, 20, 20, 1)::text,
    tpcboxZT(10, 10, 10, 20, 20, 20, span(day_of(9), day_of(19)), 1)::text,
    tpcboxT(span(day_of(9), day_of(19)), 1)::text],
  ARRAY['&&', '@>', '<@', '~=', '-|-', '<<', '&<', '>>', '&>', '<<|', '&<|',
    '|>>', '|&>', '<</', '&</', '/>>', '/&>', '<<#', '&<#', '#>>', '#&>',
    '|=|']);

DROP TABLE tbl_index_structures;
DROP FUNCTION day_of(int);
DROP FUNCTION index_structures(text, text, text[], text[], text[]);

-------------------------------------------------------------------------------

-- The distance a GiST index over temporal point cloud points reports for an
-- entry stays under the one |=| computes on the trajectories of the values,
-- so the nearest values of points at large coordinates come out of the index
-- as out of a scan
CREATE TABLE tbl_tpcpoint_far AS
SELECT k, tpcpoint(pcpoint(1, 19000000 + (k * 37 % 101) * 0.37,
  19000000 + (k * 53 % 97) * 0.29, 19000000 + (k * 71 % 89) * 0.31),
  timestamptz '2001-01-01') AS temp
FROM generate_series(1, 200) k;
CREATE INDEX tbl_tpcpoint_far_gist_idx ON tbl_tpcpoint_far USING gist(temp);
SET enable_indexscan = off;
SET enable_bitmapscan = off;
SELECT string_agg(round(d::numeric, 6)::text, ',' ORDER BY d) FROM (
  SELECT temp |=| tpcpoint(pcpoint(1, 19000017.5, 19000013.25, 19000013.75),
    timestamptz '2001-01-01') AS d
  FROM tbl_tpcpoint_far ORDER BY 1 LIMIT 10) s;
SET enable_seqscan = off;
SET enable_indexscan = on;
SELECT string_agg(round(d::numeric, 6)::text, ',' ORDER BY d) FROM (
  SELECT temp |=| tpcpoint(pcpoint(1, 19000017.5, 19000013.25, 19000013.75),
    timestamptz '2001-01-01') AS d
  FROM tbl_tpcpoint_far ORDER BY 1 LIMIT 10) s;
RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;
DROP TABLE tbl_tpcpoint_far;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- A column of tpcbox values indexed by GiST, quad-tree and k-d tree answers the
-- queries of a tpcpoint as the scan does: <<, &<, &&, &>, >>, ~=, @>, <@, &<|, <<|, |>>, |&>, -|-, &<#, <<#, #>>, #&>, &</, <</, />>, /&>, |=|
-------------------------------------------------------------------------------

CREATE TABLE tbl_tpcbox_tpcpoint AS
SELECT k, temp::tpcbox AS b FROM tbl_tpcpoint WHERE temp IS NOT NULL;
CREATE TABLE tbl_tpcpoint_q AS
SELECT temp AS q FROM tbl_tpcpoint WHERE temp IS NOT NULL ORDER BY k LIMIT 3;
CREATE FUNCTION tpcbox_tpcpoint_answers() RETURNS text AS $$
  SELECT string_agg(coalesce(c::text, 'null'), ',' ORDER BY o) FROM (
    SELECT 1, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b << q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 2, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b &< q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 3, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b && q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 4, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b &> q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 5, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b >> q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 6, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b ~= q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 7, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b @> q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 8, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b <@ q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 9, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b &<| q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 10, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b <<| q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 11, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b |>> q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 12, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b |&> q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 13, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b -|- q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 14, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b &<# q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 15, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b <<# q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 16, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b #>> q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 17, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b #&> q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 18, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b &</ q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 19, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b <</ q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 20, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b />> q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 21, (SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b /&> q) FROM tbl_tpcpoint_q
    UNION ALL
    SELECT 22, (SELECT sum(k) FROM (SELECT k FROM tbl_tpcbox_tpcpoint
      ORDER BY b |=| q, k LIMIT 5) s) FROM tbl_tpcpoint_q) t(o, c);
$$ LANGUAGE SQL;
CREATE FUNCTION tpcbox_tpcpoint_probes_index(query text)
RETURNS boolean LANGUAGE plpgsql AS $$
DECLARE
  line text;
BEGIN
  FOR line IN EXECUTE 'EXPLAIN (COSTS OFF) ' || query LOOP
    IF line LIKE '%Index Cond%' OR line LIKE '%Order By%' THEN
      RETURN true;
    END IF;
  END LOOP;
  RETURN false;
END;
$$;
CREATE FUNCTION tpcbox_tpcpoint_indexed() RETURNS text AS $$
  SELECT string_agg(CASE WHEN tpcbox_tpcpoint_probes_index(query) THEN 't' ELSE 'f' END, '' ORDER BY o)
  FROM (VALUES
    (1, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b << (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (2, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b &< (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (3, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b && (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (4, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b &> (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (5, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b >> (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (6, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b ~= (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (7, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b @> (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (8, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b <@ (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (9, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b &<| (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (10, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b <<| (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (11, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b |>> (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (12, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b |&> (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (13, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b -|- (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (14, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b &<# (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (15, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b <<# (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (16, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b #>> (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (17, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b #&> (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (18, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b &</ (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (19, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b <</ (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (20, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b />> (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (21, 'SELECT count(*) FROM tbl_tpcbox_tpcpoint WHERE b /&> (SELECT q FROM tbl_tpcpoint_q LIMIT 1)'),
    (22, 'SELECT k FROM tbl_tpcbox_tpcpoint ORDER BY b |=| (SELECT q FROM tbl_tpcpoint_q LIMIT 1) LIMIT 5')) t(o, query);
$$ LANGUAGE SQL;
CREATE TABLE tbl_tpcbox_tpcpoint_seq AS SELECT tpcbox_tpcpoint_answers() AS a;
SELECT a FROM tbl_tpcbox_tpcpoint_seq;
SET enable_seqscan = off;
CREATE INDEX tbl_tpcbox_tpcpoint_gist_idx ON tbl_tpcbox_tpcpoint USING gist(b);
SELECT tpcbox_tpcpoint_indexed(), tpcbox_tpcpoint_answers() = a
FROM tbl_tpcbox_tpcpoint_seq;
DROP INDEX tbl_tpcbox_tpcpoint_gist_idx;
CREATE INDEX tbl_tpcbox_tpcpoint_quad_idx ON tbl_tpcbox_tpcpoint USING spgist(b);
SELECT tpcbox_tpcpoint_indexed(), tpcbox_tpcpoint_answers() = a
FROM tbl_tpcbox_tpcpoint_seq;
DROP INDEX tbl_tpcbox_tpcpoint_quad_idx;
CREATE INDEX tbl_tpcbox_tpcpoint_kd_idx ON tbl_tpcbox_tpcpoint
  USING spgist(b tpcbox_kdtree_ops);
SELECT tpcbox_tpcpoint_indexed(), tpcbox_tpcpoint_answers() = a
FROM tbl_tpcbox_tpcpoint_seq;
RESET enable_seqscan;
DROP TABLE tbl_tpcbox_tpcpoint, tbl_tpcpoint_q, tbl_tpcbox_tpcpoint_seq;
DROP FUNCTION tpcbox_tpcpoint_indexed(), tpcbox_tpcpoint_answers(), tpcbox_tpcpoint_probes_index(text);

-------------------------------------------------------------------------------
-- A column of tpcbox values indexed by GiST, quad-tree and k-d tree answers the
-- queries of a tpcpatch as the scan does: <<, &<, &&, &>, >>, ~=, @>, <@, &<|, <<|, |>>, |&>, -|-, &<#, <<#, #>>, #&>, &</, <</, />>, /&>, |=|
-------------------------------------------------------------------------------

CREATE TABLE tbl_tpcbox_tpcpatch AS
SELECT k, temp::tpcbox AS b FROM tbl_tpcpatch WHERE temp IS NOT NULL;
CREATE TABLE tbl_tpcpatch_q AS
SELECT temp AS q FROM tbl_tpcpatch WHERE temp IS NOT NULL ORDER BY k LIMIT 3;
CREATE FUNCTION tpcbox_tpcpatch_answers() RETURNS text AS $$
  SELECT string_agg(coalesce(c::text, 'null'), ',' ORDER BY o) FROM (
    SELECT 1, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b << q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 2, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b &< q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 3, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b && q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 4, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b &> q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 5, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b >> q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 6, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b ~= q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 7, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b @> q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 8, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b <@ q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 9, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b &<| q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 10, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b <<| q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 11, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b |>> q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 12, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b |&> q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 13, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b -|- q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 14, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b &<# q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 15, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b <<# q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 16, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b #>> q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 17, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b #&> q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 18, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b &</ q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 19, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b <</ q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 20, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b />> q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 21, (SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b /&> q) FROM tbl_tpcpatch_q
    UNION ALL
    SELECT 22, (SELECT sum(k) FROM (SELECT k FROM tbl_tpcbox_tpcpatch
      ORDER BY b |=| q, k LIMIT 5) s) FROM tbl_tpcpatch_q) t(o, c);
$$ LANGUAGE SQL;
CREATE FUNCTION tpcbox_tpcpatch_probes_index(query text)
RETURNS boolean LANGUAGE plpgsql AS $$
DECLARE
  line text;
BEGIN
  FOR line IN EXECUTE 'EXPLAIN (COSTS OFF) ' || query LOOP
    IF line LIKE '%Index Cond%' OR line LIKE '%Order By%' THEN
      RETURN true;
    END IF;
  END LOOP;
  RETURN false;
END;
$$;
CREATE FUNCTION tpcbox_tpcpatch_indexed() RETURNS text AS $$
  SELECT string_agg(CASE WHEN tpcbox_tpcpatch_probes_index(query) THEN 't' ELSE 'f' END, '' ORDER BY o)
  FROM (VALUES
    (1, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b << (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (2, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b &< (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (3, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b && (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (4, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b &> (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (5, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b >> (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (6, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b ~= (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (7, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b @> (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (8, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b <@ (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (9, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b &<| (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (10, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b <<| (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (11, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b |>> (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (12, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b |&> (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (13, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b -|- (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (14, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b &<# (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (15, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b <<# (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (16, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b #>> (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (17, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b #&> (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (18, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b &</ (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (19, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b <</ (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (20, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b />> (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (21, 'SELECT count(*) FROM tbl_tpcbox_tpcpatch WHERE b /&> (SELECT q FROM tbl_tpcpatch_q LIMIT 1)'),
    (22, 'SELECT k FROM tbl_tpcbox_tpcpatch ORDER BY b |=| (SELECT q FROM tbl_tpcpatch_q LIMIT 1) LIMIT 5')) t(o, query);
$$ LANGUAGE SQL;
CREATE TABLE tbl_tpcbox_tpcpatch_seq AS SELECT tpcbox_tpcpatch_answers() AS a;
SELECT a FROM tbl_tpcbox_tpcpatch_seq;
SET enable_seqscan = off;
CREATE INDEX tbl_tpcbox_tpcpatch_gist_idx ON tbl_tpcbox_tpcpatch USING gist(b);
SELECT tpcbox_tpcpatch_indexed(), tpcbox_tpcpatch_answers() = a
FROM tbl_tpcbox_tpcpatch_seq;
DROP INDEX tbl_tpcbox_tpcpatch_gist_idx;
CREATE INDEX tbl_tpcbox_tpcpatch_quad_idx ON tbl_tpcbox_tpcpatch USING spgist(b);
SELECT tpcbox_tpcpatch_indexed(), tpcbox_tpcpatch_answers() = a
FROM tbl_tpcbox_tpcpatch_seq;
DROP INDEX tbl_tpcbox_tpcpatch_quad_idx;
CREATE INDEX tbl_tpcbox_tpcpatch_kd_idx ON tbl_tpcbox_tpcpatch
  USING spgist(b tpcbox_kdtree_ops);
SELECT tpcbox_tpcpatch_indexed(), tpcbox_tpcpatch_answers() = a
FROM tbl_tpcbox_tpcpatch_seq;
RESET enable_seqscan;
DROP TABLE tbl_tpcbox_tpcpatch, tbl_tpcpatch_q, tbl_tpcbox_tpcpatch_seq;
DROP FUNCTION tpcbox_tpcpatch_indexed(), tpcbox_tpcpatch_answers(), tpcbox_tpcpatch_probes_index(text);
