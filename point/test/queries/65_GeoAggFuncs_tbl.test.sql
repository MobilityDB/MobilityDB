-------------------------------------------------------------------------------
set parallel_tuple_cost=0;
set parallel_setup_cost=0;
set force_parallel_mode=off;
-------------------------------------------------------------------------------

SELECT astext(unnest(sequences(tcentroid(ts)))) FROM tbl_tgeompoint3Ds;

-------------------------------------------------------------------------------
set parallel_tuple_cost=100;
set parallel_setup_cost=100;
set force_parallel_mode=off;
-------------------------------------------------------------------------------
