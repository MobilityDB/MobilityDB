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
 * @brief Conformance classes a single pose can be written as.
 * @details The two Basic classes differ only in how they carry the
 * orientation. The Advanced class differs in the frame: it names its outer
 * frame explicitly where the Basic classes leave it implicit, and carries the
 * position inside that frame, having no member of its own for it.
 *
 * The class of a composite follows from the value instead of from this
 * choice: a temporal sequence is a Composite Sequence Series, Regular or
 * Irregular as its instants are spaced, and a Stream is written by the two
 * entry points of its own. A Series carries a quaternion in every inner frame
 * and offers no orientation choice.
 *
 * The Chain class is written from a temporal pose chain, whose links are the
 * nested frames it names. The Graph class is not implemented.
 */
typedef enum
{
  GEOPOSE_BASIC_QUATERNION = 0,  /**< {position, quaternion} canonical form */
  GEOPOSE_BASIC_YPR        = 1,  /**< {position, angles} (yaw/pitch/roll) */
  GEOPOSE_ADVANCED         = 2   /**< {frameSpecification, quaternion} */
} GeoPoseClass;

/*****************************************************************************/

#endif /* __POSE_GEOPOSE_H__ */
