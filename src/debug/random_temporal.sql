/*****************************************************************************
 *
 * random_temporal.sql
 *    Basic synthetic data generator functions for temporal data types.
 *
 * These functions use lower and upper bounds for the generated values:
 * lowvalue and highvalue for values, lowtime and hightime for timestamps.
 * When generating series of values, the maxdelta argument states the maximum
 * difference between two consecutive values, while maxminutes states the
 * the maximum number of minutes between two consecutive timestamps.
 *
 * Copyright (c) 2020, Esteban Zimanyi, Universite Libre de Bruxelles
 *
 *****************************************************************************/
-------------------------------------------------------------------------------
-- Basic types
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_bool;
CREATE FUNCTION random_bool()
  RETURNS boolean AS $$
BEGIN
  IF random() > 0.5 THEN RETURN TRUE; ELSE RETURN FALSE; END IF;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_bool() AS i
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_int;
CREATE FUNCTION random_int(lowvalue int, highvalue int) 
  RETURNS int AS $$
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  RETURN floor(random() * (highvalue - lowvalue + 1) + lowvalue);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT random_int(1,7), count(*)
FROM generate_series(1, 1e3)
GROUP BY 1
ORDER BY 1;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_int_array;
CREATE FUNCTION random_int_array(lowvalue int, highvalue int, maxdelta int,
  maxcard int) 
  RETURNS int[] AS $$
DECLARE
  result int[];
  card int;
  delta int;
  v int;
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  card = random_int(1, maxcard);
  v = random_int(lowvalue, highvalue);
  FOR i IN 1..card 
  LOOP
    result[i] = v;
    IF i = card THEN EXIT; END IF;
    delta = random_int(-1 * maxdelta, maxdelta);
    /* If neither of these conditions is satisfied the same value is kept */
    IF (v + delta >= lowvalue AND v + delta <= highvalue) THEN
      v = v + delta;
    ELSIF (v - delta >= lowvalue AND v - delta <= highvalue) THEN
      v = v - delta;
    END IF;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_int_array(-100, 100, 10, 10) AS iarr
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_intrange;
CREATE FUNCTION random_intrange(lowvalue int, highvalue int, maxdelta int) 
  RETURNS intrange AS $$
DECLARE 
  t int;
BEGIN
  IF lowvalue > highvalue - maxdelta THEN
    RAISE EXCEPTION 'lowvalue must be less or equal than highvalue - maxdelta: %, %, %',
      lowvalue, highvalue, maxdelta;
  END IF;
  t =  random_int(lowvalue, highvalue - maxdelta);
  RETURN intrange(t, t + random_int(1, maxdelta));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_intrange(-100, 100, 10) AS ir
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_float;
CREATE FUNCTION random_float(lowvalue float, highvalue float) 
  RETURNS float AS $$
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less or equal than highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  RETURN random() * (highvalue - lowvalue) + lowvalue;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_float(-100, 100) AS f
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_float_array;
CREATE FUNCTION random_float_array(lowvalue float, highvalue float,
  maxdelta float, maxcard int) 
  RETURNS float[] AS $$
DECLARE
  result float[];
  card int;
  delta float;
  v float;
BEGIN
  IF lowvalue > highvalue - maxdelta THEN
    RAISE EXCEPTION 'lowvalue must be less or equal than highvalue - maxdelta: %, %, %',
      lowvalue, highvalue, maxdelta;
  END IF;
  card = random_int(1, maxcard);
  v = random_float(lowvalue, highvalue - maxdelta);
  FOR i IN 1..card 
  LOOP
    result[i] = v;
    IF i = card THEN EXIT; END IF;
    delta = random_float(-1 * maxdelta, maxdelta);
    /* If neither of these conditions is satisfied the same value is kept */
    IF (v + delta >= lowvalue AND v + delta <= highvalue) THEN
      v = v + delta;
    ELSIF (v - delta >= lowvalue AND v - delta <= highvalue) THEN
      v = v - delta;
    END IF;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_float_array(-100, 100, 10, 10) AS farr
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_floatrange;
CREATE FUNCTION random_floatrange(lowvalue float, highvalue float, maxdelta int) 
  RETURNS floatrange AS $$
DECLARE 
  t float;
