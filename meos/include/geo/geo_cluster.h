/***********************************************************************
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
 * @brief Clustering of geometries by proximity, without GEOS
 */

#ifndef __GEO_CLUSTER_H__
#define __GEO_CLUSTER_H__

/* PostGIS */
#include "liblwgeom.h"
#include "lwunionfind.h"

/*****************************************************************************/

extern bool geo_union_dbscan(LWGEOM **geoms, uint32_t ngeoms, UNIONFIND *uf,
  double eps, uint32_t minpoints, char **in_cluster);
extern bool geo_combine_clusters(UNIONFIND *uf, LWGEOM **geoms,
  uint32_t ngeoms, LWGEOM ***clusters, uint32_t *nclusters);
extern bool geo_cluster_within_distance(LWGEOM **geoms, uint32_t ngeoms,
  double tolerance, LWGEOM ***clusters, uint32_t *nclusters);

/*****************************************************************************/

#endif /* __GEO_CLUSTER_H__ */
