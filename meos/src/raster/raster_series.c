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
 * @brief The values a trajectory reads from a raster series
 * @details A raster that changes in time is a series of rows
 * `(validtime, rast)`, each raster valid over its period, never a temporal
 * raster: a value `[r1@t1, ..., rn@tn]` would carry a whole raster at every
 * instant, and nothing between two rasters interpolates. A trajectory reads a
 * series as it reads one raster, each raster over the part of the trajectory
 * within its period, in the order of the periods. The functions here read a
 * series of PostGIS rasters and a series of Raquet tiles through the public
 * functions reading one raster, #raster_value() and #raster_tile_value(), so
 * no code of the vendored raster core is read here.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_raster.h>
#include "raster/raster_quadbin.h"
#include "temporal/span.h"
#include "temporal/temporal.h"

/**
 * @brief Function reading the rasters of a series valid over one period along
 * the part of a trajectory within it
 */
typedef Temporal *(*RasterSeriesRead)(const Temporal *traj,
  const void **values, int count, int band);

/*****************************************************************************
 * The walk along a series
 *****************************************************************************/

/**
 * @brief Return the order of the validity periods of a raster series, or
 * @p NULL when they are not the periods of a series
 * @details The periods must be time spans, at least one. Rows sharing one
 * period are the tiles of one raster in space, read together; two distinct
 * periods that overlap are refused, since two rasters valid at one time over
 * one place would give two values there
 * @param[in] spans Array of the validity periods of the rasters, one per raster
 * @param[in] count Number of elements in the array
 * @return The indexes of @p spans in the order of the periods, or @p NULL
 */
static int *
raster_series_order(const Span *spans, int count)
{
  if (! ensure_not_empty_array(count))
    return NULL;
  int *order = palloc(sizeof(int) * count);
  for (int i = 0; i < count; i++)
  {
    if (! ensure_span_isof_type(&spans[i], T_TSTZSPAN))
    {
      pfree(order);
      return NULL;
    }
    int j = i;
    while (j > 0 && span_cmp(&spans[order[j - 1]], &spans[i]) > 0)
    {
      order[j] = order[j - 1];
      j--;
    }
    order[j] = i;
  }
  /* With the periods ordered, equal ones are side by side, and a period
   * overlapping any earlier distinct one overlaps the distinct one just before
   * it */
  for (int i = 1; i < count; i++)
  {
    const Span *prev = &spans[order[i - 1]], *cur = &spans[order[i]];
    if (! span_eq(prev, cur) && overlaps_span_span(prev, cur))
    {
      pfree(order);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The validity periods of a raster series overlap");
      return NULL;
    }
  }
  return order;
}

/**
 * @brief Ensure that the validity periods are those of a raster series: time
 * spans, at least one, no two of them overlapping
 * @param[in] spans Array of the validity periods of the rasters, one per raster
 * @param[in] count Number of elements in the array
 */
static bool
ensure_raster_series(const Span *spans, int count)
{
  int *order = raster_series_order(spans, count);
  if (! order)
    return false;
  pfree(order);
  return true;
}

/**
 * @brief Ensure that two rasters of one footprint valid over one period read
 * the same values where they both answer
 * @details Two tiles of one place valid at one time are one tile given twice,
 * as a Raquet file holds one row per block and time; where the two read one
 * value they are accepted, and where they read two values they contradict each
 * other and are refused, as #ensure_tinstant_same_value() refuses two temporal
 * values merged with different values at one timestamp
 * @param[in] v1,v2 Values the two rasters read, or @p NULL when one reads none
 */
static bool
ensure_raster_series_agree(const Temporal *v1, const Temporal *v2)
{
  if (! v1 || ! v2)
    return true;
  SpanSet *ss1 = temporal_time(v1), *ss2 = temporal_time(v2);
  bool common = overlaps_spanset_spanset(ss1, ss2);
  pfree(ss1); pfree(ss2);
  if (! common || ever_ne_temporal_temporal(v1, v2) != 1)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Two rasters of one place valid over one period of a raster series read "
    "different values at one time");
  return false;
}

