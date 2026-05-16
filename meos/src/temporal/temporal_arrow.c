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
 * @brief Conversion between temporal values and the Arrow C Data Interface
 * @details The Arrow encoding is a length-one `Struct` carrying the temporal
 * value verbatim:
 * @code
 * Struct {
 *   subtype: int8,            -- temporal subtype
 *   interp:  int8,            -- interpolation
 *   flags:   int16,           -- MEOS flags, carried unaltered
 *   srid:    int32,           -- spatial reference, 0 when not spatial
 *   seqs: List< Struct {      -- one element per component sequence
 *     lower_inc: bool,
 *     upper_inc: bool,
 *     insts: List< Struct { t: timestamp[us,UTC], v: <base> } >
 *   } >
 * }
 * @endcode
 * The structural skeleton is the published contract and does not change as
 * base types are added: a new base type only swaps the `v` leaf. Any temporal
 * type, subtype, or interpolation not yet wired raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED rather than producing an approximate or
 * silent result. All temporal float subtypes (instant, sequence, sequence
 * set) and interpolations (discrete, step, linear) are wired; other base
 * types are not yet. The producer owns all memory through arenas freed by
 * the top-level release
 * callbacks (one arena per Arrow object, since the consumer releases the
 * schema and the array independently); child release callbacks are no-ops.
 */

#include "temporal/temporal.h"

/* C */
#include <assert.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <arrow_c_data_interface.h>
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "temporal/tinstant.h"

/*****************************************************************************
 * Arena ownership: every allocation for one converted value lives in a
 * singly-linked list of blocks freed once by the top-level release.
 *****************************************************************************/

typedef struct ArrowArena
{
  void **blocks;
  int count;
  int capacity;
} ArrowArena;

static void *
arena_alloc(ArrowArena *arena, size_t size)
{
  void *p = palloc0(size);
  if (arena->count == arena->capacity)
  {
    arena->capacity = arena->capacity ? arena->capacity * 2 : 16;
    arena->blocks = arena->blocks ?
      repalloc(arena->blocks, sizeof(void *) * arena->capacity) :
      palloc(sizeof(void *) * arena->capacity);
  }
  arena->blocks[arena->count++] = p;
  return p;
}

static void
arena_free(ArrowArena *arena)
{
  for (int i = 0; i < arena->count; i++)
    pfree(arena->blocks[i]);
  if (arena->blocks)
    pfree(arena->blocks);
  pfree(arena);
}

/* Child nodes are arena-owned; their release is a no-op. */
static void
noop_schema_release(struct ArrowSchema *s) { s->release = NULL; }
static void
noop_array_release(struct ArrowArray *a) { a->release = NULL; }

static void
root_schema_release(struct ArrowSchema *s)
{
  if (s->private_data)
    arena_free((ArrowArena *) s->private_data);
  s->private_data = NULL;
  s->release = NULL;
}

static void
root_array_release(struct ArrowArray *a)
{
  if (a->private_data)
    arena_free((ArrowArena *) a->private_data);
  a->private_data = NULL;
  a->release = NULL;
}

/*****************************************************************************
 * Schema node builders (arena-allocated). The root struct is the
 * caller-allocated out_schema; every descendant lives in the arena.
 *****************************************************************************/

static struct ArrowSchema *
aw_schema_leaf(ArrowArena *arena, const char *format, const char *name)
{
  struct ArrowSchema *s = arena_alloc(arena, sizeof(struct ArrowSchema));
  s->format = format;
  s->name = name;
  s->metadata = NULL;
  s->flags = 0;
  s->n_children = 0;
  s->children = NULL;
  s->dictionary = NULL;
  s->release = noop_schema_release;
  s->private_data = NULL;
  return s;
}

static struct ArrowSchema *
aw_schema_struct(ArrowArena *arena, const char *name, struct ArrowSchema **kids,
  int64_t n)
{
  struct ArrowSchema *s = aw_schema_leaf(arena, "+s", name);
  s->n_children = n;
  s->children = kids;
  return s;
}

static struct ArrowSchema *
aw_schema_list(ArrowArena *arena, const char *name, struct ArrowSchema *item)
{
  struct ArrowSchema *s = aw_schema_leaf(arena, "+l", name);
  s->n_children = 1;
  s->children = arena_alloc(arena, sizeof(struct ArrowSchema *));
  s->children[0] = item;
  return s;
}

