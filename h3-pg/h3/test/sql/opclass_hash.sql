\pset tuples_only on

CREATE TABLE h3_test_hash (hex h3index PRIMARY KEY);
INSERT INTO h3_test_hash (hex) SELECT * from h3_get_res_0_cells();
CREATE INDEX h3_btree ON h3_test_hash USING hash (hex);

--
-- TEST hash operator class
--
SELECT COUNT(hex) = 122 FROM (
    SELECT hex FROM h3_test_hash WHERE hex IN (SELECT h3_get_res_0_cells())
) q;

CREATE TABLE h3_test_hash_part (hex h3index PRIMARY KEY) PARTITION BY HASH (hex);