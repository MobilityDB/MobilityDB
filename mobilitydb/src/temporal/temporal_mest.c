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
 * @brief Multi-entry support of the temporal types: the keys of a value are
 * the boxes MEOS splits it into
 * @details The access methods of the mest extension, mgist and mspgist, index
 * a value by several keys, which its operator classes, in the extension
 * mobilitydb_mest, extract from the value. The keys of a temporal value are the
 * boxes of its pieces, never the box of the whole value, so a class of these
 * holds the predicates on the value, which a piece answers, and none of the
 * operators answered on the bounding box of the value. The consistent
 * functions of the GiST and SP-GiST classes serve these classes unchanged:
 * they recheck the predicates on the value at every key. What is proper to a
 * multi-entry class is the extraction of the keys, the parameters of the split
 * as options of the class, and a compression that keeps a key as it is, since
 * a key is already a box.
 */

#include "pg_temporal/temporal_mest.h"

/* C */
#include <assert.h>
#include <float.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/reloptions.h>
#include <access/spgist.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <pgtypes.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/temporal_tile.h"
#include "temporal/type_util.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Options of the splits
 *****************************************************************************/

/** Default number of boxes of a value or of elements of a box */
#define MEST_COUNT_DEFAULT    1
/** Largest number of boxes of a value or of elements of a box */
#define MEST_COUNT_MAX        1000
/** Default size of a tile in the value dimension */
#define MEST_VSIZE_DEFAULT    1.0
/** Largest size of a tile in the value dimension */
#define MEST_VSIZE_MAX        1000000.0
/** Origin of the time bins of a tile, the default of the tiling functions */
#define MEST_TORIGIN          "2000-01-03"

/**
 * @brief Options of a split into a number of boxes or of elements per box
 */
typedef struct
{
  int32 vl_len_;    /**< Varlena header (do not touch directly) */
  int count;        /**< Number of boxes, or of elements per box */
} MestCountOptions;

/**
 * @brief Options of a split into the tiles of a value and time grid
 */
typedef struct
{
  int32 vl_len_;    /**< Varlena header (do not touch directly) */
  double vsize;     /**< Size of a tile in the value dimension */
  int duration;     /**< Size of a tile in time, an interval string */
} MestTileOptions;

/**
 * @brief Options of a split into the time bins of a value
 */
typedef struct
{
  int32 vl_len_;    /**< Varlena header (do not touch directly) */
  int duration;     /**< Size of a bin, an interval string */
} MestBinOptions;

/** Default size of a time bin */
#define MEST_DURATION_DEFAULT  "1 day"

/**
 * @brief Return the number the options of the class state, or the default
 * @param[in] fcinfo Call of the extract method
 */
int
mest_count(FunctionCallInfo fcinfo)
{
  return PG_HAS_OPCLASS_OPTIONS() ?
    ((MestCountOptions *) PG_GET_OPCLASS_OPTIONS())->count :
    MEST_COUNT_DEFAULT;
}

/**
 * @brief Return the keys of a multi-entry index, the boxes of an array as
 * Datums
 * @param[in] boxes Array of boxes
 * @param[in] size Size of a box
 * @param[in] count Number of boxes
 */
Datum *
mest_keys(void *boxes, size_t size, int count)
{
  assert(boxes); assert(count > 0);
  Datum *result = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
    result[i] = PointerGetDatum((char *) boxes + i * size);
  return result;
}

/**
 * @brief Return the keys of a multi-entry index over the boxes of the pieces
 * of a temporal value followed by the box of the whole value
 * @details The box of the whole value carries the flag @p MEOS_FLAG_WHOLE, a
 * structure of its own (#bbox_same_structure), so that an index keeps it apart
 * from the boxes of the pieces: a box operator reads the keys of the whole
 * values and a value predicate those of the pieces
 * @param[in] temp Temporal value, whose box is a `tbox` or an `stbox`
 * @param[in] boxes Boxes of the pieces, allocated with palloc
 * @param[in] size Size of a box
 * @param[in,out] count Number of boxes of the pieces, then of keys
 */
Datum *
mest_keys_whole(const Temporal *temp, void *boxes, size_t size, int32 *count)
{
  assert(temp); assert(boxes); assert(count); assert(*count > 0);
  /* A fresh array, the boxes of the pieces being the splitter's own */
  char *keys = palloc(size * (*count + 1));
  memcpy(keys, boxes, size * (*count));
  void *whole = keys + size * (*count);
  temporal_set_bbox(temp, whole);
  int16 *flags = tnumber_type(temp->temptype) ? &((TBox *) whole)->flags :
    &((STBox *) whole)->flags;
  MEOS_FLAGS_SET_WHOLE(*flags, true);
  (*count)++;
  return mest_keys(keys, size, *count);
}

/* GENERATED-MESTTIERS-BEGIN — tools/codegen/inherited/generate.py from templates/mesttiers.c.tmpl;
 * DO NOT EDIT BY HAND; edit the template + manifest.d/meststrategies.yaml and re-run. */

/**
 * @brief Return true if a key of a multi-entry index may answer a strategy
 * @details The key of the box of a whole value (#mest_keys_whole) answers the
 * strategies whose tier is that of the whole value, the operators reading the
 * interior or a face of the box of the value, which no piece witnesses, and
 * the same box, which it answers exactly. The keys of the pieces answer every
 * other operator, each holding on the box of a value iff on the box of one of
 * its pieces or of every one. A key over the keys of several structures
 * stands for both and answers any
 * @param[in] flags Flags of the key
 * @param[in] strategy Strategy number of the operator
 */
bool
mest_key_answers(int16 flags, StrategyNumber strategy)
{
  if (MEOS_FLAGS_GET_PARTS(flags))
    return true;
  bool whole = (strategy == RTOverlapStrategyNumber ||
    strategy == RTSameStrategyNumber ||
    strategy == RTContainsStrategyNumber ||
    strategy == RTAdjacentStrategyNumber);
  return MEOS_FLAGS_GET_WHOLE(flags) == whole;
}

