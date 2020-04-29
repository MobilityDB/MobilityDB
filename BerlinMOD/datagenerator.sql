CREATE OR REPLACE FUNCTION random_gauss(avg float = 0, stddev float = 1)
RETURNS float LANGUAGE plpgsql AS $$
DECLARE x1 real; x2 real; w real;
BEGIN
  LOOP
    x1 = 2.0 * random() - 1.0;
    x2 = 2.0 * random() - 1.0;
    w = x1*x1 + x2*x2;
    EXIT WHEN w < 1.0;
  END LOOP;
  RETURN avg + x1 * sqrt(-2.0*ln(w)/w) * stddev;
END; $$;

/*
with data as (
  select t, random_gauss(100,15)::integer score from generate_series(1,1000000) t
)
select
  score,
  sum(1),
  repeat('=',sum(1)::integer/500) bar
from data
where score between 60 and 140
group by score
order by 1;
*/

CREATE OR REPLACE FUNCTION generate()
RETURNS void LANGUAGE plpgsql AS $$
DECLARE
	SCALEFACTOR float = 0.05;
	SCALEFCARS float = sqrt(SCALEFACTOR);
	SCALEFDAYS float = sqrt(SCALEFACTOR);
	P_STARTDAY date = '2000-01-03';
	P_NUMCARS integer = round((2000 * SCALEFCARS)::numeric,0)::integer;
	P_NUMDAYS integer = round((28 * SCALEFDAYS)::numeric, 0)::integer;
	P_MINPAUSE_MS int = 300000;
BEGIN
	PERFORM SCALEFACTOR, SCALEFCARS, SCALEFDAYS, P_STARTDAY, P_NUMCARS, P_NUMDAYS, P_MINPAUSE_MS;
	-- return 'THE END';
END; $$;

select generate();