/**
 * @brief Return the values a trajectory reads from a raster series, each
 * raster read over the part of the trajectory within its validity period
 * @details The rows are read in the order of their periods, in one walk, as
 * #raster_tile_value_array() reads its tiles side by side in space. A part of
 * the trajectory within no period reads no value, as a nodata pixel reads
 * none. Periods that overlap would give two values at one time, and are
 * refused
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] values Rasters of the series, a @p NULL one read as no raster
 * @param[in] spans Array of the validity periods of the rasters, one per raster
 * @param[in] count Number of elements in the arrays
 * @param[in] band Band number (1-based), passed to @p read
 * @param[in] read Function reading one raster along a trajectory
 * @return A temporal float, or @p NULL when the trajectory reads no value
 */
static Temporal *
raster_series_values(const Temporal *traj, const void **values,
  const Span *spans, int count, int band, RasterSeriesRead read)
{
  int *order = raster_series_order(spans, count);
  if (! order)
    return NULL;

  /* A reader reports an error through the error number, which is cleared
   * here and restored when the walk ends without one, as
   * #eafunc_temporal_temporal() does */
  int last_errno = meos_errno_reset();
  Temporal *result = NULL;
  const void **group = palloc(sizeof(void *) * count);
  for (int k = 0; k < count; )
  {
    /* The rows of one period, read together */
    const Span *period = &spans[order[k]];
    int ngroup = 0;
    for ( ; k < count && span_eq(&spans[order[k]], period); k++)
      if (values[order[k]] != NULL)
        group[ngroup++] = values[order[k]];
    if (ngroup == 0)
      continue;
    Temporal *part = temporal_restrict_tstzspan(traj, period, true);
    if (part == NULL)
      continue;
    Temporal *v = read(part, group, ngroup, band);
    pfree(part);
    if (v == NULL && meos_errno())
    {
      if (result)
        pfree(result);
      pfree(group); pfree(order);
      return NULL;
    }
    if (v == NULL)
      continue;
    if (result == NULL)
    {
      result = v;
      continue;
    }
    Temporal *merged = temporal_merge(result, v);
    pfree(result); pfree(v);
    result = merged;
  }
  pfree(group); pfree(order);
  meos_errno_restore(last_errno);
  return result;
}

/**
 * @brief Return true if the values a trajectory reads from a raster series
 * ever, or always, fall inside a float span, or -1 when reading them raised
 * an error
 * @details The ever and always forms read the series and answer from its
 * values as #raster_values_ever() does; a series refused while read is an
 * error, never a value that is not there, as #eafunc_temporal_temporal()
 * answers -1 for an error raised by its lifted function
 * @param[in] v Values read, or @p NULL when none is read or reading failed
 * @param[in] vspan Float value range
 * @param[in] ever True for ever, false for always
 * @param[in] last_errno Error number before the series was read
 */
static int
raster_series_ever(Temporal *v, const Span *vspan, bool ever, int last_errno)
{
  if (! v && meos_errno())
    return -1;
  meos_errno_restore(last_errno);
  return raster_values_ever(v, vspan, ever);
}

/*****************************************************************************
 * A series of PostGIS rasters
 *****************************************************************************/

/**
 * @brief Return true when two rasters cover one footprint: one grid, one
 * extent and one reference system
 * @param[in] r1,r2 Rasters
 */
static bool
raster_same_footprint(const Raster *r1, const Raster *r2)
{
  return raster_srid(r1) == raster_srid(r2) &&
    raster_width(r1) == raster_width(r2) &&
    raster_height(r1) == raster_height(r2) &&
    raster_upper_left_x(r1) == raster_upper_left_x(r2) &&
    raster_upper_left_y(r1) == raster_upper_left_y(r2) &&
    raster_scale_x(r1) == raster_scale_x(r2) &&
    raster_scale_y(r1) == raster_scale_y(r2) &&
    raster_skew_x(r1) == raster_skew_x(r2) &&
    raster_skew_y(r1) == raster_skew_y(r2);
}

