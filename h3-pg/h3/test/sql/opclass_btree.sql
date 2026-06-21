\pset tuples_only on
\set string '\'801dfffffffffff\''
\set hexagon ':string::h3index'

CREATE TABLE h3_test_btree (hex h3index PRIMARY KEY);
INSERT INTO h3_test_btree (hex) SELECT * from h3_get_res_0_cells();
CREATE INDEX h3_btree ON h3_test_btree USING btree (hex);
--
-- TEST b-tree operator class
--
SELECT hex = :hexagon FROM (
    SELECT hex FROM h3_test_btree WHERE hex = :hexagon
) q;