/**
 * @brief Return the strategy a key of a piece of a value answers in place of
 * a strategy of a multi-entry index whose keys are spans
 * @details A class whose keys are spans keys the pieces of a value, the
 * components of a span set, the elements of a set or the pieces of a temporal
 * value, a set or a span set also by its extent, which a scan that is not
 * ordered leaves out (#mest_span_key_answers). The span of a value is the same as a
 * query, or a value equals it, only when every piece lies within the query, so
 * a key of a piece answers them as contained by; a value contains a query
 * only when some piece meets it, so a key of a piece answers the contains as
 * the overlaps. Every other strategy is its own
 * @param[in] strategy Strategy number of the operator
 */
StrategyNumber
mest_pieces_strategy(StrategyNumber strategy)
{
  if (strategy == RTContainsStrategyNumber)
    return RTOverlapStrategyNumber;
  if (strategy == RTSameStrategyNumber ||
    strategy == RTEqualStrategyNumber)
    return RTContainedByStrategyNumber;
  return strategy;
}

/* GENERATED-MESTTIERS-END */

/**
 * @brief Return the scan keys a key of a piece of a value answers in place of
 * the scan keys of a multi-entry index whose keys are spans
 * @details The strategy of each key is the one #mest_pieces_strategy reads.
 * The result is the array of the scan keys itself when every key keeps its
 * strategy, else a copy allocated with palloc
 * @param[in] keys Scan keys
 * @param[in] nkeys Number of scan keys
 */
ScanKey
mest_pieces_scankeys(ScanKey keys, int nkeys)
{
  if (nkeys <= 0)
    return keys;
  int i;
  for (i = 0; i < nkeys; i++)
    if (mest_pieces_strategy(keys[i].sk_strategy) != keys[i].sk_strategy)
      break;
  if (i == nkeys)
    return keys;
  ScanKey result = palloc(sizeof(ScanKeyData) * nkeys);
  memcpy(result, keys, sizeof(ScanKeyData) * nkeys);
  for (i = 0; i < nkeys; i++)
    result[i].sk_strategy = mest_pieces_strategy(result[i].sk_strategy);
  return result;
}

/**
 * @brief Return true if a key of a multi-entry index whose keys are spans
 * answers a scan, ordered or not
 * @details A set or a span set is keyed by the spans of its elements or of
 * its components and by its extent, its bounding span, which carries the flag
 * @p MEOS_FLAG_WHOLE (#mest_span_keys_whole). The distance of the value is
 * that of its extent, which a key of a piece overstates, so an ordered scan
 * reads the key of the extent alone, which answers every strategy as the key
 * of a single-entry index does and the distance exactly; a scan that is not
 * ordered reads the keys of the pieces alone, which answer the strategies of
 * #mest_pieces_strategy more selectively. Each value is reported at the
 * smallest distance among its keys, and a key of a piece of an ordered scan
 * lies at the maximum one. The index keeps the keys of the extents and those
 * of the pieces in subtrees of their own, an inner key over both carrying the
 * flag @p MEOS_FLAG_PARTS, which answers any scan (#Span_mgist_union,
 * #Span_mquadtree_choose). The keys of a temporal value keyed by spans are
 * its pieces, whose classes hold no distance
 * @param[in] flags Flags of the key
 * @param[in] ordered True when the scan is ordered
 */
bool
mest_span_key_answers(int16 flags, bool ordered)
{
  return MEOS_FLAGS_GET_PARTS(flags) || MEOS_FLAGS_GET_WHOLE(flags) == ordered;
}

/**
 * @brief Return the keys of a multi-entry index over the spans of the pieces
 * of a set or a span set followed by its extent, as Datums
 * @details The extent carries the flag @p MEOS_FLAG_WHOLE, which the keys of
 * the pieces do not, whatever bytes the spans they were copied from held
 * (#mest_span_key_answers)
 * @param[in] spans Array of the spans of the pieces, freed
 * @param[in] extent Extent of the value
 * @param[in,out] count Number of spans of the pieces on input, of the keys on
 * output
 */
Datum *
mest_span_keys_whole(Span *spans, const Span *extent, int32 *count)
{
  assert(spans); assert(extent); assert(count); assert(*count > 0);
  Span *keys = palloc(sizeof(Span) * (*count + 1));
  memcpy(keys, spans, sizeof(Span) * *count);
  pfree(spans);
  for (int i = 0; i < *count; i++)
    keys[i].flags = 0;
  keys[*count] = *extent;
  keys[*count].flags = 0;
  MEOS_FLAGS_SET_WHOLE(keys[*count].flags, true);
  (*count)++;
  return mest_keys(keys, sizeof(Span), *count);
}

PG_FUNCTION_INFO_V1(Mest_count_options);
/**
 * @brief Options method of the multi-entry classes splitting a value into a
 * number of boxes or of elements per box
 */
Datum
Mest_count_options(PG_FUNCTION_ARGS)
{
  local_relopts *relopts = (local_relopts *) PG_GETARG_POINTER(0);
  init_local_reloptions(relopts, sizeof(MestCountOptions));
  add_local_int_reloption(relopts, "count",
    "number of boxes, or of elements per box, of the extract method",
    MEST_COUNT_DEFAULT, 1, MEST_COUNT_MAX, offsetof(MestCountOptions, count));
  PG_RETURN_VOID();
}

/**
 * @brief Raise an error unless the duration of a tile is empty or an
 * interval
 */
void
mest_duration_validate(const char *value)
{
  if (value && strlen(value) > 0)
    pfree(pg_interval_in(value, -1));
}

/**
 * @brief Copy the duration of a tile into the options and return its size
 */
Size
mest_duration_fill(const char *value, void *ptr)
{
  size_t len = value ? strlen(value) : 0;
  if (ptr)
  {
    if (value)
      memcpy(ptr, value, len + 1);
    else
      ((char *) ptr)[0] = '\0';
  }
  return len + 1;
}

PG_FUNCTION_INFO_V1(Mest_tile_options);
/**
 * @brief Options method of the multi-entry classes splitting a temporal
 * number into the tiles of a value and time grid
 */
