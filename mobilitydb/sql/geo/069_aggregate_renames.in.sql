/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS
 * TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Forward-compatible aggregate aliases per RFC #827.
 *
 * Each aggregate registered by the previous SQL files is exposed under a
 * Pascal-cased *Agg name in addition to its original lowercase or
 * camelCase name. This eliminates the case-folding collision between the
 * scalar accessors `Tmin(tbox|stbox)` / `Tmax(tbox|stbox)` and the
 * temporal-min / temporal-max aggregates, which previously differed only
 * in the casing of their first letter and so collapsed to the same
 * canonical name in catalogs that are case-insensitive but key on
 * `(name, kind)` rather than `(name, argtypes, kind)` (DuckDB, BigQuery,
 * Snowflake, Trino, ...).
 *
 * The original names remain valid for backward compatibility. Once
 * downstream tools have migrated, a future major version may drop the
 * un-suffixed aliases.
 *
 * The new aggregates share the same internal C transition functions as
 * the originals, so the only runtime cost is one extra row in
 * `pg_aggregate` per aggregate.
 */

-- From 050_geoset.in.sql
CREATE AGGREGATE SetUnionAgg(geometry) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = geomset_union_finalfn
);

CREATE AGGREGATE SetUnionAgg(geography) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = geogset_union_finalfn
);

CREATE AGGREGATE SetUnionAgg(geomset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = geomset_union_finalfn
);

CREATE AGGREGATE SetUnionAgg(geogset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = geogset_union_finalfn
);

-- From 068_tpoint_aggfuncs.in.sql
CREATE AGGREGATE TcountAgg(tgeompoint) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TcountAgg(tgeogpoint) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WcountAgg(tgeompoint, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WcountAgg(tgeogpoint, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TcentroidAgg(tgeompoint) (
  SFUNC = tcentroid_transfn,
  STYPE = internal,
  COMBINEFUNC = tcentroid_combinefn,
  FINALFUNC = tcentroid_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE MergeAgg(tgeompoint) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tgeompoint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

CREATE AGGREGATE MergeAgg(tgeogpoint) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tgeogpoint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeompoint) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeogpoint) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeompoint, text) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeogpoint, text) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeompoint, text, float, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeogpoint, text, float, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendSequenceAgg(tgeompoint) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendSequenceAgg(tgeogpoint) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

-- From 068_tgeo_aggfuncs.in.sql
CREATE AGGREGATE TcountAgg(tgeometry) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TcountAgg(tgeography) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WcountAgg(tgeometry, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WcountAgg(tgeography, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE MergeAgg(tgeometry) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tgeometry_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

CREATE AGGREGATE MergeAgg(tgeography) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tgeography_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeometry) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeometry,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeography) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeography,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeometry, text) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeometry,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeography, text) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeography,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeometry, text, float, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeometry,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tgeography, text, float, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeography,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendSequenceAgg(tgeometry) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tgeometry,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendSequenceAgg(tgeography) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tgeography,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

