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
 * @brief Comparison operators for the tquadbin type.
 *
 * Analogue of `meos/src/h3/th3index_compops.c`. Every ever /
 * always / temporal comparison flavour for tquadbin is mirrored
 * from the th3index pattern, with the bare-cell operand carried as
 * `Quadbin` (int64).
 *
 * For QUADBIN cells, equality / inequality is exactly int64 bit-equality —
 * the `datum2_eq` / `datum2_ne` primitives used by the tbigint
 * machinery already have the right semantics, so the per-instant
 * comparison primitives here are shallow reuses.
 *
 * Ordering operators (`<`, `<=`, `>`, `>=`) are NOT lifted as spatial
 * comparisons — only equality / inequality (`?= %= ?<> %<> #= #<>`) are
 * exposed, matching the th3index family. Users who want raw int64 order
 * can cast to tbigint.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_quadbin.h>
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "temporal/type_util.h"
/* tquadbin */
#include "quadbin/tquadbin.h"
#include "quadbin/quadbin.h"
#include "quadbin/quadbin_meos.h"   /* DatumGetQuadbin / QuadbinGetDatum */

/*****************************************************************************
 * Internal dispatchers
 *****************************************************************************/

/**
 * @brief Return `true` if a temporal QUADBIN cell and a bare QUADBIN cell satisfy
 * the ever/always comparison.
 * @param[in] temp Temporal QUADBIN cell
 * @param[in] cell Bare QUADBIN cell
 * @param[in] func Per-instant comparison primitive
 * @param[in] ever True for ever semantics, false for always
 */
static int
eacomp_tquadbin_quadbin(const Temporal *temp, Quadbin cell,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  if (! ensure_valid_tquadbin_quadbin(temp, cell))
    return -1;
  return eacomp_temporal_base(temp, QuadbinGetDatum(cell), func, ever);
}

/**
 * @brief Return `true` if two temporal QUADBIN cells satisfy the ever/always
 * comparison.
 */
