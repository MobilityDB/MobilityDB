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
 * @brief The integer inputs of the points of the outline of an H3 cell
 * @details The two loops below follow `_faceIjkToCellBoundary` and
 * `_faceIjkPentToCellBoundary` of the vendored h3lib/lib/faceijk.c, step for
 * step, and record the inputs H3 gives `_hex2dToVec3` where those functions
 * compute coordinates (#h3_vertex_recipe, #h3_edge_extra_recipe), so that
 * #gnomonic_face_point_nearest states the point exactly. The loops read the
 * tables of faceijk.c (`adjacentFaceDir`, `faceNeighbors`, `maxDimByCIIres`,
 * `unitScaleByCIIres`), which are static there, so this file includes it
 * rather than linking it; it includes vertex.c for the same reason, to read
 * `vertexRotations` and its direction tables (#h3_cell_vertexes). It is built with the vendored library in the object
 * library h3core, whose definitions stay out of the dynamic table of MEOS
 * (as the vendored core-math/ in coremath), since MEOS also loads libh3 and
 * its own definitions of those names.
 * Whether an edge crossing a face of the icosahedron bends at an extra vertex
 * is the one decision H3 takes in doubles (`_v2dAlmostEquals` of
 * `_v2dIntersect`): the crossing is read here on the integers of the lattice,
 * where it meets an end of the segment exactly or lies a fraction of the
 * segment away from it.
 */

#include "faceijk.c"
#include "vertex.c"

#include "meos_tls.h"
#include "h3/h3_vertex.h"

/** @brief Number of bits of the number of cells whose face coordinates and
 * vertex rotations a thread keeps (#cell_faceijk, #cell_rotations) */
#define H3_CELL_CACHE_BITS 10

/** @brief The face coordinates (`_h3ToFaceIjk`) and the vertex rotations
 * (`vertexRotations`) of the cells read last, per thread, by the cell, as
 * #quadbin_row_latitude keeps the latitudes of the QUADBIN rows: the
 * recipes, the center and the vertices of a cell read them, and so do the
 * cells around it, each of which owns some of its vertices */
typedef struct
{
  H3Index cell;
  FaceIJK fijk;
  int rot;
  bool fset, rset;
} H3CellSlot;
static MEOS_TLS H3CellSlot MEOS_H3_CELL[1 << H3_CELL_CACHE_BITS];

/** @brief Address of #MEOS_H3_CELL, read once by each public function */
MEOS_TLS_ADDRESS(H3CellSlot, cell_table, MEOS_H3_CELL[0])

/**
 * @brief Return the slot of the cell @p cell in the table @p tab, the
 * per-thread #MEOS_H3_CELL whose address each public function of this file
 * reads once and passes down, emptied where it held another cell
 */
static inline H3CellSlot *
cell_slot(H3CellSlot *tab, H3Index cell)
{
  H3CellSlot *e = &tab[MEOS_CACHE_SLOT(cell, H3_CELL_CACHE_BITS)];
  if (e->cell != cell)
  {
    e->cell = cell;
    e->fset = e->rset = false;
  }
  return e;
}

/**
 * @brief Set @p fijk to the face coordinates of the cell @p cell
 * (`_h3ToFaceIjk`), kept per thread (#MEOS_H3_CELL), and return its error
 */
static H3Error
cell_faceijk(H3CellSlot *tab, H3Index cell, FaceIJK *fijk)
{
  H3CellSlot *e = cell_slot(tab, cell);
  if (! e->fset)
  {
    H3Error err = _h3ToFaceIjk(cell, &e->fijk);
    if (err != E_SUCCESS)
      return err;
    e->fset = true;
  }
  *fijk = e->fijk;
  return E_SUCCESS;
}

/**
 * @brief Set @p rot to the vertex rotations of the cell @p cell
 * (`vertexRotations`), kept per thread (#MEOS_H3_CELL), and return its error
 */
static H3Error
cell_rotations(H3CellSlot *tab, H3Index cell, int *rot)
{
  H3CellSlot *e = cell_slot(tab, cell);
  if (! e->rset)
  {
    H3Error err = vertexRotations(cell, &e->rot);
    if (err != E_SUCCESS)
      return err;
    e->rset = true;
  }
  *rot = e->rot;
  return E_SUCCESS;
}

/**
 * @brief Return the number of the vertex of the cell @p origin at which its
 * edge in the direction @p direction starts, or INVALID_VERTEX_NUM:
 * `vertexNumForDirection` of vertex.c on the rotations kept per thread
 * (#cell_rotations)
 */
static int
cell_vertex_num(H3CellSlot *tab, H3Index origin, Direction direction)
{
  int isPent = H3_EXPORT(isPentagon)(origin);
  if (direction == CENTER_DIGIT || direction >= INVALID_DIGIT ||
      (isPent && direction == K_AXES_DIGIT))
    return INVALID_VERTEX_NUM;
  int rotations;
  if (cell_rotations(tab, origin, &rotations) != E_SUCCESS)
    return INVALID_VERTEX_NUM;
  return isPent ?
    (directionToVertexNumPent[direction] + NUM_PENT_VERTS - rotations) %
      NUM_PENT_VERTS :
    (directionToVertexNumHex[direction] + NUM_HEX_VERTS - rotations) %
      NUM_HEX_VERTS;
}

/* The point of the lattice, (2x, 2y / sqrt 3), of the planar point H3 gives
 * the coordinates i, j and k (`_ijkToHex2d`: x = i' - j' / 2,
 * y = j' sqrt(3) / 2, with i' = i - k and j' = j - k) */
static void
lattice_of_ijk(const CoordIJK *c, int64_t *p)
{
  int64_t i = c->i, j = c->j;
  i -= c->k;
  j -= c->k;
  p[0] = 2 * i - j;
  p[1] = j;
}

/* The ends of the edge of the face of the substrate triangle of size maxDim
 * in the direction dir, as faceijk.c states them: v0 = (3m, 0),
 * v1 = (-3m / 2, 3 m sqrt(3) / 2), v2 = (-3m / 2, -3 m sqrt(3) / 2) */
static void
face_edge(int dir, int maxDim, int64_t *e0, int64_t *e1)
{
  int64_t m = maxDim;
  int64_t v[3][2] = { { 6 * m, 0 }, { -3 * m, 3 * m }, { -3 * m, -3 * m } };
  int a = dir == IJ ? 0 : (dir == JK ? 1 : 2);
  int b = (a + 1) % 3;
  e0[0] = v[a][0]; e0[1] = v[a][1];
  e1[0] = v[b][0]; e1[1] = v[b][1];
}

/* Whether the crossing of the segment p0 p1 with the line e0 e1 is one of the
 * ends of the segment, read exactly on the lattice: the fraction t of the
 * segment, num / den, is 0 or 1 */
static bool
crossing_at_end(const int64_t *p0, const int64_t *p1, const int64_t *e0,
  const int64_t *e1)
{
  int64_t s1x = p1[0] - p0[0], s1y = p1[1] - p0[1];
  int64_t s2x = e1[0] - e0[0], s2y = e1[1] - e0[1];
  int64_t num = s2x * (p0[1] - e0[1]) - s2y * (p0[0] - e0[0]);
  int64_t den = - s2x * s1y + s1x * s2y;
  return num == 0 || num == den;
}

static void
recipe_vertex(int face, int res, const CoordIJK *c, H3VertexRecipe *r)
{
  r->face = face;
  r->res = res;
  lattice_of_ijk(c, r->p0);
  r->crossing = false;
  r->substrate = true;
  r->rotate = false;
}

/**
 * @brief Record the points of the outline of a hexagon from its vertex
 * @p start, @p length vertices, as `_faceIjkToCellBoundary` computes them,
 * and return their number
 */
static int
hex_recipes(const FaceIJK *h, int res, int start, int length,
  H3VertexRecipe *out)
{
  int n = 0;
  int adjRes = res;
  FaceIJK centerIJK = *h;
  FaceIJK fijkVerts[NUM_HEX_VERTS];
  _faceIjkToVerts(&centerIJK, &adjRes, fijkVerts);
  int additionalIteration = length == NUM_HEX_VERTS ? 1 : 0;
  int lastFace = -1;
  Overage lastOverage = NO_OVERAGE;
  for (int vert = start; vert < start + length + additionalIteration; vert++)
  {
    int v = vert % NUM_HEX_VERTS;
    FaceIJK fijk = fijkVerts[v];
    Overage overage = _adjustOverageClassII(&fijk, adjRes, 0, 1);
    if (isResolutionClassIII(res) && vert > start && fijk.face != lastFace &&
        lastOverage != FACE_EDGE)
    {
      int lastV = (v + 5) % NUM_HEX_VERTS;
      int face2 = (lastFace == centerIJK.face) ? fijk.face : lastFace;
      H3VertexRecipe *r = &out[n];
      r->face = centerIJK.face;
      r->res = adjRes;
      lattice_of_ijk(&fijkVerts[lastV].coord, r->p0);
      lattice_of_ijk(&fijkVerts[v].coord, r->p1);
      face_edge(adjacentFaceDir[centerIJK.face][face2],
        maxDimByCIIres[adjRes], r->e0, r->e1);
      r->crossing = true;
      r->substrate = true;
      r->rotate = false;
      if (! crossing_at_end(r->p0, r->p1, r->e0, r->e1))
        n++;
    }
    if (vert < start + NUM_HEX_VERTS)
      recipe_vertex(fijk.face, adjRes, &fijk.coord, &out[n++]);
    lastFace = fijk.face;
    lastOverage = overage;
  }
  return n;
}

/**
 * @brief Record the points of the outline of a pentagon from its vertex
 * @p start, @p length vertices, as `_faceIjkPentToCellBoundary` computes
 * them, and return their number
 */
static int
pent_recipes(const FaceIJK *h, int res, int start, int length,
  H3VertexRecipe *out)
{
  int n = 0;
  int adjRes = res;
  FaceIJK centerIJK = *h;
  FaceIJK fijkVerts[NUM_PENT_VERTS];
  _faceIjkPentToVerts(&centerIJK, &adjRes, fijkVerts);
  int additionalIteration = length == NUM_PENT_VERTS ? 1 : 0;
  FaceIJK lastFijk = { 0 };
  for (int vert = start; vert < start + length + additionalIteration; vert++)
  {
    int v = vert % NUM_PENT_VERTS;
    FaceIJK fijk = fijkVerts[v];
    _adjustPentVertOverage(&fijk, adjRes);
    if (isResolutionClassIII(res) && vert > start)
    {
      /* The last vertex read in the coordinates of the face of this one */
      FaceIJK tmpFijk = fijk;
      int currentToLastDir = adjacentFaceDir[tmpFijk.face][lastFijk.face];
      const FaceOrientIJK *fijkOrient =
        &faceNeighbors[tmpFijk.face][currentToLastDir];
      tmpFijk.face = fijkOrient->face;
      CoordIJK *ijk = &tmpFijk.coord;
      for (int i = 0; i < fijkOrient->ccwRot60; i++)
        _ijkRotate60ccw(ijk);
      CoordIJK transVec = fijkOrient->translate;
      _ijkScale(&transVec, unitScaleByCIIres[adjRes] * 3);
      _ijkAdd(ijk, &transVec, ijk);
      _ijkNormalize(ijk);
      H3VertexRecipe *r = &out[n++];
      r->face = tmpFijk.face;
      r->res = adjRes;
      lattice_of_ijk(&lastFijk.coord, r->p0);
      lattice_of_ijk(ijk, r->p1);
      face_edge(adjacentFaceDir[tmpFijk.face][fijk.face],
        maxDimByCIIres[adjRes], r->e0, r->e1);
      r->crossing = true;
      r->substrate = true;
      r->rotate = false;
    }
    if (vert < start + NUM_PENT_VERTS)
      recipe_vertex(fijk.face, adjRes, &fijk.coord, &out[n++]);
    lastFijk = fijk;
  }
  return n;
}

/* The points of the outline of a cell from its vertex start, length vertices,
 * as `vertexToLatLng` and `directedEdgeToBoundary` read them */
static int
cell_recipes(H3CellSlot *tab, H3Index cell, int start, int length,
  H3VertexRecipe *out)
{
  FaceIJK fijk;
  if (cell_faceijk(tab, cell, &fijk) != E_SUCCESS)
    return -1;
  int res = H3_GET_RESOLUTION(cell);
  return H3_EXPORT(isPentagon)(cell) ?
    pent_recipes(&fijk, res, start, length, out) :
    hex_recipes(&fijk, res, start, length, out);
}

/**
 * @brief Return in @p r the inputs of the vertex @p vertex, read from its
 * owner cell as `vertexToLatLng` reads it, and true, or false for an index
 * that is not a vertex
 */
bool
h3_vertex_recipe(uint64 vertex, H3VertexRecipe *r)
{
  if (H3_GET_MODE(vertex) != H3_VERTEX_MODE)
    return false;
  int vertexNum = H3_GET_RESERVED_BITS(vertex);
  H3Index owner = vertex;
  H3_SET_MODE(owner, H3_CELL_MODE);
  H3_SET_RESERVED_BITS(owner, 0);
  H3VertexRecipe rs[2];
  if (cell_recipes(cell_table(), owner, vertexNum, 1, rs) != 1)
    return false;
  *r = rs[0];
  return true;
}

/**
 * @brief Return the number of the vertex of its origin at which the directed
 * edge @p edge starts, the origin in @p origin, as `directedEdgeToBoundary`
 * reads them, or -1 for an index that is not a directed edge
 */
static int
edge_start(H3CellSlot *tab, H3Index edge, H3Index *origin)
{
  if (H3_GET_MODE(edge) != H3_DIRECTEDEDGE_MODE)
    return -1;
  Direction direction = H3_GET_RESERVED_BITS(edge);
  *origin = edge;
  H3_SET_MODE(*origin, H3_CELL_MODE);
  H3_SET_RESERVED_BITS(*origin, 0);
  int startVertex = cell_vertex_num(tab, *origin, direction);
  return startVertex == INVALID_VERTEX_NUM ? -1 : startVertex;
}

/**
 * @brief Return the number of extra vertices of the directed edge @p edge,
 * 0 or 1, the one in @p r, as `directedEdgeToBoundary` reads them, or -1 for
 * an index that is not a directed edge
 */
int
h3_edge_extra_recipe(uint64 edge, H3VertexRecipe *r)
{
  H3CellSlot *tab = cell_table();
  H3Index origin;
  int startVertex = edge_start(tab, edge, &origin);
  if (startVertex < 0)
    return -1;
  H3VertexRecipe rs[3];
  int n = cell_recipes(tab, origin, startVertex, 2, rs);
  if (n == 3)
  {
    *r = rs[1];
    return 1;
  }
  return n == 2 ? 0 : -1;
}

/**
 * @brief Return the number of points of the boundary of the directed edge
 * @p edge, 2 or 3, their inputs in @p out, as `directedEdgeToBoundary`
 * reads them, or -1 for an index that is not a directed edge
 */
int
h3_edge_boundary_recipes(uint64 edge, H3VertexRecipe *out)
{
  H3CellSlot *tab = cell_table();
  H3Index origin;
  int startVertex = edge_start(tab, edge, &origin);
  if (startVertex < 0)
    return -1;
  return cell_recipes(tab, origin, startVertex, 2, out);
}

/**
 * @brief Return the number of points of the boundary of the cell @p cell,
 * at most #H3_BOUNDARY_MAX, their inputs in @p out in the order of
 * `cellToBoundary`, or -1 for an index that is not a cell
 */
int
h3_cell_boundary_recipes(uint64 cell, H3VertexRecipe *out)
{
  if (H3_GET_MODE(cell) != H3_CELL_MODE)
    return -1;
  return cell_recipes(cell_table(), cell, 0, H3_EXPORT(isPentagon)(cell) ?
    NUM_PENT_VERTS : NUM_HEX_VERTS, out);
}

/**
 * @brief Return in @p nb the neighbour of the cell @p cell in the direction
 * @p dir with its rotations in @p nbrot, read by `h3NeighborRotations` once
 * per direction (#h3_cell_vertexes), and true, or false on an error of H3
 */
static bool
cell_neighbour(H3Index cell, Direction dir, H3Index *nb, int *nbrot,
  bool *have)
{
  if (! have[dir])
  {
    int r = 0;
    if (h3NeighborRotations(cell, dir, &r, &nb[dir]) != E_SUCCESS)
      return false;
    nbrot[dir] = r;
    have[dir] = true;
  }
  return true;
}

/**
 * @brief Set @p out to the vertex indices of the cell @p cell in the order
 * of `cellToVertexes`, the last H3_NULL for a pentagon, and, where @p across
 * is not NULL, @p across to the cell across each edge, the edge k running
 * from the vertex k to the next; return their number, 5 or 6, or -1 for an
 * index that is not a cell
 * @details The rule of `cellToVertex` of vertex.c, step for step: a vertex
 * belongs to the lowest index of the three cells meeting there, and is
 * numbered in that cell. `cellToVertexes` reads it vertex by vertex, so it
 * states the rotations of the cell (`vertexRotations`) twice per vertex and
 * each neighbour twice, once per vertex of the edge they share; here the
 * rotations are stated once and each neighbour once (#cell_neighbour). The
 * cell across the edge k is the neighbour in the direction of the vertex k
 * (`directionForVertexNum`), the left neighbour of that rule
 */
int
h3_cell_vertexes(uint64 cell, uint64 *out, uint64 *across)
{
  if (H3_GET_MODE(cell) != H3_CELL_MODE)
    return -1;
  int isPent = H3_EXPORT(isPentagon)(cell);
  int n = isPent ? NUM_PENT_VERTS : NUM_HEX_VERTS;
  int res = H3_GET_RESOLUTION(cell);
  H3CellSlot *tab = cell_table();
  int rot;
  if (cell_rotations(tab, cell, &rot) != E_SUCCESS)
    return -1;
  H3Index nb[NUM_DIGITS];
  int nbrot[NUM_DIGITS];
  bool have[NUM_DIGITS] = { false };
  for (int k = 0; k < n; k++)
  {
    H3Index owner = cell;
    int ownerVertexNum = k;
    Direction left = isPent ? vertexNumToDirectionPent[(k + rot) % n] :
      vertexNumToDirectionHex[(k + rot) % n];
    if (across)
    {
      if (! cell_neighbour(cell, left, nb, nbrot, have))
        return -1;
      across[k] = nb[left];
    }
    if (res == 0 || H3_GET_INDEX_DIGIT(cell, res) != CENTER_DIGIT)
    {
      if (! cell_neighbour(cell, left, nb, nbrot, have))
        return -1;
      H3Index leftNeighbor = nb[left];
      int lRotations = nbrot[left];
      if (leftNeighbor < owner)
        owner = leftNeighbor;
      if (res == 0 || H3_GET_INDEX_DIGIT(leftNeighbor, res) != CENTER_DIGIT)
      {
        int km = (k - 1 + n) % n;
        Direction right = isPent ? vertexNumToDirectionPent[(km + rot) % n] :
          vertexNumToDirectionHex[(km + rot) % n];
        if (! cell_neighbour(cell, right, nb, nbrot, have))
          return -1;
        H3Index rightNeighbor = nb[right];
        if (rightNeighbor < owner)
        {
          int rRotations = nbrot[right];
          owner = rightNeighbor;
          Direction dir = H3_EXPORT(isPentagon)(owner) ?
            directionForNeighbor(owner, cell) :
            DIRECTIONS[(revNeighborDirectionsHex[right] + rRotations) %
              NUM_HEX_VERTS];
          ownerVertexNum = cell_vertex_num(tab, owner, dir);
        }
      }
      if (owner == leftNeighbor)
      {
        int ownerIsPentagon = H3_EXPORT(isPentagon)(owner);
        Direction dir = ownerIsPentagon ? directionForNeighbor(owner, cell) :
          DIRECTIONS[(revNeighborDirectionsHex[left] + lRotations) %
            NUM_HEX_VERTS];
        ownerVertexNum = cell_vertex_num(tab, owner, dir) + 1;
        if (ownerVertexNum == NUM_HEX_VERTS ||
            (ownerIsPentagon && ownerVertexNum == NUM_PENT_VERTS))
          ownerVertexNum = 0;
      }
    }
    H3Index vertex = owner;
    H3_SET_MODE(vertex, H3_VERTEX_MODE);
    H3_SET_RESERVED_BITS(vertex, ownerVertexNum);
    out[k] = vertex;
  }
  if (isPent)
    out[NUM_PENT_VERTS] = H3_NULL;
  return n;
}

/**
 * @brief Return in @p r the inputs of the center of the cell @p cell, read
 * as `cellToLatLng` reads it (`_faceIjkToVec3`: the point of its coordinates
 * on the grid of its own resolution, the angle turned for a Class III
 * resolution), and true, or false for an index that is not a cell
 */
bool
h3_cell_center_recipe(uint64 cell, H3VertexRecipe *r)
{
  FaceIJK fijk;
  if (H3_GET_MODE(cell) != H3_CELL_MODE ||
      cell_faceijk(cell_table(), cell, &fijk) != E_SUCCESS)
    return false;
  int res = H3_GET_RESOLUTION(cell);
  r->face = fijk.face;
  r->res = res;
  lattice_of_ijk(&fijk.coord, r->p0);
  r->crossing = false;
  r->substrate = false;
  r->rotate = isResolutionClassIII(res);
  return true;
}