Datum
Mest_tile_options(PG_FUNCTION_ARGS)
{
  local_relopts *relopts = (local_relopts *) PG_GETARG_POINTER(0);
  init_local_reloptions(relopts, sizeof(MestTileOptions));
  add_local_real_reloption(relopts, "vsize",
    "size of a tile in the value dimension", MEST_VSIZE_DEFAULT, 0.0,
    MEST_VSIZE_MAX, offsetof(MestTileOptions, vsize));
  add_local_string_reloption(relopts, "duration",
    "size of a tile in time, an interval; empty splits no time", "",
    &mest_duration_validate, &mest_duration_fill,
    offsetof(MestTileOptions, duration));
  PG_RETURN_VOID();
}

PG_FUNCTION_INFO_V1(Mest_bin_options);
/**
 * @brief Options method of the multi-entry classes splitting a temporal value
 * into its time bins
 * @details No other options method exists in MobilityDB; the option is the
 * duration argument of the time bins of a temporal value, #Temporal_time_bins
 */
Datum
Mest_bin_options(PG_FUNCTION_ARGS)
{
  local_relopts *relopts = (local_relopts *) PG_GETARG_POINTER(0);
  init_local_reloptions(relopts, sizeof(MestBinOptions));
  add_local_string_reloption(relopts, "duration",
    "size of a time bin, an interval", MEST_DURATION_DEFAULT,
    &mest_duration_validate, &mest_duration_fill,
    offsetof(MestBinOptions, duration));
  PG_RETURN_VOID();
}

/*****************************************************************************
 * Compression of a key
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Mest_gist_compress);
/**
 * @brief Multi-entry GiST compress method: a key is a box, kept as it is
 */
Datum
Mest_gist_compress(PG_FUNCTION_ARGS)
{
  PG_RETURN_POINTER(PG_GETARG_POINTER(0));
}

PG_FUNCTION_INFO_V1(Mest_spgist_compress);
/**
 * @brief Multi-entry SP-GiST compress method: a key is a box, kept as it is
 */
Datum
Mest_spgist_compress(PG_FUNCTION_ARGS)
{
  PG_RETURN_DATUM(PG_GETARG_DATUM(0));
}

/*****************************************************************************
 * Extract methods of the sets and span sets
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Set_mest_equisplit);
/**
 * @brief Multi-entry extract method of a set keyed by the spans of an element
 * or of several and by its extent (#mest_span_keys_whole)
 * @details The keys of a value are a number of spans of its consecutive
 * elements, the number being the option of the class (#mest_count) and the
 * split that of #set_split_n_spans
 */
Datum
Set_mest_equisplit(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  Span *spans = set_split_n_spans(s, mest_count(fcinfo), nkeys);
  Span extent;
  set_set_span(s, &extent);
  PG_RETURN_POINTER(mest_span_keys_whole(spans, &extent, nkeys));
}

PG_FUNCTION_INFO_V1(Set_mest_segsplit);
/**
 * @brief Multi-entry extract method of a set keyed by the spans of an element
 * or of several and by its extent (#mest_span_keys_whole)
 * @details The keys of a value are the spans of its consecutive elements a
 * number at a time, the number being the option of the class (#mest_count) and
 * the split that of #set_split_each_n_spans
 */
Datum
Set_mest_segsplit(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  Span *spans = set_split_each_n_spans(s, mest_count(fcinfo), nkeys);
  Span extent;
  set_set_span(s, &extent);
  PG_RETURN_POINTER(mest_span_keys_whole(spans, &extent, nkeys));
}

PG_FUNCTION_INFO_V1(Set_mest_mergesplit);
/**
 * @brief Multi-entry extract method of a set keyed by the spans of an element
 * or of several and by its extent (#mest_span_keys_whole)
 * @details The keys of a value are a number of spans merging the spans of its
 * elements least apart, the number being the option of the class (#mest_count)
 * and the split that of #set_merge_n_spans
 */
Datum
Set_mest_mergesplit(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  Span *spans = set_merge_n_spans(s, mest_count(fcinfo), nkeys);
  Span extent;
  set_set_span(s, &extent);
  PG_RETURN_POINTER(mest_span_keys_whole(spans, &extent, nkeys));
}

PG_FUNCTION_INFO_V1(Set_mest_adaptsplit);
/**
 * @brief Multi-entry extract method of a set keyed by the spans of an element
 * or of several and by its extent (#mest_span_keys_whole)
 * @details The keys of a value are the spans merging the spans of its elements
 * a number at a time, least apart, the number being the option of the class
 * (#mest_count) and the split that of #set_merge_each_n_spans
 */
Datum
Set_mest_adaptsplit(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  Span *spans = set_merge_each_n_spans(s, mest_count(fcinfo), nkeys);
  Span extent;
  set_set_span(s, &extent);
  PG_RETURN_POINTER(mest_span_keys_whole(spans, &extent, nkeys));
}

PG_FUNCTION_INFO_V1(Spanset_mest_equisplit);
/**
 * @brief Multi-entry extract method of a span set keyed by the spans of a
 * component or of several and by its extent (#mest_span_keys_whole)
 * @details The keys of a value are a number of spans of its consecutive
 * components, the number being the option of the class (#mest_count) and the
 * split that of #spanset_split_n_spans
 */
Datum
Spanset_mest_equisplit(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  Span *spans = spanset_split_n_spans(ss, mest_count(fcinfo), nkeys);
  PG_RETURN_POINTER(mest_span_keys_whole(spans, &ss->span, nkeys));
}

PG_FUNCTION_INFO_V1(Spanset_mest_segsplit);
/**
 * @brief Multi-entry extract method of a span set keyed by the spans of a
 * component or of several and by its extent (#mest_span_keys_whole)
 * @details The keys of a value are the spans of its consecutive components a
 * number at a time, the number being the option of the class (#mest_count) and
 * the split that of #spanset_split_each_n_spans
 */