static int
eacomp_tquadbin_tquadbin(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  if (! ensure_valid_tquadbin_tquadbin(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************
 * Ever/always equality
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if a temporal QUADBIN cell is ever equal to a bare QUADBIN cell.
 */
inline int
ever_eq_tquadbin_quadbin(const Temporal *temp, Quadbin cell)
{
  return eacomp_tquadbin_quadbin(temp, cell, &datum2_eq, EVER);
}

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if a bare QUADBIN cell is ever equal to a temporal QUADBIN cell.
 */
inline int
ever_eq_quadbin_tquadbin(Quadbin cell, const Temporal *temp)
{
  return eacomp_tquadbin_quadbin(temp, cell, &datum2_eq, EVER);
}

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if a temporal QUADBIN cell is ever not equal to a bare
 * QUADBIN cell.
 */
inline int
ever_ne_tquadbin_quadbin(const Temporal *temp, Quadbin cell)
{
  return eacomp_tquadbin_quadbin(temp, cell, &datum2_ne, EVER);
}

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if a bare QUADBIN cell is ever not equal to a temporal
 * QUADBIN cell.
 */
inline int
ever_ne_quadbin_tquadbin(Quadbin cell, const Temporal *temp)
{
  return eacomp_tquadbin_quadbin(temp, cell, &datum2_ne, EVER);
}

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if a temporal QUADBIN cell is always equal to a bare
 * QUADBIN cell.
 */
inline int
always_eq_tquadbin_quadbin(const Temporal *temp, Quadbin cell)
{
  return eacomp_tquadbin_quadbin(temp, cell, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if a bare QUADBIN cell is always equal to a temporal
 * QUADBIN cell.
 */
inline int
always_eq_quadbin_tquadbin(Quadbin cell, const Temporal *temp)
{
  return eacomp_tquadbin_quadbin(temp, cell, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if a temporal QUADBIN cell is always not equal to a bare
 * QUADBIN cell.
 */
inline int
always_ne_tquadbin_quadbin(const Temporal *temp, Quadbin cell)
{
  return eacomp_tquadbin_quadbin(temp, cell, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if a bare QUADBIN cell is always not equal to a temporal
 * QUADBIN cell.
 */
inline int
always_ne_quadbin_tquadbin(Quadbin cell, const Temporal *temp)
{
  return eacomp_tquadbin_quadbin(temp, cell, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Two-temporal ever/always
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if two temporal QUADBIN cells are ever equal at some
 * shared instant.
 */
inline int
ever_eq_tquadbin_tquadbin(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tquadbin_tquadbin(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if two temporal QUADBIN cells are ever unequal at some
 * shared instant.
 */
inline int
ever_ne_tquadbin_tquadbin(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tquadbin_tquadbin(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if two temporal QUADBIN cells are always equal across
 * their shared time axis.
 */
inline int
always_eq_tquadbin_tquadbin(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tquadbin_tquadbin(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_quadbin_comp_ever
 * @brief Return `true` if two temporal QUADBIN cells are always unequal across
 * their shared time axis.
 */
inline int
always_ne_tquadbin_tquadbin(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tquadbin_tquadbin(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Temporal equality / inequality — return a tbool trajectory of per-instant
 * truth values.
 *****************************************************************************/

/**
 * @brief Internal dispatcher for per-instant temporal comparisons against a
 * bare QUADBIN cell. Equality / inequality are commutative, so the (cell, temp)
 * and (temp, cell) entry points share a single implementation.
 */
static Temporal *
tcomp_tquadbin_quadbin(const Temporal *temp, Quadbin cell,
  Datum (*func)(Datum, Datum, MeosType))
{
  if (! ensure_valid_tquadbin_quadbin(temp, cell))
    return NULL;
  return tcomp_temporal_base(temp, QuadbinGetDatum(cell), func);
}

static Temporal *
tcomp_quadbin_tquadbin(Quadbin cell, const Temporal *temp,
  Datum (*func)(Datum, Datum, MeosType))
{
  if (! ensure_valid_tquadbin_quadbin(temp, cell))
    return NULL;
  return tcomp_temporal_base(temp, QuadbinGetDatum(cell), func);
}

/**
 * @ingroup meos_quadbin_comp_temp
 * @brief Return the temporal equality between a temporal QUADBIN cell and a
 * bare QUADBIN cell.
 */
inline Temporal *
teq_tquadbin_quadbin(const Temporal *temp, Quadbin cell)
{
  return tcomp_tquadbin_quadbin(temp, cell, &datum2_eq);
}

/**
 * @ingroup meos_quadbin_comp_temp
 * @brief Return the temporal equality between a bare QUADBIN cell and a
 * temporal QUADBIN cell.
 */
inline Temporal *
teq_quadbin_tquadbin(Quadbin cell, const Temporal *temp)
{
  return tcomp_quadbin_tquadbin(cell, temp, &datum2_eq);
}

/**
 * @ingroup meos_quadbin_comp_temp
 * @brief Return the temporal inequality between a temporal QUADBIN cell and a
 * bare QUADBIN cell.
 */
inline Temporal *
tne_tquadbin_quadbin(const Temporal *temp, Quadbin cell)
{
  return tcomp_tquadbin_quadbin(temp, cell, &datum2_ne);
}

/**
 * @ingroup meos_quadbin_comp_temp
 * @brief Return the temporal inequality between a bare QUADBIN cell and a
 * temporal QUADBIN cell.
 */
inline Temporal *
tne_quadbin_tquadbin(Quadbin cell, const Temporal *temp)
{
  return tcomp_quadbin_tquadbin(cell, temp, &datum2_ne);
}

/**
 * @ingroup meos_quadbin_comp_temp
 * @brief Return the temporal equality of two temporal QUADBIN cells across
 * their shared time axis.
 */
inline Temporal *
teq_tquadbin_tquadbin(const Temporal *temp1, const Temporal *temp2)
{
  if (! ensure_valid_tquadbin_tquadbin(temp1, temp2))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_eq);
}

/**
 * @ingroup meos_quadbin_comp_temp
 * @brief Return the temporal inequality of two temporal QUADBIN cells across
 * their shared time axis.
 */
inline Temporal *
tne_tquadbin_tquadbin(const Temporal *temp1, const Temporal *temp2)
{
  if (! ensure_valid_tquadbin_tquadbin(temp1, temp2))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_ne);
}

/*****************************************************************************/
