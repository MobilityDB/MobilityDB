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
 * @brief Multi-entry support of the temporal geometries and geographies: the
 * keys of a value are the spatiotemporal boxes MEOS splits it into
 * @details The extraction methods of their multi-entry classes, as
 * temporal_mest.c holds those of the temporal types and numbers; each calls
 * the internal twin of a splitter or a merge of the temporal geometries
 */

#include "pg_temporal/temporal_mest.h"

/* C */
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/reloptions.h>
#include <access/spgist.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <pgtypes.h>
#include "temporal/temporal.h"
#include "geo/tgeo_tile.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Options of the splits by extents
 *****************************************************************************/

/** Default size of a tile, or extent of a query, in the space dimensions */
#define MEST_SIZE_DEFAULT     1.0
/** Largest size of a tile, or extent of a query, in the space dimensions */
#define MEST_SIZE_MAX         1.0e9
/** Origin of the time bins of a tile, the default of the tiling functions */
#define MEST_TORIGIN          "2000-01-03"

/**
 * @brief Options of a split of a temporal geometry by extents: the sizes of
 * a tile, or the extents of a query, in space and in time
 */
typedef struct
{
  int32 vl_len_;    /**< Varlena header (do not touch directly) */
  double xsize;     /**< Extent in the x dimension */
  double ysize;     /**< Extent in the y dimension, 0 taking xsize */
  double zsize;     /**< Extent in the z dimension, 0 taking xsize */
  int duration;     /**< Extent in time, an interval string, empty for none */
} MestStboxOptions;

PG_FUNCTION_INFO_V1(Mest_stbox_options);
/**
 * @brief Options method of the multi-entry classes splitting a temporal
 * geometry by extents, the tiles and the query merge
 * @details The space sizes and the duration of #Mest_tile_options for the
 * dimensions of a spatiotemporal box, as #Tgeo_space_time_boxes takes them
 */
Datum
Mest_stbox_options(PG_FUNCTION_ARGS)
{
  local_relopts *relopts = (local_relopts *) PG_GETARG_POINTER(0);
  init_local_reloptions(relopts, sizeof(MestStboxOptions));
  add_local_real_reloption(relopts, "xsize",
    "extent in the x dimension", MEST_SIZE_DEFAULT, 0.0, MEST_SIZE_MAX,
    offsetof(MestStboxOptions, xsize));
  add_local_real_reloption(relopts, "ysize",
    "extent in the y dimension, 0 taking xsize", 0.0, 0.0, MEST_SIZE_MAX,
    offsetof(MestStboxOptions, ysize));
  add_local_real_reloption(relopts, "zsize",
    "extent in the z dimension, 0 taking xsize", 0.0, 0.0, MEST_SIZE_MAX,
    offsetof(MestStboxOptions, zsize));
  add_local_string_reloption(relopts, "duration",
    "extent in time, an interval; empty for none", "",
    &mest_duration_validate, &mest_duration_fill,
    offsetof(MestStboxOptions, duration));
  PG_RETURN_VOID();
}

/**
 * @brief Read the extents of the options of a class, a ysize or zsize of 0
 * taking xsize, and return the duration, NULL for none
 */
static Interval *
mest_stbox_extents(FunctionCallInfo fcinfo, double *xsize, double *ysize,
  double *zsize)
{
  *xsize = MEST_SIZE_DEFAULT; *ysize = *zsize = 0.0;
  Interval *duration = NULL;
  if (PG_HAS_OPCLASS_OPTIONS())
  {
    MestStboxOptions *options = (MestStboxOptions *) PG_GET_OPCLASS_OPTIONS();
    *xsize = options->xsize;
    *ysize = options->ysize;
    *zsize = options->zsize;
    const char *str = GET_STRING_RELOPTION(options, duration);
    if (str && strlen(str) > 0)
      duration = pg_interval_in(str, -1);
  }
  if (*ysize == 0.0)
    *ysize = *xsize;
  if (*zsize == 0.0)
    *zsize = *xsize;
  return duration;
}

/*****************************************************************************
 * Extraction of the tiles of a temporal geometry and of the boxes a query
 * reads least
 *****************************************************************************/

