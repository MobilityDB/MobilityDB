/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Distance functions for temporal rigid geometries.
 */

#ifndef __TRGEO_VCLIP_H__
#define __TRGEO_VCLIP_H__

/* C */
#include <c.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include "temporal/temporal.h"
#include "pose/pose.h"

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/** Max iterations to avoid infinite loops */
#define MEOS_MAX_ITERS      1000

/** Symbolic constants for v-clip */
#define MEOS_CONTINUE       0
#define MEOS_DISJOINT       1
#define MEOS_INTERSECT     -1

/*****************************************************************************/

/* V-clip functions */

/*****************************************************************************
 * Inline helpers shared by the v-clip walk and the temporal distance
 *****************************************************************************/

/**
 * @brief Return the sum of two vertex numbers modulo the vertex count
 */
static inline uint32_t
uint_mod_add(uint32_t i, uint32_t j, uint32_t n)
{
  return (i + j) % n;
}

/**
 * @brief Return the difference of two vertex numbers modulo the vertex count
 * @pre j < n, so that adding @p n keeps the difference positive
 */
static inline uint32_t
uint_mod_sub(uint32_t i, uint32_t j, uint32_t n)
{
  return (i + n - j) % n;
}

/**
 * @brief Return the relative position of a point on a segment
 * @details
 * s < 0      -> p before point vs
 * s = 0      -> p = vs
 * 0 < s < 1  -> p = vs * (1 - s)  + ve * s
 * s = 1      -> p = ve
 * 1 < s      -> p after point ve
 */
static inline double
compute_s(POINT4D p, POINT4D vs, POINT4D ve)
{
  return ((p.x - vs.x) * (ve.x - vs.x) + (p.y - vs.y) * (ve.y - vs.y)) /
    ((ve.x - vs.x) * (ve.x - vs.x) + (ve.y - vs.y) * (ve.y - vs.y));
}

extern int v_clip_tpoly_point(const LWPOLY *poly, const LWPOINT *point,
  const Pose *pose, uint32_t *poly_feature, double *dist);
extern int v_clip_tpoly_tpoly(const LWPOLY *poly1, const LWPOLY *poly2,
  const Pose *pose1, const Pose *pose2, uint32_t *poly1_feature,
  uint32_t *poly2_feature, double *dist);

extern void apply_pose_point4d(POINT4D *p, const Pose *pose);

/*****************************************************************************/

#endif
