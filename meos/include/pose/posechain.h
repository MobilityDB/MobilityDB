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
 * @brief Basic functions for static pose chain objects.
 */

#ifndef __POSECHAIN_H__
#define __POSECHAIN_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include "pose/pose.h"

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/**
 * @brief Structure to represent pose chain values
 * @details A pose chain is an ordered list of at least one pose. The first
 * link is expressed in the chain's outer frame, which is the frame the SRID
 * names; every later link is expressed in the frame the link before it
 * defines. Clause 4.2.8 of OGC GeoPose 1.0 forbids an inner frame from being
 * topocentric, so a chain has exactly one outer frame, hence one SRID and one
 * dimension for all of its links.
 *
 * flags (8 bits, x = unused): xxZXxxxx, where the geodetic bit is set for a
 * chain whose outer frame is geographic
 * data: 2D: count blocks of [x, y, theta]
 *       3D: count blocks of [x, y, z, W, X, Y, Z]
 */
struct PoseChain
{
  int32         vl_len_;       /**< varlena header (do not touch directly!) */
  int8          flags;         /**< flags */
  uint8_t       srid[3];       /**< srid */
  int32         count;         /**< number of links, always at least one */
  double        data[];        /**< position and orientation values */
};

/**
 * @brief Return the number of doubles a single link of a pose chain occupies
 */
#define POSECHAIN_LINK_SIZE(pc) (MEOS_FLAGS_GET_Z((pc)->flags) ? 7 : 3)

/**
 * @brief Return a pointer to the values of the n-th link of a pose chain
 * @note The link index is zero-based
 */
#define POSECHAIN_LINK_PTR(pc, n) \
  ((pc)->data + (size_t) (n) * POSECHAIN_LINK_SIZE(pc))

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetPoseChainP(X)      ((PoseChain *) DatumGetPointer(X))
#define PoseChainPGetDatum(X)      PointerGetDatum(X)
#define PG_GETARG_POSECHAIN_P(X)   DatumGetPoseChainP(PG_GETARG_DATUM(X))
#define PG_RETURN_POSECHAIN_P(X)   PG_RETURN_POINTER(X)

/*****************************************************************************/

/* Validity functions */

extern bool ensure_valid_posechain_posechain(const PoseChain *pc1,
  const PoseChain *pc2);
extern bool ensure_valid_posechainset_posechain(const Set *s,
  const PoseChain *pc);

/* Input/output functions */

extern PoseChain *posechain_parse(const char **str, bool end);
extern char *posechain_wkt_out(const PoseChain *pc, bool extended, int maxdd);

/* Interpolation functions */

extern bool ensure_same_count_posechain(const PoseChain *pc1,
  const PoseChain *pc2);
extern PoseChain *posechainsegm_interpolate(const PoseChain *start,
  const PoseChain *end, double ratio);
extern long double posechainsegm_locate(const PoseChain *start,
  const PoseChain *end, const PoseChain *value);
extern bool posechain_collinear(const PoseChain *pc1, const PoseChain *pc2,
  const PoseChain *pc3, double ratio);

/* Transformation functions */

extern Datum datum_posechain_round(Datum pc, Datum size);
extern Datum datum_posechain_pose(Datum pc);

/* Spatial reference system functions */

extern void posechain_set_srid_int(PoseChain *pc, int32_t srid);
extern PoseChain *posechain_transf_pj(const PoseChain *pc, int32_t srid_to,
  const LWPROJ *pj);

/* Box functions */

extern bool posechain_set_stbox(const PoseChain *pc, STBox *box);
extern void posechainarr_set_stbox(const Datum *values, int count, STBox *box);

/*****************************************************************************/

#endif /* __POSECHAIN_H__ */