/**
 * @brief Return the values the rasters of a series valid over one period
 * read along the part of a trajectory within it
 * @details Each raster is read as #raster_value() reads it by default, the
 * nearest pixel, a nodata pixel read as no value. The rasters of one period
 * are its tiles in space, and where two answer the same time the first to
 * answer it keeps it, as #raster_tile_value_array() keeps the answer of one
 * tile. Two rasters of one footprint are one tile given twice, and are
 * refused where they read different values
 * @param[in] traj Part of the trajectory within the period
 * @param[in] values Rasters valid over the period
 * @param[in] count Number of elements in the arrays
 * @param[in] band Band number (1-based)
 */
static Temporal *
raster_series_read(const Temporal *traj, const void **values, int count,
  int band)
{
  /* The reads are kept so that two rasters of one footprint are compared */
  Temporal **reads = palloc0(sizeof(Temporal *) * count);
  for (int i = 0; i < count; i++)
  {
    reads[i] = raster_value(traj, (const Raster *) values[i], band, true,
      "nearest");
    for (int j = 0; j < i; j++)
    {
      if (raster_same_footprint((const Raster *) values[i],
            (const Raster *) values[j]) &&
          ! ensure_raster_series_agree(reads[i], reads[j]))
      {
        for (int k = 0; k <= i; k++)
          if (reads[k])
            pfree(reads[k]);
        pfree(reads);
        return NULL;
      }
    }
  }

  Temporal *result = NULL;
  for (int i = 0; i < count; i++)
  {
    Temporal *part = reads[i];
    if (part == NULL)
      continue;
    if (result == NULL)
    {
      result = part;
      continue;
    }
    SpanSet *answered = temporal_time(result);
    Temporal *rest = temporal_restrict_tstzspanset(part, answered, REST_MINUS);
    pfree(answered); pfree(part);
    if (rest == NULL)
      continue;
    Temporal *merged = temporal_merge(result, rest);
    pfree(result); pfree(rest);
    result = merged;
  }
  pfree(reads);
  return result;
}

/**
 * @ingroup meos_raster
 * @brief Return the values a trajectory reads from a raster series
 * @details Each raster is read over the part of @p traj within its validity
 * period, in the order of the periods. A part of the trajectory within no
 * period reads no value, and periods that overlap are refused
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rasts Array of the rasters of the series
 * @param[in] spans Array of the validity periods of the rasters, one per raster
 * @param[in] count Number of elements in the arrays
 * @param[in] band Band number (1-based)
 * @return A temporal float, or @p NULL when the trajectory reads no value
 * @errval NULL
 * @csqlfn #Raster_value_series()
 */
Temporal *
raster_value_series(const Temporal *traj, const Raster **rasts,
  const Span *spans, int count, int band)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(traj, NULL); VALIDATE_NOT_NULL(rasts, NULL);
  VALIDATE_NOT_NULL(spans, NULL);
  return raster_series_values(traj, (const void **) rasts, spans, count,
    band, &raster_series_read);
}

/**
 * @ingroup meos_raster
 * @brief Return a trajectory restricted to the times where the value it reads
 * from a raster series falls inside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rasts Array of the rasters of the series
 * @param[in] spans Array of the validity periods of the rasters, one per raster
 * @param[in] count Number of elements in the arrays
 * @param[in] band Band number (1-based)
 * @param[in] vspan Float value range
 * @return A trajectory, or @p NULL when no time qualifies
 * @errval NULL
 * @csqlfn #Raster_at_value_series()
 */
Temporal *
raster_at_value_series(const Temporal *traj, const Raster **rasts,
  const Span *spans, int count, int band, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(vspan, NULL);
  return raster_values_restrict(traj,
    raster_value_series(traj, rasts, spans, count, band), vspan, true);
}

/**
 * @ingroup meos_raster
 * @brief Return a trajectory restricted to the times where the value it reads
 * from a raster series falls outside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rasts Array of the rasters of the series
 * @param[in] spans Array of the validity periods of the rasters, one per raster
 * @param[in] count Number of elements in the arrays
 * @param[in] band Band number (1-based)
 * @param[in] vspan Float value range
 * @return A trajectory, or @p NULL when no time qualifies
 * @errval NULL
 * @csqlfn #Raster_minus_value_series()
 */
