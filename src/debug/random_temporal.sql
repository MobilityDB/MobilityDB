-------------------------------------------------------------------------------
-- Basic types
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_bool()
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

CREATE OR REPLACE FUNCTION random_int(low int, high int) 
	RETURNS int AS $$
BEGIN
	RETURN floor(random() * (high-low+1) + low);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
select random_int(1,7), count(*)
from generate_series(1, 1e3)
group by 1
order by 1
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_intarr(low int, high int, maxcard int) 
	RETURNS int[] AS $$
DECLARE
	result int[];
BEGIN
	SELECT array_agg(random_int(low,high)) INTO result
	FROM generate_series (1, random_int(1, maxcard)) AS x;
	RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_intarr(1, 20, 10) AS iarr
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_intrange(low int, high int, length int) 
	RETURNS intrange AS $$
DECLARE 
	t int;
BEGIN
	t =  random_int(low, high);
	RETURN intrange(t, t + random_int(1, length));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_intrange(1, 100, 10) AS ir
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_float(low float, high float) 
	RETURNS float AS $$
BEGIN
	RETURN random() * (high-low) + low;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_float(1, 20) AS f
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_floatarr(low float, high float, maxcard int) 
	RETURNS float[] AS $$
DECLARE
	result int[];
BEGIN
	SELECT array_agg(random_float(low,high)) INTO result
	FROM generate_series (1, random_int(1, maxcard)) AS x;
	RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_floatarr(1, 20, 10) AS farr
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_floatrange(low float, high float, length int) 
	RETURNS floatrange AS $$
DECLARE 
	t float;
BEGIN
	t =  random_float(low, high);
	RETURN floatrange(t, t + random_float(1, length));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_floatrange(1, 100, 10) AS fr
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_ascii() 
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

CREATE OR REPLACE FUNCTION random_text(maxlength int) 
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

CREATE OR REPLACE FUNCTION random_textarr(maxlength int, maxcard int)
	RETURNS text[] AS $$
DECLARE
	textarr text[];
	card int;
BEGIN
	card = random_int(3, maxcard);
	for i in 1..card 
	loop
		textarr[i] = random_text(maxlength);
	end loop;
	return textarr;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_textarr(20, 10) AS text
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------
-- Tbox Type
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tbox(lowx float, highx float, 
	lowt timestamptz, hight timestamptz, maxsize float, maxminutes int) 
	RETURNS tbox AS $$
DECLARE
	xmin float;
	tmin timestamptz;