/*****************************************************************************
 * Array node builders (arena-allocated). Validity buffers are NULL because
 * a converted temporal value has no Arrow-level nulls.
 *****************************************************************************/

static struct ArrowArray *
aw_array_node(ArrowArena *arena, int64_t length, int64_t n_buffers,
  int64_t n_children)
{
  struct ArrowArray *a = arena_alloc(arena, sizeof(struct ArrowArray));
  a->length = length;
  a->null_count = 0;
  a->offset = 0;
  a->n_buffers = n_buffers;
  a->n_children = n_children;
  a->buffers = n_buffers ?
    arena_alloc(arena, sizeof(void *) * n_buffers) : NULL;
  a->children = n_children ?
    arena_alloc(arena, sizeof(struct ArrowArray *) * n_children) : NULL;
  a->dictionary = NULL;
  a->release = noop_array_release;
  a->private_data = NULL;
  return a;
}

static struct ArrowArray *
aw_array_prim(ArrowArena *arena, int64_t length, const void *data)
{
  struct ArrowArray *a = aw_array_node(arena, length, 2, 0);
  a->buffers[0] = NULL;
  a->buffers[1] = data;
  return a;
}

static struct ArrowArray *
aw_array_list(ArrowArena *arena, int64_t length, const int32_t *offsets,
  struct ArrowArray *item)
{
  struct ArrowArray *a = aw_array_node(arena, length, 2, 1);
  a->buffers[0] = NULL;
  a->buffers[1] = offsets;
  a->children[0] = item;
  return a;
}

static struct ArrowArray *
aw_array_struct(ArrowArena *arena, int64_t length, struct ArrowArray **kids,
  int64_t n)
{
  struct ArrowArray *a = aw_array_node(arena, length, 1, n);
  a->buffers[0] = NULL;
  a->children = kids;
  return a;
}

/*****************************************************************************
 * Value-leaf dispatch (the only part of the schema that varies by base type:
 * temporal float decomposes to Float64 "g", temporal integer to Int32 "i")
 *****************************************************************************/

/**
 * @brief Return the @p idx-th decomposed scalar value from an Arrow value
 * buffer as a Datum, dispatching on the base type
 */
static Datum
aw_leaf_datum(bool is_tint, const void *vvals, int idx)
{
  return is_tint ? Int32GetDatum(((const int32_t *) vvals)[idx]) :
    Float8GetDatum(((const double *) vvals)[idx]);
}

/*****************************************************************************
 * Public conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_conversion
 * @brief Convert a temporal value into Arrow C Data Interface structures
 * @param[in] temp Temporal value
 * @param[out] out_schema,out_array Caller-allocated Arrow structures filled
 * by the producer; release with their @p release callbacks
 * @return True on success; on an unsupported input raises
 * @p MEOS_ERR_FEATURE_NOT_SUPPORTED and returns false
 * @note All temporal float and temporal integer subtypes and interpolations
 * are wired; other base types are not yet. The Arrow schema is the full
 * contract.
 */