Datum
Spanset_mest_segsplit(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  Span *spans = spanset_split_each_n_spans(ss, mest_count(fcinfo), nkeys);
  PG_RETURN_POINTER(mest_span_keys_whole(spans, &ss->span, nkeys));
}

PG_FUNCTION_INFO_V1(Spanset_mest_mergesplit);
/**
 * @brief Multi-entry extract method of a span set keyed by the spans of a
 * component or of several and by its extent (#mest_span_keys_whole)
 * @details The keys of a value are a number of spans merging the spans of its
 * components least apart, the number being the option of the class
 * (#mest_count) and the split that of #spanset_merge_n_spans
 */
Datum
Spanset_mest_mergesplit(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  Span *spans = spanset_merge_n_spans(ss, mest_count(fcinfo), nkeys);
  PG_RETURN_POINTER(mest_span_keys_whole(spans, &ss->span, nkeys));
}

PG_FUNCTION_INFO_V1(Spanset_mest_adaptsplit);
/**
 * @brief Multi-entry extract method of a span set keyed by the spans of a
 * component or of several and by its extent (#mest_span_keys_whole)
 * @details The keys of a value are the spans merging the spans of its
 * components a number at a time, least apart, the number being the option of
 * the class (#mest_count) and the split that of #spanset_merge_each_n_spans
 */
Datum
Spanset_mest_adaptsplit(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  Span *spans = spanset_merge_each_n_spans(ss, mest_count(fcinfo), nkeys);
  PG_RETURN_POINTER(mest_span_keys_whole(spans, &ss->span, nkeys));
}

/*****************************************************************************
 * Extraction of the tiles of a temporal number
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnumber_mest_tilesplit);
/**
 * @brief Multi-entry extract method of the temporal numbers keyed by the
 * tiles of a value and time grid
 * @details The size and the origin of the value dimension are Datums of the
 * base type of the value (#double_datum), as #Tnumber_value_time_boxes_common
 * reads them, the origin of the value dimension being 0 and the one of time
 * the default origin of the tiling functions
 */
Datum
Tnumber_mest_tilesplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  double vsize = MEST_VSIZE_DEFAULT;
  Interval *duration = NULL;
  if (PG_HAS_OPCLASS_OPTIONS())
  {
    MestTileOptions *options = (MestTileOptions *) PG_GET_OPCLASS_OPTIONS();
    vsize = options->vsize;
    const char *str = GET_STRING_RELOPTION(options, duration);
    if (str && strlen(str) > 0)
      duration = pg_interval_in(str, -1);
  }
  MeosType basetype = temptype_basetype(temp->temptype);
  TimestampTz torigin = duration ? pg_timestamptz_in(MEST_TORIGIN, -1) : 0;
  int count;
  TBox *boxes = tnumber_value_time_boxes(temp, double_datum(vsize, basetype),
    duration, double_datum(0.0, basetype), torigin, true, &count);
  *nkeys = count;
  PG_RETURN_POINTER(count > 0 ?
    mest_keys_whole(temp, boxes, sizeof(TBox), nkeys) : NULL);
}

/*****************************************************************************
 * Extraction of the time bins of a temporal value
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_mest_binsplit);
/**
 * @brief Multi-entry extract method of the temporal values keyed by their
 * time bins
 * @details The time dimension of #Tnumber_mest_tilesplit alone: the bins are
 * those of #Temporal_time_bins, of the duration of the options and from the
 * default origin of the tiling functions
 */
Datum
Temporal_mest_binsplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  const char *str = PG_HAS_OPCLASS_OPTIONS() ?
    GET_STRING_RELOPTION((MestBinOptions *) PG_GET_OPCLASS_OPTIONS(),
      duration) : MEST_DURATION_DEFAULT;
  Interval *duration = pg_interval_in(str, -1);
  int count;
  Span *bins = temporal_time_bins_intl(temp, duration,
    pg_timestamptz_in(MEST_TORIGIN, -1), true, &count);
  /* Each key is a bin trimmed to the period of the value, so the keys lie in
   * the box of the value and a predicate holding on every key holds on the
   * value; a bin meeting the period at a bound it leaves out is no key */
  Span period;
  temporal_set_tstzspan(temp, &period);
  int nbins = 0;
  for (int i = 0; i < count; i++)
  {
    if (inter_span_span(&bins[i], &period, &bins[nbins]))
      nbins++;
  }
  *nkeys = nbins;
  PG_RETURN_POINTER(nbins > 0 ? mest_keys(bins, sizeof(Span), nbins) : NULL);
}

/*****************************************************************************
 * Extraction of the boxes of a temporal number
 *****************************************************************************/

/* GENERATED-BOXMEST-BEGIN tnumber — tools/codegen/inherited/generate.py from templates/boxmest.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxsplit.yaml and re-run. */

PG_FUNCTION_INFO_V1(Tnumber_mest_equisplit);
/**
 * @brief Multi-entry extract method keyed by a number of boxes
 * @details The keys of a temporal number are a number of
 * temporal boxes of its consecutive instants or segments, the number being
 * the option of the class (#mest_count) and the split that of
 * #tnumber_split_n_tboxes
 */
Datum
Tnumber_mest_equisplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  TBox *boxes = tnumber_split_n_tboxes_intl(temp, mest_count(fcinfo), nkeys);
  PG_RETURN_POINTER(mest_keys_whole(temp, boxes, sizeof(TBox), nkeys));
}

PG_FUNCTION_INFO_V1(Tnumber_mest_segsplit);
/**
 * @brief Multi-entry extract method keyed by the boxes of a number of segments
 * @details The keys of a temporal number are the
 * temporal boxes of a number of its consecutive instants or segments each, the
 * number being the option of the class (#mest_count) and the split that of
 * #tnumber_split_each_n_tboxes
 */
Datum
Tnumber_mest_segsplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  TBox *boxes = tnumber_split_each_n_tboxes_intl(temp, mest_count(fcinfo),
    nkeys);
  PG_RETURN_POINTER(mest_keys_whole(temp, boxes, sizeof(TBox), nkeys));
}

/* GENERATED-BOXMEST-END tnumber */

