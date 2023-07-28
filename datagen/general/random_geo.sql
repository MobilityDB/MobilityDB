/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

CREATE OR REPLACE FUNCTION random_int(low int, high int)
  RETURNS int AS $$
BEGIN
  RETURN floor(random() * (high-low+1) + low);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_int(1, 100) AS f
FROM generate_series(1, 15) AS k;
*/

CREATE OR REPLACE FUNCTION random_float(low float, high float)
  RETURNS float AS $$
BEGIN
  RETURN random() * (high-low) + low;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_float(1, 100) AS f
FROM generate_series(1, 15) AS k;
*/

CREATE OR REPLACE FUNCTION random_point(lowvalue float, highvalue float)
  RETURNS point AS $$
BEGIN
  RETURN point(random_float(lowvalue, highvalue), random_float(lowvalue, highvalue));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_point(1, 100) AS point
FROM generate_series(1,10) k;
*/

CREATE OR REPLACE FUNCTION random_line(lowvalue float, highvalue float)
  RETURNS line AS $$
BEGIN
  RETURN line(random_float(lowvalue, highvalue), random_float(lowvalue, highvalue),
    random_float(lowvalue, highvalue));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_line(1, 100) AS line
FROM generate_series(1,10) k;
*/

CREATE OR REPLACE FUNCTION random_lseg(lowvalue float, highvalue float)
  RETURNS lseg AS $$
BEGIN
  RETURN lseg(random_point(lowvalue, highvalue), random_point(lowvalue, highvalue));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_lseg(1, 100) AS lseg
FROM generate_series(1,10) k;
*/

CREATE OR REPLACE FUNCTION random_box(lowvalue float, highvalue float)
  RETURNS box AS $$
BEGIN
  RETURN box(random_point(lowvalue, highvalue), random_point(lowvalue, highvalue));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_lseg(1, 100) AS box
FROM generate_series(1,10) k;
*/

CREATE OR REPLACE FUNCTION random_circle(lowvalue float, highvalue float,
  maxradius float)
  RETURNS circle AS $$
BEGIN
  RETURN circle(random_point(lowvalue, highvalue), random_float(0, maxradius));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_circle(1, 100, 5) AS circle
FROM generate_series(1,10) k;
*/

CREATE OR REPLACE FUNCTION random_path(low float, high float, mincard int, maxcard int)
  RETURNS path AS $$
DECLARE
  ptarr point[];
BEGIN
  SELECT array_agg(random_point(low, high)) INTO ptarr
  FROM generate_series(1, random_int(mincard, maxcard)) AS x;
  RETURN path(ptarr);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_path(1, 100, 10) AS path
FROM generate_series(1, 10) AS k;
*/

CREATE OR REPLACE FUNCTION random_polygon(low float, high float, mincard int,
  maxcard int)
  RETURNS polygon AS $$
DECLARE
  ptarr point[];
BEGIN
  SELECT array_agg(random_point(low, high)) INTO ptarr
  FROM generate_series(1, random_int(mincard, maxcard)) AS x;
  ptarr = array_append(ptarr, ptarr[1]);
  RETURN polygon(ptarr);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_polygon(1, 100, 10) AS polygon
FROM generate_series(1, 10) AS k;
*/

/*****************************************************************************/