BEGIN
  IF lowvalue > highvalue - maxdelta THEN
    RAISE EXCEPTION 'lowvalue must be less or equal than highvalue - maxdelta: %, %, %',
      lowvalue, highvalue, maxdelta;
  END IF;
  t =  random_float(lowvalue, highvalue - maxdelta);
  RETURN floatrange(t, t + random_float(1, maxdelta));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_floatrange(-100, 100, 10) AS fr
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_ascii;
CREATE FUNCTION random_ascii() 
  RETURNS char AS $$
BEGIN
  -- ascii('A') = 65, ascii('Z') = 90, 
  RETURN chr(random_int(65, 90));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_ascii() AS m
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_text;
CREATE FUNCTION random_text(maxlength int) 
  RETURNS text AS $$
DECLARE
  result text;
BEGIN
  SELECT string_agg(random_ascii(),'') INTO result
  FROM generate_series (1, random_int(1, maxlength)) AS x;
  result = replace(result, '"', '\"');
  RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_text(20) AS text
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_textarr;
CREATE FUNCTION random_textarr(maxlength int, maxcard int)
  RETURNS text[] AS $$
DECLARE
  textarr text[];
  card int;
BEGIN
  card = random_int(3, maxcard);
  FOR i IN 1..card 
  LOOP
    textarr[i] = random_text(maxlength);
  END LOOP;
  return textarr;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_textarr(20, 10) AS text
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Time Types
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_timestamptz;
CREATE FUNCTION random_timestamptz(lowtime timestamptz, hightime timestamptz) 
  RETURNS timestamptz AS $$
BEGIN
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN date_trunc('minute', (lowtime + random() *
    (hightime - lowtime)))::timestamptz(0);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_timestamptz('2001-01-01', '2002-01-01') AS t
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_timestamptz_array;
CREATE FUNCTION random_timestamptz_array(lowtime timestamptz,
  hightime timestamptz, maxminutes int, maxcard int)
  RETURNS timestamptz[] AS $$
DECLARE
  result timestamptz[];
  card int;
  t timestamptz;
BEGIN
  IF lowtime > hightime - interval '1 minute' * maxminutes * maxcard THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes * maxcard: %, %, %, %',
      lowtime, hightime, maxminutes, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes * card);
  FOR i IN 1..card 
  LOOP
    result[i] = t;
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_timestamptz_array('2001-01-01', '2002-01-01', 10, 10) AS tarr
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_minutes;
CREATE FUNCTION random_minutes(lowvalue int, highvalue int) 
  RETURNS interval AS $$
BEGIN
  RETURN random_int(lowvalue, highvalue) * interval '1 minutes';
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_minutes(1, 20) AS m
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tstzrange;
CREATE FUNCTION random_tstzrange(lowtime timestamptz, hightime timestamptz,
  maxminutes int) 
  RETURNS tstzrange AS $$
DECLARE 
  t timestamptz;
  lower_inc char;
  upper_inc char;
BEGIN
  IF lowtime > hightime - interval '1 minute' * maxminutes THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes minutes: %, %, %',
      lowtime, hightime, maxminutes;
  END IF;
  t = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes);
  /* Generate instantaneous tstzranges with 0.1 probability */
  IF random() < 0.1 THEN 
    RETURN tstzrange(t, t, '[]');
  ELSE
    IF random() > 0.5 THEN lower_inc = '['; ELSE lower_inc = '('; END IF;
    IF random() > 0.5 THEN upper_inc = ']'; ELSE upper_inc = ')'; END IF;
    RETURN tstzrange(t, t + random_minutes(1, maxminutes), lower_inc || upper_inc);
  END IF;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tstzrange('2001-01-01', '2002-01-01', 10) AS r
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tstzrange_array;
CREATE FUNCTION random_tstzrange_array(lowtime timestamptz,
  hightime timestamptz, maxminutes int, maxcard int) 
  RETURNS tstzrange[] AS $$
DECLARE
  result tstzrange[];
  card int;
  t1 timestamptz;
  t2 timestamptz;
  lower_inc char;
  upper_inc char;