/**
 * @brief Return the origin of a space grid of a temporal geometry, the point
 * at 0 in its reference system, as the tiling functions default it
 * @details The point of #geompoint_make2d and its siblings of the dimensions
 * and the geodetic flag of the value
 */
static GSERIALIZED *
mest_space_origin(const Temporal *temp)
{
  int32_t srid = tspatial_srid(temp);
  bool hasz = MEOS_FLAGS_GET_Z(temp->flags);
  if (MEOS_FLAGS_GET_GEODETIC(temp->flags))
    return hasz ? geogpoint_make3dz(srid, 0.0, 0.0, 0.0) :
      geogpoint_make2d(srid, 0.0, 0.0);
  return hasz ? geompoint_make3dz(srid, 0.0, 0.0, 0.0) :
    geompoint_make2d(srid, 0.0, 0.0);
}

PG_FUNCTION_INFO_V1(Tgeo_mest_tilesplit);
/**
 * @brief Multi-entry extract method of the temporal geometries keyed by the
 * tiles of a space and time grid
 * @details The tiles of #Tgeo_space_time_boxes, of the sizes of the options,
 * from the default space origin and the default origin of the time bins;
 * without a duration the grid splits space alone
 */
Datum
Tgeo_mest_tilesplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  double xsize, ysize, zsize;
  Interval *duration = mest_stbox_extents(fcinfo, &xsize, &ysize, &zsize);
  TimestampTz torigin = duration ? pg_timestamptz_in(MEST_TORIGIN, -1) : 0;
  int count;
  /* Every tile of the box of the value is read, the restriction to each
   * stating whether the value enters it */
  GSERIALIZED *sorigin = mest_space_origin(temp);
  STBox *boxes = tgeo_space_time_boxes_intl(temp, xsize, ysize, zsize,
    duration, sorigin, torigin, false, true, &count);
  pfree(sorigin);
  *nkeys = count;
  PG_RETURN_POINTER(count > 0 ? mest_keys_whole(temp, boxes, sizeof(STBox), nkeys) :
    NULL);
}

PG_FUNCTION_INFO_V1(Tgeo_mest_linearsplit);
/**
 * @brief Multi-entry extract method of the temporal geometries keyed by the
 * boxes a query of given extents is expected to read least
 * @details The query box has the extents of the options, in the reference
 * system of the value, a period only where a duration is given; its position
 * is not read (#Tgeo_merge_query_stboxes)
 */
Datum
Tgeo_mest_linearsplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  double xsize, ysize, zsize;
  Interval *duration = mest_stbox_extents(fcinfo, &xsize, &ysize, &zsize);
  Span *period = NULL;
  if (duration)
  {
    TimestampTz t = pg_timestamptz_in(MEST_TORIGIN, -1);
    period = tstzspan_make(t, add_timestamptz_interval(t, duration), true,
      true);
  }
  STBox *query = stbox_make(true, MEOS_FLAGS_GET_Z(temp->flags),
    MEOS_FLAGS_GET_GEODETIC(temp->flags), tspatial_srid(temp), 0.0, xsize,
    0.0, ysize, 0.0, zsize, period);
  int count;
  STBox *boxes = tgeo_merge_query_stboxes_intl(temp, query, &count);
  *nkeys = count;
  PG_RETURN_POINTER(count > 0 ? mest_keys_whole(temp, boxes, sizeof(STBox), nkeys) :
    NULL);
}

/*****************************************************************************
 * Extraction of the spatiotemporal boxes of a temporal geometry
 *****************************************************************************/

/* GENERATED-BOXMEST-BEGIN tgeo — tools/codegen/inherited/generate.py from templates/boxmest.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxsplit.yaml and re-run. */

PG_FUNCTION_INFO_V1(Tgeo_mest_equisplit);
/**
 * @brief Multi-entry extract method keyed by a number of boxes
 * @details The keys of a temporal geo are a number of
 * spatiotemporal boxes of its consecutive instants or segments, the number being
 * the option of the class (#mest_count) and the split that of
 * #tgeo_split_n_stboxes
 */
Datum
Tgeo_mest_equisplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  STBox *boxes = tgeo_split_n_stboxes_intl(temp, mest_count(fcinfo), nkeys);
  PG_RETURN_POINTER(mest_keys_whole(temp, boxes, sizeof(STBox), nkeys));
}