bool
meos_temporal_to_arrow(const Temporal *temp, struct ArrowSchema *out_schema,
  struct ArrowArray *out_array)
{
  if (! temp || ! out_schema || ! out_array)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG, "Null argument to to_arrow");
    return false;
  }
  if (temp->temptype != T_TFLOAT && temp->temptype != T_TINT)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_to_arrow: only temporal float and integer are wired");
    return false;
  }
  bool is_tint = (temp->temptype == T_TINT);

  /* Separate arenas: the schema and the array are independently released
   * by the consumer, so each must own (and free) its own allocations. */
  ArrowArena *arena_s = palloc0(sizeof(ArrowArena));
  ArrowArena *arena_a = palloc0(sizeof(ArrowArena));

  /* Schema: the full nested contract. The value leaf "g" is the only part
   * that varies by base type; everything else is fixed structure. */
  struct ArrowSchema *inst_kids[2];
  inst_kids[0] = aw_schema_leaf(arena_s, "tsu:UTC", "t");
  inst_kids[1] = aw_schema_leaf(arena_s, is_tint ? "i" : "g", "v");
  struct ArrowSchema *inst_st =
    aw_schema_struct(arena_s, "item", inst_kids, 2);
  struct ArrowSchema *insts_list = aw_schema_list(arena_s, "insts", inst_st);

  struct ArrowSchema *seq_kids[3];
  seq_kids[0] = aw_schema_leaf(arena_s, "b", "lower_inc");
  seq_kids[1] = aw_schema_leaf(arena_s, "b", "upper_inc");
  seq_kids[2] = insts_list;
  struct ArrowSchema *seq_st = aw_schema_struct(arena_s, "item", seq_kids, 3);
  struct ArrowSchema *seqs_list = aw_schema_list(arena_s, "seqs", seq_st);

  struct ArrowSchema **top_sc =
    arena_alloc(arena_s, sizeof(struct ArrowSchema *) * 5);
  top_sc[0] = aw_schema_leaf(arena_s, "c", "subtype");
  top_sc[1] = aw_schema_leaf(arena_s, "c", "interp");
  top_sc[2] = aw_schema_leaf(arena_s, "s", "flags");
  top_sc[3] = aw_schema_leaf(arena_s, "i", "srid");
  top_sc[4] = seqs_list;

  out_schema->format = "+s";
  out_schema->name = NULL;
  out_schema->metadata = NULL;
  out_schema->flags = 0;
  out_schema->n_children = 5;
  out_schema->children = top_sc;
  out_schema->dictionary = NULL;
  out_schema->private_data = arena_s;
  out_schema->release = root_schema_release;

  /* Scalar header buffers (length 1: one temporal value) */
  int8_t *subtype_b = arena_alloc(arena_a, sizeof(int8_t));
  int8_t *interp_b = arena_alloc(arena_a, sizeof(int8_t));
  int16_t *flags_b = arena_alloc(arena_a, sizeof(int16_t));
  int32_t *srid_b = arena_alloc(arena_a, sizeof(int32_t));
  subtype_b[0] = (int8_t) temp->subtype;
  interp_b[0] = (int8_t) MEOS_FLAGS_GET_INTERP(temp->flags);
  flags_b[0] = (int16_t) temp->flags;
  srid_b[0] = 0;

  /* Decompose into component sequences. The skeleton is uniform: an instant
   * is one pseudo-sequence of one instant; a sequence is one sequence; a
   * sequence set is its component sequences. */
  int nseqs, total;
  const TInstant *single = NULL;
  const TSequence **comp = NULL;
  if (temp->subtype == TINSTANT)
  {
    single = (const TInstant *) temp;
    nseqs = 1;
    total = 1;
  }
  else if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    nseqs = 1;
    total = seq->count;
    comp = arena_alloc(arena_a, sizeof(TSequence *));
    comp[0] = seq;
  }
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    nseqs = ss->count;
    total = ss->totalcount;
    comp = arena_alloc(arena_a, sizeof(TSequence *) * nseqs);
    for (int j = 0; j < nseqs; j++)
      comp[j] = TSEQUENCESET_SEQ_N(ss, j);
  }

  /* Per-sequence bound bitmaps (Arrow boolean is one bit per element),
   * the two levels of list offsets, and the flattened instant columns. */
  uint8_t *lower_bm = arena_alloc(arena_a, (nseqs + 7) / 8);
  uint8_t *upper_bm = arena_alloc(arena_a, (nseqs + 7) / 8);
  int32_t *seqs_off = arena_alloc(arena_a, sizeof(int32_t) * 2);
  seqs_off[0] = 0;
  seqs_off[1] = nseqs;
  int32_t *insts_off = arena_alloc(arena_a, sizeof(int32_t) * (nseqs + 1));
  insts_off[0] = 0;
  int64_t *tvals = arena_alloc(arena_a, sizeof(int64_t) * (total ? total : 1));
  size_t velt = is_tint ? sizeof(int32_t) : sizeof(double);
  void *vvals = arena_alloc(arena_a, velt * (total ? total : 1));
  int k = 0;
  for (int j = 0; j < nseqs; j++)
  {
    bool lo, up;
    if (single)
    {
      lo = up = true;
      tvals[k] = (int64_t) single->t;
      if (is_tint)
        ((int32_t *) vvals)[k] = DatumGetInt32(tinstant_value_p(single));
      else
        ((double *) vvals)[k] = DatumGetFloat8(tinstant_value_p(single));
      k++;
    }
    else
    {
      const TSequence *seq = comp[j];
      lo = seq->period.lower_inc;
      up = seq->period.upper_inc;
      for (int i = 0; i < seq->count; i++)
      {
        const TInstant *inst = TSEQUENCE_INST_N(seq, i);
        tvals[k] = (int64_t) inst->t;
        if (is_tint)
          ((int32_t *) vvals)[k] = DatumGetInt32(tinstant_value_p(inst));
        else
          ((double *) vvals)[k] = DatumGetFloat8(tinstant_value_p(inst));
        k++;
      }
    }
    if (lo) lower_bm[j >> 3] |= (uint8_t) (1 << (j & 7));
    if (up) upper_bm[j >> 3] |= (uint8_t) (1 << (j & 7));
    insts_off[j + 1] = k;
  }

  /* Array tree mirroring the schema */
  struct ArrowArray **inst_a_kids =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 2);
  inst_a_kids[0] = aw_array_prim(arena_a, total, tvals);
  inst_a_kids[1] = aw_array_prim(arena_a, total, vvals);
  struct ArrowArray *inst_st_a =
    aw_array_struct(arena_a, total, inst_a_kids, 2);
  struct ArrowArray *insts_list_a =
    aw_array_list(arena_a, nseqs, insts_off, inst_st_a);

  struct ArrowArray **seq_a_kids =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 3);
  seq_a_kids[0] = aw_array_prim(arena_a, nseqs, lower_bm);
  seq_a_kids[1] = aw_array_prim(arena_a, nseqs, upper_bm);
  seq_a_kids[2] = insts_list_a;
  struct ArrowArray *seq_st_a = aw_array_struct(arena_a, nseqs, seq_a_kids, 3);
  struct ArrowArray *seqs_list_a =
    aw_array_list(arena_a, 1, seqs_off, seq_st_a);

  struct ArrowArray **top_a =
    arena_alloc(arena_a, sizeof(struct ArrowArray *) * 5);
  top_a[0] = aw_array_prim(arena_a, 1, subtype_b);
  top_a[1] = aw_array_prim(arena_a, 1, interp_b);
  top_a[2] = aw_array_prim(arena_a, 1, flags_b);
  top_a[3] = aw_array_prim(arena_a, 1, srid_b);
  top_a[4] = seqs_list_a;

  out_array->length = 1;
  out_array->null_count = 0;
  out_array->offset = 0;
  out_array->n_buffers = 1;
  out_array->n_children = 5;
  out_array->buffers = arena_alloc(arena_a, sizeof(void *) * 1);
  out_array->buffers[0] = NULL;
  out_array->children = top_a;
  out_array->dictionary = NULL;
  out_array->private_data = arena_a;
  out_array->release = root_array_release;
  return true;
}

