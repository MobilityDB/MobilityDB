/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice
 * and this paragraph and the following two paragraphs appear in all
 * copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE
 * BRUXELLES HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/*
 * 131_trgeo_tempspatialrels.in.sql
 *
 * Temporal spatial relationships for temporal rigid geometries.
 *
 * Each function returns a `tbool` whose value at instant t is the
 * static spatial relation applied to the trgeo's body polygon at t.
 *
 * Implementation: SQL composition through the materialised polygon
 * trajectory, exposed by the `tgeometry(trgeometry)` cast added in
 * `122_trgeo.in.sql`. The cast walks the trgeo's instants, applies
 * each pose to the reference geometry, and emits a `tgeometry`
 * polygon-per-instant; tgeometry's full temporal-rel surface
 * (tContains, tCovers, tDisjoint, tIntersects, tTouches, tDwithin)
 * then applies.
 *
 * trgeometry is 2D-only by spec; no geography variants are exposed.
 */

/*****************************************************************************
 * tContains
 *****************************************************************************/

CREATE FUNCTION tContains(geometry, trgeometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tContains($1, $2::tgeometry) $$;
CREATE FUNCTION tContains(trgeometry, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tContains($1::tgeometry, $2) $$;
CREATE FUNCTION tContains(trgeometry, trgeometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tContains($1::tgeometry, $2::tgeometry) $$;

/*****************************************************************************
 * tCovers
 *****************************************************************************/

CREATE FUNCTION tCovers(geometry, trgeometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tCovers($1, $2::tgeometry) $$;
CREATE FUNCTION tCovers(trgeometry, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tCovers($1::tgeometry, $2) $$;
CREATE FUNCTION tCovers(trgeometry, trgeometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tCovers($1::tgeometry, $2::tgeometry) $$;

/*****************************************************************************
 * tDisjoint
 *****************************************************************************/

CREATE FUNCTION tDisjoint(geometry, trgeometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tDisjoint($1, $2::tgeometry) $$;
CREATE FUNCTION tDisjoint(trgeometry, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tDisjoint($1::tgeometry, $2) $$;
CREATE FUNCTION tDisjoint(trgeometry, trgeometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tDisjoint($1::tgeometry, $2::tgeometry) $$;

/*****************************************************************************
 * tIntersects
 *****************************************************************************/

CREATE FUNCTION tIntersects(geometry, trgeometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tIntersects($1, $2::tgeometry) $$;
CREATE FUNCTION tIntersects(trgeometry, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tIntersects($1::tgeometry, $2) $$;
CREATE FUNCTION tIntersects(trgeometry, trgeometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tIntersects($1::tgeometry, $2::tgeometry) $$;

/*****************************************************************************
 * tTouches
 *****************************************************************************/

CREATE FUNCTION tTouches(geometry, trgeometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tTouches($1, $2::tgeometry) $$;
CREATE FUNCTION tTouches(trgeometry, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tTouches($1::tgeometry, $2) $$;
CREATE FUNCTION tTouches(trgeometry, trgeometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tTouches($1::tgeometry, $2::tgeometry) $$;

/*****************************************************************************
 * tDwithin
 *****************************************************************************/

CREATE FUNCTION tDwithin(geometry, trgeometry, dist float)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tDwithin($1, $2::tgeometry, $3) $$;
CREATE FUNCTION tDwithin(trgeometry, geometry, dist float)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tDwithin($1::tgeometry, $2, $3) $$;
CREATE FUNCTION tDwithin(trgeometry, trgeometry, dist float)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE
  AS $$ SELECT tDwithin($1::tgeometry, $2::tgeometry, $3) $$;

/*****************************************************************************/
