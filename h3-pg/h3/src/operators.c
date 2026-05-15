/*
 * Copyright 2024 Zacharias Knudsen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <postgres.h>
#include <h3api.h>

#include <fmgr.h> // PG_FUNCTION_ARGS

#include "error.h"
#include "type.h"

PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_distance);

/* b-tree */
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_eq);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_ne);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_lt);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_le);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_gt);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_ge);

/* r-tree */
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_overlaps);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_contains);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_contained_by);

/* static helpers */
static int
containment(H3Index a, H3Index b)
{
	H3Index		aParent = a;
	H3Index		bParent = b;
	int			aRes = getResolution(a);
	int			bRes = getResolution(b);

	if (aRes > bRes)
		h3_assert(cellToParent(a, bRes, &aParent));
	else if (aRes < bRes)
		h3_assert(cellToParent(b, aRes, &bParent));

	/* a contains b */
	if (a == bParent)
		return 1;

	/* a contained by b */
	if (b == aParent)
		return -1;

	/* no overlap */
	return 0;
}

/* distance operator allowing for different resolutions */
Datum
h3index_distance(PG_FUNCTION_ARGS)
{
	H3Index		a = PG_GETARG_H3INDEX(0);
	H3Index		b = PG_GETARG_H3INDEX(1);
	int			resA = getResolution(a);
	int			resB = getResolution(b);
	H3Error		error;
	int64_t		distance;

	if (resA < resB)
		h3_assert(cellToCenterChild(a, resB, &a));
	else if (resB < resA)
		h3_assert(cellToCenterChild(b, resA, &b));

	error = gridDistance(a, b, &distance);
	/* h3_assert(error); */
	if (error)
		distance = -1;

	PG_RETURN_INT64(distance);
}

/* b-tree operators */
Datum
h3index_eq(PG_FUNCTION_ARGS)
{
	H3Index		a = PG_GETARG_H3INDEX(0);
	H3Index		b = PG_GETARG_H3INDEX(1);
	bool		ret = a == b;

	PG_RETURN_BOOL(ret);
}

Datum
h3index_ne(PG_FUNCTION_ARGS)
{
	H3Index		a = PG_GETARG_H3INDEX(0);
	H3Index		b = PG_GETARG_H3INDEX(1);
	bool		ret = a != b;

	PG_RETURN_BOOL(ret);
}

Datum
h3index_lt(PG_FUNCTION_ARGS)
{
	H3Index		a = PG_GETARG_H3INDEX(0);
	H3Index		b = PG_GETARG_H3INDEX(1);
	bool		ret = a < b;

	PG_RETURN_BOOL(ret);
}

Datum
h3index_le(PG_FUNCTION_ARGS)
{
	H3Index		a = PG_GETARG_H3INDEX(0);
	H3Index		b = PG_GETARG_H3INDEX(1);
	bool		ret = a <= b;

	PG_RETURN_BOOL(ret);
}

Datum
h3index_gt(PG_FUNCTION_ARGS)
{
	H3Index		a = PG_GETARG_H3INDEX(0);
	H3Index		b = PG_GETARG_H3INDEX(1);
	bool		ret = a > b;

	PG_RETURN_BOOL(ret);
}

Datum
h3index_ge(PG_FUNCTION_ARGS)
{
	H3Index		a = PG_GETARG_H3INDEX(0);
	H3Index		b = PG_GETARG_H3INDEX(1);
	bool		ret = a >= b;

	PG_RETURN_BOOL(ret);
}

/* r-tree operators */
Datum
h3index_overlaps(PG_FUNCTION_ARGS)
{
	H3Index		a = PG_GETARG_H3INDEX(0);
	H3Index		b = PG_GETARG_H3INDEX(1);
	bool		ret = containment(a, b) != 0;

	PG_RETURN_BOOL(ret);
}

Datum
h3index_contains(PG_FUNCTION_ARGS)
{
	H3Index		a = PG_GETARG_H3INDEX(0);
	H3Index		b = PG_GETARG_H3INDEX(1);
	bool		ret = containment(a, b) > 0;

	PG_RETURN_BOOL(ret);
}

Datum
h3index_contained_by(PG_FUNCTION_ARGS)
{
	H3Index		a = PG_GETARG_H3INDEX(0);
	H3Index		b = PG_GETARG_H3INDEX(1);
	bool		ret = containment(a, b) < 0;

	PG_RETURN_BOOL(ret);
}
