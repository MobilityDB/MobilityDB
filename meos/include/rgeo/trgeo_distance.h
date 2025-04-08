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
 * @brief Distance functions for temporal rigid geometries
 */

#ifndef __TRGEO_DISTANCE_H__
#define __TRGEO_DISTANCE_H__

#include <postgres.h>
#include <liblwgeom.h>
#include "general/temporal.h"

#include "pose/pose.h"

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/** Symbolic constants for temporal_distance */
#define MEOS_ANY             0
#define MEOS_RIGHT           1
#define MEOS_LEFT            2

/** Symbolic constants for equation solving */
#define MEOS_SOLVE_0         true
#define MEOS_SOLVE_1         false

/** Symbolic constants for cfp_elem */
#define MEOS_CFP_STORE       true
#define MEOS_CFP_STORE_NO    false

#define MEOS_CFP_FREE        true
#define MEOS_CFP_FREE_NO     false

/* Closest features pair */

typedef struct {
  LWGEOM *geom_1;
  LWGEOM *geom_2;
  Pose *pose_1;
  Pose *pose_2;
  bool free_pose_1;
  bool free_pose_2;
  uint32_t cf_1;
  uint32_t cf_2;
  TimestampTz t;
  bool store;
} cfp_elem;

/* List of CFPs */

typedef struct {
  size_t count;
  size_t size;
  cfp_elem *arr;
} cfp_array;

/* Closest features pair */

typedef struct {
  double dist;;
  TimestampTz t;
} tdist_elem;

/* List of CFPs */

typedef struct {
  size_t count;
  size_t size;
  tdist_elem *arr;
} tdist_array;

/*****************************************************************************/

#endif /* __TRGEO_DISTANCE_H__ */
