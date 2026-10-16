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

/**
 * @file
 * @brief Moving window temporal aggregate functions
 */

-- The functions are not strict
CREATE FUNCTION wMinTransition(internal, tint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_wmin_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wMaxTransition(internal, tint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_wmax_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wSumTransition(internal, tint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_wsum_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wCountTransition(internal, tint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wAvgTransition(internal, tint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_wavg_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wMin(tint, interval) (
  SFUNC = wMinTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tmin_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wMax(tint, interval) (
  SFUNC = wMaxTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tmax_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wSum(tint, interval) (
  SFUNC = wSumTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wCount(tint, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wAvg(tint, interval) (
  SFUNC = wAvgTransition,
  STYPE = internal,
  COMBINEFUNC = tavg_combinefn,
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

-- The functions are not strict
CREATE FUNCTION wMinTransition(internal, tbigint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_wmin_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wMaxTransition(internal, tbigint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_wmax_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wSumTransition(internal, tbigint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_wsum_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wCountTransition(internal, tbigint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wAvgTransition(internal, tbigint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_wavg_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wMin(tbigint, interval) (
  SFUNC = wMinTransition,
  STYPE = internal,
  COMBINEFUNC = tbigint_tmin_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wMax(tbigint, interval) (
  SFUNC = wMaxTransition,
  STYPE = internal,
  COMBINEFUNC = tbigint_tmax_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wSum(tbigint, interval) (
  SFUNC = wSumTransition,
  STYPE = internal,
  COMBINEFUNC = tbigint_tsum_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wCount(tbigint, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = tbigint_tsum_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wAvg(tbigint, interval) (
  SFUNC = wAvgTransition,
  STYPE = internal,
  COMBINEFUNC = tavg_combinefn,
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

-- The functions are not strict
CREATE FUNCTION wMinTransition(internal, tfloat, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_wmin_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wMaxTransition(internal, tfloat, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_wmax_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wSumTransition(internal, tfloat, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_wsum_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wCountTransition(internal, tfloat, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wAvgTransition(internal, tfloat, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_wavg_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wMin(tfloat, interval) (
  SFUNC = wMinTransition,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmin_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wMax(tfloat, interval) (
  SFUNC = wMaxTransition,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmax_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wSum(tfloat, interval) (
  SFUNC = wSumTransition,
  STYPE = internal,
  COMBINEFUNC = tfloat_tsum_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wCount(tfloat, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wAvg(tfloat, interval) (
  SFUNC = wAvgTransition,
  STYPE = internal,
  COMBINEFUNC = tavg_combinefn,
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/
