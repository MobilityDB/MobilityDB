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
 * @brief Edge extraction and R-tree indexing utilities for 2D geometries.
 *
 * Provides the geometry-edge enumeration and an R-tree of per-edge bounding
 * boxes. Used by the temporal point clipping path and by other operators that
 * need to query "which edges of a static geometry are near a moving object"
 * without paying O(N_edges) per call.
 */

#ifndef __TPOINT_GEOM_CLIP_H__
#define __TPOINT_GEOM_CLIP_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "temporal/meos_catalog.h"
#include "temporal/temporal_rtree.h"

/*****************************************************************************/

/**
 * @brief Enumeration defining the edge types
 */
typedef enum
{
  EDGE_POINT = 0,
  EDGE_LINE,
  EDGE_POLY
} EdgeType;

/**
 * @brief Structure keeping a geometry edge
 */
typedef struct
{
  double x1, y1, x2, y2;         /**< Coordinates of the start/end 2D points */
  double xmin, ymin, xmax, ymax; /**< Precomputed bounding box of the edge */
  double dx, dy, length;         /**< Precomputed dx, dy, and length */
  EdgeType etype;                /**< Edge type */
} Edge;

/*****************************************************************************/

extern MeosArray *geom_extract_edges(const LWGEOM *geom);

extern RTree *build_edge_rtree(const Edge *edges, int nedges, int32_t srid);

extern int point_in_polygon(double x, double y, Edge **edges, int nedges);

/*****************************************************************************/

#endif /* __TPOINT_GEOM_CLIP_H__ */