/*****************************************************************************/

/* GENERATED-BOXMEST-BEGIN temporal — tools/codegen/inherited/generate.py from templates/boxmest.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxsplit.yaml and re-run. */

PG_FUNCTION_INFO_V1(Temporal_mest_equisplit);
/**
 * @brief Multi-entry extract method keyed by a number of boxes
 * @details The keys of a temporal value are a number of
 * spans of its consecutive instants or segments, the number being
 * the option of the class (#mest_count) and the split that of
 * #temporal_split_n_spans
 */
Datum
Temporal_mest_equisplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  Span *boxes = temporal_split_n_spans_intl(temp, mest_count(fcinfo), nkeys);
  PG_RETURN_POINTER(mest_keys(boxes, sizeof(Span), *nkeys));
}

PG_FUNCTION_INFO_V1(Temporal_mest_segsplit);
/**
 * @brief Multi-entry extract method keyed by the boxes of a number of segments
 * @details The keys of a temporal value are the
 * spans of a number of its consecutive instants or segments each, the
 * number being the option of the class (#mest_count) and the split that of
 * #temporal_split_each_n_spans
 */
Datum
Temporal_mest_segsplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  Span *boxes = temporal_split_each_n_spans_intl(temp, mest_count(fcinfo),
    nkeys);
  PG_RETURN_POINTER(mest_keys(boxes, sizeof(Span), *nkeys));
}

/* GENERATED-BOXMEST-END temporal */

/*****************************************************************************
 * The structures of the keys of the sets and span sets
 *****************************************************************************/

/**
 * @brief Return the flags of a key over the keys of the sets and span sets
 * whose flags are given: @p MEOS_FLAG_WHOLE over keys of extents alone, none
 * over keys of pieces alone, and @p MEOS_FLAG_PARTS over keys of both
 * (#mest_span_key_answers)
 */
static int16
span_mest_flags(int16 flags1, int16 flags2)
{
  int16 result = 0;
  if (MEOS_FLAGS_GET_PARTS(flags1) || MEOS_FLAGS_GET_PARTS(flags2) ||
      MEOS_FLAGS_GET_WHOLE(flags1) != MEOS_FLAGS_GET_WHOLE(flags2))
    MEOS_FLAGS_SET_PARTS(result, true);
  else
    MEOS_FLAGS_SET_WHOLE(result, MEOS_FLAGS_GET_WHOLE(flags1));
  return result;
}

/**
 * @brief Return true if two keys of the sets and span sets have the same
 * structure, the extents, the pieces, or both
 */
static bool
span_mest_same_structure(int16 flags1, int16 flags2)
{
  return MEOS_FLAGS_GET_PARTS(flags1) == MEOS_FLAGS_GET_PARTS(flags2) &&
    MEOS_FLAGS_GET_WHOLE(flags1) == MEOS_FLAGS_GET_WHOLE(flags2);
}

