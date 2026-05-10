/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
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
 *****************************************************************************/

/**
 * @file
 * @brief Polygon Boolean clipping for the temporal-point spatial functions.
 * @note Backed by Clipper2 v2.0.1 (vendored under @c clipper2/).
 *       Replaces the prior Martinez-Rueda port; the legacy implementation
 *       remains available on the @c origin/martinez-rebased branch for
 *       reference.
 */

#ifndef __GEO_POLY_CLIP_H__
#define __GEO_POLY_CLIP_H__

#include <postgres.h>
#include <liblwgeom.h>

/*****************************************************************************/

/**
 * @brief Boolean operation selector for #clip_poly_poly.
 *
 * Numeric values are part of the SQL ABI used by the four
 * @c _mdb_internal_clip_* SQL functions defined in
 * @c mobilitydb/sql/geo/056_tpoint_spatialfuncs.in.sql; do not renumber.
 */
typedef enum
{
  CL_INTERSECTION = 0,
  CL_UNION        = 1,
  CL_DIFFERENCE   = 2,
  CL_XOR          = 3,
} ClipOper;

/*****************************************************************************/

extern GSERIALIZED *clip_poly_poly(const GSERIALIZED *subj,
  const GSERIALIZED *clip, ClipOper operation);

/*****************************************************************************/

#endif /* __GEO_POLY_CLIP_H__ */
