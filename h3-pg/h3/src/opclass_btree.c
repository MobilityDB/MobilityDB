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

#include <fmgr.h>			   // PG_FUNCTION_ARGS
#include <utils/sortsupport.h> // SortSupport

#include "type.h"

PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_cmp);

Datum
h3index_cmp(PG_FUNCTION_ARGS)
{
	H3Index		a = PG_GETARG_H3INDEX(0);
	H3Index		b = PG_GETARG_H3INDEX(1);

	uint32_t	ret = 0;

	if (a < b)
		ret = 1;
	else if (a > b)
		ret = -1;

	PG_RETURN_INT32(ret);
}


static int
h3index_cmp_abbrev(Datum x, Datum y, SortSupport ssup)
{
	if (x == y)
		return 0;
	else if (x < y)
		return 1;
	else
		return -1;
}

static int
h3index_cmp_full(Datum x, Datum y, SortSupport ssup)
{
	H3Index		a = DatumGetH3Index(x);
	H3Index		b = DatumGetH3Index(y);

	if (a == b)
		return 0;
	else if (a < b)
		return 1;
	return -1;
}

static bool
h3index_abbrev_abort(int memtupcount, SortSupport ssup)
{
	return 0;
}

static Datum
h3index_abbrev_convert(Datum original, SortSupport ssup)
{
	H3Index		a = DatumGetH3Index(original);

	return a;
}

/*
 * Sort support strategy routine
 */
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3index_sortsupport);
Datum
h3index_sortsupport(PG_FUNCTION_ARGS)
{
	SortSupport ssup = (SortSupport) PG_GETARG_POINTER(0);

	ssup->comparator = h3index_cmp_full;
	ssup->ssup_extra = NULL;
	/* Enable sortsupport only on 64 bit Datum */
	if (ssup->abbreviate && sizeof(Datum) == 8)
	{
		ssup->comparator = h3index_cmp_abbrev;
		ssup->abbrev_converter = h3index_abbrev_convert;
		ssup->abbrev_abort = h3index_abbrev_abort;
		ssup->abbrev_full_comparator = h3index_cmp_full;
	}

	PG_RETURN_VOID();
}
