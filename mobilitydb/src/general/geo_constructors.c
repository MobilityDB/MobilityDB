/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Constructors for the 2D geometric operations.
 *
 * This module implements the constructors  for the following geometric
 * types: `point`, `line`, `lseg` (line segment), `box`, `circle`, `path`,
 * and `polygon`.
 *
 * @note These constructors should be submitted as a PR to PostgreSQL.
 */

/* PostgreSQL */
#include "postgres.h"
#include "utils/array.h"
#include <utils/float.h>
#include "utils/geo_decls.h"
#include "utils/lsyscache.h"

/*****************************************************************************/

PG_FUNCTION_INFO_V1(point_constructor);
/**
 * Point Constructor
 */
Datum
point_constructor(PG_FUNCTION_ARGS)
{
  double    x = PG_GETARG_FLOAT8(0);
  double    y = PG_GETARG_FLOAT8(1);
  Point     *point = palloc(sizeof(Point));

  point->x = x;
  point->y = y;
  PG_RETURN_POINT_P(point);
}

PG_FUNCTION_INFO_V1(line_constructor);
/**
 * LINE Constructor
 */
Datum
line_constructor(PG_FUNCTION_ARGS)
{
  double    A = PG_GETARG_FLOAT8(0);
  double    B = PG_GETARG_FLOAT8(1);
  double    C = PG_GETARG_FLOAT8(2);
  LINE     *line = palloc(sizeof(LINE));

  line->A = A;
  line->B = B;
  line->C = C;
  PG_RETURN_LINE_P(line);
}

PG_FUNCTION_INFO_V1(lseg_constructor);
/**
 * LSEG Constructor
 */
Datum
lseg_constructor(PG_FUNCTION_ARGS)
{
  Point     *point1 = PG_GETARG_POINT_P(0);
  Point     *point2 = PG_GETARG_POINT_P(1);
  LSEG     *lseg = palloc(sizeof(LSEG));

  lseg->p[0].x = point1->x;
  lseg->p[0].y = point1->y;
  lseg->p[1].x = point2->x;
  lseg->p[1].y = point2->y;
  PG_RETURN_LSEG_P(lseg);
}

PG_FUNCTION_INFO_V1(box_constructor);
/**
 * BOX Constructor
 */
Datum
box_constructor(PG_FUNCTION_ARGS)
{
  Point     *high = PG_GETARG_POINT_P(0);
  Point     *low = PG_GETARG_POINT_P(1);
  BOX       *box = palloc(sizeof(BOX));

  box->high.x = high->x;
  box->high.y = high->y;
  box->low.x = low->x;
  box->low.y = low->y;
  PG_RETURN_BOX_P(box);
}

PG_FUNCTION_INFO_V1(circle_constructor);
/**
 * CIRCLE Constructor
 */
Datum
circle_constructor(PG_FUNCTION_ARGS)
{
  Point     *center = PG_GETARG_POINT_P(0);
  double    radius = PG_GETARG_FLOAT8(1);
  CIRCLE   *circle = palloc(sizeof(CIRCLE));

  circle->center.x = center->x;
  circle->center.y = center->y;
  circle->radius = radius;
  PG_RETURN_CIRCLE_P(circle);
}

PG_FUNCTION_INFO_V1(path_constructor);
/**
 * PATH Constructor
 */
Datum
path_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  if (ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array)) == 0)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
      errmsg("The input array cannot be empty")));

  bool byval;
  int16 typlen;
  char align;
  Point **points;
  int npts;
  get_typlenbyvalalign(array->elemtype, &typlen, &byval, &align);
  deconstruct_array(array, array->elemtype, typlen, byval, align,
    (Datum **) &points, NULL, &npts);

  PATH     *path;
  int      size;
  int      base_size;
  base_size = sizeof(path->p[0]) * npts;
  size = offsetof(PATH, p) + base_size;

  /* Check for integer overflow */
  if (base_size / npts != sizeof(path->p[0]) || size <= base_size)
    ereport(ERROR,
        (errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
         errmsg("too many points requested")));

  path = palloc(size);

  SET_VARSIZE(path, size);
  path->npts = npts;

  for (int i = 0; i < npts; i++)
  {
    path->p[i].x = points[i]->x;
    path->p[i].y = points[i]->y;
  }

  path->closed = path->p[0].x == path->p[npts - 1].x && path->p[0].y == path->p[npts - 1].y;
  /* prevent instability in unused pad bytes */
  path->dummy = 0;

  PG_RETURN_PATH_P(path);
}

/*---------------------------------------------------------------------
 * Make the smallest bounding box for the given polygon.
 *---------------------------------------------------------------------*/

static void
make_bound_box(POLYGON *poly)
{
  int      i;
  float8    x1,
        y1,
        x2,
        y2;

  Assert(poly->npts > 0);

  x1 = x2 = poly->p[0].x;
  y2 = y1 = poly->p[0].y;
  for (i = 1; i < poly->npts; i++)
  {
    if (float8_lt(poly->p[i].x, x1))
      x1 = poly->p[i].x;
    if (float8_gt(poly->p[i].x, x2))
      x2 = poly->p[i].x;
    if (float8_lt(poly->p[i].y, y1))
      y1 = poly->p[i].y;
    if (float8_gt(poly->p[i].y, y2))
      y2 = poly->p[i].y;
  }

  poly->boundbox.low.x = x1;
  poly->boundbox.high.x = x2;
  poly->boundbox.low.y = y1;
  poly->boundbox.high.y = y2;
  return;
}

PG_FUNCTION_INFO_V1(poly_constructor);
/**
 * POLYGON Constructor
 */
Datum
poly_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  if (ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array)) == 0)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
      errmsg("The input array cannot be empty")));

  bool byval;
  int16 typlen;
  char align;
  get_typlenbyvalalign(array->elemtype, &typlen, &byval, &align);
  Point **points;
  int npts;
  deconstruct_array(array, array->elemtype, typlen, byval, align,
    (Datum **) &points, NULL, &npts);

  POLYGON    *poly;
  int      size;
  int      base_size;
  base_size = sizeof(poly->p[0]) * npts;
  size = offsetof(POLYGON, p) + base_size;

  /* Check for integer overflow */
  if (base_size / npts != sizeof(poly->p[0]) || size <= base_size)
    ereport(ERROR,
        (errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
         errmsg("too many points requested")));

  poly = palloc0(size);  /* zero any holes */

  SET_VARSIZE(poly, size);
  poly->npts = npts;

  for (int i = 0; i < npts; i++)
  {
    poly->p[i].x = points[i]->x;
    poly->p[i].y = points[i]->y;
  }

  make_bound_box(poly);

  PG_RETURN_POLYGON_P(poly);
}

/*****************************************************************************/