extern PGDLLEXPORT Datum Span_gist_union(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum Span_gist_penalty(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum Span_gist_picksplit(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum Span_gist_same(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum Span_quadtree_choose(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum Span_quadtree_picksplit(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum Span_kdtree_choose(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum Span_kdtree_picksplit(PG_FUNCTION_ARGS);

PGDLLEXPORT Datum Span_mgist_union(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mgist_union);
/**
 * @brief Multi-entry GiST union method of the sets and span sets, as
 * #Span_gist_union answers it, the key carrying the flags of the structures
 * of the keys under it (#span_mest_flags)
 */
Datum
Span_mgist_union(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GISTENTRY *ent = entryvec->vector;
  Span *result = DatumGetSpanP(Span_gist_union(fcinfo));
  int16 flags = DatumGetSpanP(ent[0].key)->flags;
  for (int i = 1; i < entryvec->n; i++)
    flags = span_mest_flags(flags, DatumGetSpanP(ent[i].key)->flags);
  result->flags = flags;
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Span_mgist_penalty(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mgist_penalty);
/**
 * @brief Multi-entry GiST penalty method of the sets and span sets, as
 * #Span_gist_penalty answers it
 * @details A key goes under a key of its structure when there is one, else
 * under a key over both structures, so that the keys of the extents and those
 * of the pieces gather under keys of their own, as #bbox_gist_penalty places
 * the boxes of several structures
 */
Datum
Span_mgist_penalty(PG_FUNCTION_ARGS)
{
  GISTENTRY *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
  GISTENTRY *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
  float *penalty = (float *) PG_GETARG_POINTER(2);
  int16 orig = DatumGetSpanP(origentry->key)->flags,
    new = DatumGetSpanP(newentry->key)->flags;
  if (MEOS_FLAGS_GET_PARTS(orig))
    *penalty = FLT_MAX / 2;
  else if (! span_mest_same_structure(orig, new))
    *penalty = FLT_MAX;
  else
    return Span_gist_penalty(fcinfo);
  PG_RETURN_POINTER(penalty);
}

/**
 * @brief Return the union of the keys of some entries of a GiST page, with the
 * flags of their structures
 */
static Span *
span_mest_union_entries(GISTENTRY *ent, OffsetNumber *offsets, int count)
{
  Span *result = span_copy(DatumGetSpanP(ent[offsets[0]].key));
  int16 flags = result->flags;
  for (int i = 1; i < count; i++)
  {
    const Span *key = DatumGetSpanP(ent[offsets[i]].key);
    span_expand(key, result);
    flags = span_mest_flags(flags, key->flags);
  }
  result->flags = flags;
  return result;
}

PGDLLEXPORT Datum Span_mgist_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mgist_picksplit);
/**
 * @brief Multi-entry GiST picksplit method of the sets and span sets
 * @details The entries of the structure of the first one go to the left and
 * the others to the right, keys over both structures to the right, as
 * #bbox_gist_split_structures splits the boxes of several structures; the
 * entries of one structure, or keys over both alone, are split as
 * #Span_gist_picksplit splits them
 */
Datum
Span_mgist_picksplit(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
  GISTENTRY *ent = entryvec->vector;
  OffsetNumber maxoff = (OffsetNumber) (entryvec->n - 1), i;
  int16 first = DatumGetSpanP(ent[FirstOffsetNumber].key)->flags;
  bool split = ! MEOS_FLAGS_GET_PARTS(first);
  if (split)
  {
    for (i = OffsetNumberNext(FirstOffsetNumber); i <= maxoff;
        i = OffsetNumberNext(i))
      if (! span_mest_same_structure(first,
          DatumGetSpanP(ent[i].key)->flags))
        break;
    split = (i <= maxoff);
  }
  if (! split)
  {
    Span_gist_picksplit(fcinfo);
    /* Every entry has the structure of the first, or every one is over both */
    int16 flags = first;
    for (i = OffsetNumberNext(FirstOffsetNumber); i <= maxoff;
        i = OffsetNumberNext(i))
      flags = span_mest_flags(flags, DatumGetSpanP(ent[i].key)->flags);
    /* A side the split leaves empty has no union (#span_gist_fallback_split) */
    if (DatumGetPointer(v->spl_ldatum))
      DatumGetSpanP(v->spl_ldatum)->flags = flags;
    if (DatumGetPointer(v->spl_rdatum))
      DatumGetSpanP(v->spl_rdatum)->flags = flags;
    PG_RETURN_POINTER(v);
  }
  size_t nbytes = (maxoff + 1) * sizeof(OffsetNumber);
  v->spl_left = palloc(nbytes);
  v->spl_right = palloc(nbytes);
  v->spl_nleft = v->spl_nright = 0;
  for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
  {
    if (span_mest_same_structure(first, DatumGetSpanP(ent[i].key)->flags))
      v->spl_left[v->spl_nleft++] = i;
    else
      v->spl_right[v->spl_nright++] = i;
  }
  v->spl_ldatum = SpanPGetDatum(span_mest_union_entries(ent, v->spl_left,
    v->spl_nleft));
  v->spl_rdatum = SpanPGetDatum(span_mest_union_entries(ent, v->spl_right,
    v->spl_nright));
  PG_RETURN_POINTER(v);
}

PGDLLEXPORT Datum Span_mgist_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mgist_same);
/**
 * @brief Multi-entry GiST same method of the sets and span sets, as
 * #Span_gist_same answers it, the keys having the same flags
 */
Datum
Span_mgist_same(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  bool *result = (bool *) PG_GETARG_POINTER(2);
  *result = span_eq(s1, s2) && s1->flags == s2->flags;
  PG_RETURN_POINTER(result);
}

/**
 * @brief Choose the node of an inner tuple of the sets and span sets for the
 * keys of the extents and those of the pieces
 * @details An inner tuple whose prefix carries the flag @p MEOS_FLAG_PARTS
 * sends the keys of the structure of its prefix to its first node and the
 * others to its second one, without adding a level. A key reaching an inner
 * tuple of the other structure splits it: a tuple over both structures is
 * placed above it, the inner tuple becoming its first node, as
 * #stbox_spgist_choose_parts places the boxes of several structures
 * @return True when the key is placed by this function
 */
static bool
span_mspgist_choose_parts(spgChooseIn *in, spgChooseOut *out)
{
  Span *centroid = DatumGetSpanP(in->prefixDatum),
    *key = DatumGetSpanP(in->leafDatum);
  if (MEOS_FLAGS_GET_PARTS(centroid->flags))
  {
    out->resultType = spgMatchNode;
    out->result.matchNode.nodeN = (MEOS_FLAGS_GET_WHOLE(centroid->flags) ==
      MEOS_FLAGS_GET_WHOLE(key->flags)) ? 0 : 1;
    out->result.matchNode.levelAdd = 0;
    out->result.matchNode.restDatum = PointerGetDatum(key);
    return true;
  }
  if (MEOS_FLAGS_GET_WHOLE(centroid->flags) == MEOS_FLAGS_GET_WHOLE(key->flags))
    return false;
  Span *parts = span_copy(centroid);
  MEOS_FLAGS_SET_PARTS(parts->flags, true);
  out->resultType = spgSplitTuple;
  out->result.splitTuple.prefixHasPrefix = true;
  out->result.splitTuple.prefixPrefixDatum = SpanPGetDatum(parts);
  out->result.splitTuple.prefixNNodes = 2;
  out->result.splitTuple.prefixNodeLabels = NULL;
  out->result.splitTuple.childNodeN = 0;
  out->result.splitTuple.postfixHasPrefix = true;
  out->result.splitTuple.postfixPrefixDatum = SpanPGetDatum(span_copy(centroid));
  return true;
}

/**
 * @brief Split the keys of a leaf page of the sets and span sets
 * @details Keys of both structures go to the nodes of an inner tuple carrying
 * the flag @p MEOS_FLAG_PARTS (#span_mspgist_choose_parts); keys of one
 * structure are split by the function given, whose prefix takes the flags of
 * their structure
 */
static void
span_mspgist_picksplit(FunctionCallInfo fcinfo,
  Datum (*picksplit)(FunctionCallInfo))
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  Span *first = DatumGetSpanP(in->datums[0]);
  bool whole = MEOS_FLAGS_GET_WHOLE(first->flags);
  int i;
  for (i = 1; i < in->nTuples; i++)
    if (MEOS_FLAGS_GET_WHOLE(DatumGetSpanP(in->datums[i])->flags) != whole)
      break;
  if (i == in->nTuples)
  {
    picksplit(fcinfo);
    Span *centroid = DatumGetSpanP(out->prefixDatum);
    centroid->flags = 0;
    MEOS_FLAGS_SET_WHOLE(centroid->flags, whole);
    return;
  }
  Span *parts = span_copy(first);
  parts->flags = 0;
  MEOS_FLAGS_SET_WHOLE(parts->flags, whole);
  MEOS_FLAGS_SET_PARTS(parts->flags, true);
  out->hasPrefix = true;
  out->prefixDatum = SpanPGetDatum(parts);
  out->nNodes = 2;
  out->nodeLabels = NULL;
  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);
  for (i = 0; i < in->nTuples; i++)
  {
    out->leafTupleDatums[i] = in->datums[i];
    out->mapTuplesToNodes[i] =
      (MEOS_FLAGS_GET_WHOLE(DatumGetSpanP(in->datums[i])->flags) == whole) ?
      0 : 1;
  }
  return;
}

PGDLLEXPORT Datum Span_mquadtree_choose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mquadtree_choose);
/**
 * @brief Multi-entry SP-GiST choose method of the quad-tree of the sets and
 * span sets, as #Span_quadtree_choose answers it within a structure
 * (#span_mspgist_choose_parts)
 */
Datum
Span_mquadtree_choose(PG_FUNCTION_ARGS)
{
  if (span_mspgist_choose_parts((spgChooseIn *) PG_GETARG_POINTER(0),
      (spgChooseOut *) PG_GETARG_POINTER(1)))
    PG_RETURN_VOID();
  return Span_quadtree_choose(fcinfo);
}

PGDLLEXPORT Datum Span_mkdtree_choose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mkdtree_choose);
/**
 * @brief Multi-entry SP-GiST choose method of the k-d tree of the sets and
 * span sets, as #Span_kdtree_choose answers it within a structure
 * (#span_mspgist_choose_parts)
 */
Datum
Span_mkdtree_choose(PG_FUNCTION_ARGS)
{
  if (span_mspgist_choose_parts((spgChooseIn *) PG_GETARG_POINTER(0),
      (spgChooseOut *) PG_GETARG_POINTER(1)))
    PG_RETURN_VOID();
  return Span_kdtree_choose(fcinfo);
}

PGDLLEXPORT Datum Span_mquadtree_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mquadtree_picksplit);
/**
 * @brief Multi-entry SP-GiST picksplit method of the quad-tree of the sets and
 * span sets (#span_mspgist_picksplit)
 */
Datum
Span_mquadtree_picksplit(PG_FUNCTION_ARGS)
{
  span_mspgist_picksplit(fcinfo, Span_quadtree_picksplit);
  PG_RETURN_VOID();
}

PGDLLEXPORT Datum Span_mkdtree_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mkdtree_picksplit);
/**
 * @brief Multi-entry SP-GiST picksplit method of the k-d tree of the sets and
 * span sets (#span_mspgist_picksplit)
 */