Temporal *
raster_minus_value_series(const Temporal *traj, const Raster **rasts,
  const Span *spans, int count, int band, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(vspan, NULL);
  return raster_values_restrict(traj,
    raster_value_series(traj, rasts, spans, count, band), vspan, false);
}

/**
 * @ingroup meos_raster
 * @brief Return true if the value a trajectory reads from a raster series
 * ever falls inside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rasts Array of the rasters of the series
 * @param[in] spans Array of the validity periods of the rasters, one per raster
 * @param[in] count Number of elements in the arrays
 * @param[in] band Band number (1-based)
 * @param[in] vspan Float value range
 * @errval -1
 * @csqlfn #Eraster_value_series()
 */
int
eraster_value_series(const Temporal *traj, const Raster **rasts,
  const Span *spans, int count, int band, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(traj, -1); VALIDATE_NOT_NULL(rasts, -1);
  VALIDATE_NOT_NULL(spans, -1); VALIDATE_NOT_NULL(vspan, -1);
  if (! ensure_raster_series(spans, count))
    return -1;
  int last_errno = meos_errno_reset();
  return raster_series_ever(
    raster_value_series(traj, rasts, spans, count, band), vspan, true,
    last_errno);
}

/**
 * @ingroup meos_raster
 * @brief Return true if every value a trajectory reads from a raster series
 * falls inside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rasts Array of the rasters of the series
 * @param[in] spans Array of the validity periods of the rasters, one per raster
 * @param[in] count Number of elements in the arrays
 * @param[in] band Band number (1-based)
 * @param[in] vspan Float value range
 * @errval -1
 * @csqlfn #Araster_value_series()
 */
int
araster_value_series(const Temporal *traj, const Raster **rasts,
  const Span *spans, int count, int band, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(traj, -1); VALIDATE_NOT_NULL(rasts, -1);
  VALIDATE_NOT_NULL(spans, -1); VALIDATE_NOT_NULL(vspan, -1);
  if (! ensure_raster_series(spans, count))
    return -1;
  int last_errno = meos_errno_reset();
  return raster_series_ever(
    raster_value_series(traj, rasts, spans, count, band), vspan, false,
    last_errno);
}

/*****************************************************************************
 * A series of Raquet tiles
 *****************************************************************************/

/**
 * @brief Return the values the tiles of a series valid over one period read
 * along the part of a trajectory within it, as #raster_tile_value_array()
 * reads them, the tile of the finest zoom answering where tiles overlap
 * @param[in] traj Part of the trajectory within the period
 * @param[in] values Raquet tiles valid over the period
 * @param[in] count Number of elements in the arrays
 * @param[in] band Unused, a tile holding one band
 * @note Mirrors #raster_series_read(), which refuses two rasters of one
 * footprint reading different values, a footprint of a tile being its cell
 */
static Temporal *
raster_tile_series_read(const Temporal *traj, const void **values, int count,
  int band)
{
  (void) band;
  /* Two tiles of one QUADBIN cell are one tile given twice, which a Raquet
   * file never holds, and are refused where they read different values */
  for (int i = 1; i < count; i++)
  {
    for (int j = 0; j < i; j++)
    {
      if (raquet_quadbin((const Raquet *) values[i]) !=
          raquet_quadbin((const Raquet *) values[j]))
        continue;
      Temporal *vi = raster_tile_value(traj, (const Raquet *) values[i]);
      Temporal *vj = raster_tile_value(traj, (const Raquet *) values[j]);
      bool agree = ensure_raster_series_agree(vi, vj);
      if (vi)
        pfree(vi);
      if (vj)
        pfree(vj);
      if (! agree)
        return NULL;
    }
  }
  return raster_tile_value_array(traj, (const Raquet **) values, count);
}

/**
 * @ingroup meos_raster
 * @brief Return the values a trajectory reads from a series of Raquet tiles
 * @details Each tile is read over the part of @p traj within its validity
 * period, in the order of the periods. A part of the trajectory within no
 * period reads no value, and periods that overlap are refused
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rqarr Array of the Raquet tiles of the series
 * @param[in] spans Array of the validity periods of the tiles, one per tile
 * @param[in] count Number of elements in the arrays
 * @return A temporal float, or @p NULL when the trajectory reads no value
 * @errval NULL
 * @csqlfn #Raster_tile_value_series()
 */