BEGIN
  IF lowtime > hightime - interval '1 minute' * maxminutes * maxcard THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes * maxcard minutes: %, %, %, %',
      lowtime, hightime, maxminutes, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t1 = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card 
  LOOP
    IF random() < 0.1 THEN 
      result[i] = tstzrange(t1, t1, '[]');
      t2 = t1;
    ELSE
      IF random() > 0.5 THEN lower_inc = '['; ELSE lower_inc = '('; END IF;
      IF random() > 0.5 THEN upper_inc = ']'; ELSE upper_inc = ')'; END IF;
      t2 = t1 + random_minutes(1, maxminutes);
      result[i] = tstzrange(t1, t2, lower_inc || upper_inc);
    END IF;
    t1 = t2 + random_minutes(1, maxminutes);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tstzrange_array('2001-01-01', '2002-01-01', 10, 10) AS rarr
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_period;
CREATE FUNCTION random_period(lowtime timestamptz, hightime timestamptz,
  maxminutes int) 
  RETURNS period AS $$
DECLARE 
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  IF lowtime > hightime - interval '1 minute' * maxminutes THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes minutes: %, %, %',
      lowtime, hightime, maxminutes;
  END IF;
  t =  random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes);
  IF random() > 0.1 THEN 
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
    RETURN period(t, t + random_minutes(1, maxminutes), lower_inc, upper_inc);
  ELSE
    RETURN period(t, t, true, true);
  END IF;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_period('2001-01-01', '2002-01-01', 10) AS p
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_period_array;
CREATE FUNCTION random_period_array(lowtime timestamptz, hightime timestamptz, 
  maxminutes int, maxcard int) 
  RETURNS period[] AS $$
DECLARE
  result period[];
  card int;
  t1 timestamptz;
  t2 timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  IF lowtime > hightime - interval '1 minute' * maxminutes * maxcard * 2 THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes * maxcard * 2 minutes: %, %, %, %',
      lowtime, hightime, maxminutes, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t1 =  random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes * maxcard * 2);
  FOR i IN 1..card 
  LOOP
    /* Generate instantaneous periods with 0.1 probability */
    IF random() < 0.1 THEN 
      result[i] = period(t1, t1, true, true);
      t2 = t1;
    ELSE
      lower_inc = random() > 0.5;
      upper_inc = random() > 0.5;
      t2 = t1 + random_minutes(1, maxminutes);
      result[i] = period(t1, t2, lower_inc, upper_inc);
    END IF;
    t1 = t2 + random_minutes(1, maxminutes);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_period_array('2001-01-01', '2002-01-01', 10, 10) AS parr
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_timestampset;
CREATE FUNCTION random_timestampset(lowtime timestamptz, hightime timestamptz, 
  maxminutes int, maxcard int) 
  RETURNS timestampset AS $$
