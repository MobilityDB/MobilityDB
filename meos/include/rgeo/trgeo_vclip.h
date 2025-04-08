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
 * @brief Distance functions for temporal rigid geometries.
 */

#ifndef __TRGEO_VCLIP_H__
#define __TRGEO_VCLIP_H__

#include <c.h>
#include <postgres.h>
#include <liblwgeom.h>
#include "general/temporal.h"
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

extern int v_clip_tpoly_point(const LWPOLY *poly, const LWPOINT *point,
  const Pose *pose, uint32_t *poly_feature, double *dist);
extern int v_clip_tpoly_tpoly(const LWPOLY *poly1, const LWPOLY *poly2,
  const Pose *pose1, const Pose *pose2, uint32_t *poly1_feature, 
  uint32_t *poly2_feature, double *dist);

/*****************************************************************************/

#endif
