-- MobilityDB adapter for the cross-type parity harness.
--
-- Emits one TSV row per (operator, first-arg type) over the functions owned by
-- the mobilitydb extension. First arg = the receiver, per the MEOS name-prefix
-- object model. Pipe the output to parity_audit.py:
--
--   psql -d <db> -At -F $'\t' -f tools/parity_audit/adapters/mobilitydb.sql \
--     > /tmp/mobilitydb_funcs.tsv
--   python3 tools/parity_audit/parity_audit.py /tmp/mobilitydb_funcs.tsv
--
-- Run against a database with the build under test installed
-- (CREATE EXTENSION mobilitydb CASCADE).

SELECT p.proname,
       COALESCE((p.proargtypes[0])::regtype::text, ''),
       pg_get_function_arguments(p.oid),
       pg_get_function_result(p.oid)
FROM pg_proc p
JOIN pg_depend d ON d.objid = p.oid AND d.deptype = 'e'
JOIN pg_extension e ON e.oid = d.refobjid AND e.extname = 'mobilitydb'
ORDER BY 1, 2;