PG_FUNCTION_INFO_V1(Tgeo_mest_segsplit);
/**
 * @brief Multi-entry extract method keyed by the boxes of a number of segments
 * @details The keys of a temporal geo are the
 * spatiotemporal boxes of a number of its consecutive instants or segments each, the
 * number being the option of the class (#mest_count) and the split that of
 * #tgeo_split_each_n_stboxes
 */
Datum
Tgeo_mest_segsplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  STBox *boxes = tgeo_split_each_n_stboxes_intl(temp, mest_count(fcinfo),
    nkeys);
  PG_RETURN_POINTER(mest_keys_whole(temp, boxes, sizeof(STBox), nkeys));
}

PG_FUNCTION_INFO_V1(Tgeo_mest_mergesplit);
/**
 * @brief Multi-entry extract method keyed by a number of merged boxes
 * @details The keys of a temporal geo are a number of
 * spatiotemporal boxes merged from the boxes of its instants or segments, the number
 * being the option of the class (#mest_count) and the merge that of
 * #tgeo_merge_n_stboxes
 */
Datum
Tgeo_mest_mergesplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  STBox *boxes = tgeo_merge_n_stboxes_intl(temp, mest_count(fcinfo), nkeys);
  PG_RETURN_POINTER(mest_keys_whole(temp, boxes, sizeof(STBox), nkeys));
}

PG_FUNCTION_INFO_V1(Tgeo_mest_adaptsplit);
/**
 * @brief Multi-entry extract method keyed by boxes of a number of segments
 * @details The keys of a temporal geo are the
 * spatiotemporal boxes merged from the boxes of its instants or segments until a box
 * holds on average a number of them,
 * the number being the option of the class (#mest_count) and the merge that of
 * #tgeo_merge_each_n_stboxes
 */
Datum
Tgeo_mest_adaptsplit(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  STBox *boxes = tgeo_merge_each_n_stboxes_intl(temp, mest_count(fcinfo),
    nkeys);
  PG_RETURN_POINTER(mest_keys_whole(temp, boxes, sizeof(STBox), nkeys));
}

/* GENERATED-BOXMEST-END tgeo */

/* GENERATED-MESTCONSISTENT-BEGIN geo — tools/codegen/inherited/generate.py from templates/mestconsistent.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxtypes.yaml and re-run. */

extern PGDLLEXPORT Datum Stbox_gist_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_mgist_consistent);
/**
 * @brief Multi-entry GiST consistent method of the classes keyed by the boxes
 * of the pieces of a value, as #Stbox_gist_consistent answers it
 * @details A key covers a part of a value, so every leaf is rechecked by the
 * operator: a predicate of the box of a piece is not that of the value
 */
Datum
Stbox_mgist_consistent(PG_FUNCTION_ARGS)
{
  /* The key answers by its tier (#mest_key_answers) */
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (! mest_key_answers(((const STBox *)
      DatumGetPointer(entry->key))->flags,
      (StrategyNumber) PG_GETARG_UINT16(2)))
    PG_RETURN_BOOL(false);
  Datum result = Stbox_gist_consistent(fcinfo);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  *recheck = true;
  return result;
}

extern PGDLLEXPORT Datum Stbox_spgist_leaf_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_mspgist_leaf_consistent);
/**
 * @brief Multi-entry SP-GiST leaf consistent method of the classes keyed by
 * the boxes of the pieces of a value, as #Stbox_spgist_leaf_consistent answers it
 * @details A key covers a part of a value, so every leaf is rechecked by the
 * operator, and its distance by the distance of the value: the distance to a
 * piece bounds that of the value from below
 */
Datum
Stbox_mspgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  /* The key answers by its tier (#mest_key_answers) */
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  int16 flags = ((const STBox *) DatumGetPointer(in->leafDatum))->flags;
  for (int i = 0; i < in->nkeys; i++)
    if (! mest_key_answers(flags, in->scankeys[i].sk_strategy))
      PG_RETURN_BOOL(false);
  Datum result = Stbox_spgist_leaf_consistent(fcinfo);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
  out->recheck = true;
  if (out->distances)
    out->recheckDistances = true;
  return result;
}

/* GENERATED-MESTCONSISTENT-END geo */
