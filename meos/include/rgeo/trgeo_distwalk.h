/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Closest-feature walk for the distance between rigid geometries
 * @details See meos/src/rgeo/trgeo_distwalk.c.
 */

#ifndef __TRGEO_DISTWALK_H__
#define __TRGEO_DISTWALK_H__

/* MEOS */
#include "pose/pose.h"
#include "rgeo/trgeo_distsolve.h"

/*****************************************************************************
 * Motion in one temporal segment
 *
 * In a segment, the time t goes from 0 to 1.  A body translates linearly and
 * rotates at a constant rate about its rotation center.  A point a of the
 * body, given relative to the rotation center, moves as
 *
 *   v(t) = R(th0 + w*t) * a + (cx, cy) + t * (dx, dy)
 *
 * where R(x) is the rotation by the angle x.  A static geometry has w = 0 and
 * (dx, dy) = 0.
 *****************************************************************************/

/**
 * @brief Motion of a body in one segment
 */
typedef struct
{
  double cx, cy;   /**< Rotation center at t = 0 */
  double dx, dy;   /**< Displacement of the rotation center */
  double th0;      /**< Rotation angle at t = 0 */
  double w;        /**< Change of the rotation angle */
} DistMotion;

/**
 * @brief Motion of two bodies A and B in one segment
 * @details W(t) = (cB - cA) + t * (dB - dA) is the offset between the two
 * rotation centers.
 */
typedef struct
{
  DistMotion ma;             /**< Motion of A */
  DistMotion mb;             /**< Motion of B */
  double wx1, wx0;           /**< W_x(t) = wx1 * t + wx0 */
  double wy1, wy0;           /**< W_y(t) = wy1 * t + wy0 */
} DistSegm;

/**
 * @brief An edge of a body, relative to its rotation center
 */
typedef struct
{
  double sx, sy;   /**< Start vertex */
  double ex, ey;   /**< End vertex */
  double len;      /**< Length */
  double beta;     /**< Direction angle */
  double dots;     /**< dot(start, end - start) */
  double crss;     /**< cross(start, end - start) */
} DistRefEdge;

/**
 * @brief A vertex of a body, relative to its rotation center
 */
typedef struct
{
  double x, y;
  double rho;      /**< Distance to the rotation center */
  double alpha;    /**< Angle about the rotation center */
} DistRefVert;

/**
 * @brief A convex polygon, a segment or a point, relative to its rotation
 * center
 * @details The vertices of a polygon are in counterclockwise order, and no
 * vertex repeats or lies on the line of its two neighbors.  Edge i goes from
 * vertex i to vertex i + 1.  A segment has two vertices and two edges in
 * opposite directions.  A point has one vertex and no edges.
 */
typedef struct
{
  int nvert;             /**< Number of vertices */
  int nedge;             /**< Number of edges: nvert, or 0 for a point */
  double rmax;           /**< Largest distance of a vertex to the center */
  DistRefVert *verts;    /**< Vertices */
  DistRefEdge *edges;    /**< Edges */
} DistRefPoly;

extern void distmotion_set(DistMotion *m, double x1, double y1, double th1,
  double x2, double y2, double th2);
extern void distmotion_set_static(DistMotion *m, double x, double y);
extern void distmotion_from_pose(DistMotion *m, const Pose *p1,
  const Pose *p2);
extern void distmotion_place(const DistMotion *m, double px, double py,
  double t, double *x, double *y);
extern void distsegm_set(DistSegm *seg, const DistMotion *ma,
  const DistMotion *mb);
extern void distrefedge_set(DistRefEdge *e, double sx, double sy, double ex,
  double ey);
extern void distrefvert_set(DistRefVert *v, double x, double y);

extern bool distrefpoly_set(DistRefPoly *rp, const LWPOLY *poly);
extern bool distrefpoly_set_static(DistRefPoly *rp, const LWPOLY *poly,
  double *cx, double *cy);
extern void distrefpoly_set_segment(DistRefPoly *rp, double x1, double y1,
  double x2, double y2, double *cx, double *cy);
extern void distrefpoly_set_point(DistRefPoly *rp);
extern void distrefpoly_free(DistRefPoly *rp);

/*****************************************************************************
 * Functions of the walk
 *
 * Each function gives a #DistFun for an edge of A and a vertex of B, or for
 * two vertices, or for two edges.  The frequencies are wA, wB and wA - wB.
 *****************************************************************************/

extern void distwalk_transition(const DistSegm *seg, const DistRefEdge *e,
  const DistRefVert *v, DistFun *f);
extern void distwalk_cross(const DistSegm *seg, const DistRefEdge *e,
  const DistRefVert *v, DistFun *f);
extern void distwalk_vertdist2(const DistSegm *seg, const DistRefVert *va,
  const DistRefVert *vb, DistFun *f);
extern void distwalk_parallel(const DistSegm *seg, const DistRefEdge *ea,
  const DistRefEdge *eb, DistFun *f);

/*****************************************************************************
 * The walk
 *
 * A is a convex polygon, and B is a convex polygon, a segment or a point.
 * The distance between A and B is the distance from the origin to the
 * Minkowski difference A - B.  The features of the difference are pairs of
 * features of A and B:
 *
 *   VV(i,j)  the vertex a_i - b_j
 *   EV(i,j)  the edge from a_i - b_j to a_{i+1} - b_j
 *   VE(i,j)  the edge from a_i - b_j to a_i - b_{j+1}
 *
 * If B is a point, the difference is A moved by -b_0 and has no VE edge.
 *****************************************************************************/

/* Kinds of feature of the Minkowski difference */
#define DISTPAIR_NONE  0   /**< Unknown feature */
#define DISTPAIR_VV    1   /**< Vertex i of A and vertex j of B */
#define DISTPAIR_EV    2   /**< Edge i of A and vertex j of B */
#define DISTPAIR_VE    3   /**< Vertex i of A and edge j of B */

/**
 * @brief A feature of the Minkowski difference
 */
typedef struct
{
  int kind;              /**< One of the DISTPAIR_* values */
  uint32_t i;            /**< Index in A */
  uint32_t j;            /**< Index in B */
} DistPair;

/**
 * @brief Distance at an instant of a segment
 */
typedef struct
{
  double t;              /**< Time in [0, 1] */
  double dist;           /**< Distance */
} DistEvent;

/**
 * @brief Growable array of #DistEvent
 */
typedef struct
{
  DistEvent *ev;
  int count;
  int size;
} DistEvents;

/* Results of #distwalk_segm() */
#define DISTWALK_OK       0   /**< Success */
#define DISTWALK_CYCLE  (-1)  /**< The walk does not advance */
#define DISTWALK_SOLVE  (-2)  /**< The root isolation failed */

extern void distevents_init(DistEvents *events);
extern void distevents_free(DistEvents *events);

extern double distwalk_ftol(const DistSegm *seg, const DistRefPoly *ra,
  const DistRefPoly *rb);
extern int distwalk_segm(const DistSegm *seg, const DistRefPoly *ra,
  const DistRefPoly *rb, double ftol, double level, DistPair *cf,
  DistEvents *events);

/*****************************************************************************/

#endif /* __TRGEO_DISTWALK_H__ */