DECLARE
  result timestamptz[];
  t timestamptz;
  card int;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card 
  LOOP
    result[i] = t;
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN timestampset(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_timestampset('2001-01-01', '2002-01-01', 10, 10) AS ps
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_periodset;
CREATE FUNCTION random_periodset(lowtime timestamptz, hightime timestamptz, 
  maxminutes int, maxcard int) 
  RETURNS periodset AS $$
DECLARE
  result period[];
  card int;
  t1 timestamptz;
  t2 timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  card = random_int(1, maxcard);
  t1 = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card 
  LOOP
    IF random() > 0.1 THEN 
      lower_inc = random() > 0.5;
      upper_inc = random() > 0.5;
      t2 = t1 + random_minutes(1, maxminutes);
      result[i] = period(t1, t2, lower_inc, upper_inc);
    ELSE
      result[i] = period(t1, t1, true, true);
      t2 = t1;
    END IF;  
    t1 = t2 + random_minutes(1, maxminutes);
  END LOOP;
  RETURN periodset(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_periodset('2001-01-01', '2002-01-01', 10, 10) AS ps
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Tbox Type
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tbox;
CREATE FUNCTION random_tbox(lowvalue float, highvalue float, 
  lowtime timestamptz, hightime timestamptz, maxdelta float, maxminutes int) 
  RETURNS tbox AS $$
DECLARE
  xmin float;
  tmin timestamptz;
BEGIN
  IF lowvalue > highvalue - maxdelta THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxdelta: %, %, %',
      lowvalue, highvalue, maxdelta;
  END IF;
  IF lowtime > hightime - interval '1 minute' * maxminutes THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes minutes: %, %, %',
      lowtime, hightime, maxminutes;
  END IF;
  xmin = random_float(lowvalue, highvalue - maxdelta);
  tmin = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes);
  RETURN tbox(xmin, tmin, xmin + random_float(1, maxdelta), 
    tmin + random_minutes(1, maxminutes));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tbox(-100, 100, '2001-01-01', '2002-01-01', 10, 10) AS b
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Temporal Instant
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tboolinst;
CREATE FUNCTION random_tboolinst(lowtime timestamptz, hightime timestamptz) 
  RETURNS tbool AS $$
BEGIN
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tboolinst(random_bool(), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tboolinst('2001-01-01', '2002-01-01') AS inst
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tintinst;
CREATE FUNCTION random_tintinst(lowvalue int, highvalue int, 
  lowtime timestamptz, hightime timestamptz) 
  RETURNS tint AS $$
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tintinst(random_int(lowvalue, highvalue), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tintinst(1, 20, '2001-01-01', '2002-01-01') AS inst
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tfloatinst;
CREATE FUNCTION random_tfloatinst(lowvalue float, highvalue float, 
  lowtime timestamptz, hightime timestamptz) 
  RETURNS tfloat AS $$
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tfloatinst(random_float(lowvalue, highvalue), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tfloatinst(1, 20, '2001-01-01', '2002-01-01') AS inst
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_ttextinst;
CREATE FUNCTION random_ttextinst(lowtime timestamptz, hightime timestamptz, maxlength int) 
  RETURNS ttext AS $$
BEGIN
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN ttextinst(random_text(maxlength), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_ttextinst('2001-01-01', '2002-01-01', 20) AS inst
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------
-- Temporal Instant Set
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tbooli;
CREATE FUNCTION random_tbooli(lowtime timestamptz, hightime timestamptz,
  maxminutes int, maxcard int)
  RETURNS tbool AS $$
DECLARE
  result tbool[];
  card int;
  t timestamptz;
BEGIN
  IF lowtime > hightime - interval '1 minute' * maxminutes * maxcard THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes * maxcard minutes: %, %, %, %',
      lowtime, hightime, maxminutes, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes * maxcard);
  FOR i IN 1..card 
  LOOP
    result[i] = tboolinst(random_bool(), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tbooli(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tbooli('2001-01-01', '2002-01-01', 10, 10) AS ti
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tinti;
CREATE FUNCTION random_tinti(lowvalue int, highvalue int, lowtime timestamptz,
  hightime timestamptz, maxdelta int, maxminutes int, maxcard int)
  RETURNS tint AS $$
DECLARE
  intarr int[];
  result tint[];
  card int;
  t timestamptz;
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %', lowvalue, highvalue;
  END IF;
  IF lowtime > hightime - interval '1 minute' * maxminutes * maxcard THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes * maxcard minutes: %, %, %, %',
      lowtime, hightime, maxminutes, maxcard;
  END IF;
  SELECT random_int_array(lowvalue, highvalue, maxdelta, maxcard) INTO intarr;
  card = array_length(intarr, 1);
  t = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes * maxcard);
  FOR i IN 1..card 
  LOOP
    result[i] = tintinst(intarr[i], t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tinti(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tinti(-100, 100, '2001-01-01', '2002-01-01', 10, 10, 10) AS ti
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tfloati;
CREATE FUNCTION random_tfloati(lowvalue float, highvalue float, 
  lowtime timestamptz, hightime timestamptz, maxdelta float, maxminutes int,
  maxcard int)
  RETURNS tint AS $$
DECLARE
  floatarr int[];
  result tint[];
  card int;
  t timestamptz;
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %', lowvalue, highvalue;
  END IF;
  IF lowtime > hightime - interval '1 minute' * maxminutes * maxcard THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes * maxcard minutes: %, %, %, %',
      lowtime, hightime, maxminutes, maxcard;
  END IF;
  SELECT random_float_array(lowvalue, highvalue, maxdelta, maxcard)
  INTO floatarr;
  card = array_length(floatarr, 1);
  t = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes * maxcard);
  FOR i IN 1..card 
  LOOP
    result[i] = tfloatinst(floatarr[i], t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tinti(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tfloati(-100, 100, '2001-01-01', '2002-01-01', 10, 10, 10) AS ti
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_ttexti;
CREATE FUNCTION random_ttexti(lowtime timestamptz, hightime timestamptz,
  maxtextlength int, maxminutes int, maxcard int)
  RETURNS ttext AS $$
DECLARE
  result ttext[];
  card int;
  t timestamptz;
BEGIN
  IF lowtime > hightime - interval '1 minute' * maxminutes * maxcard THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes * maxcard minutes: %, %, %, %',
      lowtime, hightime, maxminutes, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes * maxcard);
  FOR i IN 1..card 
  LOOP
    result[i] = ttextinst(random_text(maxtextlength), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN ttexti(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_ttexti('2001-01-01', '2002-01-01', 10, 10, 10) AS ti
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------
-- Temporal Sequence
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tboolseq;
CREATE FUNCTION random_tboolseq(lowtime timestamptz, hightime timestamptz,
  maxminutes int, maxcard int) 
  RETURNS tbool AS $$
DECLARE
  result tbool[];
  card int;
  v boolean;
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  IF lowtime > hightime - interval '1 minute' * maxminutes * maxcard THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxcard * maxminutes minutes: %, %, %, %',
      lowtime, hightime, maxminutes, maxcard;
  END IF;
  card = random_int(1, maxcard);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  v = random_bool();
  t = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes * maxcard);
  FOR i IN 1..card - 1
  LOOP
    result[i] = tboolinst(v, t);
    v = NOT v;
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have  
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc THEN
    v = NOT v;
  END IF;
  result[card] = tboolinst(v, t);
  RETURN tboolseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tboolseq('2001-01-01', '2002-01-01', 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tintseq;
CREATE FUNCTION random_tintseq(lowvalue int, highvalue int, lowtime timestamptz,
  hightime timestamptz, maxdelta int, maxminutes int, maxcard int) 
  RETURNS tint AS $$
DECLARE
  intarr int[];
  result tint[];
  card int;
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %', lowvalue, highvalue;
  END IF;
  IF lowtime > hightime - interval '1 minute' * maxcard * maxminutes THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxcard * maxminutes minutes: %, %, %, %',
      lowtime, hightime, maxminutes, maxcard;
  END IF;
  SELECT random_int_array(lowvalue, highvalue, maxdelta, maxcard)
  INTO intarr;
  card = array_length(intarr, 1);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  t = random_timestamptz(lowtime, hightime - interval '1 minute' * maxcard * maxminutes);
  FOR i IN 1..card - 1
  LOOP
    result[i] = tintinst(intarr[i], t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  -- Sequences with step interpolation AND exclusive upper bound must have  
  -- the same value IN the last two instants
  IF card = 1 or upper_inc THEN
    result[card] = tintinst(intarr[card], t);
  ELSE
    result[card] = tintinst(intarr[card - 1], t);
  END IF;
  RETURN tintseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tintseq(-100, 100, '2001-01-01', '2002-01-01', 10, 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tfloatseq;
CREATE FUNCTION random_tfloatseq(lowvalue float, highvalue float, 
  lowtime timestamptz, hightime timestamptz, maxdelta float, maxminutes int,
  maxcard int) 
  RETURNS tfloat AS $$
DECLARE
  floatarr float[];
  result tfloat[];
  card int;
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %', lowvalue, highvalue;
  END IF;
  IF lowtime > hightime - interval '1 minute' * maxcard * maxminutes THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxcard * maxminutes minutes: %, %, %, %',
      lowtime, hightime, maxminutes, maxcard;
  END IF;
  SELECT random_float_array(lowvalue, highvalue, maxdelta, maxcard)
  INTO floatarr;
  card = array_length(floatarr, 1);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  t = random_timestamptz(lowtime, hightime - interval '1 minute' * maxcard * maxminutes);
  FOR i IN 1..card
  LOOP
    result[i] = tfloatinst(floatarr[i], t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tfloatseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tfloatseq(-100, 100, '2001-01-01', '2002-01-01', 10, 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_ttextseq;
CREATE FUNCTION random_ttextseq(lowtime timestamptz, hightime timestamptz, 
  maxtextlength int, maxminutes int, maxcard int) 
  RETURNS ttext AS $$
DECLARE
  result ttext[];
  card int;
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
  val text;
BEGIN
  IF lowtime > hightime - interval '1 minute' * maxcard * maxminutes THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxcard * maxminutes minutes: %, %, %, %',
      lowtime, hightime, maxminutes, maxcard;
  END IF;
  card = random_int(1, maxcard);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card-1 
  LOOP
    val = random_text(maxtextlength);
    t = t + random_minutes(1, maxminutes - interval '1 minute' * maxcard * maxminutes);
    result[i] = ttextinst(val, t);
  END LOOP;
  -- Sequences with step interpolation AND exclusive upper bound must have  
  -- the same value IN the last two instants
  t = t + random_minutes(1, maxminutes);
  IF card = 1 or upper_inc THEN
    val = random_text(maxtextlength);
  END IF;
  result[card] = ttextinst(val, t);
  RETURN ttextseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_ttextseq('2001-01-01', '2002-01-01', 10, 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Temporal Sequence Set
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tbools;
CREATE FUNCTION random_tbools(lowtime timestamptz, hightime timestamptz,
  maxminutes int, maxcardseq int, maxcard int) 
  RETURNS tbool AS $$
DECLARE
  result tbool[];
  seq tbool;
  card int;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  IF lowtime > hightime - interval '1 minute' * 
    ( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - '
      '( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) minutes: %, %, %, %, %',
      lowtime, hightime, maxminutes, maxcardseq, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * (maxcard - 1)) + ((maxcard - 1) * maxminutes) );
  FOR i IN 1..card 
  LOOP
    SELECT random_tboolseq(t1, t2, maxminutes, maxcardseq)
    INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq);
  END LOOP;
  RETURN tbools(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tbools('2001-01-01', '2002-01-01', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tints;
CREATE FUNCTION random_tints(lowvalue int, highvalue int, lowtime timestamptz,
  hightime timestamptz, maxdelta int, maxminutes int, maxcardseq int, maxcard int) 
  RETURNS tint AS $$
DECLARE
  result tint[];
  seq tint;
  card int;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  IF lowtime > hightime - interval '1 minute' * 
    ( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - '
      '( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) minutes: %, %, %, %, %',
      lowtime, hightime, maxminutes, maxcardseq, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * (maxcard - 1)) + ((maxcard - 1) * maxminutes) );
  FOR i IN 1..card 
  LOOP
    SELECT random_tintseq(lowvalue, highvalue, t, hightime, maxdelta,
      maxminutes, maxcardseq) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq);
  END LOOP;
  RETURN tints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tints(1, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tfloats;
CREATE FUNCTION random_tfloats(lowvalue float, highvalue float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, maxcardseq int, maxcard int) 
  RETURNS tfloat AS $$
DECLARE
  result tfloat[];
  seq tfloat;
  card float;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  IF lowtime > hightime - interval '1 minute' * 
    ( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - '
      '( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) minutes: %, %, %, %, %',
      lowtime, hightime, maxminutes, maxcardseq, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * (maxcard - 1)) + ((maxcard - 1) * maxminutes) );
  FOR i IN 1..card 
  LOOP
    SELECT random_tfloatseq(lowvalue, highvalue, t, hightime, maxdelta,
      maxminutes, maxcardseq) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq);
  END LOOP;
  RETURN tfloats(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tfloats(1, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_ttexts;
CREATE FUNCTION random_ttexts(lowtime timestamptz, hightime timestamptz,
  maxminutes int, maxcardseq int, maxcard int) 
  RETURNS ttext AS $$
DECLARE
  result ttext[];
  seq ttext;
  card int;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  IF lowtime > hightime - interval '1 minute' * 
    ( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - '
      '( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) minutes: %, %, %, %, %',
      lowtime, hightime, maxminutes, maxcardseq, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * (maxcard - 1)) + ((maxcard - 1) * maxminutes) );
  FOR i IN 1..card 
  LOOP
    SELECT random_ttextseq(t, hightime, maxminutes, maxcardseq)
    INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq);
  END LOOP;
  RETURN ttexts(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_ttexts('2001-01-01', '2002-01-01', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------
