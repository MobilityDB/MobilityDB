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
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief PG wrappers for the tpcpoint lifted temporal type (Phase 8H).
 *
 * Most SQL bindings delegate to the generic @c Temporal_* PG wrappers in
 * @c mobilitydb/src/temporal/temporal.c — the generic parser / output /
 * constructor / accessor path dispatches through @c meosType and works
 * correctly for @c T_TPCPOINT because Phase 8D wired the base type into
 * @c basetype_in_state / @c basetype_out / @c datum_cmp / @c datum_eq /
 * @c datum_hash. This file holds only the per-type accessors that need
 * to know they are looking at pcpoint data — namely @c pcid and the
 * schema-aware dimension projections.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* pgpointcloud */
#include "pc_api.h"
/* MEOS — we cannot include utils/builtins.h here: meos_internal.h pulls
 * in <json-c/json.h>, whose `struct json_object` collides with PG's
 * `Datum json_object(PG_FUNCTION_ARGS)` in utils/fmgrprotos.h. The
 * two headers are mutually exclusive. For `text_to_cstring` we inline
 * an equivalent below. */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "geo/tgeo_spatialfuncs.h"  /* geopoint_make */
#include "pointcloud/pcpoint.h"
/* MobilityDB */
#include "pg_pointcloud/schema_cache.h"

/* Local minimal text→cstring — see rationale above. */
static char *
tpcpoint_text_to_cstring(const text *t)
{
  int len = (int) VARSIZE_ANY_EXHDR(t);
  char *result = palloc(len + 1);
  memcpy(result, VARDATA_ANY(t), (size_t) len);
  result[len] = '\0';
  return result;
}

/*****************************************************************************
 * Per-type pcid accessor
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpoint_pcid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_pcid);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the pgpointcloud schema id (pcid) of a tpcpoint.
 * @details All instants of a tpcpoint must share the same pcid — the
 *   Phase 8E set_make_exp enforcement guarantees this at construction
 *   time. This function just reads the first instant's pcid.
 * @sqlfn pcid()
 */
Datum
Tpcpoint_pcid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum first = temporal_start_value(temp);
  const Pcpoint *pt = (const Pcpoint *) DatumGetPointer(first);
  uint32_t pcid = pcpoint_pcid(pt);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32((int32) pcid);
}

/*****************************************************************************
 * Per-dimension projection — tpcpoint → tfloat (Phase 8J)
 *
 * Walks the Temporal structure and replaces each pcpoint instant with a
 * @c float8 derived from the pcpoint's value for the requested
 * dimension. Because the dimension-read path is schema-aware (lives in
 * libpc.a and needs a PCSCHEMA), the projection is expressed as a
 * function pointer (pcpoint → double) that the caller binds against
 * @c pc_point_get_x / _y / _z / _get_double_by_name.
 *
 * The schema cache is warmed on the first instant (all subsequent
 * instants share the same pcid by construction — enforced at build
 * time by Phase 8E's set_make_exp).
 *****************************************************************************/

typedef bool (*pcpoint_dim_fn)(const PCPOINT *pt, void *extra, double *out);

static bool
dim_get_x(const PCPOINT *pt, void *extra, double *out)
{ (void) extra; return pc_point_get_x((PCPOINT *) pt, out); }

static bool
dim_get_y(const PCPOINT *pt, void *extra, double *out)
{ (void) extra; return pc_point_get_y((PCPOINT *) pt, out); }

static bool
dim_get_z(const PCPOINT *pt, void *extra, double *out)
{ (void) extra; return pc_point_get_z((PCPOINT *) pt, out); }

static bool
dim_get_by_name(const PCPOINT *pt, void *extra, double *out)
{
  const char *name = (const char *) extra;
  return pc_point_get_double_by_name((PCPOINT *) pt, name, out) != 0;
}

static TInstant *
tpcpointinst_project(const TInstant *inst, pcpoint_dim_fn fn, void *extra)
{
  const Pcpoint *pt =
    (const Pcpoint *) DatumGetPointer(tinstant_value_p(inst));
  PCSCHEMA *schema = mobilitydb_pc_schema(pt->pcid);
  PCPOINT pcpt;
  pcpt.readonly = 1;
  pcpt.schema = schema;
  pcpt.data = (uint8_t *) pt->data;
  double v;
  if (! fn(&pcpt, extra, &v))
    return NULL;
  return tinstant_make(Float8GetDatum(v), T_TFLOAT, inst->t);
}

static TSequence *
tpcpointseq_project(const TSequence *seq, pcpoint_dim_fn fn, void *extra)
{
  TInstant **insts = palloc(sizeof(TInstant *) * seq->count);
  int n = 0;
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *out = tpcpointinst_project(TSEQUENCE_INST_N(seq, i), fn, extra);
    if (! out) continue;
    insts[n++] = out;
  }
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  return tsequence_make_free(insts, n, seq->period.lower_inc,
    seq->period.upper_inc, interp, NORMALIZE);
}

static TSequenceSet *
tpcpointseqset_project(const TSequenceSet *ss, pcpoint_dim_fn fn, void *extra)
{
  TSequence **seqs = palloc(sizeof(TSequence *) * ss->count);
  int n = 0;
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *out = tpcpointseq_project(TSEQUENCESET_SEQ_N(ss, i), fn, extra);
    if (! out) continue;
    seqs[n++] = out;
  }
  return tsequenceset_make_free(seqs, n, NORMALIZE);
}

