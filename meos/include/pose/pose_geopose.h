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
 * @brief OGC GeoPose JSON I/O — Basic-YPR and Basic-Quaternion conformance.
 */

#ifndef __POSE_GEOPOSE_H__
#define __POSE_GEOPOSE_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>

/*****************************************************************************
 * GeoPose conformance classes
 *****************************************************************************/

/**
 * @brief Orientation encodings of an OGC GeoPose Basic document.
 * @details The two Basic conformance classes differ only in how they carry
 * the orientation, so one value chooses between them. It is orthogonal to
 * the target conformance class, which follows from the value being written:
 * a pose and a single temporal instant are Basic documents, a temporal
 * sequence is a Composite Sequence Series. A Series carries a quaternion in
 * every inner frame and offers no such choice.
 *
 * The Advanced, Chain, Graph and Stream classes are not implemented.
 */
typedef enum
{
  GEOPOSE_BASIC_QUATERNION = 0,  /**< {position, quaternion} canonical form */
  GEOPOSE_BASIC_YPR        = 1   /**< {position, angles} (yaw/pitch/roll) */
} GeoPoseClass;

/*****************************************************************************/

#endif /* __POSE_GEOPOSE_H__ */
