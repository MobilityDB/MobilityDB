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
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/reloptions.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <pgtypes.h>
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
  PG_RETURN_POINTER(count > 0 ? mest_keys(boxes, sizeof(TBox), count) : NULL);
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
  *nkeys = count;
  PG_RETURN_POINTER(count > 0 ? mest_keys(bins, sizeof(Span), count) : NULL);
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
  PG_RETURN_POINTER(mest_keys(boxes, sizeof(TBox), *nkeys));
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
  PG_RETURN_POINTER(mest_keys(boxes, sizeof(TBox), *nkeys));
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