BEGIN
	xmin = random_float(lowx, highx);
	tmin = random_timestamptz(lowt, hight);
	RETURN tbox(xmin, tmin, xmin + random_float(1, maxsize), 
		tmin + random_minutes(1, maxminutes));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tbox(0, 100, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------
-- Time Types
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_timestamptz(low timestamptz, high timestamptz) 
	RETURNS timestamptz AS $$
BEGIN
	RETURN date_trunc('minute', (low + random() * (high - low)))::timestamptz(0);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_timestamptz('2001-01-01', '2001-12-31') AS t
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_timestamptzarr(low timestamptz, high timestamptz, maxcard int) 
	RETURNS timestamptz[] AS $$
DECLARE
	result timestamptz[];
BEGIN
	SELECT array_agg(random_timestamptz(low,high)) INTO result
	FROM generate_series (1, random_int(1, maxcard)) AS x;
	RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_timestamptzarr('2001-01-01', '2001-12-31', 10) AS tarr
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_minutes(low int, high int) 
	RETURNS interval AS $$
BEGIN
	RETURN random_int(low, high) * '1 minutes'::interval;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_minutes(1, 20) AS m
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tstzrange(low timestamptz, high timestamptz, maxminutes int) 
	RETURNS tstzrange AS $$
DECLARE 
	t timestamptz;
	lower_inc char;
	upper_inc char;
BEGIN
	t = random_timestamptz(low, high);
	IF random() > 0.1 THEN 
		IF random() > 0.5 THEN lower_inc = '['; ELSE lower_inc = '('; END IF;
		IF random() > 0.5 THEN upper_inc = ']'; ELSE upper_inc = ')'; END IF;
		RETURN tstzrange(t, t + random_minutes(1, maxminutes), lower_inc || upper_inc);
	ELSE
		RETURN tstzrange(t, t, '[]');
	END IF;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tstzrange('2001-01-01', '2001-12-31', 10) AS r
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tstzrangearr(low timestamptz, high timestamptz, 
	maxminutes int, maxcard int) 
	RETURNS tstzrange[] AS $$
DECLARE
	result tstzrange[];
	card int;
	t1 timestamptz;
	t2 timestamptz;
	lower_inc char;
	upper_inc char;
BEGIN
	card = random_int(1, maxcard);
	t1 = random_timestamptz(low,high);
	for i in 1..card 
	loop
		IF random() > 0.1 THEN 
			IF random() > 0.5 THEN lower_inc = '['; ELSE lower_inc = '('; END IF;
			IF random() > 0.5 THEN upper_inc = ']'; ELSE upper_inc = ')'; END IF;
			t2 = t1 + random_minutes(1, maxminutes);
			result[i] = tstzrange(t1, t2, lower_inc || upper_inc);
		ELSE
			result[i] = tstzrange(t1, t1, '[]');
			t2 = t1;
		END IF;
		t1 = t2 + random_minutes(1, maxminutes);
	end loop;
	RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tstzrangearr('2001-01-01', '2001-12-31', 10, 10) AS rarr
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_period(low timestamptz, high timestamptz, maxminutes int) 
	RETURNS period AS $$
DECLARE 
	t timestamptz;
	lower_inc boolean;
	upper_inc boolean;
BEGIN
	t =  random_timestamptz(low, high);
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
SELECT k, random_period('2001-01-01', '2001-12-31', 10) AS p
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_periodarr(low timestamptz, high timestamptz, 
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
	card = random_int(1, maxcard);
	t1 = random_timestamptz(low,high);
	for i in 1..card 
	loop
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
	end loop;
	RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_periodarr('2001-01-01', '2001-12-31', 10, 10) AS parr
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_timestampset(low timestamptz, high timestamptz, 
	maxminutes int, maxcard int) 
	RETURNS timestampset AS $$
DECLARE
	result timestamptz[];
	t timestamptz;
	card int;
BEGIN
	card = random_int(1, maxcard);
	t = random_timestamptz(low, high);
	for i in 1..card 
	loop
		result[i] = t;
		t = t + random_minutes(1, maxminutes);
	end loop;
	RETURN timestampset(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_timestampset('2001-01-01', '2001-12-31', 10, 10) AS ps
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_periodset(low timestamptz, high timestamptz, 
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
	t1 = random_timestamptz(low, high);
	for i in 1..card 
	loop
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
	end loop;
	RETURN periodset(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_periodset('2001-01-01', '2001-12-31', 10, 10) AS ps
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------
-- Temporal Instant
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tboolinst(lowtime timestamptz, hightime timestamptz) 
	RETURNS tbool AS $$
BEGIN
	RETURN tboolinst(random_bool(), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tboolinst('2001-01-01', '2001-12-31') AS inst
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tintinst(lowvalue int, highvalue int, 
	lowtime timestamptz, hightime timestamptz) 
	RETURNS tint AS $$
BEGIN
	RETURN tintinst(random_int(lowvalue, highvalue), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tintinst(1, 20, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tfloatinst(lowvalue float, highvalue float, 
	lowtime timestamptz, hightime timestamptz) 
	RETURNS tfloat AS $$
BEGIN
	RETURN tfloatinst(random_float(lowvalue, highvalue), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tfloatinst(1, 20, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_ttextinst(lowtime timestamptz, hightime timestamptz, maxlength int) 
	RETURNS ttext AS $$
BEGIN
	RETURN ttextinst(random_text(maxlength), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_ttextinst('2001-01-01', '2001-12-31', 20) AS inst
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------
-- Temporal Instant Set
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tbooli(lowtime timestamptz, hightime timestamptz, 
	maxminutes int, maxcard int)
	RETURNS tbool AS $$
DECLARE
	result tbool[];
	card int;
	t timestamptz;
BEGIN
	card = random_int(1, maxcard);
	t = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		result[i] = tboolinst(random_bool(), t);
		t = t + random_minutes(1, maxminutes);
	end loop;
	RETURN tbooli(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tbooli('2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tinti(lowvalue int, highvalue int, 
	lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int)
	RETURNS tint AS $$
DECLARE
	result tint[];
	card int;
	t timestamptz;
BEGIN
	card = random_int(1, maxcard);
	t = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		result[i] = tintinst(random_int(lowvalue, highvalue), t);
		t = t + random_minutes(1, maxminutes);
	end loop;
	RETURN tinti(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tinti(1, 20, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tfloati(lowvalue float, highvalue float, 
	lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int)
	RETURNS tfloat AS $$
DECLARE
	result tfloat[];
	card int;
	t timestamptz;
BEGIN
	card = random_int(1, maxcard);
	t = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		result[i] = tfloatinst(random_float(lowvalue, highvalue), t);
		t = t + random_minutes(1, maxminutes);
	end loop;
	RETURN tfloati(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tfloati(1, 20, '2001-01-01', '2001-12-31', 10, 10) AS ti
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_ttexti(lowtime timestamptz, hightime timestamptz, 
	maxtextlength int, maxminutes int, maxcard int)
	RETURNS ttext AS $$
DECLARE
	result ttext[];
	card int;
	t timestamptz;
BEGIN
	card = random_int(1, maxcard);
	t = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		result[i] = ttextinst(random_text(maxtextlength), t);
		t = t + random_minutes(1, maxminutes);
	end loop;
	RETURN ttexti(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_ttexti('2001-01-01', '2001-12-31', 10, 10, 10) AS ti
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------
-- Temporal Sequence
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tboolseq(lowtime timestamptz, hightime timestamptz, 
	maxcard int, maxminutes int) 
	RETURNS tbool AS $$
DECLARE
	result tbool[];
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
	val boolean;
BEGIN
	card = random_int(1, maxcard);
	if card = 1 then
		lower_inc = true;
		upper_inc = true;
	else
		lower_inc = random() > 0.5;
		upper_inc = random() > 0.5;
	end if;
	/* Necessary to avoid a NULL when card = 1 and we do not enter the loop below */
	val = random_bool();
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card-1
	loop
		val = NOT val;
		t1 = t1 + random_minutes(1, maxminutes);
		result[i] = tboolinst(val, t1);
	end loop;
	-- Sequences with step interpolation and exclusive upper bound must have  
	-- the same value in the last two instants
	t1 = t1 + random_minutes(1, maxminutes);
	if card = 1 or upper_inc then
        val = NOT val;
	end if;
	result[card] = tboolinst(val, t1);
	RETURN tboolseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tboolseq('2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tintseq(lowvalue int, highvalue int, 
	lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int) 
	RETURNS tint AS $$
DECLARE
	result tint[];
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
	val int;
BEGIN
	card = random_int(1, maxcard);
	if card = 1 then
		lower_inc = true;
		upper_inc = true;
	else
		lower_inc = random() > 0.5;
		upper_inc = random() > 0.5;
	end if;
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card-1
	loop
		val = random_int(lowvalue, highvalue);
		t1 = t1 + random_minutes(1, maxminutes);
		result[i] = tintinst(val, t1);
	end loop;
	-- Sequences with step interpolation and exclusive upper bound must have  
	-- the same value in the last two instants
	t1 = t1 + random_minutes(1, maxminutes);
	if card = 1 or upper_inc then
		val = random_int(lowvalue, highvalue);
	end if;
	result[card] = tintinst(val, t1);
	RETURN tintseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tintseq(1, 20, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tfloatseq(lowvalue float, highvalue float, 
	lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int) 
	RETURNS tfloat AS $$
DECLARE
	result tfloat[];
	card int;
	t1 timestamptz;
	v1 float;
	lower_inc boolean;
	upper_inc boolean;
BEGIN
	card = random_int(1, maxcard);
	if card = 1 then
		lower_inc = true;
		upper_inc = true;
	else
		lower_inc = random() > 0.5;
		upper_inc = random() > 0.5;
	end if;
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		t1 = t1 + random_minutes(1, maxminutes);
		result[i] = tfloatinst(random_float(lowvalue, highvalue), t1);
	end loop;
	RETURN tfloatseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tfloatseq(1, 20, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_ttextseq(lowtime timestamptz, hightime timestamptz, 
	maxtextlength int, maxminutes int, maxcard int) 
	RETURNS ttext AS $$
DECLARE
	result ttext[];
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
	val text;
BEGIN
	card = random_int(1, maxcard);
	if card = 1 then
		lower_inc = true;
		upper_inc = true;
	else
		lower_inc = random() > 0.5;
		upper_inc = random() > 0.5;
	end if;
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card-1 
	loop
		val = random_text(maxtextlength);
		t1 = t1 + random_minutes(1, maxminutes);
		result[i] = ttextinst(val, t1);
	end loop;
	-- Sequences with step interpolation and exclusive upper bound must have  
	-- the same value in the last two instants
	t1 = t1 + random_minutes(1, maxminutes);
	if card = 1 or upper_inc then
		val = random_text(maxtextlength);
	end if;
	result[card] = ttextinst(val, t1);
	RETURN ttextseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_ttextseq('2001-01-01', '2001-12-31', 10, 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------
-- Temporal Sequence Set
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tbools(lowtime timestamptz, hightime timestamptz, 
	maxminutes int, maxcardseq int, maxcard int) 
	RETURNS tbool AS $$
DECLARE
	result tbool[];
	instants tbool[];
	cardseq int;
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
	val boolean;
BEGIN
	card = random_int(1, maxcard);
	t1 = random_timestamptz(lowtime, hightime);
	/* Necessary to avoid a NULL when card = 1 and we do not enter the loop below */
	val = random_bool();
	for i in 1..card 
	loop
		cardseq = random_int(1, maxcardseq);
		if cardseq = 1 then
			lower_inc = true;
			upper_inc = true;
		else
			lower_inc = random() > 0.5;
			upper_inc = random() > 0.5;
		end if;
		for j in 1..cardseq-1
		loop
			val = NOT val;
			t1 = t1 + random_minutes(1, maxminutes);
			instants[j] = tboolinst(val, t1);
		end loop;
		-- Sequences with step interpolation and exclusive upper bound must have  
		-- the same value in the last two instants
		t1 = t1 + random_minutes(1, maxminutes);
		if cardseq = 1 or upper_inc then
			val = NOT val;
		end if;
		instants[cardseq] = tboolinst(val, t1);
		result[i] = tboolseq(instants, lower_inc, upper_inc);
		instants = NULL;
		t1 = t1 + random_minutes(1, maxminutes);
	end loop;
	RETURN tbools(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tbools('2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tints(lowvalue int, highvalue int, 
	lowtime timestamptz, hightime timestamptz, 
	maxminutes int, maxcardseq int, maxcard int) 
	RETURNS tint AS $$
DECLARE
	result tint[];
	instants tint[];
	cardseq int;
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
	val int;
BEGIN
	card = random_int(1, maxcard);
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		cardseq = random_int(1, maxcardseq);
		if cardseq = 1 then
			lower_inc = true;
			upper_inc = true;
		else
			lower_inc = random() > 0.5;
			upper_inc = random() > 0.5;
		end if;
		for j in 1..cardseq-1
		loop
			val = random_int(lowvalue, highvalue);
			t1 = t1 + random_minutes(1, maxminutes);
			instants[j] = tintinst(val, t1);
		end loop;
		-- Sequences with step interpolation and exclusive upper bound must have  
		-- the same value in the last two instants
		t1 = t1 + random_minutes(1, maxminutes);
		if cardseq = 1 or upper_inc then
			val = random_int(lowvalue, highvalue);
		end if;
		instants[cardseq] = tintinst(val, t1);
		result[i] = tintseq(instants, lower_inc, upper_inc);
		instants = NULL;
		t1 = t1 + random_minutes(1, maxminutes);
	end loop;
	RETURN tints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tints(1, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tfloats(lowvalue int, highvalue int, 
	lowtime timestamptz, hightime timestamptz, 
	maxminutes int, maxcardseq int, maxcard int) 
	RETURNS tfloat AS $$
DECLARE
	result tfloat[];
	instants tfloat[];
	cardseq int;
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
BEGIN
	card = random_int(1, maxcard);
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card 
	loop
		cardseq = random_int(1, maxcardseq);
		if cardseq = 1 then
			lower_inc = true;
			upper_inc = true;
		else
			lower_inc = random() > 0.5;
			upper_inc = random() > 0.5;
		end if;
		for j in 1..cardseq
		loop
			t1 = t1 + random_minutes(1, maxminutes);
			instants[j] = tfloatinst(random_float(lowvalue, highvalue), t1);
		end loop;
		result[i] = tfloatseq(instants, lower_inc, upper_inc);
		instants = NULL;
		t1 = t1 + random_minutes(1, maxminutes);
	end loop;
	RETURN tfloats(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tfloats(1, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_ttexts(lowtime timestamptz, hightime timestamptz, 
	maxtextlength int, maxminutes int, maxcardseq int, maxcard int) 
	RETURNS ttext AS $$
DECLARE
	result ttext[];
	instants ttext[];
	cardseq int;
	card int;
	t1 timestamptz;
	lower_inc boolean;
	upper_inc boolean;
	val text;
BEGIN
	card = random_int(1, maxcard);
	t1 = random_timestamptz(lowtime, hightime);
	for i in 1..card
	loop
		cardseq = random_int(1, maxcardseq);
		if cardseq = 1 then
			lower_inc = true;
			upper_inc = true;
		else
			lower_inc = random() > 0.5;
			upper_inc = random() > 0.5;
		end if;
		for j in 1..cardseq-1
		loop
			val = random_text(maxtextlength);
			t1 = t1 + random_minutes(1, maxminutes);
			instants[j] = ttextinst(val, t1);
		end loop;
		-- Sequences with step interpolation and exclusive upper bound must have  
		-- the same value in the last two instants
		t1 = t1 + random_minutes(1, maxminutes);
		if cardseq = 1 or upper_inc then
			val = random_text(maxtextlength);
		end if;
		instants[cardseq] = ttextinst(val, t1);
		result[i] = ttextseq(instants, lower_inc, upper_inc);
		instants = NULL;
		t1 = t1 + random_minutes(1, maxminutes);
	end loop;
	RETURN ttexts(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_ttexts('2001-01-01', '2001-12-31', 10, 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------