static Temporal *
tpcpoint_project(const Temporal *temp, pcpoint_dim_fn fn, void *extra)
{
  assert(temp->temptype == T_TPCPOINT);
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tpcpointinst_project((TInstant *) temp, fn, extra);
    case TSEQUENCE:
      return (Temporal *) tpcpointseq_project((TSequence *) temp, fn, extra);
    default: /* TSEQUENCESET */
      return (Temporal *)
        tpcpointseqset_project((TSequenceSet *) temp, fn, extra);
  }
}

#define TPCPOINT_PROJ(fn_name, dim_fn) \
  PGDLLEXPORT Datum fn_name(PG_FUNCTION_ARGS); \
  PG_FUNCTION_INFO_V1(fn_name); \
  Datum fn_name(PG_FUNCTION_ARGS) \
  { \
    Temporal *temp = PG_GETARG_TEMPORAL_P(0); \
    Temporal *result = tpcpoint_project(temp, dim_fn, NULL); \
    PG_FREE_IF_COPY(temp, 0); \
    if (! result) PG_RETURN_NULL(); \
    PG_RETURN_POINTER(result); \
  }

TPCPOINT_PROJ(Tpcpoint_get_x, dim_get_x)
TPCPOINT_PROJ(Tpcpoint_get_y, dim_get_y)
TPCPOINT_PROJ(Tpcpoint_get_z, dim_get_z)

/*****************************************************************************
 * Projection to tgeompoint (Phase 8K)
 *
 * Per-instant mapper: pcpoint → POINT gserialized with the schema's
 * SRID and Z-presence. Reuses the schema cache on the first instant;
 * the same-pcid invariant (enforced at construction by Phase 8E) lets
 * us cache schema + z-flag once per sequence.
 *****************************************************************************/

static TInstant *
tpcpointinst_tgeompointinst(const TInstant *inst, const PCSCHEMA *schema)
{
  const Pcpoint *pt =
    (const Pcpoint *) DatumGetPointer(tinstant_value_p(inst));
  PCPOINT pcpt;
  pcpt.readonly = 1;
  pcpt.schema = schema;
  pcpt.data = (uint8_t *) pt->data;
  double x, y, z = 0.0;
  if (! pc_point_get_x(&pcpt, &x) || ! pc_point_get_y(&pcpt, &y))
    return NULL;
  bool hasz = (schema->zdim != NULL);
  if (hasz && ! pc_point_get_z(&pcpt, &z))
    return NULL;
  GSERIALIZED *gs = geopoint_make(x, y, z, hasz, /* geodetic */ false,
    (int32_t) schema->srid);
  TInstant *result = tinstant_make(PointerGetDatum(gs), T_TGEOMPOINT, inst->t);
  pfree(gs);
  return result;
}

static TSequence *
tpcpointseq_tgeompointseq(const TSequence *seq, const PCSCHEMA *schema)
{
  TInstant **insts = palloc(sizeof(TInstant *) * seq->count);
  int n = 0;
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *out = tpcpointinst_tgeompointinst(TSEQUENCE_INST_N(seq, i),
      schema);
    if (! out) continue;
    insts[n++] = out;
  }
  /* Step-interpolated in; linear is semantically meaningful for the
   * projected XY path (the sensor physically moved), so we promote. */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  if (interp == STEP) interp = LINEAR;
  return tsequence_make_free(insts, n, seq->period.lower_inc,
    seq->period.upper_inc, interp, NORMALIZE);
}

static TSequenceSet *
tpcpointseqset_tgeompointseqset(const TSequenceSet *ss, const PCSCHEMA *schema)
{
  TSequence **seqs = palloc(sizeof(TSequence *) * ss->count);
  int n = 0;
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *out = tpcpointseq_tgeompointseq(TSEQUENCESET_SEQ_N(ss, i),
      schema);
    if (! out) continue;
    seqs[n++] = out;
  }
  return tsequenceset_make_free(seqs, n, NORMALIZE);
}

PGDLLEXPORT Datum Tpcpoint_to_tgeompoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_to_tgeompoint);
/**
 * @ingroup mobilitydb_pointcloud_conversion
 * @brief Project a tpcpoint onto a tgeompoint by extracting X/Y/[Z]
 *   from each instant's pcpoint via the schema cache.
 * @sqlfn tgeompoint()
 */
Datum
Tpcpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* All instants share the same pcid; read it from the first. */
  const Pcpoint *first =
    (const Pcpoint *) DatumGetPointer(temporal_start_value(temp));
  PCSCHEMA *schema = mobilitydb_pc_schema(first->pcid);
  Temporal *result;
  switch (temp->subtype)
  {
    case TINSTANT:
      result = (Temporal *)
        tpcpointinst_tgeompointinst((TInstant *) temp, schema);
      break;
    case TSEQUENCE:
      result = (Temporal *)
        tpcpointseq_tgeompointseq((TSequence *) temp, schema);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal *)
        tpcpointseqset_tgeompointseqset((TSequenceSet *) temp, schema);
  }
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpcpoint_get_dim(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpoint_get_dim);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the per-instant projection of a tpcpoint onto a named
 *   dimension (Intensity, GpsTime, etc.) as a tfloat.
 * @sqlfn getDim()
 */
Datum
Tpcpoint_get_dim(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *name_txt = PG_GETARG_TEXT_P(1);
  char *name = tpcpoint_text_to_cstring(name_txt);
  Temporal *result = tpcpoint_project(temp, dim_get_by_name, name);
  pfree(name);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(name_txt, 1);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