/**
 * @ingroup meos_temporal_conversion
 * @brief Reconstruct a temporal value from Arrow C Data Interface structures
 * @param[in] schema,array Arrow structures produced by
 * #meos_temporal_to_arrow
 * @return New temporal value, or NULL on an unsupported layout
 * (raises @p MEOS_ERR_FEATURE_NOT_SUPPORTED)
 */
Temporal *
meos_temporal_from_arrow(const struct ArrowSchema *schema,
  const struct ArrowArray *array)
{
  if (! schema || ! array || ! schema->format ||
      strcmp(schema->format, "+s") != 0 || schema->n_children != 5 ||
      array->n_children != 5 || array->length != 1)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: unsupported Arrow layout");
    return NULL;
  }

  /* The base-type leaf gates this: an unrecognised value column routes to
   * NOT_SUPPORTED rather than being misread. Float64 "g" is temporal float,
   * Int32 "i" is temporal integer. */
  const struct ArrowSchema *v_sc = schema->children[4]    /* seqs list */
    ->children[0]                                          /* seq struct */
    ->children[2]                                          /* insts list */
    ->children[0]                                          /* inst struct */
    ->children[1];                                         /* v leaf */
  int8_t subtype = ((const int8_t *) array->children[0]->buffers[1])[0];
  int8_t interp = ((const int8_t *) array->children[1]->buffers[1])[0];
  int32_t srid = ((const int32_t *) array->children[3]->buffers[1])[0];
  bool is_tint = v_sc->format && strcmp(v_sc->format, "i") == 0;
  if (! v_sc->format ||
      (strcmp(v_sc->format, "g") != 0 && strcmp(v_sc->format, "i") != 0) ||
      srid != 0 ||
      (subtype != TINSTANT && subtype != TSEQUENCE &&
       subtype != TSEQUENCESET))
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: only temporal float and integer are wired");
    return NULL;
  }

  const struct ArrowArray *seq_st_a = array->children[4]->children[0];
  const uint8_t *lower_bm =
    (const uint8_t *) seq_st_a->children[0]->buffers[1];
  const uint8_t *upper_bm =
    (const uint8_t *) seq_st_a->children[1]->buffers[1];
  const struct ArrowArray *insts_a = seq_st_a->children[2];
  const int32_t *insts_off = (const int32_t *) insts_a->buffers[1];
  const struct ArrowArray *inst_st_a = insts_a->children[0];
  const int64_t *tvals = (const int64_t *) inst_st_a->children[0]->buffers[1];
  const void *vvals = inst_st_a->children[1]->buffers[1];
  int nseqs = (int) seq_st_a->length;
  int total = nseqs > 0 ? insts_off[nseqs] - insts_off[0] : 0;
  if (nseqs <= 0 || total <= 0)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "meos_temporal_from_arrow: empty value unsupported");
    return NULL;
  }

  if (subtype == TINSTANT)
    return (Temporal *) tinstant_make(aw_leaf_datum(is_tint, vvals, 0),
      is_tint ? T_TINT : T_TFLOAT, (TimestampTz) tvals[0]);

  interpType ip = (interpType) interp;
  if (subtype == TSEQUENCE)
  {
    int n = insts_off[1] - insts_off[0];
    bool lo = (lower_bm[0] & 0x01) != 0;
    bool up = (upper_bm[0] & 0x01) != 0;
    TInstant **instants = palloc(sizeof(TInstant *) * n);
    for (int i = 0; i < n; i++)
      instants[i] = tinstant_make(aw_leaf_datum(is_tint, vvals, i),
        is_tint ? T_TINT : T_TFLOAT, (TimestampTz) tvals[i]);
    TSequence *result = tsequence_make(instants, n, lo, up, ip, true);
    for (int i = 0; i < n; i++)
      pfree(instants[i]);
    pfree(instants);
    return (Temporal *) result;
  }

  /* TSEQUENCESET: rebuild each component sequence, then assemble */
  TSequence **seqs = palloc(sizeof(TSequence *) * nseqs);
  for (int j = 0; j < nseqs; j++)
  {
    int lo_off = insts_off[j], cnt = insts_off[j + 1] - lo_off;
    bool lo = ((lower_bm[j >> 3] >> (j & 7)) & 1) != 0;
    bool up = ((upper_bm[j >> 3] >> (j & 7)) & 1) != 0;
    TInstant **instants = palloc(sizeof(TInstant *) * cnt);
    for (int i = 0; i < cnt; i++)
      instants[i] = tinstant_make(aw_leaf_datum(is_tint, vvals, lo_off + i),
        is_tint ? T_TINT : T_TFLOAT, (TimestampTz) tvals[lo_off + i]);
    seqs[j] = tsequence_make(instants, cnt, lo, up, ip, true);
    for (int i = 0; i < cnt; i++)
      pfree(instants[i]);
    pfree(instants);
  }
  TSequenceSet *result = tsequenceset_make(seqs, nseqs, true);
  for (int j = 0; j < nseqs; j++)
    pfree(seqs[j]);
  pfree(seqs);
  return (Temporal *) result;
}

/**
 * @ingroup meos_temporal_conversion
 * @brief Round-trip a temporal value through the Arrow C Data Interface
 * @details Convert @p temp to the Arrow C Data Interface and reconstruct it,
 * returning the new value. The result is equal to @p temp; an unsupported
 * base type raises @p MEOS_ERR_FEATURE_NOT_SUPPORTED. Validates the lossless
 * correspondence and is the interop self-check reused by language bindings.
 * @param[in] temp Temporal value
 * @return New temporal value equal to @p temp, or @p NULL on an unsupported
 * base type
 */
Temporal *
meos_temporal_arrow_roundtrip(const Temporal *temp)
{
  struct ArrowSchema schema = {0};
  struct ArrowArray array = {0};
  if (! meos_temporal_to_arrow(temp, &schema, &array))
    return NULL;
  Temporal *result = meos_temporal_from_arrow(&schema, &array);
  if (schema.release)
    schema.release(&schema);
  if (array.release)
    array.release(&array);
  return result;
}

/*****************************************************************************/
