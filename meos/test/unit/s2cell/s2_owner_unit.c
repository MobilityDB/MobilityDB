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
 * @brief Test of the owner of a boundary point of an S2 cell of
 * meos/src/s2cell/s2cell.c
 * @details For every cell of the levels 0 to a maximum, the argument of the
 * program (4 by default), each edge of its ring is held by exactly one of the
 * two cells sharing it (#s2cell_cell_edge_held), and each vertex by exactly
 * one of the cells meeting there (#s2cell_cell_vertex_held), the cube edges,
 * the cube corners and the poles included. Each edge a cell holds faces its
 * lower index coordinates, and the north pole and the two
 * cube corners no cell holds as its low corner have the owners the rule of
 * the cell walks names (doc/contributing/cells/RULE.md). A corner is named
 * by its exact point of the cube, scaled to integers: the cube coordinate
 * `2 a / n - 1` of the index `a` of a level of `n` cells a side is read as
 * `2 a - n`, which
 * names the same point since the projection of the face is symmetric about
 * its centre. The test also checks that the vertex k of
 * #s2cell_cell_vertices is the corner the two functions read as k: it lies
 * nearer that corner, placed by the quadratic projection of S2, than any
 * other corner of the cell.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <meos.h>
#include "s2cell/s2cell.h"

#define MAXLEVEL 4

/* The point of the cube of the corner (a, b) of a face, scaled by n */
static void
cube_corner(int face, int a, int b, int n, int p[3])
{
  int u = 2 * a - n, v = 2 * b - n;
  switch (face)
  {
    case 0: p[0] = n; p[1] = u; p[2] = v; break;
    case 1: p[0] = - u; p[1] = n; p[2] = v; break;
    case 2: p[0] = - u; p[1] = - v; p[2] = n; break;
    case 3: p[0] = - n; p[1] = - v; p[2] = - u; break;
    case 4: p[0] = v; p[1] = - n; p[2] = - u; break;
    default: p[0] = v; p[1] = u; p[2] = - n; break;
  }
}

/* A table of keys, three or six integers, and the number of owners of each */
typedef struct
{
  int (*key)[6];
  int *owners;
  int count, size;
} Table;

static void
table_add(Table *t, const int *key, int len, int owners)
{
  int k[6] = {0};
  memcpy(k, key, (size_t) len * sizeof(int));
  for (int i = 0; i < t->count; i++)
    if (memcmp(t->key[i], k, sizeof k) == 0)
    {
      t->owners[i] += owners;
      return;
    }
  if (t->count == t->size)
  {
    t->size = t->size ? 2 * t->size : 64;
    t->key = realloc(t->key, (size_t) t->size * sizeof *t->key);
    t->owners = realloc(t->owners, (size_t) t->size * sizeof(int));
    if (! t->key || ! t->owners)
    {
      printf("FAIL: out of memory\n");
      exit(EXIT_FAILURE);
    }
  }
  memcpy(t->key[t->count], k, sizeof k);
  t->owners[t->count++] = owners;
}

/* The cube coordinate of the face coordinate s, by the quadratic projection
 * of S2 */
static double
st_to_uv(double s)
{
  return (s >= 0.5) ? (4.0 * s * s - 1.0) / 3.0 :
    (1.0 - 4.0 * (1.0 - s) * (1.0 - s)) / 3.0;
}

/* The direction of the corner (a, b) of a face of n cells a side */
static void
corner_dir(int face, int a, int b, int n, double d[3])
{
  double u = st_to_uv((double) a / n), v = st_to_uv((double) b / n);
  switch (face)
  {
    case 0: d[0] = 1; d[1] = u; d[2] = v; break;
    case 1: d[0] = - u; d[1] = 1; d[2] = v; break;
    case 2: d[0] = - u; d[1] = - v; d[2] = 1; break;
    case 3: d[0] = -1; d[1] = - v; d[2] = - u; break;
    case 4: d[0] = v; d[1] = -1; d[2] = - u; break;
    default: d[0] = v; d[1] = u; d[2] = -1; break;
  }
}

/* The squared distance from a position in degrees to a direction */
static double
gap(double lon, double lat, const double p[3])
{
  double x = cos(lat * M_PI / 180) * cos(lon * M_PI / 180);
  double y = cos(lat * M_PI / 180) * sin(lon * M_PI / 180);
  double z = sin(lat * M_PI / 180);
  double nrm = sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
  double dx = x - p[0] / nrm, dy = y - p[1] / nrm, dz = z - p[2] / nrm;
  return dx * dx + dy * dy + dz * dz;
}

int
main(int argc, char **argv)
{
  int maxlevel = argc > 1 ? atoi(argv[1]) : MAXLEVEL;
  meos_initialize();
  int failures = 0;
  for (int level = 0; level <= maxlevel; level++)
  {
    int n = 1 << level;
    uint32_t size = S2_MAX_SIZE >> level;
    Table edges = {0}, verts = {0};
    for (int face = 0; face < 6; face++)
      for (int a = 0; a < n; a++)
        for (int b = 0; b < n; b++)
        {
          S2CellId cell;
          s2cell_from_face_ij((uint32_t) face, (uint32_t) a * size,
            (uint32_t) b * size, (uint32_t) level, &cell);
          /* The corners in the order of #s2cell_cell_vertices */
          const int da[4] = {0, 1, 1, 0}, db[4] = {0, 0, 1, 1};
          int c[4][3];
          double dir[4][3];
          for (int k = 0; k < 4; k++)
          {
            cube_corner(face, a + da[k], b + db[k], n, c[k]);
            corner_dir(face, a + da[k], b + db[k], n, dir[k]);
          }
          double lon[4], lat[4];
          s2cell_cell_vertices(cell, lon, lat);
          for (int k = 0; k < 4; k++)
          {
            for (int m = 0; m < 4; m++)
              if (m != k && gap(lon[k], lat[k], dir[m]) <
                  gap(lon[k], lat[k], dir[k]))
              {
                printf("FAIL level %d: vertex %d of face %d (%d, %d) is not "
                  "its corner %d\n", level, k, face, a, b, k);
                failures++;
                break;
              }
            int e[6];
            const int *p = c[k], *q = c[(k + 1) % 4];
            /* The edge, its two corners in a fixed order */
            if (memcmp(p, q, 3 * sizeof(int)) < 0)
            {
              memcpy(e, p, 3 * sizeof(int));
              memcpy(e + 3, q, 3 * sizeof(int));
            }
            else
            {
              memcpy(e, q, 3 * sizeof(int));
              memcpy(e + 3, p, 3 * sizeof(int));
            }
            bool eheld = s2cell_cell_edge_held(k);
            table_add(&edges, e, 6, eheld ? 1 : 0);
            /* An edge a cell holds faces its lower index coordinates: both
             * its corners lie at the cell's own i, or both at its own j */
            int k1 = (k + 1) % 4;
            bool low = (da[k] == 0 && da[k1] == 0) ||
              (db[k] == 0 && db[k1] == 0);
            if (eheld != low)
            {
              printf("FAIL level %d: edge %d of face %d (%d, %d) is %sheld\n",
                level, k, face, a, b, eheld ? "" : "not ");
              failures++;
            }
            bool vheld = s2cell_cell_vertex_held(cell, k);
            table_add(&verts, c[k], 3, vheld ? 1 : 0);
            /* The vertices RULE.md names: the north pole, the centre of
             * face 2, belongs to the cell of that face whose low corner it
             * is, and each of the two cube corners no cell holds as its low
             * corner to the cell of the lowest face meeting there */
            const int pole[3] = {0, 0, n}, c0[3] = {n, - n, n},
              c1[3] = {- n, n, - n};
            bool named = level > 0 && memcmp(c[k], pole, sizeof pole) == 0;
            bool owner = face == 2 && a == n / 2 && b == n / 2 && k == 0;
            if (memcmp(c[k], c0, sizeof c0) == 0)
            {
              named = true;
              owner = face == 0 && a == 0 && b == n - 1 && k == 3;
            }
            else if (memcmp(c[k], c1, sizeof c1) == 0)
            {
              named = true;
              owner = face == 1 && a == n - 1 && b == 0 && k == 1;
            }
            if (named && vheld != owner)
            {
              printf("FAIL level %d: vertex %d of face %d (%d, %d) is %sheld"
                "\n", level, k, face, a, b, vheld ? "" : "not ");
              failures++;
            }
          }
        }
    int bad_edges = 0, bad_verts = 0;
    for (int i = 0; i < edges.count; i++)
      bad_edges += (edges.owners[i] != 1);
    for (int i = 0; i < verts.count; i++)
      bad_verts += (verts.owners[i] != 1);
    printf("level %d: %d edges and %d vertices, %d edges and %d vertices "
      "without exactly one owner\n", level, edges.count, verts.count,
      bad_edges, bad_verts);
    failures += bad_edges + bad_verts;
    free(edges.key); free(edges.owners);
    free(verts.key); free(verts.owners);
  }
  meos_finalize();
  printf("%d failures\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
