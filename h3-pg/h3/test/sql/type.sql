\pset tuples_only on
\set string '\'801dfffffffffff\''
\set asbigint 576988517884755967
\set hexagon ':string::h3index'
\set pentagon '\'844c001ffffffff\'::h3index'

--
-- TEST operators
--
SELECT :hexagon = :hexagon;
SELECT NOT :hexagon = :pentagon;
SELECT NOT :hexagon <> :hexagon;
SELECT :hexagon <> :pentagon;
SELECT :pentagon <@ h3_cell_to_parent(:pentagon);
SELECT bool_and(:pentagon @> c) FROM (
    SELECT h3_cell_to_children(:pentagon) c
) q;

--
-- TEST bigint casting
--
SELECT :asbigint = :hexagon::bigint;

SELECT :hexagon = :asbigint::h3index;

--
-- TEST binary io
--
CREATE TEMPORARY TABLE h3_test_binary_send (hex h3index PRIMARY KEY);
CREATE TEMPORARY TABLE h3_test_binary_recv (hex h3index PRIMARY KEY);
INSERT INTO h3_test_binary_send (hex) SELECT * from h3_get_res_0_cells();

-- we need to use \copy instead of SQL COPY in order to have relative path
\copy h3_test_binary_send TO 'h3_test_binary.bin' (FORMAT binary)
\copy h3_test_binary_recv FROM 'h3_test_binary.bin' (FORMAT binary)

-- make sure re-imported data matches original data
SELECT array_agg(hex) is null FROM (
    SELECT hex FROM h3_test_binary_send
    EXCEPT SELECT hex FROM h3_test_binary_recv
) q;