Datum
Span_mkdtree_picksplit(PG_FUNCTION_ARGS)
{
  span_mspgist_picksplit(fcinfo, Span_kdtree_picksplit);
  PG_RETURN_VOID();
}

/* GENERATED-MESTCONSISTENT-BEGIN temporal — tools/codegen/inherited/generate.py from templates/mestconsistent.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxtypes.yaml and re-run. */

extern PGDLLEXPORT Datum Span_gist_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mgist_consistent);
/**
 * @brief Multi-entry GiST consistent method of the classes keyed by the boxes
 * of the pieces of a value, as #Span_gist_consistent answers it
 * @details A key covers a part of a value, so every leaf is rechecked by the
 * operator: a predicate of the box of a piece is not that of the value
 */
Datum
Span_mgist_consistent(PG_FUNCTION_ARGS)
{
  /* A key answers by the scan, ordered or not, and by the structure of the
   * keys under it (#mest_span_key_answers); a key of a piece answers the
   * strategy of #mest_pieces_strategy */
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  bool ordered = PG_NARGS() > 5 && PG_GETARG_BOOL(5);
  if (! mest_span_key_answers(DatumGetSpanP(entry->key)->flags, ordered))
    PG_RETURN_BOOL(false);
  if (! ordered)
    fcinfo->args[2].value = UInt16GetDatum(
      mest_pieces_strategy((StrategyNumber) PG_GETARG_UINT16(2)));
  Datum result = Span_gist_consistent(fcinfo);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  *recheck = true;
  return result;
}

extern PGDLLEXPORT Datum Span_gist_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mgist_distance);
/**
 * @brief Multi-entry GiST distance method of the classes keyed by the spans of
 * the pieces and of the extent of a value, as #Span_gist_distance answers it
 * @details The distance of a value is that of its extent, which the key of a
 * piece overstates, so a key over pieces alone lies at the maximum distance, as
 * #Span_gist_distance places a key it cannot measure, and the value is reported at the
 * distance of the key of its extent, the smallest of its keys
 */
Datum
Span_mgist_distance(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  int16 flags = DatumGetSpanP(entry->key)->flags;
  if (! MEOS_FLAGS_GET_WHOLE(flags) && ! MEOS_FLAGS_GET_PARTS(flags))
  {
    bool *recheck = (bool *) PG_GETARG_POINTER(4);
    *recheck = false;
    PG_RETURN_FLOAT8(DBL_MAX);
  }
  return Span_gist_distance(fcinfo);
}

extern PGDLLEXPORT Datum Span_kdtree_inner_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mkdtree_inner_consistent);
/**
 * @brief Multi-entry SP-GiST inner consistent method of the classes keyed by
 * the spans of the pieces of a value, as
 * #Span_kdtree_inner_consistent answers it
 * @details The nodes are chosen for the strategies a key of a piece answers
 * (#mest_pieces_strategy), or, in an ordered scan, which the key of the extent
 * of a value answers (#mest_span_key_answers), for the strategies themselves
 */