Temporal *
raster_tile_value_series(const Temporal *traj, const Raquet **rqarr,
  const Span *spans, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(traj, NULL); VALIDATE_NOT_NULL(rqarr, NULL);
  VALIDATE_NOT_NULL(spans, NULL);
  return raster_series_values(traj, (const void **) rqarr, spans, count, 1,
    &raster_tile_series_read);
}

/**
 * @ingroup meos_raster
 * @brief Return a trajectory restricted to the times where the value it reads
 * from a series of Raquet tiles falls inside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rqarr Array of the Raquet tiles of the series
 * @param[in] spans Array of the validity periods of the tiles, one per tile
 * @param[in] count Number of elements in the arrays
 * @param[in] vspan Float value range
 * @return A trajectory, or @p NULL when no time qualifies
 * @errval NULL
 * @csqlfn #Raster_tile_at_value_series()
 */
Temporal *
raster_tile_at_value_series(const Temporal *traj, const Raquet **rqarr,
  const Span *spans, int count, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(vspan, NULL);
  return raster_values_restrict(traj,
    raster_tile_value_series(traj, rqarr, spans, count), vspan, true);
}

/**
 * @ingroup meos_raster
 * @brief Return a trajectory restricted to the times where the value it reads
 * from a series of Raquet tiles falls outside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rqarr Array of the Raquet tiles of the series
 * @param[in] spans Array of the validity periods of the tiles, one per tile
 * @param[in] count Number of elements in the arrays
 * @param[in] vspan Float value range
 * @return A trajectory, or @p NULL when no time qualifies
 * @errval NULL
 * @csqlfn #Raster_tile_minus_value_series()
 */
Temporal *
raster_tile_minus_value_series(const Temporal *traj, const Raquet **rqarr,
  const Span *spans, int count, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(vspan, NULL);
  return raster_values_restrict(traj,
    raster_tile_value_series(traj, rqarr, spans, count), vspan, false);
}

/**
 * @ingroup meos_raster
 * @brief Return true if the value a trajectory reads from a series of Raquet
 * tiles ever falls inside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rqarr Array of the Raquet tiles of the series
 * @param[in] spans Array of the validity periods of the tiles, one per tile
 * @param[in] count Number of elements in the arrays
 * @param[in] vspan Float value range
 * @errval -1
 * @csqlfn #Eraster_tile_value_series()
 */
int
eraster_tile_value_series(const Temporal *traj, const Raquet **rqarr,
  const Span *spans, int count, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(traj, -1); VALIDATE_NOT_NULL(rqarr, -1);
  VALIDATE_NOT_NULL(spans, -1); VALIDATE_NOT_NULL(vspan, -1);
  if (! ensure_raster_series(spans, count))
    return -1;
  int last_errno = meos_errno_reset();
  return raster_series_ever(
    raster_tile_value_series(traj, rqarr, spans, count), vspan, true,
    last_errno);
}

/**
 * @ingroup meos_raster
 * @brief Return true if every value a trajectory reads from a series of
 * Raquet tiles falls inside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rqarr Array of the Raquet tiles of the series
 * @param[in] spans Array of the validity periods of the tiles, one per tile
 * @param[in] count Number of elements in the arrays
 * @param[in] vspan Float value range
 * @errval -1
 * @csqlfn #Araster_tile_value_series()
 */
int
araster_tile_value_series(const Temporal *traj, const Raquet **rqarr,
  const Span *spans, int count, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(traj, -1); VALIDATE_NOT_NULL(rqarr, -1);
  VALIDATE_NOT_NULL(spans, -1); VALIDATE_NOT_NULL(vspan, -1);
  if (! ensure_raster_series(spans, count))
    return -1;
  int last_errno = meos_errno_reset();
  return raster_series_ever(
    raster_tile_value_series(traj, rqarr, spans, count), vspan, false,
    last_errno);
}

/*****************************************************************************/