Datum
Span_mkdtree_inner_consistent(PG_FUNCTION_ARGS)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  /* A tuple over the keys of the extents and of the pieces sends the search
   * to the node of the structure the scan reads, where its tree starts
   * without a traversal value; a tuple of the other structure to none
   * (#mest_span_key_answers) */
  const Span *centroid = DatumGetSpanP(in->prefixDatum);
  bool ordered = in->norderbys > 0;
  if (MEOS_FLAGS_GET_PARTS(centroid->flags) ||
      ! mest_span_key_answers(centroid->flags, ordered))
  {
    spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
    out->nNodes = 0;
    out->nodeNumbers = palloc(sizeof(int));
    out->levelAdds = palloc0(sizeof(int));
    out->traversalValues = palloc0(sizeof(void *));
    if (ordered)
      out->distances = palloc(sizeof(double *));
    if (MEOS_FLAGS_GET_PARTS(centroid->flags))
    {
      out->nodeNumbers[0] =
        (MEOS_FLAGS_GET_WHOLE(centroid->flags) == ordered) ? 0 : 1;
      if (ordered)
        out->distances[0] = palloc0(sizeof(double) * in->norderbys);
      out->nNodes = 1;
    }
    PG_RETURN_VOID();
  }
  ScanKey keys = in->scankeys;
  if (! ordered)
    in->scankeys = mest_pieces_scankeys(keys, in->nkeys);
  Span_kdtree_inner_consistent(fcinfo);
  if (in->scankeys != keys)
    pfree(in->scankeys);
  in->scankeys = keys;
  PG_RETURN_VOID();
}

extern PGDLLEXPORT Datum Span_quadtree_inner_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mquadtree_inner_consistent);
/**
 * @brief Multi-entry SP-GiST inner consistent method of the classes keyed by
 * the spans of the pieces of a value, as
 * #Span_quadtree_inner_consistent answers it
 * @details The nodes are chosen for the strategies a key of a piece answers
 * (#mest_pieces_strategy), or, in an ordered scan, which the key of the extent
 * of a value answers (#mest_span_key_answers), for the strategies themselves
 */
Datum
Span_mquadtree_inner_consistent(PG_FUNCTION_ARGS)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  /* A tuple over the keys of the extents and of the pieces sends the search
   * to the node of the structure the scan reads, where its tree starts
   * without a traversal value; a tuple of the other structure to none
   * (#mest_span_key_answers) */
  const Span *centroid = DatumGetSpanP(in->prefixDatum);
  bool ordered = in->norderbys > 0;
  if (MEOS_FLAGS_GET_PARTS(centroid->flags) ||
      ! mest_span_key_answers(centroid->flags, ordered))
  {
    spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
    out->nNodes = 0;
    out->nodeNumbers = palloc(sizeof(int));
    out->levelAdds = palloc0(sizeof(int));
    out->traversalValues = palloc0(sizeof(void *));
    if (ordered)
      out->distances = palloc(sizeof(double *));
    if (MEOS_FLAGS_GET_PARTS(centroid->flags))
    {
      out->nodeNumbers[0] =
        (MEOS_FLAGS_GET_WHOLE(centroid->flags) == ordered) ? 0 : 1;
      if (ordered)
        out->distances[0] = palloc0(sizeof(double) * in->norderbys);
      out->nNodes = 1;
    }
    PG_RETURN_VOID();
  }
  ScanKey keys = in->scankeys;
  if (! ordered)
    in->scankeys = mest_pieces_scankeys(keys, in->nkeys);
  Span_quadtree_inner_consistent(fcinfo);
  if (in->scankeys != keys)
    pfree(in->scankeys);
  in->scankeys = keys;
  PG_RETURN_VOID();
}

extern PGDLLEXPORT Datum Span_spgist_leaf_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_mspgist_leaf_consistent);
/**
 * @brief Multi-entry SP-GiST leaf consistent method of the classes keyed by
 * the boxes of the pieces of a value, as #Span_spgist_leaf_consistent answers it
 * @details A key covers a part of a value, so every leaf is rechecked by the
 * operator, and its distance by the distance of the value: the distance to a
 * piece bounds that of the value from below
 */
Datum
Span_mspgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  /* A leaf answers by the scan, ordered or not (#mest_span_key_answers); a
   * key of a piece answers the strategies of #mest_pieces_strategy, and the
   * key of the extent the distance of the value, exactly */
  bool ordered = in->norderbys > 0;
  if (! mest_span_key_answers(DatumGetSpanP(in->leafDatum)->flags, ordered))
    PG_RETURN_BOOL(false);
  ScanKey keys = in->scankeys;
  if (! ordered)
    in->scankeys = mest_pieces_scankeys(keys, in->nkeys);
  Datum result = Span_spgist_leaf_consistent(fcinfo);
  if (in->scankeys != keys)
    pfree(in->scankeys);
  in->scankeys = keys;
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
  out->recheck = true;
  return result;
}

extern PGDLLEXPORT Datum Tbox_spgist_leaf_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_mspgist_leaf_consistent);
/**
 * @brief Multi-entry SP-GiST leaf consistent method of the classes keyed by
 * the boxes of the pieces of a value, as #Tbox_spgist_leaf_consistent answers it
 * @details A key covers a part of a value, so every leaf is rechecked by the
 * operator, and its distance by the distance of the value: the distance to a
 * piece bounds that of the value from below
 */
Datum
Tbox_mspgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  /* The key answers by its tier (#mest_key_answers) */
  int16 flags = ((const TBox *) DatumGetPointer(in->leafDatum))->flags;
  for (int i = 0; i < in->nkeys; i++)
    if (! mest_key_answers(flags, in->scankeys[i].sk_strategy))
      PG_RETURN_BOOL(false);
  Datum result = Tbox_spgist_leaf_consistent(fcinfo);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
  out->recheck = true;
  if (out->distances)
    out->recheckDistances = true;
  return result;
}

extern PGDLLEXPORT Datum Tnumber_gist_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_mgist_consistent);
/**
 * @brief Multi-entry GiST consistent method of the classes keyed by the boxes
 * of the pieces of a value, as #Tnumber_gist_consistent answers it
 * @details A key covers a part of a value, so every leaf is rechecked by the
 * operator: a predicate of the box of a piece is not that of the value
 */
Datum
Tnumber_mgist_consistent(PG_FUNCTION_ARGS)
{
  /* The key answers by its tier (#mest_key_answers) */
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (! mest_key_answers(((const TBox *)
      DatumGetPointer(entry->key))->flags,
      (StrategyNumber) PG_GETARG_UINT16(2)))
    PG_RETURN_BOOL(false);
  Datum result = Tnumber_gist_consistent(fcinfo);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  *recheck = true;
  return result;
}

/* GENERATED-MESTCONSISTENT-END temporal */
